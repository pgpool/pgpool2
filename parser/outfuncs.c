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
 *	  $PostgreSQL: pgsql/src/backend/nodes/outfuncs.c,v 1.261.2.1 2005/11/14 23:54:34 tgl Exp $
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

#include "pool_memory.h"
#include "parser.h"
#include "pool_string.h"
#include "pg_list.h"
#include "parsenodes.h"

#define booltostr(x)  ((x) ? "true" : "false")

static void _outNode(String *str, void *obj);
static void _outList(String *str, List *node);
static void _outIdList(String *str, List *node);
static void _outAlias(String *str, Alias *node);
static void _outRangeVar(String *str, RangeVar *node);
static void _outVar(String *str, Var *node);
static void _outConst(String *str, Const *node);
static void _outParam(String *str, Param *node);
static void _outAggref(String *str, Aggref *node);
static void _outArrayRef(String *str, ArrayRef *node);
static void _outFuncExpr(String *str, FuncExpr *node);
static void _outOpExpr(String *str, OpExpr *node);
static void _outDistinctExpr(String *str, DistinctExpr *node);
static void _outScalarArrayOpExpr(String *str, ScalarArrayOpExpr *node);
static void _outBoolExpr(String *str, BoolExpr *node);
static void _outSubLink(String *str, SubLink *node);
static void _outSubPlan(String *str, SubPlan *node);
static void _outFieldSelect(String *str, FieldSelect *node);
static void _outFieldStore(String *str, FieldStore *node);
static void _outRelabelType(String *str, RelabelType *node);
static void _outConvertRowtypeExpr(String *str, ConvertRowtypeExpr *node);
static void _outCaseExpr(String *str, CaseExpr *node);
static void _outCaseWhen(String *str, CaseWhen *node);
static void _outCaseTestExpr(String *str, CaseTestExpr *node);
static void _outArrayExpr(String *str, ArrayExpr *node);
static void _outRowExpr(String *str, RowExpr *node);
static void _outCoalesceExpr(String *str, CoalesceExpr *node);
static void _outMinMaxExpr(String *str, MinMaxExpr *node);
static void _outNullIfExpr(String *str, NullIfExpr *node);
static void _outNullTest(String *str, NullTest *node);
static void _outBooleanTest(String *str, BooleanTest *node);
static void _outCoerceToDomain(String *str, CoerceToDomain *node);
static void _outCoerceToDomainValue(String *str, CoerceToDomainValue *node);
static void _outSetToDefault(String *str, SetToDefault *node);
static void _outTargetEntry(String *str, TargetEntry *node);
static void _outRangeTblRef(String *str, RangeTblRef *node);
static void _outJoinExpr(String *str, JoinExpr *node);
static void _outFromExpr(String *str, FromExpr *node);
static void _outCreateStmt(String *str, CreateStmt *node);
static void _outIndexStmt(String *str, IndexStmt *node);
static void _outNotifyStmt(String *str, NotifyStmt *node);
static void _outDeclareCursorStmt(String *str, DeclareCursorStmt *node);
static void _outSelectStmt(String *str, SelectStmt *node);
static void _outFuncCall(String *str, FuncCall *node);
static void _outDefElem(String *str, DefElem *node);
static void _outLockingClause(String *str, LockingClause *node);
static void _outColumnDef(String *str, ColumnDef *node);
static void _outTypeName(String *str, TypeName *node);
static void _outTypeCast(String *str, TypeCast *node);
static void _outIndexElem(String *str, IndexElem *node);
static void _outSortClause(String *str, SortClause *node);
static void _outGroupClause(String *str, GroupClause *node);
static void _outSetOperationStmt(String *str, SetOperationStmt *node);
static void _outAExpr(String *str, A_Expr *node);
static void _outValue(String *str, Value *value);
static void _outColumnRef(String *str, ColumnRef *node);
static void _outParamRef(String *str, ParamRef *node);
static void _outAConst(String *str, A_Const *node);
static void _outA_Indices(String *str, A_Indices *node);
static void _outA_Indirection(String *str, A_Indirection *node);
static void _outResTarget(String *str, ResTarget *node);
static void _outConstraint(String *str, Constraint *node);
static void _outFkConstraint(String *str, FkConstraint *node);

static void _outSortBy(String *str, SortBy *node);
static void _outInsertStmt(String *str, InsertStmt *node);
static void _outUpdateStmt(String *str, UpdateStmt *node);
static void _outDeleteStmt(String *str, DeleteStmt *node);
static void _outTransactionStmt(String *str, TransactionStmt *node);
static void _outTruncateStmt(String *str, TruncateStmt *node);
static void _outVacuumStmt(String *str, VacuumStmt *node);
static void _outExplainStmt(String *str, ExplainStmt *node);
static void _outClusterStmt(String *str, ClusterStmt *node);
static void _outCheckPointStmt(String *str, CheckPointStmt *node);
static void _outClosePortalStmt(String *str, ClosePortalStmt *node);
static void _outListenStmt(String *str, ListenStmt *node);
static void _outUnlistenStmt(String *str, UnlistenStmt *node);
static void _outLoadStmt(String *str, LoadStmt *node);
static void _outCopyStmt(String *str, CopyStmt *node);
static void _outDeallocateStmt(String *str, DeallocateStmt *node);
static void _outRenameStmt(String *str, RenameStmt *node);
static void _outCreateRoleStmt(String *str, CreateRoleStmt *node);
static void _outAlterRoleStmt(String *str, AlterRoleStmt *node);
static void _outDropRoleStmt(String *str, DropRoleStmt *node);
static void _outCreateSchemaStmt(String *str, CreateSchemaStmt *node);
static void _outVariableSetStmt(String *str, VariableSetStmt *node);
static void _outVariableShowStmt(String *str, VariableShowStmt *node);
static void _outConstraintsSetStmt(String *str, ConstraintsSetStmt *node);
static void _outAlterTableStmt(String *str, AlterTableStmt *node);
static void _outCreateSeqStmt(String *str, CreateSeqStmt *node);
static void _outAlterSeqStmt(String *str, AlterSeqStmt *node);
static void _outCreatePLangStmt(String *str, CreatePLangStmt *node);
static void _outDropPLangStmt(String *str, DropPLangStmt *node);
static void _outCreateTableSpaceStmt(String *str, CreateTableSpaceStmt *node);
static void _outDropTableSpaceStmt(String *str, DropTableSpaceStmt *node);
static void _outCreateTrigStmt(String *str, CreateTrigStmt *node);
static void _outDropPropertyStmt(String *str, DropPropertyStmt *node);
static void _outDefineStmt(String *str, DefineStmt *node);
static void _outCreateOpClassStmt(String *str, CreateOpClassStmt *node);
static void _outRemoveOpClassStmt(String *str, RemoveOpClassStmt *node);
static void _outDropStmt(String *str, DropStmt *node);
static void _outFetchStmt(String *str, FetchStmt *node);
static void _outGrantStmt(String *str, GrantStmt *node);
static void _outGrantRoleStmt(String *str, GrantRoleStmt *node);
static void _outCreateFunctionStmt(String *str, CreateFunctionStmt *node);
static void _outAlterFunctionStmt(String *str, AlterFunctionStmt *node);
static void _outCreateCastStmt(String *str, CreateCastStmt *node);
static void _outDropCastStmt(String *str, DropCastStmt *node);
static void _outReindexStmt(String *str, ReindexStmt *node);
static void _outRuleStmt(String *str, RuleStmt *node);
static void _outViewStmt(String *str, ViewStmt *node);
static void _outCreatedbStmt(String *str, CreatedbStmt *node);
static void _outAlterDatabaseStmt(String *str, AlterDatabaseStmt *node);
static void _outAlterDatabaseSetStmt(String *str, AlterDatabaseSetStmt *node);
static void _outDropdbStmt(String *str, DropdbStmt *node);
static void _outCreateDomainStmt(String *str, CreateDomainStmt *node);
static void _outAlterDomainStmt(String *str, AlterDomainStmt *node);
static void _outCreateConversionStmt(String *str, CreateConversionStmt *node);
static void _outPrepareStmt(String *str, PrepareStmt *node);
static void _outExecuteStmt(String *str, ExecuteStmt *node);
static void _outLockStmt(String *str, LockStmt *node);
static void _outCommentStmt(String *str, CommentStmt *node);
static void _outDiscardStmt(String *str, DiscardStmt *node);
static void _outCreateOpFamilyStmt(String *str, CreateOpFamilyStmt *node);
static void _outAlterOpFamilyStmt(String *str, AlterOpFamilyStmt *node);
static void _outRemoveOpFamilyStmt(String *str, RemoveOpFamilyStmt *node);
static void _outCreateEnumStmt(String *str, CreateEnumStmt *node);
static void _outDropOwnedStmt(String *str, DropOwnedStmt *node);
static void _outReassignOwnedStmt(String *str, ReassignOwnedStmt *node);
static void _outAlterTSDictionaryStmt(String *str, AlterTSDictionaryStmt *node);
static void _outAlterTSConfigurationStmt(String *str, AlterTSConfigurationStmt *node);
static void _outXmlSerialize(String *str, XmlSerialize *node);

static void _outFuncName(String *str, List *func_name);
static void _outSetRest(String *str, VariableSetStmt *node);
static void _outSetTransactionModeList(String *str, List *list);
static void _outAlterTableCmd(String *str, AlterTableCmd *node);
static void _outOptSeqList(String *str, List *options);
static void _outPrivGrantee(String *str, PrivGrantee *node);
static void _outPrivTarget(String *str, PrivTarget *node);
static void _outFuncWithArgs(String *str, FuncWithArgs *node);
static void _outFunctionParameter(String *str, FunctionParameter *node);
static void _outPrivilegeList(String *str, List *list);
static void _outFuncOptList(String *str, List *list);
static void _outCreatedbOptList(String *str, List *options);
static void _outOperatorArgTypes(String *str, List *args);
static void _outRangeFunction(String *str, RangeFunction *node);
static void _outInhRelation(String *str, InhRelation *node);
static void _outWithDefinition(String *str, List *def_list);
static void _outCurrentOfExpr(String *str, CurrentOfExpr *node);

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

static void _outIdList(String *str, List *node)
{
	ListCell   *lc;
	char first = 0;

	foreach(lc, node)
	{
		Value *v = lfirst(lc);

		if (first == 0)
			first = 1;
		else
			string_append_char(str, ", ");

		string_append_char(str, "\"");
		string_append_char(str, v->val.str);
		string_append_char(str, "\"");
	}
}

static void _outList(String *str, List *node)
{
	ListCell   *lc;
	char first = 0;

	foreach(lc, node)
	{
		if (first == 0)
			first = 1;
		else
		{	
			if (!IsA(lfirst(lc), A_Indices))
				string_append_char(str, ",");
		}
		_outNode(str, lfirst(lc));
	}
}



/*****************************************************************************
 *
 *	Stuff from primnodes.h.
 *
 *****************************************************************************/

static void
_outAlias(String *str, Alias *node)
{
	string_append_char(str, " AS \"");
	string_append_char(str, node->aliasname);
	string_append_char(str, "\"");
	
	if (node->colnames)
	{
		string_append_char(str, "(");
		_outNode(str, node->colnames);
		string_append_char(str, ")");
	}
}

static void
_outRangeVar(String *str, RangeVar *node)
{
	if (node->catalogname)
	{
		string_append_char(str, "\"");
		string_append_char(str, node->catalogname);
		string_append_char(str, "\".");
	}

	if (node->schemaname)
	{
		string_append_char(str, "\"");
		string_append_char(str, node->schemaname);
		string_append_char(str, "\".");
	}

	string_append_char(str, "\"");
	string_append_char(str, node->relname);
	string_append_char(str, "\"");

	if (node->alias)
		_outNode(str, node->alias);

	if (node->inhOpt == INH_YES)
	{
		string_append_char(str, " * ");
	}
}

static void
_outVar(String *str, Var *node)
{

}

static void
_outConst(String *str, Const *node)
{

}

static void
_outParam(String *str, Param *node)
{

}

static void
_outAggref(String *str, Aggref *node)
{

}

static void
_outArrayRef(String *str, ArrayRef *node)
{
}

static void
_outFuncExpr(String *str, FuncExpr *node)
{

}

static void
_outOpExpr(String *str, OpExpr *node)
{

}

static void
_outDistinctExpr(String *str, DistinctExpr *node)
{

}

static void
_outScalarArrayOpExpr(String *str, ScalarArrayOpExpr *node)
{

}

static void
_outBoolExpr(String *str, BoolExpr *node)
{

}

static void
_outSubLink(String *str, SubLink *node)
{
	_outNode(str, node->testexpr);

	if (node->operName != NIL)
	{
		Value *v = linitial(node->operName);
		if (strcmp(v->val.str, "=") == 0)
			string_append_char(str, " IN ");
		else
		{
			string_append_char(str, v->val.str);
		}
	}

	switch (node->subLinkType)
	{
		case EXISTS_SUBLINK:
			string_append_char(str, " EXISTS ");
			break;

		case ARRAY_SUBLINK:
			string_append_char(str, " ARRAY ");
			break;

		case ANY_SUBLINK:
			if (node->operName != NIL)
			{
				Value *v = linitial(node->operName);
				if (strcmp(v->val.str, "=") != 0)
				{
					string_append_char(str, v->val.str);
					string_append_char(str, " ANY ");
				}
			}
			break;

		case ALL_SUBLINK:
			string_append_char(str, " ALL ");
			break;

		default:
			break;
	}


	if (node->subselect)
	{
		string_append_char(str, "(");
		_outNode(str, node->subselect);
		string_append_char(str, ")");
	}
}

static void
_outSubPlan(String *str, SubPlan *node)
{

}

static void
_outFieldSelect(String *str, FieldSelect *node)
{

}

static void
_outFieldStore(String *str, FieldStore *node)
{

}

static void
_outRelabelType(String *str, RelabelType *node)
{

}

static void
_outConvertRowtypeExpr(String *str, ConvertRowtypeExpr *node)
{

}

static void
_outCaseExpr(String *str, CaseExpr *node)
{
	ListCell *lc;

	string_append_char(str, "CASE ");
	if (node->arg)
		_outNode(str, node->arg);

	foreach (lc, node->args)
	{
		_outNode(str, lfirst(lc));
	}

	if (node->defresult)
	{
		string_append_char(str, " ELSE ");
		_outNode(str, node->defresult);
	}

	string_append_char(str, " END");
}

static void
_outCaseWhen(String *str, CaseWhen *node)
{
	string_append_char(str, " WHEN ");
	_outNode(str, node->expr);
	string_append_char(str, " THEN ");
	_outNode(str, node->result);
}

static void
_outCaseTestExpr(String *str, CaseTestExpr *node)
{

}

static void
_outArrayExpr(String *str, ArrayExpr *node)
{
	string_append_char(str, "[");
	_outNode(str, node->elements);
	string_append_char(str, "]");
}

static void
_outRowExpr(String *str, RowExpr *node)
{
	if (node->args == NIL)
		string_append_char(str, "ROW ()");
	else
	{
		string_append_char(str, "ROW (");
		_outNode(str, node->args);
		string_append_char(str, ")");
	}
}

static void
_outCoalesceExpr(String *str, CoalesceExpr *node)
{
	string_append_char(str, "COALESCE (");
	_outNode(str, node->args);
	string_append_char(str, ")");
}

