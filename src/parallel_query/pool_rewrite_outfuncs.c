/* -*-pgsql-c-*- */
/*
 * $Header$
 * 
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2013	PgPool Global Development Group
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of the
 * author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * This file was created based on outfuncs.c of PostgreSQL. I retain
 * original header comment of the file below.
 */
/*-------------------------------------------------------------------------
 *
 * outfuncs.c
 *	  Output functions for Postgres tree nodes.
 *
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/dblink/nodes/outfuncs.c,v 1.261.2.1 2005/11/14 23:54:34 tgl Exp $
 *
 * NOTES
 *	  Every node type that can appear in stored rules' parsetrees *must*
 *	  have an output function defined here (as well as an input function
 *	  in readfuncs.c).	For use in debugging, we also provide output
 *	  functions for nodes that appear in raw parsetrees, path, and plan trees.
 *	  These nodes however need not have input functions.
 *
 *-------------------------------------------------------------------------
 */
#include <string.h>
#include <limits.h>

#include "pool.h"
#include "utils/palloc.h"
#include "utils/elog.h"
#include "parser/parser.h"
#include "parser/pool_string.h"
#include "parser/pg_list.h"
#include "parser/pg_trigger.h"
#include "parser/parsenodes.h"
#include "parallel_query/pool_rewrite_query.h"

#define booltostr(x)  ((x) ? "true" : "false")


extern void _outNode(String *str, void *obj);
static void _rewriteNode(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, void *obj);
static void _rewriteList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *node);
static void _rewriteIdList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *node);
static void _rewriteAlias(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Alias *node);
static void _rewriteRangeVar(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeVar *node);
static void _rewriteVar(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Var *node);
static void _rewriteConst(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Const *node);
static void _rewriteParam(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Param *node);
static void _rewriteAggref(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Aggref *node);
static void _rewriteArrayRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ArrayRef *node);
static void _rewriteFuncExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncExpr *node);
static void _rewriteNameArgExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, NamedArgExpr *node);
static void _rewriteOpExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, OpExpr *node);
static void _rewriteDistinctExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DistinctExpr *node);
static void _rewriteScalarArrayOpExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ScalarArrayOpExpr *node);
static void _rewriteBoolExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, BoolExpr *node);
static void _rewriteSubLink(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SubLink *node);
static void _rewriteSubPlan(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SubPlan *node);
static void _rewriteFieldSelect(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FieldSelect *node);
static void _rewriteFieldStore(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FieldStore *node);
static void _rewriteRelabelType(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RelabelType *node);
static void _rewriteConvertRowtypeExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ConvertRowtypeExpr *node);
static void _rewriteCaseExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CaseExpr *node);
static void _rewriteCaseWhen(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CaseWhen *node);
static void _rewriteCaseTestExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CaseTestExpr *node);
static void _rewriteArrayExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ArrayExpr *node);
static void _rewriteRowExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RowExpr *node);
static void _rewriteCoalesceExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CoalesceExpr *node);
static void _rewriteMinMaxExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, MinMaxExpr *node);
static void _rewriteNullIfExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, NullIfExpr *node);
static void _rewriteNullTest(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, NullTest *node);
static void _rewriteBooleanTest(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, BooleanTest *node);
static void _rewriteCoerceToDomain(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CoerceToDomain *node);
static void _rewriteCoerceToDomainValue(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CoerceToDomainValue *node);
static void _rewriteSetToDefault(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SetToDefault *node);
static void _rewriteTargetEntry(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TargetEntry *node);
static void _rewriteRangeTblRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeTblRef *node);
static void _rewriteJoinExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, JoinExpr *node);
static void _rewriteFromExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FromExpr *node);
static void _rewriteCreateStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateStmt *node);
static void _rewriteIndexStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, IndexStmt *node);
static void _rewriteNotifyStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, NotifyStmt *node);
static void _rewriteDeclareCursorStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DeclareCursorStmt *node);
static void _rewriteSelectStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node);
static void _rewriteFuncCall(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncCall *node);
static void _rewriteDefElem(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DefElem *node);
static void _rewriteLockingClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, LockingClause *node);
static void _rewriteColumnDef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ColumnDef *node);
static void _rewriteTypeName(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TypeName *node);
static void _rewriteTypeCast(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TypeCast *node);
static void _rewriteIndexElem(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, IndexElem *node);
static void _rewriteSortGroupClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SortGroupClause *node);
static void _rewriteWindowClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, WindowClause *node);
static void _rewriteSetOperationStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SetOperationStmt *node);
static void _rewriteAExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Expr *node);
static void _rewriteValue(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Value *value);
static void _rewriteColumnRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ColumnRef *node);
static void _rewriteParamRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ParamRef *node);
static void _rewriteAConst(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Const *node);
static void _rewriteA_Indices(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Indices *node);
static void _rewriteA_Indirection(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Indirection *node);
static void _rewriteA_ArrayExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_ArrayExpr *node);
static void _rewriteResTarget(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ResTarget *node);
static void _rewriteConstraint(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Constraint *node);

static void _rewriteSortBy(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SortBy *node);
static void _rewriteInsertStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, InsertStmt *node);
static void _rewriteUpdateStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, UpdateStmt *node);
static void _rewriteDeleteStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DeleteStmt *node);
static void _rewriteTransactionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TransactionStmt *node);
static void _rewriteTruncateStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TruncateStmt *node);
static void _rewriteVacuumStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, VacuumStmt *node);
static void _rewriteExplainStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ExplainStmt *node);
static void _rewriteClusterStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ClusterStmt *node);
static void _rewriteCheckPointStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CheckPointStmt *node);
static void _rewriteClosePortalStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ClosePortalStmt *node);
static void _rewriteListenStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ListenStmt *node);
static void _rewriteUnlistenStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, UnlistenStmt *node);
static void _rewriteLoadStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, LoadStmt *node);
static void _rewriteCopyStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CopyStmt *node);
static void _rewriteDeallocateStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DeallocateStmt *node);
static void _rewriteRenameStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RenameStmt *node);
static void _rewriteCreateRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateRoleStmt *node);
static void _rewriteAlterRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterRoleStmt *node);
static void _rewriteDropRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropRoleStmt *node);
static void _rewriteCreateSchemaStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateSchemaStmt *node);
static void _rewriteVariableSetStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, VariableSetStmt *node);
static void _rewriteVariableShowStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, VariableShowStmt *node);
static void _rewriteConstraintsSetStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ConstraintsSetStmt *node);
static void _rewriteAlterTableStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterTableStmt *node);
static void _rewriteCreateSeqStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateSeqStmt *node);
static void _rewriteAlterSeqStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterSeqStmt *node);
static void _rewriteCreatePLangStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreatePLangStmt *node);
static void _rewriteCreateTableSpaceStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateTableSpaceStmt *node);
static void _rewriteDropTableSpaceStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropTableSpaceStmt *node);
static void _rewriteCreateTrigStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateTrigStmt *node);
static void _rewriteDefineStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DefineStmt *node);
static void _rewriteCreateOpClassStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateOpClassStmt *node);
static void _rewriteDropStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropStmt *node);
static void _rewriteFetchStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FetchStmt *node);
static void _rewriteGrantStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, GrantStmt *node);
static void _rewriteGrantRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, GrantRoleStmt *node);
static void _rewriteCreateFunctionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateFunctionStmt *node);
static void _rewriteAlterFunctionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterFunctionStmt *node);
static void _rewriteCreateCastStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateCastStmt *node);
static void _rewriteReindexStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ReindexStmt *node);
static void _rewriteRuleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RuleStmt *node);
static void _rewriteViewStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ViewStmt *node);
static void _rewriteCreatedbStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreatedbStmt *node);
static void _rewriteAlterDatabaseStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterDatabaseStmt *node);
static void _rewriteAlterDatabaseSetStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterDatabaseSetStmt *node);
static void _rewriteDropdbStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropdbStmt *node);
static void _rewriteCreateDomainStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateDomainStmt *node);
static void _rewriteAlterDomainStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterDomainStmt *node);
static void _rewriteCreateConversionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateConversionStmt *node);
static void _rewritePrepareStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, PrepareStmt *node);
static void _rewriteExecuteStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ExecuteStmt *node);
static void _rewriteLockStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, LockStmt *node);
static void _rewriteCommentStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CommentStmt *node);

static void _rewriteFuncName(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *func_name);
static void _rewriteSetRest(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, VariableSetStmt *node);
static void _rewriteSetTransactionModeList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);
static void _rewriteAlterTableCmd(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterTableCmd *node);
static void _rewriteOptSeqList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *options);
static void _rewritePrivGrantee(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, PrivGrantee *node);
static void _rewriteFuncWithArgs(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncWithArgs *node);
static void _rewriteFunctionParameter(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FunctionParameter *node);
static void _rewritePrivilegeList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);
static void _rewriteFuncOptList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);
static void _rewriteCreatedbOptList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *options);
static void _rewriteOperatorArgTypes(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *args);
static void _rewriteRangeFunction(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeFunction *node);
static void _rewriteWithDefinition(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *def_list);

/* analyze */
static void KeepRewriteQueryCode(RewriteQuery *message, int current_select);
static void KeepMessages(RewriteQuery *message,int current_select,int part);
static int CheckWhereCaluse(Node *BaseSelect,RewriteQuery *message,ConInfoTodblink *dblink,String *str,int true_count);
static int _writewhereClause(A_Expr *expr,RewriteQuery *message,ConInfoTodblink *dblink, String *str,int true_count);
static char *escape_string(char *str);
static void delay_string_append_char(RewriteQuery *message,String *str, char *parts);
static void build_range_info(RewriteQuery *message,DistDefInfo *info,RepliDefInfo *info2,SelectDefInfo *info3,char *alias,int select_num,int i_num);
static void AnalyzeReturnRecord(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);
static void build_virtual_table(RewriteQuery *message,void *obj, int next);
static char *search_type_from_virtual(VirtualTable *virtual,char *table,char *col);
static int _checkVirtualColumn(ColumnRef *col,RewriteQuery *message);
static void SeekColumnName(Aggexpr *agg,ColumnRef *node,int state);
/* rewirte */
static void writeSelectHeader(RewriteQuery *message,ConInfoTodblink *dblink, String *str,int parallel,int state);
static void writeSelectFooter(RewriteQuery *message,String *str,AnalyzeSelect *analyze,int state);
static void KeepRewriteQueryReturnCode(RewriteQuery *message, int r_code);
static void writeRangeHeader(RewriteQuery *message,ConInfoTodblink *dblink, String *str,DistDefInfo *info, RepliDefInfo *info2,char *alias);
static void writeRangeFooter(RewriteQuery *message,ConInfoTodblink *dblink, String *str,DistDefInfo *info, RepliDefInfo *info2,char *alias);
static bool CheckAggOpt(RewriteQuery *message);
static char *GetNameFromColumnRef(ColumnRef *node,bool state);
static void AvgFuncCall(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncCall *node);

static void
_rewriteFuncNameList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);

/* under define is used in _rewritejoinExpr */
#define JDEFAULT 0
#define JNORMAL  1
#define JNATURAL_INNER  2
#define JNATURAL_LEFT  3
#define JNATURAL_RIGHT 4
#define JNATURAL_FULL  5
#define JINNER   6
#define JLEFT    7
#define JRIGHT   8
#define JFULL    9
#define JUSING   10

#define LOADBALANCE false
#define PARALLEL true

#define SELECT_START -1
#define SELECT_FROMCLAUSE 0
#define SELECT_TARGETLIST 1
#define SELECT_WHERECLAUSE 2
#define SELECT_GROUPBYCLAUSE 3
#define SELECT_HAVINGCLAUSE 4
#define SELECT_SORTCLAUSE 5
#define SELECT_OFFSETCLAUSE 6
#define SELECT_LIMITCLAUSE 7
#define SELECT_OTHER 8

static char *escape_string(char *str)
{
	int len = strlen(str), i, j;
	char *es = palloc0(len * 2 + 1);

	if (es == NULL)
	{
		return NULL;
	}

	for (i = 0, j = 0; i < len; i++, j++)
	{
		if (str[i] == '\'')
		{
			es[j++] = '\'';
		}
		else if (str[i] == '\\')
		{
			es[j++] = '\\';
		}
		es[j] = str[i];
	}

	return es;
}

static void
delay_string_append_char(RewriteQuery *message,String *str, char *parts)
{
	if(message->r_code == SELECT_DEFAULT && message->ignore_rewrite == -1)
	{
		if(parts)
		{
			string_append_char( str, parts);
			ereport(DEBUG4,
				(errmsg("debug Rewrite  Query %s", str->data)));
		}
		else
			message->r_code = SELECT_RELATION_ERROR;
	}
}


/*
 * Is this column  member of table (message->table_relname) ?
 * return 1 -- member
 * return 0 or -1 -- not mmeber
 */
static int
_checkVirtualColumn(ColumnRef *col,RewriteQuery *message)
{
	ListCell *c;
	List *list;
	VirtualTable *virtual = NULL;
	AnalyzeSelect *analyze = NULL;
	char first = 0;
	char *tmp_table = NULL;
	char *colname = NULL;
	char *table = NULL;
	int check_col = 0;
	int no = message->current_select;
	int v_colnum;
	int i;

	list = col->fields;
	analyze = message->analyze[no];

	if(list->length > 2 || list->length == 0)
	{
		/* send error message */
		return -1;
	}

	foreach (c, col->fields)
	{
		Node *n = (Node *) lfirst(c);

		if (IsA(n, String))
		{
			Value *v = (Value *) lfirst(c);
			if(list->length == 2 && first == 0)
			{
				first = 1;
				tmp_table = v->val.str;
			}
			else
				colname = v->val.str;
		}
	}

	if(!colname)
		return check_col;

	if(analyze->partstate[SELECT_FROMCLAUSE] == 'S')
	{
		if (message->table_relname)
		{
			if(tmp_table && strcmp(message->table_relname,tmp_table) != 0)
			{
				return check_col;
			}
			table = message->table_relname;
		}
		else
		{
				return check_col;
		}

		virtual = analyze->virtual;
		v_colnum = virtual->col_num;

		for(i = 0; i < v_colnum; i++)
		{
			if(!strcmp(virtual->table_list[i],table) &&
					!strcmp(virtual->col_list[i],colname))
			{
					check_col = 1;
					break;
			}
		}
	} else {
		virtual = analyze->virtual;
		v_colnum = virtual->col_num;

		for(i = 0; i < v_colnum; i++)
		{
			if(tmp_table)
			{
				if(!strcmp(virtual->table_list[i],tmp_table) &&
					!strcmp(virtual->col_list[i],colname))
				{
					check_col = 1;
					break;
				}
			}
			else
			{
				if(!strcmp(virtual->col_list[i],colname))
				{
					check_col = 1;
					break;
				}
			}
		}
	}
	return check_col;
}

static void KeepMessages(RewriteQuery *message,int current_select,int part)
{
	message->current_select = current_select;
	message->part = part;
}

static void KeepRewriteQueryCode(RewriteQuery *message, int current_select)
{
		message->current_select = current_select;
}

static void KeepRewriteQueryReturnCode(RewriteQuery *message, int r_code)
{
 if((message->r_code != SELECT_RELATION_ERROR)
      && (message->r_code != SELECT_PGCATALOG))
    message->r_code = r_code;
}

/*
 * A_Expr check
 * if A_Expr-tree use subquery,function,or other table member
 * rewrite expression as "TRUE"
 *
 * return value is counter of rewriting query.
 */
static int
_writewhereClause(A_Expr *expr,RewriteQuery *message,ConInfoTodblink *dblink, String *str,int true_count)
{
	int message_r_code = message->r_code;

	switch (expr->kind)
	{
		case AEXPR_OP:
			if (list_length(expr->name) == 1)
			{
				KeepRewriteQueryReturnCode(message, SELECT_AEXPR);
				_rewriteNode(NULL, message, dblink, str, expr->lexpr);
				if(message->r_code == SELECT_AEXPR)
				{
					_rewriteNode(NULL, message, dblink, str, expr->rexpr);
					if(message->r_code == SELECT_AEXPR)
					{
						Value *op = (Value *) lfirst(list_head(expr->name));
						KeepRewriteQueryReturnCode(message, message_r_code);
						_rewriteNode(NULL, message, dblink, str, expr->lexpr);
						delay_string_append_char(message, str, op->val.str);
						_rewriteNode(NULL, message, dblink, str, expr->rexpr);
						KeepRewriteQueryReturnCode(message, message_r_code);
					} else {
						KeepRewriteQueryReturnCode(message, message_r_code);
						delay_string_append_char(message, str,"TRUE");
						true_count++;
						break;
					}
				} else {
					KeepRewriteQueryReturnCode(message, message_r_code);
					delay_string_append_char(message, str,"TRUE");
					true_count++;
				}
				break;
			}
			else
			{
				delay_string_append_char(message, str,"TRUE");
				true_count++;
			}
			break;

		case AEXPR_AND:
			delay_string_append_char(message, str, " (");
			true_count = true_count + CheckWhereCaluse(expr->lexpr,message,dblink,str,true_count);
			delay_string_append_char(message, str, " AND ");
			true_count = true_count + CheckWhereCaluse(expr->rexpr,message,dblink,str,true_count);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_OR:
			delay_string_append_char(message, str, " (");
			true_count = true_count + CheckWhereCaluse(expr->lexpr,message,dblink,str,true_count);
			delay_string_append_char(message, str, " OR ");
			true_count = true_count +  CheckWhereCaluse(expr->rexpr,message,dblink,str,true_count);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_OP_ANY:
			/* not implemented yet */
			 break;

		case AEXPR_OP_ALL:
			/* not implemented yet */
			break;
#if 0

		case AEXPR_NOT:
			delay_string_append_char(message, str, " (NOT ");
			CheckWhereCaluse(expr->rexpr,str,tablename,dbname, schemaname,aliasname);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_DISTINCT:
			delay_string_append_char(message, str, " (");
			_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
			delay_string_append_char(message, str, " IS DISTINCT FROM ");
			_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_NULLIF:
			delay_string_append_char(message, str, " NULLIF(");
			_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
			delay_string_append_char(message, str, ", ");
			_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_OF:
			_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
			if (*(char *)lfirst(list_head(node->name)) == '!')
				delay_string_append_char(message, str, " IS NOT OF (");
			else
				delay_string_append_char(message, str, " IS OF (");
				_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
				delay_string_append_char(message, str, ")");
			break;
#endif
		default:
			delay_string_append_char(message, str,"TRUE");
			true_count++;
			break;
	}
	return true_count;
}

/*
 *  Start whereClasue check
 *  call _writewherelause();
 *
 *  if messgae->r_code is SELECT DEFAULT,
 *  	write down whereClause
 *  else
 *  	check whereClause
 *
 *  return value is counter of rewriting query
 *  if return value is zero, all whereclause can insert dblink function.
 */
static int
CheckWhereCaluse(Node *BaseSelect,RewriteQuery *message,ConInfoTodblink *dblink,String *str,int true_count)
{
	A_Expr *expr = NULL;

	if(!IsA(BaseSelect, A_Expr))
	{
		delay_string_append_char(message, str, "TRUE");
		true_count++;
		return true_count;
	}

	expr = (A_Expr *) BaseSelect;

	if(expr->kind == AEXPR_NOT)
	{
		delay_string_append_char(message, str, "TRUE");
		true_count++;
		return true_count;
	}
	true_count = _writewhereClause(expr,message,dblink,str,true_count);
	return true_count;
}

static void _rewriteIdList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *node)
{
	ListCell   *lc;
	char first = 0;

	foreach(lc, node)
	{
		Value *v = lfirst(lc);

		if (first == 0)
			first = 1;
		else
			delay_string_append_char(message, str, ", ");

		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, v->val.str);
		delay_string_append_char(message, str, "\"");
	}
}

static void _rewriteList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *node)
{
	ListCell   *lc;
	char first = 0;
	char state = (char)0;
	int loop = 0;
	bool from;
	int current_select = message->current_select;
	bool lock = false;
	AnalyzeSelect *analyze;

	analyze = message->analyze[current_select];

	if(message->r_code == SELECT_DEFAULT && message->ignore_rewrite == -1 &&
				analyze->part == SELECT_TARGETLIST)
	{
		if(analyze->retlock == false)
		{
			lock = true;
			analyze->retlock = true;
		}
	}

	foreach(lc, node)
	{
		if (first == 0)
		{
			first = 1;
		}
		else
		{
			if(lfirst(lc) && !IsA(lfirst(lc),A_Indices))
				delay_string_append_char(message, str, ",");
		}

		/* init JoinTable */
		if(message->r_code == SELECT_ANALYZE && message->fromClause)
		{
			if(lfirst(lc) && IsA(lfirst(lc),JoinExpr))
			{
				if(!analyze->join)
				{
					analyze->join = (JoinTable *) palloc(sizeof(JoinTable));
				}
				analyze->join->col_num = 0;
				analyze->join->col_list = NULL;
				analyze->join->type_list = NULL;
				analyze->join->table_list = NULL;
				analyze->join->using_list = NULL;
				analyze->join->using_length = 0;
				analyze->join->state = (char)0;
			}
		}

		from = message->fromClause;
		_rewriteNode(BaseSelect, message, dblink, str, lfirst(lc));
		message->current_select = current_select;
		message->fromClause = from;

		if(message->r_code == SELECT_ANALYZE && message->fromClause)
		{
			/*
			if(lfirst(lc))
			{
				if(IsA(lfirst(lc),JoinExpr))
				{
					JoinExpr *join = lfirst(lc);
					if (join->quals)
					{
						_rewriteNode(BaseSelect, message, dblink, str, join->quals);
					}
				}
			}
			*/

			if(loop == 0)
			{
				state = message->table_state;
			}
			else
			{
				if(state =='L' && message->table_state == 'L')
				{
					message->table_state = 'L';
				}
				else if (state == 'L' && message->table_state == 'P')
				{
					message->table_state = 'P';
				}
				else if (state == 'P' && message->table_state == 'L')
				{
					message->table_state = 'P';
				}
				else
				{
					state = 'S';
					message->table_state = 'S';
				}
			}
		}
		loop++;

		if(message->r_code == SELECT_DEFAULT && message->ignore_rewrite == -1 &&
				 analyze->part == SELECT_TARGETLIST)
		{
			ereport(DEBUG2,
				(errmsg("rewriteList select=%d,count=%d", current_select,message->analyze[current_select]->ret_count)));

			if(lock)
				message->analyze[current_select]->ret_count++;
		}
	}

	if(message->r_code == SELECT_DEFAULT && message->ignore_rewrite == -1 &&
				analyze->part == SELECT_TARGETLIST)
	{
		if(analyze->retlock && lock)
		{
			analyze->retlock = false;
		}
	}
}


/*****************************************************************************
 *
 *	Stuff from primnodes.h.
 *
 *****************************************************************************/

static void
_rewriteAlias(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Alias *node)
{
	delay_string_append_char(message, str, " AS ");
	delay_string_append_char(message, str, node->aliasname);
	delay_string_append_char(message, str, " ");

	if (node->colnames)
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->colnames);
		delay_string_append_char(message, str, ")");
	}
}

static void
append_all_virtual(AnalyzeSelect *analyze,char *table)
{
	VirtualTable *virtual;
	SelectDefInfo *select_ret;
	int num;
	int test_num;
	int base;
	int i;
	int counter = 0;

	virtual = analyze->virtual;
	test_num = virtual->col_num;
	select_ret = analyze->select_ret;
	base = select_ret->col_num;

	if(!table)
	{
		num = test_num;
	}
	else
	{
		num = 0;
		for(i = 0; i< test_num; i++)
		{
			if(!strcmp(virtual->table_list[i],table))
				num++;
		}
	}

	if(base == 0)
	{
		select_ret->col_list = (char **) palloc(num * sizeof(char *));
		select_ret->type_list = (char **) palloc(num * sizeof(char *));
		select_ret->return_list = (int *) palloc(num * sizeof(int));
	}
	else
	{
		select_ret->col_list = (char **) repalloc(select_ret->col_list,(base + num) * sizeof(char *));
		select_ret->type_list = (char **) repalloc(select_ret->type_list,(base + num) * sizeof(char *));
		select_ret->return_list = (int *) repalloc(select_ret->return_list,(base + num) * sizeof(int));
	}

	for(i = 0; i< test_num;i++)
	{
		if(table && strcmp(virtual->table_list[i],table))
		{
			continue;
		}
		select_ret->col_list[base + counter]  = virtual->col_list[i];
		select_ret->type_list[base + counter] = virtual->type_list[i];
		select_ret->return_list[base + counter] = -1;
		ereport(DEBUG2,
				(errmsg("append_all_virtual: analyze[%d] col=%s,type=%s", analyze->now_select,
						select_ret->col_list[base + counter],select_ret->type_list[base + counter])));
		counter++;
	}
	select_ret->col_num = base + num;
}

static void
append_select_def_info(SelectDefInfo *select_ret,char *col,char *type)
{
	int base;

	base  = select_ret->col_num;
	ereport(DEBUG2,
			(errmsg("append_select_def_info: base=%d",base)));

	if(!type)
	{
		type = (char *)palloc(sizeof(char) * strlen("text") + 1);
 		strcpy(type,"text");
	}

	if(base == 0)
	{
		select_ret->col_list = (char **) palloc(sizeof(char *));
		select_ret->type_list = (char **) palloc(sizeof(char *));
		select_ret->return_list = (int *) palloc(sizeof(int));
	}
	else
	{
		select_ret->col_list = (char **) repalloc(select_ret->col_list,(base + 1) * sizeof(char *));
		select_ret->type_list = (char **) repalloc(select_ret->type_list,(base + 1) * sizeof(char *));
		select_ret->return_list = (int *) repalloc(select_ret->return_list,(base + 1) * sizeof(int));
	}

	select_ret->col_list[base] = col;
	select_ret->type_list[base] = type;
	select_ret->return_list[base] = -1;

	select_ret->col_num++;
	ereport(DEBUG2,
			(errmsg("append_select_def_info: col=%s,type=%s base=%d",
					select_ret->col_list[base],select_ret->type_list[base],select_ret->col_num)));

}

static char *search_type_from_virtual(VirtualTable *virtual,char *table,char *col)
{
	int num = virtual->col_num;
	int i;

	for(i = 0; i < num; i++)
	{
		if(table)
		{
			if(!strcmp(virtual->table_list[i],table)
					&& !strcmp(virtual->col_list[i],col))
				return virtual->type_list[i];
		}
		else
		{
			if(!strcmp(virtual->col_list[i],col))
				return virtual->type_list[i];
		}
	}
	return NULL;
}


static void
AnalyzeReturnRecord(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list)
{
	int select_num;
	AnalyzeSelect *analyze;
	VirtualTable *virtual;
	ListCell   *lc;
	int n_sublink = 0;
	int c_sublink = 0;

	select_num = message->current_select;

	analyze=message->analyze[select_num];

	virtual = analyze->virtual;

	analyze->select_ret = (SelectDefInfo *) palloc(sizeof(SelectDefInfo));

	analyze->select_ret->valid = false;
	analyze->select_ret->col_num = 0;

	
	ereport(DEBUG2,
			(errmsg("AnalyzeReturnRecord: current_select=%d",select_num)));

	foreach(lc, list)
	{
		ResTarget *target = NULL;
		Node *n = lfirst(lc);
		void *obj;
		char *alias = NULL;
		char *typecast = NULL;
		char *colname = NULL;
		char *table_name = NULL;
		char *gettype = NULL;

		target = (ResTarget *) n;

		/* alias name */
		if(target->name)
			alias = target->name;

    /* type name */
		if(IsA(target->val, TypeCast))
		{
			TypeCast *type = (TypeCast *) target->val;
			TypeName *typename = (TypeName *)type->typeName;
			obj = type->arg;
			typecast = strVal(lfirst(list_head(typename->names)));
		} else {
			obj = target->val;
		}
		if(typecast && alias)
		{
			append_select_def_info(analyze->select_ret,alias,typecast);
			continue;
		}

    /* column name */
		if (obj && (IsA(obj, ColumnRef)))
		{
			int first = 0;
			ListCell *c;
			ColumnRef *col = NULL;

			col = (ColumnRef *) obj;
			foreach (c, col->fields)
			{
				Node *n = (Node *) lfirst(c);

				if (IsA(n, String))
				{
					Value *v = (Value *) lfirst(c);
					if(col->fields->length == 2 && first == 0)
					{
						first = 1;
						table_name = v->val.str;
					}
					else
						colname = v->val.str;
				}
				else if (IsA(n, A_Star))
				{
					colname = "*";
				}
			}
		}
		else if(obj && (IsA(obj, A_Expr)))
		{
			if(alias)
				append_select_def_info(analyze->select_ret,alias,typecast);
			else
			{
				char *colname;
				colname = (char *)palloc(sizeof(char) * strlen("\"?column?\"") + 1);
 				strcpy(colname,"\"?column?\"");
				append_select_def_info(analyze->select_ret,colname,typecast);
			}
			continue;
		}
		else if(obj && (IsA(obj, FuncCall)))
		{
			if(alias)
			{
				append_select_def_info(analyze->select_ret,alias,typecast);
			}
			else
			{
				FuncCall *func = NULL;
				char *funcname;
				func = (FuncCall *) obj;
				funcname = strVal(lfirst(list_head(func->funcname)));
				append_select_def_info(analyze->select_ret,funcname,typecast);
			}
			continue;
		}
		else if (obj && (IsA(obj,SubLink)))
		{
			int i;
			int max = message->analyze_num;
			AnalyzeSelect *sublink = NULL;

			for(i = select_num + 1; i < max;i++)
			{
				sublink = message->analyze[i];

				if(sublink->last_select == select_num
						&& sublink->part == 1 && c_sublink == n_sublink)
					break;
			}

			n_sublink++;
			if(sublink && (sublink->select_ret->col_num == 1))
			{
				if(alias)
				{
					append_select_def_info(analyze->select_ret,alias,sublink->select_ret->type_list[0]);
					continue;
				}
				else
				{
					char *column;
					column = (char *)palloc(sizeof(char) * strlen("\"?column?\"") + 1);
 					strcpy(column,"\"?column?\"");
					append_select_def_info(analyze->select_ret,column,sublink->select_ret->type_list[0]);
					continue;
				}
			}
		}
		else if(obj && (IsA(obj,A_Const)))
		{
			char *column = NULL;

			if(!alias)
			{
				column = (char *)palloc(sizeof(char) * strlen("\"?column?\"") + 1);
 				strcpy(column,"\"?column?\"");
				alias = column;
			}

			append_select_def_info(analyze->select_ret,alias,typecast);
			continue;
		}

		if(colname && !strcmp(colname,"*"))
		{
			append_all_virtual(analyze,table_name);
			continue;
		}

		if(colname)
			gettype = search_type_from_virtual(virtual,table_name,colname);
		else
		{
			char *column = NULL;
			column = (char *)palloc(sizeof(char) * strlen("\"?column?\"") + 1);
 			strcpy(column,"\"?column?\"");
			append_select_def_info(analyze->select_ret,column,NULL);
			continue;
		}

		if(gettype)
		{
			char *t_n;
			char *t_t;

			if(alias)
				t_n = alias;
			else
				t_n = colname;

			if(typecast)
				t_t = typecast;
			else
				t_t = gettype;
			append_select_def_info(analyze->select_ret,t_n,t_t);
		}
	}
}

