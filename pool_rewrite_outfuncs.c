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
#include "parser/parser.h"
#include "parser/pool_string.h"
#include "parser/pg_list.h"
#include "parser/parsenodes.h"
#include "pool_rewrite_query.h"

#define booltostr(x)  ((x) ? "true" : "false")

static void KeepRewriteQueryCode(RewriteQuery *message, int r_code);

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
static void _rewriteSortClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SortClause *node);
static void _rewriteGroupClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, GroupClause *node);
static void _rewriteSetOperationStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SetOperationStmt *node);
static void _rewriteAExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Expr *node);
static void _rewriteValue(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Value *value);
static void _rewriteColumnRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ColumnRef *node);
static void _rewriteParamRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ParamRef *node);
static void _rewriteAConst(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Const *node);
static void _rewriteA_Indices(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Indices *node);
static void _rewriteA_Indirection(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Indirection *node);
static void _rewriteResTarget(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ResTarget *node);
static void _rewriteConstraint(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Constraint *node);
static void _rewriteFkConstraint(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FkConstraint *node);

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
static void _rewriteDropPLangStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropPLangStmt *node);
static void _rewriteCreateTableSpaceStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateTableSpaceStmt *node);
static void _rewriteDropTableSpaceStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropTableSpaceStmt *node);
static void _rewriteCreateTrigStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateTrigStmt *node);
static void _rewriteDropPropertyStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropPropertyStmt *node);
static void _rewriteDefineStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DefineStmt *node);
static void _rewriteCreateOpClassStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateOpClassStmt *node);
static void _rewriteRemoveOpClassStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RemoveOpClassStmt *node);
static void _rewriteDropStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropStmt *node);
static void _rewriteFetchStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FetchStmt *node);
static void _rewriteGrantStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, GrantStmt *node);
static void _rewriteGrantRoleStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, GrantRoleStmt *node);
static void _rewriteCreateFunctionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateFunctionStmt *node);
static void _rewriteAlterFunctionStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, AlterFunctionStmt *node);
static void _rewriteRemoveFuncStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RemoveFuncStmt *node);
static void _rewriteCreateCastStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateCastStmt *node);
static void _rewriteDropCastStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropCastStmt *node);
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
static void _rewritePrivTarget(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, PrivTarget *node);
static void _rewriteFuncWithArgs(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncWithArgs *node);
static void _rewriteFunctionParameter(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FunctionParameter *node);
static void _rewritePrivilegeList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);
static void _rewriteFuncOptList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);
static void _rewriteCreatedbOptList(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *options);
static void _rewriteOperatorArgTypes(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *args);
static void _rewriteRangeFunction(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeFunction *node);
static void _rewriteWithDefinition(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *def_list);

/* use optimization of pool_parallel */
static int checkFuncArgs(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);
static int _rewriteWhereClause(Node *BaseSelect,RewriteQuery *message,ConInfoTodblink *dblink,String *str,int true_count);
static int checkSelectStmtOneTable(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node);
static int checkIntensiveOneTable(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node);
static void compareGroupby(RewriteQuery *message,List *targetList,List *groupby,String *str,int record);
static void _rewriteIntensivedblink(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list);
static void _rewriteReturnRecords(Node *BaseSelect,RewriteQuery *message,List *list,String *str);
static void _rewriteIntensiveTargetList(RewriteQuery *message,List *list,String *str);
static int _checkDistDefColumn(ColumnRef *col,RewriteQuery *message);
static int _writewhereClause(A_Expr *expr,RewriteQuery *message,ConInfoTodblink *dblink, String *str,int true_count);
static void _rewriteSelectStmtIntensive(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node);
static char *escape_string(char *str);
static void delay_string_append_char(RewriteQuery *message,String *str, char *parts);
static int checkGroupbyClause(List *list, char *colname);

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
	if(message->r_code == SELECT_DEFAULT || 
		message->r_code == SELECT_DEFAULT_INSIDE_DBLINK ||
		message->r_code == SELECT_ONETABLE)
		string_append_char(str, parts);
}

/*
 *  check Group by Clause
 *  if return value is zero, 
 *  cannot do intensive optimization.
 */ 
static int
checkGroupbyClause(List *list, char *colname)
{
	ListCell *lc;
	int ok_group_by = 0;
	foreach(lc ,list)
	{
		Node *n = lfirst(lc);
		if(!IsA(n,ColumnRef))
			return 0;

		if (IsA(n, ColumnRef))
		{
			ColumnRef *col = NULL;
			col = (ColumnRef *) n;
			if(colname && 
				strcmp(colname, strVal(lfirst(list_head(col->fields)))) == 0)
			{
				ok_group_by++;
				return ok_group_by;
			} else if (!colname) {
				ok_group_by++;	
			}			
		}
	}
	return ok_group_by;
}

/* 
 * Include function,sub-query
 * and other table member in function args? 
 */
int 
checkFuncArgs(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list)
{
	void *obj;
	int message_r_code = message->r_code;	
	if(!list)
		return 0;

	if(IsA(lfirst(list_head(list)), TypeCast))
	{
		TypeCast *type = (TypeCast *) lfirst(list_head(list));
		obj = type->arg;
	} else {
		obj = lfirst(list_head(list));
	}

	if(IsA(obj,ColumnRef))
	{
		ColumnRef *col = (ColumnRef *) obj;
		KeepRewriteQueryCode(message, SELECT_AEXPR);
		_rewriteNode(NULL, message, NULL, str, col);
		if(message->r_code == SELECT_AEXPR)
		{
			KeepRewriteQueryCode(message, message_r_code);
			return 1;
		} else {
			KeepRewriteQueryCode(message, message_r_code);
			return 0;
		}
	}
	else if(IsA(obj,A_Expr))
	{
		A_Expr *expr = (A_Expr *) obj; 
		if(expr->kind != AEXPR_OP)
			return 0;
		
		if (list_length(expr->name) == 1)
		{
			KeepRewriteQueryCode(message, SELECT_AEXPR);
			_rewriteNode(NULL, message, NULL, str, expr->lexpr);
			if(message->r_code == SELECT_AEXPR)
			{
				_rewriteNode(NULL, message, NULL, str, expr->rexpr);
				if(message->r_code == SELECT_AEXPR)
				{
					_rewriteNode(NULL, message, NULL, str, expr->rexpr);
					KeepRewriteQueryCode(message, message_r_code);
					return 1;
				} else {
					KeepRewriteQueryCode(message, message_r_code);
					return 0;
				}
			} else {
				KeepRewriteQueryCode(message, message_r_code);
				return 0;
			}
		}
		else
		{
			KeepRewriteQueryCode(message, message_r_code);
			return 0;
		}
	} else { 
		return 0;
	}
}

/* 
 * if can Intensive Optimization, 
 * return value is 1
 */