static void
_outMinMaxExpr(String *str, MinMaxExpr *node)
{
	if (node->op == IS_GREATEST)
	{
		string_append_char(str, "GREATEST (");
		_outNode(str, node->args);
		string_append_char(str, ")");
	}
	else if (node->op == IS_LEAST)
	{
		string_append_char(str, "LEAST (");
		_outNode(str, node->args);
		string_append_char(str, ")");
	}
}

static void
_outNullIfExpr(String *str, NullIfExpr *node)
{

}

static void
_outNullTest(String *str, NullTest *node)
{
	_outNode(str, node->arg);
	if (node->nulltesttype == IS_NOT_NULL)
		string_append_char(str, " IS NOT NULL");
	else
		string_append_char(str, " IS NULL");
}

static void
_outBooleanTest(String *str, BooleanTest *node)
{
	_outNode(str, node->arg);

	switch (node->booltesttype)
	{
		case IS_TRUE:
			string_append_char(str, " IS TRUE");
			break;

		case IS_NOT_TRUE:
			string_append_char(str, " IS NOT TRUE");
			break;

		case IS_FALSE:
			string_append_char(str, " IS FALSE");
			break;

		case IS_NOT_FALSE:
			string_append_char(str, " IS NOT FALSE");
			break;

		case IS_UNKNOWN:
			string_append_char(str, " IS UNKNOWN");
			break;

		case IS_NOT_UNKNOWN:
			string_append_char(str, " IS NOT UNKNOWN");
			break;
	}
}

static void
_outCoerceToDomain(String *str, CoerceToDomain *node)
{

}

static void
_outCoerceToDomainValue(String *str, CoerceToDomainValue *node)
{

}

static void
_outSetToDefault(String *str, SetToDefault *node)
{
	string_append_char(str, "DEFAULT");
}

static void
_outTargetEntry(String *str, TargetEntry *node)
{

}

static void
_outRangeTblRef(String *str, RangeTblRef *node)
{

}

static void
_outJoinExpr(String *str, JoinExpr *node)
{
	_outNode(str, node->larg);

	if (node->isNatural == TRUE)
		string_append_char(str, " NATURAL");

	if (node->jointype == JOIN_INNER)
	{
		if (node->using == NIL && node->quals == NULL && !node->isNatural)
			string_append_char(str, " CROSS JOIN ");
		else
			string_append_char(str, " JOIN ");
	}
	else if (node->jointype == JOIN_INNER)
		string_append_char(str, " JOIN ");
	else if (node->jointype == JOIN_LEFT)
		string_append_char(str, " LEFT OUTER JOIN ");
	else if (node->jointype == JOIN_FULL)
		string_append_char(str, " FULL OUTER JOIN ");
	else if (node->jointype == JOIN_RIGHT)
		string_append_char(str, " RIGHT OUTER JOIN ");

	_outNode(str, node->rarg);

	if (node->using != NIL && IsA(node->using, List))
	{
		ListCell *lc;
		char comma = 0;

		string_append_char(str, " USING(");

		foreach (lc, node->using)
		{
			Value *value;

			if (comma == 0)
				comma = 1;
			else
				string_append_char(str, ",");
			
			value = lfirst(lc);
			string_append_char(str, "\"");
			string_append_char(str, value->val.str);
			string_append_char(str, "\"");
		}
		
		string_append_char(str, ")");
	}

	if (node->quals)
	{
		string_append_char(str, " ON ");
		_outNode(str, node->quals);
	}
}

static void
_outFromExpr(String *str, FromExpr *node)
{

}

/*****************************************************************************
 *
 *	Stuff from parsenodes.h.
 *
 *****************************************************************************/

static void
_outCreateStmt(String *str, CreateStmt *node)
{
	string_append_char(str, "CREATE ");
	if (node->relation->istemp)
		string_append_char(str, "TEMP ");
	string_append_char(str, "TABLE ");
	_outNode(str, node->relation);
	string_append_char(str, " (");
	_outNode(str, node->tableElts);
	string_append_char(str, ") ");

	if (node->inhRelations != NIL)
	{
		string_append_char(str, "INHERITS (");
		_outNode(str, node->inhRelations);
		string_append_char(str, ")");
	}

	if (node->options)
		_outWithDefinition(str, node->options);

	switch (node->oncommit)
	{
		case ONCOMMIT_DROP:
			string_append_char(str, " ON COMMIT DROP");
			break;

		case ONCOMMIT_DELETE_ROWS:
			string_append_char(str, " ON COMMIT DELETE ROWS");
			break;

		case ONCOMMIT_PRESERVE_ROWS:
			string_append_char(str, " ON COMMIT PRESERVE ROWS");
			break;

		default:
			break;
	}

	if (node->tablespacename)
	{
		string_append_char(str, " TABLESPACE \"");
		string_append_char(str, node->tablespacename);
		string_append_char(str, "\"");
	}
}

static void
_outIndexStmt(String *str, IndexStmt *node)
{
	string_append_char(str, "CREATE ");

	if (node->unique == TRUE)
		string_append_char(str, "UNIQUE ");

	if (node->concurrent == true)
		string_append_char(str, "INDEX CONCURRENTLY \"");
	else
		string_append_char(str, "INDEX \"");		
	string_append_char(str, node->idxname);
	string_append_char(str, "\" ON ");
	_outNode(str, node->relation);
	
	if (strcmp(node->accessMethod, DEFAULT_INDEX_TYPE))
	{
		string_append_char(str, " USING ");
		string_append_char(str, node->accessMethod);
	}

	string_append_char(str, "(");
	_outNode(str, node->indexParams);
	string_append_char(str, ")");

	if (node->tableSpace)
	{
		string_append_char(str, " TABLESPACE \"");
		string_append_char(str, node->tableSpace);
		string_append_char(str, "\"");
	}

	if (node->whereClause)
	{
		string_append_char(str, " WHERE ");
		_outNode(str, node->whereClause);
	}
}

static void
_outNotifyStmt(String *str, NotifyStmt *node)
{
	string_append_char(str, "NOTIFY ");
	_outNode(str, node->relation);
}

static void
_outDeclareCursorStmt(String *str, DeclareCursorStmt *node)
{
	string_append_char(str, "DECLARE \"");
	string_append_char(str, node->portalname);
	string_append_char(str, "\" ");

	if (node->options & CURSOR_OPT_SCROLL)
		string_append_char(str, "SCROLL ");
	if (node->options & CURSOR_OPT_BINARY)
		string_append_char(str, "BINARY ");
	if (node->options & CURSOR_OPT_INSENSITIVE)
		string_append_char(str, "INSENSITIVE ");

	string_append_char(str, "CURSOR ");
	if (node->options & CURSOR_OPT_HOLD)
		string_append_char(str, "WITH HOLD ");
	string_append_char(str, "FOR");
	_outNode(str, node->query);
}

static void
_outSelectStmt(String *str, SelectStmt *node)
{
	if (node->larg) /* SETOP */
	{
		string_append_char(str, "(");
		_outNode(str, node->larg);
		string_append_char(str, ") ");

		switch (node->op)
		{
			case SETOP_UNION:
				string_append_char(str, " UNION ");
				break;
				
			case SETOP_INTERSECT:
				string_append_char(str, " INTERSECT ");
				break;
				
			case SETOP_EXCEPT:
				string_append_char(str, " EXCEPT ");
				
			default:
				break;
		}

		if (node->all)
			string_append_char(str, "ALL ");

		if (node->rarg)
		{
			string_append_char(str, "(");
			_outNode(str, node->rarg);
			string_append_char(str, ") ");
		}
	}
	else if (node->valuesLists) /* VALUES ... */
	{
		ListCell *lc;
		int comma = 0;

		foreach (lc, node->valuesLists)
		{
			if (comma == 0)
				comma = 1;
			else
				string_append_char(str, ",");

			string_append_char(str, " VALUES (");
			_outNode(str, lfirst(lc));
			string_append_char(str, ")");
		}
	}
	else
	{
		if (node->intoClause)
		{
			IntoClause *into = (IntoClause *)node->intoClause;
			RangeVar *rel = (RangeVar *)into->rel;

			string_append_char(str, "CREATE ");
			if (rel->istemp == true)
				string_append_char(str, "TEMP ");
			string_append_char(str, "TABLE ");
			_outNode(str, into->rel);

			if (into->colNames)
			{
				string_append_char(str, " (");
				_outNode(str, into->colNames);
				string_append_char(str, ") ");
			}
			
			if (into->options)
				_outWithDefinition(str, into->options);

			switch (into->onCommit)
			{
				case ONCOMMIT_DROP:
					string_append_char(str, " ON COMMIT DROP");
					break;

				case ONCOMMIT_DELETE_ROWS:
					string_append_char(str, " ON COMMIT DELETE ROWS");
					break;

				case ONCOMMIT_PRESERVE_ROWS:
					string_append_char(str, " ON COMMIT PRESERVE ROWS");
					break;

				default:
					break;
			}

			string_append_char(str, " AS");
		}

		string_append_char(str, " SELECT ");

		if (node->distinctClause)
		{
			string_append_char(str, "DISTINCT ");
			if (lfirst(list_head(node->distinctClause)) != NIL)
			{
				string_append_char(str, "ON (");
				_outNode(str, node->distinctClause);
				string_append_char(str, " ) ");
			}
		}

		_outNode(str, node->targetList);

		if (node->fromClause)
		{
			string_append_char(str, " FROM ");
			_outNode(str, node->fromClause);
		}

		if (node->whereClause)
		{
			string_append_char(str, " WHERE ");
			_outNode(str, node->whereClause);
		}

		if (node->groupClause)
		{
			string_append_char(str, " GROUP BY ");
			_outNode(str, node->groupClause);
		}

		if (node->havingClause)
		{
			string_append_char(str, " HAVING ");
			_outNode(str, node->havingClause);
		}
	}

	if (node->sortClause)
	{
		string_append_char(str, " ORDER BY ");
		_outNode(str, node->sortClause);
	}

	if (node->limitOffset)
	{
		string_append_char(str, " OFFSET ");
		_outNode(str, node->limitOffset);
	}

	if (node->limitCount)
	{
		string_append_char(str, " LIMIT ");
		if (IsA(node->limitCount, A_Const) &&
			((A_Const *)node->limitCount)->val.type == T_Null)
		{
			string_append_char(str, "ALL ");
		}
		else
		{
			_outNode(str, node->limitCount);
		}
	}

	_outNode(str, node->lockingClause);
}

static void
_outFuncCall(String *str, FuncCall *node)
{
	char *funcname;
	_outFuncName(str, node->funcname);

	funcname = strVal(lfirst(list_head(node->funcname)));

    if(strcmp(funcname,"user") == 0 ||
       strcmp(funcname,"current_user") == 0 ||
       strcmp(funcname,"session_user") == 0 ||
       strcmp(funcname,"current_role") == 0)
        return ;

	string_append_char(str, "(");

	if (node->agg_distinct == TRUE)
		string_append_char(str, "DISTINCT ");

	if (node->agg_star == TRUE)
		string_append_char(str, "*");
	else
		_outNode(str, node->args);

	string_append_char(str, ")");
}

static void
_outDefElem(String *str, DefElem *node)
{

}

static void
_outLockingClause(String *str, LockingClause *node)
{
	if (node == NULL)
		return;

	if (node->forUpdate == TRUE)
		string_append_char(str, " FOR UPDATE");
	else
		string_append_char(str, " FOR SHARED");

	_outNode(str, node->lockedRels);

	if (node->noWait == TRUE)
		string_append_char(str, " NOWAIT ");
}

static void
_outColumnDef(String *str, ColumnDef *node)
{
	string_append_char(str, "\"");
	string_append_char(str, node->colname);
	string_append_char(str, "\" ");
	_outNode(str, node->typename);
	_outNode(str, node->constraints);
}

static void
_outTypeName(String *str, TypeName *node)
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
			string_append_char(str, ".");
		if(node->typemod < 0)
		{
			string_append_char(str, "\"");
			string_append_char(str, typename);
			string_append_char(str, "\"");
		} else 
			string_append_char(str, typename);
	}
	
	if (node->typemod > 0)
	{
        int lower;
        char buf[16];
        string_append_char(str, "(");
        snprintf(buf, 16, "%d", ((node->typemod - VARHDRSZ) >> 16) & 0x00FF);
        string_append_char(str, buf);
        lower = (node->typemod-VARHDRSZ) & 0x00FF;

        if(lower != 0)
        {
            char buf2[16];
            string_append_char(str, ",");
            snprintf(buf2, 16, "%d",lower);
            string_append_char(str, buf2);
        }

        string_append_char(str, ")");
	}
}

static void
_outTypeCast(String *str, TypeCast *node)
{
	_outNode(str, node->arg);
	string_append_char(str, "::");
	_outNode(str, node->typename);

}

static void
_outIndexElem(String *str, IndexElem *node)
{
	if (node->name)
	{
		string_append_char(str, "\"");
		string_append_char(str, node->name);
		string_append_char(str, "\"");
		if (node->opclass != NIL)
			_outNode(str, node->opclass);
	}
	else
	{
		string_append_char(str, "(");
		_outNode(str, node->expr);
		string_append_char(str, ")");
		if (node->opclass != NIL)
			_outNode(str, node->opclass);
	}
}


static void
_outSortClause(String *str, SortClause *node)
{

}

static void
_outGroupClause(String *str, GroupClause *node)
{

}

static void
_outSetOperationStmt(String *str, SetOperationStmt *node)
{

}


static void
_outAExpr(String *str, A_Expr *node)
{
	Value *v;

	switch (node->kind)
	{
		case AEXPR_OP:
			if (list_length(node->name) == 1)
			{
				Value *op = (Value *) lfirst(list_head(node->name));

				string_append_char(str, " (");
				_outNode(str, node->lexpr);
				string_append_char(str, op->val.str);
				_outNode(str, node->rexpr);
				string_append_char(str, " )");
			}
			break;

		case AEXPR_AND:
			string_append_char(str, " (");
			_outNode(str, node->lexpr);
			string_append_char(str, " AND ");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");
			break;
				
		case AEXPR_OR:
			string_append_char(str, " (");
			_outNode(str, node->lexpr);
			string_append_char(str, " OR ");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");
			break;

		case AEXPR_NOT:
			string_append_char(str, " (NOT ");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");
			break;

		case AEXPR_OP_ANY:
			_outNode(str, node->lexpr);
			v = linitial(node->name);
			string_append_char(str, v->val.str);
			string_append_char(str, "ANY(");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");
			break;

		case AEXPR_OP_ALL:
			_outNode(str, node->lexpr);
			v = linitial(node->name);
			string_append_char(str, v->val.str);
			string_append_char(str, "ALL(");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");
			break;

		case AEXPR_DISTINCT:
			string_append_char(str, " (");
			_outNode(str, node->lexpr);
			string_append_char(str, " IS DISTINCT FROM ");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");				
			break;

		case AEXPR_NULLIF:
			string_append_char(str, " NULLIF(");
			_outNode(str, node->lexpr);
			string_append_char(str, ", ");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");
			break;

		case AEXPR_OF:
			_outNode(str, node->lexpr);
			v = linitial(node->name);
			if (v->val.str[0] == '!')
				string_append_char(str, " IS NOT OF (");
			else
				string_append_char(str, " IS OF (");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");
			break;

		case AEXPR_IN:
			_outNode(str, node->lexpr);
			v = (Value *)lfirst(list_head(node->name));
			if (v->val.str[0] == '=')
				string_append_char(str, " IN (");
			else
				string_append_char(str, " NOT IN (");
			_outNode(str, node->rexpr);
			string_append_char(str, ")");
			break;

		default:
			break;
	}
}