static void
append_virtual_table(RewriteQuery *message,VirtualTable *virtual,char **col_list,char **type_list,int col_num,char *table_name,char state,int next)
{
	int base;
	int i;

	if(virtual->col_num == 0)
	{
		base = 0;
		virtual->col_list   = (char**) palloc(sizeof(char*) * col_num);
		virtual->type_list  = (char**) palloc(sizeof(char*) * col_num);
		virtual->table_list = (char**) palloc(sizeof(char*) * col_num);
		virtual->state_list = (char*)  palloc(sizeof(char) * col_num);
		virtual->column_no  = (int*)   palloc(sizeof(int) * col_num);
		virtual->valid      = (int*)   palloc(sizeof(int) * col_num);
	}
	else
  {
		base = virtual->col_num;
		virtual->col_list   = (char**) repalloc(virtual->col_list,sizeof(char*)  * (base + col_num));
		virtual->type_list  = (char**) repalloc(virtual->type_list,sizeof(char*) * (base + col_num));
		virtual->table_list = (char**) repalloc(virtual->table_list,sizeof(char*)* (base + col_num));
		virtual->state_list = (char*)  repalloc(virtual->state_list,sizeof(char) * (base + col_num));
		virtual->column_no  = (int*)   repalloc(virtual->column_no,sizeof(int) * (base + col_num));
		virtual->valid      = (int*)   repalloc(virtual->valid,sizeof(int) * (base + col_num));
	}

	for(i = 0; i< col_num; i++)
	{
		int j = base + i;

		virtual->col_list[j]   = col_list[i];
		virtual->type_list[j]  = type_list[i];
		virtual->table_list[j] = table_name;
		virtual->state_list[j] = state;

		/* (next > 0) means this function call from RangeSubselect */
		if(next > 0)
		{
			AnalyzeSelect *analyze = message->analyze[next];
			SelectDefInfo *select_ret;
			select_ret = analyze->select_ret;
			select_ret->return_list[i] = message->column;
			ereport(DEBUG2,
					(errmsg("append_virtual_table return_list[%d]=%d,analyze[%d]",i,message->column,next)));
		}
		virtual->column_no[j] = message->column;
		message->column++;
		virtual->valid[j] = -1;
		ereport(DEBUG2,
				(errmsg("append_virtual_table select=%d, no=%d,col=%s,type=%s,table=%s,state=%c,valid=%d",
			message->current_select,
			virtual->column_no[j],virtual->col_list[j],virtual->type_list[j]
			,virtual->table_list[j],virtual->state_list[j],virtual->valid[j])));
	}
	virtual->col_num = base + col_num;
}


static void append_join_using(AnalyzeSelect *analyze,char **col_list,char **type_list,int col_num,char *table_name)
{
	int num,i,j,k;
	int same[analyze->join->col_num];
	int lvalid[analyze->join->col_num];
	int rvalid[col_num];
	char **using;
	int lc = 0;
	int rc = 0;
	int sc = 0;
	int total;
	JoinTable *join = analyze->join;
	JoinTable *new = (JoinTable *) palloc(sizeof(JoinTable));

	using = join->using_list;
	num = join->using_length;

	for(i = 0; i < join->col_num; i++)
	{
		char *colname = join->col_list[i];
		for(j = 0; j < num; j++)
		{
			if(!strcmp(colname, using[j]))
			{
				same[sc] = i;
				sc++;
			} else {
				lvalid[lc] = i;
				lc++;
			}
		}
	}


	for(i = 0; i < col_num; i++)
	{
		char *colname = col_list[i];
		for(j = 0; j < num; j++)
		{
			if(strcmp(colname, using[j]))
			{
				rvalid[rc] = i;
				rc++;
			}
		}
	}

	total= sc + lc + rc;
	new->col_num = total;
	new->col_list = (char**) palloc(sizeof(char*) * (total));
	new->type_list = (char**) palloc(sizeof(char*) * (total));
	new->table_list = (char**) palloc(sizeof(char*) * (total));

	for(k = 0; k < sc; k++)
	{
		new->col_list[k] = join->col_list[same[k]];
		new->type_list[k] = join->type_list[same[k]];
		new->table_list[k] = join->table_list[same[k]];
	}

	for(k = sc; k < sc + lc; k++)
	{
		new->col_list[k] = join->col_list[lvalid[k - sc]];
		new->type_list[k] = join->type_list[lvalid[k - sc]];
		new->table_list[k] = join->table_list[lvalid[k - sc]];
	}
	for(k = sc + lc; k < sc + lc + rc; k++)
	{
		new->col_list[k] = col_list[rvalid[k - lc - sc]];
		new->type_list[k] = type_list[rvalid[k - lc - sc]];
		if(table_name)
		{
			new->table_list[k] = table_name;
		}
		else
			new->table_list[k] = NULL;
	}

	analyze->join = new;

	for(i = 0; i< analyze->join->col_num; i++)
	{
		ereport(DEBUG2,
				(errmsg("append_join_using no = %d ,col=%s,type=%s,table=%s",
						i,new->col_list[i],new->type_list[i],new->table_list[i])));
	}
}

static void append_join_natural(AnalyzeSelect *analyze,char **col_list,char **type_list,int base,int col_num,char *table_name)
{
	int same[base + 1];
	int rvalid[base + 1];
	int linvalid[col_num + 1];
	int lvalid[col_num +1];
	int i,j,k;
	int sc = 0;
	int vc = 0;
	int ic = 0;
	int total = 0;
	int count;
	JoinTable *join = analyze->join;
	JoinTable *new = (JoinTable *) palloc(sizeof(JoinTable));

	for(i = 0; i < base; i++)
	{
		char *colname = join->col_list[i];
		int match  = 0;
		int array = -1;
		for(j = 0; j < col_num; j++)
		{
			if(!strcmp(colname, col_list[j]))
			{
				match++;
				array = j;
			}
		}

		if(match == 0)
		{
			rvalid[vc] = i;
			vc++;
		}
		else if(match == 1)
		{
			same[sc] = i;
			linvalid[ic] = array;
			sc++;
			ic++;
		}
		else
		{
			/* XXX */
		}
	}

	total=sc + vc + (col_num - ic);
	new->col_num = total;
	new->col_list = (char**) palloc(sizeof(char*) * (total));
	new->type_list = (char**) palloc(sizeof(char*) * (total));
	new->table_list = (char**) palloc(sizeof(char*) * (total));

	for(k = 0; k < sc; k++)
	{
		new->col_list[k] = join->col_list[same[k]];
		new->type_list[k] = join->type_list[same[k]];
		if(table_name)
		{
			new->table_list[k] = table_name;
		}
		else
			new->table_list[k] = join->table_list[same[k]];
	}

	for(k = sc; k < sc + vc; k++)
	{
		new->col_list[k] = join->col_list[rvalid[k - sc]];
		new->type_list[k] = join->type_list[rvalid[k - sc]];
		if(table_name)
		{
			new->table_list[k] = table_name;
		}
		else
			new->table_list[k] = join->table_list[rvalid[k - sc]];
	}

	count = 0;
	for(k = 0; k <col_num ; k++)
	{
		int l = 0;
		bool test = true;
		for(l = 0; l< ic; l++)
		{
			if(k == linvalid[l])
				test = false;
		}
		if(test)
		{
			lvalid[count] = k;
			count++;
		}
	}

	for(k = sc + vc; k < sc + vc + count; k++)
	{
		new->col_list[k] = col_list[lvalid[k - sc - vc]];
		new->type_list[k] = type_list[lvalid[k - sc - vc]];
		if(table_name)
		{
			new->table_list[k] = table_name;
		}
		else
			new->table_list[k] = NULL;
	}
	analyze->join = new;

	for(i = 0; i< analyze->join->col_num; i++)
	{
		ereport(DEBUG2,
				(errmsg("append_join_natural no = %d ,col=%s,type=%s,table=%s",
						i,new->col_list[i],new->type_list[i],new->table_list[i])));
	}
}

static void append_join_simple(JoinTable *join,char **col_list,char **type_list,int col_num,char *table_name)
{
	int base;
	int i;
	ereport(DEBUG2,
			(errmsg("append_join_table start")));

	if(join->col_num == 0)
	{
		base = 0;
		join->col_list = (char**) palloc(sizeof(char*) * col_num);
		join->type_list = (char**) palloc(sizeof(char*) * col_num);
		join->table_list = (char**) palloc(sizeof(char*) * col_num);
	}
	else
	{
		base = join->col_num;
		join->col_list = (char**) repalloc(join->col_list,sizeof(char*) * (base + col_num));
		join->type_list = (char**) repalloc(join->type_list,sizeof(char*) * (base + col_num));
		join->table_list = (char**) repalloc(join->table_list,sizeof(char*) * (base + col_num));
	}

	for(i = 0; i< col_num; i++)
	{
		join->col_list[base + i] = col_list[i];
		join->type_list[base + i] = type_list[i];
		join->table_list[base + i] = table_name;
		ereport(DEBUG2,
				(errmsg("append_join_table no = %d ,col=%s,type=%s,table=%s",
						base + i,join->col_list[base + i],join->type_list[base + i],join->table_list[base + i])));
	}
	join->col_num = base + col_num;
}

static void build_join_table(RewriteQuery *message,char *alias, int type)
{
	char *table_name = NULL;
	char state;
	char lstate;
	int left_num,right_num,range_num,select_num;
	DistDefInfo *distinfo = NULL;
	RepliDefInfo *repliinfo = NULL;
	SelectDefInfo *selectinfo = NULL;
	JoinTable *join;
	RangeInfo *range;
	AnalyzeSelect *analyze;

	select_num = message->current_select;
	analyze=message->analyze[select_num];
	join = analyze->join;
	left_num = join->col_num;
	range_num = analyze->rangeinfo_num;
	range = analyze->range[range_num - 1];
	state = range->state;
	lstate = join->state;
	right_num = 0;

	distinfo = range->distinfo;
	repliinfo = range->repliinfo;
	selectinfo = range->selectinfo;

	if(alias)
		table_name = alias;
	else if(!alias && range->alias)
		table_name = range->alias;

	if(distinfo && !repliinfo && !selectinfo)
	{
		right_num = distinfo->col_num;
		if(!table_name)
			table_name = distinfo->table_name;
		ereport(DEBUG2,
				(errmsg("inside build_join_info dist state=%c  %s",range->state,table_name)));

		if(type == JNATURAL_INNER || type == JNATURAL_RIGHT
		  || type == JNATURAL_LEFT || type == JNATURAL_FULL)
			append_join_natural(analyze,distinfo->col_list,distinfo->type_list,join->col_num,distinfo->col_num,table_name);
		else
			append_join_simple(join,distinfo->col_list,distinfo->type_list,distinfo->col_num,table_name);
	}
	else if (repliinfo && !distinfo && !selectinfo)
	{
		right_num = repliinfo->col_num;
		if(!table_name)
			table_name = repliinfo->table_name;

		ereport(DEBUG2,
				(errmsg("inside build_join_info repli state=%c %s",range->state,table_name)));

		if(type == JNATURAL_INNER || type == JNATURAL_RIGHT
		  || type == JNATURAL_LEFT || type == JNATURAL_FULL)
			append_join_natural(analyze,repliinfo->col_list,repliinfo->type_list,join->col_num,repliinfo->col_num,table_name);
		else if(join->using_length != 0)
			append_join_using(analyze,repliinfo->col_list,repliinfo->type_list,repliinfo->col_num,table_name);
		else
			append_join_simple(join,repliinfo->col_list,repliinfo->type_list,repliinfo->col_num,table_name);
	}
	else if (selectinfo && !repliinfo && !distinfo)
	{
		ereport(DEBUG2,
				(errmsg("inside build_join_info select state=%c %s",range->state,table_name)));
		right_num = selectinfo->col_num;

		if(type == JNATURAL_INNER || type == JNATURAL_RIGHT
		  || type == JNATURAL_LEFT || type == JNATURAL_FULL)
			append_join_natural(analyze,selectinfo->col_list,selectinfo->type_list,join->col_num,selectinfo->col_num,table_name);
		else if(join->using_length != 0)
			append_join_using(analyze,selectinfo->col_list,selectinfo->type_list,selectinfo->col_num,table_name);
		else
			append_join_simple(join,selectinfo->col_list,selectinfo->type_list,selectinfo->col_num,table_name);
	}

	if(type == JDEFAULT)
	{
		join->state = state;
		return;
	}

	join =analyze->join;

	if(lstate =='E' || state == 'E')
	{
		join->state = 'E';
		return;
	}
	if(lstate =='L' && state =='L')
	{
		join->state = 'L';
		return;
	}
	if(lstate =='L' && state =='P')
	{
		int total = left_num + right_num;
		if(type == JRIGHT || type == JNORMAL || type == JINNER || type == JNATURAL_INNER)
			join->state = 'P';
		else if((join->col_num <= total) && (type == JNATURAL_RIGHT))
			join->state = 'P';
		else if((join->col_num == total) && (type == JNATURAL_LEFT))
			join->state = 'P';
		else
			join->state = 'S';
	}
	else if(lstate =='P' && state == 'L')
	{
		int total = left_num + right_num;
		if(type == JLEFT || type == JNORMAL || type == JINNER || type == JNATURAL_INNER)
			join->state ='P';
		else if((join->col_num <= total) && (type == JNATURAL_LEFT))
			join->state = 'P';
		else if((join->col_num == total) && (type == JNATURAL_RIGHT))
			join->state = 'P';
		else
			join->state ='S';
	}
	else
		join->state = 'S';
}

static void change_analyze_state(AnalyzeSelect *analyze,char state)
{
	if(state == 'E' || analyze->state == 'E')
		return;

	if(!analyze->state)
	{
		analyze->state = state;
	}
	else if(analyze->state == 'P' && state =='L')
	{
		analyze->state = 'P';
	}
	else if(analyze->state == 'L' && state =='P')
	{
		analyze->state = 'P';
	}
	else if(analyze->state == 'L' && state =='L')
	{
			return;
	}
	else
		analyze->state = 'S';
}

static void build_virtual_table(RewriteQuery *message,void *obj,int next)
{
	int select_num;
	int range_num;
	char *alias = NULL;
	char *table_name = NULL;
	char state;
	AnalyzeSelect *analyze;
	VirtualTable *virtual;
	RangeInfo *range;
	DistDefInfo *distinfo;
	RepliDefInfo *repliinfo;
	SelectDefInfo *selectinfo;

	select_num = message->current_select;
	analyze=message->analyze[select_num];
	virtual = analyze->virtual;

	/* last range */
	range_num = analyze->rangeinfo_num;
	range = analyze->range[range_num - 1];
	distinfo = range->distinfo;
	repliinfo = range->repliinfo;
	selectinfo = range->selectinfo;

	if(range->alias)
		alias = range->alias;

	state = range->state;

	if(distinfo && !repliinfo && !selectinfo)
	{
		if(alias)
			table_name =alias;
		else
			table_name = distinfo->table_name;

		ereport(DEBUG2,
				(errmsg("inside build_virtual_info dist state=%c  %s",range->state,table_name)));
		append_virtual_table(message,virtual,distinfo->col_list,distinfo->type_list,distinfo->col_num,table_name,state,-2);
		change_analyze_state(analyze,state);
		return;
	}
	else if (repliinfo && !distinfo && !selectinfo)
	{
		if(alias)
			table_name =alias;
		else
			table_name = repliinfo->table_name;
		ereport(DEBUG2,
				(errmsg("inside build_virtual_info repli state=%c %s",range->state,table_name)));
		append_virtual_table(message,virtual,repliinfo->col_list,repliinfo->type_list,repliinfo->col_num,table_name,state,-3);
		change_analyze_state(analyze,state);
		return;
	}
	else if (selectinfo && !repliinfo	&& !distinfo)
	{
		table_name = alias;
		ereport(DEBUG2,
				(errmsg("inside build_virtual_info select state=%c %s",range->state,table_name)));
		append_virtual_table(message,virtual,selectinfo->col_list,selectinfo->type_list,selectinfo->col_num,table_name,state,next);
		change_analyze_state(analyze,state);
		return;
	}
	else if (!selectinfo && !repliinfo	&& !distinfo)
	{
		ereport(DEBUG2,
				(errmsg("inside build_virtual_info no dist state=%c %s",range->state,alias)));
		change_analyze_state(analyze,'E');
	}
}

static void
build_range_info(RewriteQuery *message,DistDefInfo *info,RepliDefInfo *info2,SelectDefInfo *info3,char *alias, int select_num,int i_num)
{
	int num;
	AnalyzeSelect *analyze;

	analyze=message->analyze[select_num];

	if(analyze->range)
	{
		++analyze->rangeinfo_num;
		num = analyze->rangeinfo_num;
		analyze->range =
							(RangeInfo **) repalloc(analyze->range,sizeof(RangeInfo *) * num);

		analyze->range[num-1] = (RangeInfo *) palloc(sizeof(RangeInfo));
		ereport(DEBUG2,
				(errmsg("inside build_range_info num= %d current_select=%d",num,select_num)));
	}
	else
	{
		num = 1;
		analyze->range = (RangeInfo **) palloc(sizeof(RangeInfo *));
		analyze->range[0] = (RangeInfo *) palloc(sizeof(RangeInfo));
		analyze->rangeinfo_num = 1;
		ereport(DEBUG2,
				(errmsg("inside build_range_info num= %d current_select=%d",num,select_num)));
	}

	analyze->range[num -1]->ret_num = num - 1;
	analyze->range[num -1]->selectinfo = NULL;
	/* set dist_def_info */
	if(info && !info2 && !info3)
	{
		message->is_loadbalance = false;
		analyze->range[num -1]->distinfo = info;
		analyze->range[num -1]->repliinfo = NULL;
		analyze->select_ret = NULL;
		analyze->range[num -1]->alias = alias;
		analyze->range[num -1]->state = 'P';
		ereport(DEBUG2,
				(errmsg("inside build_range_info dist %d",select_num)));
	}

	/* set repli_def_info */
	if(info2 && !info && !info3)
	{
		analyze->range[num -1]->distinfo = NULL;
		analyze->range[num -1]->repliinfo = info2;
		analyze->select_ret = NULL;
		analyze->range[num -1]->alias = alias;
		analyze->range[num -1]->state = 'L';
		ereport(DEBUG2,
				(errmsg("inside build_range_info repli %d",select_num)));
		return;
	}

	/* CALL FROM _rertiteRangeSubselect */
	if(info3 && !info && !info2)
	{
		char state = (char)0;
		analyze->range[num -1]->distinfo = NULL;
		analyze->range[num -1]->repliinfo = NULL;
		analyze->range[num -1]->selectinfo = info3;
		analyze->range[num -1]->alias = alias;
		state = message->analyze[i_num]->state;
		analyze->range[num -1]->state = state;
		ereport(DEBUG2,
				(errmsg("inside build_range_info select %d, state = %c (%c)",select_num,state,analyze->state)));
		return;
	}

	if(info && info2)
	{
		/*TODO: error*/
		analyze->select_ret = NULL;
		ereport(DEBUG2,
				(errmsg("inside build_range_info error  %d",select_num)));
		return;
	}

	if(!info && !info2 && !info3)
	{
		/*TODO: error*/
		message->is_loadbalance = true;
		analyze->range[num -1]->distinfo = NULL;
		analyze->range[num -1]->repliinfo = NULL;
		analyze->range[num -1]->state = 'E';
		analyze->select_ret = NULL;
		analyze->range[num -1]->alias = alias;
		ereport(DEBUG2,
				(errmsg("inside build_range_info const or func  %d",select_num)));
	}
}

static void
_rewriteRangeVar(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeVar *node)
{
	DistDefInfo *info = NULL;
	RepliDefInfo *info2 = NULL;

	if(message->r_code  == SELECT_ANALYZE)
	{
		info = pool_get_dist_def_info(dblink->dbname, node->schemaname, node->relname);
		info2 = pool_get_repli_def_info(dblink->dbname, node->schemaname, node->relname);
	}

	if(!(message->r_code == SELECT_DEFAULT && message->rewritelock == -1 && message->ignore_rewrite == -1))
	{
		if (node->catalogname)
		{
			delay_string_append_char(message, str, node->catalogname);
			delay_string_append_char(message, str, ".");
		}

		if (node->schemaname)
		{
			delay_string_append_char(message, str, node->schemaname);
			delay_string_append_char(message, str, ".");

			if(strcmp(node->schemaname,"pg_catalog") == 0)
			{
				message->is_pg_catalog = true;
			}
		}

		delay_string_append_char(message, str, node->relname);

		if (node->alias)
		{
			Alias *alias = node->alias;
			_rewriteNode(BaseSelect, message, dblink, str, node->alias);
			if(message->r_code == SELECT_ANALYZE)
				build_range_info(message,info,info2,NULL,alias->aliasname,message->current_select,-1);
		}
		else
		{
			if(message->r_code == SELECT_ANALYZE)
				build_range_info(message,info,info2,NULL,node->relname,message->current_select,-1);
		}

		if (node->inhOpt == INH_YES)
		{
			delay_string_append_char(message, str, " * ");
		}

	}
	else
	{
		/* rewrite query using dblink connection */
		char *alias_name = NULL;
		SelectStmt *select = (SelectStmt *)BaseSelect;

		if(node->alias)
		{
			alias_name = node->alias->aliasname;
		}
		/*
		 * iff schemaname is pg_catalog, send query to
		 * one node not system db.
		 */
		if(node->schemaname &&
			(strcmp(node->schemaname,"pg_catalog") == 0))
		{
			message->is_pg_catalog =true;
			return;
		}

		info = pool_get_dist_def_info(dblink->dbname, node->schemaname, node->relname);
		info2 = pool_get_repli_def_info(dblink->dbname, node->schemaname, node->relname);

		writeRangeHeader(message,dblink,str,info,info2,alias_name);

		if (node->catalogname)
		{
			delay_string_append_char(message, str, node->catalogname);
			delay_string_append_char(message, str, ".");
		}

		if (node->schemaname)
		{
			delay_string_append_char(message, str, node->schemaname);
			delay_string_append_char(message, str, ".");
			message->schemaname = node->schemaname;

			if(strcmp(node->schemaname,"pg_catalog") == 0)
			{
				message->is_pg_catalog = true;
			}

		}
		else
			message->schemaname = NULL;

		delay_string_append_char(message, str, node->relname);

		if (alias_name)
		{
			delay_string_append_char(message, str, " AS ");
			delay_string_append_char(message, str, alias_name);
		}

		if(select->whereClause &&
			!(message->r_code == SELECT_PGCATALOG))
		{
				char * temp = NULL;
				int message_code = message->r_code;
				delay_string_append_char(message, str, " WHERE ");

				if(message->table_relname)
					temp = message->table_relname;

				if(alias_name)
					message->table_relname = alias_name;
				else
					message->table_relname = node->relname;

				message->rewritelock = message->current_select;
				CheckWhereCaluse(select->whereClause, message,dblink,str,0);
				message->rewritelock = -1;
				message->table_relname = temp;
				KeepRewriteQueryReturnCode(message, message_code);
		}

		writeRangeFooter(message,dblink,str,info,info2,alias_name);

		if (node->inhOpt == INH_YES)
		{
			delay_string_append_char(message, str, " * ");
		}
	}

	/*2009/07/27*/
	if(message->r_code == SELECT_ANALYZE && message->fromClause)
	{
		int next = message->analyze_num;
		build_virtual_table(message,node,next);
	}
}

static void
_rewriteVar(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Var *node)
{

}

static void
_rewriteConst(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Const *node)
{

}

static void
_rewriteParam(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Param *node)
{

}

static void
_rewriteAggref(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Aggref *node)
{

}

static void
_rewriteArrayRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ArrayRef *node)
{

}

static void
_rewriteFuncExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncExpr *node)
{

}

static void
_rewriteNameArgExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, NamedArgExpr *node)
{
	delay_string_append_char(message, str, node->name);
	delay_string_append_char(message, str, " := ");
	_rewriteNode(BaseSelect, message, dblink, str, node->arg);
}

static void
_rewriteOpExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, OpExpr *node)
{

}

static void
_rewriteDistinctExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DistinctExpr *node)
{

}

static void
_rewriteScalarArrayOpExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ScalarArrayOpExpr *node)
{

}

static void
_rewriteBoolExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, BoolExpr *node)
{

}

static void
_rewriteSubLink(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SubLink *node)
{
	if(message->r_code == SELECT_AEXPR)
	{
		KeepRewriteQueryReturnCode(message, SELECT_AEXPR_FALSE);
		return;
	}
	_rewriteNode(BaseSelect, message, dblink, str, node->testexpr);

	if (node->operName != NIL)
	{
		Value *v = linitial(node->operName);
		if (strcmp(v->val.str, "=") == 0)
			delay_string_append_char(message, str, " IN ");
		else
		{
			delay_string_append_char(message, str, v->val.str);
		}
	}

	switch (node->subLinkType)
	{
		case EXISTS_SUBLINK:
			delay_string_append_char(message, str, " EXISTS ");
			break;

		case ARRAY_SUBLINK:
			delay_string_append_char(message, str, " ARRAY ");
			break;

		case ANY_SUBLINK:
			if (node->operName != NIL)
			{
				Value *v = linitial(node->operName);
				if (strcmp(v->val.str, "=") != 0)
				{
					delay_string_append_char(message, str, v->val.str);
					delay_string_append_char(message, str, " ANY ");
				}
			}
			break;

		case ALL_SUBLINK:
			delay_string_append_char(message, str, " ALL ");
			break;

		default:
			break;
	}

	if (node->subselect)
	{
		int count = message->current_select;
		int part  = message->part;
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->subselect);
		delay_string_append_char(message, str, ")");
		KeepMessages(message,count,part);
	}
}

static void
_rewriteSubPlan(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SubPlan *node)
{

}

static void
_rewriteFieldSelect(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FieldSelect *node)
{

}

static void
_rewriteFieldStore(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FieldStore *node)
{

}

static void
_rewriteRelabelType(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RelabelType *node)
{

}

static void
_rewriteConvertRowtypeExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ConvertRowtypeExpr *node)
{

}

static void
_rewriteCaseExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CaseExpr *node)
{
	ListCell *lc;

	delay_string_append_char(message, str, "CASE ");
	if (node->arg)
		_rewriteNode(BaseSelect, message, dblink, str, node->arg);

	foreach (lc, node->args)
	{
		_rewriteNode(BaseSelect, message, dblink, str, lfirst(lc));
	}

	if (node->defresult)
	{
		delay_string_append_char(message, str, " ELSE ");
		_rewriteNode(BaseSelect, message, dblink, str, node->defresult);
	}

	delay_string_append_char(message, str, " END");
}

static void
_rewriteCaseWhen(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CaseWhen *node)
{
	delay_string_append_char(message, str, " WHEN ");
	_rewriteNode(BaseSelect, message, dblink, str, node->expr);
	delay_string_append_char(message, str, " THEN ");
	_rewriteNode(BaseSelect, message, dblink, str, node->result);
}

static void
_rewriteCaseTestExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CaseTestExpr *node)
{

}

static void
_rewriteArrayExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ArrayExpr *node)
{
	delay_string_append_char(message, str, "[");
	_rewriteNode(BaseSelect, message, dblink, str, node->elements);
	delay_string_append_char(message, str, "]");
}

static void
_rewriteRowExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RowExpr *node)
{
	if (node->args == NIL)
		delay_string_append_char(message, str, "ROW ()");
	else
	{
		delay_string_append_char(message, str, "ROW (");
		_rewriteNode(BaseSelect, message, dblink, str, node->args);
		delay_string_append_char(message, str, ")");
	}
}

static void
_rewriteCoalesceExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CoalesceExpr *node)
{
	delay_string_append_char(message, str, "COALESCE (");
	_rewriteNode(BaseSelect, message, dblink, str, node->args);
	delay_string_append_char(message, str, ")");
}

static void
_rewriteMinMaxExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, MinMaxExpr *node)
{
	if (node->op == IS_GREATEST)
	{
		delay_string_append_char(message, str, "GREATEST (");
		_rewriteNode(BaseSelect, message, dblink, str, node->args);
		delay_string_append_char(message, str, ")");
	}
	else if (node->op == IS_LEAST)
	{
		delay_string_append_char(message, str, "LEAST (");
		_rewriteNode(BaseSelect, message, dblink, str, node->args);
		delay_string_append_char(message, str, ")");
	}
}

static void
_rewriteNullIfExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, NullIfExpr *node)
{

}

static void
_rewriteNullTest(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, NullTest *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->arg);
	if (node->nulltesttype == IS_NOT_NULL)
		delay_string_append_char(message, str, " IS NOT NULL");
	else
		delay_string_append_char(message, str, " IS NULL");
}

static void
_rewriteBooleanTest(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, BooleanTest *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->arg);

	switch (node->booltesttype)
	{
		case IS_TRUE:
			delay_string_append_char(message, str, " IS TRUE");
			break;

		case IS_NOT_TRUE:
			delay_string_append_char(message, str, " IS NOT TRUE");
			break;

		case IS_FALSE:
			delay_string_append_char(message, str, " IS FALSE");
			break;

		case IS_NOT_FALSE:
			delay_string_append_char(message, str, " IS NOT FALSE");
			break;

		case IS_UNKNOWN:
			delay_string_append_char(message, str, " IS UNKNOWN");
			break;

		case IS_NOT_UNKNOWN:
			delay_string_append_char(message, str, " IS NOT UNKNOWN");
			break;
	}
}

static void
_rewriteCoerceToDomain(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CoerceToDomain *node)
{

}

static void
_rewriteCoerceToDomainValue(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CoerceToDomainValue *node)
{

}

static void
_rewriteSetToDefault(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SetToDefault *node)
{
	delay_string_append_char(message, str, "DEFAULT");
}

static void
_rewriteTargetEntry(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TargetEntry *node)
{

}

static void
_rewriteRangeTblRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeTblRef *node)
{

}

static int
RetVirtualColumn(VirtualTable *virtual, char *tablename, char *colname)
{
	int col_num = virtual->col_num;
	int i;

	if (tablename && colname)
	{
		for( i = 0; i < col_num; i++)
		{
			if(strcmp(virtual->col_list[i], colname) == 0
			   && strcmp(virtual->table_list[i], tablename) == 0)
			{
				return virtual->column_no[i];
			}
		}
	}
	/* Error */
	return -1;
}

static void 
SearchVirtualColumnID(VirtualTable *virtual, Node *node, char *colname, int *col_id, bool *ambiguous)
{
	int tmp_id;
	char *name = NULL;

	if(IsA(node, RangeVar))
	{
		RangeVar *range = (RangeVar *)node;
		if(range->alias)
		{
			Alias *alias = range->alias;
			name = alias->aliasname;
		} else {
			name = range->relname;
		}
	}
	else if (IsA(node,RangeSubselect))
	{
		RangeSubselect *select = (RangeSubselect *)node;
		Alias *alias = select->alias;
		name = alias->aliasname;
	}
	else if (IsA(node,JoinExpr))
	{
		JoinExpr *jexpr = (JoinExpr *)node;
		if (jexpr->larg)
			SearchVirtualColumnID(virtual, jexpr->larg, colname, col_id, ambiguous);
		if (jexpr->rarg)
			SearchVirtualColumnID(virtual, jexpr->rarg, colname, col_id, ambiguous);
	}

	if (name && (tmp_id = RetVirtualColumn(virtual, name, colname)) != -1)
	{
		if (*col_id == -1)
			*col_id = tmp_id;
		else
			*ambiguous = true;
	}
}

static int
GetVirtualColumnID(VirtualTable *virtual, Node *node, char *colname)
{
	int col_id = -1;
	bool ambiguous = false;
	SearchVirtualColumnID(virtual, node, colname, &col_id, &ambiguous);
	if (ambiguous)
		return -1;
	else
		return col_id;
}

static void
ConvertFromUsingToON(RewriteQuery *message, String *str, JoinExpr *node, int select_num)
{
	char comma = 0;
	ListCell *lc;
	VirtualTable *virtual = message->analyze[select_num]->virtual;

	delay_string_append_char(message, str, " ON");
	foreach (lc, node->usingClause)
	{
		Value *value;
		char lbuf[16];
		char rbuf[16];

		if (comma == 0)
			comma = 1;
		else
			delay_string_append_char(message, str, " AND ");

		value = lfirst(lc);
		snprintf(lbuf, 16, "%d", GetVirtualColumnID(virtual, node->larg, value->val.str));
		snprintf(rbuf, 16, "%d", GetVirtualColumnID(virtual, node->rarg, value->val.str));
		delay_string_append_char(message, str, " \"pool_c$");
		delay_string_append_char(message, str, lbuf);
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, " = ");
		delay_string_append_char(message, str, "\"pool_c$");
		delay_string_append_char(message, str, rbuf);
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, " ");
	}
}