int 
checkIntensiveOneTable(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node)
{
	RangeVar *var = NULL;
	int message_r_code = message->r_code;

	/* Intensive optimization can do when tree have only one table */
	if(!((list_length(node->fromClause) == 1)
            && IsA(lfirst(list_head(node->fromClause)),RangeVar)))
	{
		return 0;
	}

	/* check groupClause's tree has only Column's list */
	if(node->groupClause && 
			checkGroupbyClause(node->groupClause, NULL) == 0)
	{
		return 0;
	}

	/* TODO: can use intensive optimization if having Clause exits */ 
	if (node->havingClause)
		return 0;
	
	var = (RangeVar *) lfirst(list_head(node->fromClause));
		
	/* set dblink info to message */
	message->table_relname = var->relname;
	if(var->schemaname)
		message->schemaname = var->schemaname;
	else
		message->schemaname = NULL;

	if(var->alias)
	{
		Alias *alias_p = (Alias *) var->alias;
		message->table_alias = alias_p->aliasname;
	} else {
		message->table_alias = NULL;
	}
	message->dbname = dblink->dbname;

	/* search target list */	
	if(node->targetList)
	{
		ListCell *lc;
		foreach (lc, node->targetList)
        {
			ResTarget *target = NULL;
			Node *n = lfirst(lc);
			void *obj;
	
			if (!IsA(n, ResTarget))
			{
				return 0;
			}
			
			target = (ResTarget *) n;

			if(!target->val)
			{
				return 0;
			}
			/* check type cast */
			if(IsA(target->val, TypeCast))
			{
				TypeCast *type = (TypeCast *) target->val;
				obj = type->arg;
			} else {
				obj = target->val;
			}

			/* check function,  max,min,count and sum can do */
			if (obj && (IsA(obj, FuncCall)))
			{
				FuncCall *func = (FuncCall *) obj;
				char *funcname = strVal(lfirst(list_head(func->funcname)));
 
				if(strcmp(funcname,"max") == 0 || strcmp(funcname,"min") == 0)
				{
					/* check function args: args must not be function or sub-query */ 
					if(checkFuncArgs(BaseSelect,message,dblink,str,func->args) == 0)
						return 0;
				} 
				else if (strcmp(funcname,"count") == 0 || strcmp(funcname,"sum") == 0)
				{
					/* check function args: args must not be function or sub-query */ 
					if(checkFuncArgs(BaseSelect,message,dblink,str,func->args) == 0 && func->agg_star != TRUE)
					{
						return 0;
					}
				} else {
					return 0;
				}
			}
			/* check ColumnRef */
			else if (obj && (IsA(obj, ColumnRef)))
			{
				ColumnRef *col = NULL;
				col = (ColumnRef *) obj;
				/* if ColumnRef is exits, groupCluase's tree must be exits */
				if(!node->groupClause)
				{
					return 0;
				} 
				else 
				{
					/* Is this Column member of this table ? */
					 KeepRewriteQueryCode(message, SELECT_AEXPR);
			        _rewriteNode(NULL, message, NULL, str, col);

					if(message->r_code != SELECT_AEXPR)
					{
						KeepRewriteQueryCode(message, message_r_code);
						return 0;
					}

					KeepRewriteQueryCode(message, message_r_code);
					
					/* Is this Column member in Groupby ? */
					if(checkGroupbyClause(node->groupClause, strVal(lfirst(list_head(col->fields)))) == 0)
						return 0;
				}
			} else {
				return 0;
			}
		}
	}
	return 1;
}

/*
 * compare targetList to groupbylist
 */
static void
compareGroupby(RewriteQuery *message,List *targetList,List *groupby,String *str,int record)
{
    ListCell *lc_groupby;
    int counter = 0;

    foreach(lc_groupby,groupby)
    {
        char *colname;
        Node *n = lfirst(lc_groupby);
        ColumnRef *col = NULL;
        ListCell *lc_targetList;
        int write_flag =0;

        col = (ColumnRef *) n;
        colname = strVal(lfirst(list_head(col->fields)));
        foreach(lc_targetList,targetList)
        {
            ResTarget *target = NULL;
            ColumnRef *col = NULL;
            Node *n = lfirst(lc_targetList);
            target = (ResTarget *) n;
            if (!(target->val && (IsA(target->val, ColumnRef))))
                continue;
            col = (ColumnRef *) target->val;

            if(strcmp(colname,strVal(lfirst(list_head(col->fields)))) == 0);
            {
                write_flag = 1;
                break;
            }
        }

		if(write_flag == 0)
		{
			if(record == 0)
			{
				string_append_char(str, " ,");
				string_append_char(str, colname);
				string_append_char(str, " ");
			} else {
				DistDefInfo *info = pool_get_dist_def_info(message->dbname, message->schemaname, message->table_relname);
				int exist_col =  0;
				int i;
				string_append_char(str, " ,");
				string_append_char(str, colname);
				string_append_char(str, " ");

				if(!info)
				{
					message->r_code = SELECT_RELATION_ERROR;
					return;
				}

				for(i = 0; i < info->col_num; i++)
				{
					if(strcmp(info->col_list[i],colname) == 0)
					{
						delay_string_append_char(message, str, " ");
						delay_string_append_char(message, str, info->type_list[i]);
						exist_col = 1;
						break;
					}
				}
				if(exist_col != 1)
				{
					message->r_code = SELECT_RELATION_ERROR;
					return;	
				}
			}
		}
		counter++;
	}
}

static void
_rewriteIntensivedblink(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, List *list)
{
	ListCell *lc;
	int list_num = 1;
	SelectStmt *select = (SelectStmt *)BaseSelect;

	foreach(lc,list)
	{
		ResTarget *target = NULL;
		char *typecast = NULL;
		Node *n = lfirst(lc);
		void *obj;

		target = (ResTarget *) n;

		if(IsA(target->val, TypeCast))
		{
			TypeCast *type = (TypeCast *) target->val;
			TypeName *typename = (TypeName *)type->typename;
			obj = type->arg;
			typecast = strVal(lfirst(list_head(typename->names)));
		} else {
			obj = target->val;
		}

		if (obj && (IsA(obj, FuncCall)))
		{
			FuncCall *func = (FuncCall *) obj;
			_rewriteNode(BaseSelect, message, dblink, str, func);
		}
		else if (target->val && (IsA(target->val, ColumnRef)))
		{
			ColumnRef *col = NULL;
			col = (ColumnRef *) target->val;
			_rewriteNode(BaseSelect, message, dblink, str, col);
		}

		if(list_num != list_length(list))
			delay_string_append_char(message,str, ",");
		list_num++;
	}
	delay_string_append_char(message,str, " ");
	if(select->groupClause)
		compareGroupby(message,list,select->groupClause,str,0);
}

/*
 * Write down records from dblink()'return
 */