static void
_outValue(String *str, Value *value)
{
	char buf[16];

	switch (value->type)
	{
		case T_Integer:
			sprintf(buf, "%ld", value->val.ival);
			string_append_char(str, buf);
			break;

		case T_Float:
			string_append_char(str, value->val.str);
			break;

		case T_String:
			string_append_char(str, "'");
			string_append_char(str, escape_string(value->val.str));
			string_append_char(str, "'");
			break;

		case T_Null:
			string_append_char(str, "NULL");
			break;

		default:
			break;
	}
}

static void
_outColumnRef(String *str, ColumnRef *node)
{
	ListCell *c;
	char first = 0;

	foreach (c, node->fields)
	{
		Node *n = (Node *) lfirst(c);

		if (IsA(n, String))
		{
			Value *v = (Value *) lfirst(c);

			if (first == 0)
				first = 1;
			else
				string_append_char(str, ".");

			string_append_char(str, "\"");
			string_append_char(str, v->val.str);
			string_append_char(str, "\"");
		}
	}
}

static void
_outParamRef(String *str, ParamRef *node)
{
	char buf[16];

	snprintf(buf, 16, "%d", node->number);
	string_append_char(str, "$");
	string_append_char(str, buf);
}

static void
_outAConst(String *str, A_Const *node)
{
	char buf[16];

	switch (node->val.type)
	{
		case T_Integer:
			sprintf(buf, "%ld", node->val.val.ival);
			string_append_char(str, buf);
			break;

		case T_Float:
			string_append_char(str, node->val.val.str);
			break;

		case T_String:
			string_append_char(str, "'");
			string_append_char(str, escape_string(node->val.val.str));
			string_append_char(str, "'");
			break;

		case T_Null:
			string_append_char(str, "NULL");
			break;

		default:
			break;
	}
}

static void
_outA_Indices(String *str, A_Indices *node)
{
	string_append_char(str, "[");
	if (node->lidx)
	{
		_outNode(str, node->lidx);
		string_append_char(str, ":");
	}
	_outNode(str, node->uidx);
	string_append_char(str, "]");
}

static void
_outA_Indirection(String *str, A_Indirection *node)
{
	_outNode(str, node->arg);
	_outNode(str, node->indirection);
}

static void
_outResTarget(String *str, ResTarget *node)
{
	if (node->indirection != NIL)
	{
		string_append_char(str, "\"");
		string_append_char(str, node->name);
		string_append_char(str, "\"=");
		_outNode(str, node->val);
	}
	else
	{
		_outNode(str, node->val);

		if (node->name)
		{
			string_append_char(str, " AS ");
			string_append_char(str, "\"");
			string_append_char(str, node->name);
			string_append_char(str, "\" ");
		}
	}
}

static void
_outConstraint(String *str, Constraint *node)
{
	if (node->name)
	{
		string_append_char(str, "CONSTRAINT \"");
		string_append_char(str, node->name);
		string_append_char(str, "\"");
	}

	switch (node->contype)
	{
		case CONSTR_CHECK:
			string_append_char(str, " CHECK (");
			_outNode(str, node->raw_expr);
			string_append_char(str, ")");
			break;

		case CONSTR_UNIQUE:
			string_append_char(str, " UNIQUE");
			if (node->keys)
			{
				string_append_char(str, "(");
				_outIdList(str, node->keys);
				string_append_char(str, ")");
			}

			if (node->options)
			{
				_outWithDefinition(str, node->options);
			}
			
			if (node->indexspace)
			{
				string_append_char(str, " USING INDEX TABLESPACE \"");
				string_append_char(str, node->indexspace);
				string_append_char(str, "\"");
			}
			break;

		case CONSTR_PRIMARY:
			string_append_char(str, " PRIMARY KEY");
			if (node->keys)
			{
				string_append_char(str, "(");
				_outIdList(str, node->keys);
				string_append_char(str, ")");
			}
			if (node->options)
				;

			if (node->indexspace)
			{
				string_append_char(str, " USING INDEX TABLESPACE \"");
				string_append_char(str, node->indexspace);
				string_append_char(str, "\"");
			}
			break;

		case CONSTR_NOTNULL:
			string_append_char(str, " NOT NULL");
			break;

		case CONSTR_NULL:
			string_append_char(str, " NULL");
			break;

		case CONSTR_DEFAULT:
			string_append_char(str, "DEFAULT ");
			_outNode(str, node->raw_expr);
			break;

		default:
			break;
	}
}

static void
_outFkConstraint(String *str, FkConstraint *node)
{
	if (node->constr_name)
	{
		string_append_char(str, "CONSTRAINT \"");
		string_append_char(str, node->constr_name);
		string_append_char(str, "\"");
	}

	if (node->fk_attrs != NIL)
	{
		string_append_char(str, " FOREIGN KEY (");
		_outIdList(str, node->fk_attrs);
		string_append_char(str, ")" );
	}

	string_append_char(str, " REFERENCES ");
	_outNode(str, node->pktable);

	if (node->pk_attrs != NIL)
	{
		string_append_char(str, "(");
		_outIdList(str, node->pk_attrs);
		string_append_char(str, ")");
	}

	switch (node->fk_matchtype)
	{
		case FKCONSTR_MATCH_FULL:
			string_append_char(str, " MATCH FULL");
			break;

		case FKCONSTR_MATCH_PARTIAL:
			string_append_char(str, " MATCH PARTIAL");
			break;

		default:
			break;
	}

	switch (node->fk_upd_action)
	{
		case FKCONSTR_ACTION_RESTRICT:
			string_append_char(str, " ON UPDATE RESTRICT");
			break;

		case FKCONSTR_ACTION_CASCADE:
			string_append_char(str, " ON UPDATE CASCADE");
			break;

		case FKCONSTR_ACTION_SETNULL:
			string_append_char(str, " ON UPDATE SET NULL");
			break;

		case FKCONSTR_ACTION_SETDEFAULT:
			string_append_char(str, " ON UPDATE SET DEFAULT");
			break;

		default:
			break;
	}

	switch (node->fk_del_action)
	{
		case FKCONSTR_ACTION_RESTRICT:
			string_append_char(str, " ON DELETE RESTRICT");
			break;

		case FKCONSTR_ACTION_CASCADE:
			string_append_char(str, " ON DELETE CASCADE");
			break;

		case FKCONSTR_ACTION_SETNULL:
			string_append_char(str, " ON DELETE SET NULL");
			break;

		case FKCONSTR_ACTION_SETDEFAULT:
			string_append_char(str, " ON DELETE SET DEFAULT");
			break;

		default:
			break;
	}

	if (node->deferrable)
		string_append_char(str, " DEFERRABLE");

	if (node->initdeferred)
		string_append_char(str, " INITIALLY DEFERRED");
}


static void
_outSortBy(String *str, SortBy *node)
{
	_outNode(str, node->node);
	
	if (node->sortby_dir == SORTBY_USING)
	{
		string_append_char(str, " USING ");
		_outNode(str, node->useOp);
	}
	else if (node->sortby_dir == SORTBY_DESC)
		string_append_char(str, " DESC ");

	if (node->sortby_nulls == SORTBY_NULLS_FIRST)
		string_append_char(str, " NULLS FIRST ");
	else if (node->sortby_nulls == SORTBY_NULLS_LAST)
		string_append_char(str, " NULLS LAST ");
}

static void _outInsertStmt(String *str, InsertStmt *node)
{
	string_append_char(str, "INSERT INTO ");
	_outNode(str, node->relation);

	if (node->cols == NIL && node->selectStmt == NULL)
		string_append_char(str, " DEFAULT VALUES");

	if (node->cols)
	{
		char comma = 0;
		ListCell *lc;

		string_append_char(str, "(");

		foreach (lc, node->cols)
		{
			ResTarget *node = lfirst(lc);
			if (comma == 0)
				comma = 1;
			else
				string_append_char(str, ", ");

			string_append_char(str, "\"");
			string_append_char(str, node->name);
			string_append_char(str, "\"");
		}
		string_append_char(str, ")");
	}

	if (node->selectStmt)
	{
		_outNode(str, node->selectStmt);
	}

	if (node->returningList)
	{
		string_append_char(str, " RETURNING ");
		_outNode(str, node->returningList);
	}
}

static void _outUpdateStmt(String *str, UpdateStmt *node)
{
	ListCell *lc;
	char comma = 0;

	string_append_char(str, "UPDATE ");

	_outNode(str, node->relation);

	string_append_char(str, " SET ");
	foreach (lc, node->targetList)
	{
		ResTarget *node = lfirst(lc);
		if (comma == 0)
			comma = 1;
		else
			string_append_char(str, ", ");

		string_append_char(str, "\"");
		string_append_char(str, node->name);
		string_append_char(str, "\" = ");
		_outNode(str, node->val);
	}

	if (node->fromClause)
	{
		string_append_char(str, " FROM ");
		_outNode(str, node->fromClause);
	}

	if (node->whereClause)
	{
		string_append_char(str, " WHERE ");
		_outNode(str, node->whereClause);
	}

	if (node->returningList)
	{
		string_append_char(str, " RETURNING ");
		_outNode(str, node->returningList);
	}
}

static void _outDeleteStmt(String *str, DeleteStmt *node)
{
	string_append_char(str, "DELETE FROM ");

	_outNode(str, node->relation);

	if (node->usingClause)
	{
		string_append_char(str, " USING ");
		_outNode(str, node->usingClause);
	}

	if (node->whereClause)
	{
		string_append_char(str, " WHERE ");
		_outNode(str, node->whereClause);
	}

	if (node->returningList)
	{
		string_append_char(str, " RETURNING ");
		_outNode(str, node->returningList);
	}
}

static void _outTransactionStmt(String *str, TransactionStmt *node)
{
	switch (node->kind)
	{
		case TRANS_STMT_BEGIN:
			string_append_char(str, "BEGIN ");
			break;

		case TRANS_STMT_START:
			string_append_char(str, "START TRANSACTION ");
			break;

		case TRANS_STMT_COMMIT:
			string_append_char(str, "COMMIT ");
			break;

		case TRANS_STMT_ROLLBACK:
			string_append_char(str, "ABORT ");
			break;

		case TRANS_STMT_SAVEPOINT:
			string_append_char(str, "SAVEPOINT ");
			break;

		case TRANS_STMT_RELEASE:
			string_append_char(str, "RELEASE ");
			break;

		case TRANS_STMT_ROLLBACK_TO:
			string_append_char(str, "ROLLBACK TO ");
			break;

		case TRANS_STMT_PREPARE:
			string_append_char(str, "PREPARE TRANSACTION ");
			break;

		case TRANS_STMT_COMMIT_PREPARED:
			string_append_char(str, "COMMIT PREPARED ");
			break;

		case TRANS_STMT_ROLLBACK_PREPARED:
			string_append_char(str, "ROLLBACK PREPARED ");
			break;

		default:
			break;
	}

	if (node->options)
		_outSetTransactionModeList(str, node->options);

	if (node->gid)
		string_append_char(str, node->gid);
}


static void _outTruncateStmt(String *str, TruncateStmt *node)
{
	string_append_char(str, "TRUNCATE ");
	_outNode(str, node->relations);
}

static void _outVacuumStmt(String *str, VacuumStmt *node)
{
	if (node->vacuum == true)
		string_append_char(str, "VACUUM ");
	else
		string_append_char(str, "ANALYZE ");

	if (node->full == TRUE)
		string_append_char(str, "FULL ");
	
	if (node->freeze_min_age == 0)
		string_append_char(str, "FREEZE ");

	if (node->verbose == TRUE)
		string_append_char(str, "VERBOSE ");

	if (node->analyze)
		string_append_char(str, "ANALYZE ");

	_outNode(str, node->relation);
	if (node->va_cols)
	{
		string_append_char(str, "(");
		_outIdList(str, node->va_cols);
		string_append_char(str, ") ");
	}
}

static void _outExplainStmt(String *str, ExplainStmt *node)
{
	string_append_char(str, "EXPLAIN ");
	
	if (node->analyze == TRUE)
		string_append_char(str, "ANALYZE ");
	if (node->verbose == TRUE)
		string_append_char(str, "VERBOSE ");

	_outNode(str, node->query);
}

static void _outClusterStmt(String *str, ClusterStmt *node)
{
	string_append_char(str, "CLUSTER ");
	
	if (node->indexname)
	{
		string_append_char(str, "\"");
		string_append_char(str, node->indexname);
		string_append_char(str, "\" ON ");
	}
	if (node->relation)
		_outNode(str, node->relation);
}

static void _outCheckPointStmt(String *str, CheckPointStmt *node)
{
	string_append_char(str, "CHECKPOINT");
}

static void _outClosePortalStmt(String *str, ClosePortalStmt *node)
{
	string_append_char(str, "CLOSE ");
	string_append_char(str, "\"");
	string_append_char(str, node->portalname);
	string_append_char(str, "\"");
}

static void _outListenStmt(String *str, ListenStmt *node)
{
	string_append_char(str, "LISTEN ");
	_outNode(str, node->relation);
}

static void _outUnlistenStmt(String *str, UnlistenStmt *node)
{
	string_append_char(str, "UNLISTEN ");
	_outNode(str, node->relation);
}

static void _outLoadStmt(String *str, LoadStmt *node)
{
	string_append_char(str, "LOAD '");
	string_append_char(str, node->filename);
	string_append_char(str, "'");
}

static void _outCopyStmt(String *str, CopyStmt *node)
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

	foreach (lc, node->options)
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

	string_append_char(str, "COPY ");

	if (node->query)
	{
		string_append_char(str, "(");
		_outNode(str, node->query);
		string_append_char(str, ")");
	}

	_outNode(str, node->relation);

	if (node->attlist)
	{
		string_append_char(str, "(");
		_outIdList(str, node->attlist);
		string_append_char(str, ") ");
	}

	if (node->is_from == TRUE)
		string_append_char(str, " FROM ");
	else
		string_append_char(str, " TO ");

	if (node->filename)
	{
		string_append_char(str, "'");
		string_append_char(str, node->filename);
		string_append_char(str, "'");
	}
	else
		string_append_char(str, node->is_from == TRUE ? "STDIN" : "STDOUT");

	if (binary == TRUE)
		string_append_char(str, " BINARY ");

	if (oids == TRUE)
		string_append_char(str, " OIDS ");

	if (delimiter)
	{
		string_append_char(str, " DELIMITERS '");
		string_append_char(str, delimiter);
		string_append_char(str, "'");
	}

	if (null)
	{
		string_append_char(str, " NULL '");
		string_append_char(str, null);
		string_append_char(str, "' ");
	}

	if (csv == TRUE)
		string_append_char(str, "CSV ");

	if (header == TRUE)
		string_append_char(str, "HEADER ");

	if (quote)
	{
		string_append_char(str, "QUOTE '");
		string_append_char(str, quote);
		string_append_char(str, "' ");
	}

	if (escape)
	{
		string_append_char(str, "ESCAPE '");
		string_append_char(str, escape);
		string_append_char(str, "' ");
	}

	if (force_quote)
	{
		string_append_char(str, "FORCE QUOTE ");
		_outNode(str, force_quote);
	}

	if (force_notnull)
	{
		string_append_char(str, " FORCE NOT NULL ");
		_outNode(str, force_notnull);
	}
}