static void
_rewriteJoinExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, JoinExpr *node)
{
	bool from;
	int select_num;
	bool natural = false;
	bool inner   = false;
	bool cross   = false;
	bool full    = false;
	bool left    = false;
	bool right   = false;
	char *aliasname;
	int using_length = 0;
	char **using_list;

	Alias *alias = node->alias;
	if(alias)
		aliasname = alias->aliasname;
	else
		aliasname = NULL;

	from = message->fromClause;
	select_num = message->current_select;

	_rewriteNode(BaseSelect, message, dblink, str, node->larg);

	/* reset message */
	message->fromClause = from;
	message->current_select = select_num;

	if(message->r_code == SELECT_ANALYZE &&
			(IsA(node->larg, RangeVar) || IsA(node->larg,RangeSubselect)))
	{
			build_join_table(message,aliasname,JDEFAULT);
	}


	if (node->isNatural == TRUE)
	{
		natural = true;
		delay_string_append_char(message, str, " NATURAL");
	}

	if (node->jointype == JOIN_INNER)
	{
		if (node->usingClause == NIL && node->quals == NULL && !node->isNatural)
		{
			cross = true;
			delay_string_append_char(message, str, " CROSS JOIN ");
		}
		else
		{
			delay_string_append_char(message, str, " JOIN ");
			inner = true;
		}

	}
	else if (node->jointype == JOIN_LEFT)
	{
		delay_string_append_char(message, str, " LEFT OUTER JOIN ");
		left = true;
	}
	else if (node->jointype == JOIN_FULL)
	{
		delay_string_append_char(message, str, " FULL OUTER JOIN ");
		full = true;
	}
	else if (node->jointype == JOIN_RIGHT)
	{
		delay_string_append_char(message, str, " RIGHT OUTER JOIN ");
		right = true;
	}

	_rewriteNode(BaseSelect, message, dblink, str, node->rarg);

	message->fromClause = from;
	message->current_select = select_num;

	if(message->r_code == SELECT_ANALYZE)
	{
		if(cross && (IsA(node->rarg, RangeVar) || IsA(node->rarg,RangeSubselect)))
			build_join_table(message,aliasname,JNORMAL);

		if(IsA(node->rarg, RangeVar) || IsA(node->rarg,RangeSubselect))
		{
			if(natural && inner)
				build_join_table(message,aliasname,JNATURAL_INNER);
			else if(natural && left)
				build_join_table(message,aliasname,JNATURAL_LEFT);
			else if(natural && right)
				build_join_table(message,aliasname,JNATURAL_RIGHT);
			else if(natural && full)
				build_join_table(message,aliasname,JFULL);
		}
	}

	if (node->usingClause != NIL && IsA(node->usingClause, List))
	{
		ListCell *lc;
		char comma = 0;
		int count = 0;

		using_length= list_length(node->usingClause);
		using_list = (char **) palloc(sizeof(char *) * using_length);

		if(message->r_code == SELECT_DEFAULT && message->rewritelock == -1
				&& message->analyze[select_num]->state == 'S')
		{
			/* Rewrite Using Cluase to On Clause */
			ConvertFromUsingToON(message, str, node, select_num);
		} else {
			delay_string_append_char(message, str, " USING(");

			foreach (lc, node->usingClause)
			{
				Value *value;

				if (comma == 0)
					comma = 1;
				else
					delay_string_append_char(message, str, ",");

				value = lfirst(lc);
				delay_string_append_char(message, str, value->val.str);
				using_list[count] = value->val.str;
				count++;
			}

			message->analyze[select_num]->join->using_length = using_length;
			message->analyze[select_num]->join->using_list = using_list;

			delay_string_append_char(message, str, ")");
		}
	}

	if(message->r_code == SELECT_ANALYZE)
	{
		if(IsA(node->rarg, RangeVar) || IsA(node->rarg,RangeSubselect))
		{
	  	if(!natural && inner)
				build_join_table(message,aliasname,JINNER);
			else if(!natural && left)
				build_join_table(message,aliasname,JLEFT);
			else if(!natural && right)
				build_join_table(message,aliasname,JRIGHT);
			else if(!natural && full)
				build_join_table(message,aliasname,JFULL);
		}
		message->analyze[select_num]->state = message->analyze[select_num]->join->state;
	}

	if (node->quals)
	{
		int on_select = message->analyze_num - 1;

		delay_string_append_char(message, str, " ON ");
		_rewriteNode(BaseSelect, message, dblink, str, node->quals);

		/* This condition means that the sub select is in ON CLAUSE*/
		if(on_select < message->analyze_num -1)
		{
			int count = message->analyze_num - 1 - on_select;
			char joinstate = message->analyze[select_num]->state;

			if(joinstate == 'S')
				return;
			else 
			{
				int i;
				for(i= on_select + 1; i < count + on_select + 1;i++)
				{
					char onstate = message->analyze[i]->state;
					if (onstate == 'S' || onstate == 'P')
					{
						message->analyze[select_num]->state = 'S';
						ereport(DEBUG2,
								(errmsg("_rewriteJoinExpr: Change Join state from %c to %c",joinstate,
								   message->analyze[select_num]->state)));

						return;
					}
				} 
			}
		}
	}
}

static void
_rewriteFromExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FromExpr *node)
{
}

/*****************************************************************************
 *
 *	Stuff from parsenodes.h.
 *
 *****************************************************************************/

static void
_rewriteCreateStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateStmt *node)
{
	delay_string_append_char(message, str, "CREATE ");
	if (node->relation->relpersistence)
		delay_string_append_char(message, str, "TEMP ");
	delay_string_append_char(message, str, "TABLE ");
	_rewriteNode(BaseSelect, message, dblink, str, node->relation);
	delay_string_append_char(message, str, " (");
	_rewriteNode(BaseSelect, message, dblink, str, node->tableElts);
	delay_string_append_char(message, str, ") ");

	if (node->inhRelations != NIL)
	{
		delay_string_append_char(message, str, "INHERITS (");
		_rewriteNode(BaseSelect, message, dblink, str, node->inhRelations);
		delay_string_append_char(message, str, ")");
	}

	if (node->options)
		_rewriteWithDefinition(BaseSelect, message, dblink, str, node->options);

	switch (node->oncommit)
	{
		case ONCOMMIT_DROP:
			delay_string_append_char(message, str, " ON COMMIT DROP");
			break;

		case ONCOMMIT_DELETE_ROWS:
			delay_string_append_char(message, str, " ON COMMIT DELETE ROWS");

		case ONCOMMIT_PRESERVE_ROWS:
			delay_string_append_char(message, str, " ON COMMIT PRESERVE ROWS");
			break;

		default:
      break;
	}

	if (node->tablespacename)
	{
		delay_string_append_char(message, str, " TABLESPACE \"");
		delay_string_append_char(message, str, node->tablespacename);
		delay_string_append_char(message, str, "\"");
	}
}

static void
_rewriteIndexStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, IndexStmt *node)
{
	delay_string_append_char(message, str, "CREATE ");

	if (node->unique == TRUE)
		delay_string_append_char(message, str, "UNIQUE ");

	if (node->concurrent == true)
		delay_string_append_char(message, str, "INDEX CONCURRENTLY \"");
	else
		delay_string_append_char(message, str, "INDEX ");
	if (node->idxname)
	{
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->idxname);
		delay_string_append_char(message, str, "\" ");
	}
	delay_string_append_char(message, str, "\" ON ");
	_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	if (strcmp(node->accessMethod, DEFAULT_INDEX_TYPE))
	{
		delay_string_append_char(message, str, " USING ");
		delay_string_append_char(message, str, node->accessMethod);
	}

	delay_string_append_char(message, str, "(");
	_rewriteNode(BaseSelect, message, dblink, str, node->indexParams);
	delay_string_append_char(message, str, ")");

	if (node->tableSpace)
	{
		delay_string_append_char(message, str, " TABLESPACE \"");
		delay_string_append_char(message, str, node->tableSpace);
		delay_string_append_char(message, str, "\"");
	}

	if (node->whereClause)
	{
		delay_string_append_char(message, str, " WHERE ");
		_rewriteNode(BaseSelect, message, dblink, str, node->whereClause);
	}
}

static void
_rewriteNotifyStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, NotifyStmt *node)
{
	delay_string_append_char(message, str, "NOTIFY ");
	delay_string_append_char(message, str, "\"");
	_rewriteNode(BaseSelect, message, dblink, str, node->conditionname);
	delay_string_append_char(message, str, "\"");
}

static void
_rewriteDeclareCursorStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DeclareCursorStmt *node)
{
	delay_string_append_char(message, str, "DECLARE \"");
	delay_string_append_char(message, str, node->portalname);
	delay_string_append_char(message, str, "\" ");

	if (node->options & CURSOR_OPT_SCROLL)
		delay_string_append_char(message, str, "SCROLL ");
	if (node->options & CURSOR_OPT_BINARY)
		delay_string_append_char(message, str, "BINARY ");
	if (node->options & CURSOR_OPT_INSENSITIVE)
		delay_string_append_char(message, str, "INSENSITIVE ");

	delay_string_append_char(message, str, "CURSOR ");

	if (node->options & CURSOR_OPT_HOLD)
		delay_string_append_char(message, str, "WITH HOLD ");

	delay_string_append_char(message, str, "FOR");
	_rewriteNode(BaseSelect, message, dblink, str, node->query);
}

static void
initSelectStmt(RewriteQuery *message,SelectStmt *node)
{
	int count;
	int last;
	int i;
	AnalyzeSelect *analyze;

	if(message->r_code != SELECT_ANALYZE && message->r_code != SELECT_DEFAULT)
		return;

	count = message->analyze_num++;
	last = message->current_select;
	message->current_select = count;

	if(message->r_code == SELECT_ANALYZE)
	{
		if(count == 0)
		{
			message->analyze = (AnalyzeSelect **) palloc(sizeof(AnalyzeSelect *));
			message->analyze[count]=(AnalyzeSelect *) palloc(sizeof(AnalyzeSelect));
			message->part = SELECT_START;

			analyze = message->analyze[count];
			analyze->now_select = 0;
			analyze->last_select = -1;
			analyze->part = SELECT_START;
			analyze->call_part = SELECT_START;
		}
		else
		{
			message->analyze = (AnalyzeSelect **) repalloc(message->analyze,sizeof(AnalyzeSelect *) * (count+1));
			message->analyze[count]=(AnalyzeSelect *) palloc(sizeof(AnalyzeSelect));
			analyze = message->analyze[count];
			analyze->now_select = count;
			analyze->last_select = last;
			analyze->part = message->part;
			analyze->call_part = message->analyze[last]->part;
		}

		analyze->range =NULL;
		analyze->rangeinfo_num =-1;
		analyze->virtual = NULL;
		analyze->join = NULL;
		analyze->state = (char) 0;
		analyze->aggregate = false;
		analyze->aggexpr = NULL;
		analyze->table_name = NULL;
		analyze->select_range = false;
		analyze->rarg_count = -1;
		analyze->larg_count = -1;
		analyze->ret_count = 0;
		analyze->retlock = false;

		for(i = 0; i< 8; i++)
			analyze->partstate[i] = (char)0;

		if(node->larg && node->rarg)
			analyze->select_union = true;
		else
			analyze->select_union = false;

		ereport(DEBUG2,
				(errmsg("initSelectStmt: ANALYZE now(%d)",message->current_select)));

		/* set default table_state */
		/* S means that tuples was create by systemdb */
		/* L means that tuples was create by one node */
		/* P means that tuples was create by parallel node */
		message->table_state = 'S';
	}
}

static void writeRangeHeader(RewriteQuery *message,ConInfoTodblink *dblink, String *str,DistDefInfo *info, RepliDefInfo *info2,char *alias)
{
	char port[8];
	char *table = NULL;


	ereport(DEBUG2,
			(errmsg("writeRangeHeader select_no=%d",message->current_select)));

	sprintf(port,"%d",dblink->port);

	delay_string_append_char(message, str, "dblink(");
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, "host=");
	delay_string_append_char(message, str, dblink->hostaddr);
	delay_string_append_char(message, str, " dbname=");
	delay_string_append_char(message, str, dblink->dbname);
	delay_string_append_char(message, str, " port=");
	delay_string_append_char(message, str, port);
	delay_string_append_char(message, str, " user=");
	delay_string_append_char(message, str, dblink->user);

	if(strlen(dblink->password))
	{
		delay_string_append_char(message, str, " password=");
		delay_string_append_char(message, str, dblink->password);
	}

	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, ",");
	delay_string_append_char(message, str, "'");

	if(info && !info2)
	{
		delay_string_append_char(message, str, "SELECT pool_parallel(\"");
		if(alias)
			table = alias;
		else
			table = info->table_name;
	}
	else if (!info && info2)
	{
		delay_string_append_char(message, str, "SELECT pool_loadbalance(\"");
		if(alias)
			table = alias;
		else
			table = info2->table_name;
	}

	{
		VirtualTable *virtual = NULL;
		int no = message->current_select;
		int v_colnum;
		int i;
		int first = 0;

		AnalyzeSelect *analyze = message->analyze[no];
		virtual = analyze->virtual;
		v_colnum = virtual->col_num;

		delay_string_append_char(message, str, "SELECT ");
		if(table)
		{
			for(i = 0; i < v_colnum; i++)
			{
				if(!strcmp(virtual->table_list[i],table) && virtual->valid[i] != -1)
				{

					if(first == 0)
					{
						delay_string_append_char(message, str, virtual->table_list[i]);
						delay_string_append_char(message, str, ".");
						delay_string_append_char(message, str, virtual->col_list[i]);
						first = 1;
					}
					else
					{
						delay_string_append_char(message, str, ", ");
						delay_string_append_char(message, str, virtual->table_list[i]);
						delay_string_append_char(message, str, ".");
						delay_string_append_char(message, str, virtual->col_list[i]);
					}
				}
			}
		}

		if(first == 0)
			delay_string_append_char(message, str, " * ");

		delay_string_append_char(message, str, " FROM ");
	}
}

static void writeRangeFooter(RewriteQuery *message,ConInfoTodblink *dblink, String *str,DistDefInfo *info, RepliDefInfo *info2,char *alias)
{
	int i,num;
	char *table = NULL;

	delay_string_append_char(message, str, "\"");
	delay_string_append_char(message, str, ")");
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, ",false)");

	delay_string_append_char(message, str," AS ");

	if(alias)
		table = alias;
	else
	{
		if(info && !info2)
		{
			table = info->table_name;
		}
		else if (!info && info2)
		{
			table = info2->table_name;
		}
	}

	/* send one node */
	if(!table)
	{
		message->r_code = SELECT_RELATION_ERROR;
		/* 
		 * since we are unable to get the table name so nothing much we can
		 * do so bailout
		 */
		return;
	}

	delay_string_append_char(message, str, table);
	delay_string_append_char(message, str, "(");

	{
		VirtualTable *virtual = NULL;
		int first = 0;
		int no = message->current_select;
		AnalyzeSelect *analyze = message->analyze[no];
		virtual = analyze->virtual;
		num = virtual->col_num;

		for(i = 0; i < num; i++)
		{
			char buf[16];
			if(!strcmp(virtual->table_list[i],table) && virtual->valid[i] != -1)
			{

				if(first == 0)
					first = 1;
				else
					delay_string_append_char(message, str, ",");

				snprintf(buf, 16, "%d", analyze->virtual->column_no[i]);
				delay_string_append_char(message, str,"\"pool_c$");
				delay_string_append_char(message, str,buf);
				delay_string_append_char(message, str, "\"");
				delay_string_append_char(message, str, " ");
				delay_string_append_char(message, str,virtual->type_list[i]);
			}
		}

		if(first == 0)
		{
			for(i = 0; i < num; i++)
			{
				char buf[16];
				if(!strcmp(virtual->table_list[i],table))
				{
					if(first == 0)
						first = 1;
					else
						delay_string_append_char(message, str, ",");

					snprintf(buf, 16, "%d", analyze->virtual->column_no[i]);
					delay_string_append_char(message, str,"\"pool_c$");
					delay_string_append_char(message, str,buf);
					delay_string_append_char(message, str,"\"");
					delay_string_append_char(message, str, " ");
					delay_string_append_char(message, str,virtual->type_list[i]);
				}
			}
		}
		delay_string_append_char(message, str, ")");
	}
}

static void writeSelectAggHeader(RewriteQuery *message,ConInfoTodblink *dblink, String *str,int state)
{
	char port[8];
	int count = message->current_select;
	AnalyzeSelect  *analyze;
	Aggexpr *agg;
	int i;
	int ret_count = 0;

	analyze = message->analyze[count];
	agg = analyze->aggexpr;

	ret_count = agg->t_num + agg->c_num + agg->h_num;

	sprintf(port,"%d",dblink->port);
	ereport(DEBUG2,
			(errmsg("writeSelectAggHeader select_no=%d state=%d",message->current_select,state)));

	delay_string_append_char(message, str, "dblink(");
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, "host=");
	delay_string_append_char(message, str, dblink->hostaddr);
	delay_string_append_char(message, str, " dbname=");
	delay_string_append_char(message, str, dblink->dbname);
	delay_string_append_char(message, str, " port=");
	delay_string_append_char(message, str, port);
	delay_string_append_char(message, str, " user=");
	delay_string_append_char(message, str, dblink->user);

	if(strlen(dblink->password))
	{
		delay_string_append_char(message, str, " password=");
		delay_string_append_char(message, str, dblink->password);
	}
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, ",");
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, "SELECT pool_parallel(\"");
	delay_string_append_char(message, str, "SELECT ");

	message->rewritelock = count;

	for(i = 0; i< agg->t_num; i++)
	{
		char *funcname = NULL;
		funcname = strVal(lfirst(list_head(agg->tfunc_p[i]->funcname)));
		if(strcmp(funcname,"avg"))
		{
			_rewriteFuncCall(NULL,message,NULL,str,agg->tfunc_p[i]);
		}
		else
		{
			AvgFuncCall(NULL,message,NULL,str,agg->tfunc_p[i]);
		}
		ret_count--;
		if(ret_count != 0)
			delay_string_append_char(message, str, ",");
	}

	for(i = 0; i< agg->c_num; i++)
	{
		_rewriteColumnRef(NULL,message,NULL,str,agg->col_p[i]);
		ret_count--;
		if(ret_count != 0)
			delay_string_append_char(message, str, ",");
	}

	for(i = 0; i< agg->h_num; i++)
	{
		char *funcname = NULL;
		funcname = strVal(lfirst(list_head(agg->hfunc_p[i]->funcname)));
		if(strcmp(funcname,"avg"))
		{
			_rewriteFuncCall(NULL,message,NULL,str,agg->hfunc_p[i]);
		}
		else
		{
			AvgFuncCall(NULL,message,NULL,str,agg->hfunc_p[i]);
		}
		ret_count--;
		if(ret_count != 0)
			delay_string_append_char(message, str, ",");
	}
	delay_string_append_char(message, str, " FROM ");
}

static void writeSelectHeader(RewriteQuery *message,ConInfoTodblink *dblink, String *str,int parallel,int state)
{
	char port[8];

	sprintf(port,"%d",dblink->port);
	ereport(DEBUG2,
			(errmsg("writeSelectHeader select_no=%d state=%d",message->current_select,state)));

	if(state == SELECT_START)
	{
		delay_string_append_char(message, str, "SELECT * FROM ");
	}

	delay_string_append_char(message, str, "dblink(");
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, "host=");
	delay_string_append_char(message, str, dblink->hostaddr);
	delay_string_append_char(message, str, " dbname=");
	delay_string_append_char(message, str, dblink->dbname);
	delay_string_append_char(message, str, " port=");
	delay_string_append_char(message, str, port);
	delay_string_append_char(message, str, " user=");
	delay_string_append_char(message, str, dblink->user);

	if(strlen(dblink->password))
	{
		delay_string_append_char(message, str, " password=");
		delay_string_append_char(message, str, dblink->password);
	}
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, ",");
	delay_string_append_char(message, str, "'");
	if(parallel)
		delay_string_append_char(message, str, "SELECT pool_parallel(\"");
	else
		delay_string_append_char(message, str, "SELECT pool_loadbalance(\"");

	if(state == SELECT_FROMCLAUSE)
	{
		int no = message->current_select;
		int v_colnum;
		int i;
		int first = 0;

		AnalyzeSelect *analyze = message->analyze[no];

		v_colnum = analyze->virtual->col_num;

		delay_string_append_char(message, str, "SELECT ");

		for(i = 0; i < v_colnum; i++)
		{
			if(analyze->virtual->valid[i] != -1)
			{
				char *col_name = analyze->virtual->col_list[i];

				if(first == 0)
				{
					if(strcmp(col_name,"\"?column?\""))
						delay_string_append_char(message, str, col_name);
					else
					{
						delay_string_append_char(message, str, "\"");
						delay_string_append_char(message, str, col_name);
						delay_string_append_char(message, str, "\"");
					}
					first = 1;
				}
				else
				{
					delay_string_append_char(message, str, ", ");
					delay_string_append_char(message, str, analyze->virtual->table_list[i]);
					delay_string_append_char(message, str, ".");
					if(strcmp(col_name,"\"?column?\""))
						delay_string_append_char(message, str, col_name);
					else
					{
						delay_string_append_char(message, str, "\"");
						delay_string_append_char(message, str, col_name);
						delay_string_append_char(message, str, "\"");
					}
				}
			}
		}

		if(first == 0)
			delay_string_append_char(message, str, " * ");

		delay_string_append_char(message, str, " FROM ");
	}
}

static char *estimateFuncTypes(AnalyzeSelect *analyze,FuncCall *func)
{
	char *funcname = NULL;
	void *obj;
	char *type;
	obj = lfirst(list_head(func->args));

	funcname = strVal(lfirst(list_head(func->funcname)));

	if(!strcmp(funcname,"max") || !strcmp(funcname,"min"))
	{
		if (obj && (IsA(obj, ColumnRef)))
		{
			char *table_name = NULL;
			char *column_name = NULL;
			VirtualTable  *virtual = analyze->virtual;

			column_name = GetNameFromColumnRef((ColumnRef *) obj,true);
			table_name = GetNameFromColumnRef((ColumnRef *) obj ,false);
			type = search_type_from_virtual(virtual,table_name,column_name);
			return type;
		}
		else if( obj && (IsA(obj,TypeCast)))
		{
			TypeCast *typecast = (TypeCast *) obj;
			TypeName *typename = (TypeName *)typecast->typeName;
			type = strVal(lfirst(list_head(typename->names)));
			return type;
		}
		else
		{
	  	char *numeric = "numeric";
			type = (char *) palloc(sizeof(char) * strlen(numeric) +1);
			strcpy(type,numeric);
			return type;
		}
	}
	else if(!strcmp(funcname,"sum"))
	{
	  char *numeric = "numeric";
		type = (char *) palloc(sizeof(char) * strlen(numeric) +1);
		strcpy(type,numeric);
		return type;
	}

	return "numeric";
}

static void writeSelectAggFooter(RewriteQuery *message,String *str,AnalyzeSelect *analyze)
{
	int count = message->current_select;
	Aggexpr *agg;
	int i;
	int ret_count = 0;
	int group_count = 0;

	analyze = message->analyze[count];
	agg = analyze->aggexpr;

	ret_count = agg->t_num + agg->c_num + agg->h_num;

	group_count = agg->c_num;

	if(group_count != 0)
	{
		delay_string_append_char(message, str, " GROUP BY ");

		for(i = 0; i< agg->c_num; i++)
		{
			_rewriteColumnRef(NULL,message,NULL,str,agg->col_p[i]);
			group_count--;
			if(group_count != 0)
				delay_string_append_char(message, str, ",");
		}
	}

	delay_string_append_char(message, str, "\"");
	delay_string_append_char(message, str, ")");
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, ",false)");

	delay_string_append_char(message, str," AS ");
	delay_string_append_char(message, str, analyze->table_name);

	/* symbol of aggregate opt */
	delay_string_append_char(message, str,"g");
	delay_string_append_char(message, str, " (");

	message->rewritelock = -1;

	for(i = 0; i < ret_count; i++)
	{
		char buf[16];
		snprintf(buf, 16, "%d", i);
		delay_string_append_char(message, str, "pool_g$");
		delay_string_append_char(message, str, buf);
		delay_string_append_char(message, str, " ");

		if(i < agg->t_num)
		{
			char *funcname = NULL;
			funcname = strVal(lfirst(list_head(agg->tfunc_p[i]->funcname)));
			if(!strcmp(funcname,"count"))
				delay_string_append_char(message, str, "bigint ");
			else if(!strcmp(funcname,"avg"))
			{
				delay_string_append_char(message, str, "numeric ");
				delay_string_append_char(message, str, ",");
				delay_string_append_char(message, str, "pool_g$");
				delay_string_append_char(message, str, buf);
				delay_string_append_char(message, str, "c");
				delay_string_append_char(message, str, " ");
				delay_string_append_char(message, str, "bigint ");
			}
			else
			{
				char *type = estimateFuncTypes(analyze,agg->tfunc_p[i]);
				delay_string_append_char(message, str, type);
			}
		}
		else if(i >= agg->t_num && i < agg->t_num + agg->c_num)
		{
			char *table_name = NULL;
			char *column_name = NULL;
			char *type = NULL;
			VirtualTable  *virtual = analyze->virtual;

			column_name = GetNameFromColumnRef(agg->col_p[i - agg->t_num],true);
			table_name = GetNameFromColumnRef(agg->col_p[i - agg->t_num],false);
			type = search_type_from_virtual(virtual,table_name,column_name);
			delay_string_append_char(message, str, type);
		}
		else if(i >= agg->t_num + agg->c_num)
		{
			char *funcname = NULL;
			int arg = i - agg->t_num - agg->c_num;
			funcname = strVal(lfirst(list_head(agg->hfunc_p[arg]->funcname)));
			if(!strcmp(funcname,"count"))
				delay_string_append_char(message, str, "bigint ");
			else if(!strcmp(funcname,"avg"))
			{
				delay_string_append_char(message, str, "numeric ");
				delay_string_append_char(message, str, ",");
				delay_string_append_char(message, str, "pool_g$");
				delay_string_append_char(message, str, buf);
				delay_string_append_char(message, str, "c");
				delay_string_append_char(message, str, " ");
				delay_string_append_char(message, str, "bigint ");
			}
			else
			{
				char *type = estimateFuncTypes(analyze,agg->hfunc_p[arg]);
				delay_string_append_char(message, str, type);
			}
		}

		if( i + 1 != ret_count)
			delay_string_append_char(message, str, ",");
	}
	delay_string_append_char(message, str, ") ");
}

static void writeSelectFooter(RewriteQuery *message,String *str,AnalyzeSelect *analyze,int state)
{
	int i,num;

	delay_string_append_char(message, str, "\"");
	delay_string_append_char(message, str, ")");
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, ",false)");

	delay_string_append_char(message, str," AS ");
	delay_string_append_char(message, str, analyze->table_name);
	delay_string_append_char(message, str, "(");
	ereport(DEBUG2,
			(errmsg("writeSelectFooter %s",analyze->table_name)));

	if(state != SELECT_FROMCLAUSE)
	{
		num = analyze->select_ret->col_num;
		for(i = 0; i < num; i++)
		{
			delay_string_append_char(message, str, analyze->select_ret->col_list[i]);
			delay_string_append_char(message, str, " ");
			delay_string_append_char(message, str, analyze->select_ret->type_list[i]);
			if (i != num -1)
				delay_string_append_char(message, str, ",");
		}
		delay_string_append_char(message, str, ")");
	}
	else
	{
		int first = 0;
		num = analyze->virtual->col_num;

		for(i = 0; i < num; i++)
		{
			if(analyze->virtual->valid[i] != -1)
			{
				char buf[16];
				if(first == 0)
				{
					snprintf(buf, 16, "%d", analyze->virtual->column_no[i]);
					delay_string_append_char(message, str, "\"pool_c$");
					delay_string_append_char(message, str, buf);
					delay_string_append_char(message, str, "\"");
					delay_string_append_char(message, str, " ");
					delay_string_append_char(message, str, analyze->virtual->type_list[i]);
					first = 1;
				} else {
					delay_string_append_char(message, str, ",");
					snprintf(buf, 16, "%d", analyze->virtual->column_no[i]);
					delay_string_append_char(message, str, "\"pool_c$");
					delay_string_append_char(message, str, buf);
					delay_string_append_char(message, str, "\"");
					delay_string_append_char(message, str, " ");
					delay_string_append_char(message, str, analyze->virtual->type_list[i]);
				}
			}
		}

		if(first == 0)
		{
			for(i = 0; i < num; i++)
			{
				if(first == 0)
				{
					delay_string_append_char(message, str, analyze->virtual->col_list[i]);
					delay_string_append_char(message, str, " ");
					delay_string_append_char(message, str, analyze->virtual->type_list[i]);
					first = 1;
				} else {
					delay_string_append_char(message, str, ",");
					delay_string_append_char(message, str, analyze->virtual->col_list[i]);
					delay_string_append_char(message, str, " ");
					delay_string_append_char(message, str, analyze->virtual->type_list[i]);
				}
			}
		}

		delay_string_append_char(message, str, ")");
	}
}

static void
CopyFromLeftArg(RewriteQuery *message,int current_num)
{
	AnalyzeSelect *l_analyze = message->analyze[current_num + 1];
	AnalyzeSelect *analyze = message->analyze[current_num];
	VirtualTable  *virtual = message->analyze[current_num]->virtual;
	int col_num;
	int i;
	char **col_list = NULL;
	char **type_list = NULL;
	char state;
	char *table_name;

	table_name = l_analyze->table_name;
	col_list  = l_analyze->select_ret->col_list;
	type_list = l_analyze->select_ret->type_list;
	col_num   = l_analyze->select_ret->col_num;

	state = l_analyze->state;
  append_virtual_table(message,virtual,col_list,type_list,col_num,table_name,state,current_num + 1);

	analyze->select_ret = (SelectDefInfo *) palloc(sizeof(SelectDefInfo));
	analyze->select_ret->valid = false;
	analyze->select_ret->col_num = 0;
	for(i =0; i<col_num; i++)
	{
		append_select_def_info(analyze->select_ret,col_list[i],type_list[i]);
	}
}


static void
ChangeStateByCluase(RewriteQuery *message,void *obj,int before, int after)
{
	AnalyzeSelect *analyze;
	int count = message->current_select;

	if(message->r_code != SELECT_ANALYZE)
		return;

	analyze = message->analyze[count];

	if (obj && analyze->partstate[before] == 'P')
			analyze->partstate[after] = 'S';
	else
			analyze->partstate[after] = analyze->partstate[before];
}

static void
ChangeStateRewriteFooter(RewriteQuery *message,String *str,int defore, int after)
{
	AnalyzeSelect *analyze;
	int count = message->current_select;
	char state;

	if(message->r_code != SELECT_DEFAULT)
		return;

	analyze = message->analyze[count];

	if(analyze->state == 'S' && message->rewritelock == count
		&& message->ignore_rewrite == -1)
	{
		state = analyze->partstate[defore];

		if(state == 'L' || state == 'P')
		{
			if(state != analyze->partstate[after])
			{
				writeSelectFooter(message,str,analyze,SELECT_FROMCLAUSE);
				message->rewritelock = -1;
			}
		}
	}
}

static bool
CheckUnionFromClause(RewriteQuery *message)
{
	int check;
	int count = message->current_select;
	AnalyzeSelect *analyze;
	analyze = message->analyze[count];

	check = analyze->last_select;
	ereport(DEBUG2,
			(errmsg("CheckUnion select=%d last_select=%d", count,check)));

	if(check == -1)
		return false;
	else
	{
		if(message->analyze[check]->select_union &&
				message->analyze[check]->call_part == SELECT_FROMCLAUSE)
		{
			ereport(DEBUG2,
					(errmsg("CheckUnion true")));
			return true;
		}
		else
			return false;
	}
}