static void
_rewriteReturnRecords(Node *BaseSelect,RewriteQuery *message,List *list,String *str)
{
	ListCell *lc;
	int list_first = 0;
	int args_num = 0;
	DistDefInfo *info = pool_get_dist_def_info(message->dbname, message->schemaname, message->table_relname);
	SelectStmt *select = (SelectStmt *)BaseSelect;
	ColumnRef *col = NULL;
	char *colname = NULL;

	foreach(lc,list)
	{
		ResTarget *target = NULL;
		char *typecast = NULL;
		char number[16];
		Node *n = lfirst(lc);
		void *obj;

		target = (ResTarget *) n;

		if(list_first !=0)
			delay_string_append_char(message,str, ",");
			
		if(IsA(target->val, TypeCast))
		{
			TypeCast *type = (TypeCast *) target->val;
			TypeName *typename = (TypeName *)type->typename;
			obj = type->arg;
			typecast = strVal(lfirst(list_head(typename->names)));
		} else {
			obj = target->val;
		}

		if (obj && (IsA(obj, FuncCall)))
		{
			FuncCall *func = (FuncCall *) obj;
			char *funcname = strVal(lfirst(list_head(func->funcname)));

			sprintf(number,"%d",args_num);

			if(strcmp(funcname,"max") == 0)
			{
				delay_string_append_char(message,str, " args_$");
				delay_string_append_char(message,str,number);
				delay_string_append_char(message,str, " numeric");
			}
			else if(strcmp(funcname,"min") == 0)
			{
				delay_string_append_char(message,str, " args_$");
				delay_string_append_char(message,str,number);
				delay_string_append_char(message,str, " numeric");
			}
			else if(strcmp(funcname,"sum") == 0)
			{
				delay_string_append_char(message,str, " args_$");
				delay_string_append_char(message,str,number);
				delay_string_append_char(message,str, " numeric");
			}
			else if(strcmp(funcname,"count") == 0)
			{
				delay_string_append_char(message,str, " args_$");
				delay_string_append_char(message,str,number);
				delay_string_append_char(message,str, " bigint");
			}
			args_num++;
		}
		else if (target->val && (IsA(target->val, ColumnRef)))
		{
			int i;
			int exist_col = 0;
			col = (ColumnRef *) target->val;
			colname = strVal(lfirst(list_head(col->fields)));
			delay_string_append_char(message,str,strVal(lfirst(list_head(col->fields))));

			if(!info)
			{
				message->r_code = SELECT_RELATION_ERROR;
				return;
			}

			for(i = 0; i < info->col_num; i++)
			{
				if(strcmp(info->col_list[i],colname) == 0)
				{
					delay_string_append_char(message, str, " ");
					delay_string_append_char(message, str, info->type_list[i]);
					exist_col = 1;
					break;
				}
			}
			if(exist_col != 1)
			{
				message->r_code = SELECT_RELATION_ERROR;
				return;
			}
		}	
		++list_first;
	}
	if(select->groupClause)
		compareGroupby(message,list,select->groupClause,str,1);

	delay_string_append_char(message,str, " ");
}

/*
 * Write down targetList in outer dblink.
 */
static void
_rewriteIntensiveTargetList(RewriteQuery *message,List *list,String *str)
{
	ListCell *lc;
	int list_num = 1;
	int args_num = 0;

	foreach(lc,list)
	{
		ResTarget *target = NULL;
		char *typecast = NULL;
		char number[16];
		Node *n = lfirst(lc);
		void *obj;
		char *funcname = NULL;
		TypeName *typename = NULL;

		target = (ResTarget *) n;

		if(IsA(target->val, TypeCast))
		{
			TypeCast *type = (TypeCast *) target->val;
			typename = (TypeName *)type->typename;
			obj = type->arg;
			typecast = strVal(lfirst(list_head(typename->names)));
		} else {
			obj = target->val;
		}

		if (obj && (IsA(obj, FuncCall)))
		{
			FuncCall *func = (FuncCall *) obj;
			funcname = strVal(lfirst(list_head(func->funcname)));

			sprintf(number,"%d",args_num);

			if(strcmp(funcname,"max") == 0)
			{
				delay_string_append_char(message,str, " max(args_$");
			}
			else if(strcmp(funcname,"min") == 0)
			{
				delay_string_append_char(message,str, " min(args_$");
			}
			else if(strcmp(funcname,"sum") == 0)
			{
				delay_string_append_char(message,str, " sum(args_$");
			}
			else if(strcmp(funcname,"count") == 0)
			{
				delay_string_append_char(message,str, " sum(args_$");
			}
			delay_string_append_char(message,str,number);
			delay_string_append_char(message,str, ")");
			args_num++;
		}
		else if (target->val && (IsA(target->val, ColumnRef)))
		{
			ColumnRef *col = NULL;
			col = (ColumnRef *) target->val;
			delay_string_append_char(message,str,strVal(lfirst(list_head(col->fields))));
		}

		if(typename)
		{
			delay_string_append_char(message,str, "::");
			_rewriteTypeName(NULL,message,NULL, str, typename);
		}

		if(target->name)
		{
			delay_string_append_char(message,str, " AS ");
			delay_string_append_char(message,str, target->name);
			delay_string_append_char(message,str, " ");
		} 
		else if(funcname && !target->name && (strcmp(funcname,"count") == 0))
		{
			delay_string_append_char(message,str, " AS ");
			delay_string_append_char(message,str, "COUNT");
			delay_string_append_char(message,str, " ");
		} 
		if(list_num != list_length(list))
			delay_string_append_char(message,str, ",");
		list_num++;
	}
	delay_string_append_char(message,str, " ");
}

/*
 * check select stmt
 * this function check fromClause and where 
 */ 
int 
checkSelectStmtOneTable(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node)
{
	RangeVar *var = NULL;
	int use_select_onetable = -1;

	if(node->larg || node->all || node->rarg || node->intoClause ||
	   node->distinctClause || node->havingClause)
	{
		return 0;
	}

	if(message->r_code == SELECT_AEXPR)
	{
		KeepRewriteQueryCode(message, SELECT_AEXPR_FALSE);
		return 0;
	}

	KeepRewriteQueryCode(message, SELECT_ONETABLE);

	if(node->fromClause == NULL)
		return 0;
	
	if(!((list_length(node->fromClause) == 1)
	    && list_head(node->fromClause) 
		&& lfirst(list_head(node->fromClause)) 
		&& IsA(lfirst(list_head(node->fromClause)),RangeVar)))
	{
		return 0;
	}
	
	var = (RangeVar *) lfirst(list_head(node->fromClause));
		

	message->table_relname = var->relname;
	if(var->schemaname)
		message->schemaname = var->schemaname;
	else
		message->schemaname = NULL;

	if(var->alias)
	{
		Alias *alias_p = (Alias *) var->alias;
		message->table_alias = alias_p->aliasname;
	} else
		message->table_alias = NULL;
	
	message->dbname = dblink->dbname;

	if (node->whereClause)
	{
		int true_count = 0;
		KeepRewriteQueryCode(message, SELECT_AEXPR);
		true_count = _rewriteWhereClause(node->whereClause, message,dblink,str,true_count);
		if(true_count == 0)
			use_select_onetable = 1; 
	} else {
			use_select_onetable = 1; 	
	}

	return use_select_onetable;
}

/*
 *  if message->r_code is SELECT_RELATION_ERROR
 *  or SELECT_PGCATALOG, don't change message->r_code
 */
static void KeepRewriteQueryCode(RewriteQuery *message, int r_code)
{
	if((message->r_code != SELECT_RELATION_ERROR) 
			&& (message->r_code != SELECT_PGCATALOG))
	{
		message->r_code = r_code;
	}
}

/*
 * Is this column  member of table (message->table_relname) ?
 * return 1 -- member
 * return 0 or -1 -- not mmeber 
 */