static void _outDeallocateStmt(String *str, DeallocateStmt *node)
{
	string_append_char(str, "DEALLOCATE \"");
	string_append_char(str, node->name);
	string_append_char(str, "\"");
}

static void _outRenameStmt(String *str, RenameStmt *node)
{
	ListCell *lc;
	char comma = 0;
	
	string_append_char(str, "ALTER ");

	switch (node->renameType)
	{
		case OBJECT_AGGREGATE:
			string_append_char(str, "AGGREGATE ");
			_outNode(str, node->object);
			string_append_char(str, " (");
			string_append_char(str, ") RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_CONVERSION:
			string_append_char(str, "CONVERSION ");
			_outNode(str, node->object);
			string_append_char(str, " RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_DATABASE:
			string_append_char(str, "DATABASE \"");
			string_append_char(str, node->subname);
			string_append_char(str, "\" RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_FUNCTION:
			string_append_char(str, "FUNCTION ");

			foreach (lc, node->object)
			{
				Node *n = lfirst(lc);
				if (IsA(n, String))
				{
					Value *value = (Value *) n;
					if (comma == 0)
						comma = 1;
					else
						string_append_char(str, ".");
					string_append_char(str, "\"");
					string_append_char(str, value->val.str);
					string_append_char(str, "\"");
				}
				else
					_outNode(str, n);
			}

			string_append_char(str, "(");
			_outNode(str, node->objarg);
			string_append_char(str, ")");
			string_append_char(str, " RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_ROLE:
			string_append_char(str, "ROLE \"");
			string_append_char(str, node->subname);
			string_append_char(str, "\" RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_LANGUAGE:
			string_append_char(str, "LANGUAGE \"");
			string_append_char(str, node->subname);
			string_append_char(str, "\" RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_OPCLASS:
			string_append_char(str, "OPERATOR CLASS ");
			_outNode(str, node->object);
			string_append_char(str, " USING ");
			string_append_char(str, node->subname);
			string_append_char(str, " RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_SCHEMA:
			string_append_char(str, "SCHEMA \"");
			string_append_char(str, node->subname);
			string_append_char(str, "\" RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_TABLE:
			string_append_char(str, "TABLE ");
			_outNode(str, node->relation);
			string_append_char(str, " RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;
		
		case OBJECT_INDEX:
			string_append_char(str, "INDEX ");
			_outNode(str, node->relation);
			string_append_char(str, " RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_COLUMN:
			string_append_char(str, "TABLE ");
			_outNode(str, node->relation);
			string_append_char(str, " RENAME \"");
			string_append_char(str, node->subname);
			string_append_char(str, "\" TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_TRIGGER:
			string_append_char(str, "TRIGGER \"");
			string_append_char(str, node->subname);
			string_append_char(str, "\" ON ");
			_outNode(str, node->relation);
			string_append_char(str, " RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		case OBJECT_TABLESPACE:
			string_append_char(str, "TABLESPACE \"");
			string_append_char(str, node->subname);
			string_append_char(str, "\" RENAME TO \"");
			string_append_char(str, node->newname);
			string_append_char(str, "\"");
			break;

		default:
			break;
	}
}

static void
_outOptRoleList(String *str, List *options)
{
	ListCell *lc;
	
	foreach (lc, options)
	{
		DefElem *elem = lfirst(lc);
		Value *value = (Value *) elem->arg;

		if (strcmp(elem->defname, "password") == 0)
		{
			if (value == NULL)
				string_append_char(str, " PASSWORD NULL");
			else
			{
				string_append_char(str, " PASSWORD '");
				string_append_char(str, value->val.str);
				string_append_char(str, "'");
			}
		}
		else if (strcmp(elem->defname, "encryptedPassword") == 0)
		{
			string_append_char(str, " ENCRYPTED PASSWORD '");
			string_append_char(str, value->val.str);
			string_append_char(str, "'");
		}
		else if (strcmp(elem->defname, "unencryptedPassword") == 0)
		{
			string_append_char(str, " UNENCRYPTED PASSWORD '");
			string_append_char(str, value->val.str);
			string_append_char(str, "'");
		}
		else if (strcmp(elem->defname, "superuser") == 0)
		{
			if (value->val.ival == TRUE)
				string_append_char(str, " SUPERUSER");
			else
				string_append_char(str, " NOSUPERUSER");
		}
		else if (strcmp(elem->defname, "inherit") == 0)
		{
			if (value->val.ival == TRUE)
				string_append_char(str, " INHERIT");
			else
				string_append_char(str, " NOINHERIT");
		}
		else if (strcmp(elem->defname, "createdb") == 0)
		{
			if (value->val.ival == TRUE)
				string_append_char(str, " CREATEDB");
			else
				string_append_char(str, " NOCREATEDB");
		}
		else if (strcmp(elem->defname, "createrole") == 0)
		{
			if (value->val.ival == TRUE)
				string_append_char(str, " CREATEROLE");
			else
				string_append_char(str, " NOCREATEROLE");
		}
		else if (strcmp(elem->defname, "canlogin") == 0)
		{
			if (value->val.ival == TRUE)
				string_append_char(str, " LOGIN");
			else
				string_append_char(str, " NOLOGIN");
		}
		else if (strcmp(elem->defname, "connectionlimit") == 0)
		{
			char buf[16];
			
			string_append_char(str, " CONNECTION LIMIT ");
			snprintf(buf, 16, "%ld", value->val.ival);
			string_append_char(str, buf);
		}
		else if (strcmp(elem->defname, "validUntil") == 0)
		{
			string_append_char(str, " VALID UNTIL '");
			string_append_char(str, value->val.str);
			string_append_char(str, "'");
		}
		else if (strcmp(elem->defname, "rolemembers") == 0)
		{
			string_append_char(str, " ROLE ");
			_outIdList(str, (List *) elem->arg);
		}
		else if (strcmp(elem->defname, "sysid") == 0)
		{
			char buf[16];
			
			string_append_char(str, " SYSID ");
			snprintf(buf, 16, "%ld", value->val.ival);
			string_append_char(str, buf);
		}
		else if (strcmp(elem->defname, "adminmembers") == 0)
		{
			string_append_char(str, " ADMIN ");
			_outIdList(str, (List *) elem->arg);
		}
		else if (strcmp(elem->defname, "addroleto") == 0)
		{
			string_append_char(str, " IN ROLE ");
			_outIdList(str, (List *) elem->arg);
		}
	}
}

static void
_outCreateRoleStmt(String *str, CreateRoleStmt *node)
{
	string_append_char(str, "CREATE ");
	switch (node->stmt_type)
	{
		case ROLESTMT_ROLE:
			string_append_char(str, "ROLE \"");
			break;

		case ROLESTMT_USER:
			string_append_char(str, "USER \"");
			break;

		case ROLESTMT_GROUP:
			string_append_char(str, "GROUP \"");
			break;
	}
	string_append_char(str, node->role);
	string_append_char(str, "\"");

	_outOptRoleList(str, node->options);
}

static void
_outAlterRoleStmt(String *str, AlterRoleStmt *node)
{
	string_append_char(str, "ALTER ROLE \"");
	string_append_char(str, node->role);
	string_append_char(str, "\"");

	if (node->options)
		_outOptRoleList(str, node->options);
}

static void
_outAlterRoleSetStmt(String *str, AlterRoleSetStmt *node)
{
	string_append_char(str, "ALTER ROLE \"");
	string_append_char(str, node->role);
	string_append_char(str, "\" ");

	if (node->setstmt)
	{
		_outNode(str, node->setstmt);
	}
}


static void
_outSetTransactionModeList(String *str, List *list)
{
	ListCell *lc;
	char comma = 0;

	foreach (lc, list)
	{
		DefElem *elem = lfirst(lc);

		if (comma == 0)
			comma = 1;
		else
			string_append_char(str, ",");

		if (strcmp(elem->defname, "transaction_isolation") == 0)
		{
			A_Const *v = (A_Const *) elem->arg;
			string_append_char(str, " ISOLATION LEVEL ");
			string_append_char(str, v->val.val.str);
		}
		else if (strcmp(elem->defname, "transaction_read_only") == 0)
		{
			A_Const *n = (A_Const *) elem->arg;
			if (n->val.val.ival == TRUE)
				string_append_char(str, "READ ONLY ");
			else
				string_append_char(str, "READ WRITE ");
		}
	}
}


static void
_outSetRest(String *str, VariableSetStmt *node)
{
	if (strcmp(node->name, "timezone") == 0)
	{
		string_append_char(str, "TIME ZONE ");
		if (node->kind != VAR_RESET)
			_outNode(str, node->args);
	}
	else if (strcmp(node->name, "TRANSACTION") == 0)
	{
		string_append_char(str, "TRANSACTION ");
		_outSetTransactionModeList(str, node->args);
	}
	else if (strcmp(node->name, "SESSION CHARACTERISTICS") == 0)
	{
		string_append_char(str, "SESSION CHARACTERISTICS AS TRANSACTION ");
		_outSetTransactionModeList(str, node->args);
	}
	else if (strcmp(node->name, "role") == 0)
	{
		string_append_char(str, "ROLE ");
		if (node->kind != VAR_RESET)
			_outNode(str, node->args);
	}
	else if (strcmp(node->name, "session_authorization") == 0)
	{
		string_append_char(str, "SESSION AUTHORIZATION ");
		if (node->args == NIL && node->kind != VAR_RESET)
			string_append_char(str, "DEFAULT");
		else
			_outNode(str, node->args);
	}
	else if (strcmp(node->name, "transaction_isolation") == 0)
	{
		string_append_char(str, "TRANSACTION ISOLATION LEVEL");
		if (node->kind != VAR_RESET)
			_outSetTransactionModeList(str, node->args);
	}
	else if (strcmp(node->name, "xmloption") == 0)
	{
		A_Const *v = linitial(node->args);
		string_append_char(str, "XML OPTOIN ");
		string_append_char(str, v->val.val.str);
	}
	else
	{
		string_append_char(str, node->name);
		if (node->kind != VAR_RESET)
		{
			if (node->kind == VAR_SET_CURRENT)
			{
				string_append_char(str, " FROM CURRENT");
			}
			else
			{
				string_append_char(str, " TO ");
				if (node->args == NULL)
				{
					string_append_char(str, "DEFAULT");
				}
				else
					_outNode(str, node->args);
			}
		}
	}
}

static void
_outDropRoleStmt(String *str, DropRoleStmt *node)
{
	string_append_char(str, "DROP ROLE ");
	if (node->missing_ok == TRUE)
		string_append_char(str, "IF EXISTS ");
	_outIdList(str, node->roles);
}

static void
_outCreateSchemaStmt(String *str, CreateSchemaStmt *node)
{
	string_append_char(str, "CREATE SCHEMA \"");
	string_append_char(str, node->schemaname);
	string_append_char(str, "\"");
	if (node->authid)
	{
		string_append_char(str, "AUTHORIZATION \"");
		string_append_char(str, node->authid);
		string_append_char(str, "\" ");
	}
	_outNode(str, node->schemaElts);
}

static void
_outVariableSetStmt(String *str, VariableSetStmt *node)
{
	if (node->kind == VAR_RESET_ALL)
	{
		string_append_char(str, "RESET ALL");
		return;
	}

	if (node->kind == VAR_RESET)
		string_append_char(str, "RESET ");
	else
		string_append_char(str, "SET ");

	if (node->is_local)
		string_append_char(str, "LOCAL ");
	
	_outSetRest(str, node);
}

static void
_outVariableShowStmt(String *str, VariableShowStmt *node)
{
	if (strcmp(node->name, "timezone") == 0)
		string_append_char(str, "SHOW TIME ZONE");
	else if (strcmp(node->name, "transaction_isolation") == 0)
		string_append_char(str, "SHOW TRANSACTION ISOLATION LEVEL");
	else if (strcmp(node->name, "session_authorization") == 0)
		string_append_char(str, "SHOW SESSION AUTHORIZATION");
	else if (strcmp(node->name, "all") == 0)
		string_append_char(str, "SHOW ALL");
	else
	{
		string_append_char(str, "SHOW ");
		string_append_char(str, node->name);
	}
}

static void
_outConstraintsSetStmt(String *str, ConstraintsSetStmt *node)
{
	string_append_char(str, "SET CONSTRAINTS ");

	if (node->constraints == NIL)
		string_append_char(str, "ALL");
	else
		_outNode(str, node->constraints);

	string_append_char(str, node->deferred == TRUE ? " DEFERRED" : " IMMEDIATE");
}

static void
_outAlterTableCmd(String *str, AlterTableCmd *node)
{
	char buf[16];
	
	switch (node->subtype)
	{
		case AT_AddColumn:
			string_append_char(str, "ADD ");
			_outNode(str, node->def);
			break;

		case AT_ColumnDefault:
			string_append_char(str, "ALTER \"");
			string_append_char(str, node->name);
			string_append_char(str, "\" ");
			if (node->def == NULL)
				string_append_char(str, "DROP DEFAULT");
			else
			{
				string_append_char(str, "SET DEFAULT ");
				_outNode(str, node->def);
			}
			break;

		case AT_DropNotNull:
			string_append_char(str, "ALTER \"");
			string_append_char(str, node->name);
			string_append_char(str, "\" DROP NOT NULL");
			break;

		case AT_SetNotNull:
			string_append_char(str, "ALTER \"");
			string_append_char(str, node->name);
			string_append_char(str, "\" SET NOT NULL");
			break;

		case AT_SetStatistics:
			string_append_char(str, "ALTER \"");
			string_append_char(str, node->name);
			string_append_char(str, "\" SET STATISTICS ");
			snprintf(buf, 16, "%ld", ((Value *) node->def)->val.ival);
			string_append_char(str, buf);
			break;

		case AT_SetStorage:
			string_append_char(str, "ALTER \"");
			string_append_char(str, node->name);
			string_append_char(str, "\" SET STORAGE ");
			string_append_char(str, ((Value *) node->def)->val.str);
			break;

		case AT_DropColumn:
			string_append_char(str, "DROP \"");
			string_append_char(str, node->name);
			string_append_char(str, "\" ");
			if (node->behavior == DROP_CASCADE)
				string_append_char(str, "CASCADE");
			break;

		case AT_AlterColumnType:
			string_append_char(str, "ALTER \"");
			string_append_char(str, node->name);
			string_append_char(str, "\" TYPE ");
			_outNode(str, node->def);
			if (node->transform)
			{
				string_append_char(str, " USING ");
				_outNode(str, node->transform);
			}
			break;

		case AT_AddConstraint:
			string_append_char(str, "ADD ");
			_outNode(str, node->def);
			break;

		case AT_DropConstraint:
			string_append_char(str, "DROP CONSTRAINT \"");
			string_append_char(str, node->name);
			string_append_char(str, "\"");
			if (node->behavior == DROP_CASCADE)
				string_append_char(str, " CASCADE");
			break;

		case AT_DropOids:
			string_append_char(str, "SET WITHOUT OIDS");
			break;

		case AT_ClusterOn:
			string_append_char(str, "CLUSTER ON \"");
			string_append_char(str, node->name);
			string_append_char(str, "\"");
			break;

		case AT_DropCluster:
			string_append_char(str, "SET WITHOUT CLUSTER");
			break;

		case AT_EnableTrig:
			string_append_char(str, "ENABLE TRIGGER \"");
			string_append_char(str, node->name);
			string_append_char(str, "\"");
			break;

		case AT_EnableAlwaysTrig:
			/* not implemented */
			break;

		case AT_EnableReplicaTrig:
			/* not implemented */
			break;

		case AT_EnableTrigAll:
			string_append_char(str, "ENABLE TRIGGER ALL");
			break;

		case AT_EnableTrigUser:
			string_append_char(str, "ENABLE TRIGGER USER");
			break;

		case AT_DisableTrig:
			string_append_char(str, "DISABLE TRIGGER \"");
			string_append_char(str, node->name);
			string_append_char(str, "\"");
			break;

		case AT_DisableTrigAll:
			string_append_char(str, "DISABLE TRIGGER ALL");
			break;

		case AT_DisableTrigUser:
			string_append_char(str, "DISABLE TRIGGER USER");
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

		case AT_ChangeOwner:
			string_append_char(str, "OWNER TO \"");
			string_append_char(str, node->name);
			string_append_char(str, "\"");
			break;

		case AT_SetTableSpace:
			string_append_char(str, "SET TABLESPACE \"");
			string_append_char(str, node->name);
			string_append_char(str, "\"");
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
_outAlterTableStmt(String *str, AlterTableStmt *node)
{
	if (node->relkind == OBJECT_TABLE)
		string_append_char(str, "ALTER TABLE ");
	else
		string_append_char(str, "ALTER INDEX ");

	_outNode(str, node->relation);
	string_append_char(str, " ");
	_outNode(str, node->cmds);
}

static void
_outOptSeqList(String *str, List *options)
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
				string_append_char(str, " CYCLE");
			else
				string_append_char(str, " NO CYCLE");
		}
		else if (strcmp(e->defname, "minvalue") == 0 && !v)
			string_append_char(str, " NO MINVALUE");
		else if (strcmp(e->defname, "maxvalue") == 0 && !v)
			string_append_char(str, " NO MAXVALUE");
		else if (strcmp(e->defname, "owned_by") == 0)
		{
			string_append_char(str, " OWNED BY ");
			_outIdList(str, (List *)e->arg);
		}
		else
		{
			if (strcmp(e->defname, "cache") == 0)
				string_append_char(str, " CACHE ");
			else if (strcmp(e->defname, "increment") == 0)
				string_append_char(str, " INCREMENT ");
			else if (strcmp(e->defname, "maxvalue") == 0 && v)
				string_append_char(str, " MAXVALUE ");
			else if (strcmp(e->defname, "minvalue") == 0 && v)
				string_append_char(str, " MINVALUE ");
			else if (strcmp(e->defname, "start") == 0)
				string_append_char(str, " START ");
			else if (strcmp(e->defname, "restart") == 0)
				string_append_char(str, " RESTART ");
			
			if (IsA(e->arg, String))
				string_append_char(str, v->val.str);
			else
			{
				snprintf(buf, 16, "%ld", v->val.ival);
				string_append_char(str, buf);
			}
		}
	}
}

static void
_outCreateSeqStmt(String *str, CreateSeqStmt *node)
{
	string_append_char(str, "CREATE ");
	if (node->sequence->istemp)
		string_append_char(str, "TEMP ");
	string_append_char(str, "SEQUENCE ");
	_outNode(str, node->sequence);

	_outOptSeqList(str, node->options);
}

static void
_outAlterSeqStmt(String *str, AlterSeqStmt *node)
{
	string_append_char(str, "ALTER SEQUENCE ");
	_outNode(str, node->sequence);
	_outOptSeqList(str, node->options);
}

static void
_outCreatePLangStmt(String *str, CreatePLangStmt *node)
{
	string_append_char(str, "CREATE ");
	if (node->pltrusted == true)
		string_append_char(str, "TRUSTED ");
	string_append_char(str, "LANGUAGE \"");
	string_append_char(str, node->plname);
	string_append_char(str, "\"");

	if (node->plhandler != NIL)
	{
		ListCell *lc;
		char dot = 0;

		string_append_char(str, " HANDLER ");
		foreach (lc, node->plhandler)
		{
			Value *v = lfirst(lc);
			
			if (dot == 0)
				dot = 1;
			else
				string_append_char(str, ".");

			string_append_char(str, "\"");
			string_append_char(str, v->val.str);
			string_append_char(str, "\"");
		}
	}

	if (node->plvalidator != NIL)
	{
		ListCell *lc;
		char dot = 0;

		string_append_char(str, " VALIDATOR ");
		foreach (lc, node->plvalidator)
		{
			Value *v = lfirst(lc);
			
			if (dot == 0)
				dot = 1;
			else
				string_append_char(str, ".");

			string_append_char(str, "\"");
			string_append_char(str, v->val.str);
			string_append_char(str, "\"");
		}
	}
}

static void
_outDropPLangStmt(String *str, DropPLangStmt *node)
{
	string_append_char(str, "DROP LANGUAGE \"");
	string_append_char(str, node->plname);
	string_append_char(str, "\"");
	if (node->behavior == DROP_CASCADE)
		string_append_char(str, " CASCADE");
}

static void
_outCreateTableSpaceStmt(String *str, CreateTableSpaceStmt *node)
{
	string_append_char(str, "CREATE TABLESPACE \"");
	string_append_char(str, node->tablespacename);
	string_append_char(str, "\" ");

	if (node->owner)
	{
		string_append_char(str, "OWNER \"");
		string_append_char(str, node->owner);
		string_append_char(str, "\" ");
	}

	string_append_char(str, "LOCATION '");
	string_append_char(str, node->location);
	string_append_char(str, "'");
}

static void
_outDropTableSpaceStmt(String *str, DropTableSpaceStmt *node)
{
	string_append_char(str, "DROP TABLESPACE \"");
	string_append_char(str, node->tablespacename);
	string_append_char(str, "\"");
}

static void
_outFuncName(String *str, List *func_name)
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
			string_append_char(str, ".");

		if (IsA(v, String))
		{
			string_append_char(str, "\"");
			string_append_char(str, v->val.str);
			string_append_char(str, "\"");
		}
		else
		{
			_outNode(str, v);
		}
	}
}

static void
_outCreateTrigStmt(String *str, CreateTrigStmt *node)
{
	int i, len;

	if (node->isconstraint == TRUE)
		string_append_char(str, "CREATE CONSTRAINT TRIGGER \"");
	else
		string_append_char(str, "CREATE TRIGGER \"");
	string_append_char(str, node->trigname);
	string_append_char(str, "\" ");

	if (node->before == TRUE)
		string_append_char(str, "BEFORE ");
	else
		string_append_char(str, "AFTER ");

	len = strlen(node->actions);
	for (i = 0; i < len; i++)
	{
		if (i)
			string_append_char(str, "OR ");
		
		if (node->actions[i] == 'i')
			string_append_char(str, "INSERT ");
		else if (node->actions[i] == 'd')
			string_append_char(str, "DELETE ");
		else
			string_append_char(str, "UPDATE ");
	}

	string_append_char(str, "ON ");
	_outNode(str, node->relation);

	if (node->constrrel)
	{
		string_append_char(str, " FROM ");
		_outNode(str, node->constrrel);
	}

	if (node->deferrable)
		string_append_char(str, " DEFERRABLE");
	if (node->initdeferred)
		string_append_char(str, " INITIALLY DEFERRED");

	if (node->row == TRUE)
		string_append_char(str, " FOR EACH ROW ");
	else
		string_append_char(str, " FOR EACH STATEMENT ");

	string_append_char(str, "EXECUTE PROCEDURE ");

	_outFuncName(str, node->funcname);
	string_append_char(str, "(");
	_outNode(str, node->args);
	string_append_char(str, ")");
}

static void
_outDropPropertyStmt(String *str, DropPropertyStmt *node)
{
	switch (node->removeType)
	{
		case OBJECT_TRIGGER:
			string_append_char(str, "DROP TRIGGER \"");
			string_append_char(str, node->property);
			string_append_char(str, "\" ON ");
			_outNode(str, node->relation);
			if (node->behavior == DROP_CASCADE)
				string_append_char(str, " CASCADE");
			break;

		case OBJECT_RULE:
			string_append_char(str, "DROP RULE \"");
			string_append_char(str, node->property);
			string_append_char(str, "\" ON ");
			_outNode(str, node->relation);
			if (node->behavior == DROP_CASCADE)
				string_append_char(str, " CASCADE");
			break;

		default:
			break;
	}
}

static void
_outDefinition(String *str, List *definition)
{
	ListCell *lc;
	char comma = 0;

	if (definition == NIL)
		return;
	
	string_append_char(str, "(");
	foreach (lc, definition)
	{
		DefElem *e = lfirst(lc);

		if (comma == 0)
			comma = 1;
		else
			string_append_char(str, ", ");

		string_append_char(str, "\"");
		string_append_char(str, e->defname);
		string_append_char(str, "\"");

		if (e->arg)
		{
			string_append_char(str, "=");
			_outNode(str, e->arg);
		}
	}
	string_append_char(str, ")");
}

static void
_outDefineStmt(String *str, DefineStmt *node)
{
	ListCell *lc;
	char dot = 0;

	switch (node->kind)
	{
		case OBJECT_AGGREGATE:
			string_append_char(str, "CREATE AGGREGATE ");
			_outFuncName(str, node->defnames);
			string_append_char(str, " ");
			_outDefinition(str, node->definition);
			break;

		case OBJECT_OPERATOR:
			string_append_char(str, "CREATE OPERATOR ");
			
			foreach (lc, node->defnames)
			{
				Value *v = lfirst(lc);
				
				if (dot == 0)
					dot = 1;
				else
					string_append_char(str, ".");

				string_append_char(str, v->val.str);
			}
			
			string_append_char(str, " ");
			_outDefinition(str, node->definition);
			break;

		case OBJECT_TYPE:
			string_append_char(str, "CREATE TYPE");
			_outFuncName(str, node->defnames);
			string_append_char(str, " ");
			_outDefinition(str, node->definition);
			break;

		case OBJECT_TSPARSER:
			string_append_char(str, "CREATE TEXT SEARCH PARSER ");
			_outIdList(str, node->defnames);
			_outDefinition(str, node->definition);
			break;

		case OBJECT_TSDICTIONARY:
			string_append_char(str, "CREATE TEXT SEARCH DICTIONARY ");
			_outIdList(str, node->defnames);
			_outDefinition(str, node->definition);
			break;

		case OBJECT_TSTEMPLATE:
			string_append_char(str, "CREATE TEXT SEARCH TEMPLATE ");
			_outIdList(str, node->defnames);
			_outDefinition(str, node->definition);
			break;

		case OBJECT_TSCONFIGURATION:
			string_append_char(str, "CREATE TEXT SEARCH CONFIGURATION ");
			_outIdList(str, node->defnames);
			_outDefinition(str, node->definition);
			break;
			
		default:
			break;
	}
}

static void
_outOperatorName(String *str, List *list)
{
	char dot = 0;
	ListCell *lc;

	foreach (lc, list)
	{
		Value *v = lfirst(lc);
		
		if (dot == 0)
			dot = 1;
		else
			string_append_char(str, ".");
		
		string_append_char(str, v->val.str);
	}
}

static void
_outCreateOpClassItem(String *str, CreateOpClassItem *node)
{
	char buf[16];
	
	switch (node->itemtype)
	{
		case OPCLASS_ITEM_OPERATOR:
			string_append_char(str, "OPERATOR ");
			snprintf(buf, 16, "%d", node->number);
			string_append_char(str, buf);
			string_append_char(str, " ");
			_outOperatorName(str, node->name);

			if (node->args != NIL)
			{
				string_append_char(str, "(");
				_outNode(str, node->args);
				string_append_char(str, ")");
			}
			if (node->recheck == TRUE)
				string_append_char(str, " RECHECK");
			break;

		case OPCLASS_ITEM_FUNCTION:
			string_append_char(str, "FUNCTION ");
			snprintf(buf, 16, "%d", node->number);
			string_append_char(str, buf);
			string_append_char(str, " ");
			_outFuncName(str, node->name);
			string_append_char(str, "(");
			_outNode(str, node->args);
			string_append_char(str, ")");
			break;

		case OPCLASS_ITEM_STORAGETYPE:
			string_append_char(str, "STORAGE ");
			_outNode(str, node->storedtype);
			break;

		default:
			break;
	}
			
}

static void
_outCreateOpClassStmt(String *str, CreateOpClassStmt *node)
{
	string_append_char(str, "CREATE OPERATOR CLASS ");
	_outFuncName(str, node->opclassname);

	if (node->isDefault == TRUE)
		string_append_char(str, " DEFAULT");

	string_append_char(str, " FOR TYPE ");
	_outNode(str, node->datatype);
	string_append_char(str, " USING ");
	string_append_char(str, node->amname);
	string_append_char(str, " AS ");
	_outNode(str, node->items);
}

static void
_outRemoveOpClassStmt(String *str, RemoveOpClassStmt *node)
{
	string_append_char(str, "DROP OPERATOR CLASS ");
	_outFuncName(str, node->opclassname);
	string_append_char(str, " USING ");
	string_append_char(str, node->amname);
	if (node->behavior == DROP_CASCADE)
		string_append_char(str, " CASCADE");
}

static void
_outDropStmt(String *str, DropStmt *node)
{
	ListCell *lc;
	char comma = 0;
	
	string_append_char(str, "DROP ");
	switch (node->removeType)
	{
		case OBJECT_TABLE:
			string_append_char(str, "TABLE ");
			break;

		case OBJECT_SEQUENCE:
			string_append_char(str, "SEQUENCE ");
			break;

		case OBJECT_VIEW:
			string_append_char(str, "VIEW ");
			break;

		case OBJECT_INDEX:
			string_append_char(str, "INDEX ");
			break;

		case OBJECT_TYPE:
			string_append_char(str, "TYPE ");
			break;

		case OBJECT_DOMAIN:
			string_append_char(str, "DOMAIN ");
			break;

		case OBJECT_CONVERSION:
			string_append_char(str, "CONVERSION ");
			break;

		case OBJECT_SCHEMA:
			string_append_char(str, "SCHEMA ");
			break;

		default:
			break;
	}

	foreach (lc, node->objects)
	{
		if (comma == 0)
			comma = 1;
		else
			string_append_char(str, ", ");
		_outFuncName(str, lfirst(lc));
	}

	if (node->behavior == DROP_CASCADE)
		string_append_char(str, " CASCADE");
}

static void
_outFetchStmt(String *str, FetchStmt *node)
{
	char buf[16];

	snprintf(buf, 16, "%ld", node->howMany);

	if (node->ismove == TRUE)
		string_append_char(str, "MOVE ");
	else
		string_append_char(str, "FETCH ");

	switch (node->direction)
	{
		case FETCH_FORWARD:
			string_append_char(str, "FORWARD ");
			if (node->howMany == FETCH_ALL)
				string_append_char(str, "ALL ");
			else
			{
				string_append_char(str, buf);
				string_append_char(str, " ");
			}
			break;

		case FETCH_BACKWARD:
			string_append_char(str, "BACKWARD ");
			if (node->howMany == FETCH_ALL)
				string_append_char(str, "ALL ");
			else
			{
				string_append_char(str, buf);
				string_append_char(str, " ");
			}
			break;

		case FETCH_ABSOLUTE:
			if (node->howMany == 1)
				string_append_char(str, "FIRST ");
			else if (node->howMany == -1)
				string_append_char(str, "LAST ");
			else
			{
				string_append_char(str, "ABSOLUTE ");
				string_append_char(str, buf);
				string_append_char(str, " ");
			}
			break;

		case FETCH_RELATIVE:
			string_append_char(str, "RELATIVE ");
			string_append_char(str, buf);
			string_append_char(str, " ");
			break;
	}

	string_append_char(str, "IN \"");
	string_append_char(str, node->portalname);
	string_append_char(str, "\"");
}

static void
_outPrivilegeList(String *str, List *list)
{
	ListCell *lc;
	char comma = 0;
	
	if (list == NIL)
		string_append_char(str, "ALL");
	else
	{
		foreach (lc, list)
		{
			Value *v = lfirst(lc);
			
			if (comma == 0)
				comma = 1;
			else
				string_append_char(str, ", ");

			string_append_char(str, v->val.str);
		}
	}
}

static void
_outFunctionParameter(String *str, FunctionParameter *node)
{
	switch (node->mode)
	{
		case FUNC_PARAM_OUT:
			string_append_char(str, "OUT ");
			break;

		case FUNC_PARAM_INOUT:
			string_append_char(str, "INOUT ");
			break;

		default:
			break;
	}

	/* function name */
	if (node->name)
	{
		string_append_char(str, "\"");
		string_append_char(str, node->name);
		string_append_char(str, "\" ");
	}

	_outNode(str, node->argType);
}

static void
_outFuncWithArgs(String *str, FuncWithArgs *node)
{
	_outFuncName(str, node->funcname);
	string_append_char(str, "(");
	_outNode(str, node->funcargs);
	string_append_char(str, ")");
}

static void
_outPrivTarget(String *str, PrivTarget *node)
{
	switch (node->objtype)
	{
		case ACL_OBJECT_RELATION:
			_outNode(str, node->objs);
			break;

		case ACL_OBJECT_SEQUENCE:
			string_append_char(str, "SEQUENCE ");
			_outNode(str, node->objs);
			break;

		case ACL_OBJECT_FUNCTION:
			string_append_char(str, "FUNCTION ");
			_outNode(str, node->objs);
			break;

		case ACL_OBJECT_DATABASE:
			string_append_char(str, "DATABASE ");
			_outIdList(str, node->objs);
			break;

		case ACL_OBJECT_LANGUAGE:
			string_append_char(str, "LANGUAGE ");
			_outIdList(str, node->objs);
			break;

		case ACL_OBJECT_NAMESPACE:
			string_append_char(str, "SCHEMA ");
			_outIdList(str, node->objs);
			break;

		case ACL_OBJECT_TABLESPACE:
			string_append_char(str, "TABLESPACE ");
			_outIdList(str, node->objs);
			break;
	}
}

static void
_outPrivGrantee(String *str, PrivGrantee *node)
{
	if (node->rolname == NULL)
		string_append_char(str, "PUBLIC");
	else
	{
		string_append_char(str, "\"");
		string_append_char(str, node->rolname);
		string_append_char(str, "\"");
	}
}

static void
_outGrantStmt(String *str, GrantStmt *node)
{
	PrivTarget *n;
	
	if (node->is_grant == true)
		string_append_char(str, "GRANT ");
	else
	{
		string_append_char(str, "REVOKE ");
		if (node->grant_option == true)
			string_append_char(str, "GRANT OPTION FOR ");
	}
	
	_outPrivilegeList(str, node->privileges);

	string_append_char(str, " ON ");

	n = makeNode(PrivTarget);
	n->objtype = node->objtype;
	n->objs = node->objects;
	_outNode(str, n);
	pfree(n);

	if (node->is_grant == true)
		string_append_char(str, " TO ");
	else
		string_append_char(str, " FROM ");
	_outNode(str, node->grantees);

	if (node->is_grant == true && node->grant_option == TRUE)
		string_append_char(str, " WITH GRANT OPTION");

	if (node->behavior == DROP_CASCADE)
		string_append_char(str, " CASCADE");
}

static void
_outGrantRoleStmt(String *str, GrantRoleStmt *node)
{
	if (node->is_grant == true)
		string_append_char(str, "GRANT ");
	else
	{
		string_append_char(str, "REVOKE ");
		if (node->admin_opt == true)
			string_append_char(str, "ADMIN OPTION FOR ");
	}

	_outIdList(str, node->granted_roles);

	string_append_char(str, node->is_grant == true ? " TO " : " FROM ");

	_outIdList(str, node->grantee_roles);

	if (node->admin_opt == true && node->is_grant == true)
		string_append_char(str, "  WITH ADMIN OPTION");

	if (node->grantor != NULL)
	{
		string_append_char(str, " GRANTED BY \"");
		string_append_char(str, node->grantor);
		string_append_char(str, "\"");
	}

	if (node->behavior == DROP_CASCADE)
		string_append_char(str, " CASCADE");
}

static void
_outFuncOptList(String *str, List *list)
{
	ListCell *lc;

	foreach (lc, list)
	{
		DefElem *e = lfirst(lc);
		Value *v = (Value *) e->arg;
		
		if (strcmp(e->defname, "strict") == 0)
		{
			if (v->val.ival == TRUE)
				string_append_char(str, " STRICT");
			else
				string_append_char(str, " CALLED ON NULL INPUT");
		}
		else if (strcmp(e->defname, "volatility") == 0)
		{
			char *s = v->val.str;
			if (strcmp(s, "immutable") == 0)
				string_append_char(str, " IMMUTABLE");
			else if (strcmp(s, "stable") == 0)
				string_append_char(str, " STABLE");
			else if (strcmp(s, "volatile") == 0)
				string_append_char(str, " VOLATILE");
		}
		else if (strcmp(e->defname, "security") == 0)
		{
			if (v->val.ival == TRUE)
				string_append_char(str, " SECURITY DEFINER");
			else
				string_append_char(str, " SECURITY INVOKER");
		}
		else if (strcmp(e->defname, "as") == 0)
		{
			string_append_char(str, " AS ");
			_outNode(str, e->arg);
		}
		else if (strcmp(e->defname, "language") == 0)
		{
			string_append_char(str, " LANGUAGE '");
			string_append_char(str, v->val.str);
			string_append_char(str, "'");
		}
	}
}

static void
_outCreateFunctionStmt(String *str, CreateFunctionStmt *node)
{
	string_append_char(str, "CREATE ");
	if (node->replace == true)
		string_append_char(str, "OR REPLACE ");
	string_append_char(str, "FUNCTION ");

	_outFuncName(str, node->funcname);

	string_append_char(str, " (");
	_outNode(str, node->parameters);
	string_append_char(str, ")");

	if (node->returnType)
	{
		string_append_char(str, " RETURNS ");
		_outNode(str, node->returnType);
	}

	_outFuncOptList(str, node->options);

	if (node->withClause)
	{
		string_append_char(str, " WITH ");
		_outDefinition(str, node->withClause);
	}
}

static void
_outAlterFunctionStmt(String *str, AlterFunctionStmt *node)
{
	string_append_char(str, "ALTER FUNCTION ");
	_outNode(str, node->func);
	_outFuncOptList(str, node->actions);
}

static void
_outRemoveFuncStmt(String *str, RemoveFuncStmt *node)
{
	switch (node->kind)
	{
		case OBJECT_FUNCTION:
			string_append_char(str, "DROP FUNCTION ");
			break;


		case OBJECT_AGGREGATE:
			string_append_char(str, "DROP AGGREGATE ");
			break;

		case OBJECT_OPERATOR:
			string_append_char(str, "DROP OPERATOR CLASS ");
			break;

		default:
			break;
	}

	if (node->missing_ok)
		string_append_char(str, "IF EXISTS ");

	_outFuncName(str, node->name);

	string_append_char(str, " (");
	_outNode(str, node->args);
	string_append_char(str, ")");

	if (node->behavior == DROP_CASCADE)
		string_append_char(str, " CASCADE");
}

static void
_outCreateCastStmt(String *str, CreateCastStmt *node)
{
	string_append_char(str, "CREATE CAST (");
	_outNode(str, node->sourcetype);
	string_append_char(str, " AS ");
	_outNode(str, node->targettype);
	string_append_char(str, ") WITH FUNCTION ");
	_outNode(str, node->func);
	
	switch (node->context)
	{
		case COERCION_IMPLICIT:
			string_append_char(str, " AS IMPLICIT");
			break;

		case COERCION_ASSIGNMENT:
			string_append_char(str, " AS ASSIGNMENT");
			break;

		default:
			break;
	}
}

static void
_outDropCastStmt(String *str, DropCastStmt *node)
{
	string_append_char(str, "DROP CAST (");
	_outNode(str, node->sourcetype);
	string_append_char(str, " AS ");
	_outNode(str, node->targettype);
	string_append_char(str, ")");

	if (node->behavior == DROP_CASCADE)
		string_append_char(str, " CASCADE");
}

static void
_outReindexStmt(String *str, ReindexStmt *node)
{
	string_append_char(str, "REINDEX ");

	switch (node->kind)
	{
		case OBJECT_DATABASE:
			if (node->do_system == true && node->do_user == false)
				string_append_char(str, "SYSTEM ");
			else
				string_append_char(str, "DATABASE ");
			break;

		case OBJECT_INDEX:
			string_append_char(str, "INDEX ");
			break;

		case OBJECT_TABLE:
			string_append_char(str, "TABLE ");
			break;

		default:
			break;
	}

	if (node->relation)
		_outNode(str, node->relation);

	if (node->name)
	{
		string_append_char(str, "\"");
		string_append_char(str, (char *) node->name);
		string_append_char(str, "\"");
	}
}

static void
_outAlterObjectSchemaStmt(String *str, AlterObjectSchemaStmt *node)
{
	string_append_char(str, "ALTER ");

	switch (node->objectType)
	{
		case OBJECT_AGGREGATE:
			string_append_char(str, "AGGREGATE ");
			_outFuncName(str, node->object);
			string_append_char(str, "(");
			if (lfirst(list_head(node->objarg)) == NULL)
				string_append_char(str, "*");
			else
				_outNode(str, lfirst(list_head(node->objarg)));
			string_append_char(str, ") SET SCHAME \"");
			string_append_char(str, node->newschema);
			string_append_char(str, "\"");
			break;

		case OBJECT_DOMAIN:
			string_append_char(str, "DOMAIN ");
			_outFuncName(str, node->object);
			string_append_char(str, " SET SCHEMA \"");
			string_append_char(str, node->newschema);
			string_append_char(str, "\"");
			break;

		case OBJECT_FUNCTION:
			string_append_char(str, "FUNCTION ");
			_outFuncName(str, node->object);
			string_append_char(str, "(");
			_outNode(str, node->objarg);
			string_append_char(str, ") SET SCHEMA \"");
			string_append_char(str, node->newschema);
			string_append_char(str, "\"");
			break;

		case OBJECT_SEQUENCE:
			string_append_char(str, "SEQUENCE ");
			_outNode(str, node->relation);
			string_append_char(str, " SET SCHEMA \"");
			string_append_char(str, node->newschema);
			string_append_char(str, "\"");
			break;

		case OBJECT_TABLE:
			string_append_char(str, "TABLE ");
			_outNode(str, node->relation);
			string_append_char(str, " SET SCHEMA \"");
			string_append_char(str, node->newschema);
			string_append_char(str, "\"");
			break;

		case OBJECT_TYPE:
			string_append_char(str, "TYPE ");
			_outFuncName(str, node->object);
			string_append_char(str, " SET SCHEMA \"");
			string_append_char(str, node->newschema);
			string_append_char(str, "\"");
			break;

		default:
			break;
	}
}

static void
_outAlterOwnerStmt(String *str, AlterOwnerStmt *node)
{
	string_append_char(str, "ALTER ");

	switch (node->objectType)
	{
		case OBJECT_AGGREGATE:
			string_append_char(str, "AGGREGATE ");
			_outFuncName(str, node->object);
			string_append_char(str, "(");
			if (lfirst(list_head(node->objarg)) == NULL)
				string_append_char(str, "*");
			else
				_outNode(str, lfirst(list_head(node->objarg)));
			string_append_char(str, ") OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		case OBJECT_CONVERSION:
			string_append_char(str, "CONVERSION ");
			_outFuncName(str, node->object);
			string_append_char(str, " OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		case OBJECT_DATABASE:
			string_append_char(str, "DATABASE \"");
			_outIdList(str, node->object);
			string_append_char(str, "\" OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		case OBJECT_DOMAIN:
			string_append_char(str, "DOMAIN ");
			_outFuncName(str, node->object);
			string_append_char(str, " OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		case OBJECT_FUNCTION:
			string_append_char(str, "FUNCTION ");
			_outFuncName(str, node->object);
			string_append_char(str, "(");
			_outNode(str, node->objarg);
			string_append_char(str, ") OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		case OBJECT_OPERATOR:
			string_append_char(str, "OPERATOR ");
			_outOperatorName(str, node->object);
			string_append_char(str, "(");
			_outOperatorArgTypes(str, node->objarg);
			string_append_char(str, ") OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");			
			break;

		case OBJECT_OPCLASS:
			string_append_char(str, "OPERATOR CLASS ");
			_outFuncName(str, node->object);
			string_append_char(str, " USING ");
			string_append_char(str, node->addname);
			string_append_char(str, " OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		case OBJECT_SCHEMA:
			string_append_char(str, "SCHEMA \"");
			string_append_char(str, linitial(node->object));
			string_append_char(str, "\" OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		case OBJECT_TYPE:
			string_append_char(str, "TYPE ");
			_outFuncName(str, node->object);
			string_append_char(str, " OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		case OBJECT_TABLESPACE:
			string_append_char(str, "TABLESPACE \"");
			string_append_char(str, linitial(node->object));
			string_append_char(str, "\" OWNER TO \"");
			string_append_char(str, node->newowner);
			string_append_char(str, "\"");
			break;

		default:
			break;
	}
}

static void
_outRuleStmt(String *str, RuleStmt *node)
{
	string_append_char(str, "CREATE ");
	if (node->replace)
		string_append_char(str, "OR REPLACE ");
	string_append_char(str, "RULE \"");
	string_append_char(str, node->rulename);
	string_append_char(str, "\" AS ON ");

	switch (node->event)
	{
		case CMD_SELECT:
			string_append_char(str, "SELECT");
			break;

		case CMD_UPDATE:
			string_append_char(str, "UPDATE");
			break;

		case CMD_DELETE:
			string_append_char(str, "DELETE");
			break;

		case CMD_INSERT:
			string_append_char(str, "INSERT");
			break;

		default:
			break;
	}

	string_append_char(str, " TO ");
	_outNode(str, node->relation);
	
	if (node->whereClause)
	{
		string_append_char(str, " WHERE ");
		_outNode(str, node->whereClause);
	}

	string_append_char(str, " DO ");

	if (node->instead)
		string_append_char(str, "INSTEAD ");

	if (node->actions == NIL)
		string_append_char(str, "NOTHING");
	else if (list_length(node->actions) == 1)
		_outNode(str, linitial(node->actions));
	else
	{
		ListCell *lc;
		char semi = 0;

		string_append_char(str, "(");

		foreach (lc, node->actions)
		{
			if (semi == 0)
				semi = 1;
			else
				string_append_char(str, ";");
			
			_outNode(str, lfirst(lc));
		}

		string_append_char(str, ")");
	}
}

static void
_outViewStmt(String *str, ViewStmt *node)
{
	if (node->replace)
		string_append_char(str, "CREATE OR REPLACE ");
	else
		string_append_char(str, "CREATE ");

	if (node->view->istemp == TRUE)
		string_append_char(str, "TEMP ");

	string_append_char(str, "VIEW ");
	_outNode(str, node->view);

	if (node->aliases)
	{
		string_append_char(str, "(");
		_outIdList(str, node->aliases);
		string_append_char(str, ")");
	}

	string_append_char(str, " AS");
	_outNode(str, node->query);
}

static void
_outCreatedbOptList(String *str, List *options)
{
	ListCell *lc;

	foreach (lc, options)
	{
		DefElem *e = lfirst(lc);
		Value *v = (Value *) e->arg;
		int sconst = false;

		/* keyword */
		if (strcmp(e->defname, "template") == 0)
			string_append_char(str, " TEMPLATE ");
		else if (strcmp(e->defname, "location") == 0)
		{
			string_append_char(str, " LOCATION ");
			sconst = true;
		}
		else if (strcmp(e->defname, "tablespace") == 0)
			string_append_char(str, " TABLESPACE ");
		else if (strcmp(e->defname, "encoding") == 0)
		{
			string_append_char(str, " ENCODING ");
			sconst = true;
		}
		else if (strcmp(e->defname, "owner") == 0)
			string_append_char(str, " OWNER ");
		else if (strcmp(e->defname, "connectionlimit") == 0)
			string_append_char(str, " CONNECTION LIMIT ");

		/* value */
		if (v == NULL)
			string_append_char(str, "DEFAULT");
		else if (IsA((Node *)v, String))
		{
			string_append_char(str, sconst ? "'" : "'");
			string_append_char(str, v->val.str);
			string_append_char(str, sconst ? "'" : "'");
		}
		else
		{
			char buf[16];
			snprintf(buf, 16, "%ld", v->val.ival);
			string_append_char(str, buf);
		}
	}
}

static void
_outCreatedbStmt(String *str, CreatedbStmt *node)
{
	string_append_char(str, "CREATE DATABASE \"");
	string_append_char(str, node->dbname);
	string_append_char(str, "\"");

	_outCreatedbOptList(str, node->options);
}

static void
_outAlterDatabaseStmt(String *str, AlterDatabaseStmt *node)
{
	string_append_char(str, "ALTER DATABASE \"");
	string_append_char(str, node->dbname);
	string_append_char(str, "\" ");

	_outCreatedbOptList(str, node->options);
}

static void
_outAlterDatabaseSetStmt(String *str, AlterDatabaseSetStmt *node)
{
	string_append_char(str, "ALTER DATABASE \"");
	string_append_char(str, node->dbname);
	string_append_char(str, "\" ");

	_outNode(str, node->setstmt);
}

static void
_outDropdbStmt(String *str, DropdbStmt *node)
{
	string_append_char(str, "DROP DATABASE \"");
	string_append_char(str, node->dbname);
	string_append_char(str, "\"");
}

static void
_outCreateDomainStmt(String *str, CreateDomainStmt *node)
{
	ListCell *lc;

	string_append_char(str, "CREATE DOMAIN ");
	_outFuncName(str, node->domainname);
	string_append_char(str, " ");
	_outNode(str, node->typename);


	foreach (lc, node->constraints)
	{
		string_append_char(str, " ");
		_outNode(str, lfirst(lc));
	}
}

static void
_outAlterDomainStmt(String *str, AlterDomainStmt *node)
{
	string_append_char(str, "ALTER DOMAIN ");
	_outFuncName(str, node->typename);

	switch (node->subtype)
	{
		case 'T':
			if (node->def)
			{
				string_append_char(str, " SET DEFAULT ");
				_outNode(str, node->def);
			}
			else
				string_append_char(str, " DROP DEFAULT");
			break;

		case 'N':
			string_append_char(str, " DROP NOT NULL");
			break;

		case 'O':
			string_append_char(str, " SET NOT NULL");
			break;

		case 'C':
			string_append_char(str, " ADD ");
			_outNode(str, node->def);
			break;

		case 'X':
			string_append_char(str, " DROP CONSTRAINT \"");
			string_append_char(str, node->name);
			string_append_char(str, "\"");
			if (node->behavior == DROP_CASCADE)
				string_append_char(str, " CASCADE");
			break;
	}
}

static void
_outCreateConversionStmt(String *str, CreateConversionStmt *node)
{
	string_append_char(str, "CREATE ");

	if (node->def == TRUE)
		string_append_char(str, "DEFAULT ");

	string_append_char(str, "CONVERSION ");

	_outFuncName(str, node->conversion_name);

	string_append_char(str, " FOR '");
	string_append_char(str, node->for_encoding_name);
	string_append_char(str, "' TO '");
	string_append_char(str, node->to_encoding_name);
	string_append_char(str, " FROM ");
	_outFuncName(str, node->func_name);
}

static void
_outPrepareStmt(String *str, PrepareStmt *node)
{
	string_append_char(str, "PREPARE \"");
	string_append_char(str, node->name);
	string_append_char(str, "\" ");

	if (node->argtypes != NIL)
	{
		string_append_char(str, "(");
		_outNode(str, node->argtypes);
		string_append_char(str, ") ");
	}

	string_append_char(str, "AS ");
	_outNode(str, node->query);
}

static void
_outExecuteStmt(String *str, ExecuteStmt *node)
{
	if (node->into)
	{
		IntoClause *into = node->into;
		RangeVar *rel = into->rel;

		string_append_char(str, "CREATE ");
		if (rel->istemp == TRUE)
			string_append_char(str, "TEMP ");
		string_append_char(str, "TABLE ");
		_outNode(str, into->rel);
		string_append_char(str, " AS ");
	}

	string_append_char(str, "EXECUTE \"");
	string_append_char(str, node->name);
	string_append_char(str, "\" ");

	if (node->params != NIL)
	{
		string_append_char(str, "(");
		_outNode(str, node->params);
		string_append_char(str, ")");
	}
}

static void
_outLockStmt(String *str, LockStmt *node)
{
	string_append_char(str, "LOCK TABLE ");
	_outNode(str, node->relations);

	string_append_char(str, " IN ");
	switch (node->mode)
	{
		case AccessShareLock:
			string_append_char(str, "ACCESS SHARE ");
			break;

		case RowShareLock:
			string_append_char(str, "ROW SHARE ");
			break;

		case RowExclusiveLock:
			string_append_char(str, "ROW EXCLUSIVE ");
			break;

		case ShareUpdateExclusiveLock:
			string_append_char(str, "SHARE UPDATE EXCLUSIVE ");
			break;

		case ShareLock:
			string_append_char(str, "SHARE ");
			break;

		case ShareRowExclusiveLock:
			string_append_char(str, "SHARE ROW EXCLUSIVE ");
			break;

		case ExclusiveLock:
			string_append_char(str, "EXCLUSIVE ");
			break;

		case AccessExclusiveLock:
			string_append_char(str, "ACCESS EXCLUSIVE ");
			break;
	}
	string_append_char(str, "MODE");

	if (node->nowait == TRUE)
		string_append_char(str, " NOWAIT");
}

static void
_outOperatorArgTypes(String *str, List *args)
{
	TypeName *left, *right;

	left = linitial(args);
	right = lsecond(args);

	if (left)
		_outNode(str, left);
	else
		string_append_char(str, "NONE");
	string_append_char(str, ", ");
	if (right)
		_outNode(str, right);
	else
		string_append_char(str, "NONE");
}

static void
_outCommentStmt(String *str, CommentStmt *node)
{
	TypeName *t;
	Value *v;
	char buf[16];

	string_append_char(str, "COMMENT ON ");

	switch (node->objtype)
	{
		case OBJECT_AGGREGATE:
			string_append_char(str, "AGGREGATE ");
			_outFuncName(str, node->objname);
			string_append_char(str, "(");

			t = linitial(node->objargs);
			if (t)
				_outNode(str, t);
			else
				string_append_char(str, "*");
			string_append_char(str, ")");
			break;

		case OBJECT_FUNCTION:
			string_append_char(str, "FUNCTION ");
			_outFuncName(str, node->objname);
			string_append_char(str, "(");
			_outNode(str, node->objargs);
			string_append_char(str, ")");
			break;

		case OBJECT_OPERATOR:
			string_append_char(str, "OPERATOR ");
			_outOperatorName(str, node->objname);
			string_append_char(str, "(");
			_outOperatorArgTypes(str, node->objargs);
			string_append_char(str, ")");
			break;

		case OBJECT_CONSTRAINT:
			string_append_char(str, "CONSTRAINT \"");
			v = lsecond(node->objname);
			string_append_char(str, v->val.str);
			string_append_char(str, "\" ON ");
			_outFuncName(str, linitial(node->objargs));
			break;

		case OBJECT_RULE:
			string_append_char(str, "RULE \"");
			v = lsecond(node->objname);
			string_append_char(str, v->val.str);
			string_append_char(str, "\" ON ");
			_outFuncName(str, linitial(node->objargs));
			break;

		case OBJECT_TRIGGER:
			string_append_char(str, "TRIGGER \"");
			v = lsecond(node->objname);
			string_append_char(str, v->val.str);
			string_append_char(str, "\" ON ");
			_outFuncName(str, linitial(node->objargs));
			break;

		case OBJECT_OPCLASS:
			string_append_char(str, "OPERATOR CLASS ");
			_outFuncName(str, node->objname);
			string_append_char(str, " USING ");
			v = linitial(node->objargs);
			string_append_char(str, v->val.str);
			break;

		case OBJECT_LARGEOBJECT:
			string_append_char(str, "LARGE OBJECT ");
			v = linitial(node->objname);
			if (IsA(v, String))
				string_append_char(str, v->val.str);
			else if (IsA(v, Integer))
			{
				snprintf(buf, 16, "%ld", v->val.ival);
				string_append_char(str, buf);
			}

		case OBJECT_CAST:
			string_append_char(str, "CAST (");
			_outNode(str, linitial(node->objname));
			string_append_char(str, " AS ");
			_outNode(str, linitial(node->objargs));
			string_append_char(str, ")");
			break;

		case OBJECT_LANGUAGE:
			string_append_char(str, "LANGUAGE ");
			_outFuncName(str, node->objname);
			break;

		default:
			switch (node->objtype)
			{
				case OBJECT_COLUMN:
					string_append_char(str, "COLUMN ");
					break;
				case OBJECT_DATABASE:
					string_append_char(str, "DATABASE ");
					break;
				case OBJECT_SCHEMA:
					string_append_char(str, "SCHEMA ");
					break;
				case OBJECT_INDEX:
					string_append_char(str, "INDEX ");
					break;
				case OBJECT_SEQUENCE:
					string_append_char(str, "SEQUENCE ");
					break;
				case OBJECT_TABLE:
					string_append_char(str, "TABLE ");
					break;
				case OBJECT_DOMAIN:
					string_append_char(str, "DOMAIN ");
					break;
				case OBJECT_TYPE:
					string_append_char(str, "TYPE ");
					break;
				case OBJECT_VIEW:
					string_append_char(str, "VIEW ");
					break;
				default:
					break;
			}
			_outFuncName(str, node->objname);
			break;
	}

	string_append_char(str, " IS ");
	if (node->comment)
	{
		string_append_char(str, "'");
		string_append_char(str, node->comment);
		string_append_char(str, "'");
	}
	else
		string_append_char(str, "NULL");
}

static void
_outRangeSubselect(String *str, RangeSubselect *node)
{
	string_append_char(str, "(");
	_outNode(str, node->subquery);
	string_append_char(str, ")");

	_outNode(str, node->alias);
}

static void
_outRangeFunction(String *str, RangeFunction *node)
{
	_outNode(str, node->funccallnode);
	if (node->alias)
	{
		_outNode(str, node->alias);
	}		

	if (node->coldeflist)
	{
		string_append_char(str, " (");
		_outNode(str, node->coldeflist);
		string_append_char(str, ")");
	}
}

static void
_outDiscardStmt(String *str, DiscardStmt *node)
{
	switch (node->target)
	{
		case DISCARD_ALL:
			string_append_char(str, "DISCARD ALL");
			break;

		case DISCARD_TEMP:
			string_append_char(str, "DISCARD TEMP");
			break;

		case DISCARD_PLANS:
			string_append_char(str, "DISCARD PLANS");
			break;

		default:
			break;
	}
}

static void
_outCreateOpFamilyStmt(String *str, CreateOpFamilyStmt *node)
{
	string_append_char(str, "CREATE OPERATOR FAMILY ");
	_outIdList(str, node->opfamilyname);
	string_append_char(str, " USING \"");
	string_append_char(str, node->amname);
	string_append_char(str, "\"");
}

static void
_outAlterOpFamilyStmt(String *str, AlterOpFamilyStmt *node)
{
}

static void
_outRemoveOpFamilyStmt(String *str, RemoveOpFamilyStmt *node)
{
	string_append_char(str, "DROP OPERATOR FAMILY ");
	if (node->missing_ok)
		string_append_char(str, "IF EXISTS ");
	_outIdList(str, node->opfamilyname);
	string_append_char(str, " USING \"");
	string_append_char(str, node->amname);
	string_append_char(str, "\"");
}

static void
_outCreateEnumStmt(String *str, CreateEnumStmt *node)
{
	string_append_char(str, "CREATE TYPE ");
	_outIdList(str, node->typename);
	string_append_char(str, " AS ENUM (");
	_outNode(str, node->vals);
	string_append_char(str, ")");
}

static void
_outDropOwnedStmt(String *str, DropOwnedStmt *node)
{
	string_append_char(str, "DROP OWNED BY ");
	_outIdList(str, node->roles);
	if (node->behavior == DROP_CASCADE)
		string_append_char(str, " CASCADE");
}

static void
_outReassignOwnedStmt(String *str, ReassignOwnedStmt *node)
{
	string_append_char(str, "REASSIGN OWNED BY ");
	_outIdList(str, node->roles);
	string_append_char(str, " TO \"");
	string_append_char(str, node->newrole);
	string_append_char(str, "\"");
}

static void
_outAlterTSDictionaryStmt(String *str, AlterTSDictionaryStmt *node)
{
	string_append_char(str, "ALTER TEXT SEARCH DICTIONARY ");
	_outIdList(str, node->dictname);
	string_append_char(str, "(");
	_outNode(str, node->options);
	string_append_char(str, ")");
}

static void
_outAlterTSConfigurationStmt(String *str, AlterTSConfigurationStmt *node)
{
	string_append_char(str, "ALTER TEXT SEARCH CONFIGURATION ");
	_outIdList(str, node->cfgname);
	if (node->override == false && node->replace == false)
	{
		string_append_char(str, "ADD MAPPING FOR ");
		_outIdList(str, node->tokentype);
		string_append_char(str, " WITH ");
		_outIdList(str, node->dicts);
	}
	else if (node->override == true && node->replace == false)
	{
		string_append_char(str, "ALTER MAPPING FOR ");
		_outIdList(str, node->tokentype);
		string_append_char(str, " WITH ");
		_outIdList(str, node->dicts);
	}
	else if (node->override == false && node->replace == true)
	{
		if (node->tokentype == NIL)
			string_append_char(str, "ALTER MAPPING ");
		else
		{
			string_append_char(str, "ALTER MAPPING FOR ");
			_outIdList(str, node->tokentype);
		}
		string_append_char(str, "REPLACE ");
		_outNode(str, linitial(node->dicts));
		string_append_char(str, " WITH ");
		_outNode(str, lsecond(node->dicts));
	}
	else if (node->missing_ok == false)
	{
		string_append_char(str, " DROP MAPPING FOR ");
		_outIdList(str, node->tokentype);
	}
	else if (node->missing_ok == true)
	{
		string_append_char(str, " DROP MAPPING IF EXISTS FOR ");
		_outIdList(str, node->tokentype);
	}
}

static void
_outXmlSerialize(String *str, XmlSerialize *node)
{

}

static void
_outInhRelation(String *str, InhRelation *node)
{
	ListCell *lc;

	string_append_char(str, "LIKE ");
	_outNode(str, node->relation);
	foreach (lc, node->options)
	{
		CreateStmtLikeOption v;
		v = (CreateStmtLikeOption)lfirst(lc);

		switch (v)
		{
			case CREATE_TABLE_LIKE_INCLUDING_DEFAULTS:
				string_append_char(str, " INCLUDING DEFAULTS");
				break;

			case CREATE_TABLE_LIKE_EXCLUDING_DEFAULTS:
				string_append_char(str, " EXCLUDING DEFAULTS");
				break;

			case CREATE_TABLE_LIKE_INCLUDING_CONSTRAINTS:
				string_append_char(str, " INCLUDING CONSTRAINTS");
				break;

			case CREATE_TABLE_LIKE_EXCLUDING_CONSTRAINTS:
				string_append_char(str, " EXCLUDING CONSTRAINTS");
				break;

			case CREATE_TABLE_LIKE_INCLUDING_INDEXES:
				string_append_char(str, " INCLUDING INDEXES");
				break;

			case CREATE_TABLE_LIKE_EXCLUDING_INDEXES:
				string_append_char(str, " EXCLUDING INDEXES");
				break;

			default:
				break;
		}
	}
}

static void
_outWithDefinition(String *str, List *def_list)
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
				string_append_char(str, " WITH OIDS ");
			else
				string_append_char(str, " WITHOUT OIDS ");
			oid = 1;
		}
	}

	if (oid == 1)
		return;

	string_append_char(str, " WITH ");
	_outDefinition(str, def_list);
}

static void
_outCurrentOfExpr(String *str, CurrentOfExpr *node)
{
	string_append_char(str, "CURRENT OF ");
	if (node->cursor_name == NULL)
	{
		char n[10];
		snprintf(n, sizeof(n), "$%d", node->cursor_param);
		string_append_char(str, n);
	}
	else
		string_append_char(str, node->cursor_name);
}

/*
 * _outNode -
 *	  converts a Node into ascii string and append it to 'str'
 */
static void
_outNode(String *str, void *obj)
{
	if (obj == NULL)
		return;
	else if (IsA(obj, List) ||IsA(obj, IntList) || IsA(obj, OidList))
		_outList(str, obj);
	else if (IsA(obj, Integer) ||
			 IsA(obj, Float) ||
			 IsA(obj, String) ||
			 IsA(obj, BitString))
	{
		/* nodeRead does not want to see { } around these! */
		_outValue(str, obj);
	}
	else
	{
		switch (nodeTag(obj))
		{
			case T_Alias:
				_outAlias(str, obj);
				break;
			case T_RangeVar:
				_outRangeVar(str, obj);
				break;
			case T_Var:
				_outVar(str, obj);
				break;
			case T_Const:
				_outConst(str, obj);
				break;
			case T_Param:
				_outParam(str, obj);
				break;
			case T_Aggref:
				_outAggref(str, obj);
				break;
			case T_ArrayRef:
				_outArrayRef(str, obj);
				break;
			case T_FuncExpr:
				_outFuncExpr(str, obj);
				break;
			case T_OpExpr:
				_outOpExpr(str, obj);
				break;
			case T_DistinctExpr:
				_outDistinctExpr(str, obj);
				break;
			case T_ScalarArrayOpExpr:
				_outScalarArrayOpExpr(str, obj);
				break;
			case T_BoolExpr:
				_outBoolExpr(str, obj);
				break;
			case T_SubLink:
				_outSubLink(str, obj);
				break;
			case T_SubPlan:
				_outSubPlan(str, obj);
				break;
			case T_FieldSelect:
				_outFieldSelect(str, obj);
				break;
			case T_FieldStore:
				_outFieldStore(str, obj);
				break;
			case T_RelabelType:
				_outRelabelType(str, obj);
				break;
			case T_ConvertRowtypeExpr:
				_outConvertRowtypeExpr(str, obj);
				break;
			case T_CaseExpr:
				_outCaseExpr(str, obj);
				break;
			case T_CaseWhen:
				_outCaseWhen(str, obj);
				break;
			case T_CaseTestExpr:
				_outCaseTestExpr(str, obj);
				break;
			case T_ArrayExpr:
				_outArrayExpr(str, obj);
				break;
			case T_RowExpr:
				_outRowExpr(str, obj);
				break;
			case T_CoalesceExpr:
				_outCoalesceExpr(str, obj);
				break;
			case T_MinMaxExpr:
				_outMinMaxExpr(str, obj);
				break;
			case T_NullIfExpr:
				_outNullIfExpr(str, obj);
				break;
			case T_NullTest:
				_outNullTest(str, obj);
				break;
			case T_BooleanTest:
				_outBooleanTest(str, obj);
				break;
			case T_CoerceToDomain:
				_outCoerceToDomain(str, obj);
				break;
			case T_CoerceToDomainValue:
				_outCoerceToDomainValue(str, obj);
				break;
			case T_SetToDefault:
				_outSetToDefault(str, obj);
				break;
			case T_TargetEntry:
				_outTargetEntry(str, obj);
				break;
			case T_RangeTblRef:
				_outRangeTblRef(str, obj);
				break;
			case T_JoinExpr:
				_outJoinExpr(str, obj);
				break;
			case T_FromExpr:
				_outFromExpr(str, obj);
				break;

			case T_CreateStmt:
				_outCreateStmt(str, obj);
				break;
			case T_IndexStmt:
				_outIndexStmt(str, obj);
				break;
			case T_NotifyStmt:
				_outNotifyStmt(str, obj);
				break;
			case T_DeclareCursorStmt:
				_outDeclareCursorStmt(str, obj);
				break;
			case T_SelectStmt:
				_outSelectStmt(str, obj);
				break;
			case T_ColumnDef:
				_outColumnDef(str, obj);
				break;
			case T_TypeName:
				_outTypeName(str, obj);
				break;
			case T_TypeCast:
				_outTypeCast(str, obj);
				break;
			case T_IndexElem:
				_outIndexElem(str, obj);
				break;
			case T_SortClause:
				_outSortClause(str, obj);
				break;
			case T_GroupClause:
				_outGroupClause(str, obj);
				break;
			case T_SetOperationStmt:
				_outSetOperationStmt(str, obj);
				break;
/*			case T_RangeTblEntry:
				_outRangeTblEntry(str, obj);
				break;*/
			case T_A_Expr:
				_outAExpr(str, obj);
				break;
			case T_ColumnRef:
				_outColumnRef(str, obj);
				break;
			case T_ParamRef:
				_outParamRef(str, obj);
				break;
			case T_A_Const:
				_outAConst(str, obj);
				break;
			case T_A_Indices:
				_outA_Indices(str, obj);
				break;
			case T_A_Indirection:
				_outA_Indirection(str, obj);
				break;
			case T_ResTarget:
				_outResTarget(str, obj);
				break;
			case T_Constraint:
				_outConstraint(str, obj);
				break;
			case T_FkConstraint:
				_outFkConstraint(str, obj);
				break;
			case T_FuncCall:
				_outFuncCall(str, obj);
				break;
			case T_DefElem:
				_outDefElem(str, obj);
				break;
			case T_LockingClause:
				_outLockingClause(str, obj);
				break;

			case T_SortBy:
				_outSortBy(str, obj);
				break;

			case T_InsertStmt:
				_outInsertStmt(str, obj);
				break;

			case T_UpdateStmt:
				_outUpdateStmt(str, obj);
				break;

			case T_DeleteStmt:
				_outDeleteStmt(str, obj);
				break;

			case T_TransactionStmt:
				_outTransactionStmt(str, obj);
				break;

			case T_TruncateStmt:
				_outTruncateStmt(str, obj);
				break;

			case T_VacuumStmt:
				_outVacuumStmt(str, obj);
				break;

			case T_ExplainStmt:
				_outExplainStmt(str, obj);
				break;

			case T_ClusterStmt:
				_outClusterStmt(str, obj);
				break;

			case T_CheckPointStmt:
				_outCheckPointStmt(str, obj);
				break;

			case T_ClosePortalStmt:
				_outClosePortalStmt(str, obj);
				break;

			case T_ListenStmt:
				_outListenStmt(str, obj);
				break;

			case T_UnlistenStmt:
				_outUnlistenStmt(str, obj);
				break;

			case T_LoadStmt:
				_outLoadStmt(str, obj);
				break;

			case T_CopyStmt:
				_outCopyStmt(str, obj);
				break;

			case T_DeallocateStmt:
				_outDeallocateStmt(str, obj);
				break;

			case T_RenameStmt:
				_outRenameStmt(str, obj);
				break;

			case T_CreateRoleStmt:
				_outCreateRoleStmt(str, obj);
				break;

			case T_AlterRoleStmt:
				_outAlterRoleStmt(str, obj);
				break;

			case T_AlterRoleSetStmt:
				_outAlterRoleSetStmt(str, obj);
				break;

			case T_DropRoleStmt:
				_outDropRoleStmt(str, obj);
				break;

			case T_CreateSchemaStmt:
				_outCreateSchemaStmt(str, obj);
				break;

			case T_VariableSetStmt:
				_outVariableSetStmt(str, obj);
				break;

			case T_VariableShowStmt:
				_outVariableShowStmt(str, obj);
				break;

			case T_ConstraintsSetStmt:
				_outConstraintsSetStmt(str, obj);
				break;

			case T_AlterTableStmt:
				_outAlterTableStmt(str, obj);
				break;

			case T_AlterTableCmd:
				_outAlterTableCmd(str, obj);
				break;

			case T_CreateSeqStmt:
				_outCreateSeqStmt(str, obj);
				break;

			case T_AlterSeqStmt:
				_outAlterSeqStmt(str, obj);
				break;

			case T_CreatePLangStmt:
				_outCreatePLangStmt(str, obj);
				break;

			case T_DropPLangStmt:
				_outDropPLangStmt(str, obj);
				break;

			case T_CreateTableSpaceStmt:
				_outCreateTableSpaceStmt(str, obj);
				break;

			case T_DropTableSpaceStmt:
				_outDropTableSpaceStmt(str, obj);
				break;

			case T_CreateTrigStmt:
				_outCreateTrigStmt(str, obj);
				break;

			case T_DropPropertyStmt:
				_outDropPropertyStmt(str, obj);
				break;

			case T_DefineStmt:
				_outDefineStmt(str, obj);
				break;

			case T_CreateOpClassStmt:
				_outCreateOpClassStmt(str, obj);
				break;

			case T_CreateOpClassItem:
				_outCreateOpClassItem(str, obj);
				break;

			case T_RemoveOpClassStmt:
				_outRemoveOpClassStmt(str, obj);
				break;

			case T_DropStmt:
				_outDropStmt(str, obj);
				break;

			case T_FetchStmt:
				_outFetchStmt(str, obj);
				break;

			case T_GrantStmt:
				_outGrantStmt(str, obj);
				break;

			case T_PrivTarget:
				_outPrivTarget(str, obj);
				break;

			case T_FuncWithArgs:
				_outFuncWithArgs(str, obj);
				break;

			case T_FunctionParameter:
				_outFunctionParameter(str, obj);
				break;

			case T_PrivGrantee:
				_outPrivGrantee(str, obj);
				break;

			case T_GrantRoleStmt:
				_outGrantRoleStmt(str, obj);
				break;

			case T_CreateFunctionStmt:
				_outCreateFunctionStmt(str, obj);
				break;

			case T_AlterFunctionStmt:
				_outAlterFunctionStmt(str, obj);
				break;

			case T_RemoveFuncStmt:
				_outRemoveFuncStmt(str, obj);
				break;

			case T_CreateCastStmt:
				_outCreateCastStmt(str, obj);
				break;

			case T_DropCastStmt:
				_outDropCastStmt(str, obj);
				break;

			case T_ReindexStmt:
				_outReindexStmt(str, obj);
				break;

			case T_AlterObjectSchemaStmt:
				_outAlterObjectSchemaStmt(str, obj);
				break;

			case T_AlterOwnerStmt:
				_outAlterOwnerStmt(str, obj);
				break;

			case T_RuleStmt:
				_outRuleStmt(str, obj);
				break;

			case T_ViewStmt:
				_outViewStmt(str, obj);
				break;

			case T_CreatedbStmt:
				_outCreatedbStmt(str, obj);
				break;

			case T_AlterDatabaseStmt:
				_outAlterDatabaseStmt(str, obj);
				break;


			case T_AlterDatabaseSetStmt:
				_outAlterDatabaseSetStmt(str, obj);
				break;

			case T_DropdbStmt:
				_outDropdbStmt(str, obj);
				break;

			case T_CreateDomainStmt:
				_outCreateDomainStmt(str, obj);
				break;

			case T_AlterDomainStmt:
				_outAlterDomainStmt(str, obj);
				break;

			case T_CreateConversionStmt:
				_outCreateConversionStmt(str, obj);
				break;

			case T_PrepareStmt:
				_outPrepareStmt(str, obj);
				break;

			case T_ExecuteStmt:
				_outExecuteStmt(str, obj);
				break;

			case T_LockStmt:
				_outLockStmt(str, obj);
				break;

			case T_CommentStmt:
				_outCommentStmt(str, obj);
				break;

			case T_RangeSubselect:
				_outRangeSubselect(str, obj);
				break;

			case T_RangeFunction:
				_outRangeFunction(str, obj);
				break;

			case T_DiscardStmt:
				_outDiscardStmt(str, obj);
				break;

			case T_CreateOpFamilyStmt:
				_outCreateOpFamilyStmt(str, obj);
				break;

			case T_AlterOpFamilyStmt:
				_outAlterOpFamilyStmt(str, obj);
				break;

			case T_RemoveOpFamilyStmt:
				_outRemoveOpFamilyStmt(str, obj);
				break;

			case T_CreateEnumStmt:
				_outCreateEnumStmt(str, obj);
				break;

			case T_DropOwnedStmt:
				_outDropOwnedStmt(str, obj);
				break;

			case T_ReassignOwnedStmt:
				_outReassignOwnedStmt(str, obj);
				break;

			case T_AlterTSDictionaryStmt:
				_outAlterTSDictionaryStmt(str, obj);
				break;

			case T_AlterTSConfigurationStmt:
				_outAlterTSConfigurationStmt(str, obj);
				break;

			case T_XmlSerialize:
				_outXmlSerialize(str, obj);
				break;

			case T_InhRelation:
				_outInhRelation(str, obj);
				break;

			case T_CurrentOfExpr:
				_outCurrentOfExpr(str, obj);

			default:
				break;
		}
	}
}


/*
 * nodeToString -
 *	   returns the ascii representation of the Node as a palloc'd string
 */
char *
nodeToString(void *obj)
{
	String *str;

	str = init_string("");
	_outNode(str, obj);
	return str->data;
}