static void
_rewriteSelectStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node)
{
	BaseSelect = (Node *) node;
	AnalyzeSelect *analyze;
	int count;
	int target_analyze = 0;
	int from_analyze = 0;
	bool lock = false;
	bool direct = false;
	bool aggrewrite = false;

	count = message->analyze_num;

  /* Allocate Memory Space and initialize some Flags*/
	initSelectStmt(message,node);

	if(message->r_code == SELECT_DEFAULT)
	{
		if(message->current_select == 0)
			ereport(DEBUG2,
			(errmsg("_rewriteSelectStmt:START QueryRewrite")));
	}


	analyze = message->analyze[count];
	analyze->part = SELECT_START;

	if(message->r_code == SELECT_DEFAULT && message->rewritelock == -1
			&& message->ignore_rewrite == -1)
	{
#if 0
		if(analyze->state == 'P')
		{
			if(analyze->call_part != SELECT_FROMCLAUSE || !CheckUnionFromClause(message))
			{
				writeSelectHeader(message,dblink,str,PARALLEL,analyze->part);
				message->rewritelock = count;
				direct = true;
			}
			else
				lock =true;
		}
#endif
		if(analyze->state == 'L')
		{
			if(analyze->call_part != SELECT_FROMCLAUSE && !CheckUnionFromClause(message))
			{
				writeSelectHeader(message,dblink,str,LOADBALANCE,analyze->part);
				message->rewritelock = count;
				direct = true;
			}
			else
				lock =true;
		}
	}

	if (node->larg) /* SETOP */
	{
		if(message->r_code == SELECT_ANALYZE)
		{
			char buf[16];
			char *temp = "pool_t$";
			analyze->virtual = (VirtualTable *) palloc(sizeof(VirtualTable));
			snprintf(buf, 16, "%d", message->virtual_num);
			message->virtual_num++;
			analyze->table_name = (char *) palloc(sizeof(char) * (strlen(temp) + strlen(buf) + 1));
 			strcpy(analyze->table_name,temp);
 			strcat(analyze->table_name,buf);
			analyze->virtual->col_num = 0;
		}

		delay_string_append_char(message, str, "(");
		message->part = SELECT_START;
		_rewriteNode(BaseSelect, message, dblink, str, node->larg);
		ereport(DEBUG2,
			(errmsg("union larg select_no=%d(%d)",count,message->analyze_num)));

		delay_string_append_char(message, str, ")");
		KeepMessages(message,count,SELECT_START);

		/* COPY analyze of left arg */
		if(message->r_code == SELECT_ANALYZE)
		{
			analyze->larg_count = count + 1;
			analyze->rarg_count = message->analyze_num;
			ereport(DEBUG2,
			(errmsg("_rewriteSelectStmt: COUNT larg=%d, rarg=%d",analyze->larg_count, analyze->rarg_count)));
		}

		switch (node->op)
		{
			case SETOP_UNION:
				delay_string_append_char(message, str, " UNION ");
				break;

			case SETOP_INTERSECT:
				delay_string_append_char(message, str, " INTERSECT ");
				break;

			case SETOP_EXCEPT:
				delay_string_append_char(message, str, " EXCEPT ");

			default:
				break;
		}

		if (node->all)
			delay_string_append_char(message, str, "ALL ");

		if (node->rarg)
		{
			delay_string_append_char(message, str, "(");
			message->part = SELECT_START;
			_rewriteNode(BaseSelect, message, dblink, str, node->rarg);
			delay_string_append_char(message, str, ")");
			KeepMessages(message,count,SELECT_START);

			if(message->r_code == SELECT_ANALYZE)
				CopyFromLeftArg(message,count);
		}

		if(message->r_code == SELECT_ANALYZE)
		{
			ereport(DEBUG2,
			(errmsg("_rewriteSelectStmt: STATE larg=%c, rarg=%c",
					message->analyze[analyze->larg_count]->state,
						message->analyze[analyze->rarg_count]->state)));

		}

		if(message->r_code == SELECT_DEFAULT)
		{
			ereport(DEBUG2,
			(errmsg("_rewriteSelectStmt: DEFAULT COUNT larg=%d, rarg=%d",analyze->larg_count, analyze->rarg_count)));
		}

		if(message->r_code == SELECT_ANALYZE)
		{
			int lcount = analyze->larg_count;
			int rcount = analyze->rarg_count;
			if(message->analyze[lcount]->state == 'L' &&
				 message->analyze[rcount]->state == 'L')
			{
				int j;
				for(j = 0; j < SELECT_SORTCLAUSE; j++)
				{
					analyze->partstate[j]='L';
				}
			}
			else
			{
				int j;
				for(j = 0; j < SELECT_SORTCLAUSE; j++)
				{
					analyze->partstate[j]='S';
				}
			}
		}
	}
	else
	{
		if (node->intoClause)
		{
			IntoClause *into = node->intoClause;
			RangeVar *rel = (RangeVar *)into->rel;

			delay_string_append_char(message, str, "CREATE ");
			if (rel->relpersistence == true)
				delay_string_append_char(message, str, "TEMP ");
			delay_string_append_char(message, str, "TABLE ");
			_rewriteNode(BaseSelect, message, dblink, str, into->rel);
			KeepRewriteQueryReturnCode(message, SELECT_RELATION_ERROR);

			if (into->colNames)
			{
				delay_string_append_char(message, str, " (");
				_rewriteNode(BaseSelect, message, dblink, str, into->colNames);
				delay_string_append_char(message, str, ") ");
				KeepRewriteQueryReturnCode(message, SELECT_RELATION_ERROR);
			}

			if (into->options)
				_rewriteWithDefinition(BaseSelect, message, dblink, str, into->options);

			switch (into->onCommit)
			{
				case ONCOMMIT_DROP:
					delay_string_append_char(message, str, " ON COMMIT DROP");
					break;

				case ONCOMMIT_DELETE_ROWS:
					delay_string_append_char(message, str, " ON COMMIT DELETE ROWS");
					break;

				case ONCOMMIT_PRESERVE_ROWS:
					delay_string_append_char(message, str, " ON COMMIT PRESERVE ROWS");
					break;

				default:
					break;
			}
			delay_string_append_char(message, str, " AS");
		}

		delay_string_append_char(message, str, " SELECT ");

		/*
     * Check from-clause before Checking target-list
     */
		if(node->fromClause)
		{
			message->part = SELECT_FROMCLAUSE;
			analyze->part = SELECT_FROMCLAUSE;
			message->fromClause = true;

			if(message->r_code == SELECT_ANALYZE)
			{
				char buf[16];
				char *temp = "pool_t$";
				analyze->virtual = (VirtualTable *) palloc(sizeof(VirtualTable));
				snprintf(buf, 16, "%d", message->virtual_num);
				message->virtual_num++;
				analyze->table_name = (char *) palloc(sizeof(char) * (strlen(temp) + strlen(buf) + 1));
 				strcpy(analyze->table_name,temp);
 				strcat(analyze->table_name,buf);
				analyze->virtual->col_num = 0;
			}

			if(message->r_code == SELECT_DEFAULT && message->ignore_rewrite == -1)
				message->ignore_rewrite = count;

      /* remember analyze_num */
			from_analyze = message->analyze_num;

			_rewriteNode(BaseSelect, message, dblink, str, node->fromClause);

			if(message->r_code == SELECT_DEFAULT && message->ignore_rewrite == count)
				message->ignore_rewrite = -1;

			message->fromClause = false;

			if(message->r_code == SELECT_ANALYZE)
			{
				message->table_state = analyze->state;
				analyze->partstate[SELECT_FROMCLAUSE] = analyze->state;
			}
		}
		else
		{
			/* this is const or function call*/
			message->part = SELECT_TARGETLIST;
			if(message->r_code == SELECT_ANALYZE)
				build_range_info(message,NULL,NULL,NULL,NULL,message->current_select,-1);
		}

		message->part = SELECT_OTHER;
		analyze->part = SELECT_OTHER;

		if (message->r_code == SELECT_ANALYZE && node->groupClause)
		{
			analyze->aggregate = true;
		}

		if (node->distinctClause)
		{
			if(message->r_code == SELECT_ANALYZE)
				analyze->partstate[SELECT_TARGETLIST] = 'S';
#if 0
			/* TODO
       * DISTINCT optimization
       */
			if(message->r_code == SELECT_ANALYZE)
				analyze->aggregate = true;
#endif
			delay_string_append_char(message, str, "DISTINCT ");
			if (lfirst(list_head(node->distinctClause)) != NIL)
			{
				delay_string_append_char(message, str, "ON (");
				_rewriteNode(BaseSelect, message, dblink,str, node->distinctClause);
				KeepMessages(message,count,SELECT_OTHER);
				delay_string_append_char(message, str, ") ");
			}
		}

		message->part = SELECT_TARGETLIST;
		analyze->part = SELECT_TARGETLIST;

		if(analyze->partstate[SELECT_FROMCLAUSE] == 'P' && analyze->aggregate)
			analyze->partstate[SELECT_TARGETLIST] = 'S';

		/* TARGETLIST START */
		_rewriteNode(BaseSelect, message, dblink, str, node->targetList);

		aggrewrite = CheckAggOpt(message);

		target_analyze = message->analyze_num;

		if(message->r_code == SELECT_ANALYZE)
		{
			if (analyze->aggregate && (analyze->partstate[SELECT_FROMCLAUSE] == 'P'
					|| analyze->partstate[SELECT_TARGETLIST] == 'P'))
					analyze->partstate[SELECT_TARGETLIST] = 'S';
			else if (!analyze->partstate[SELECT_TARGETLIST])
				analyze->partstate[SELECT_TARGETLIST] = analyze->partstate[SELECT_FROMCLAUSE];
		}

		KeepMessages(message,count,SELECT_TARGETLIST);

		if (node->fromClause && message->r_code != SELECT_ANALYZE)
		{

			message->analyze_num = from_analyze;

			message->part = SELECT_FROMCLAUSE;
			analyze->part = SELECT_FROMCLAUSE;
			delay_string_append_char(message, str, " FROM ");

			if(message->r_code == SELECT_DEFAULT &&
					(analyze->state == 'S' || lock)
					&& message->rewritelock == -1 && message->ignore_rewrite ==-1)
			{
				if(analyze->partstate[SELECT_FROMCLAUSE] == 'L')
				{
					writeSelectHeader(message,dblink,str,LOADBALANCE, message->part);
					message->rewritelock = count;
				}
				else if(analyze->partstate[SELECT_FROMCLAUSE] == 'P')
				{
					if(aggrewrite)
					{
						writeSelectAggHeader(message,dblink,str, message->part);
					}
					else
					{
						writeSelectHeader(message,dblink,str,PARALLEL, message->part);
					}
					message->rewritelock = count;
				}
			}

			message->fromClause = true;
			_rewriteNode(BaseSelect, message, dblink, str, node->fromClause);
			message->fromClause = false;
			KeepMessages(message,count,SELECT_FROMCLAUSE);

			if(message->r_code == SELECT_DEFAULT && analyze->state == 'S'
					&& message->rewritelock == count && message->ignore_rewrite == -1)
			{
				if(analyze->partstate[SELECT_FROMCLAUSE] == 'L')
				{
					if(analyze->partstate[SELECT_WHERECLAUSE] != 'L')
					{
						if(node->whereClause)
						{
							int message_code = message->r_code;
							delay_string_append_char(message, str, " WHERE ");
							CheckWhereCaluse(node->whereClause, message,dblink,str,0);
							KeepRewriteQueryReturnCode(message, message_code);
						}

						writeSelectFooter(message,str,analyze,SELECT_FROMCLAUSE);
						message->rewritelock = -1;
					}
				}
				else if(analyze->partstate[SELECT_FROMCLAUSE] == 'P')
				{
					if(analyze->partstate[SELECT_WHERECLAUSE] != 'P')
					{
						if(node->whereClause)
						{
							int message_code = message->r_code;
							delay_string_append_char(message, str, " WHERE ");
							CheckWhereCaluse(node->whereClause, message,dblink,str,0);
							KeepRewriteQueryReturnCode(message, message_code);
						}
						writeSelectFooter(message,str,analyze,SELECT_FROMCLAUSE);
						message->rewritelock = -1;
					}
				}
			}
			message->analyze_num = target_analyze;
		}

		/* WHERE CLAUSE */

		message->part = SELECT_OTHER;
		if (node->whereClause)
			BaseSelect = NULL;

		if (node->whereClause)
		{
			message->part = SELECT_WHERECLAUSE;
			analyze->part = SELECT_WHERECLAUSE;
			delay_string_append_char(message, str, " WHERE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->whereClause);
			KeepMessages(message,count,SELECT_OTHER);
		}

		if(!analyze->partstate[SELECT_WHERECLAUSE] && message->r_code == SELECT_ANALYZE)
			analyze->partstate[SELECT_WHERECLAUSE] = analyze->partstate[SELECT_FROMCLAUSE];

		if(aggrewrite)
				writeSelectAggFooter(message,str,analyze);
		else
			ChangeStateRewriteFooter(message,str,SELECT_WHERECLAUSE, SELECT_GROUPBYCLAUSE);

		/* GROUPBY CLAUSE */
		if (node->groupClause)
		{
			analyze->part = SELECT_GROUPBYCLAUSE;
			delay_string_append_char(message, str, " GROUP BY ");
			_rewriteNode(BaseSelect, message, dblink, str, node->groupClause);
			KeepMessages(message,count,SELECT_OTHER);
		}

		ChangeStateByCluase(message,node->groupClause,SELECT_WHERECLAUSE,SELECT_GROUPBYCLAUSE);

		/* HAVING CLAUSE */
		if (node->havingClause)
		{
			analyze->part = SELECT_HAVINGCLAUSE;
			delay_string_append_char(message, str, " HAVING ");
			_rewriteNode(BaseSelect, message, dblink, str, node->havingClause);
			KeepMessages(message,count,SELECT_OTHER);

		}

		if(message->r_code == SELECT_ANALYZE)
			analyze->partstate[SELECT_HAVINGCLAUSE] = analyze->partstate[SELECT_GROUPBYCLAUSE];
	}

	ChangeStateRewriteFooter(message,str,SELECT_HAVINGCLAUSE, SELECT_SORTCLAUSE);

	if (node->sortClause)
	{
		analyze->part = SELECT_SORTCLAUSE;
		delay_string_append_char(message, str, " ORDER BY ");
		_rewriteNode(BaseSelect, message, dblink, str, node->sortClause);
		KeepMessages(message,count,SELECT_OTHER);
	}

	ChangeStateByCluase(message,node->sortClause,SELECT_HAVINGCLAUSE,SELECT_SORTCLAUSE);


	ChangeStateRewriteFooter(message,str, SELECT_SORTCLAUSE, SELECT_OFFSETCLAUSE);

	if (node->limitOffset)
	{
		analyze->part = SELECT_OFFSETCLAUSE;
		delay_string_append_char(message, str, " OFFSET ");
		_rewriteNode(BaseSelect, message, dblink, str, node->limitOffset);
		KeepMessages(message,count,SELECT_OTHER);

	}

	ChangeStateByCluase(message,node->limitOffset,SELECT_SORTCLAUSE,SELECT_OFFSETCLAUSE);

	ChangeStateRewriteFooter(message,str,SELECT_OFFSETCLAUSE,SELECT_LIMITCLAUSE);

	if (node->limitCount)
	{
		analyze->part = SELECT_LIMITCLAUSE;
		delay_string_append_char(message, str, " LIMIT ");
		if (IsA(node->limitCount, A_Const) &&
			((A_Const *)node->limitCount)->val.type == T_Null)
		{
			delay_string_append_char(message, str, "ALL ");
		}
		else
		{
			_rewriteNode(BaseSelect, message, dblink, str, node->limitCount);
			KeepMessages(message,count,SELECT_OTHER);
		}
	}

	ChangeStateByCluase(message,node->limitCount,SELECT_OFFSETCLAUSE,SELECT_LIMITCLAUSE);

	if(message->r_code == SELECT_ANALYZE)
	{
		int i;
		analyze->state = analyze->partstate[SELECT_LIMITCLAUSE];

		for(i = 0; i< 8; i++)
		{
			char s = analyze->partstate[i];
			if(s == 'S')
			{
				analyze->state = s;
				break;
			}
		}
	}

	_rewriteNode(BaseSelect, message, dblink, str, node->lockingClause);
	KeepMessages(message,count,SELECT_OTHER);

	if(message->r_code == SELECT_ANALYZE)
	{
		if(node->targetList)
			AnalyzeReturnRecord(BaseSelect,message,dblink,str,node->targetList);

		ereport(DEBUG2,
			(errmsg("_rewriteSelectStmt select_no=%d state=%s",message->current_select,analyze->partstate)));

		if(strstr(analyze->partstate,"E"))
			message->is_loadbalance = true;

  	/* change state */
		if(count != 0)
		{
			AnalyzeSelect *last = message->analyze[analyze->last_select];
			if(last->part == SELECT_WHERECLAUSE)
			{
				char fromstate = last->partstate[SELECT_FROMCLAUSE];
				char wherestate = (char) 0;

				if(last->partstate[SELECT_WHERECLAUSE])
					wherestate = last->partstate[SELECT_WHERECLAUSE];

				if(fromstate == 'P' && analyze->state == 'L')
				{
					if(wherestate && wherestate == 'P')
						last->partstate[SELECT_WHERECLAUSE] = 'P';
				}
				else if(fromstate == 'L' && analyze->state == 'L')
				{
					last->partstate[SELECT_WHERECLAUSE] = 'L';
				}
				else
					last->partstate[SELECT_WHERECLAUSE] = 'S';
			}

			if(last->part == SELECT_TARGETLIST)
			{
				char fromstate = last->partstate[SELECT_FROMCLAUSE];

				if(fromstate == 'P' && analyze->state == 'L')
				{
					last->partstate[SELECT_TARGETLIST] = 'P';
				}
				else if(fromstate == 'L' && analyze->state == 'L')
				{
					last->partstate[SELECT_TARGETLIST] = 'L';
				}
				else
					last->partstate[SELECT_TARGETLIST] = 'S';
			}
		}
	}

	if(message->r_code == SELECT_DEFAULT && message->rewritelock == count
		&& message->ignore_rewrite ==-1 )
	{
		if(direct)
			writeSelectFooter(message,str,analyze,analyze->call_part);
		else
			writeSelectFooter(message,str,analyze,SELECT_FROMCLAUSE);

		message->rewritelock = -1;
	}
}

static void
initAggexpr(AnalyzeSelect *analyze)
{
	Aggexpr *agg;

	if(analyze->aggexpr)
		return;

	analyze->aggexpr = (Aggexpr *) palloc(sizeof(Aggexpr));
	agg = analyze->aggexpr;
	agg->usec_p = NULL;
	agg->tfunc_p = NULL;
	agg->col_p = NULL;
	agg->hfunc_p = NULL;
	agg->umapc = NULL;
	agg->u_num = 0;
	agg->t_num = 0;
	agg->c_num = 0;
	agg->h_num = 0;
	agg->hc_num = 0;
	agg->s_num = 0;
	agg->sc_num = 0;
	agg->opt = true;
}

static int
FuncChangebyAggregate(AnalyzeSelect *analyze, FuncCall *fnode)
{
	Aggexpr *agg;
	int i;


	if(analyze->aggregate && analyze->aggexpr)
		agg = analyze->aggexpr;
	else
		return -1;

	if(!agg->opt)
		return -1;

	if(analyze->part == SELECT_TARGETLIST)
	{
		for(i = 0; i < agg->t_num; i++)
		{
			if(fnode == agg->tfunc_p[i])
			{
				return i;
			}
		}
	}
	else if(analyze->part == SELECT_HAVINGCLAUSE)
	{
		for(i = 0; i < agg->h_num; i++)
		{
			if(fnode == agg->hfunc_p[i])
			{
				return i;
			}
		}
	}
	else if(analyze->part == SELECT_SORTCLAUSE)
	{
		String *sortfunc;
		sortfunc = init_string("");
		_outNode(sortfunc, fnode);

		for(i = 0; i < agg->t_num; i++)
		{
			String *having;
			having = init_string("");
			_outNode(having, agg->tfunc_p[i]);
			if(!strcmp(sortfunc->data,having->data))
			{
				free_string(having);
				free_string(sortfunc);
				return i;
			} else {
				free_string(having);
			}
		}
		free_string(sortfunc);
	}
	return -1;
}

static int
ColumnChangebyAggregate(AnalyzeSelect *analyze,ColumnRef *cnode)
{
	Aggexpr *agg;
	int i;


	if(analyze->aggregate && analyze->aggexpr)
		agg = analyze->aggexpr;
	else
		return -1;

	if(!agg->opt)
		return -1;

	if(analyze->part == SELECT_GROUPBYCLAUSE)
	{
		for(i = 0; i < agg->c_num; i++)
		{
			if(cnode == agg->col_p[i])
			{
				return agg->t_num + i;
			}
		}
	}
	else if(analyze->part == SELECT_TARGETLIST)
	{
		for(i = 0; i < agg->u_num; i++)
		{
			char *n_c = NULL;
			char *n_t = NULL;
			char *c_c = NULL;
			char *c_t = NULL;
			n_c = GetNameFromColumnRef(cnode,true);
			n_t = GetNameFromColumnRef(cnode,false);
			c_c = GetNameFromColumnRef(agg->usec_p[i],true);
			c_t = GetNameFromColumnRef(agg->usec_p[i],false);

			if(n_t && c_t)
			{
				if(!strcmp(n_t,c_t) && !strcmp(n_c,c_c))
					return agg->t_num + agg->umapc[i];
			}
			else
			{
				if(!strcmp(n_c,c_c))
					return agg->t_num + agg->umapc[i];
			}
		}
	}
	else if(analyze->part == SELECT_HAVINGCLAUSE || analyze->part == SELECT_SORTCLAUSE)
	{
		for(i = 0; i < agg->c_num; i++)
		{
			char *n_c = NULL;
			char *n_t = NULL;
			char *c_c = NULL;
			char *c_t = NULL;
			n_c = GetNameFromColumnRef(cnode,true);
			n_t = GetNameFromColumnRef(cnode,false);
			c_c = GetNameFromColumnRef(agg->col_p[i],true);
			c_t = GetNameFromColumnRef(agg->col_p[i],false);

			if(n_t && c_t)
			{
				if(!strcmp(n_t,c_t) && !strcmp(n_c,c_c))
					return agg->t_num + i;
			}
			else
			{
				if(!strcmp(n_c,c_c))
					return agg->t_num + i;
			}
		}
	}
	return -1;
}

static void
AppendAggregate(AnalyzeSelect *analyze,FuncCall *fnode,ColumnRef *cnode)
{
	Aggexpr *agg;

	initAggexpr(analyze);

	agg = analyze->aggexpr;


	if(analyze->part == SELECT_GROUPBYCLAUSE)
	{
		if(!agg->col_p)
		{
			agg->col_p = (ColumnRef **) palloc(sizeof(ColumnRef *));
		}
		else
		{
			agg->col_p = (ColumnRef **) repalloc(agg->col_p,(agg->c_num + 1) *sizeof(ColumnRef *));
		}
		agg->col_p[agg->c_num] = cnode;
		SeekColumnName(agg,cnode,SELECT_GROUPBYCLAUSE);
		agg->c_num++;
	}
	else if(analyze->part == SELECT_TARGETLIST)
	{
		if(fnode)  /* for function */
		{
			if(!agg->tfunc_p)
			{
				agg->tfunc_p = (FuncCall **) palloc(sizeof(FuncCall *));
			}
			else
			{
				agg->tfunc_p = (FuncCall **) repalloc(agg->tfunc_p,(agg->t_num + 1) *sizeof(FuncCall *));
			}
			agg->tfunc_p[agg->t_num] = fnode;
			agg->t_num++;
		} else {   /* for Column */
			if(!agg->usec_p)
			{
				agg->usec_p = (ColumnRef **) palloc(sizeof(ColumnRef *));
				agg->umapc = (int *) palloc(sizeof(int));
			}
			else
			{
				agg->usec_p = (ColumnRef **) repalloc(agg->usec_p,(agg->u_num + 1) *sizeof(ColumnRef *));
				agg->umapc = (int *) repalloc(agg->umapc,(agg->u_num + 1) *sizeof(int));
			}
			agg->usec_p[agg->u_num] = cnode;
			agg->u_num++;
		}
	}
	else if(analyze->part == SELECT_HAVINGCLAUSE)
	{
		if(fnode)  /* for function */
		{
			if(!agg->hfunc_p)
			{
				agg->hfunc_p = (FuncCall **) palloc(sizeof(FuncCall *));
			}
			else
			{
				agg->hfunc_p = (FuncCall **) repalloc(agg->hfunc_p,(agg->h_num + 1) *sizeof(FuncCall *));
			}
			agg->hfunc_p[agg->h_num] = fnode;
			agg->h_num++;
		}
	}
}

static void
_rewriteFuncCall(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncCall *node)
{
	char *funcname;
	bool avg_flag = false;
	if(message->r_code == SELECT_AEXPR)
	{
		KeepRewriteQueryReturnCode(message, SELECT_AEXPR_FALSE);
		return;
	}

	funcname = strVal(lfirst(list_head(node->funcname)));

	if (CheckAggOpt(message))
	{
		int i;
		AnalyzeSelect *analyze;
		int no = message->current_select;
		analyze = message->analyze[no];

		i = FuncChangebyAggregate(analyze,node);
		if(i != -1)
		{
			if(!strcmp(funcname,"count"))
				delay_string_append_char(message, str, "sum");
			else if(!strcmp(funcname,"avg"))
			{
				delay_string_append_char(message, str, "(sum");
				avg_flag =true;
			}
			else
				_rewriteFuncName(BaseSelect, message, dblink, str, node->funcname);
		}
		else
		_rewriteFuncName(BaseSelect, message, dblink, str, node->funcname);

	}
	else
		_rewriteFuncName(BaseSelect, message, dblink, str, node->funcname);

	if(message->r_code == SELECT_ANALYZE && funcname)
	{
		/* aggregate functions */
		if(!strcmp(funcname,"count") || !strcmp(funcname,"max") || !strcmp(funcname,"min")
			|| !strcmp(funcname,"sum") || !strcmp(funcname,"avg") || !strcmp(funcname,"bit_and")
			|| !strcmp(funcname,"bit_or") || !strcmp(funcname,"bool_and") || !strcmp(funcname,"bool_or")
			|| !strcmp(funcname,"every") || !strcmp(funcname,"corr") || !strcmp(funcname,"covar_pop")
			|| !strcmp(funcname,"covar_samp") || !strcmp(funcname,"regr_avgx") || !strcmp(funcname,"regr_avgy")
			|| !strcmp(funcname,"regr_count") || !strcmp(funcname,"regr_intercept") || !strcmp(funcname,"regr_r2")
			|| !strcmp(funcname,"regr_slope") || !strcmp(funcname,"regr_sxx") || !strcmp(funcname,"regr_sxy")
			|| !strcmp(funcname,"regr_syy") || !strcmp(funcname,"stddev") || !strcmp(funcname,"stddev_pop")
			|| !strcmp(funcname,"stddev_samp") || !strcmp(funcname,"variance") || !strcmp(funcname,"var_pop")
			|| !strcmp(funcname,"var_samp"))
		{
			AnalyzeSelect *analyze;
			int no = message->current_select;
			analyze = message->analyze[no];
			analyze->aggregate = true;

			if(analyze->part == SELECT_TARGETLIST || analyze->part == SELECT_HAVINGCLAUSE)
			{
				if(!strcmp(funcname,"count") || !strcmp(funcname,"max") ||
						!strcmp(funcname,"min") || !strcmp(funcname,"sum") || !strcmp(funcname,"avg"))
				{
					AppendAggregate(analyze,node,NULL);
				}
				else
				{
					initAggexpr(analyze);
					analyze->aggexpr->opt = false;
				}
			}
		}
	}

	if(funcname &&
	   (strcmp(funcname,"user") == 0 ||
	   strcmp(funcname,"current_user") == 0 ||
	   strcmp(funcname,"session_user") == 0 ||
	   strcmp(funcname,"current_role") == 0))
		return ;

	delay_string_append_char(message, str, "(");

	if (node->func_variadic == TRUE)
		delay_string_append_char(message, str, "VARIADIC ");

	if (node->agg_distinct == TRUE)
		delay_string_append_char(message, str, "DISTINCT ");

	if (CheckAggOpt(message))
	{
		int i;
		AnalyzeSelect *analyze;
		int no = message->current_select;
		analyze = message->analyze[no];
		i = FuncChangebyAggregate(analyze,node);
		if(i != -1)
		{
		 	char buf[16];
		 	snprintf(buf, 16, "%d", i);
		 	delay_string_append_char(message, str, "pool_g$");
		 	delay_string_append_char(message, str, buf);
		 	ereport(DEBUG2,
			(errmsg("_FuncCall: aggregate no = %d",i)));
			delay_string_append_char(message, str, ")");

			if(avg_flag)
			{
				delay_string_append_char(message, str, "/");
				delay_string_append_char(message, str, "sum(");
		 		delay_string_append_char(message, str, "pool_g$");
		 		delay_string_append_char(message, str, buf);
		 		delay_string_append_char(message, str, "c");
				delay_string_append_char(message, str, "))");
			}
		 	return ;
		}
	}

	if (node->agg_star == TRUE)
		delay_string_append_char(message, str, "*");
	else
		_rewriteNode(BaseSelect, message, dblink, str, node->args);

	if (node->agg_order != NIL)
	{
		delay_string_append_char(message, str, " ORDER BY ");
		_rewriteNode(BaseSelect, message, dblink, str, node->agg_order);
	}

	delay_string_append_char(message, str, ")");
}

static void
AvgFuncCall(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncCall *node)
{
	if(message->r_code == SELECT_AEXPR)
	{
		KeepRewriteQueryReturnCode(message, SELECT_AEXPR_FALSE);
		return;
	}

	delay_string_append_char(message, str, "sum");

	delay_string_append_char(message, str, "(");

	if (node->agg_distinct == TRUE)
		delay_string_append_char(message, str, "DISTINCT ");

	_rewriteNode(BaseSelect, message, dblink, str, node->args);

	delay_string_append_char(message, str, ")");
	delay_string_append_char(message, str, ",");
	delay_string_append_char(message, str, "count");

	delay_string_append_char(message, str, "(");

	if (node->agg_distinct == TRUE)
		delay_string_append_char(message, str, "DISTINCT ");

	_rewriteNode(BaseSelect, message, dblink, str, node->args);

	delay_string_append_char(message, str, ")");
}

static void
_rewriteDefElem(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DefElem *node)
{

}

static void
_rewriteLockingClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, LockingClause *node)
{
	if (node == NULL)
		return;
	switch(node->strength)
	{
		case LCS_FORKEYSHARE:
			delay_string_append_char(message, str, " FOR KEY SHARE");
			break;
		case LCS_FORSHARE:
			delay_string_append_char(message, str, " FOR SHARE");
			break;
		case LCS_FORNOKEYUPDATE:
			delay_string_append_char(message, str, " FOR NO KEY UPDATE");
			break;
		case LCS_FORUPDATE:
			delay_string_append_char(message, str, " FOR UPDATE");
			break;
	}

	_rewriteNode(BaseSelect, message, dblink, str, node->lockedRels);

	if (node->noWait == TRUE)
		delay_string_append_char(message, str, " NOWAIT ");
}

static void
_rewriteColumnDef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ColumnDef *node)
{
	delay_string_append_char(message, str, "\"");
	delay_string_append_char(message, str, node->colname);
	delay_string_append_char(message, str, "\" ");
	_rewriteNode(BaseSelect, message, dblink, str, node->typeName);
	_rewriteNode(BaseSelect, message, dblink, str, node->constraints);
}