static int
_checkDistDefColumn(ColumnRef *col,RewriteQuery *message)
{
	ListCell *c;
	List *list;
	char first = 0;
	char *tmp_table = NULL;
	char *colname = NULL;
	DistDefInfo *info = NULL;
	int i;
	int check_col = 0;

	list = col->fields;

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

	/* alias check */
	if(message->table_alias)
	{
		if(tmp_table && strcmp(message->table_alias,tmp_table) != 0)
		{
			return check_col;
		}
	} 
	else if (!message->table_alias && message->table_relname)
	{
		if(tmp_table && strcmp(message->table_relname,tmp_table) != 0)
		{
			return check_col;
		}
	} 
	else
	{
			return check_col;
	}
	
	/* get dist_def_info */
	info = pool_get_dist_def_info(message->dbname, message->schemaname, message->table_relname);

	if(!info)
	{
		/* send error message */
		return -1;
	}

	for(i = 0; i < info->col_num; i++)
	{
		if(strcmp(info->col_list[i],colname) == 0)
		{
			check_col = 1;
		}

	}

	return check_col;
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
				KeepRewriteQueryCode(message, SELECT_AEXPR);
				_rewriteNode(NULL, message, dblink, str, expr->lexpr);
				if(message->r_code == SELECT_AEXPR)
				{
					_rewriteNode(NULL, message, dblink, str, expr->rexpr);
					if(message->r_code == SELECT_AEXPR)
					{
						Value *op = (Value *) lfirst(list_head(expr->name));
						KeepRewriteQueryCode(message, message_r_code);
						_rewriteNode(NULL, message, dblink, str, expr->lexpr);
						delay_string_append_char(message, str, op->val.str);
						_rewriteNode(NULL, message, dblink, str, expr->rexpr);
						KeepRewriteQueryCode(message, message_r_code);
					} else {
						KeepRewriteQueryCode(message, message_r_code);
						delay_string_append_char(message, str,"TRUE");
						true_count++;
						break;
					}
				} else {
					KeepRewriteQueryCode(message, message_r_code);
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
			true_count = true_count + _rewriteWhereClause(expr->lexpr,message,dblink,str,true_count);
			delay_string_append_char(message, str, " AND ");
			true_count = true_count + _rewriteWhereClause(expr->rexpr,message,dblink,str,true_count);
			delay_string_append_char(message, str, ")");
			break;

		case AEXPR_OR:
			delay_string_append_char(message, str, " (");
			true_count = true_count + _rewriteWhereClause(expr->lexpr,message,dblink,str,true_count);
			delay_string_append_char(message, str, " OR ");
			true_count = true_count +  _rewriteWhereClause(expr->rexpr,message,dblink,str,true_count);
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
			_rewriteWhereClause(expr->rexpr,str,tablename,dbname, schemaname,aliasname);
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
_rewriteWhereClause(Node *BaseSelect,RewriteQuery *message,ConInfoTodblink *dblink,String *str,int true_count)
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

	foreach(lc, node)
	{
		if (first == 0)
			first = 1;
		else
		{
			if(!IsA(lfirst(lc),A_Indices))
				delay_string_append_char(message, str, ",");
		}
		_rewriteNode(BaseSelect, message, dblink, str, lfirst(lc));
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
_rewriteRangeVar(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeVar *node)
{
	if(message->r_code == SELECT_PGCATALOG)
		return ;

	if(message->r_code != SELECT_DEFAULT && 
	   message->r_code != SELECT_ONETABLE)
	{
		if (node->catalogname)
		{
			delay_string_append_char(message, str, "\"");
			delay_string_append_char(message, str, node->catalogname);
			delay_string_append_char(message, str, "\".");
		}

		if (node->schemaname)
		{
			delay_string_append_char(message, str, "\"");
			delay_string_append_char(message, str, node->schemaname);
			delay_string_append_char(message, str, "\".");

			if(strcmp(node->schemaname,"pg_catalog") == 0)
			{
				message->r_code = SELECT_PGCATALOG;
			}

		}

		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->relname);
		delay_string_append_char(message, str, "\"");

		if (node->alias)
			_rewriteNode(BaseSelect, message, dblink, str, node->alias);

		if (node->inhOpt == INH_YES)
		{
			delay_string_append_char(message, str, " * ");
		}
	} else {

		/* rewrite query using dblink connection */
		DistDefInfo *info;
		int i;
		char port[8];
		SelectStmt *select = (SelectStmt *)BaseSelect;

		/* 
		 * iff schemaname is pg_catalog, send query to 
		 * one node not system db.
		 */
		if(node->schemaname && 
			(strcmp(node->schemaname,"pg_catalog") == 0))
		{
			message->r_code = SELECT_PGCATALOG;
			return;
		}

		sprintf(port,"%d",dblink->port);

		if(node->alias)
			delay_string_append_char(message, str, "(SELECT * FROM dblink(");
		else
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
		delay_string_append_char(message, str, "SELECT pool_parallel(\"select * from ");

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
				message->r_code = SELECT_PGCATALOG;
			}

		}
		else
			message->schemaname = NULL;

		delay_string_append_char(message, str, node->relname);
		
		if(select->whereClause && 
			!(message->r_code == SELECT_PGCATALOG))
		{
			int message_code = message->r_code;
			if(node->alias)
			{
				Alias *alias_p = (Alias *) node->alias;
				_rewriteNode(BaseSelect, message, dblink, str, node->alias);
				delay_string_append_char(message, str, " WHERE ");
				message->table_alias = alias_p->aliasname;
			}
			else
			{
				delay_string_append_char(message, str, " WHERE ");
				message->table_alias = NULL;
			}
			message->table_relname = node->relname;
			message->dbname = dblink->dbname;
			KeepRewriteQueryCode(message, SELECT_DEFAULT_INSIDE_DBLINK);
			_rewriteWhereClause(select->whereClause, message,dblink,str,0);
			KeepRewriteQueryCode(message, message_code);
		}

		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, ")");
		delay_string_append_char(message, str, "'");
		delay_string_append_char(message, str, ")");


		delay_string_append_char(message, str," AS ");
		delay_string_append_char(message, str, node->relname);
		delay_string_append_char(message, str, "(");

		/* get dist_def_info */
		info = pool_get_dist_def_info (dblink->dbname, node->schemaname, node->relname);

		if(!info)
		{
			message->r_code = SELECT_RELATION_ERROR;
			return;
		}

		for(i = 0; i < info->col_num; i++)
		{
			delay_string_append_char(message, str, info->col_list[i]);
			delay_string_append_char(message, str, " ");
			delay_string_append_char(message, str, info->type_list[i]);
			if (i != info->col_num -1)
				delay_string_append_char(message, str, ",");
		}
		delay_string_append_char(message, str, ")");

		if(node->alias)
			delay_string_append_char(message, str, ")");
		
		if(node->alias)
		{
			_rewriteNode(BaseSelect, message, dblink, str, node->alias);
		}

		if (node->inhOpt == INH_YES)
		{
			delay_string_append_char(message, str, " * ");
		}
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
		KeepRewriteQueryCode(message, SELECT_AEXPR_FALSE);
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
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->subselect);
		delay_string_append_char(message, str, ")");
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

static void
_rewriteJoinExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, JoinExpr *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->larg);

	if (node->isNatural == TRUE)
		delay_string_append_char(message, str, " NATURAL");

	if (node->jointype == JOIN_INNER)
	{
		if (node->using == NIL && node->quals == NULL && !node->isNatural)
			delay_string_append_char(message, str, " CROSS JOIN ");
		else
			delay_string_append_char(message, str, " JOIN ");
	}
	else if (node->jointype == JOIN_INNER)
		delay_string_append_char(message, str, " JOIN ");
	else if (node->jointype == JOIN_LEFT)
		delay_string_append_char(message, str, " LEFT OUTER JOIN ");
	else if (node->jointype == JOIN_FULL)
		delay_string_append_char(message, str, " FULL OUTER JOIN ");
	else if (node->jointype == JOIN_RIGHT)
		delay_string_append_char(message, str, " RIGHT OUTER JOIN ");

	_rewriteNode(BaseSelect, message, dblink, str, node->rarg);

	if (node->using != NIL && IsA(node->using, List))
	{
		ListCell *lc;
		char comma = 0;

		delay_string_append_char(message, str, " USING(");

		foreach (lc, node->using)
		{
			Value *value;

			if (comma == 0)
				comma = 1;
			else
				delay_string_append_char(message, str, ",");
			
			value = lfirst(lc);
			delay_string_append_char(message, str, "\"");
			delay_string_append_char(message, str, value->val.str);
			delay_string_append_char(message, str, "\"");
		}
		
		delay_string_append_char(message, str, ")");
	}

	if (node->quals)
	{
		delay_string_append_char(message, str, " ON ");
		_rewriteNode(BaseSelect, message, dblink, str, node->quals);
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
	if (node->relation->istemp)
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
		delay_string_append_char(message, str, "INDEX \"");		
	delay_string_append_char(message, str, node->idxname);
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
	_rewriteNode(BaseSelect, message, dblink, str, node->relation);
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
_rewriteSelectStmtIntensive(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node)
{
	int message_r_code = message->r_code;
	RangeVar *var = NULL;
	char port[8];

	BaseSelect = (Node *) node;

	/* set table*/
	var = (RangeVar *) lfirst(list_head(node->fromClause));
	
	delay_string_append_char(message, str, " SELECT ");
	_rewriteIntensiveTargetList(message,node->targetList,str);
	delay_string_append_char(message, str, " FROM ");

	sprintf(port,"%d",dblink->port);

	if(var->alias)
		delay_string_append_char(message, str, " (SELECT * FROM dblink(");
	else
		delay_string_append_char(message, str, " dblink(");

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
	delay_string_append_char(message, str, "SELECT pool_parallel(\"select ");
	KeepRewriteQueryCode(message, SELECT_DEFAULT_INSIDE_DBLINK);
	_rewriteIntensivedblink(BaseSelect,message,dblink, str,node->targetList);
	KeepRewriteQueryCode(message, message_r_code);

	delay_string_append_char(message, str, " FROM ");
	if (var->catalogname)
	{
		delay_string_append_char(message, str, var->catalogname);
		delay_string_append_char(message, str, ".");
	}

	if (var->schemaname)
	{
		delay_string_append_char(message, str, var->schemaname);
		delay_string_append_char(message, str, ".");
		message->schemaname = var->schemaname;
	}
	else
		message->schemaname = NULL;

	delay_string_append_char(message, str, var->relname);
		
	/* insert whereClause into dblink() */
	if(node->whereClause) 
	{
		delay_string_append_char(message, str, " WHERE ");
		KeepRewriteQueryCode(message, SELECT_DEFAULT_INSIDE_DBLINK);
		_rewriteNode(BaseSelect, message, dblink, str, node->whereClause);
		KeepRewriteQueryCode(message, message_r_code);
	}
	if (node->groupClause)
	{
		delay_string_append_char(message, str, " GROUP BY ");
		_rewriteNode(BaseSelect, message, dblink, str, node->groupClause);
		KeepRewriteQueryCode(message, message_r_code);
	}

	delay_string_append_char(message, str, "\"");
	delay_string_append_char(message, str, ")");
	delay_string_append_char(message, str, "'");
	delay_string_append_char(message, str, ")");


	delay_string_append_char(message, str," AS ");
	delay_string_append_char(message, str, var->relname);
	delay_string_append_char(message, str, "(");
	_rewriteReturnRecords(BaseSelect,message,node->targetList,str);
	delay_string_append_char(message, str, ")");

	if(var->alias)
		delay_string_append_char(message, str, ")");
		
	if(var->alias)
	{
		_rewriteNode(BaseSelect, message, dblink, str, var->alias);
	}

	if (node->groupClause)
	{
		delay_string_append_char(message, str, " GROUP BY ");
		_rewriteNode(BaseSelect, message, dblink, str, node->groupClause);
		KeepRewriteQueryCode(message, message_r_code);
	}

	if (node->sortClause)
	{
		delay_string_append_char(message, str, " ORDER BY ");
		_rewriteNode(BaseSelect, message, dblink, str, node->sortClause);
		KeepRewriteQueryCode(message, message_r_code);
	}


	if (node->limitOffset)
	{
		delay_string_append_char(message, str, " OFFSET ");
		_rewriteNode(BaseSelect, message, dblink, str, node->limitOffset);
		KeepRewriteQueryCode(message, message_r_code);
	}

	if (node->limitCount)
	{
		delay_string_append_char(message, str, " LIMIT ");
		if (IsA(node->limitCount, A_Const) &&
			((A_Const *)node->limitCount)->val.type == T_Null)
		{
			delay_string_append_char(message, str, "ALL ");
		}
		else
		{
			_rewriteNode(BaseSelect, message, dblink, str, node->limitCount);
			KeepRewriteQueryCode(message, message_r_code);
		}
	}
}

static void
_rewriteSelectStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SelectStmt *node)
{
	int message_r_code = message->r_code;
	BaseSelect = (Node *) node;


	if (node->larg) /* SETOP */
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->larg);
		delay_string_append_char(message, str, ")");
		KeepRewriteQueryCode(message, message_r_code);

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
			_rewriteNode(BaseSelect, message, dblink, str, node->rarg);
			delay_string_append_char(message, str, ")");
			KeepRewriteQueryCode(message, message_r_code);
		}
	}
	else
	{
		if (node->intoClause)
		{
			IntoClause *into = node->intoClause;
			RangeVar *rel = (RangeVar *)into->rel;

			delay_string_append_char(message, str, "CREATE ");
			if (rel->istemp == true)
				delay_string_append_char(message, str, "TEMP ");
			delay_string_append_char(message, str, "TABLE ");
			_rewriteNode(BaseSelect, message, dblink, str, into->rel);
			KeepRewriteQueryCode(message, message_r_code);

			if (into->colNames)
			{
				delay_string_append_char(message, str, " (");
				_rewriteNode(BaseSelect, message, dblink, str, into->colNames);
				delay_string_append_char(message, str, ") ");
				KeepRewriteQueryCode(message, message_r_code);
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

		if (node->distinctClause)
		{
			delay_string_append_char(message, str, "DISTINCT ");
			if (lfirst(list_head(node->distinctClause)) != NIL)
			{
				delay_string_append_char(message, str, "ON (");
				_rewriteNode(BaseSelect, message, dblink,str, node->distinctClause);
				KeepRewriteQueryCode(message, message_r_code);
				delay_string_append_char(message, str, ") ");
			}
		}

		_rewriteNode(BaseSelect, message, dblink, str, node->targetList);
		KeepRewriteQueryCode(message, message_r_code);

		if (node->fromClause)
		{
			delay_string_append_char(message, str, " FROM ");
			_rewriteNode(BaseSelect, message, dblink, str, node->fromClause);
			KeepRewriteQueryCode(message, message_r_code);
		}
	
		/* reset where clause */
		if (node->whereClause)
			BaseSelect = NULL;

		if (node->whereClause && message->r_code != SELECT_ONETABLE)
		{
			delay_string_append_char(message, str, " WHERE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->whereClause);
			KeepRewriteQueryCode(message, message_r_code);
		}

		if (node->groupClause)
		{
			delay_string_append_char(message, str, " GROUP BY ");
			_rewriteNode(BaseSelect, message, dblink, str, node->groupClause);
			KeepRewriteQueryCode(message, message_r_code);
		}

		if (node->havingClause)
		{
			delay_string_append_char(message, str, " HAVING ");
			_rewriteNode(BaseSelect, message, dblink, str, node->havingClause);
			KeepRewriteQueryCode(message, message_r_code);
		}
	}

	if (node->sortClause)
	{
		delay_string_append_char(message, str, " ORDER BY ");
		_rewriteNode(BaseSelect, message, dblink, str, node->sortClause);
		KeepRewriteQueryCode(message, message_r_code);
	}


	if (node->limitOffset)
	{
		delay_string_append_char(message, str, " OFFSET ");
		_rewriteNode(BaseSelect, message, dblink, str, node->limitOffset);
		KeepRewriteQueryCode(message, message_r_code);
	}

	if (node->limitCount)
	{
		delay_string_append_char(message, str, " LIMIT ");
		if (IsA(node->limitCount, A_Const) &&
			((A_Const *)node->limitCount)->val.type == T_Null)
		{
			delay_string_append_char(message, str, "ALL ");
		}
		else
		{
			_rewriteNode(BaseSelect, message, dblink, str, node->limitCount);
			KeepRewriteQueryCode(message, message_r_code);
		}
	}

	_rewriteNode(BaseSelect, message, dblink, str, node->lockingClause);
	KeepRewriteQueryCode(message, message_r_code);
}

static void
_rewriteFuncCall(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FuncCall *node)
{
	char *funcname;
	if(message->r_code == SELECT_AEXPR)
	{ 
		KeepRewriteQueryCode(message, SELECT_AEXPR_FALSE);
		return;
	}

	_rewriteFuncName(BaseSelect, message, dblink, str, node->funcname);
	funcname = strVal(lfirst(list_head(node->funcname)));

	if(strcmp(funcname,"user") == 0 ||
	   strcmp(funcname,"current_user") == 0 ||
	   strcmp(funcname,"session_user") == 0 ||
	   strcmp(funcname,"current_role") == 0)	
		return ;

	delay_string_append_char(message, str, "(");

	if (node->agg_distinct == TRUE)
		delay_string_append_char(message, str, "DISTINCT ");

	if (node->agg_star == TRUE)
		delay_string_append_char(message, str, "*");
	else
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

	if (node->forUpdate == TRUE)
		delay_string_append_char(message, str, " FOR UPDATE");
	else
		delay_string_append_char(message, str, " FOR SHARED");

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
	_rewriteNode(BaseSelect, message, dblink, str, node->typename);
	_rewriteNode(BaseSelect, message, dblink, str, node->constraints);
}

static void
_rewriteTypeName(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TypeName *node)
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
				if(message->r_code == SELECT_DEFAULT_INSIDE_DBLINK)
				{
					delay_string_append_char(message, str, "\"\"");
					delay_string_append_char(message, str, typename);
					delay_string_append_char(message, str, "\"\"");
				} else {
					delay_string_append_char(message, str, "\"");
					delay_string_append_char(message, str, typename);
					delay_string_append_char(message, str, "\"");
				}
			} else 
				delay_string_append_char(message, str, typename);
	}
	
	if (node->typemod > 0)
	{
		int lower;
		char buf[16];
		delay_string_append_char(message, str, "(");
		snprintf(buf, 16, "%d", ((node->typemod - VARHDRSZ) >> 16) & 0x00FF);
		delay_string_append_char(message, str, buf);
		lower = (node->typemod-VARHDRSZ) & 0x00FF;

		if(lower != 0)
		{
			char buf2[16];
			delay_string_append_char(message, str, ",");
			snprintf(buf2, 16, "%d",lower);
			delay_string_append_char(message, str, buf2);
		}

		delay_string_append_char(message, str, ")");
	}
}