static void
_rewriteTypeName(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TypeName *node)
{
	if (list_length(node->names) == 2 &&
		strcmp("pg_catalog", strVal(linitial(node->names))) == 0)
   	{
		delay_string_append_char(message, str, strVal(lsecond(node->names)));

		if (strcmp("interval", strVal(lsecond(node->names))) == 0)
		{
			if (node->typmods != NIL)
			{
				A_Const		*v = (A_Const *) linitial(node->typmods);
				int			mask = v->val.val.ival;

				/* precision for SECOND field.
				 * backword comaptibility.
				 * use `'1.2 second'::interval(0) second'
				 * not `'1.2 second'::interval second(0)'(standarad for 8.4).
				 */
				if ((INTERVAL_MASK(SECOND) & mask) &&
					list_length(node->typmods) == 2)
				{
					delay_string_append_char(message, str, "(");
					_rewriteAConst(BaseSelect, message, dblink, str,
						   	lsecond(node->typmods));
					delay_string_append_char(message, str, ")");
				}

				/* optional fields */
				if (mask == INTERVAL_MASK(YEAR))
					delay_string_append_char(message, str, " YEAR");
				else if (mask == INTERVAL_MASK(MONTH))
					delay_string_append_char(message, str, " MONTH");
				else if (mask == INTERVAL_MASK(DAY))
					delay_string_append_char(message, str, " DAY");
				else if (mask == INTERVAL_MASK(HOUR))
					delay_string_append_char(message, str, " HOUR");
				else if (mask == INTERVAL_MASK(MINUTE))
					delay_string_append_char(message, str, " MINUTE");
				else if (mask == INTERVAL_MASK(SECOND))
					delay_string_append_char(message, str, " SECOND");
				else if (mask == (INTERVAL_MASK(YEAR) | INTERVAL_MASK(MONTH)))
					delay_string_append_char(message, str, " YEAR TO MONTH");
				else if (mask == (INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR)))
					delay_string_append_char(message, str, " DAY TO HOUR");
				else if (mask == (INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR) |
								  INTERVAL_MASK(MINUTE)))
					delay_string_append_char(message, str, " DAY TO MINUTE");
				else if (mask == (INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR) |
								  INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND)))
					delay_string_append_char(message, str, " DAY TO SECOND");
				else if (mask == (INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE)))
					delay_string_append_char(message, str, " HOUR TO MINUTE");
				else if (mask == (INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE) |
								  INTERVAL_MASK(SECOND)))
					delay_string_append_char(message, str, " HOUR TO SECOND");
				else if (mask == (INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND)))
					delay_string_append_char(message, str, " MINUTE TO SECOND");
			}

			return;
		}
	}
	else
	{
		ListCell *lc;
		char dot = 0;

		foreach (lc, node->names)
		{
			Value *v = (Value *) lfirst(lc);
			char *typename = v->val.str;

			if (dot == 0)
				dot = 1;
			else
				delay_string_append_char(message, str, ".");

			if(node->typemod < 0)
			{
				if(message->rewritelock != -1)
				{
					delay_string_append_char(message, str, "\"\"");
					delay_string_append_char(message, str, typename);
					delay_string_append_char(message, str, "\"\"");
				}
				else
				{
					delay_string_append_char(message, str, typename);
				}
			}
			else
			{
				delay_string_append_char(message, str, typename);
			}
		}
	}

	/* precisions */
	if (node->typmods)
	{
		delay_string_append_char(message, str, "(");
		_rewriteList(BaseSelect, message, dblink, str, node->typmods);
		delay_string_append_char(message, str, ")");
	}
}

static void
_rewriteTypeCast(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TypeCast *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->arg);
	delay_string_append_char(message, str, "::");
	_rewriteNode(BaseSelect, message, dblink, str, node->typeName);

}

static void
_rewriteIndexElem(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, IndexElem *node)
{
	if (node->name)
	{
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->name);
		delay_string_append_char(message, str, "\"");
		if (node->opclass != NIL)
			_rewriteNode(BaseSelect, message, dblink, str, node->opclass);
	}
	else
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->expr);
		delay_string_append_char(message, str, ")");
		if (node->opclass != NIL)
			_rewriteNode(BaseSelect, message, dblink, str, node->opclass);
	}
}


static void
_rewriteSortGroupClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SortGroupClause *node)
{

}

static void
_rewriteWindowClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, WindowClause *node)
{

}

static void
_rewriteSetOperationStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SetOperationStmt *node)
{

}


static void
_rewriteAExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Expr *node)
{
	Value *v;

	switch (node->kind)
	{
		case AEXPR_OP:
			if (list_length(node->name) == 1)
			{
				Value *op = (Value *) lfirst(list_head(node->name));

				delay_string_append_char(message, str, " (");
				_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
				delay_string_append_char(message, str, op->val.str);
				_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
				delay_string_append_char(message, str, " )");
			}
			break;

		case AEXPR_AND:
			delay_string_append_char(message, str, " (");
			_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
			delay_string_append_char(message, str, " AND ");
			_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_OR:
			delay_string_append_char(message, str, " (");
			_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
			delay_string_append_char(message, str, " OR ");
			_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_NOT:
			delay_string_append_char(message, str, " (NOT ");
			_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_OP_ANY:
			_rewriteNode(BaseSelect,message,dblink,str, node->lexpr);
			v = linitial(node->name);
			delay_string_append_char(message,str, v->val.str);
			delay_string_append_char(message,str, "ANY(");
			_rewriteNode(BaseSelect,message,dblink,str, node->rexpr);
			delay_string_append_char(message,str, ")");
			break;

		case AEXPR_OP_ALL:
			_rewriteNode(BaseSelect,message,dblink,str, node->lexpr);
			v = linitial(node->name);
			delay_string_append_char(message,str, v->val.str);
			delay_string_append_char(message,str, "ALL(");
			_rewriteNode(BaseSelect,message,dblink,str, node->rexpr);
			delay_string_append_char(message,str, ")");
			break;

		case AEXPR_DISTINCT:
			delay_string_append_char(message, str, " (");
			_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
			delay_string_append_char(message, str, " IS DISTINCT FROM ");
			_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_NULLIF:
			delay_string_append_char(message, str, " NULLIF(");
			_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
			delay_string_append_char(message, str, ", ");
			_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_OF:
			_rewriteNode(BaseSelect, message, dblink, str, node->lexpr);
			v = linitial(node->name);
			if (v->val.str[0] == '!')
				delay_string_append_char(message, str, " IS NOT OF (");
			else
				delay_string_append_char(message, str, " IS OF (");
			_rewriteNode(BaseSelect, message, dblink, str, node->rexpr);
			delay_string_append_char(message, str, ")");
			break;
		case AEXPR_IN:
			_rewriteNode(BaseSelect,message,dblink,str, node->lexpr);
			v = (Value *)lfirst(list_head(node->name));
			if (v->val.str[0] == '=')
				delay_string_append_char(message,str, " IN (");
			else
				delay_string_append_char(message,str, " NOT IN (");
			_rewriteNode(BaseSelect,message,dblink,str, node->rexpr);
			delay_string_append_char(message,str, ")");
			break;
		default:
			break;
	}
}

static void
_rewriteValue(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Value *value)
{
	char buf[16];

	switch (value->type)
	{
		case T_Integer:
			sprintf(buf, "%ld", value->val.ival);
			delay_string_append_char(message, str, buf);
			break;

		case T_Float:
			delay_string_append_char(message, str, value->val.str);
			break;

		case T_String:
			/* delay_string_append_char(message, str, "'"); */
			delay_string_append_char(message, str, escape_string(value->val.str));
			/* delay_string_append_char(message, str, "'"); */
			break;

		case T_Null:
			delay_string_append_char(message, str, "NULL");
			break;

		default:
			break;
	}
}

static char
GetInnerRef(RewriteQuery *message,int last,char *table_name,char *column,char state)
{
	AnalyzeSelect *analyze;
	VirtualTable *virtual;
	int num, i;
	bool get = false;

	analyze = message->analyze[last];
	virtual = analyze->virtual;
	num = virtual->col_num;

	for(i=0; i < num; i++)
	{
		char *vcol = virtual->col_list[i];
		char *vtable = virtual->table_list[i];
		if(table_name
			&& !strcmp(table_name,vtable) && !strcmp(column,vcol))
		{
			ereport(DEBUG2,
			(errmsg("GetInnerRef state = %c now select(%d), table_name=(%s), col_name=(%s) detect",state,last,vtable,vcol)));
			get = true;
			break;
		}
		else if(!table_name && !strcmp(column,vcol))
		{
			ereport(DEBUG2,
			(errmsg("GetInnerRef state = %c  now select(%d), table_name=(%s), col_name=(%s) detect",state,last,vtable,vcol)));
			get = true;
			break;
		}
	}

	if(get)
		return state;
	else
	{
		/* there isn't the inner refarence at parallel part */
		if(state == 'P' || analyze->partstate[SELECT_FROMCLAUSE] =='P')
			return 'S';

		if(last == 0)
			return 'E';
		else
			return GetInnerRef(message,analyze->last_select,table_name,column,state);
	}
}

static void
ChangeStatebyColumnRef(RewriteQuery *message,ColumnRef *col)
{
	AnalyzeSelect *analyze;
	ListCell *c;
	List *list;
	char first = 0;
	char *table_name = NULL;
	char *column = NULL;
	int no;

	list = col->fields;

	no = message->current_select;
	analyze = message->analyze[no];

	if(list->length > 2 || list->length == 0)
	{
		/* Is this error ? */
		message->analyze[no]->state = 'S';
		return;
	}

	foreach (c, col->fields)
	{
		Node *n = (Node *) lfirst(c);

		if (IsA(n, String))
		{
			Value *v = (Value *) lfirst(c);
			if(list->length == 2 && first == 0)
			{
				first = 1;
				table_name = v->val.str;
			}
			else
				column = v->val.str;
		}
	}

	if(!column)
	{
		message->table_state = 'S';
		return;
	}

	ereport(DEBUG2,
			(errmsg("ChangeStatebyColumnRef %s now(%d),last(%d) part(%d) call_part(%d)",
							column,
							analyze->now_select,
							analyze->last_select,
							analyze->part,
							analyze->call_part
							)));


	if(message->part==SELECT_WHERECLAUSE || message->part == SELECT_TARGETLIST)
	{
		if(analyze->partstate[message->part] != 'S')
		{
			analyze->partstate[message->part] =
				GetInnerRef(message,no,table_name,column,analyze->partstate[SELECT_FROMCLAUSE]);
			ereport(DEBUG2,
			(errmsg("return state is %c",analyze->partstate[message->part])));
		}
	}
}

static bool DetectValidColumn(RewriteQuery *message,char *table_name,char *column_name,int no, int call)
{
	AnalyzeSelect *analyze;
	VirtualTable *virtual;
	int v_num,i;
	int get = 0;
	int call_num;
	bool star = false;

	call_num = message->current_select;

	if(call != -1 && no != call_num  && call == SELECT_FROMCLAUSE)
	{
		int last = message->analyze[no]->last_select;
		int call_part = message->analyze[no]->call_part;

		if(last == -1)
		{
			return false;
		}
		else
		{
			return DetectValidColumn(message,table_name,column_name,last,call_part);
		}
	}

	analyze = message->analyze[no];
	virtual = analyze->virtual;
	v_num = virtual->col_num;

	for(i = 0; i< v_num; i++)
	{
		char *vcol = virtual->col_list[i];
		char *vtable = virtual->table_list[i];

		if(table_name && !strcmp(table_name,vtable) && !strcmp(column_name,"*"))
		{
			virtual->valid[i] = message->current_select;
			star = true;
		}
		else if (!table_name && !strcmp(column_name,"*"))
		{
			virtual->valid[i] = message->current_select;
			star = true;
		}

		if(table_name
			&& !strcmp(table_name,vtable) && !strcmp(column_name,vcol))
		{
			ereport(DEBUG2,
			(errmsg("DetectValidColumn no = %d, table_name=(%s), col_name=(%s) detect",no, vtable, vcol)));
			get++;

			if(virtual->valid[i] == -1)
			{
				virtual->valid[i] = message->current_select;
			}
			else if (virtual->valid[i] > no)
			{
				virtual->valid[i] = message->current_select;
			}
		}
		else if(!table_name && !strcmp(column_name,vcol))
		{
			ereport(DEBUG2,
			(errmsg("DetectValidColumn no = %d, col_name=(%s) detect",no, vcol)));
			get++;

			if(virtual->valid[i] == -1)
			{
				virtual->valid[i] = message->current_select;
			}
			else if (virtual->valid[i] > no)
			{
				virtual->valid[i] = message->current_select;
			}
		}
	}

	if(star)
		return true;

	if(get == 1)
	{
		return true;
	}
	else if(get == 0)
	{
		int last = analyze->last_select;
		if(last != -1)
		{
			return DetectValidColumn(message,table_name,column_name,analyze->last_select,analyze->call_part);
		}
	}
	else if(get>= 2)
		ereport(DEBUG2,
			(errmsg("DetectValidColumn select_no=(%d) col_name=(%s) ambiguous",message->current_select,column_name)));

	return false;
}

static bool GetPoolColumn(RewriteQuery *message,String *str,char *table_name,char *column_name,int no, int call,bool state)
{
	AnalyzeSelect *analyze;

#if 0
	AnalyzeSelect *analyze_now;
#endif

	VirtualTable *virtual;
	int v_num,i;
	int get = 0;
	int call_num;
	bool star = false;

	call_num = message->current_select;

	if(call != -1 && no != call_num  && call == SELECT_FROMCLAUSE)
	{
		int last = message->analyze[no]->last_select;
		int call_part = message->analyze[no]->call_part;

		if(last == -1)
		{
			return false;
		}
		else
		{
			return GetPoolColumn(message,str,table_name,column_name,last,call_part,state);
		}
	}

	analyze = message->analyze[no];

#if 0
	analyze_now = message->analyze[call_num];
#endif

	virtual = analyze->virtual;
	v_num = virtual->col_num;

	for(i = 0; i< v_num; i++)
	{
		char *vcol = virtual->col_list[i];
		char *vtable = virtual->table_list[i];

		if(table_name && !strcmp(table_name,vtable) && !strcmp(column_name,"*"))
		{
			star = true;
			break;
		}
		else if (!table_name && !strcmp(column_name,"*"))
		{
			star = true;
			break;
		}

		if(table_name
			&& !strcmp(table_name,vtable) && !strcmp(column_name,vcol))
		{
			ereport(DEBUG2,
			(errmsg("GetPoolColumn no = %d, table_name=(%s), col_name=(%s) new_colname$%d detect",no, vtable, vcol,i)));
			get++;
			break;
		}
		else if(!table_name && !strcmp(column_name,vcol))
		{
			ereport(DEBUG2,
			(errmsg("GetPoolColumn no = %d, col_name=(%s) new_colname=pool_c$%d detect",no, vcol,i)));
			get++;
			break;
		}
	}

	if(star)
	{
		if(table_name)
		{
			int first = 0;
			for(i = 0; i < v_num; i++)
			{
				char buf[16];
				if(!strcmp(virtual->table_list[i],table_name) && virtual->valid[i] != -1)
				{
					if(first == 0)
						first = 1;
					else
					delay_string_append_char(message, str, ",");

          if(message->rewritelock == -1)
					{
						snprintf(buf, 16, "%d", analyze->virtual->column_no[i]);
						delay_string_append_char(message, str,"\"pool_c$");
						delay_string_append_char(message, str,buf);
						delay_string_append_char(message, str, "\"");

						if(message->ignore_rewrite == -1 && call_num != 0 && analyze->call_part == SELECT_FROMCLAUSE)
						{
							char buf2[16];
							int col_no = analyze->select_ret->return_list[analyze->ret_count];
							delay_string_append_char(message, str, " AS ");
							snprintf(buf2, 16, "%d", col_no);
							delay_string_append_char(message, str,"\"pool_c$");
							delay_string_append_char(message, str,buf2);
							delay_string_append_char(message, str, "\"");
							delay_string_append_char(message, str," ");
							ereport(DEBUG2,
			(errmsg("GetPoolColumn analyze[%d] targetlist=* =>  pool_c$%s AS pool_c$%s",
								no,buf,buf2)));
							analyze->ret_count++;
							continue;
						}
					} else {
						delay_string_append_char(message, str,analyze->virtual->col_list[i]);
					}

					delay_string_append_char(message, str, " ");

					if(state && message->rewritelock == -1)
					{
						delay_string_append_char(message, str, " AS ");
						delay_string_append_char(message, str, "\"");
						delay_string_append_char(message, str, analyze->select_ret->col_list[message->ret_num]);
						delay_string_append_char(message, str, "\" ");
						message->ret_num++;
					}
				}
			}
		}
		else
		{
			int first = 0;
			for(i = 0; i < v_num; i++)
			{
				char buf[16];
				if(virtual->valid[i] != -1)
				{
					if(first == 0)
						first = 1;
					else
					delay_string_append_char(message, str, ",");

					if(message->rewritelock == -1)
					{
						snprintf(buf, 16, "%d", analyze->virtual->column_no[i]);
						delay_string_append_char(message, str,"\"pool_c$");
						delay_string_append_char(message, str,buf); 
						delay_string_append_char(message, str,"\"");

						if(message->ignore_rewrite == -1 && call_num != 0 && analyze->call_part == SELECT_FROMCLAUSE)
						{
							char buf2[16];
							int col_no = analyze->select_ret->return_list[analyze->ret_count];
							delay_string_append_char(message, str, " AS ");
							snprintf(buf2, 16, "%d", col_no);
							delay_string_append_char(message, str,"\"pool_c$");
							delay_string_append_char(message, str,buf2);
							delay_string_append_char(message, str,"\"");
							delay_string_append_char(message, str," ");
							analyze->ret_count++;
							ereport(DEBUG2,
			(errmsg("GetPoolColumn analyze[%d] targetlist=%s =>  pool_c$%s AS pool_c$%s",
								no,column_name,buf,buf2)));
							continue;
						}

					} else {
						delay_string_append_char(message, str,analyze->virtual->col_list[i]);
					}
					delay_string_append_char(message, str, " ");

					if(state && message->rewritelock == -1)
					{
						delay_string_append_char(message, str, " AS ");
						delay_string_append_char(message, str, analyze->select_ret->col_list[message->ret_num]);
						message->ret_num++;
					}
				}
			}
		}
		return true;
	}

	if(get == 1)
	{
		char buf[16];
		snprintf(buf, 16, "%d", virtual->column_no[i]);

#if 0
		if(analyze_now->partstate[SELECT_FROMCLAUSE] != 'S') {
			delay_string_append_char(message, str, analyze->table_name);
			delay_string_append_char(message, str, ".");
		} else {
			delay_string_append_char(message, str, virtual->table_list[i]);
			delay_string_append_char(message, str, ".");
		}
#endif
		delay_string_append_char(message, str, "\"pool_c$");
		delay_string_append_char(message, str, buf);
		delay_string_append_char(message, str, "\"");

		return true;
	}
	else if(get == 0)
	{
		int last = analyze->last_select;
		if(last != -1)
		{
			return GetPoolColumn(message,str,table_name,column_name,analyze->last_select,analyze->call_part,state);
		}
		else
			return false;
	}

	return false;
}

static void SeekColumnName(Aggexpr *agg,ColumnRef *node, int state)
{
	char *column = NULL;
	char *table = NULL;
	int i;

	/* get column & table name from group by */
	column = GetNameFromColumnRef(node,true);
	table = GetNameFromColumnRef(node,false);

	if(state == SELECT_GROUPBYCLAUSE)
	{
		for(i = 0; i < agg->u_num; i++)
		{
			ColumnRef *unode = agg->usec_p[i];
			char *t_column = NULL;
			char *t_table = NULL;
			t_column = GetNameFromColumnRef(unode,true);
			t_table = GetNameFromColumnRef(unode,false);

			if(table && t_table)
			{
				if(!strcmp(table,t_table) && !strcmp(column,t_column))
				{
					agg->umapc[i] = agg->c_num;
					return;
				}
			}
			else
			{
				if(!strcmp(column,t_column))
				{
					agg->umapc[i] = agg->c_num;
					return;
				}
			}
		}
	}
}

static bool CheckAggOpt(RewriteQuery *message)
{
	AnalyzeSelect *analyze;
	if(message->r_code != SELECT_DEFAULT)
		return false;

	analyze = message->analyze[message->current_select];

	if(analyze->aggregate && analyze->aggexpr && analyze->aggexpr->opt
			&& (analyze->part == SELECT_TARGETLIST || analyze->part == SELECT_GROUPBYCLAUSE
				|| analyze->part == SELECT_HAVINGCLAUSE || analyze->part == SELECT_SORTCLAUSE)
			&& analyze->partstate[SELECT_FROMCLAUSE] == 'P'
			&& analyze->partstate[SELECT_TARGETLIST] == 'S'
			&& analyze->partstate[SELECT_WHERECLAUSE] == 'P')
		return true;

	return false;
}

static char *GetNameFromColumnRef(ColumnRef *node,bool state)
{
	ListCell *c;
	List *list;
	list = node->fields;
	int first = 0;
	char *table_name = NULL;
  char *column_name = NULL;

	foreach (c, node->fields)
	{
		Node *n = (Node *) lfirst(c);

		if (IsA(n, String))
		{
			Value *v = (Value *) lfirst(c);
			if(list->length == 2 && first == 0)
			{
				first = 1;
				table_name = v->val.str;
			}
			else
				column_name = v->val.str;
		}
	}

	if(state)
		return column_name;
	else
		return table_name;
}

static void
_rewriteColumnRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ColumnRef *node)
{
	ListCell *c;
	List *list;
	char first = 0;
	char *table_name = NULL;
	char *column_name = "";

	if(CheckAggOpt(message))
	{
		int i;
		AnalyzeSelect *analyze = message->analyze[message->current_select];

		i = ColumnChangebyAggregate(analyze,node);
		if(i != -1)
		{
		 char buf[16];
		 snprintf(buf, 16, "%d", i);
		 delay_string_append_char(message, str, "pool_g$");
		 delay_string_append_char(message, str, buf);
		 ereport(DEBUG2,
			(errmsg("_rewriteColumnRef: aggregate no = %d",i)));
		 return ;
		}
	}

	list = node->fields;

	foreach (c, node->fields)
	{
		Node *n = (Node *) lfirst(c);

		if (IsA(n, String))
		{

			if(message->r_code == SELECT_AEXPR &&
				(_checkVirtualColumn(node, message) != 1))
			{
				KeepRewriteQueryReturnCode(message, SELECT_AEXPR_FALSE);
				return;
			}

			Value *v = (Value *) lfirst(c);
			if(list->length == 2 && first == 0)
			{
				first = 1;
				table_name = v->val.str;
			}
			else
				column_name = v->val.str;
		}
		else if (IsA(n, A_Star))
		{
			column_name = "*";
		}
	}

	if(message->r_code == SELECT_ANALYZE)
	{
		AnalyzeSelect *analyze = message->analyze[message->current_select];

		if(analyze->part == SELECT_GROUPBYCLAUSE || analyze->part == SELECT_TARGETLIST)
			AppendAggregate(analyze,NULL,node);

		if(!DetectValidColumn(message,table_name,column_name,message->current_select,-1))
		{
			message->is_loadbalance = true;
		 	ereport(DEBUG2,
			(errmsg("_rewriteColumnRef: wrong column select_no=%d",message->current_select)));
		}
		if(strcmp(column_name,"*"))
				ChangeStatebyColumnRef(message,node);
	}
	else if(message->r_code == SELECT_DEFAULT)
	{

		if(message->rewritelock == -1)
		{
			if(!GetPoolColumn(message,str,table_name,column_name,message->current_select,-1,false))
				delay_string_append_char(message, str, column_name);
		}
		else
		{
			if(table_name)
			{
				if(message->rewritelock == -1)
				{
					AnalyzeSelect *analyze = message->analyze[message->current_select];
					char s = analyze->partstate[SELECT_FROMCLAUSE];

					if(s == 'L' || s =='P')
					{
						delay_string_append_char(message, str, analyze->table_name);
					}
				}
				else
					delay_string_append_char(message, str, table_name);
				delay_string_append_char(message, str, ".");
			}
			delay_string_append_char(message, str, column_name);
		}
	}
}

static void
_rewriteParamRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ParamRef *node)
{
	char buf[16];

	snprintf(buf, 16, "%d", node->number);
	delay_string_append_char(message, str, "$");
	delay_string_append_char(message, str, buf);
}

static void
_rewriteAConst(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Const *node)
{
	char buf[16];

	switch (node->val.type)
	{
		case T_Integer:
			sprintf(buf, "%ld", node->val.val.ival);
			delay_string_append_char(message, str, buf);
			break;

		case T_Float:
			delay_string_append_char(message, str, node->val.val.str);
			break;

		case T_String:
			if(message->rewritelock != -1)
			{
				delay_string_append_char(message, str, "\'\'");
				delay_string_append_char(message, str, escape_string(node->val.val.str));
				delay_string_append_char(message, str, "\'\'");
			} else {
				delay_string_append_char(message, str, "\'");
				delay_string_append_char(message, str, escape_string(node->val.val.str));
				delay_string_append_char(message, str, "\'");
			}
			break;

		case T_Null:
			delay_string_append_char(message, str, "NULL");
			break;

		default:
			break;
	}
}

static void
_rewriteA_Indices(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Indices *node)
{
	delay_string_append_char(message, str, "[");
	if (node->lidx)
	{
		_rewriteNode(BaseSelect, message, dblink, str, node->lidx);
		delay_string_append_char(message, str, ":");
	}
	_rewriteNode(BaseSelect, message, dblink, str, node->uidx);
	delay_string_append_char(message, str, "]");
}

static void
_rewriteA_Indirection(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Indirection *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->arg);
	_rewriteNode(BaseSelect, message, dblink, str, node->indirection);

}

static void
_rewriteA_ArrayExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_ArrayExpr *node)
{
	delay_string_append_char(message, str, "ARRAY [");
	_rewriteNode(BaseSelect, message, dblink, str, node->elements);
	delay_string_append_char(message, str, "]");
}

static bool
AliasToResTargetCondition(RewriteQuery *message,String *str)
{
	int select_no = message->current_select;
	AnalyzeSelect *n_analyze;
	AnalyzeSelect *u_analyze;
	char buf[16];
	int ret;
	int col_no;

	if(select_no == 0 || message->r_code != SELECT_DEFAULT
			|| message->rewritelock != -1 || message->ignore_rewrite != -1)
		return false;

	n_analyze=message->analyze[select_no];
	u_analyze=message->analyze[select_no - 1];

	if(n_analyze->call_part == SELECT_FROMCLAUSE)
	{
		ret = n_analyze->ret_count;
		col_no = n_analyze->select_ret->return_list[n_analyze->ret_count];
		ereport(DEBUG2,
			(errmsg("AliasToResTargetCondition select no =%d,ret_no = %d,col_no =%d,colname=%s",
					select_no,ret,col_no,n_analyze->select_ret->col_list[ret])));
		delay_string_append_char(message, str, " AS ");
		snprintf(buf, 16, "%d", col_no);
		delay_string_append_char(message, str,"\"pool_c$");
		delay_string_append_char(message, str,buf);
		delay_string_append_char(message, str,"\"");
		delay_string_append_char(message, str," ");
		return true;
	}
	else if(u_analyze->select_union && u_analyze->call_part == SELECT_FROMCLAUSE)
	{
		ret = u_analyze->ret_count;
		col_no = u_analyze->select_ret->return_list[u_analyze->ret_count];
		ereport(DEBUG2,
			(errmsg("AliasToResTargetCondition(union) select now=%d up=%d,ret_no = %d,col_no =%d,colname=%s",
				   select_no,select_no-1,ret,col_no,u_analyze->select_ret->col_list[ret])));
		delay_string_append_char(message, str, " AS ");
		snprintf(buf, 16, "%d", col_no);
		delay_string_append_char(message, str,"\"pool_c$");
		delay_string_append_char(message, str,buf);
		delay_string_append_char(message, str,"\"");
		delay_string_append_char(message, str," ");
		u_analyze->ret_count++;
		return true;
	}
	else
		return false;
}

static void
_rewriteResTarget(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ResTarget *node)
{
	int select_no = message->current_select;
	AnalyzeSelect *analyze = NULL;
	SelectDefInfo *select = NULL;

	if(message->r_code == SELECT_DEFAULT)
	{
 		analyze= message->analyze[select_no];
		select = analyze->select_ret;
	}

	if (node->indirection != NIL)
	{
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->name);
		delay_string_append_char(message, str, "\"=");
		_rewriteNode(BaseSelect, message, dblink, str, node->val);
	}
	else
	{
		char *star = NULL;
		char *table_name = NULL;

		if(message->r_code == SELECT_DEFAULT && !node->name)
		{
			if (node->val && (IsA(node->val, ColumnRef)))
			{
				int first = 0;
				ListCell *c;
				ColumnRef *col;

				col = (ColumnRef *) node->val;
				foreach (c, col->fields)
				{
					Node *n = (Node *) lfirst(c);

					if (IsA(n, String))
					{
						Value *v = (Value *) lfirst(c);
						if(col->fields->length == 2 && first == 0)
						{
							first = 1;
							table_name = v->val.str;
						}
						else
							star = v->val.str;
					}
					else if (IsA(n, A_Star))
					{
						star = "*";
					}
				}
			}

			if(star && strcmp(star,"*"))
			{
				star = NULL;
			}
		}

		if(select_no == 0 && star)
		{
				GetPoolColumn(message,str,table_name,star,message->current_select,-1,true);
				return;
		}
		else
		{
			_rewriteNode(BaseSelect, message, dblink, str, node->val);
			if(star)
				return;
		}

		if(AliasToResTargetCondition(message,str))
			return;

		if (node->name)
		{
			delay_string_append_char(message, str, " AS ");
			delay_string_append_char(message, str, node->name);
			if(message->r_code == SELECT_DEFAULT && select_no == 0)
				message->ret_num++;
			return;
		}
		else if(message->r_code == SELECT_DEFAULT && select_no == 0
					&& !node->name)
		{
			char *col_name = select->col_list[message->ret_num];
			ereport(DEBUG2,
			(errmsg("_rewriteResTarget: check(%d) ret_num=%d",message->current_select,message->ret_num)));
			ereport(DEBUG2,
			(errmsg("_rewriteResTarget: col ret_num=%d col_name=%s",message->current_select,col_name)));
			delay_string_append_char(message, str, " AS ");
			delay_string_append_char(message, str, col_name);

			message->ret_num++;
		}
	}
}

static void
_rewriteConstraint(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Constraint *node)
{
	if (node->conname)
	{
		delay_string_append_char(message, str, "CONSTRAINT \"");
		delay_string_append_char(message, str, node->conname);
		delay_string_append_char(message, str, "\"");
	}

	switch (node->contype)
	{
		case CONSTR_CHECK:
			delay_string_append_char(message, str, " CHECK (");
			_rewriteNode(BaseSelect, message, dblink, str, node->raw_expr);
			delay_string_append_char(message, str, ")");
			break;

		case CONSTR_UNIQUE:
			delay_string_append_char(message, str, " UNIQUE");
			if (node->keys)
			{
				delay_string_append_char(message, str, "(");
				_rewriteIdList(BaseSelect, message, dblink, str, node->keys);
				delay_string_append_char(message, str, ")");
			}

			if (node->indexspace)
			{
				delay_string_append_char(message, str, " USING INDEX TABLESPACE \"");
				delay_string_append_char(message, str, node->indexspace);
				delay_string_append_char(message, str, "\"");
			}
			break;

		case CONSTR_PRIMARY:
			delay_string_append_char(message, str, " PRIMARY KEY");
			if (node->keys)
			{
				delay_string_append_char(message, str, "(");
				_rewriteIdList(BaseSelect, message, dblink, str, node->keys);
				delay_string_append_char(message, str, ")");
			}
			if (node->indexspace)
			{
				delay_string_append_char(message, str, " USING INDEX TABLESPACE \"");
				delay_string_append_char(message, str, node->indexspace);
				delay_string_append_char(message, str, "\"");
			}
			break;

		case CONSTR_FOREIGN:
			if (node->fk_attrs != NIL)
			{
				delay_string_append_char(message, str, " FOREIGN KEY (");
				_rewriteIdList(BaseSelect, message, dblink, str, node->fk_attrs);
				delay_string_append_char(message, str, ")" );
			}

			delay_string_append_char(message, str, " REFERENCES ");
			_rewriteNode(BaseSelect, message, dblink, str, node->pktable);

			if (node->pk_attrs != NIL)
			{
				delay_string_append_char(message, str, "(");
				_rewriteIdList(BaseSelect, message, dblink, str, node->pk_attrs);
				delay_string_append_char(message, str, ")");
			}

			switch (node->fk_matchtype)
			{
				case FKCONSTR_MATCH_FULL:
					delay_string_append_char(message, str, " MATCH FULL");
					break;

				case FKCONSTR_MATCH_PARTIAL:
					delay_string_append_char(message, str, " MATCH PARTIAL");
					break;

				default:
					break;
			}

			switch (node->fk_upd_action)
			{
				case FKCONSTR_ACTION_RESTRICT:
					delay_string_append_char(message, str, " ON UPDATE RESTRICT");
					break;

				case FKCONSTR_ACTION_CASCADE:
					delay_string_append_char(message, str, " ON UPDATE CASCADE");
					break;

				case FKCONSTR_ACTION_SETNULL:
					delay_string_append_char(message, str, " ON UPDATE SET NULL");
					break;

				case FKCONSTR_ACTION_SETDEFAULT:
					delay_string_append_char(message, str, " ON UPDATE SET DEFAULT");
					break;

				default:
					break;
			}

			switch (node->fk_del_action)
			{
				case FKCONSTR_ACTION_RESTRICT:
					delay_string_append_char(message, str, " ON DELETE RESTRICT");
					break;

				case FKCONSTR_ACTION_CASCADE:
					delay_string_append_char(message, str, " ON DELETE CASCADE");
					break;

				case FKCONSTR_ACTION_SETNULL:
					delay_string_append_char(message, str, " ON DELETE SET NULL");
					break;

				case FKCONSTR_ACTION_SETDEFAULT:
					delay_string_append_char(message, str, " ON DELETE SET DEFAULT");
					break;

				default:
					break;
			}

			if (node->deferrable)
				delay_string_append_char(message, str, " DEFERRABLE");

			if (node->initdeferred)
				delay_string_append_char(message, str, " INITIALLY DEFERRED");
			break;

		case CONSTR_NOTNULL:
			delay_string_append_char(message, str, " NOT NULL");
			break;

		case CONSTR_NULL:
			delay_string_append_char(message, str, " NULL");
			break;

		case CONSTR_DEFAULT:
			delay_string_append_char(message, str, "DEFAULT ");
			_rewriteNode(BaseSelect, message, dblink, str, node->raw_expr);
			break;

		default:
			break;
	}
}


static void
_rewriteSortBy(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SortBy *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->node);

	if (node->sortby_dir == SORTBY_USING)
	{
		delay_string_append_char(message, str, " USING ");
		_rewriteNode(BaseSelect, message, dblink, str, node->useOp);
	}
	else if (node->sortby_dir == SORTBY_DESC)
		delay_string_append_char(message, str, " DESC ");

	if (node->sortby_nulls == SORTBY_NULLS_FIRST)
		delay_string_append_char(message, str, " NULLS FIRST ");
	else if (node->sortby_nulls == SORTBY_NULLS_LAST)
		delay_string_append_char(message, str, " NULLS LAST ");
}

static void _rewriteInsertStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, InsertStmt *node)
{
	delay_string_append_char(message, str, "INSERT INTO ");
	_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	if (node->cols == NIL && node->selectStmt == NULL)
		delay_string_append_char(message, str, " DEFAULT VALUES");

	if (node->cols)
	{
		char comma = 0;
		ListCell *lc;

		delay_string_append_char(message, str, "(");

		foreach (lc, node->cols)
		{
			ResTarget *node = lfirst(lc);
			if (comma == 0)
				comma = 1;
			else
				delay_string_append_char(message, str, ", ");

			delay_string_append_char(message, str, "\"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\"");
		}
		delay_string_append_char(message, str, ")");
	}

	if (node->selectStmt)
	{
		_rewriteNode(BaseSelect, message, dblink, str, node->selectStmt);
	}

	if (node->returningList)
	{
		delay_string_append_char(message, str, " RETURNING ");
		_rewriteNode(BaseSelect, message, dblink, str, node->returningList);
	}
}

static void _rewriteUpdateStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, UpdateStmt *node)
{
	ListCell *lc;
	char comma = 0;

	delay_string_append_char(message, str, "UPDATE ");

	_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	delay_string_append_char(message, str, " SET ");
	foreach (lc, node->targetList)
	{
		ResTarget *node = lfirst(lc);
		if (comma == 0)
			comma = 1;
		else
			delay_string_append_char(message, str, ", ");

		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->name);
		delay_string_append_char(message, str, "\" =");
		_rewriteNode(BaseSelect, message, dblink, str, node->val);
	}

	if (node->fromClause)
	{
		delay_string_append_char(message, str, " FROM ");
		_rewriteNode(BaseSelect, message, dblink, str, node->fromClause);
	}

	if (node->whereClause)
	{
		delay_string_append_char(message, str, " WHERE ");
		_rewriteNode(BaseSelect, message, dblink, str, node->whereClause);
	}

	if (node->returningList)
	{
		delay_string_append_char(message, str, " RETURNING ");
		_rewriteNode(BaseSelect, message, dblink, str, node->returningList);
	}
}

static void _rewriteDeleteStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DeleteStmt *node)
{
	delay_string_append_char(message, str, "DELETE FROM ");

	_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	if (node->usingClause)
	{
		delay_string_append_char(message, str, " USING ");
		_rewriteNode(BaseSelect, message, dblink, str, node->usingClause);
	}

	if (node->whereClause)
	{
		delay_string_append_char(message, str, " WHERE ");
		_rewriteNode(BaseSelect, message, dblink, str, node->whereClause);
	}

	if (node->returningList)
	{
		delay_string_append_char(message, str, " RETURNING ");
		_rewriteNode(BaseSelect, message, dblink, str, node->returningList);
	}
}

static void _rewriteTransactionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TransactionStmt *node)
{
	switch (node->kind)
	{
		case TRANS_STMT_BEGIN:
			delay_string_append_char(message, str, "BEGIN ");
			break;

		case TRANS_STMT_START:
			delay_string_append_char(message, str, "START TRANSACTION ");
			break;

		case TRANS_STMT_COMMIT:
			delay_string_append_char(message, str, "COMMIT ");
			break;

		case TRANS_STMT_ROLLBACK:
			delay_string_append_char(message, str, "ABORT ");
			break;

		case TRANS_STMT_SAVEPOINT:
			delay_string_append_char(message, str, "SAVEPOINT ");
			break;

		case TRANS_STMT_RELEASE:
			delay_string_append_char(message, str, "RELEASE ");
			break;

		case TRANS_STMT_ROLLBACK_TO:
			delay_string_append_char(message, str, "ROLLBACK TO ");
			break;

		case TRANS_STMT_PREPARE:
			delay_string_append_char(message, str, "PREPARE TRANSACTION ");
			break;

		case TRANS_STMT_COMMIT_PREPARED:
			delay_string_append_char(message, str, "COMMIT PREPARED ");
			break;

		case TRANS_STMT_ROLLBACK_PREPARED:
			delay_string_append_char(message, str, "ROLLBACK PREPARED ");
			break;

		default:
			break;
	}

	if (node->options)
		_rewriteSetTransactionModeList(BaseSelect, message, dblink, str, node->options);

	if (node->gid)
		delay_string_append_char(message, str, node->gid);
}


static void _rewriteTruncateStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TruncateStmt *node)
{
	delay_string_append_char(message, str, "TRUNCATE ");
	_rewriteNode(BaseSelect, message, dblink, str, node->relations);
}

static void _rewriteVacuumStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, VacuumStmt *node)
{
	if (node->options & VACOPT_VACUUM)
		delay_string_append_char(message, str, "VACUUM ");
	else
		delay_string_append_char(message, str, "ANALYZE ");

	if (node->options & VACOPT_FULL)
		delay_string_append_char(message, str, "FULL ");

	if (node->options & VACOPT_FREEZE)
		delay_string_append_char(message, str, "FREEZE ");

	if (node->options & VACOPT_VERBOSE)
		delay_string_append_char(message, str, "VERBOSE ");

	if (node->options & VACOPT_ANALYZE)
		delay_string_append_char(message, str, "ANALYZE ");

	_rewriteNode(BaseSelect, message, dblink, str, node->va_cols);
}

static void _rewriteExplainStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ExplainStmt *node)
{
	ListCell   *lc;

	delay_string_append_char(message, str, "EXPLAIN ");

	if (server_version_num < 90000)
	{
		foreach(lc, node->options)
		{
			DefElem    *opt = (DefElem *) lfirst(lc);

			if (strcmp(opt->defname, "analyze") == 0)
				delay_string_append_char(message, str, "ANALYZE ");
			else if (strcmp(opt->defname, "verbose") == 0)
				delay_string_append_char(message, str, "VERBOSE ");
		}
	}
	else
	{
		if (node->options)
		{
			delay_string_append_char(message, str, "(");
			foreach(lc, node->options)
			{
				DefElem    *opt = (DefElem *) lfirst(lc);

				if (list_head(node->options) != lc)
					delay_string_append_char(message, str, ", ");

				delay_string_append_char(message, str, opt->defname);
				delay_string_append_char(message, str, " ");
				if (opt->arg)
					_rewriteNode(BaseSelect, message, dblink, str, opt->arg);
			}
			delay_string_append_char(message, str, ")");
		}
	}

	_rewriteNode(BaseSelect, message, dblink, str, node->query);
}

static void _rewriteClusterStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ClusterStmt *node)
{
	delay_string_append_char(message, str, "CLUSTER ");

	if (node->indexname)
	{
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->indexname);
		delay_string_append_char(message, str, "\" ON ");
	}
	if (node->relation)
		_rewriteNode(BaseSelect, message, dblink, str, node->relation);
}

static void _rewriteCheckPointStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CheckPointStmt *node)
{
	delay_string_append_char(message, str, "CHECKPOINT");
}

static void _rewriteClosePortalStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ClosePortalStmt *node)
{
	delay_string_append_char(message, str, "CLOSE ");
	delay_string_append_char(message, str, "\"");
	delay_string_append_char(message, str, node->portalname);
	delay_string_append_char(message, str, "\"");
}

static void _rewriteListenStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ListenStmt *node)
{
	delay_string_append_char(message, str, "LISTEN ");
	delay_string_append_char(message, str, "\"");
	_rewriteNode(BaseSelect, message, dblink, str, node->conditionname);
	delay_string_append_char(message, str, "\"");
}

static void _rewriteUnlistenStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, UnlistenStmt *node)
{
	delay_string_append_char(message, str, "UNLISTEN ");
	if (node->conditionname == NULL)
		delay_string_append_char(message, str, "*");
	else
	{
		delay_string_append_char(message, str, "\"");
		_rewriteNode(BaseSelect, message, dblink, str, node->conditionname);
		delay_string_append_char(message, str, "\"");
	}
}

static void _rewriteLoadStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, LoadStmt *node)
{
	delay_string_append_char(message, str, "LOAD '");
	delay_string_append_char(message, str, node->filename);
	delay_string_append_char(message, str, "'");
}

static void _rewriteCopyStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CopyStmt *node)
{
	ListCell *lc;

	delay_string_append_char(message, str, "COPY ");

	if (node->query)
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->query);
		delay_string_append_char(message, str, ")");
	}

	_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	if (node->attlist)
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->attlist);
		delay_string_append_char(message, str, ")");
	}

	if (node->is_from == TRUE)
		delay_string_append_char(message, str, " FROM ");
	else
		delay_string_append_char(message, str, " TO ");

	if (node->filename)
	{
		delay_string_append_char(message, str, "'");
		delay_string_append_char(message, str, node->filename);
		delay_string_append_char(message, str, "' ");
	}
	else
		delay_string_append_char(message, str, node->is_from == TRUE ? "STDIN " : "STDOUT ");

	if (server_version_num < 90000)
	{
		foreach (lc, node->options)
		{
			DefElem *e = lfirst(lc);

			if (strcmp(e->defname, "format") == 0)
			{
				char *fmt = strVal(e->arg);
				
				if (strcmp(fmt, "text") == 0)
					;
				else if (strcmp(fmt, "binary") == 0)
					delay_string_append_char(message, str, "BINARY ");
				else if (strcmp(fmt, "csv") == 0)
					delay_string_append_char(message, str, "CSV ");
			}
			else if (strcmp(e->defname, "oids") == 0)
				delay_string_append_char(message, str, "OIDS ");
			else if (strcmp(e->defname, "delimiter") == 0)
			{
				delay_string_append_char(message, str, "DELIMITERS ");
				_rewriteValue(BaseSelect, message, dblink, str, (Value *) e->arg);
				delay_string_append_char(message, str, " ");
			}
			else if (strcmp(e->defname, "null") == 0)
			{
				delay_string_append_char(message, str, "NULL ");
				_rewriteValue(BaseSelect, message, dblink, str, (Value *) e->arg);
				delay_string_append_char(message, str, " ");
			}
			else if (strcmp(e->defname, "header") == 0)
				delay_string_append_char(message, str, "HEADER ");
			else if (strcmp(e->defname, "quote") == 0)
			{
				delay_string_append_char(message, str, "QUOTE ");
				_rewriteValue(BaseSelect, message, dblink, str, (Value *) e->arg);
				delay_string_append_char(message, str, " ");
			}
			else if (strcmp(e->defname, "escape") == 0)
			{
				delay_string_append_char(message, str, "ESCAPE ");
				_rewriteValue(BaseSelect, message, dblink, str, (Value *) e->arg);
				delay_string_append_char(message, str, " ");
			}
			else if (strcmp(e->defname, "force_quote") == 0)
			{
				delay_string_append_char(message, str, "FORCE QUOTE ");
				_rewriteIdList(BaseSelect, message, dblink, str, (List *) e->arg);
			}
			else if (strcmp(e->defname, "force_not_null") == 0)
			{
				delay_string_append_char(message, str, "FORCE NOT NULL ");
				_rewriteIdList(BaseSelect, message, dblink, str, (List *) e->arg);
			}
		}
	}
	else
	{
		/* version_num >= 90000 */
		if (node->options)
		{
			delay_string_append_char(message, str, "(");

			foreach (lc, node->options)
			{
				DefElem *e = lfirst(lc);

				if (list_head(node->options) != lc)
					delay_string_append_char(message, str, ", ");

				delay_string_append_char(message, str, e->defname);
				delay_string_append_char(message, str, " ");

				if (strcmp(e->defname, "format") == 0
					|| strcmp(e->defname, "oids") == 0
					|| strcmp(e->defname, "delimiter") == 0
					|| strcmp(e->defname, "null") == 0
					|| strcmp(e->defname, "header") == 0
					|| strcmp(e->defname, "quote") == 0
					|| strcmp(e->defname, "escape") == 0)
					_rewriteValue(BaseSelect, message, dblink, str, (Value *) e->arg);
				else if (strcmp(e->defname, "force_not_null") == 0)
				{
					delay_string_append_char(message, str, "(");
					_rewriteIdList(BaseSelect, message, dblink, str, (List *) e->arg);
					delay_string_append_char(message, str, ")");
				}
				else if (strcmp(e->defname, "force_quote") == 0)
				{
					if (IsA(e->arg, A_Star))
						delay_string_append_char(message, str, "*");
					else if (IsA(e->arg, List))
					{
						delay_string_append_char(message, str, "(");
						_rewriteIdList(BaseSelect, message, dblink, str, (List *) e->arg);
						delay_string_append_char(message, str, ")");
					}
				}
			}
			delay_string_append_char(message, str, ")");
		}
	}
}

static void _rewriteDeallocateStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DeallocateStmt *node)
{
	delay_string_append_char(message, str, "DEALLOCATE \"");
	delay_string_append_char(message, str, node->name);
	delay_string_append_char(message, str, "\"");
}

static void _rewriteRenameStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RenameStmt *node)
{
	ListCell *lc;
	char comma = 0;

	delay_string_append_char(message, str, "ALTER ");

	switch (node->renameType)
	{
		case OBJECT_AGGREGATE:
			delay_string_append_char(message, str, "AGGREGATE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " (");
			delay_string_append_char(message, str, ") RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_CONVERSION:
			delay_string_append_char(message, str, "CONVERSION ");
			_rewriteNode(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_DATABASE:
			delay_string_append_char(message, str, "DATABASE \"");
			delay_string_append_char(message, str, node->subname);
			delay_string_append_char(message, str, "\" RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_FUNCTION:
			delay_string_append_char(message, str, "FUNCTION ");

			foreach (lc, node->object)
			{
				Node *n = lfirst(lc);
				if (IsA(n, String))
				{
					Value *value = (Value *) n;
					if (comma == 0)
						comma = 1;
					else
						delay_string_append_char(message, str, ".");
					delay_string_append_char(message, str, "\"");
					delay_string_append_char(message, str, value->val.str);
					delay_string_append_char(message, str, "\"");
				}
				else
					_rewriteNode(BaseSelect, message, dblink, str, n);
			}

			delay_string_append_char(message, str, "(");
			_rewriteNode(BaseSelect, message, dblink, str, node->objarg);
			delay_string_append_char(message, str, ")");
			delay_string_append_char(message, str, " RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_ROLE:
			delay_string_append_char(message, str, "ROLE \"");
			delay_string_append_char(message, str, node->subname);
			delay_string_append_char(message, str, "\" RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_LANGUAGE:
			delay_string_append_char(message, str, "LANGUAGE \"");
			delay_string_append_char(message, str, node->subname);
			delay_string_append_char(message, str, "\" RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_OPCLASS:
			delay_string_append_char(message, str, "OPERATOR CLASS ");
			_rewriteNode(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " USING ");
			delay_string_append_char(message, str, node->subname);
			delay_string_append_char(message, str, " RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_SCHEMA:
			delay_string_append_char(message, str, "SCHEMA \"");
			delay_string_append_char(message, str, node->subname);
			delay_string_append_char(message, str, "\" RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_TABLE:
			delay_string_append_char(message, str, "TABLE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->relation);
			delay_string_append_char(message, str, " RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_INDEX:
			delay_string_append_char(message, str, "INDEX ");
			_rewriteNode(BaseSelect, message, dblink, str, node->relation);
			delay_string_append_char(message, str, " RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_COLUMN:
			delay_string_append_char(message, str, "TABLE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->relation);
			delay_string_append_char(message, str, " RENAME \"");
			delay_string_append_char(message, str, node->subname);
			delay_string_append_char(message, str, "\" TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_TRIGGER:
			delay_string_append_char(message, str, "TRIGGER \"");
			delay_string_append_char(message, str, node->subname);
			delay_string_append_char(message, str, "\" ON ");
			_rewriteNode(BaseSelect, message, dblink, str, node->relation);
			delay_string_append_char(message, str, " RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_TABLESPACE:
			delay_string_append_char(message, str, "TABLESPACE \"");
			delay_string_append_char(message, str, node->subname);
			delay_string_append_char(message, str, "\" RENAME TO \"");
			delay_string_append_char(message, str, node->newname);
			delay_string_append_char(message, str, "\"");
			break;

		default:
			break;
	}
}

static void
_rewriteOptRoleList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *options)
{
	ListCell *lc;

	foreach (lc, options)
	{
		DefElem *elem = lfirst(lc);
		Value *value = (Value *) elem->arg;

		if (strcmp(elem->defname, "password") == 0)
		{
			if (value == NULL)
				delay_string_append_char(message, str, " PASSWORD NULL");
			else
			{
				delay_string_append_char(message, str, " PASSWORD '");
				delay_string_append_char(message, str, value->val.str);
				delay_string_append_char(message, str, "'");
			}
		}
		else if (strcmp(elem->defname, "encryptedPassword") == 0)
		{
			delay_string_append_char(message, str, " ENCRYPTED PASSWORD '");
			delay_string_append_char(message, str, value->val.str);
			delay_string_append_char(message, str, "'");
		}
		else if (strcmp(elem->defname, "unencryptedPassword") == 0)
		{
			delay_string_append_char(message, str, " UNENCRYPTED PASSWORD '");
			delay_string_append_char(message, str, value->val.str);
			delay_string_append_char(message, str, "'");
		}
		else if (strcmp(elem->defname, "superuser") == 0)
		{
			if (value->val.ival == TRUE)
				delay_string_append_char(message, str, " SUPERUSER");
			else
				delay_string_append_char(message, str, " NOSUPERUSER");
		}
		else if (strcmp(elem->defname, "inherit") == 0)
		{
			if (value->val.ival == TRUE)
				delay_string_append_char(message, str, " INHERIT");
			else
				delay_string_append_char(message, str, " NOINHERIT");
		}
		else if (strcmp(elem->defname, "createdb") == 0)
		{
			if (value->val.ival == TRUE)
				delay_string_append_char(message, str, " CREATEDB");
			else
				delay_string_append_char(message, str, " NOCREATEDB");
		}
		else if (strcmp(elem->defname, "createrole") == 0)
		{
			if (value->val.ival == TRUE)
				delay_string_append_char(message, str, " CREATEROLE");
			else
				delay_string_append_char(message, str, " NOCREATEROLE");
		}
		else if (strcmp(elem->defname, "canlogin") == 0)
		{
			if (value->val.ival == TRUE)
				delay_string_append_char(message, str, " LOGIN");
			else
				delay_string_append_char(message, str, " NOLOGIN");
		}
		else if (strcmp(elem->defname, "connectionlimit") == 0)
		{
			char buf[16];

			delay_string_append_char(message, str, " CONNECTION LIMIT ");
			snprintf(buf, 16, "%ld", value->val.ival);
			delay_string_append_char(message, str, buf);
		}
		else if (strcmp(elem->defname, "validUntil") == 0)
		{
			delay_string_append_char(message, str, " VALID UNTIL '");
			delay_string_append_char(message, str, value->val.str);
			delay_string_append_char(message, str, "'");
		}
		else if (strcmp(elem->defname, "rolemembers") == 0)
		{
			delay_string_append_char(message, str, " ROLE ");
			_rewriteIdList(BaseSelect, message, dblink, str, (List *) elem->arg);
		}
		else if (strcmp(elem->defname, "sysid") == 0)
		{
			char buf[16];

			delay_string_append_char(message, str, " SYSID ");
			snprintf(buf, 16, "%ld", value->val.ival);
			delay_string_append_char(message, str, buf);
		}
		else if (strcmp(elem->defname, "adminmembers") == 0)
		{
			delay_string_append_char(message, str, " ADMIN ");
			_rewriteIdList(BaseSelect, message, dblink, str, (List *) elem->arg);
		}
		else if (strcmp(elem->defname, "addroleto") == 0)
		{
			delay_string_append_char(message, str, " IN ROLE ");
			_rewriteIdList(BaseSelect, message, dblink, str, (List *) elem->arg);
		}
	}
}

static void
_rewriteCreateRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateRoleStmt *node)
{
	delay_string_append_char(message, str, "CREATE ");
	switch (node->stmt_type)
	{
		case ROLESTMT_ROLE:
			delay_string_append_char(message, str, "ROLE \"");
			break;

		case ROLESTMT_USER:
			delay_string_append_char(message, str, "USER \"");
			break;

		case ROLESTMT_GROUP:
			delay_string_append_char(message, str, "GROUP \"");
			break;
	}
	delay_string_append_char(message, str, node->role);
	delay_string_append_char(message, str, "\"");

	_rewriteOptRoleList(BaseSelect, message, dblink, str, node->options);
}

static void
_rewriteAlterRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterRoleStmt *node)
{
	delay_string_append_char(message, str, "ALTER ROLE \"");
	delay_string_append_char(message, str, node->role);
	delay_string_append_char(message, str, "\"");
	if (node->options)
		_rewriteOptRoleList(BaseSelect, message, dblink, str, node->options);
}

static void
_rewriteAlterRoleSetStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterRoleSetStmt *node)
{
	delay_string_append_char(message, str, "ALTER ROLE \"");
	delay_string_append_char(message, str, node->role);
	delay_string_append_char(message, str, "\" ");

	if (node->setstmt)
	{
		_rewriteNode(BaseSelect, message, dblink, str, node->setstmt);
	}
}


static void
_rewriteSetTransactionModeList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list)
{
	ListCell *lc;
	char comma = 0;

	foreach (lc, list)
	{
		DefElem *elem = lfirst(lc);

		if (comma == 0)
			comma = 1;
		else
			delay_string_append_char(message, str, ",");

		if (strcmp(elem->defname, "transaction_isolation") == 0)
		{
			A_Const *v = (A_Const *) elem->arg;
			delay_string_append_char(message, str, " ISOLATION LEVEL ");
			delay_string_append_char(message, str, v->val.val.str);
		}
		else if (strcmp(elem->defname, "transaction_read_only") == 0)
		{
			A_Const *n = (A_Const *) elem->arg;
			if (n->val.val.ival == TRUE)
				delay_string_append_char(message, str, "READ ONLY ");
			else
				delay_string_append_char(message, str, "READ WRITE ");
		}
	}
}


static void
_rewriteSetRest(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, VariableSetStmt *node)
{
	if (strcmp(node->name, "timezone") == 0)
	{
		delay_string_append_char(message, str, "TIME ZONE ");
		if (node->kind != VAR_RESET)
			_rewriteNode(BaseSelect, message, dblink, str, node->args);
	}
	else if (strcmp(node->name, "TRANSACTION") == 0)
	{
		delay_string_append_char(message, str, "TRANSACTION ");
		_rewriteSetTransactionModeList(BaseSelect, message, dblink, str, node->args);
	}
	else if (strcmp(node->name, "SESSION CHARACTERISTICS") == 0)
	{
		delay_string_append_char(message, str, "SESSION CHARACTERISTICS AS TRANSACTION ");
		_rewriteSetTransactionModeList(BaseSelect, message, dblink, str, node->args);
	}
	else if (strcmp(node->name, "role") == 0)
	{
		delay_string_append_char(message, str, "ROLE ");
		if (node->kind != VAR_RESET)
			_rewriteNode(BaseSelect, message, dblink, str, node->args);
	}
	else if (strcmp(node->name, "session_authorization") == 0)
	{
		delay_string_append_char(message, str, "SESSION AUTHORIZATION ");
		if (node->args == NIL && node->kind != VAR_RESET)
			delay_string_append_char(message, str, "DEFAULT");
		else
			_rewriteNode(BaseSelect, message, dblink, str, node->args);
	}
	else if (strcmp(node->name, "transaction_isolation") == 0)
	{
		delay_string_append_char(message, str, "TRANSACTION ISOLATION LEVEL");
		if (node->kind != VAR_RESET)
			_rewriteSetTransactionModeList(BaseSelect, message, dblink, str, node->args);
	}
	else if (strcmp(node->name, "xmloption") == 0)
	{
		A_Const *v = linitial(node->args);
		delay_string_append_char(message, str, "XML OPTOIN ");
		delay_string_append_char(message, str, v->val.val.str);
	}
	else
	{
		delay_string_append_char(message, str, node->name);
		if (node->kind != VAR_RESET)
		{
			if (node->kind == VAR_SET_CURRENT)
			{
				delay_string_append_char(message, str, " FROM CURRENT");
			}
			else
			{
				delay_string_append_char(message, str, " TO ");
				if (node->args == NULL)
				{
					delay_string_append_char(message, str, "DEFAULT");
				}
				else
					_rewriteNode(BaseSelect, message, dblink, str, node->args);
			}
		}
	}
}

static void
_rewriteDropRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropRoleStmt *node)
{
	delay_string_append_char(message, str, "DROP ROLE ");
	if (node->missing_ok == TRUE)
		delay_string_append_char(message, str, "IF EXISTS ");
	_rewriteIdList(BaseSelect, message, dblink, str, node->roles);
}

static void
_rewriteCreateSchemaStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateSchemaStmt *node)
{
	delay_string_append_char(message, str, "CREATE SCHEMA \"");
	delay_string_append_char(message, str, node->schemaname);
	delay_string_append_char(message, str, "\"");
	if (node->authid)
	{
		delay_string_append_char(message, str, "AUTHORIZATION \"");
		delay_string_append_char(message, str, node->authid);
		delay_string_append_char(message, str, "\" ");
	}
	_rewriteNode(BaseSelect, message, dblink, str, node->schemaElts);
}

static void
_rewriteVariableSetStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, VariableSetStmt *node)
{
	if (node->kind == VAR_RESET_ALL)
	{
		delay_string_append_char(message, str, "RESET ALL");
		return;
	}

	if (node->kind == VAR_RESET)
		delay_string_append_char(message, str, "RESET ");
	else
		delay_string_append_char(message, str, "SET ");

	if (node->is_local)
		delay_string_append_char(message, str, "LOCAL ");

	_rewriteSetRest(BaseSelect, message, dblink, str, node);
}

static void
_rewriteVariableShowStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, VariableShowStmt *node)
{
	if (strcmp(node->name, "timezone") == 0)
		delay_string_append_char(message, str, "SHOW TIME ZONE");
	else if (strcmp(node->name, "transaction_isolation") == 0)
		delay_string_append_char(message, str, "SHOW TRANSACTION ISOLATION LEVEL");
	else if (strcmp(node->name, "session_authorization") == 0)
		delay_string_append_char(message, str, "SHOW SESSION AUTHORIZATION");
	else if (strcmp(node->name, "all") == 0)
		delay_string_append_char(message, str, "SHOW ALL");
	else
	{
		delay_string_append_char(message, str, "SHOW ");
		delay_string_append_char(message, str, node->name);
	}
}

static void
_rewriteConstraintsSetStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ConstraintsSetStmt *node)
{
	delay_string_append_char(message, str, "SET CONSTRAINTS ");

	if (node->constraints == NIL)
		delay_string_append_char(message, str, "ALL");
	else
		_rewriteNode(BaseSelect, message, dblink, str, node->constraints);

	delay_string_append_char(message, str, node->deferred == TRUE ? " DEFERRED" : " IMMEDIATE");
}

static void
_rewriteAlterTableCmd(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterTableCmd *node)
{
	char buf[16];

	switch (node->subtype)
	{
		case AT_AddColumn:
			delay_string_append_char(message, str, "ADD ");
			_rewriteNode(BaseSelect, message, dblink, str, node->def);
			break;

		case AT_ColumnDefault:
			delay_string_append_char(message, str, "ALTER \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\" ");
			if (node->def == NULL)
				delay_string_append_char(message, str, "DROP DEFAULT");
			else
			{
				delay_string_append_char(message, str, "SET DEFAULT ");
				_rewriteNode(BaseSelect, message, dblink, str, node->def);
			}
			break;

		case AT_DropNotNull:
			delay_string_append_char(message, str, "ALTER \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\" DROP NOT NULL");
			break;

		case AT_SetNotNull:
			delay_string_append_char(message, str, "ALTER \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\" SET NOT NULL");
			break;

		case AT_SetStatistics:
			delay_string_append_char(message, str, "ALTER \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\" SET STATISTICS ");
			snprintf(buf, 16, "%ld", ((Value *) node->def)->val.ival);
			delay_string_append_char(message, str, buf);
			break;

		case AT_SetStorage:
			delay_string_append_char(message, str, "ALTER \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\" SET STORAGE ");
			delay_string_append_char(message, str, ((Value *) node->def)->val.str);
			break;

		case AT_DropColumn:
			delay_string_append_char(message, str, "DROP \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\" ");
			if (node->behavior == DROP_CASCADE)
				delay_string_append_char(message, str, "CASCADE");
			break;

		case AT_AlterColumnType:
			delay_string_append_char(message, str, "ALTER \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\" TYPE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->def);
			break;

		case AT_AddConstraint:
			delay_string_append_char(message, str, "ADD ");
			_rewriteNode(BaseSelect, message, dblink, str, node->def);
			break;

		case AT_DropConstraint:
			delay_string_append_char(message, str, "DROP CONSTRAINT \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\"");
			if (node->behavior == DROP_CASCADE)
				delay_string_append_char(message, str, " CASCADE");
			break;

		case AT_DropOids:
			delay_string_append_char(message, str, "SET WITHOUT OIDS");
			break;

		case AT_ClusterOn:
			delay_string_append_char(message, str, "CLUSTER ON \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\"");
			break;

		case AT_EnableAlwaysTrig:
			/* not implemented */
			break;

		case AT_EnableReplicaTrig:
			/* not implemented */
			break;

		case AT_DropCluster:
			delay_string_append_char(message, str, "SET WITHOUT CLUSTER");
			break;

		case AT_EnableTrig:
			delay_string_append_char(message, str, "ENABLE TRIGGER \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\"");
			break;

		case AT_EnableTrigAll:
			delay_string_append_char(message, str, "ENABLE TRIGGER ALL");
			break;

		case AT_EnableRule:
			/* not implemented */
			break;

		case AT_EnableReplicaRule:
			/* not implemented */
			break;

		case AT_EnableAlwaysRule:
			/* not implemented */
			break;

		case AT_DisableRule:
			/* not implemented */
			break;

		case AT_AddInherit:
			/* not implemented */
			break;

		case AT_EnableTrigUser:
			delay_string_append_char(message, str, "ENABLE TRIGGER USER");
			break;

		case AT_DisableTrig:
			delay_string_append_char(message, str, "DISABLE TRIGGER \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\"");
			break;

		case AT_DisableTrigAll:
			delay_string_append_char(message, str, "DISABLE TRIGGER ALL");
			break;

		case AT_DisableTrigUser:
			delay_string_append_char(message, str, "DISABLE TRIGGER USER");
			break;

		case AT_ChangeOwner:
			delay_string_append_char(message, str, "OWNER TO \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\"");
			break;

		case AT_SetTableSpace:
			delay_string_append_char(message, str, "SET TABLESPACE \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\"");
			break;

		case AT_SetRelOptions:
			/* not implemented */
			break;

		case AT_ResetRelOptions:
			/* not implemented */
			break;

		default:
			break;
	}
}

static void
_rewriteAlterTableStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterTableStmt *node)
{
	if (node->relkind == OBJECT_TABLE)
		delay_string_append_char(message, str, "ALTER TABLE ");
	else
		delay_string_append_char(message, str, "ALTER INDEX ");

	_rewriteNode(BaseSelect, message, dblink, str, node->relation);
	delay_string_append_char(message, str, " ");
	_rewriteNode(BaseSelect, message, dblink, str, node->cmds);
}

static void
_rewriteOptSeqList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *options)
{
	ListCell *lc;

	foreach (lc, options)
	{
		DefElem *e = lfirst(lc);
		Value *v = (Value *)e->arg;
		char buf[16];

		if (strcmp(e->defname, "cycle") == 0)
		{
			if (v->val.ival == TRUE)
				delay_string_append_char(message, str, " CYCLE");
			else
				delay_string_append_char(message, str, " NO CYCLE");
		}
		else if (strcmp(e->defname, "minvalue") == 0 && !v)
			delay_string_append_char(message, str, " NO MINVALUE");
		else if (strcmp(e->defname, "maxvalue") == 0 && !v)
			delay_string_append_char(message, str, " NO MAXVALUE");
		else if (strcmp(e->defname, "owned_by") == 0)
		{
			delay_string_append_char(message, str, " OWNED BY ");
			_rewriteIdList(BaseSelect, message, dblink, str, (List *)e->arg);
		}
		else
		{
			if (strcmp(e->defname, "cache") == 0)
				delay_string_append_char(message, str, " CACHE ");
			else if (strcmp(e->defname, "increment") == 0)
				delay_string_append_char(message, str, " INCREMENT ");
			else if (strcmp(e->defname, "maxvalue") == 0 && v)
				delay_string_append_char(message, str, " MAXVALUE ");
			else if (strcmp(e->defname, "minvalue") == 0 && v)
				delay_string_append_char(message, str, " MINVALUE ");
			else if (strcmp(e->defname, "start") == 0)
				delay_string_append_char(message, str, " START ");
			else if (strcmp(e->defname, "restart") == 0)
				delay_string_append_char(message, str, " RESTART ");

			if (IsA(e->arg, String))
				delay_string_append_char(message, str, v->val.str);
			else
			{
				snprintf(buf, 16, "%ld", v->val.ival);
				delay_string_append_char(message, str, buf);
			}
		}
	}
}

static void
_rewriteCreateSeqStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateSeqStmt *node)
{
	delay_string_append_char(message, str, "CREATE ");
	if (node->sequence->relpersistence)
		delay_string_append_char(message, str, "TEMP ");
	delay_string_append_char(message, str, "SEQUENCE ");
	_rewriteNode(BaseSelect, message, dblink, str, node->sequence);

	_rewriteOptSeqList(BaseSelect, message, dblink, str, node->options);
}

static void
_rewriteAlterSeqStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterSeqStmt *node)
{
	delay_string_append_char(message, str, "ALTER SEQUENCE ");
	_rewriteNode(BaseSelect, message, dblink, str, node->sequence);
	_rewriteOptSeqList(BaseSelect, message, dblink, str, node->options);
}

static void
_rewriteCreatePLangStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreatePLangStmt *node)
{
	delay_string_append_char(message, str, "CREATE ");
	if (node->pltrusted == true)
		delay_string_append_char(message, str, "TRUSTED ");
	delay_string_append_char(message, str, "LANGUAGE \"");
	delay_string_append_char(message, str, node->plname);
	delay_string_append_char(message, str, "\"");

	if (node->plhandler != NIL)
	{
		ListCell *lc;
		char dot = 0;

		delay_string_append_char(message, str, " HANDLER ");
		foreach (lc, node->plhandler)
		{
			Value *v = lfirst(lc);

			if (dot == 0)
				dot = 1;
			else
				delay_string_append_char(message, str, ".");

			delay_string_append_char(message, str, "\"");
			delay_string_append_char(message, str, v->val.str);
			delay_string_append_char(message, str, "\"");
		}
	}

	if (node->plvalidator != NIL)
	{
		ListCell *lc;
		char dot = 0;

		delay_string_append_char(message, str, " VALIDATOR ");
		foreach (lc, node->plvalidator)
		{
			Value *v = lfirst(lc);

			if (dot == 0)
				dot = 1;
			else
				delay_string_append_char(message, str, ".");

			delay_string_append_char(message, str, "\"");
			delay_string_append_char(message, str, v->val.str);
			delay_string_append_char(message, str, "\"");
		}
	}
}

static void
_rewriteCreateTableSpaceStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateTableSpaceStmt *node)
{
	delay_string_append_char(message, str, "CREATE TABLESPACE \"");
	delay_string_append_char(message, str, node->tablespacename);
	delay_string_append_char(message, str, "\" ");

	if (node->owner)
	{
		delay_string_append_char(message, str, "OWNER \"");
		delay_string_append_char(message, str, node->owner);
		delay_string_append_char(message, str, "\" ");
	}

	delay_string_append_char(message, str, "LOCATION '");
	delay_string_append_char(message, str, node->location);
	delay_string_append_char(message, str, "'");
}

static void
_rewriteDropTableSpaceStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropTableSpaceStmt *node)
{
	delay_string_append_char(message, str, "DROP TABLESPACE \"");
	delay_string_append_char(message, str, node->tablespacename);
	delay_string_append_char(message, str, "\"");
}