static void
_rewriteTypeCast(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, TypeCast *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->arg);
	delay_string_append_char(message, str, "::");
	_rewriteNode(BaseSelect, message, dblink, str, node->typename);

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
_rewriteSortClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SortClause *node)
{

}

static void
_rewriteGroupClause(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, GroupClause *node)
{

}

static void
_rewriteSetOperationStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, SetOperationStmt *node)
{

}


static void
_rewriteAExpr(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, A_Expr *node)
{
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
			/* not implemented yet */
			break;

		case AEXPR_OP_ALL:
			/* not implemented yet */
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
			delay_string_append_char(message, str, "'");
			delay_string_append_char(message, str, escape_string(value->val.str));
			delay_string_append_char(message, str, "'");
			break;

		case T_Null:
			delay_string_append_char(message, str, "NULL");
			break;

		default:
			break;
	}
}

static void
_rewriteColumnRef(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ColumnRef *node)
{
	ListCell *c;
	char first = 0;

	foreach (c, node->fields)
	{
		Node *n = (Node *) lfirst(c);

		if (IsA(n, String))
		{
			Value *v = (Value *) lfirst(c);

			if(message->r_code == SELECT_AEXPR && 
				(_checkDistDefColumn(node, message) != 1))
			{
				KeepRewriteQueryCode(message, SELECT_AEXPR_FALSE);
				return;
			}

			if (first == 0)
				first = 1;
			else
				delay_string_append_char(message, str, ".");

			/* delay_string_append_char(message, str, "\""); */
			delay_string_append_char(message, str, v->val.str);
			/* delay_string_append_char(message, str, "\""); */
			delay_string_append_char(message, str, " ");
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
			if(message->r_code == SELECT_DEFAULT_INSIDE_DBLINK)
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
_rewriteResTarget(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ResTarget *node)
{
	if (node->indirection != NIL)
	{
		delay_string_append_char(message, str, "\"");
		delay_string_append_char(message, str, node->name);
		delay_string_append_char(message, str, "\"=");
		_rewriteNode(BaseSelect, message, dblink, str, node->val);
	}
	else
	{
		_rewriteNode(BaseSelect, message, dblink, str, node->val);

		if (node->name)
		{
			delay_string_append_char(message, str, " AS ");
			delay_string_append_char(message, str, "\"");
			delay_string_append_char(message, str, node->name);
			delay_string_append_char(message, str, "\" ");
		}
	}
}

static void
_rewriteConstraint(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, Constraint *node)
{
	if (node->name)
	{
		delay_string_append_char(message, str, "CONSTRAINT \"");
		delay_string_append_char(message, str, node->name);
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
_rewriteFkConstraint(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, FkConstraint *node)
{
	if (node->constr_name)
	{
		delay_string_append_char(message, str, "CONSTRAINT \"");
		delay_string_append_char(message, str, node->constr_name);
		delay_string_append_char(message, str, "\"");
	}

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
	if (node->vacuum == true)
		delay_string_append_char(message, str, "VACUUM ");
	else
		delay_string_append_char(message, str, "ANALYZE ");

	if (node->full == TRUE)
		delay_string_append_char(message, str, "FULL ");
	
	if (node->freeze_min_age == 0)
		delay_string_append_char(message, str, "FREEZE ");

	if (node->verbose == TRUE)
		delay_string_append_char(message, str, "VERBOSE ");

	if (node->analyze)
		delay_string_append_char(message, str, "ANALYZE ");

	_rewriteNode(BaseSelect, message, dblink, str, node->va_cols);
}

static void _rewriteExplainStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, ExplainStmt *node)
{
	delay_string_append_char(message, str, "EXPLAIN ");
	
	if (node->analyze == TRUE)
		delay_string_append_char(message, str, "ANALYZE ");
	if (node->verbose == TRUE)
		delay_string_append_char(message, str, "VERBOSE ");

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
	_rewriteNode(BaseSelect, message, dblink, str, node->relation);
}

static void _rewriteUnlistenStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, UnlistenStmt *node)
{
	delay_string_append_char(message, str, "UNLISTEN ");
	_rewriteNode(BaseSelect, message, dblink, str, node->relation);
}

static void _rewriteLoadStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, LoadStmt *node)
{
	delay_string_append_char(message, str, "LOAD '");
	delay_string_append_char(message, str, node->filename);
	delay_string_append_char(message, str, "'");
}

static void _rewriteCopyStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CopyStmt *node)
{
	int binary = FALSE;
	int oids = FALSE;
	char *delimiter = NULL;
	char *null = NULL;
	int csv = FALSE;
	int header = FALSE;
	char *quote = NULL;
	char *escape = NULL;
	Node *force_quote = NULL;
	Node *force_notnull = NULL;
	ListCell *lc;

	foreach (lc, node->attlist)
	{
		DefElem *e = lfirst(lc);

		if (strcmp(e->defname, "binary") == 0)
			binary = TRUE;
		else if (strcmp(e->defname, "oids") == 0)
			oids = TRUE;
		else if (strcmp(e->defname, "delimiter") == 0)
			delimiter = ((Value *) e->arg)->val.str;
		else if (strcmp(e->defname, "null") == 0)
			null = ((Value *) e->arg)->val.str;
		else if (strcmp(e->defname, "csv") == 0)
			csv = TRUE;
		else if (strcmp(e->defname, "header") == 0)
			header = TRUE;
		else if (strcmp(e->defname, "quote") == 0)
			quote = ((Value *) e->arg)->val.str;
		else if (strcmp(e->defname, "escape") == 0)
			escape = ((Value *) e->arg)->val.str;
		else if (strcmp(e->defname, "force_quote") == 0)
			force_quote = e->arg;
		else if (strcmp(e->defname, "force_notnull") == 0)
			force_notnull = e->arg;
	}

	delay_string_append_char(message, str, "COPY ");

	if (node->query)
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->query);
		delay_string_append_char(message, str, ")");
	}


	if (binary == TRUE)
		delay_string_append_char(message, str, "BINARY ");

	_rewriteNode(BaseSelect, message, dblink, str, node->relation);

	if (node->attlist)
	{
		delay_string_append_char(message, str, "(");
		_rewriteNode(BaseSelect, message, dblink, str, node->attlist);
		delay_string_append_char(message, str, ") ");
	}

	if (oids == TRUE)
		delay_string_append_char(message, str, " OIDS ");

	if (node->is_from == TRUE)
		delay_string_append_char(message, str, " FROM ");
	else
		delay_string_append_char(message, str, " TO ");

	if (node->filename)
	{
		delay_string_append_char(message, str, "'");
		delay_string_append_char(message, str, node->filename);
		delay_string_append_char(message, str, "'");
	}
	else
		delay_string_append_char(message, str, node->is_from == TRUE ? "STDIN" : "STDOUT");

	if (delimiter)
	{
		delay_string_append_char(message, str, " DELIMITERS '");
		delay_string_append_char(message, str, delimiter);
		delay_string_append_char(message, str, "'");
	}

	if (null)
	{
		delay_string_append_char(message, str, "NULL '");
		delay_string_append_char(message, str, null);
		delay_string_append_char(message, str, "' ");
	}

	if (csv == TRUE)
		delay_string_append_char(message, str, "CSV ");

	if (header == TRUE)
		delay_string_append_char(message, str, "HEADER ");

	if (quote)
	{
		delay_string_append_char(message, str, "QUOTE '");
		delay_string_append_char(message, str, quote);
		delay_string_append_char(message, str, "' ");
	}

	if (escape)
	{
		delay_string_append_char(message, str, "ESCAPE '");
		delay_string_append_char(message, str, escape);
		delay_string_append_char(message, str, "' ");
	}

	if (force_quote)
	{
		delay_string_append_char(message, str, "FORCE QUOTE ");
		_rewriteNode(BaseSelect, message, dblink, str, force_quote);
	}

	if (force_notnull)
	{
		delay_string_append_char(message, str, " FORCE NOT NULL ");
		_rewriteNode(BaseSelect, message, dblink, str, force_notnull);
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
			if (node->transform)
			{
				delay_string_append_char(message, str, " USING ");
				_rewriteNode(BaseSelect, message, dblink, str, node->transform);
			}
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
	if (node->sequence->istemp)
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
_rewriteDropPLangStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropPLangStmt *node)
{
	delay_string_append_char(message, str, "DROP LANGUAGE \"");
	delay_string_append_char(message, str, node->plname);
	delay_string_append_char(message, str, "\"");
	if (node->behavior == DROP_CASCADE)
		delay_string_append_char(message, str, " CASCADE");
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
			//delay_string_append_char(message, str, "\"");
			delay_string_append_char(message, str, v->val.str);
			//delay_string_append_char(message, str, "\"");
		}
		else
		{
		}
	}
}