static void
_rewriteFuncName(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *func_name)
{
	ListCell *lc;
	Value *v;
	char dot = 0;

	if (func_name == NULL)
		return;

	foreach (lc, func_name)
	{
		v = (Value *) lfirst(lc);

		if (dot == 0)
			dot = 1;
		else
			delay_string_append_char(message, str, ".");

		if (IsA(v, String))
		{
			delay_string_append_char(message, str, v->val.str);
		}
	}
}

static void
_rewriteCreateTrigStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateTrigStmt *node)
{
	bool	has_events = false;

	if (node->isconstraint == TRUE)
		delay_string_append_char(message, str, "CREATE CONSTRAINT TRIGGER \"");
	else
		delay_string_append_char(message, str, "CREATE TRIGGER \"");
	delay_string_append_char(message, str, node->trigname);
	delay_string_append_char(message, str, "\" ");

	if (node->timing == TRUE)
		delay_string_append_char(message, str, "BEFORE ");
	else
		delay_string_append_char(message, str, "AFTER ");

	if (node->events & TRIGGER_TYPE_INSERT)
	{
		delay_string_append_char(message, str, "INSERT ");
		has_events = true;
	}
	if (node->events & TRIGGER_TYPE_DELETE)
	{
		if (has_events)
			delay_string_append_char(message, str, "OR ");
		delay_string_append_char(message, str, "DELETE ");
		has_events = true;
	}
	if (node->events & TRIGGER_TYPE_UPDATE)
	{
		if (has_events)
			delay_string_append_char(message, str, "OR ");
		delay_string_append_char(message, str, "UPDATE ");
		has_events = true;
	}
	if (node->events & TRIGGER_TYPE_TRUNCATE)
	{
		if (has_events)
			delay_string_append_char(message, str, "OR ");
		delay_string_append_char(message, str, "TRUNCATE ");
		has_events = true;
	}

	delay_string_append_char(message, str, "ON ");
	_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	if (node->constrrel)
	{
		delay_string_append_char(message, str, " FROM ");
		_rewriteNode(BaseSelect, message, dblink, str, node->constrrel);
	}

	if (node->deferrable)
		delay_string_append_char(message, str, " DEFERRABLE");
	if (node->initdeferred)
		delay_string_append_char(message, str, " INITIALLY DEFERRED");

	if (node->row == TRUE)
		delay_string_append_char(message, str, " FOR EACH ROW ");
	else
		delay_string_append_char(message, str, " FOR EACH STATEMENT ");

	delay_string_append_char(message, str, "EXECUTE PROCEDURE ");

	_rewriteFuncName(BaseSelect, message, dblink, str, node->funcname);
	delay_string_append_char(message, str, "(");
	_rewriteNode(BaseSelect, message, dblink, str, node->args);
	delay_string_append_char(message, str, ")");
}

static void
_rewriteDefinition(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *definition)
{
	ListCell *lc;
	char comma = 0;

	if (definition == NIL)
		return;

	delay_string_append_char(message, str, "(");
	foreach (lc, definition)
	{
		DefElem *e = lfirst(lc);

		if (comma == 0)
			comma = 1;
		else
			delay_string_append_char(message, str, ", ");

		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, e->defname);
		delay_string_append_char(message, str, "\"");

		if (e->arg)
		{
			delay_string_append_char(message, str, "=");
			_rewriteNode(BaseSelect, message, dblink, str, e->arg);
		}
	}
	delay_string_append_char(message, str, ")");
}

static void
_rewriteDefineStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DefineStmt *node)
{
	ListCell *lc;
	char dot = 0;

	switch (node->kind)
	{
		case OBJECT_AGGREGATE:
			delay_string_append_char(message, str, "CREATE AGGREGATE ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->defnames);
			delay_string_append_char(message, str, " ");
			_rewriteDefinition(BaseSelect, message, dblink, str, node->definition);
			break;

		case OBJECT_OPERATOR:
			delay_string_append_char(message, str, "CREATE OPERATOR ");

			foreach (lc, node->defnames)
			{
				Value *v = lfirst(lc);

				if (dot == 0)
					dot = 1;
				else
					delay_string_append_char(message, str, ".");

				delay_string_append_char(message, str, v->val.str);
			}

			delay_string_append_char(message, str, " ");
			_rewriteDefinition(BaseSelect, message, dblink, str, node->definition);
			break;

		case OBJECT_TYPE:
			delay_string_append_char(message, str, "CREATE TYPE");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->defnames);
			delay_string_append_char(message, str, " ");
			_rewriteDefinition(BaseSelect, message, dblink, str, node->definition);
			break;

		case OBJECT_TSPARSER:
			delay_string_append_char(message, str, "CREATE TEXT SEARCH PARSER ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->defnames);
			_rewriteDefinition(BaseSelect, message, dblink, str, node->definition);
			break;

		case OBJECT_TSDICTIONARY:
			delay_string_append_char(message, str, "CREATE TEXT SEARCH DICTIONARY ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->defnames);
			_rewriteDefinition(BaseSelect, message, dblink, str, node->definition);
			break;

		case OBJECT_TSTEMPLATE:
			delay_string_append_char(message, str, "CREATE TEXT SEARCH TEMPLATE ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->defnames);
			_rewriteDefinition(BaseSelect, message, dblink, str, node->definition);
			break;

		case OBJECT_TSCONFIGURATION:
			delay_string_append_char(message, str, "CREATE TEXT SEARCH CONFIGURATION ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->defnames);
			_rewriteDefinition(BaseSelect, message, dblink, str, node->definition);
			break;

		default:
			break;
	}
}

static void
_rewriteOperatorName(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list)
{
	char dot = 0;
	ListCell *lc;

	foreach (lc, list)
	{
		Value *v = lfirst(lc);

		if (dot == 0)
			dot = 1;
		else
			delay_string_append_char(message, str, ".");

		delay_string_append_char(message, str, v->val.str);
	}
}

static void
_rewriteCreateOpClassItem(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateOpClassItem *node)
{
	char buf[16];

	switch (node->itemtype)
	{
		case OPCLASS_ITEM_OPERATOR:
			delay_string_append_char(message, str, "OPERATOR ");
			snprintf(buf, 16, "%d", node->number);
			delay_string_append_char(message, str, buf);
			delay_string_append_char(message, str, " ");
			_rewriteOperatorName(BaseSelect, message, dblink, str, node->name);

			if (node->args != NIL)
			{
				delay_string_append_char(message, str, "(");
				_rewriteNode(BaseSelect, message, dblink, str, node->args);
				delay_string_append_char(message, str, ")");
			}
			/* XXX
			if (node->recheck == TRUE)
				delay_string_append_char(message, str, " RECHECK");
			*/
			break;

		case OPCLASS_ITEM_FUNCTION:
			delay_string_append_char(message, str, "FUNCTION ");
			snprintf(buf, 16, "%d", node->number);
			delay_string_append_char(message, str, buf);
			delay_string_append_char(message, str, " ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->name);
			delay_string_append_char(message, str, "(");
			_rewriteNode(BaseSelect, message, dblink, str, node->args);
			delay_string_append_char(message, str, ")");
			break;

		case OPCLASS_ITEM_STORAGETYPE:
			delay_string_append_char(message, str, "STORAGE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->storedtype);
			break;

		default:
			break;
	}

}

static void
_rewriteCreateOpClassStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateOpClassStmt *node)
{
	delay_string_append_char(message, str, "CREATE OPERATOR CLASS ");
	_rewriteFuncName(BaseSelect, message, dblink, str, node->opclassname);

	if (node->isDefault == TRUE)
		delay_string_append_char(message, str, " DEFAULT");

	delay_string_append_char(message, str, " FOR TYPE ");
	_rewriteNode(BaseSelect, message, dblink, str, node->datatype);
	delay_string_append_char(message, str, " USING ");
	delay_string_append_char(message, str, node->amname);
	delay_string_append_char(message, str, " AS ");
	_rewriteNode(BaseSelect, message, dblink, str, node->items);
}

static void
_rewriteDropStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropStmt *node)
{
	//ListCell *lc;
	//char comma = 0;

	/*
	 * types of drop statement
	 * 1: DROP obj1, target2, ...
	 * 2: DROP obj (arg1, arg2, ...)
	 * 3: DROP obj ON obj
	 * 0: other
	 */
	int drop_stmt_type = 1;

	delay_string_append_char(message, str, "DROP ");
	switch (node->removeType)
	{
		case OBJECT_TABLE:
			delay_string_append_char(message, str, "TABLE ");
			break;

		case OBJECT_SEQUENCE:
			delay_string_append_char(message, str, "SEQUENCE ");
			break;

		case OBJECT_VIEW:
			delay_string_append_char(message, str, "VIEW ");
			break;

		case OBJECT_INDEX:
			delay_string_append_char(message, str, "INDEX ");
			break;

		case OBJECT_TYPE:
			delay_string_append_char(message, str, "TYPE ");
			break;

		case OBJECT_DOMAIN:
			delay_string_append_char(message, str, "DOMAIN ");
			break;

		case OBJECT_CONVERSION:
			delay_string_append_char(message, str, "CONVERSION ");
			break;

		case OBJECT_SCHEMA:
			delay_string_append_char(message, str, "SCHEMA ");
			break;

		case OBJECT_FUNCTION:
			drop_stmt_type = 2;
			delay_string_append_char(message, str, "FUNCTION ");
			break;

		case OBJECT_AGGREGATE:
			drop_stmt_type = 2;
			delay_string_append_char(message, str, "AGGREGATE ");
			break;

		case OBJECT_OPERATOR:
			drop_stmt_type = 2;
			delay_string_append_char(message, str, "OPERATOR CLASS ");
			break;

        case OBJECT_TRIGGER:
			drop_stmt_type = 3;
            delay_string_append_char(message, str, "TRIGGER ");
            break;

        case OBJECT_RULE:
			drop_stmt_type = 3;
            delay_string_append_char(message, str, "RULE ");
            break;

		case OBJECT_LANGUAGE:
			delay_string_append_char(message, str, "LANGUAGE ");
            break;

		case OBJECT_CAST:
			drop_stmt_type = 0;
			delay_string_append_char(message, str, "CAST (");
			//_rewriteNode(BaseSelect, message, dblink, str, node->objects);
			_rewriteFuncNameList(BaseSelect, message, dblink, str, node->objects);
			delay_string_append_char(message, str, " AS ");
			//_rewriteNode(BaseSelect, message, dblink, str, node->arguments);
			_rewriteFuncNameList(BaseSelect, message, dblink, str, node->arguments);
			delay_string_append_char(message, str, ")");
            break;

		case OBJECT_OPCLASS:
			drop_stmt_type = 0;
			delay_string_append_char(message, str, "OPERATOR CLASS ");
			//_rewriteFuncName(BaseSelect, message, dblink, str, node->objects);
			_rewriteFuncNameList(BaseSelect, message, dblink, str, node->objects);
			delay_string_append_char(message, str, " USING ");
			//delay_string_append_char(message, str, node->arguments);
			_rewriteFuncNameList(BaseSelect, message, dblink, str, node->arguments);
			break;

		default:
			break;
	}

	/* IF EXISTS */
	if (node->missing_ok)
		delay_string_append_char(message, str, "IF EXISTS ");

	switch (drop_stmt_type)
	{
		case 1: /* DROP obj1, obj2, ... */
			/*
			foreach Array(lc, node->objects)
			{
				if (comma == 0)
					comma = 1;
				else
					delay_string_append_char(message, str, ", ");
				_rewriteFuncName(BaseSelect, message, dblink, str, lfirst(lc));
			}
			*/
			_rewriteFuncNameList(BaseSelect, message, dblink, str, node->objects);
			break;

		case 2: /* DROP obj (arg1, arg2, ...) */
			delay_string_append_char(message, str, " (");
			_rewriteNode(BaseSelect, message, dblink, str, node->arguments);
			delay_string_append_char(message, str, ")");
			break;

		case 3: /* DROP obj ON obj */
			//delay_string_append_char(message, str, node->objects); // ???
			_rewriteFuncNameList(BaseSelect, message, dblink, str, node->objects);
			delay_string_append_char(message, str, "\" ON ");
			_rewriteNode(BaseSelect, message, dblink, str, node->objects);
			break;

		default:
			break;
	}

	/* CASCADE */
	if (node->behavior == DROP_CASCADE)
		delay_string_append_char(message, str, " CASCADE");
}

static void
_rewriteFuncNameList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list)
{
	ListCell *lc;
	char comma = 0;

	foreach (lc, list)
	{
		if (comma == 0)
			comma = 1;
		else
			delay_string_append_char(message, str, ", ");
		_rewriteFuncName(BaseSelect, message, dblink, str, lfirst(lc));
	}
}

static void
_rewriteFetchStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FetchStmt *node)
{
	char buf[16];

	snprintf(buf, 16, "%ld", node->howMany);

	if (node->ismove == TRUE)
		delay_string_append_char(message, str, "MOVE ");
	else
		delay_string_append_char(message, str, "FETCH ");

	switch (node->direction)
	{
		case FETCH_FORWARD:
			delay_string_append_char(message, str, "FORWARD ");
			if (node->howMany == FETCH_ALL)
				delay_string_append_char(message, str, "ALL ");
			else
			{
				delay_string_append_char(message, str, buf);
				delay_string_append_char(message, str, " ");
			}
			break;

		case FETCH_BACKWARD:
			delay_string_append_char(message, str, "BACKWARD ");
			if (node->howMany == FETCH_ALL)
				delay_string_append_char(message, str, "ALL ");
			else
			{
				delay_string_append_char(message, str, buf);
				delay_string_append_char(message, str, " ");
			}
			break;

		case FETCH_ABSOLUTE:
			if (node->howMany == 1)
				delay_string_append_char(message, str, "FIRST ");
			else if (node->howMany == -1)
				delay_string_append_char(message, str, "LAST ");
			else
			{
				delay_string_append_char(message, str, "ABSOLUTE ");
				delay_string_append_char(message, str, buf);
				delay_string_append_char(message, str, " ");
			}
			break;

		case FETCH_RELATIVE:
			delay_string_append_char(message, str, "RELATIVE ");
			delay_string_append_char(message, str, buf);
			delay_string_append_char(message, str, " ");
			break;
	}

	delay_string_append_char(message, str, "IN \"");
	delay_string_append_char(message, str, node->portalname);
	delay_string_append_char(message, str, "\"");
}

static void
_rewritePrivilegeList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list)
{
	ListCell *lc;
	char comma = 0;

	if (list == NIL)
		delay_string_append_char(message, str, "ALL");
	else
	{
		foreach (lc, list)
		{
			Value *v = lfirst(lc);

			if (comma == 0)
				comma = 1;
			else
				delay_string_append_char(message, str, ", ");

			delay_string_append_char(message, str, v->val.str);
		}
	}
}

static void
_rewriteFunctionParameter(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FunctionParameter *node)
{
	switch (node->mode)
	{
		case FUNC_PARAM_OUT:
			delay_string_append_char(message, str, "OUT ");
			break;

		case FUNC_PARAM_INOUT:
			delay_string_append_char(message, str, "INOUT ");
			break;

		default:
			break;
	}

	/* function name */
	if (node->name)
	{
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->name);
		delay_string_append_char(message, str, "\" ");
	}

	_rewriteNode(BaseSelect, message, dblink, str, node->argType);
}

static void
_rewriteFuncWithArgs(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncWithArgs *node)
{
	_rewriteFuncName(BaseSelect, message, dblink, str, node->funcname);
	delay_string_append_char(message, str, "(");
	_rewriteNode(BaseSelect, message, dblink, str, node->funcargs);
	delay_string_append_char(message, str, ")");
}


static void
_rewritePrivGrantee(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, PrivGrantee *node)
{
	if (node->rolname == NULL)
		delay_string_append_char(message, str, "PUBLIC");
	else
	{
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->rolname);
		delay_string_append_char(message, str, "\"");
	}
}

static void
_rewriteGrantStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, GrantStmt *node)
{
	if (node->is_grant == true)
		delay_string_append_char(message, str, "GRANT ");
	else
	{
		delay_string_append_char(message, str, "REVOKE ");
		if (node->grant_option == true)
			delay_string_append_char(message, str, "GRANT OPTION FOR ");
	}

	_rewritePrivilegeList(BaseSelect, message, dblink, str, node->privileges);

	delay_string_append_char(message, str, " ON ");

	switch (node->objtype)
	{
		case ACL_OBJECT_RELATION:
			_rewriteNode(BaseSelect, message, dblink, str, node->objects);
			break;

		case ACL_OBJECT_SEQUENCE:
			delay_string_append_char(message, str, "SEQUENCE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->objects);
			break;

		case ACL_OBJECT_FUNCTION:
			delay_string_append_char(message, str, "FUNCTION ");
			_rewriteNode(BaseSelect, message, dblink, str, node->objects);
			break;

		case ACL_OBJECT_DATABASE:
			delay_string_append_char(message, str, "DATABASE ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->objects);
			break;

		case ACL_OBJECT_LANGUAGE:
			delay_string_append_char(message, str, "LANGUAGE ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->objects);
			break;

		case ACL_OBJECT_NAMESPACE:
			delay_string_append_char(message, str, "SCHEMA ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->objects);
			break;

		case ACL_OBJECT_TABLESPACE:
			delay_string_append_char(message, str, "TABLESPACE ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->objects);
			break;

		case ACL_OBJECT_COLUMN:
		case ACL_OBJECT_FDW:
		case ACL_OBJECT_FOREIGN_SERVER:
			ereport(LOG,
				(errmsg("rewriting GRANT statement"),
					 errdetail("modules for ACL_OBJECT_COLUMN, ACL_OBJECT_FDW and ACL_OBJECT_FOREIGN_SERVER are not implemented yet")));
			break;

		default:			
			ereport(LOG,
				(errmsg("rewriting GRANT statement"),
					 errdetail("unknowm node type %d", node->objtype)));
			break;
	}

	if (node->is_grant == true)
		delay_string_append_char(message, str, " TO ");
	else
		delay_string_append_char(message, str, " FROM ");
	_rewriteNode(BaseSelect, message, dblink, str, node->grantees);

	if (node->is_grant == true && node->grant_option == TRUE)
		delay_string_append_char(message, str, " WITH GRANT OPTION");

	if (node->behavior == DROP_CASCADE)
		delay_string_append_char(message, str, " CASCADE");

}

static void
_rewriteGrantRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, GrantRoleStmt *node)
{
	if (node->is_grant == true)
		delay_string_append_char(message, str, "GRANT ");
	else
	{
		delay_string_append_char(message, str, "REVOKE ");
		if (node->admin_opt == true)
			delay_string_append_char(message, str, "ADMIN OPTION FOR ");
	}

	_rewriteIdList(BaseSelect, message, dblink, str, node->granted_roles);

	delay_string_append_char(message, str, node->is_grant == true ? " TO " : " FROM ");

	_rewriteIdList(BaseSelect, message, dblink, str, node->grantee_roles);

	if (node->admin_opt == true && node->is_grant == true)
		delay_string_append_char(message, str, "  WITH ADMIN OPTION");

	if (node->grantor != NULL)
	{
		delay_string_append_char(message, str, " GRANTED BY \"");
		delay_string_append_char(message, str, node->grantor);
		delay_string_append_char(message, str, "\"");
	}

	if (node->behavior == DROP_CASCADE)
		delay_string_append_char(message, str, " CASCADE");
}

static void
_rewriteFuncOptList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list)
{
	ListCell *lc;

	foreach (lc, list)
	{
		DefElem *e = lfirst(lc);
		Value *v = (Value *) e->arg;

		if (strcmp(e->defname, "strict") == 0)
		{
			if (v->val.ival == TRUE)
				delay_string_append_char(message, str, " STRICT");
			else
				delay_string_append_char(message, str, " CALLED ON NULL INPUT");
		}
		else if (strcmp(e->defname, "volatility") == 0)
		{
			char *s = v->val.str;
			if (strcmp(s, "immutable") == 0)
				delay_string_append_char(message, str, " IMMUTABLE");
			else if (strcmp(s, "stable") == 0)
				delay_string_append_char(message, str, " STABLE");
			else if (strcmp(s, "volatile") == 0)
				delay_string_append_char(message, str, " VOLATILE");
		}
		else if (strcmp(e->defname, "security") == 0)
		{
			if (v->val.ival == TRUE)
				delay_string_append_char(message, str, " SECURITY DEFINER");
			else
				delay_string_append_char(message, str, " SECURITY INVOKER");
		}
		else if (strcmp(e->defname, "as") == 0)
		{
			delay_string_append_char(message, str, " AS ");
			_rewriteNode(BaseSelect, message, dblink, str, e->arg);
		}
		else if (strcmp(e->defname, "language") == 0)
		{
			delay_string_append_char(message, str, " LANGUAGE '");
			delay_string_append_char(message, str, v->val.str);
			delay_string_append_char(message, str, "'");
		}
	}
}

static void
_rewriteCreateFunctionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateFunctionStmt *node)
{
	delay_string_append_char(message, str, "CREATE ");
	if (node->replace == true)
		delay_string_append_char(message, str, "OR REPLACE ");
	delay_string_append_char(message, str, "FUNCTION ");

	_rewriteFuncName(BaseSelect, message, dblink, str, node->funcname);

	delay_string_append_char(message, str, " (");
	_rewriteNode(BaseSelect, message, dblink, str, node->parameters);
	delay_string_append_char(message, str, ")");

	if (node->returnType)
	{
		delay_string_append_char(message, str, " RETURNS ");
		_rewriteNode(BaseSelect, message, dblink, str, node->returnType);
	}

	_rewriteFuncOptList(BaseSelect, message, dblink, str, node->options);

	if (node->withClause)
	{
		delay_string_append_char(message, str, " WITH ");
		_rewriteDefinition(BaseSelect, message, dblink, str, node->withClause);
	}
}

static void
_rewriteAlterFunctionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterFunctionStmt *node)
{
	delay_string_append_char(message, str, "ALTER FUNCTION ");
	_rewriteNode(BaseSelect, message, dblink, str, node->func);
	_rewriteFuncOptList(BaseSelect, message, dblink, str, node->actions);
}

static void
_rewriteCreateCastStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateCastStmt *node)
{
	delay_string_append_char(message, str, "CREATE CAST (");
	_rewriteNode(BaseSelect, message, dblink, str, node->sourcetype);
	delay_string_append_char(message, str, " AS ");
	_rewriteNode(BaseSelect, message, dblink, str, node->targettype);
	delay_string_append_char(message, str, ") WITH FUNCTION ");
	_rewriteNode(BaseSelect, message, dblink, str, node->func);

	switch (node->context)
	{
		case COERCION_IMPLICIT:
			delay_string_append_char(message, str, " AS IMPLICIT");
			break;

		case COERCION_ASSIGNMENT:
			delay_string_append_char(message, str, " AS ASSIGNMENT");
			break;

		default:
			break;
	}
}

static void
_rewriteReindexStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ReindexStmt *node)
{
	delay_string_append_char(message, str, "REINDEX ");

	switch (node->kind)
	{
		case OBJECT_DATABASE:
			if (node->do_system == true && node->do_user == false)
				delay_string_append_char(message, str, "SYSTEM ");
			else
				delay_string_append_char(message, str, "DATABASE ");
			break;

		case OBJECT_INDEX:
			delay_string_append_char(message, str, "INDEX ");
			break;

		case OBJECT_TABLE:
			delay_string_append_char(message, str, "TABLE ");
			break;

		default:
			break;
	}

	if (node->relation)
		_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	if (node->name)
	{
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, (char *) node->name);
		delay_string_append_char(message, str, "\"");
	}
}

static void
_rewriteAlterObjectSchemaStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterObjectSchemaStmt *node)
{
	delay_string_append_char(message, str, "ALTER ");

	switch (node->objectType)
	{
		case OBJECT_AGGREGATE:
			delay_string_append_char(message, str, "AGGREGATE ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, "(");
			if (lfirst(list_head(node->objarg)) == NULL)
				delay_string_append_char(message, str, "*");
			else
				_rewriteNode(BaseSelect, message, dblink, str, lfirst(list_head(node->objarg)));
			delay_string_append_char(message, str, ") SET SCHAME \"");
			delay_string_append_char(message, str, node->newschema);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_DOMAIN:
			delay_string_append_char(message, str, "DOMAIN ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " SET SCHEMA \"");
			delay_string_append_char(message, str, node->newschema);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_FUNCTION:
			delay_string_append_char(message, str, "FUNCTION ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, "(");
			_rewriteNode(BaseSelect, message, dblink, str, node->objarg);
			delay_string_append_char(message, str, ") SET SCHEMA \"");
			delay_string_append_char(message, str, node->newschema);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_SEQUENCE:
			delay_string_append_char(message, str, "SEQUENCE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->relation);
			delay_string_append_char(message, str, " SET SCHEMA \"");
			delay_string_append_char(message, str, node->newschema);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_TABLE:
			delay_string_append_char(message, str, "TABLE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->relation);
			delay_string_append_char(message, str, " SET SCHEMA \"");
			delay_string_append_char(message, str, node->newschema);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_TYPE:
			delay_string_append_char(message, str, "TYPE ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " SET SCHEMA \"");
			delay_string_append_char(message, str, node->newschema);
			delay_string_append_char(message, str, "\"");
			break;

		default:
			break;
	}
}

static void
_rewriteAlterOwnerStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterOwnerStmt *node)
{
	delay_string_append_char(message, str, "ALTER ");

	switch (node->objectType)
	{
		case OBJECT_AGGREGATE:
			delay_string_append_char(message, str, "AGGREGATE ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, "(");
			if (lfirst(list_head(node->objarg)) == NULL)
				delay_string_append_char(message, str, "*");
			else
				_rewriteNode(BaseSelect, message, dblink, str, lfirst(list_head(node->objarg)));
			delay_string_append_char(message, str, ") OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_CONVERSION:
			delay_string_append_char(message, str, "CONVERSION ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_DATABASE:
			delay_string_append_char(message, str, "DATABASE \"");
			_rewriteIdList(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, "\" OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_DOMAIN:
			delay_string_append_char(message, str, "DOMAIN ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_FUNCTION:
			delay_string_append_char(message, str, "FUNCTION ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, "(");
			_rewriteNode(BaseSelect, message, dblink, str, node->objarg);
			delay_string_append_char(message, str, ") OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_OPERATOR:
			delay_string_append_char(message, str, "OPERATOR ");
			_rewriteOperatorName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, "(");
			_rewriteOperatorArgTypes(BaseSelect, message, dblink, str, node->objarg);
			delay_string_append_char(message, str, ") OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_OPCLASS:
			delay_string_append_char(message, str, "OPERATOR CLASS ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " USING ");
			delay_string_append_char(message, str, linitial(node->objarg));
			delay_string_append_char(message, str, " OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_SCHEMA:
			delay_string_append_char(message, str, "SCHEMA \"");
			delay_string_append_char(message, str, linitial(node->object));
			delay_string_append_char(message, str, "\" OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_TYPE:
			delay_string_append_char(message, str, "TYPE ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->object);
			delay_string_append_char(message, str, " OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		case OBJECT_TABLESPACE:
			delay_string_append_char(message, str, "TABLESPACE \"");
			delay_string_append_char(message, str, linitial(node->object));
			delay_string_append_char(message, str, "\" OWNER TO \"");
			delay_string_append_char(message, str, node->newowner);
			delay_string_append_char(message, str, "\"");
			break;

		default:
			break;
	}
}

static void
_rewriteRuleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RuleStmt *node)
{
	delay_string_append_char(message, str, "CREATE ");
	if (node->replace)
		delay_string_append_char(message, str, "OR REPLACE ");
	delay_string_append_char(message, str, "RULE \"");
	delay_string_append_char(message, str, node->rulename);
	delay_string_append_char(message, str, "\" AS ON ");

	switch (node->event)
	{
		case CMD_SELECT:
			delay_string_append_char(message, str, "SELECT");
			break;

		case CMD_UPDATE:
			delay_string_append_char(message, str, "UPDATE");
			break;

		case CMD_DELETE:
			delay_string_append_char(message, str, "DELETE");
			break;

		case CMD_INSERT:
			delay_string_append_char(message, str, "INSERT");
			break;

		default:
			break;
	}

	delay_string_append_char(message, str, " TO ");
	_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	if (node->whereClause)
	{
		delay_string_append_char(message, str, " WHERE ");
		_rewriteNode(BaseSelect, message, dblink, str, node->whereClause);
	}

	delay_string_append_char(message, str, " DO ");

	if (node->instead)
		delay_string_append_char(message, str, "INSTEAD ");

	if (node->actions == NIL)
		delay_string_append_char(message, str, "NOTHING");
	else if (list_length(node->actions) == 1)
		_rewriteNode(BaseSelect, message, dblink, str, linitial(node->actions));
	else
	{
		ListCell *lc;
		char semi = 0;

		delay_string_append_char(message, str, "(");

		foreach (lc, node->actions)
		{
			if (semi == 0)
				semi = 1;
			else
				delay_string_append_char(message, str, ";");

			_rewriteNode(BaseSelect, message, dblink, str, lfirst(lc));
		}

		delay_string_append_char(message, str, ")");
	}
}

static void
_rewriteViewStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ViewStmt *node)
{
	if (node->replace)
		delay_string_append_char(message, str, "CREATE OR REPLACE ");
	else
		delay_string_append_char(message, str, "CREATE ");

	if (node->view->relpersistence == TRUE)
		delay_string_append_char(message, str, "TEMP ");

	delay_string_append_char(message, str, "VIEW ");
	_rewriteNode(BaseSelect, message, dblink, str, node->view);

	if (node->aliases)
	{
		delay_string_append_char(message, str, "(");
		_rewriteIdList(BaseSelect, message, dblink, str, node->aliases);
		delay_string_append_char(message, str, ")");
	}

	delay_string_append_char(message, str, " AS");
	_rewriteNode(BaseSelect, message, dblink, str, node->query);
}

static void
_rewriteCreatedbOptList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *options)
{
	ListCell *lc;

	foreach (lc, options)
	{
		DefElem *e = lfirst(lc);
		Value *v = (Value *) e->arg;
		int sconst = false;

		/* keyword */
		if (strcmp(e->defname, "template") == 0)
			delay_string_append_char(message, str, " TEMPLATE ");
		else if (strcmp(e->defname, "location") == 0)
		{
			delay_string_append_char(message, str, " LOCATION ");
			sconst = true;
		}
		else if (strcmp(e->defname, "tablespace") == 0)
			delay_string_append_char(message, str, " TABLESPACE ");
		else if (strcmp(e->defname, "encoding") == 0)
		{
			delay_string_append_char(message, str, " ENCODING ");
			sconst = true;
		}
		else if (strcmp(e->defname, "owner") == 0)
			delay_string_append_char(message, str, " OWNER ");
		else if (strcmp(e->defname, "connectionlimit") == 0)
			delay_string_append_char(message, str, " CONNECTION LIMIT ");

		/* value */
		if (v == NULL)
			delay_string_append_char(message, str, "DEFAULT");
		else if (IsA((Node *)v, String))
		{
			delay_string_append_char(message, str, sconst ? "'" : "'");
			delay_string_append_char(message, str, v->val.str);
			delay_string_append_char(message, str, sconst ? "'" : "'");
		}
		else
		{
			char buf[16];
			snprintf(buf, 16, "%ld", v->val.ival);
			delay_string_append_char(message, str, buf);
		}
	}
}

static void
_rewriteCreatedbStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreatedbStmt *node)
{
	delay_string_append_char(message, str, "CREATE DATABASE \"");
	delay_string_append_char(message, str, node->dbname);
	delay_string_append_char(message, str, "\"");

	_rewriteCreatedbOptList(BaseSelect, message, dblink, str, node->options);
}

static void
_rewriteAlterDatabaseStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterDatabaseStmt *node)
{
	delay_string_append_char(message, str, "ALTER DATABASE \"");
	delay_string_append_char(message, str, node->dbname);
	delay_string_append_char(message, str, "\" ");

	_rewriteCreatedbOptList(BaseSelect, message, dblink, str, node->options);
}

static void
_rewriteAlterDatabaseSetStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterDatabaseSetStmt *node)
{
	delay_string_append_char(message, str, "ALTER DATABASE \"");
	delay_string_append_char(message, str, node->dbname);
	delay_string_append_char(message, str, "\" ");

	_rewriteNode(BaseSelect, message, dblink, str, node->setstmt);
}

static void
_rewriteDropdbStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropdbStmt *node)
{
	delay_string_append_char(message, str, "DROP DATABASE \"");
	delay_string_append_char(message, str, node->dbname);
	delay_string_append_char(message, str, "\"");
}

static void
_rewriteCreateDomainStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateDomainStmt *node)
{
	ListCell *lc;

	delay_string_append_char(message, str, "CREATE DOMAIN ");
	_rewriteFuncName(BaseSelect, message, dblink, str, node->domainname);
	delay_string_append_char(message, str, " ");
	_rewriteNode(BaseSelect, message, dblink, str, node->typeName);


	foreach (lc, node->constraints)
	{
		delay_string_append_char(message, str, " ");
		_rewriteNode(BaseSelect, message, dblink, str, lfirst(lc));
	}
}

static void
_rewriteAlterDomainStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterDomainStmt *node)
{
	delay_string_append_char(message, str, "ALTER DOMAIN ");
	_rewriteFuncName(BaseSelect, message, dblink, str, node->typeName);

	switch (node->subtype)
	{
		case 'T':
			if (node->def)
			{
				delay_string_append_char(message, str, " SET DEFAULT ");
				_rewriteNode(BaseSelect, message, dblink, str, node->def);
			}
			else
				delay_string_append_char(message, str, " DROP DEFAULT");
			break;

		case 'N':
			delay_string_append_char(message, str, " DROP NOT NULL");
			break;

		case 'O':
			delay_string_append_char(message, str, " SET NOT NULL");
			break;

		case 'C':
			delay_string_append_char(message, str, " ADD ");
			_rewriteNode(BaseSelect, message, dblink, str, node->def);
			break;

		case 'X':
			delay_string_append_char(message, str, " DROP CONSTRAINT \"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\"");
			if (node->behavior == DROP_CASCADE)
				delay_string_append_char(message, str, " CASCADE");
			break;
	}
}

static void
_rewriteCreateConversionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateConversionStmt *node)
{
	delay_string_append_char(message, str, "CREATE ");

	if (node->def == TRUE)
		delay_string_append_char(message, str, "DEFAULT ");

	delay_string_append_char(message, str, "CONVERSION ");

	_rewriteFuncName(BaseSelect, message, dblink, str, node->conversion_name);

	delay_string_append_char(message, str, " FOR '");
	delay_string_append_char(message, str, node->for_encoding_name);
	delay_string_append_char(message, str, "' TO '");
	delay_string_append_char(message, str, node->to_encoding_name);
	delay_string_append_char(message, str, " FROM ");
	_rewriteFuncName(BaseSelect, message, dblink, str, node->func_name);
}

static void
_rewritePrepareStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, PrepareStmt *node)
{
	delay_string_append_char(message, str, "PREPARE \"");
	delay_string_append_char(message, str, node->name);
	delay_string_append_char(message, str, "\" ");

	if (node->argtypes != NIL)
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->argtypes);
		delay_string_append_char(message, str, ") ");
	}

	delay_string_append_char(message, str, "AS ");
	_rewriteNode(BaseSelect, message, dblink, str, node->query);
}

static void
_rewriteExecuteStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ExecuteStmt *node)
{
	if (IsA(node, CreateTableAsStmt))
	{
		IntoClause *into = ((CreateTableAsStmt *)node)->into;
		RangeVar *rel = into->rel;

		delay_string_append_char(message, str, "CREATE ");
		if (rel->relpersistence == TRUE)
			delay_string_append_char(message, str, "TEMP ");
		delay_string_append_char(message, str, "TABLE ");
		_rewriteNode(BaseSelect, message, dblink, str, into->rel);
		delay_string_append_char(message, str, " AS ");
	}

	delay_string_append_char(message, str, "EXECUTE \"");
	delay_string_append_char(message, str, node->name);
	delay_string_append_char(message, str, "\" ");

	if (node->params != NIL)
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->params);
		delay_string_append_char(message, str, ")");
	}
}

static void
_rewriteLockStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, LockStmt *node)
{
	delay_string_append_char(message, str, "LOCK TABLE ");
	_rewriteNode(BaseSelect, message, dblink, str, node->relations);

	delay_string_append_char(message, str, " IN ");
	switch (node->mode)
	{
		case AccessShareLock:
			delay_string_append_char(message, str, "ACCESS SHARE ");
			break;

		case RowShareLock:
			delay_string_append_char(message, str, "ROW SHARE ");
			break;

		case RowExclusiveLock:
			delay_string_append_char(message, str, "ROW EXCLUSIVE ");
			break;

		case ShareUpdateExclusiveLock:
			delay_string_append_char(message, str, "SHARE UPDATE EXCLUSIVE ");
			break;

		case ShareLock:
			delay_string_append_char(message, str, "SHARE ");
			break;

		case ShareRowExclusiveLock:
			delay_string_append_char(message, str, "SHARE ROW EXCLUSIVE ");
			break;

		case ExclusiveLock:
			delay_string_append_char(message, str, "EXCLUSIVE ");
			break;

		case AccessExclusiveLock:
			delay_string_append_char(message, str, "ACCESS EXCLUSIVE ");
			break;
	}
	delay_string_append_char(message, str, "MODE");

	if (node->nowait == TRUE)
		delay_string_append_char(message, str, " NOWAIT");
}

static void
_rewriteOperatorArgTypes(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *args)
{
	TypeName *left, *right;

	left = linitial(args);
	right = lsecond(args);

	if (left)
		_rewriteNode(BaseSelect, message, dblink, str, left);
	else
		delay_string_append_char(message, str, "NONE");
	delay_string_append_char(message, str, ", ");
	if (right)
		_rewriteNode(BaseSelect, message, dblink, str, right);
	else
		delay_string_append_char(message, str, "NONE");
}

static void
_rewriteCommentStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CommentStmt *node)
{
	TypeName *t;
	Value *v;
	char buf[16];

	delay_string_append_char(message, str, "COMMENT ON ");

	switch (node->objtype)
	{
		case OBJECT_AGGREGATE:
			delay_string_append_char(message, str, "AGGREGATE ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->objname);
			delay_string_append_char(message, str, "(");

			t = linitial(node->objargs);
			if (t)
				_rewriteNode(BaseSelect, message, dblink, str, t);
			else
				delay_string_append_char(message, str, "*");
			delay_string_append_char(message, str, ")");
			break;

		case OBJECT_FUNCTION:
			delay_string_append_char(message, str, "FUNCTION ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->objname);
			delay_string_append_char(message, str, "(");
			_rewriteNode(BaseSelect, message, dblink, str, node->objargs);
			delay_string_append_char(message, str, ")");
			break;

		case OBJECT_OPERATOR:
			delay_string_append_char(message, str, "OPERATOR ");
			_rewriteOperatorName(BaseSelect, message, dblink, str, node->objname);
			delay_string_append_char(message, str, "(");
			_rewriteOperatorArgTypes(BaseSelect, message, dblink, str, node->objargs);
			delay_string_append_char(message, str, ")");
			break;

		case OBJECT_CONSTRAINT:
			delay_string_append_char(message, str, "CONSTRAINT \"");
			v = lsecond(node->objname);
			delay_string_append_char(message, str, v->val.str);
			delay_string_append_char(message, str, "\" ON ");
			_rewriteFuncName(BaseSelect, message, dblink, str, linitial(node->objargs));
			break;

		case OBJECT_RULE:
			delay_string_append_char(message, str, "RULE \"");
			v = lsecond(node->objname);
			delay_string_append_char(message, str, v->val.str);
			delay_string_append_char(message, str, "\" ON ");
			_rewriteFuncName(BaseSelect, message, dblink, str, linitial(node->objargs));
			break;

		case OBJECT_TRIGGER:
			delay_string_append_char(message, str, "TRIGGER \"");
			v = lsecond(node->objname);
			delay_string_append_char(message, str, v->val.str);
			delay_string_append_char(message, str, "\" ON ");
			_rewriteFuncName(BaseSelect, message, dblink, str, linitial(node->objargs));
			break;

		case OBJECT_OPCLASS:
			delay_string_append_char(message, str, "OPERATOR CLASS ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->objname);
			delay_string_append_char(message, str, " USING ");
			v = linitial(node->objargs);
			delay_string_append_char(message, str, v->val.str);
			break;

		case OBJECT_LARGEOBJECT:
			delay_string_append_char(message, str, "LARGE OBJECT ");
			v = linitial(node->objname);
			if (IsA(v, String))
				delay_string_append_char(message, str, v->val.str);
			else if (IsA(v, Integer))
			{
				snprintf(buf, 16, "%ld", v->val.ival);
				delay_string_append_char(message, str, buf);
			}
			break;

		case OBJECT_CAST:
			delay_string_append_char(message, str, "CAST (");
			_rewriteNode(BaseSelect, message, dblink, str, linitial(node->objname));
			delay_string_append_char(message, str, " AS ");
			_rewriteNode(BaseSelect, message, dblink, str, linitial(node->objargs));
			delay_string_append_char(message, str, ")");
			break;

		case OBJECT_LANGUAGE:
			delay_string_append_char(message, str, "LANGUAGE ");
			_rewriteFuncName(BaseSelect, message, dblink, str, node->objname);
			break;

		default:
			switch (node->objtype)
			{
				case OBJECT_COLUMN:
					delay_string_append_char(message, str, "COLUMN ");
					break;
				case OBJECT_DATABASE:
					delay_string_append_char(message, str, "DATABASE ");
					break;
				case OBJECT_SCHEMA:
					delay_string_append_char(message, str, "SCHEMA ");
					break;
				case OBJECT_INDEX:
					delay_string_append_char(message, str, "INDEX ");
					break;
				case OBJECT_SEQUENCE:
					delay_string_append_char(message, str, "SEQUENCE ");
					break;
				case OBJECT_TABLE:
					delay_string_append_char(message, str, "TABLE ");
					break;
				case OBJECT_DOMAIN:
					delay_string_append_char(message, str, "DOMAIN ");
					break;
				case OBJECT_TYPE:
					delay_string_append_char(message, str, "TYPE ");
					break;
				case OBJECT_VIEW:
					delay_string_append_char(message, str, "VIEW ");
					break;
				default:
					break;
			}
			_rewriteFuncName(BaseSelect, message, dblink, str, node->objname);
			break;
	}

	delay_string_append_char(message, str, " IS ");
	if (node->comment)
	{
		delay_string_append_char(message, str, "'");
		delay_string_append_char(message, str, node->comment);
		delay_string_append_char(message, str, "'");
	}
	else
		delay_string_append_char(message, str, "NULL");
}

static void
_rewriteRangeSubselect(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeSubselect *node)
{
	Assert(node && node->alias);
	int last = message->current_select;
	Alias *alias = node->alias;
	char *table_name = alias->aliasname;
	int sub_no = message->analyze_num;
	int CallFromClause = 0;
	int next = message->analyze_num;

  if(message->r_code == SELECT_AEXPR)
	{
		KeepRewriteQueryReturnCode(message, SELECT_AEXPR_FALSE);
		return;
	}

	if(message->r_code == SELECT_ANALYZE && message->fromClause)
			CallFromClause = 1;

	delay_string_append_char(message, str, "(");
	_rewriteNode(BaseSelect, message, dblink, str, node->subquery);
	delay_string_append_char(message, str, ")");

	if(message->r_code == SELECT_ANALYZE)
	{
		AnalyzeSelect *analyze;
		analyze=message->analyze[sub_no];

		if(node->alias->colnames)
		{
			ListCell   *lc;
			int num = list_length(node->alias->colnames);
			// XXXX: segfault occurs here
			int ret_num = message->analyze[last + 1]->select_ret->col_num;
			if(num == ret_num)
			{
				char **col_list;
				int i = 0;
				col_list = (char **) palloc(sizeof(char *) * num);

				foreach(lc,node->alias->colnames)
				{
					Node *n = lfirst(lc);
					Value *value = (Value *) n;
					col_list[i] = value->val.str;
					i++;
				}
				message->analyze[last+1]->select_ret->col_list = col_list;
			}
		}



		message->analyze[last]->select_range = true;

		ereport(DEBUG2,
			(errmsg("_rewriteRangeSubSelect: select range ture %d",sub_no)));
		build_range_info(message,NULL,NULL,analyze->select_ret,table_name,last,sub_no);

		/*2009/07/27*/
		if(CallFromClause)
		{
			int temp = message->current_select;
			/* now Subquery's current_select is set
			 * change the current_select
			 */
			message->current_select = last;
			build_virtual_table(message,node,next);
			message->current_select = temp;
		}
	}
	else if(message->r_code == SELECT_DEFAULT && message->ignore_rewrite == -1)
	{
		Alias *alias = (Alias *) node->alias;
		delay_string_append_char(message, str, " AS ");
		delay_string_append_char(message, str, alias->aliasname);
		delay_string_append_char(message, str, " ");

		if (alias->colnames)
		{
			AnalyzeSelect *analyze;
			int ret_num;
			int i;
			analyze = message->analyze[last + 1];
			ret_num = analyze->select_ret->col_num;
			delay_string_append_char(message, str, "(");

			for (i = 0; i< ret_num; i++)
			{
				char buf[16];
				snprintf(buf, 16, "%d", analyze->select_ret->return_list[i]);
				delay_string_append_char(message, str," \"pool_c$");
				delay_string_append_char(message, str,buf);
				delay_string_append_char(message, str,"\"");
				if(i != ret_num -1)
					delay_string_append_char(message, str,",");
			}
			delay_string_append_char(message, str, ")");
		}
	} else {
		_rewriteNode(BaseSelect, message, dblink, str, node->alias);
	}
	
}

static void
_rewriteRangeFunction(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeFunction *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->functions); //TODO
	if (node->alias)
	{
		_rewriteNode(BaseSelect, message, dblink, str, node->alias);
	}

	if (node->coldeflist)
	{
		delay_string_append_char(message, str, " (");
		_rewriteNode(BaseSelect, message, dblink, str, node->coldeflist);
		delay_string_append_char(message, str, ")");
	}
}

static void
_rewriteWithDefinition(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *def_list)
{
	int oid = 0;

	if (list_length(def_list) == 1)
	{
		DefElem *elem;

		elem = linitial(def_list);
		if (strcmp(elem->defname, "oids") == 0)
		{
			Value *v = (Value *)elem->arg;
			if (v->val.ival == 1)
				delay_string_append_char(message, str, " WITH OIDS ");
			else
				delay_string_append_char(message, str, " WITHOUT OIDS ");
			oid = 1;
		}
	}

	if (oid == 1)
		return;

	delay_string_append_char(message, str, " WITH ");
	_rewriteDefinition(BaseSelect, message, dblink, str, def_list);
}

static void
_rewriteCurrentOfExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CurrentOfExpr *node)
{
	delay_string_append_char(message, str, "CURRENT OF ");
	if (node->cursor_name == NULL)
	{
		char n[10];
		snprintf(n, sizeof(n), "$%d", node->cursor_param);
		delay_string_append_char(message, str, n);
	}
	else
		delay_string_append_char(message, str, node->cursor_name);
}

/*
 * _rewriteNode -
 *	  converts a Node into ascii string and append it to 'str'
 */
static void
_rewriteNode(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, void *obj)
{
	if(!obj)
		return;
	else if (IsA(obj, List) ||IsA(obj, IntList) || IsA(obj, OidList))
		_rewriteList(BaseSelect, message, dblink, str, obj);
	else if (IsA(obj, Integer) ||
			 IsA(obj, Float) ||
			 IsA(obj, String) ||
			 IsA(obj, BitString))
	{
		/* nodeRead does not want to see { } around these! */
		_rewriteValue(BaseSelect, message, dblink, str, obj);
	}
	else
	{
		switch (nodeTag(obj))
		{
			case T_Alias:
				_rewriteAlias(BaseSelect, message, dblink, str, obj);
				break;
			case T_RangeVar:
				_rewriteRangeVar(BaseSelect, message, dblink, str, obj);
				break;
			case T_Var:
				_rewriteVar(BaseSelect, message, dblink, str, obj);
				break;
			case T_Const:
				_rewriteConst(BaseSelect, message, dblink, str, obj);
				break;
			case T_Param:
				_rewriteParam(BaseSelect, message, dblink, str, obj);
				break;
			case T_Aggref:
				_rewriteAggref(BaseSelect, message, dblink, str, obj);
				break;
			case T_ArrayRef:
				_rewriteArrayRef(BaseSelect, message, dblink, str, obj);
				break;
			case T_FuncExpr:
				_rewriteFuncExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_NamedArgExpr:
				_rewriteNameArgExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_OpExpr:
				_rewriteOpExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_DistinctExpr:
				_rewriteDistinctExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_ScalarArrayOpExpr:
				_rewriteScalarArrayOpExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_BoolExpr:
				_rewriteBoolExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_SubLink:
				_rewriteSubLink(BaseSelect, message, dblink, str, obj);
				break;
			case T_SubPlan:
				_rewriteSubPlan(BaseSelect, message, dblink, str, obj);
				break;
			case T_FieldSelect:
				_rewriteFieldSelect(BaseSelect, message, dblink, str, obj);
				break;
			case T_FieldStore:
				_rewriteFieldStore(BaseSelect, message, dblink, str, obj);
				break;
			case T_RelabelType:
				_rewriteRelabelType(BaseSelect, message, dblink, str, obj);
				break;
			case T_ConvertRowtypeExpr:
				_rewriteConvertRowtypeExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_CaseExpr:
				_rewriteCaseExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_CaseWhen:
				_rewriteCaseWhen(BaseSelect, message, dblink, str, obj);
				break;
			case T_CaseTestExpr:
				_rewriteCaseTestExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_ArrayExpr:
				_rewriteArrayExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_RowExpr:
				_rewriteRowExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_CoalesceExpr:
				_rewriteCoalesceExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_MinMaxExpr:
				_rewriteMinMaxExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_NullIfExpr:
				_rewriteNullIfExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_NullTest:
				_rewriteNullTest(BaseSelect, message, dblink, str, obj);
				break;
			case T_BooleanTest:
				_rewriteBooleanTest(BaseSelect, message, dblink, str, obj);
				break;
			case T_CoerceToDomain:
				_rewriteCoerceToDomain(BaseSelect, message, dblink, str, obj);
				break;
			case T_CoerceToDomainValue:
				_rewriteCoerceToDomainValue(BaseSelect, message, dblink, str, obj);
				break;
			case T_SetToDefault:
				_rewriteSetToDefault(BaseSelect, message, dblink, str, obj);
				break;
			case T_TargetEntry:
				_rewriteTargetEntry(BaseSelect, message, dblink, str, obj);
				break;
			case T_RangeTblRef:
				_rewriteRangeTblRef(BaseSelect, message, dblink, str, obj);
				break;
			case T_JoinExpr:
				_rewriteJoinExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_FromExpr:
				_rewriteFromExpr(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateStmt:
				_rewriteCreateStmt(BaseSelect, message, dblink, str, obj);
				break;
			case T_IndexStmt:
				_rewriteIndexStmt(BaseSelect, message, dblink, str, obj);
				break;
			case T_NotifyStmt:
				_rewriteNotifyStmt(BaseSelect, message, dblink, str, obj);
				break;
			case T_DeclareCursorStmt:
				_rewriteDeclareCursorStmt(BaseSelect, message, dblink, str, obj);
				break;
			case T_SelectStmt:
					_rewriteSelectStmt(BaseSelect, message, dblink, str, obj);
				break;
			case T_ColumnDef:
				_rewriteColumnDef(BaseSelect, message, dblink, str, obj);
				break;
			case T_TypeName:
				_rewriteTypeName(BaseSelect, message, dblink, str, obj);
				break;
			case T_TypeCast:
				_rewriteTypeCast(BaseSelect, message, dblink, str, obj);
				break;
			case T_IndexElem:
				_rewriteIndexElem(BaseSelect, message, dblink, str, obj);
				break;
			case T_SortGroupClause:
				_rewriteSortGroupClause(BaseSelect, message, dblink, str, obj);
				break;
			case T_WindowClause:
				_rewriteWindowClause(BaseSelect, message, dblink, str, obj);
				break;
			case T_SetOperationStmt:
				_rewriteSetOperationStmt(BaseSelect, message, dblink, str, obj);
				break;
/*			case T_RangeTblEntry:
				_rewriteRangeTblEntry(BaseSelect, message, dblink, str, obj);
				break;*/
			case T_A_Expr:
				_rewriteAExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_ColumnRef:
				_rewriteColumnRef(BaseSelect, message, dblink, str, obj);
				break;
			case T_ParamRef:
				_rewriteParamRef(BaseSelect, message, dblink, str, obj);
				break;
			case T_A_Const:
				_rewriteAConst(BaseSelect, message, dblink, str, obj);
				break;
			case T_A_Indices:
				_rewriteA_Indices(BaseSelect, message, dblink, str, obj);
				break;
 			case T_A_ArrayExpr:
				_rewriteA_ArrayExpr(BaseSelect, message, dblink, str, obj);
				break;
			case T_A_Indirection:
				_rewriteA_Indirection(BaseSelect, message, dblink, str, obj);
				break;
			case T_ResTarget:
				_rewriteResTarget(BaseSelect, message, dblink, str, obj);
				break;
			case T_Constraint:
				_rewriteConstraint(BaseSelect, message, dblink, str, obj);
				break;
			case T_FuncCall:
				_rewriteFuncCall(BaseSelect, message, dblink, str, obj);
				break;
			case T_DefElem:
				_rewriteDefElem(BaseSelect, message, dblink, str, obj);
				break;
			case T_LockingClause:
				_rewriteLockingClause(BaseSelect, message, dblink, str, obj);
				break;

			case T_SortBy:
				_rewriteSortBy(BaseSelect, message, dblink, str, obj);
				break;

			case T_InsertStmt:
				_rewriteInsertStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_UpdateStmt:
				_rewriteUpdateStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_DeleteStmt:
				_rewriteDeleteStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_TransactionStmt:
				_rewriteTransactionStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_TruncateStmt:
				_rewriteTruncateStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_VacuumStmt:
				_rewriteVacuumStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_ExplainStmt:
				_rewriteExplainStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_ClusterStmt:
				_rewriteClusterStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CheckPointStmt:
				_rewriteCheckPointStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_ClosePortalStmt:
				_rewriteClosePortalStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_ListenStmt:
				_rewriteListenStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_UnlistenStmt:
				_rewriteUnlistenStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_LoadStmt:
				_rewriteLoadStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CopyStmt:
				_rewriteCopyStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_DeallocateStmt:
				_rewriteDeallocateStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_RenameStmt:
				_rewriteRenameStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateRoleStmt:
				_rewriteCreateRoleStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterRoleStmt:
				_rewriteAlterRoleStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterRoleSetStmt:
				_rewriteAlterRoleSetStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_DropRoleStmt:
				_rewriteDropRoleStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateSchemaStmt:
				_rewriteCreateSchemaStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_VariableSetStmt:
				_rewriteVariableSetStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_VariableShowStmt:
				_rewriteVariableShowStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_ConstraintsSetStmt:
				_rewriteConstraintsSetStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterTableStmt:
				_rewriteAlterTableStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterTableCmd:
				_rewriteAlterTableCmd(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateSeqStmt:
				_rewriteCreateSeqStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterSeqStmt:
				_rewriteAlterSeqStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreatePLangStmt:
				_rewriteCreatePLangStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateTableSpaceStmt:
				_rewriteCreateTableSpaceStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_DropTableSpaceStmt:
				_rewriteDropTableSpaceStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateTrigStmt:
				_rewriteCreateTrigStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_DefineStmt:
				_rewriteDefineStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateOpClassStmt:
				_rewriteCreateOpClassStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateOpClassItem:
				_rewriteCreateOpClassItem(BaseSelect, message, dblink, str, obj);
				break;

			case T_DropStmt:
				_rewriteDropStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_FetchStmt:
				_rewriteFetchStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_GrantStmt:
				_rewriteGrantStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_FuncWithArgs:
				_rewriteFuncWithArgs(BaseSelect, message, dblink, str, obj);
				break;

			case T_FunctionParameter:
				_rewriteFunctionParameter(BaseSelect, message, dblink, str, obj);
				break;

			case T_PrivGrantee:
				_rewritePrivGrantee(BaseSelect, message, dblink, str, obj);
				break;

			case T_GrantRoleStmt:
				_rewriteGrantRoleStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateFunctionStmt:
				_rewriteCreateFunctionStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterFunctionStmt:
				_rewriteAlterFunctionStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateCastStmt:
				_rewriteCreateCastStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_ReindexStmt:
				_rewriteReindexStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterObjectSchemaStmt:
				_rewriteAlterObjectSchemaStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterOwnerStmt:
				_rewriteAlterOwnerStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_RuleStmt:
				_rewriteRuleStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_ViewStmt:
				_rewriteViewStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreatedbStmt:
				_rewriteCreatedbStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterDatabaseStmt:
				_rewriteAlterDatabaseStmt(BaseSelect, message, dblink, str, obj);
				break;


			case T_AlterDatabaseSetStmt:
				_rewriteAlterDatabaseSetStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_DropdbStmt:
				_rewriteDropdbStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateDomainStmt:
				_rewriteCreateDomainStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_AlterDomainStmt:
				_rewriteAlterDomainStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateConversionStmt:
				_rewriteCreateConversionStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_PrepareStmt:
				_rewritePrepareStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_ExecuteStmt:
				_rewriteExecuteStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_LockStmt:
				_rewriteLockStmt(BaseSelect, message, dblink, str, obj);
				break;
			case T_CommentStmt:
				_rewriteCommentStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_RangeSubselect:
				_rewriteRangeSubselect(BaseSelect, message, dblink, str, obj);
				break;

			case T_RangeFunction:
				_rewriteRangeFunction(BaseSelect, message, dblink, str, obj);
				break;

			case T_CurrentOfExpr:
				_rewriteCurrentOfExpr(BaseSelect, message, dblink, str, obj);
				break;

			case T_DiscardStmt:
			case T_CreateOpFamilyStmt:
			case T_AlterOpFamilyStmt:
			case T_CreateEnumStmt:
			case T_DropOwnedStmt:
			case T_ReassignOwnedStmt:
			case T_AlterTSDictionaryStmt:
			case T_AlterTSConfigurationStmt:
			case T_XmlSerialize:
				break;

			default:
				KeepRewriteQueryCode(message, SELECT_RELATION_ERROR);
				break;
		}
	}
}


/*
 * nodeToString -
 *	   returns the ascii representation of the Node as a palloc'd string
 */
void
nodeToRewriteString(RewriteQuery *message, ConInfoTodblink *dblink, void *obj)
{
	String *str;

	str = init_string("");
	message->analyze_num = 0;
	_rewriteNode(NULL, message, dblink, str, obj);
	message->rewrite_query = str->data;
}