static void
_rewriteCreateTrigStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, CreateTrigStmt *node)
{
	int i, len;

	if (node->isconstraint == TRUE)
		delay_string_append_char(message, str, "CREATE CONSTRAINT TRIGGER \"");
	else
		delay_string_append_char(message, str, "CREATE TRIGGER \"");
	delay_string_append_char(message, str, node->trigname);
	delay_string_append_char(message, str, "\" ");

	if (node->before == TRUE)
		delay_string_append_char(message, str, "BEFORE ");
	else
		delay_string_append_char(message, str, "AFTER ");

	len = strlen(node->actions);
	for (i = 0; i < len; i++)
	{
		if (i)
			delay_string_append_char(message, str, "OR ");
		
		if (node->actions[i] == 'i')
			delay_string_append_char(message, str, "INSERT ");
		else if (node->actions[i] == 'd')
			delay_string_append_char(message, str, "DELETE ");
		else
			delay_string_append_char(message, str, "UPDATE ");
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
_rewriteDropPropertyStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropPropertyStmt *node)
{
	switch (node->removeType)
	{
		case OBJECT_TRIGGER:
			delay_string_append_char(message, str, "DROP TRIGGER \"");
			delay_string_append_char(message, str, node->property);
			delay_string_append_char(message, str, "\" ON ");
			_rewriteNode(BaseSelect, message, dblink, str, node->relation);
			if (node->behavior == DROP_CASCADE)
				delay_string_append_char(message, str, " CASCADE");
			break;

		case OBJECT_RULE:
			delay_string_append_char(message, str, "DROP RULE \"");
			delay_string_append_char(message, str, node->property);
			delay_string_append_char(message, str, "\" ON ");
			_rewriteNode(BaseSelect, message, dblink, str, node->relation);
			if (node->behavior == DROP_CASCADE)
				delay_string_append_char(message, str, " CASCADE");
			break;

		default:
			break;
	}
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
			if (node->recheck == TRUE)
				delay_string_append_char(message, str, " RECHECK");
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
_rewriteRemoveOpClassStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RemoveOpClassStmt *node)
{
	delay_string_append_char(message, str, "DROP OPERATOR CLASS ");
	_rewriteFuncName(BaseSelect, message, dblink, str, node->opclassname);
	delay_string_append_char(message, str, " USING ");
	delay_string_append_char(message, str, node->amname);
	if (node->behavior == DROP_CASCADE)
		delay_string_append_char(message, str, " CASCADE");
}

static void
_rewriteDropStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropStmt *node)
{
	ListCell *lc;
	char comma = 0;
	
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

		default:
			break;
	}

	foreach (lc, node->objects)
	{
		if (comma == 0)
			comma = 1;
		else
			delay_string_append_char(message, str, ", ");
		_rewriteFuncName(BaseSelect, message, dblink, str, lfirst(lc));
	}

	if (node->behavior == DROP_CASCADE)
		delay_string_append_char(message, str, " CASCADE");
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
_rewritePrivTarget(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, PrivTarget *node)
{
	switch (node->objtype)
	{
		case ACL_OBJECT_RELATION:
			_rewriteNode(BaseSelect, message, dblink, str, node->objs);
			break;

		case ACL_OBJECT_SEQUENCE:
			delay_string_append_char(message, str, "SEQUENCE ");
			_rewriteNode(BaseSelect, message, dblink, str, node->objs);
			break;

		case ACL_OBJECT_FUNCTION:
			delay_string_append_char(message, str, "FUNCTION ");
			_rewriteNode(BaseSelect, message, dblink, str, node->objs);
			break;

		case ACL_OBJECT_DATABASE:
			delay_string_append_char(message, str, "DATABASE ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->objs);
			break;

		case ACL_OBJECT_LANGUAGE:
			delay_string_append_char(message, str, "LANGUAGE ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->objs);
			break;

		case ACL_OBJECT_NAMESPACE:
			delay_string_append_char(message, str, "SCHEMA ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->objs);
			break;

		case ACL_OBJECT_TABLESPACE:
			delay_string_append_char(message, str, "TABLESPACE ");
			_rewriteIdList(BaseSelect, message, dblink, str, node->objs);
			break;
	}
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
	PrivTarget *n;
	
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

	n = makeNode(PrivTarget);
	n->objtype = node->objtype;
	n->objs = node->objects;
	_rewriteNode(BaseSelect, message, dblink, str, n);
	pfree(n);

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
_rewriteRemoveFuncStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RemoveFuncStmt *node)
{
	switch (node->kind)
	{
		case OBJECT_FUNCTION:
			delay_string_append_char(message, str, "DROP FUNCTION ");
			break;

		case OBJECT_AGGREGATE:
			delay_string_append_char(message, str, "DROP AGGREGATE ");
			break;

		case OBJECT_OPERATOR:
			delay_string_append_char(message, str, "DROP OPERATOR CLASS ");
			break;

		default:
			break;
	}

	if (node->missing_ok)
		delay_string_append_char(message, str, "IF EXISTS ");

	_rewriteFuncName(BaseSelect, message, dblink, str, node->name);

	delay_string_append_char(message, str, " (");
	_rewriteNode(BaseSelect, message, dblink, str, node->args);
	delay_string_append_char(message, str, ")");

	if (node->behavior == DROP_CASCADE)
		delay_string_append_char(message, str, " CASCADE");
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
_rewriteDropCastStmt(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, DropCastStmt *node)
{
	delay_string_append_char(message, str, "DROP CAST (");
	_rewriteNode(BaseSelect, message, dblink, str, node->sourcetype);
	delay_string_append_char(message, str, " AS ");
	_rewriteNode(BaseSelect, message, dblink, str, node->targettype);
	delay_string_append_char(message, str, ")");

	if (node->behavior == DROP_CASCADE)
		delay_string_append_char(message, str, " CASCADE");
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
			delay_string_append_char(message, str, node->addname);
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

	if (node->view->istemp == TRUE)
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
	_rewriteNode(BaseSelect, message, dblink, str, node->typename);


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
	_rewriteFuncName(BaseSelect, message, dblink, str, node->typename);

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
	if (node->into)
	{
		IntoClause *into = node->into;
		RangeVar *rel = into->rel;

		delay_string_append_char(message, str, "CREATE ");
		if (rel->istemp == TRUE)
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
	
	if(message->r_code == SELECT_AEXPR) 
	{
		KeepRewriteQueryCode(message, SELECT_AEXPR_FALSE);
		return;
	}

	delay_string_append_char(message, str, "(");
	_rewriteNode(BaseSelect, message, dblink, str, node->subquery);
	delay_string_append_char(message, str, ")");

	_rewriteNode(BaseSelect, message, dblink, str, node->alias);
}

static void
_rewriteRangeFunction(Node *BaseSelect, RewriteQuery *message, ConInfoTodblink *dblink, String *str, RangeFunction *node)
{
	_rewriteNode(BaseSelect, message, dblink, str, node->funccallnode);
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
		Value *v;
		
		elem = linitial(def_list);
		v = (Value *)elem->arg;
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

	string_append_char(str, " WITH ");
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
	if (obj == NULL)
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
				{
					int message_r_code = message->r_code;

					if(message_r_code != SELECT_ONETABLE &&
						checkSelectStmtOneTable(BaseSelect, message, dblink, str,obj) == 1)
					{
						/* Special Query rewriteing when fromClause has one-table */
						KeepRewriteQueryCode(message, SELECT_ONETABLE);

						/* Intensive Query rewriting starts */
						if(checkIntensiveOneTable(BaseSelect, message, dblink, str, obj) == 1)
						{
							pool_debug("OneTable Intensive optimization");
							_rewriteSelectStmtIntensive(BaseSelect, message, dblink, str, obj);
							KeepRewriteQueryCode(message, message_r_code);
							break;
						}
					} else {
						KeepRewriteQueryCode(message, SELECT_DEFAULT);
					}
					/* default dblink action */
					_rewriteSelectStmt(BaseSelect, message, dblink, str, obj);
					KeepRewriteQueryCode(message, message_r_code);
				}
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
			case T_SortClause:
				_rewriteSortClause(BaseSelect, message, dblink, str, obj);
				break;
			case T_GroupClause:
				_rewriteGroupClause(BaseSelect, message, dblink, str, obj);
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
			case T_A_Indirection:
				_rewriteA_Indirection(BaseSelect, message, dblink, str, obj);
				break;
			case T_ResTarget:
				_rewriteResTarget(BaseSelect, message, dblink, str, obj);
				break;
			case T_Constraint:
				_rewriteConstraint(BaseSelect, message, dblink, str, obj);
				break;
			case T_FkConstraint:
				_rewriteFkConstraint(BaseSelect, message, dblink, str, obj);
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

			case T_DropPLangStmt:
				_rewriteDropPLangStmt(BaseSelect, message, dblink, str, obj);
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

			case T_DropPropertyStmt:
				_rewriteDropPropertyStmt(BaseSelect, message, dblink, str, obj);
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

			case T_RemoveOpClassStmt:
				_rewriteRemoveOpClassStmt(BaseSelect, message, dblink, str, obj);
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

			case T_PrivTarget:
				_rewritePrivTarget(BaseSelect, message, dblink, str, obj);
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

			case T_RemoveFuncStmt:
				_rewriteRemoveFuncStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_CreateCastStmt:
				_rewriteCreateCastStmt(BaseSelect, message, dblink, str, obj);
				break;

			case T_DropCastStmt:
				_rewriteDropCastStmt(BaseSelect, message, dblink, str, obj);
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
			case T_RemoveOpFamilyStmt:
			case T_CreateEnumStmt:
			case T_DropOwnedStmt:
			case T_ReassignOwnedStmt:
			case T_AlterTSDictionaryStmt:
			case T_AlterTSConfigurationStmt:
			case T_XmlSerialize:
			case T_InhRelation:
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
	_rewriteNode(NULL, message, dblink, str, obj);
	message->rewrite_query = str->data;
}
