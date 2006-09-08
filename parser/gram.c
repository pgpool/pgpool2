/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ABORT_P = 258,
     ABSOLUTE_P = 259,
     ACCESS = 260,
     ACTION = 261,
     ADD = 262,
     ADMIN = 263,
     AFTER = 264,
     AGGREGATE = 265,
     ALL = 266,
     ALSO = 267,
     ALTER = 268,
     ANALYSE = 269,
     ANALYZE = 270,
     AND = 271,
     ANY = 272,
     ARRAY = 273,
     AS = 274,
     ASC = 275,
     ASSERTION = 276,
     ASSIGNMENT = 277,
     ASYMMETRIC = 278,
     AT = 279,
     AUTHORIZATION = 280,
     BACKWARD = 281,
     BEFORE = 282,
     BEGIN_P = 283,
     BETWEEN = 284,
     BIGINT = 285,
     BINARY = 286,
     BIT = 287,
     BOOLEAN_P = 288,
     BOTH = 289,
     BY = 290,
     CACHE = 291,
     CALLED = 292,
     CASCADE = 293,
     CASE = 294,
     CAST = 295,
     CHAIN = 296,
     CHAR_P = 297,
     CHARACTER = 298,
     CHARACTERISTICS = 299,
     CHECK = 300,
     CHECKPOINT = 301,
     CLASS = 302,
     CLOSE = 303,
     CLUSTER = 304,
     COALESCE = 305,
     COLLATE = 306,
     COLUMN = 307,
     COMMENT = 308,
     COMMIT = 309,
     COMMITTED = 310,
     CONNECTION = 311,
     CONSTRAINT = 312,
     CONSTRAINTS = 313,
     CONVERSION_P = 314,
     CONVERT = 315,
     COPY = 316,
     CREATE = 317,
     CREATEDB = 318,
     CREATEROLE = 319,
     CREATEUSER = 320,
     CROSS = 321,
     CSV = 322,
     CURRENT_DATE = 323,
     CURRENT_ROLE = 324,
     CURRENT_TIME = 325,
     CURRENT_TIMESTAMP = 326,
     CURRENT_USER = 327,
     CURSOR = 328,
     CYCLE = 329,
     DATABASE = 330,
     DAY_P = 331,
     DEALLOCATE = 332,
     DEC = 333,
     DECIMAL_P = 334,
     DECLARE = 335,
     DEFAULT = 336,
     DEFAULTS = 337,
     DEFERRABLE = 338,
     DEFERRED = 339,
     DEFINER = 340,
     DELETE_P = 341,
     DELIMITER = 342,
     DELIMITERS = 343,
     DESC = 344,
     DISABLE_P = 345,
     DISTINCT = 346,
     DO = 347,
     DOMAIN_P = 348,
     DOUBLE_P = 349,
     DROP = 350,
     EACH = 351,
     ELSE = 352,
     ENABLE_P = 353,
     ENCODING = 354,
     ENCRYPTED = 355,
     END_P = 356,
     ESCAPE = 357,
     EXCEPT = 358,
     EXCLUDING = 359,
     EXCLUSIVE = 360,
     EXECUTE = 361,
     EXISTS = 362,
     EXPLAIN = 363,
     EXTERNAL = 364,
     EXTRACT = 365,
     FALSE_P = 366,
     FETCH = 367,
     FIRST_P = 368,
     FLOAT_P = 369,
     FOR = 370,
     FORCE = 371,
     FOREIGN = 372,
     FORWARD = 373,
     FREEZE = 374,
     FROM = 375,
     FULL = 376,
     FUNCTION = 377,
     GLOBAL = 378,
     GRANT = 379,
     GRANTED = 380,
     GREATEST = 381,
     GROUP_P = 382,
     HANDLER = 383,
     HAVING = 384,
     HEADER = 385,
     HOLD = 386,
     HOUR_P = 387,
     ILIKE = 388,
     IMMEDIATE = 389,
     IMMUTABLE = 390,
     IMPLICIT_P = 391,
     IN_P = 392,
     INCLUDING = 393,
     INCREMENT = 394,
     INDEX = 395,
     INHERIT = 396,
     INHERITS = 397,
     INITIALLY = 398,
     INNER_P = 399,
     INOUT = 400,
     INPUT_P = 401,
     INSENSITIVE = 402,
     INSERT = 403,
     INSTEAD = 404,
     INT_P = 405,
     INTEGER = 406,
     INTERSECT = 407,
     INTERVAL = 408,
     INTO = 409,
     INVOKER = 410,
     IS = 411,
     ISNULL = 412,
     ISOLATION = 413,
     JOIN = 414,
     KEY = 415,
     LANCOMPILER = 416,
     LANGUAGE = 417,
     LARGE_P = 418,
     LAST_P = 419,
     LEADING = 420,
     LEAST = 421,
     LEFT = 422,
     LEVEL = 423,
     LIKE = 424,
     LIMIT = 425,
     LISTEN = 426,
     LOAD = 427,
     LOCAL = 428,
     LOCALTIME = 429,
     LOCALTIMESTAMP = 430,
     LOCATION = 431,
     LOCK_P = 432,
     LOGIN_P = 433,
     MATCH = 434,
     MAXVALUE = 435,
     MINUTE_P = 436,
     MINVALUE = 437,
     MODE = 438,
     MONTH_P = 439,
     MOVE = 440,
     NAMES = 441,
     NATIONAL = 442,
     NATURAL = 443,
     NCHAR = 444,
     NEW = 445,
     NEXT = 446,
     NO = 447,
     NOCREATEDB = 448,
     NOCREATEROLE = 449,
     NOCREATEUSER = 450,
     NOINHERIT = 451,
     NOLOGIN_P = 452,
     NONE = 453,
     NOSUPERUSER = 454,
     NOT = 455,
     NOTHING = 456,
     NOTIFY = 457,
     NOTNULL = 458,
     NOWAIT = 459,
     NULL_P = 460,
     NULLIF = 461,
     NUMERIC = 462,
     OBJECT_P = 463,
     OF = 464,
     OFF = 465,
     OFFSET = 466,
     OIDS = 467,
     OLD = 468,
     ON = 469,
     ONLY = 470,
     OPERATOR = 471,
     OPTION = 472,
     OR = 473,
     ORDER = 474,
     OUT_P = 475,
     OUTER_P = 476,
     OVERLAPS = 477,
     OVERLAY = 478,
     OWNER = 479,
     PARTIAL = 480,
     PASSWORD = 481,
     PLACING = 482,
     POSITION = 483,
     PRECISION = 484,
     PRESERVE = 485,
     PREPARE = 486,
     PREPARED = 487,
     PRIMARY = 488,
     PRIOR = 489,
     PRIVILEGES = 490,
     PROCEDURAL = 491,
     PROCEDURE = 492,
     QUOTE = 493,
     READ = 494,
     REAL = 495,
     RECHECK = 496,
     REFERENCES = 497,
     REINDEX = 498,
     RELATIVE_P = 499,
     RELEASE = 500,
     RENAME = 501,
     REPEATABLE = 502,
     REPLACE = 503,
     RESET = 504,
     RESTART = 505,
     RESTRICT = 506,
     RETURNS = 507,
     REVOKE = 508,
     RIGHT = 509,
     ROLE = 510,
     ROLLBACK = 511,
     ROW = 512,
     ROWS = 513,
     RULE = 514,
     SAVEPOINT = 515,
     SCHEMA = 516,
     SCROLL = 517,
     SECOND_P = 518,
     SECURITY = 519,
     SELECT = 520,
     SEQUENCE = 521,
     SERIALIZABLE = 522,
     SESSION = 523,
     SESSION_USER = 524,
     SET = 525,
     SETOF = 526,
     SHARE = 527,
     SHOW = 528,
     SIMILAR = 529,
     SIMPLE = 530,
     SMALLINT = 531,
     SOME = 532,
     STABLE = 533,
     START = 534,
     STATEMENT = 535,
     STATISTICS = 536,
     STDIN = 537,
     STDOUT = 538,
     STORAGE = 539,
     STRICT_P = 540,
     SUBSTRING = 541,
     SUPERUSER_P = 542,
     SYMMETRIC = 543,
     SYSID = 544,
     SYSTEM_P = 545,
     TABLE = 546,
     TABLESPACE = 547,
     TEMP = 548,
     TEMPLATE = 549,
     TEMPORARY = 550,
     THEN = 551,
     TIME = 552,
     TIMESTAMP = 553,
     TO = 554,
     TOAST = 555,
     TRAILING = 556,
     TRANSACTION = 557,
     TREAT = 558,
     TRIGGER = 559,
     TRIM = 560,
     TRUE_P = 561,
     TRUNCATE = 562,
     TRUSTED = 563,
     TYPE_P = 564,
     UNCOMMITTED = 565,
     UNENCRYPTED = 566,
     UNION = 567,
     UNIQUE = 568,
     UNKNOWN = 569,
     UNLISTEN = 570,
     UNTIL = 571,
     UPDATE = 572,
     USER = 573,
     USING = 574,
     VACUUM = 575,
     VALID = 576,
     VALIDATOR = 577,
     VALUES = 578,
     VARCHAR = 579,
     VARYING = 580,
     VERBOSE = 581,
     VIEW = 582,
     VOLATILE = 583,
     WHEN = 584,
     WHERE = 585,
     WITH = 586,
     WITHOUT = 587,
     WORK = 588,
     WRITE = 589,
     YEAR_P = 590,
     ZONE = 591,
     UNIONJOIN = 592,
     IDENT = 593,
     FCONST = 594,
     SCONST = 595,
     BCONST = 596,
     XCONST = 597,
     Op = 598,
     ICONST = 599,
     PARAM = 600,
     POSTFIXOP = 601,
     UMINUS = 602,
     TYPECAST = 603
   };
#endif
#define ABORT_P 258
#define ABSOLUTE_P 259
#define ACCESS 260
#define ACTION 261
#define ADD 262
#define ADMIN 263
#define AFTER 264
#define AGGREGATE 265
#define ALL 266
#define ALSO 267
#define ALTER 268
#define ANALYSE 269
#define ANALYZE 270
#define AND 271
#define ANY 272
#define ARRAY 273
#define AS 274
#define ASC 275
#define ASSERTION 276
#define ASSIGNMENT 277
#define ASYMMETRIC 278
#define AT 279
#define AUTHORIZATION 280
#define BACKWARD 281
#define BEFORE 282
#define BEGIN_P 283
#define BETWEEN 284
#define BIGINT 285
#define BINARY 286
#define BIT 287
#define BOOLEAN_P 288
#define BOTH 289
#define BY 290
#define CACHE 291
#define CALLED 292
#define CASCADE 293
#define CASE 294
#define CAST 295
#define CHAIN 296
#define CHAR_P 297
#define CHARACTER 298
#define CHARACTERISTICS 299
#define CHECK 300
#define CHECKPOINT 301
#define CLASS 302
#define CLOSE 303
#define CLUSTER 304
#define COALESCE 305
#define COLLATE 306
#define COLUMN 307
#define COMMENT 308
#define COMMIT 309
#define COMMITTED 310
#define CONNECTION 311
#define CONSTRAINT 312
#define CONSTRAINTS 313
#define CONVERSION_P 314
#define CONVERT 315
#define COPY 316
#define CREATE 317
#define CREATEDB 318
#define CREATEROLE 319
#define CREATEUSER 320
#define CROSS 321
#define CSV 322
#define CURRENT_DATE 323
#define CURRENT_ROLE 324
#define CURRENT_TIME 325
#define CURRENT_TIMESTAMP 326
#define CURRENT_USER 327
#define CURSOR 328
#define CYCLE 329
#define DATABASE 330
#define DAY_P 331
#define DEALLOCATE 332
#define DEC 333
#define DECIMAL_P 334
#define DECLARE 335
#define DEFAULT 336
#define DEFAULTS 337
#define DEFERRABLE 338
#define DEFERRED 339
#define DEFINER 340
#define DELETE_P 341
#define DELIMITER 342
#define DELIMITERS 343
#define DESC 344
#define DISABLE_P 345
#define DISTINCT 346
#define DO 347
#define DOMAIN_P 348
#define DOUBLE_P 349
#define DROP 350
#define EACH 351
#define ELSE 352
#define ENABLE_P 353
#define ENCODING 354
#define ENCRYPTED 355
#define END_P 356
#define ESCAPE 357
#define EXCEPT 358
#define EXCLUDING 359
#define EXCLUSIVE 360
#define EXECUTE 361
#define EXISTS 362
#define EXPLAIN 363
#define EXTERNAL 364
#define EXTRACT 365
#define FALSE_P 366
#define FETCH 367
#define FIRST_P 368
#define FLOAT_P 369
#define FOR 370
#define FORCE 371
#define FOREIGN 372
#define FORWARD 373
#define FREEZE 374
#define FROM 375
#define FULL 376
#define FUNCTION 377
#define GLOBAL 378
#define GRANT 379
#define GRANTED 380
#define GREATEST 381
#define GROUP_P 382
#define HANDLER 383
#define HAVING 384
#define HEADER 385
#define HOLD 386
#define HOUR_P 387
#define ILIKE 388
#define IMMEDIATE 389
#define IMMUTABLE 390
#define IMPLICIT_P 391
#define IN_P 392
#define INCLUDING 393
#define INCREMENT 394
#define INDEX 395
#define INHERIT 396
#define INHERITS 397
#define INITIALLY 398
#define INNER_P 399
#define INOUT 400
#define INPUT_P 401
#define INSENSITIVE 402
#define INSERT 403
#define INSTEAD 404
#define INT_P 405
#define INTEGER 406
#define INTERSECT 407
#define INTERVAL 408
#define INTO 409
#define INVOKER 410
#define IS 411
#define ISNULL 412
#define ISOLATION 413
#define JOIN 414
#define KEY 415
#define LANCOMPILER 416
#define LANGUAGE 417
#define LARGE_P 418
#define LAST_P 419
#define LEADING 420
#define LEAST 421
#define LEFT 422
#define LEVEL 423
#define LIKE 424
#define LIMIT 425
#define LISTEN 426
#define LOAD 427
#define LOCAL 428
#define LOCALTIME 429
#define LOCALTIMESTAMP 430
#define LOCATION 431
#define LOCK_P 432
#define LOGIN_P 433
#define MATCH 434
#define MAXVALUE 435
#define MINUTE_P 436
#define MINVALUE 437
#define MODE 438
#define MONTH_P 439
#define MOVE 440
#define NAMES 441
#define NATIONAL 442
#define NATURAL 443
#define NCHAR 444
#define NEW 445
#define NEXT 446
#define NO 447
#define NOCREATEDB 448
#define NOCREATEROLE 449
#define NOCREATEUSER 450
#define NOINHERIT 451
#define NOLOGIN_P 452
#define NONE 453
#define NOSUPERUSER 454
#define NOT 455
#define NOTHING 456
#define NOTIFY 457
#define NOTNULL 458
#define NOWAIT 459
#define NULL_P 460
#define NULLIF 461
#define NUMERIC 462
#define OBJECT_P 463
#define OF 464
#define OFF 465
#define OFFSET 466
#define OIDS 467
#define OLD 468
#define ON 469
#define ONLY 470
#define OPERATOR 471
#define OPTION 472
#define OR 473
#define ORDER 474
#define OUT_P 475
#define OUTER_P 476
#define OVERLAPS 477
#define OVERLAY 478
#define OWNER 479
#define PARTIAL 480
#define PASSWORD 481
#define PLACING 482
#define POSITION 483
#define PRECISION 484
#define PRESERVE 485
#define PREPARE 486
#define PREPARED 487
#define PRIMARY 488
#define PRIOR 489
#define PRIVILEGES 490
#define PROCEDURAL 491
#define PROCEDURE 492
#define QUOTE 493
#define READ 494
#define REAL 495
#define RECHECK 496
#define REFERENCES 497
#define REINDEX 498
#define RELATIVE_P 499
#define RELEASE 500
#define RENAME 501
#define REPEATABLE 502
#define REPLACE 503
#define RESET 504
#define RESTART 505
#define RESTRICT 506
#define RETURNS 507
#define REVOKE 508
#define RIGHT 509
#define ROLE 510
#define ROLLBACK 511
#define ROW 512
#define ROWS 513
#define RULE 514
#define SAVEPOINT 515
#define SCHEMA 516
#define SCROLL 517
#define SECOND_P 518
#define SECURITY 519
#define SELECT 520
#define SEQUENCE 521
#define SERIALIZABLE 522
#define SESSION 523
#define SESSION_USER 524
#define SET 525
#define SETOF 526
#define SHARE 527
#define SHOW 528
#define SIMILAR 529
#define SIMPLE 530
#define SMALLINT 531
#define SOME 532
#define STABLE 533
#define START 534
#define STATEMENT 535
#define STATISTICS 536
#define STDIN 537
#define STDOUT 538
#define STORAGE 539
#define STRICT_P 540
#define SUBSTRING 541
#define SUPERUSER_P 542
#define SYMMETRIC 543
#define SYSID 544
#define SYSTEM_P 545
#define TABLE 546
#define TABLESPACE 547
#define TEMP 548
#define TEMPLATE 549
#define TEMPORARY 550
#define THEN 551
#define TIME 552
#define TIMESTAMP 553
#define TO 554
#define TOAST 555
#define TRAILING 556
#define TRANSACTION 557
#define TREAT 558
#define TRIGGER 559
#define TRIM 560
#define TRUE_P 561
#define TRUNCATE 562
#define TRUSTED 563
#define TYPE_P 564
#define UNCOMMITTED 565
#define UNENCRYPTED 566
#define UNION 567
#define UNIQUE 568
#define UNKNOWN 569
#define UNLISTEN 570
#define UNTIL 571
#define UPDATE 572
#define USER 573
#define USING 574
#define VACUUM 575
#define VALID 576
#define VALIDATOR 577
#define VALUES 578
#define VARCHAR 579
#define VARYING 580
#define VERBOSE 581
#define VIEW 582
#define VOLATILE 583
#define WHEN 584
#define WHERE 585
#define WITH 586
#define WITHOUT 587
#define WORK 588
#define WRITE 589
#define YEAR_P 590
#define ZONE 591
#define UNIONJOIN 592
#define IDENT 593
#define FCONST 594
#define SCONST 595
#define BCONST 596
#define XCONST 597
#define Op 598
#define ICONST 599
#define PARAM 600
#define POSTFIXOP 601
#define UMINUS 602
#define TYPECAST 603




/* Copy the first part of user declarations.  */
#line 1 "gram.y"


/*#define YYDEBUG 1*/
/*-------------------------------------------------------------------------
 *
 * gram.y
 *	  POSTGRES SQL YACC rules/actions
 *
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/parser/gram.y,v 2.511.2.2 2006/01/31 22:40:12 tgl Exp $
 *
 * HISTORY
 *	  AUTHOR			DATE			MAJOR EVENT
 *	  Andrew Yu			Sept, 1994		POSTQUEL to SQL conversion
 *	  Andrew Yu			Oct, 1994		lispy code conversion
 *
 * NOTES
 *	  CAPITALS are used to represent terminal symbols.
 *	  non-capitals are used to represent non-terminals.
 *	  SQL92-specific syntax is separated from plain SQL/Postgres syntax
 *	  to help isolate the non-extensible portions of the parser.
 *
 *	  In general, nothing in this file should initiate database accesses
 *	  nor depend on changeable state (such as SET variables).  If you do
 *	  database accesses, your code will fail when we have aborted the
 *	  current transaction and are just parsing commands to find the next
 *	  ROLLBACK or COMMIT.  If you make use of SET variables, then you
 *	  will do the wrong thing in multi-query strings like this:
 *			SET SQL_inheritance TO off; SELECT * FROM foo;
 *	  because the entire string is parsed by gram.y before the SET gets
 *	  executed.  Anything that depends on the database or changeable state
 *	  should be handled inside parse_analyze() so that it happens at the
 *	  right time not the wrong time.  The handling of SQL_inheritance is
 *	  a good example.
 *
 * WARNINGS
 *	  If you use a list, make sure the datum is a node so that the printing
 *	  routines work.
 *
 *	  Sometimes we assign constants to makeStrings. Make sure we don't free
 *	  those.
 *
 *-------------------------------------------------------------------------
 */
#include "pool_parser.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "pool_memory.h"
#include "gramparse.h"
#include "makefuncs.h"

#define yylex sql_yylex

#define ereport(a,b)
#define Assert

List *parsetree;			/* final parse result is delivered here */

static bool QueryIsRule = FALSE;

extern TypeName *SystemTypeName(char *name);
extern List *SystemFuncName(char *name);
extern char *NameListToQuotedString(List *names);
extern void yyerror(const char *s);

/*
 * If you need access to certain yacc-generated variables and find that
 * they're static by default, uncomment the next line.  (this is not a
 * problem, yet.)
 */
/*#define __YYSCLASS*/

static Node *makeColumnRef(char *relname, List *indirection);
static Node *makeTypeCast(Node *arg, TypeName *typename);
static Node *makeStringConst(char *str, TypeName *typename);
static Node *makeIntConst(int val);
static Node *makeFloatConst(char *str);
static Node *makeAConst(Value *v);
static Node *makeRowNullTest(NullTestType test, RowExpr *row);
static DefElem *makeDefElem(char *name, Node *arg);
static A_Const *makeBoolAConst(bool state);
static FuncCall *makeOverlaps(List *largs, List *rargs);
static void check_qualified_name(List *names);
static List *check_func_name(List *names);
static List *extractArgTypes(List *parameters);
static SelectStmt *findLeftmostSelect(SelectStmt *node);
static void insertSelectOptions(SelectStmt *stmt,
								List *sortClause, Node *lockingClause,
								Node *limitOffset, Node *limitCount);
static Node *makeSetOp(SetOperation op, bool all, Node *larg, Node *rarg);
static Node *doNegate(Node *n);
static void doNegateFloat(Value *v);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 106 "gram.y"
typedef union YYSTYPE {
	int					ival;
	char				chr;
	char				*str;
	const char			*keyword;
	bool				boolean;
	JoinType			jtype;
	DropBehavior		dbehavior;
	OnCommitAction		oncommit;
	ContainsOids		withoids;
	List				*list;
	Node				*node;
	Value				*value;
	ObjectType			objtype;

	TypeName			*typnam;
	FunctionParameter   *fun_param;
	FunctionParameterMode fun_param_mode;
	FuncWithArgs		*funwithargs;
	DefElem				*defelt;
	SortBy				*sortby;
	JoinExpr			*jexpr;
	IndexElem			*ielem;
	Alias				*alias;
	RangeVar			*range;
	A_Indices			*aind;
	ResTarget			*target;
	PrivTarget			*privtarget;

	InsertStmt			*istmt;
	VariableSetStmt		*vsetstmt;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 907 "gram.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 919 "gram.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  555
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   34910

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  366
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  391
/* YYNRULES -- Number of rules. */
#define YYNRULES  1613
/* YYNRULES -- Number of states. */
#define YYNSTATES  2807

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   603

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned short yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   354,     2,     2,
     359,   360,   352,   350,   364,   351,   362,   353,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   365,   363,
     347,   346,   348,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   357,     2,   358,   355,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   349,   356,   361
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     9,    11,    13,    15,    17,    19,
      21,    23,    25,    27,    29,    31,    33,    35,    37,    39,
      41,    43,    45,    47,    49,    51,    53,    55,    57,    59,
      61,    63,    65,    67,    69,    71,    73,    75,    77,    79,
      81,    83,    85,    87,    89,    91,    93,    95,    97,    99,
     101,   103,   105,   107,   109,   111,   113,   115,   117,   119,
     121,   123,   125,   127,   129,   131,   133,   135,   137,   139,
     141,   143,   145,   147,   149,   151,   153,   155,   157,   159,
     161,   163,   165,   167,   169,   171,   173,   175,   177,   178,
     184,   186,   187,   190,   191,   194,   198,   202,   204,   206,
     208,   210,   212,   214,   216,   218,   220,   222,   224,   226,
     230,   234,   237,   240,   243,   246,   250,   254,   260,   266,
     272,   277,   283,   289,   294,   298,   302,   308,   315,   317,
     319,   323,   330,   335,   337,   338,   341,   342,   344,   346,
     348,   350,   352,   354,   357,   361,   365,   369,   373,   377,
     380,   386,   389,   392,   396,   400,   402,   406,   408,   410,
     412,   416,   418,   420,   422,   425,   428,   431,   433,   435,
     437,   439,   441,   443,   445,   449,   456,   458,   460,   462,
     464,   466,   467,   469,   471,   474,   478,   483,   487,   490,
     493,   497,   502,   506,   509,   514,   516,   518,   520,   522,
     524,   529,   534,   536,   540,   544,   549,   556,   563,   570,
     577,   582,   589,   592,   597,   601,   605,   609,   613,   617,
     621,   625,   629,   633,   637,   639,   641,   645,   649,   653,
     657,   660,   662,   664,   665,   668,   669,   672,   683,   685,
     687,   689,   691,   693,   696,   697,   699,   701,   705,   709,
     711,   713,   717,   721,   725,   730,   732,   733,   736,   737,
     741,   742,   744,   745,   757,   770,   772,   774,   777,   780,
     783,   786,   787,   789,   790,   792,   796,   798,   800,   802,
     806,   809,   810,   814,   816,   818,   821,   823,   826,   830,
     835,   838,   844,   846,   849,   852,   855,   859,   862,   865,
     866,   870,   872,   877,   883,   890,   902,   906,   907,   909,
     913,   915,   918,   921,   924,   925,   927,   929,   932,   935,
     936,   940,   944,   947,   949,   951,   954,   957,   962,   963,
     966,   969,   970,   974,   979,   984,   985,   988,   989,   994,
     995,  1003,  1007,  1011,  1013,  1017,  1018,  1020,  1024,  1026,
    1032,  1037,  1040,  1041,  1044,  1046,  1049,  1053,  1056,  1059,
    1062,  1065,  1069,  1073,  1075,  1076,  1078,  1080,  1082,  1085,
    1087,  1093,  1103,  1105,  1106,  1108,  1111,  1114,  1115,  1118,
    1119,  1125,  1127,  1128,  1135,  1138,  1139,  1143,  1158,  1178,
    1180,  1182,  1184,  1188,  1194,  1196,  1198,  1200,  1204,  1205,
    1207,  1208,  1210,  1212,  1214,  1218,  1219,  1221,  1223,  1225,
    1227,  1229,  1231,  1234,  1235,  1237,  1240,  1242,  1245,  1246,
    1249,  1251,  1254,  1257,  1264,  1273,  1278,  1283,  1288,  1293,
    1301,  1305,  1307,  1311,  1315,  1317,  1319,  1321,  1323,  1325,
    1338,  1340,  1344,  1349,  1357,  1362,  1365,  1367,  1368,  1370,
    1371,  1379,  1384,  1386,  1388,  1390,  1392,  1394,  1396,  1398,
    1400,  1402,  1406,  1408,  1411,  1414,  1418,  1422,  1429,  1439,
    1447,  1457,  1466,  1475,  1482,  1491,  1501,  1509,  1520,  1528,
    1530,  1532,  1534,  1536,  1538,  1540,  1542,  1544,  1546,  1548,
    1550,  1552,  1557,  1560,  1565,  1568,  1569,  1571,  1573,  1575,
    1577,  1580,  1583,  1585,  1587,  1589,  1592,  1595,  1597,  1600,
    1603,  1605,  1607,  1615,  1623,  1634,  1636,  1638,  1641,  1643,
    1647,  1649,  1651,  1653,  1655,  1657,  1660,  1663,  1666,  1669,
    1672,  1675,  1677,  1681,  1683,  1686,  1690,  1691,  1693,  1697,
    1700,  1707,  1714,  1724,  1728,  1729,  1733,  1734,  1747,  1749,
    1750,  1753,  1754,  1756,  1760,  1763,  1766,  1771,  1773,  1776,
    1777,  1787,  1795,  1798,  1799,  1803,  1806,  1808,  1812,  1816,
    1820,  1823,  1826,  1828,  1830,  1832,  1834,  1837,  1839,  1841,
    1843,  1848,  1854,  1856,  1859,  1864,  1870,  1872,  1874,  1876,
    1878,  1882,  1886,  1889,  1892,  1895,  1898,  1900,  1902,  1906,
    1909,  1910,  1916,  1918,  1921,  1923,  1924,  1930,  1938,  1940,
    1942,  1950,  1952,  1956,  1960,  1964,  1966,  1970,  1982,  1993,
    1996,  1999,  2000,  2009,  2014,  2019,  2024,  2026,  2028,  2030,
    2031,  2041,  2048,  2055,  2063,  2070,  2077,  2087,  2094,  2101,
    2108,  2117,  2126,  2133,  2140,  2147,  2149,  2150,  2160,  2167,
    2175,  2182,  2189,  2196,  2206,  2213,  2220,  2227,  2235,  2245,
    2255,  2262,  2269,  2276,  2277,  2292,  2294,  2296,  2300,  2304,
    2306,  2308,  2310,  2312,  2314,  2316,  2318,  2319,  2321,  2323,
    2325,  2327,  2329,  2331,  2332,  2339,  2342,  2345,  2348,  2351,
    2354,  2358,  2362,  2365,  2368,  2371,  2374,  2378,  2381,  2387,
    2392,  2396,  2400,  2404,  2406,  2408,  2409,  2413,  2416,  2419,
    2421,  2425,  2428,  2430,  2431,  2439,  2449,  2452,  2458,  2461,
    2462,  2466,  2470,  2474,  2478,  2482,  2486,  2490,  2494,  2498,
    2503,  2507,  2511,  2513,  2514,  2520,  2526,  2531,  2534,  2535,
    2540,  2544,  2551,  2556,  2563,  2570,  2576,  2584,  2586,  2587,
    2598,  2603,  2606,  2608,  2613,  2619,  2625,  2628,  2633,  2635,
    2637,  2639,  2640,  2642,  2643,  2645,  2646,  2650,  2651,  2656,
    2658,  2660,  2662,  2664,  2666,  2668,  2670,  2671,  2677,  2681,
    2682,  2684,  2688,  2690,  2692,  2694,  2696,  2700,  2710,  2714,
    2715,  2718,  2722,  2727,  2732,  2735,  2737,  2745,  2750,  2752,
    2756,  2759,  2765,  2768,  2769,  2775,  2779,  2780,  2783,  2786,
    2789,  2793,  2795,  2799,  2801,  2804,  2806,  2807,  2814,  2822,
    2823,  2827,  2830,  2833,  2836,  2837,  2840,  2843,  2845,  2847,
    2851,  2855,  2857,  2860,  2865,  2870,  2872,  2874,  2883,  2888,
    2893,  2898,  2901,  2902,  2906,  2910,  2915,  2920,  2925,  2930,
    2933,  2935,  2937,  2938,  2940,  2942,  2943,  2945,  2951,  2953,
    2954,  2956,  2957,  2961,  2963,  2967,  2971,  2974,  2977,  2979,
    2984,  2989,  2992,  2995,  3000,  3002,  3003,  3005,  3007,  3009,
    3013,  3014,  3017,  3018,  3023,  3028,  3032,  3034,  3035,  3038,
    3039,  3042,  3043,  3045,  3049,  3051,  3054,  3056,  3059,  3065,
    3072,  3078,  3080,  3083,  3085,  3090,  3094,  3099,  3103,  3109,
    3114,  3120,  3125,  3131,  3134,  3139,  3141,  3144,  3147,  3150,
    3152,  3154,  3155,  3160,  3163,  3165,  3168,  3171,  3176,  3178,
    3181,  3182,  3184,  3188,  3191,  3194,  3198,  3204,  3211,  3215,
    3220,  3221,  3223,  3225,  3227,  3229,  3231,  3234,  3240,  3243,
    3245,  3247,  3249,  3251,  3253,  3255,  3257,  3259,  3261,  3263,
    3265,  3268,  3271,  3274,  3277,  3280,  3282,  3286,  3287,  3293,
    3297,  3298,  3304,  3308,  3309,  3311,  3313,  3315,  3317,  3323,
    3326,  3328,  3330,  3332,  3334,  3340,  3343,  3346,  3349,  3351,
    3355,  3359,  3362,  3364,  3365,  3369,  3370,  3376,  3379,  3385,
    3388,  3390,  3394,  3398,  3399,  3401,  3403,  3405,  3407,  3409,
    3411,  3415,  3419,  3423,  3427,  3431,  3435,  3439,  3440,  3442,
    3446,  3452,  3455,  3458,  3462,  3466,  3470,  3474,  3478,  3482,
    3486,  3490,  3494,  3498,  3501,  3504,  3508,  3512,  3515,  3519,
    3525,  3530,  3537,  3541,  3547,  3552,  3559,  3564,  3571,  3577,
    3585,  3588,  3592,  3595,  3600,  3604,  3608,  3613,  3617,  3622,
    3626,  3631,  3637,  3644,  3652,  3659,  3667,  3674,  3682,  3686,
    3691,  3696,  3703,  3706,  3708,  3712,  3715,  3718,  3722,  3726,
    3730,  3734,  3738,  3742,  3746,  3750,  3754,  3758,  3761,  3764,
    3770,  3777,  3785,  3787,  3789,  3792,  3797,  3799,  3801,  3803,
    3806,  3809,  3812,  3814,  3818,  3823,  3829,  3835,  3840,  3842,
    3844,  3849,  3851,  3856,  3858,  3863,  3865,  3870,  3872,  3874,
    3876,  3878,  3885,  3890,  3895,  3900,  3905,  3912,  3918,  3924,
    3930,  3935,  3942,  3947,  3954,  3959,  3964,  3969,  3974,  3978,
    3984,  3986,  3988,  3990,  3992,  3994,  3996,  3998,  4000,  4002,
    4004,  4006,  4008,  4010,  4012,  4014,  4019,  4021,  4026,  4028,
    4033,  4035,  4038,  4040,  4043,  4045,  4049,  4053,  4054,  4058,
    4060,  4062,  4066,  4070,  4074,  4076,  4078,  4080,  4082,  4084,
    4086,  4088,  4090,  4095,  4099,  4102,  4106,  4107,  4111,  4115,
    4118,  4121,  4123,  4124,  4127,  4130,  4134,  4137,  4139,  4141,
    4145,  4151,  4153,  4156,  4161,  4164,  4165,  4167,  4168,  4170,
    4173,  4176,  4179,  4183,  4189,  4191,  4194,  4195,  4198,  4200,
    4201,  4203,  4207,  4211,  4213,  4215,  4217,  4221,  4226,  4231,
    4233,  4237,  4239,  4241,  4243,  4245,  4247,  4251,  4253,  4256,
    4258,  4262,  4264,  4266,  4268,  4270,  4272,  4274,  4276,  4279,
    4281,  4283,  4285,  4287,  4289,  4292,  4296,  4303,  4305,  4307,
    4309,  4311,  4313,  4315,  4317,  4320,  4322,  4324,  4326,  4328,
    4330,  4332,  4334,  4336,  4338,  4340,  4342,  4344,  4346,  4348,
    4350,  4352,  4354,  4356,  4358,  4360,  4362,  4364,  4366,  4368,
    4370,  4372,  4374,  4376,  4378,  4380,  4382,  4384,  4386,  4388,
    4390,  4392,  4394,  4396,  4398,  4400,  4402,  4404,  4406,  4408,
    4410,  4412,  4414,  4416,  4418,  4420,  4422,  4424,  4426,  4428,
    4430,  4432,  4434,  4436,  4438,  4440,  4442,  4444,  4446,  4448,
    4450,  4452,  4454,  4456,  4458,  4460,  4462,  4464,  4466,  4468,
    4470,  4472,  4474,  4476,  4478,  4480,  4482,  4484,  4486,  4488,
    4490,  4492,  4494,  4496,  4498,  4500,  4502,  4504,  4506,  4508,
    4510,  4512,  4514,  4516,  4518,  4520,  4522,  4524,  4526,  4528,
    4530,  4532,  4534,  4536,  4538,  4540,  4542,  4544,  4546,  4548,
    4550,  4552,  4554,  4556,  4558,  4560,  4562,  4564,  4566,  4568,
    4570,  4572,  4574,  4576,  4578,  4580,  4582,  4584,  4586,  4588,
    4590,  4592,  4594,  4596,  4598,  4600,  4602,  4604,  4606,  4608,
    4610,  4612,  4614,  4616,  4618,  4620,  4622,  4624,  4626,  4628,
    4630,  4632,  4634,  4636,  4638,  4640,  4642,  4644,  4646,  4648,
    4650,  4652,  4654,  4656,  4658,  4660,  4662,  4664,  4666,  4668,
    4670,  4672,  4674,  4676,  4678,  4680,  4682,  4684,  4686,  4688,
    4690,  4692,  4694,  4696,  4698,  4700,  4702,  4704,  4706,  4708,
    4710,  4712,  4714,  4716,  4718,  4720,  4722,  4724,  4726,  4728,
    4730,  4732,  4734,  4736,  4738,  4740,  4742,  4744,  4746,  4748,
    4750,  4752,  4754,  4756,  4758,  4760,  4762,  4764,  4766,  4768,
    4770,  4772,  4774,  4776,  4778,  4780,  4782,  4784,  4786,  4788,
    4790,  4792,  4794,  4796,  4798,  4800,  4802,  4804,  4806,  4808,
    4810,  4812,  4814,  4816,  4818,  4820,  4822,  4824,  4826,  4828,
    4830,  4832,  4834,  4836,  4838,  4840,  4842,  4844,  4846,  4848,
    4850,  4852,  4854,  4856,  4858,  4860,  4862,  4864,  4866,  4868,
    4870,  4872,  4874,  4876,  4878,  4880,  4882,  4884,  4886,  4888,
    4890,  4892,  4894,  4896,  4898,  4900,  4902,  4904,  4906,  4908,
    4910,  4912,  4914,  4916,  4918,  4920,  4922,  4924,  4926,  4928,
    4930,  4932,  4934,  4936,  4938,  4940,  4942,  4944,  4946,  4948,
    4950,  4952,  4954,  4956,  4958,  4960,  4962,  4964,  4966,  4968,
    4970,  4972,  4974,  4976,  4978,  4980,  4982,  4984,  4986,  4988,
    4990,  4992,  4994,  4996,  4998,  5000,  5002,  5004,  5006,  5008,
    5010,  5012,  5014,  5016
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     367,     0,    -1,   368,    -1,   368,   363,   369,    -1,   369,
      -1,   590,    -1,   591,    -1,   596,    -1,   548,    -1,   382,
      -1,   565,    -1,   566,    -1,   457,    -1,   406,    -1,   376,
      -1,   375,    -1,   378,    -1,   377,    -1,   601,    -1,   405,
      -1,   414,    -1,   599,    -1,   507,    -1,   402,    -1,   415,
      -1,   451,    -1,   488,    -1,   557,    -1,   598,    -1,   595,
      -1,   534,    -1,   381,    -1,   495,    -1,   464,    -1,   385,
      -1,   456,    -1,   424,    -1,   471,    -1,   474,    -1,   370,
      -1,   374,    -1,   586,    -1,   616,    -1,   628,    -1,   490,
      -1,   621,    -1,   489,    -1,   559,    -1,   384,    -1,   500,
      -1,   469,    -1,   575,    -1,   501,    -1,   473,    -1,   487,
      -1,   379,    -1,   380,    -1,   594,    -1,   614,    -1,   607,
      -1,   510,    -1,   513,    -1,   524,    -1,   528,    -1,   617,
      -1,   577,    -1,   585,    -1,   623,    -1,   576,    -1,   610,
      -1,   560,    -1,   552,    -1,   551,    -1,   554,    -1,   563,
      -1,   514,    -1,   525,    -1,   567,    -1,   631,    -1,   579,
      -1,   506,    -1,   578,    -1,   627,    -1,   600,    -1,   401,
      -1,   389,    -1,   400,    -1,   584,    -1,    -1,    62,   255,
     746,   371,   372,    -1,   331,    -1,    -1,   372,   373,    -1,
      -1,   226,   745,    -1,   100,   226,   745,    -1,   311,   226,
     745,    -1,   287,    -1,   199,    -1,   141,    -1,   196,    -1,
      63,    -1,   193,    -1,    64,    -1,   194,    -1,    65,    -1,
     195,    -1,   178,    -1,   197,    -1,    56,   170,   747,    -1,
     321,   316,   745,    -1,   318,   735,    -1,   289,   744,    -1,
       8,   735,    -1,   255,   735,    -1,   137,   255,   735,    -1,
     137,   127,   735,    -1,    62,   318,   746,   371,   372,    -1,
      13,   255,   746,   371,   372,    -1,    13,   255,   746,   270,
     390,    -1,    13,   255,   746,   401,    -1,    13,   318,   746,
     371,   372,    -1,    13,   318,   746,   270,   390,    -1,    13,
     318,   746,   401,    -1,    95,   255,   735,    -1,    95,   318,
     735,    -1,    62,   127,   746,   371,   372,    -1,    13,   127,
     746,   383,   318,   735,    -1,     7,    -1,    95,    -1,    95,
     127,   735,    -1,    62,   261,   386,    25,   746,   387,    -1,
      62,   261,   748,   387,    -1,   748,    -1,    -1,   387,   388,
      -1,    -1,   424,    -1,   528,    -1,   456,    -1,   474,    -1,
     513,    -1,   584,    -1,   270,   390,    -1,   270,   173,   390,
      -1,   270,   268,   390,    -1,   391,   299,   392,    -1,   391,
     346,   392,    -1,   297,   336,   397,    -1,   302,   582,    -1,
     268,    44,    19,   302,   582,    -1,   186,   398,    -1,   255,
     399,    -1,   268,    25,   399,    -1,   268,    25,    81,    -1,
     748,    -1,   391,   362,   748,    -1,   393,    -1,    81,    -1,
     394,    -1,   393,   364,   394,    -1,   396,    -1,   399,    -1,
     461,    -1,   239,   310,    -1,   239,    55,    -1,   247,   239,
      -1,   267,    -1,   306,    -1,   111,    -1,   214,    -1,   210,
      -1,   745,    -1,   338,    -1,   688,   745,   690,    -1,   688,
     359,   744,   360,   745,   690,    -1,   461,    -1,    81,    -1,
     173,    -1,   745,    -1,    81,    -1,    -1,   748,    -1,   340,
      -1,   273,   391,    -1,   273,   297,   336,    -1,   273,   302,
     158,   168,    -1,   273,   268,    25,    -1,   273,    11,    -1,
     249,   391,    -1,   249,   297,   336,    -1,   249,   302,   158,
     168,    -1,   249,   268,    25,    -1,   249,    11,    -1,   270,
      58,   403,   404,    -1,    11,    -1,   735,    -1,    84,    -1,
     134,    -1,    46,    -1,    13,   291,   662,   407,    -1,    13,
     140,   662,   409,    -1,   408,    -1,   407,   364,   408,    -1,
       7,   564,   429,    -1,    13,   564,   748,   411,    -1,    13,
     564,   748,    95,   200,   205,    -1,    13,   564,   748,   270,
     200,   205,    -1,    13,   564,   748,   270,   281,   463,    -1,
      13,   564,   748,   270,   284,   748,    -1,    95,   564,   748,
     412,    -1,    13,   564,   748,   309,   667,   413,    -1,     7,
     436,    -1,    95,    57,   736,   412,    -1,   270,   332,   212,
      -1,    62,   300,   291,    -1,    49,   214,   736,    -1,   270,
     332,    49,    -1,    98,   304,   736,    -1,    98,   304,    11,
      -1,    98,   304,   318,    -1,    90,   304,   736,    -1,    90,
     304,    11,    -1,    90,   304,   318,    -1,   410,    -1,   410,
      -1,   409,   364,   410,    -1,   224,   299,   746,    -1,   270,
     292,   736,    -1,   270,    81,   691,    -1,    95,    81,    -1,
      38,    -1,   251,    -1,    -1,   319,   691,    -1,    -1,    48,
     736,    -1,    61,   420,   734,   438,   421,   416,   417,   422,
     371,   418,    -1,   120,    -1,   299,    -1,   745,    -1,   282,
      -1,   283,    -1,   418,   419,    -1,    -1,    31,    -1,   212,
      -1,    87,   597,   745,    -1,   205,   597,   745,    -1,    67,
      -1,   130,    -1,   238,   597,   745,    -1,   102,   597,   745,
      -1,   116,   238,   439,    -1,   116,   200,   205,   439,    -1,
      31,    -1,    -1,   331,   212,    -1,    -1,   423,    88,   745,
      -1,    -1,   319,    -1,    -1,    62,   425,   291,   734,   359,
     426,   360,   446,   447,   448,   449,    -1,    62,   425,   291,
     734,   209,   734,   359,   426,   360,   447,   448,   449,    -1,
     295,    -1,   293,    -1,   173,   295,    -1,   173,   293,    -1,
     123,   295,    -1,   123,   293,    -1,    -1,   427,    -1,    -1,
     428,    -1,   427,   364,   428,    -1,   429,    -1,   434,    -1,
     436,    -1,   748,   667,   430,    -1,   430,   431,    -1,    -1,
      57,   736,   432,    -1,   432,    -1,   433,    -1,   200,   205,
      -1,   205,    -1,   313,   450,    -1,   233,   160,   450,    -1,
      45,   359,   691,   360,    -1,    81,   692,    -1,   242,   734,
     438,   441,   442,    -1,    83,    -1,   200,    83,    -1,   143,
      84,    -1,   143,   134,    -1,   169,   734,   435,    -1,   138,
      82,    -1,   104,    82,    -1,    -1,    57,   736,   437,    -1,
     437,    -1,    45,   359,   691,   360,    -1,   313,   359,   439,
     360,   450,    -1,   233,   160,   359,   439,   360,   450,    -1,
     117,   160,   359,   439,   360,   242,   734,   438,   441,   442,
     484,    -1,   359,   439,   360,    -1,    -1,   440,    -1,   439,
     364,   440,    -1,   748,    -1,   179,   121,    -1,   179,   225,
      -1,   179,   275,    -1,    -1,   443,    -1,   444,    -1,   443,
     444,    -1,   444,   443,    -1,    -1,   214,   317,   445,    -1,
     214,    86,   445,    -1,   192,     6,    -1,   251,    -1,    38,
      -1,   270,   205,    -1,   270,    81,    -1,   142,   359,   733,
     360,    -1,    -1,   331,   212,    -1,   332,   212,    -1,    -1,
     214,    54,    95,    -1,   214,    54,    86,   258,    -1,   214,
      54,   230,   258,    -1,    -1,   292,   736,    -1,    -1,   319,
     140,   292,   736,    -1,    -1,    62,   425,   291,   734,   453,
     452,   631,    -1,   331,   212,    19,    -1,   332,   212,    19,
      -1,    19,    -1,   359,   454,   360,    -1,    -1,   455,    -1,
     454,   364,   455,    -1,   748,    -1,    62,   425,   266,   734,
     458,    -1,    13,   266,   734,   458,    -1,   458,   459,    -1,
      -1,    36,   461,    -1,    74,    -1,   192,    74,    -1,   139,
     460,   461,    -1,   180,   461,    -1,   182,   461,    -1,   192,
     180,    -1,   192,   182,    -1,   279,   371,   461,    -1,   250,
     371,   461,    -1,    35,    -1,    -1,   462,    -1,   463,    -1,
     339,    -1,   351,   339,    -1,   747,    -1,    62,   465,   470,
     162,   399,    -1,    62,   465,   470,   162,   399,   128,   466,
     467,   468,    -1,   308,    -1,    -1,   736,    -1,   736,   505,
      -1,   322,   466,    -1,    -1,   161,   745,    -1,    -1,    95,
     470,   162,   399,   412,    -1,   236,    -1,    -1,    62,   292,
     736,   472,   176,   745,    -1,   224,   736,    -1,    -1,    95,
     292,   736,    -1,    62,   304,   736,   475,   476,   214,   734,
     478,   106,   237,   742,   359,   481,   360,    -1,    62,    57,
     304,   736,     9,   476,   214,   734,   483,   484,   115,    96,
     257,   106,   237,   742,   359,   481,   360,    -1,    27,    -1,
       9,    -1,   477,    -1,   477,   218,   477,    -1,   477,   218,
     477,   218,   477,    -1,   148,    -1,    86,    -1,   317,    -1,
     115,   479,   480,    -1,    -1,    96,    -1,    -1,   257,    -1,
     280,    -1,   482,    -1,   481,   364,   482,    -1,    -1,   344,
      -1,   339,    -1,   745,    -1,   341,    -1,   342,    -1,   748,
      -1,   120,   734,    -1,    -1,   485,    -1,   485,   486,    -1,
     486,    -1,   486,   485,    -1,    -1,   200,    83,    -1,    83,
      -1,   143,   134,    -1,   143,    84,    -1,    95,   304,   736,
     214,   734,   412,    -1,    62,    21,   736,    45,   359,   691,
     360,   484,    -1,    95,    21,   736,   412,    -1,    62,    10,
     742,   491,    -1,    62,   216,   556,   491,    -1,    62,   309,
     504,   491,    -1,    62,   309,   504,    19,   359,   665,   360,
      -1,   359,   492,   360,    -1,   493,    -1,   492,   364,   493,
      -1,   751,   346,   494,    -1,   751,    -1,   542,    -1,   700,
      -1,   461,    -1,   745,    -1,    62,   216,    47,   504,   498,
     115,   309,   667,   319,   738,    19,   496,    -1,   497,    -1,
     496,   364,   497,    -1,   216,   744,   556,   499,    -1,   216,
     744,   556,   359,   555,   360,   499,    -1,   122,   744,   742,
     536,    -1,   284,   667,    -1,    81,    -1,    -1,   241,    -1,
      -1,    95,   216,    47,   504,   319,   738,   412,    -1,    95,
     502,   503,   412,    -1,   291,    -1,   266,    -1,   327,    -1,
     140,    -1,   309,    -1,    93,    -1,    59,    -1,   261,    -1,
     504,    -1,   503,   364,   504,    -1,   748,    -1,   748,   505,
      -1,   362,   739,    -1,   505,   362,   739,    -1,   307,   638,
     733,    -1,    53,   214,   508,   504,   156,   509,    -1,    53,
     214,    10,   742,   359,   553,   360,   156,   509,    -1,    53,
     214,   122,   742,   536,   156,   509,    -1,    53,   214,   216,
     556,   359,   555,   360,   156,   509,    -1,    53,   214,    57,
     736,   214,   504,   156,   509,    -1,    53,   214,   259,   736,
     214,   504,   156,   509,    -1,    53,   214,   259,   736,   156,
     509,    -1,    53,   214,   304,   736,   214,   504,   156,   509,
      -1,    53,   214,   216,    47,   504,   319,   738,   156,   509,
      -1,    53,   214,   163,   208,   461,   156,   509,    -1,    53,
     214,    40,   359,   667,    19,   667,   360,   156,   509,    -1,
      53,   214,   470,   162,   504,   156,   509,    -1,    52,    -1,
      75,    -1,   261,    -1,   140,    -1,   266,    -1,   291,    -1,
      93,    -1,   309,    -1,   327,    -1,    59,    -1,   745,    -1,
     205,    -1,   112,   511,   512,   736,    -1,   112,   736,    -1,
     185,   511,   512,   736,    -1,   185,   736,    -1,    -1,   191,
      -1,   234,    -1,   113,    -1,   164,    -1,     4,   747,    -1,
     244,   747,    -1,   747,    -1,    11,    -1,   118,    -1,   118,
     747,    -1,   118,    11,    -1,    26,    -1,    26,   747,    -1,
      26,    11,    -1,   120,    -1,   137,    -1,   124,   515,   214,
     518,   299,   519,   521,    -1,   253,   515,   214,   518,   120,
     519,   412,    -1,   253,   124,   217,   115,   515,   214,   518,
     120,   519,   412,    -1,   516,    -1,    11,    -1,    11,   235,
      -1,   517,    -1,   516,   364,   517,    -1,   265,    -1,   242,
      -1,    62,    -1,   748,    -1,   733,    -1,   291,   733,    -1,
     122,   522,    -1,    75,   735,    -1,   162,   735,    -1,   261,
     735,    -1,   292,   735,    -1,   520,    -1,   519,   364,   520,
      -1,   746,    -1,   127,   746,    -1,   331,   124,   217,    -1,
      -1,   523,    -1,   522,   364,   523,    -1,   742,   536,    -1,
     124,   516,   299,   735,   526,   527,    -1,   253,   516,   120,
     735,   527,   412,    -1,   253,     8,   217,   115,   516,   120,
     735,   527,   412,    -1,   331,     8,   217,    -1,    -1,   125,
      35,   746,    -1,    -1,    62,   529,   140,   740,   214,   734,
     530,   359,   531,   360,   449,   664,    -1,   313,    -1,    -1,
     319,   738,    -1,    -1,   532,    -1,   531,   364,   532,    -1,
     748,   533,    -1,   694,   533,    -1,   359,   691,   360,   533,
      -1,   504,    -1,   319,   504,    -1,    -1,    62,   535,   122,
     742,   536,   252,   541,   543,   547,    -1,    62,   535,   122,
     742,   536,   543,   547,    -1,   218,   248,    -1,    -1,   359,
     537,   360,    -1,   359,   360,    -1,   538,    -1,   537,   364,
     538,    -1,   539,   540,   542,    -1,   540,   539,   542,    -1,
     540,   542,    -1,   539,   542,    -1,   542,    -1,   137,    -1,
     220,    -1,   145,    -1,   137,   220,    -1,   750,    -1,   542,
      -1,   667,    -1,   749,   505,   354,   309,    -1,   271,   749,
     505,   354,   309,    -1,   545,    -1,   543,   545,    -1,    37,
     214,   205,   146,    -1,   252,   205,   214,   205,   146,    -1,
     285,    -1,   135,    -1,   278,    -1,   328,    -1,   109,   264,
      85,    -1,   109,   264,   155,    -1,   264,    85,    -1,   264,
     155,    -1,    19,   546,    -1,   162,   399,    -1,   544,    -1,
     745,    -1,   745,   364,   745,    -1,   331,   491,    -1,    -1,
      13,   122,   523,   549,   550,    -1,   544,    -1,   549,   544,
      -1,   251,    -1,    -1,    95,   122,   742,   536,   412,    -1,
      95,    10,   742,   359,   553,   360,   412,    -1,   667,    -1,
     352,    -1,    95,   216,   556,   359,   555,   360,   412,    -1,
     667,    -1,   667,   364,   667,    -1,   198,   364,   667,    -1,
     667,   364,   198,    -1,   697,    -1,   748,   362,   556,    -1,
      62,    40,   359,   667,    19,   667,   360,   331,   122,   523,
     558,    -1,    62,    40,   359,   667,    19,   667,   360,   332,
     122,   558,    -1,    19,   136,    -1,    19,    22,    -1,    -1,
      95,    40,   359,   667,    19,   667,   360,   412,    -1,   243,
     561,   734,   562,    -1,   243,   290,   736,   562,    -1,   243,
      75,   736,   562,    -1,   140,    -1,   291,    -1,   116,    -1,
      -1,    13,    10,   742,   359,   553,   360,   246,   299,   736,
      -1,    13,    59,   504,   246,   299,   736,    -1,    13,    75,
     737,   246,   299,   737,    -1,    13,   122,   742,   536,   246,
     299,   736,    -1,    13,   127,   746,   246,   299,   746,    -1,
      13,   162,   736,   246,   299,   736,    -1,    13,   216,    47,
     504,   319,   738,   246,   299,   736,    -1,    13,   261,   736,
     246,   299,   736,    -1,    13,   291,   662,   246,   299,   736,
      -1,    13,   140,   662,   246,   299,   736,    -1,    13,   291,
     662,   246,   564,   736,   299,   736,    -1,    13,   304,   736,
     214,   662,   246,   299,   736,    -1,    13,   255,   746,   246,
     299,   746,    -1,    13,   318,   746,   246,   299,   746,    -1,
      13,   292,   736,   246,   299,   736,    -1,    52,    -1,    -1,
      13,    10,   742,   359,   553,   360,   270,   261,   736,    -1,
      13,    93,   504,   270,   261,   736,    -1,    13,   122,   742,
     536,   270,   261,   736,    -1,    13,   266,   662,   270,   261,
     736,    -1,    13,   291,   662,   270,   261,   736,    -1,    13,
     309,   504,   270,   261,   736,    -1,    13,    10,   742,   359,
     553,   360,   224,   299,   746,    -1,    13,    59,   504,   224,
     299,   746,    -1,    13,    75,   737,   224,   299,   746,    -1,
      13,    93,   504,   224,   299,   746,    -1,    13,   122,   742,
     536,   224,   299,   746,    -1,    13,   216,   556,   359,   555,
     360,   224,   299,   746,    -1,    13,   216,    47,   504,   319,
     738,   224,   299,   746,    -1,    13,   261,   736,   224,   299,
     746,    -1,    13,   309,   504,   224,   299,   746,    -1,    13,
     292,   736,   224,   299,   746,    -1,    -1,    62,   535,   259,
     736,    19,   568,   214,   573,   299,   734,   664,    92,   574,
     569,    -1,   201,    -1,   571,    -1,   359,   570,   360,    -1,
     570,   363,   572,    -1,   572,    -1,   631,    -1,   617,    -1,
     627,    -1,   621,    -1,   576,    -1,   571,    -1,    -1,   265,
      -1,   317,    -1,    86,    -1,   148,    -1,   149,    -1,    12,
      -1,    -1,    95,   259,   736,   214,   734,   412,    -1,   202,
     734,    -1,   171,   734,    -1,   315,   734,    -1,   315,   352,
      -1,     3,   580,    -1,    28,   580,   583,    -1,   279,   302,
     583,    -1,    54,   580,    -1,   101,   580,    -1,   256,   580,
      -1,   260,   748,    -1,   245,   260,   748,    -1,   245,   748,
      -1,   256,   580,   299,   260,   748,    -1,   256,   580,   299,
     748,    -1,   231,   302,   745,    -1,    54,   232,   745,    -1,
     256,   232,   745,    -1,   333,    -1,   302,    -1,    -1,   158,
     168,   395,    -1,   239,   215,    -1,   239,   334,    -1,   581,
      -1,   582,   364,   581,    -1,   582,   581,    -1,   582,    -1,
      -1,    62,   425,   327,   734,   438,    19,   631,    -1,    62,
     218,   248,   425,   327,   734,   438,    19,   631,    -1,   172,
     741,    -1,    62,    75,   737,   371,   587,    -1,   587,   588,
      -1,    -1,   292,   589,   736,    -1,   292,   589,    81,    -1,
     176,   589,   745,    -1,   176,   589,    81,    -1,   294,   589,
     736,    -1,   294,   589,    81,    -1,    99,   589,   745,    -1,
      99,   589,   744,    -1,    99,   589,    81,    -1,    56,   170,
     589,   747,    -1,   224,   589,   736,    -1,   224,   589,    81,
      -1,   346,    -1,    -1,    13,    75,   737,   371,   592,    -1,
      13,    75,   737,   270,   390,    -1,    13,    75,   737,   401,
      -1,   592,   593,    -1,    -1,    56,   170,   589,   747,    -1,
      95,    75,   737,    -1,    62,    93,   504,   597,   667,   430,
      -1,    13,    93,   504,   411,    -1,    13,    93,   504,    95,
     200,   205,    -1,    13,    93,   504,   270,   200,   205,    -1,
      13,    93,   504,     7,   436,    -1,    13,    93,   504,    95,
      57,   736,   412,    -1,    19,    -1,    -1,    62,   498,    59,
     504,   115,   745,   299,   745,   120,   504,    -1,    49,   740,
     214,   734,    -1,    49,   734,    -1,    49,    -1,   320,   604,
     605,   603,    -1,   320,   604,   605,   603,   734,    -1,   320,
     604,   605,   603,   601,    -1,   602,   603,    -1,   602,   603,
     734,   606,    -1,    15,    -1,    14,    -1,   326,    -1,    -1,
     121,    -1,    -1,   119,    -1,    -1,   359,   735,   360,    -1,
      -1,   108,   609,   603,   608,    -1,   631,    -1,   617,    -1,
     627,    -1,   621,    -1,   628,    -1,   614,    -1,   602,    -1,
      -1,   231,   736,   611,    19,   613,    -1,   359,   612,   360,
      -1,    -1,   667,    -1,   612,   364,   667,    -1,   631,    -1,
     617,    -1,   627,    -1,   621,    -1,   106,   736,   615,    -1,
      62,   425,   291,   734,   453,    19,   106,   736,   615,    -1,
     359,   702,   360,    -1,    -1,    77,   736,    -1,    77,   231,
     736,    -1,   148,   154,   734,   618,    -1,   323,   359,   730,
     360,    -1,    81,   323,    -1,   631,    -1,   359,   619,   360,
     323,   359,   730,   360,    -1,   359,   619,   360,   631,    -1,
     620,    -1,   619,   364,   620,    -1,   748,   724,    -1,    86,
     120,   662,   622,   664,    -1,   319,   655,    -1,    -1,   177,
     638,   733,   624,   626,    -1,   137,   625,   183,    -1,    -1,
       5,   272,    -1,   257,   272,    -1,   257,   105,    -1,   272,
     317,   105,    -1,   272,    -1,   272,   257,   105,    -1,   105,
      -1,     5,   105,    -1,   204,    -1,    -1,   317,   662,   270,
     728,   654,   664,    -1,    80,   736,   629,    73,   630,   115,
     631,    -1,    -1,   629,   192,   262,    -1,   629,   262,    -1,
     629,    31,    -1,   629,   147,    -1,    -1,   331,   131,    -1,
     332,   131,    -1,   633,    -1,   632,    -1,   359,   633,   360,
      -1,   359,   632,   360,    -1,   635,    -1,   634,   642,    -1,
     634,   641,   651,   646,    -1,   634,   641,   645,   652,    -1,
     635,    -1,   632,    -1,   265,   640,   726,   636,   654,   664,
     649,   650,    -1,   634,   312,   639,   634,    -1,   634,   152,
     639,   634,    -1,   634,   103,   639,   634,    -1,   154,   637,
      -1,    -1,   295,   638,   734,    -1,   293,   638,   734,    -1,
     173,   295,   638,   734,    -1,   173,   293,   638,   734,    -1,
     123,   295,   638,   734,    -1,   123,   293,   638,   734,    -1,
     291,   734,    -1,   734,    -1,   291,    -1,    -1,    11,    -1,
      91,    -1,    -1,    91,    -1,    91,   214,   359,   702,   360,
      -1,    11,    -1,    -1,   642,    -1,    -1,   219,    35,   643,
      -1,   644,    -1,   643,   364,   644,    -1,   691,   319,   700,
      -1,   691,    20,    -1,   691,    89,    -1,   691,    -1,   170,
     647,   211,   648,    -1,   211,   648,   170,   647,    -1,   170,
     647,    -1,   211,   648,    -1,   170,   647,   364,   648,    -1,
     645,    -1,    -1,   691,    -1,    11,    -1,   691,    -1,   127,
      35,   702,    -1,    -1,   129,   691,    -1,    -1,   115,   317,
     653,   626,    -1,   115,   272,   653,   626,    -1,   115,   239,
     215,    -1,   651,    -1,    -1,   209,   735,    -1,    -1,   120,
     655,    -1,    -1,   656,    -1,   655,   364,   656,    -1,   662,
      -1,   662,   658,    -1,   663,    -1,   663,   658,    -1,   663,
      19,   359,   665,   360,    -1,   663,    19,   748,   359,   665,
     360,    -1,   663,   748,   359,   665,   360,    -1,   632,    -1,
     632,   658,    -1,   657,    -1,   359,   657,   360,   658,    -1,
     359,   657,   360,    -1,   656,    66,   159,   656,    -1,   656,
     337,   656,    -1,   656,   659,   159,   656,   661,    -1,   656,
     159,   656,   661,    -1,   656,   188,   659,   159,   656,    -1,
     656,   188,   159,   656,    -1,    19,   748,   359,   735,   360,
      -1,    19,   748,    -1,   748,   359,   735,   360,    -1,   748,
      -1,   121,   660,    -1,   167,   660,    -1,   254,   660,    -1,
     144,    -1,   221,    -1,    -1,   319,   359,   735,   360,    -1,
     214,   691,    -1,   734,    -1,   734,   352,    -1,   215,   734,
      -1,   215,   359,   734,   360,    -1,   694,    -1,   330,   691,
      -1,    -1,   666,    -1,   665,   364,   666,    -1,   748,   667,
      -1,   669,   668,    -1,   271,   669,   668,    -1,   669,    18,
     357,   744,   358,    -1,   271,   669,    18,   357,   744,   358,
      -1,   668,   357,   358,    -1,   668,   357,   744,   358,    -1,
      -1,   671,    -1,   672,    -1,   676,    -1,   680,    -1,   687,
      -1,   688,   690,    -1,   688,   359,   744,   360,   690,    -1,
     749,   505,    -1,   671,    -1,   672,    -1,   677,    -1,   681,
      -1,   687,    -1,   749,    -1,   150,    -1,   151,    -1,   276,
      -1,    30,    -1,   240,    -1,   114,   673,    -1,    94,   229,
      -1,    79,   675,    -1,    78,   675,    -1,   207,   674,    -1,
      33,    -1,   359,   744,   360,    -1,    -1,   359,   744,   364,
     744,   360,    -1,   359,   744,   360,    -1,    -1,   359,   744,
     364,   744,   360,    -1,   359,   744,   360,    -1,    -1,   678,
      -1,   679,    -1,   678,    -1,   679,    -1,    32,   685,   359,
     744,   360,    -1,    32,   685,    -1,   682,    -1,   683,    -1,
     682,    -1,   683,    -1,   684,   359,   744,   360,   686,    -1,
     684,   686,    -1,    43,   685,    -1,    42,   685,    -1,   324,
      -1,   187,    43,   685,    -1,   187,    42,   685,    -1,   189,
     685,    -1,   325,    -1,    -1,    43,   270,   748,    -1,    -1,
     298,   359,   744,   360,   689,    -1,   298,   689,    -1,   297,
     359,   744,   360,   689,    -1,   297,   689,    -1,   153,    -1,
     331,   297,   336,    -1,   332,   297,   336,    -1,    -1,   335,
      -1,   184,    -1,    76,    -1,   132,    -1,   181,    -1,   263,
      -1,   335,   299,   184,    -1,    76,   299,   132,    -1,    76,
     299,   181,    -1,    76,   299,   263,    -1,   132,   299,   181,
      -1,   132,   299,   263,    -1,   181,   299,   263,    -1,    -1,
     693,    -1,   691,   361,   667,    -1,   691,    24,   297,   336,
     691,    -1,   350,   691,    -1,   351,   691,    -1,   691,   350,
     691,    -1,   691,   351,   691,    -1,   691,   352,   691,    -1,
     691,   353,   691,    -1,   691,   354,   691,    -1,   691,   355,
     691,    -1,   691,   347,   691,    -1,   691,   348,   691,    -1,
     691,   346,   691,    -1,   691,   699,   691,    -1,   699,   691,
      -1,   691,   699,    -1,   691,    16,   691,    -1,   691,   218,
     691,    -1,   200,   691,    -1,   691,   169,   691,    -1,   691,
     169,   691,   102,   691,    -1,   691,   200,   169,   691,    -1,
     691,   200,   169,   691,   102,   691,    -1,   691,   133,   691,
      -1,   691,   133,   691,   102,   691,    -1,   691,   200,   133,
     691,    -1,   691,   200,   133,   691,   102,   691,    -1,   691,
     274,   299,   691,    -1,   691,   274,   299,   691,   102,   691,
      -1,   691,   200,   274,   299,   691,    -1,   691,   200,   274,
     299,   691,   102,   691,    -1,   691,   157,    -1,   691,   156,
     205,    -1,   691,   203,    -1,   691,   156,   200,   205,    -1,
     695,   222,   695,    -1,   691,   156,   306,    -1,   691,   156,
     200,   306,    -1,   691,   156,   111,    -1,   691,   156,   200,
     111,    -1,   691,   156,   314,    -1,   691,   156,   200,   314,
      -1,   691,   156,    91,   120,   691,    -1,   691,   156,   209,
     359,   704,   360,    -1,   691,   156,   200,   209,   359,   704,
     360,    -1,   691,    29,   725,   692,    16,   692,    -1,   691,
     200,    29,   725,   692,    16,   692,    -1,   691,    29,   288,
     692,    16,   692,    -1,   691,   200,    29,   288,   692,    16,
     692,    -1,   691,   137,   715,    -1,   691,   200,   137,   715,
      -1,   691,   701,   696,   632,    -1,   691,   701,   696,   359,
     691,   360,    -1,   313,   632,    -1,   693,    -1,   692,   361,
     667,    -1,   350,   692,    -1,   351,   692,    -1,   692,   350,
     692,    -1,   692,   351,   692,    -1,   692,   352,   692,    -1,
     692,   353,   692,    -1,   692,   354,   692,    -1,   692,   355,
     692,    -1,   692,   347,   692,    -1,   692,   348,   692,    -1,
     692,   346,   692,    -1,   692,   699,   692,    -1,   699,   692,
      -1,   692,   699,    -1,   692,   156,    91,   120,   692,    -1,
     692,   156,   209,   359,   704,   360,    -1,   692,   156,   200,
     209,   359,   704,   360,    -1,   721,    -1,   743,    -1,   345,
     724,    -1,   359,   691,   360,   724,    -1,   716,    -1,   694,
      -1,   632,    -1,   107,   632,    -1,    18,   632,    -1,    18,
     706,    -1,   695,    -1,   742,   359,   360,    -1,   742,   359,
     702,   360,    -1,   742,   359,    11,   702,   360,    -1,   742,
     359,    91,   702,   360,    -1,   742,   359,   352,   360,    -1,
      68,    -1,    70,    -1,    70,   359,   744,   360,    -1,    71,
      -1,    71,   359,   744,   360,    -1,   174,    -1,   174,   359,
     744,   360,    -1,   175,    -1,   175,   359,   744,   360,    -1,
      69,    -1,    72,    -1,   269,    -1,   318,    -1,    40,   359,
     691,    19,   667,   360,    -1,   110,   359,   703,   360,    -1,
     223,   359,   708,   360,    -1,   228,   359,   710,   360,    -1,
     286,   359,   711,   360,    -1,   303,   359,   691,    19,   667,
     360,    -1,   305,   359,    34,   714,   360,    -1,   305,   359,
     165,   714,   360,    -1,   305,   359,   301,   714,   360,    -1,
     305,   359,   714,   360,    -1,    60,   359,   691,   319,   504,
     360,    -1,    60,   359,   702,   360,    -1,   206,   359,   691,
     364,   691,   360,    -1,    50,   359,   702,   360,    -1,   126,
     359,   702,   360,    -1,   166,   359,   702,   360,    -1,   257,
     359,   702,   360,    -1,   257,   359,   360,    -1,   359,   702,
     364,   691,   360,    -1,    17,    -1,   277,    -1,    11,    -1,
     343,    -1,   698,    -1,   350,    -1,   351,    -1,   352,    -1,
     353,    -1,   354,    -1,   355,    -1,   347,    -1,   348,    -1,
     346,    -1,   343,    -1,   216,   359,   556,   360,    -1,   697,
      -1,   216,   359,   556,   360,    -1,   697,    -1,   216,   359,
     556,   360,    -1,   169,    -1,   200,   169,    -1,   133,    -1,
     200,   133,    -1,   691,    -1,   702,   364,   691,    -1,   707,
     120,   691,    -1,    -1,   704,   364,   667,    -1,   667,    -1,
     706,    -1,   705,   364,   706,    -1,   357,   702,   358,    -1,
     357,   705,   358,    -1,   338,    -1,   335,    -1,   184,    -1,
      76,    -1,   132,    -1,   181,    -1,   263,    -1,   340,    -1,
     691,   709,   712,   713,    -1,   691,   709,   712,    -1,   227,
     691,    -1,   692,   137,   692,    -1,    -1,   691,   712,   713,
      -1,   691,   713,   712,    -1,   691,   712,    -1,   691,   713,
      -1,   702,    -1,    -1,   120,   691,    -1,   115,   691,    -1,
     691,   120,   702,    -1,   120,   702,    -1,   702,    -1,   632,
      -1,   359,   702,   360,    -1,    39,   720,   717,   719,   101,
      -1,   718,    -1,   717,   718,    -1,   329,   691,   296,   691,
      -1,    97,   691,    -1,    -1,   691,    -1,    -1,   732,    -1,
     732,   723,    -1,   362,   739,    -1,   362,   352,    -1,   357,
     691,   358,    -1,   357,   691,   365,   691,   358,    -1,   722,
      -1,   723,   722,    -1,    -1,   724,   722,    -1,    23,    -1,
      -1,   727,    -1,   726,   364,   727,    -1,   691,    19,   751,
      -1,   691,    -1,   352,    -1,   729,    -1,   728,   364,   729,
      -1,   748,   724,   346,   691,    -1,   748,   724,   346,    81,
      -1,   731,    -1,   730,   364,   731,    -1,   691,    -1,    81,
      -1,   756,    -1,   748,    -1,   734,    -1,   733,   364,   734,
      -1,   732,    -1,   732,   723,    -1,   736,    -1,   735,   364,
     736,    -1,   748,    -1,   748,    -1,   748,    -1,   751,    -1,
     748,    -1,   745,    -1,   750,    -1,   732,   723,    -1,   744,
      -1,   339,    -1,   745,    -1,   341,    -1,   342,    -1,   670,
     745,    -1,   688,   745,   690,    -1,   688,   359,   744,   360,
     745,   690,    -1,   306,    -1,   111,    -1,   205,    -1,   344,
      -1,   340,    -1,   748,    -1,   344,    -1,   351,   344,    -1,
     338,    -1,   752,    -1,   753,    -1,   338,    -1,   752,    -1,
     338,    -1,   752,    -1,   754,    -1,   338,    -1,   752,    -1,
     753,    -1,   754,    -1,   755,    -1,     3,    -1,     4,    -1,
       5,    -1,     6,    -1,     7,    -1,     8,    -1,     9,    -1,
      10,    -1,    12,    -1,    13,    -1,    21,    -1,    22,    -1,
      24,    -1,    26,    -1,    27,    -1,    28,    -1,    35,    -1,
      36,    -1,    37,    -1,    38,    -1,    41,    -1,    44,    -1,
      46,    -1,    47,    -1,    48,    -1,    49,    -1,    53,    -1,
      54,    -1,    55,    -1,    56,    -1,    58,    -1,    59,    -1,
      61,    -1,    63,    -1,    64,    -1,    65,    -1,    67,    -1,
      73,    -1,    74,    -1,    75,    -1,    76,    -1,    77,    -1,
      80,    -1,    82,    -1,    84,    -1,    85,    -1,    86,    -1,
      87,    -1,    88,    -1,    90,    -1,    93,    -1,    94,    -1,
      95,    -1,    96,    -1,    98,    -1,    99,    -1,   100,    -1,
     102,    -1,   104,    -1,   105,    -1,   106,    -1,   108,    -1,
     109,    -1,   112,    -1,   113,    -1,   116,    -1,   118,    -1,
     122,    -1,   123,    -1,   125,    -1,   128,    -1,   130,    -1,
     131,    -1,   132,    -1,   134,    -1,   135,    -1,   136,    -1,
     138,    -1,   139,    -1,   140,    -1,   141,    -1,   142,    -1,
     146,    -1,   147,    -1,   148,    -1,   149,    -1,   155,    -1,
     158,    -1,   160,    -1,   161,    -1,   162,    -1,   163,    -1,
     164,    -1,   168,    -1,   171,    -1,   172,    -1,   173,    -1,
     176,    -1,   177,    -1,   178,    -1,   179,    -1,   180,    -1,
     181,    -1,   182,    -1,   183,    -1,   184,    -1,   185,    -1,
     186,    -1,   191,    -1,   192,    -1,   193,    -1,   194,    -1,
     195,    -1,   196,    -1,   197,    -1,   199,    -1,   201,    -1,
     202,    -1,   204,    -1,   208,    -1,   209,    -1,   212,    -1,
     216,    -1,   217,    -1,   224,    -1,   225,    -1,   226,    -1,
     231,    -1,   232,    -1,   230,    -1,   234,    -1,   235,    -1,
     236,    -1,   237,    -1,   238,    -1,   239,    -1,   241,    -1,
     243,    -1,   244,    -1,   245,    -1,   246,    -1,   247,    -1,
     248,    -1,   249,    -1,   250,    -1,   251,    -1,   252,    -1,
     253,    -1,   255,    -1,   256,    -1,   258,    -1,   259,    -1,
     260,    -1,   261,    -1,   262,    -1,   263,    -1,   264,    -1,
     266,    -1,   267,    -1,   268,    -1,   270,    -1,   272,    -1,
     273,    -1,   275,    -1,   278,    -1,   279,    -1,   280,    -1,
     281,    -1,   282,    -1,   283,    -1,   284,    -1,   287,    -1,
     289,    -1,   290,    -1,   285,    -1,   292,    -1,   293,    -1,
     294,    -1,   295,    -1,   300,    -1,   302,    -1,   304,    -1,
     307,    -1,   308,    -1,   309,    -1,   310,    -1,   311,    -1,
     314,    -1,   315,    -1,   316,    -1,   317,    -1,   320,    -1,
     321,    -1,   322,    -1,   323,    -1,   325,    -1,   327,    -1,
     328,    -1,   331,    -1,   332,    -1,   333,    -1,   334,    -1,
     335,    -1,   336,    -1,    30,    -1,    32,    -1,    33,    -1,
      42,    -1,    43,    -1,    50,    -1,    60,    -1,    78,    -1,
      79,    -1,   107,    -1,   110,    -1,   114,    -1,   126,    -1,
     145,    -1,   150,    -1,   151,    -1,   153,    -1,   166,    -1,
     187,    -1,   189,    -1,   198,    -1,   206,    -1,   207,    -1,
     220,    -1,   223,    -1,   228,    -1,   229,    -1,   240,    -1,
     257,    -1,   271,    -1,   276,    -1,   286,    -1,   297,    -1,
     298,    -1,   303,    -1,   305,    -1,   324,    -1,    25,    -1,
      29,    -1,    31,    -1,    66,    -1,   119,    -1,   121,    -1,
     133,    -1,   144,    -1,   156,    -1,   157,    -1,   159,    -1,
     167,    -1,   169,    -1,   188,    -1,   203,    -1,   221,    -1,
     222,    -1,   254,    -1,   274,    -1,   326,    -1,    11,    -1,
      14,    -1,    15,    -1,    16,    -1,    17,    -1,    18,    -1,
      19,    -1,    20,    -1,    23,    -1,    34,    -1,    39,    -1,
      40,    -1,    45,    -1,    51,    -1,    52,    -1,    57,    -1,
      62,    -1,    68,    -1,    69,    -1,    70,    -1,    71,    -1,
      72,    -1,    81,    -1,    83,    -1,    89,    -1,    91,    -1,
      92,    -1,    97,    -1,   101,    -1,   103,    -1,   111,    -1,
     115,    -1,   117,    -1,   120,    -1,   124,    -1,   127,    -1,
     129,    -1,   137,    -1,   143,    -1,   152,    -1,   154,    -1,
     165,    -1,   170,    -1,   174,    -1,   175,    -1,   190,    -1,
     200,    -1,   205,    -1,   210,    -1,   211,    -1,   213,    -1,
     214,    -1,   215,    -1,   218,    -1,   219,    -1,   227,    -1,
     233,    -1,   242,    -1,   265,    -1,   269,    -1,   277,    -1,
     288,    -1,   291,    -1,   296,    -1,   299,    -1,   301,    -1,
     306,    -1,   312,    -1,   313,    -1,   318,    -1,   319,    -1,
     329,    -1,   330,    -1,   213,    -1,   190,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   478,   478,   482,   488,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   507,   508,   509,   510,   511,
     512,   513,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,   526,   527,   528,   529,   530,   531,
     532,   533,   534,   535,   536,   537,   538,   539,   540,   541,
     542,   543,   544,   545,   546,   547,   548,   549,   550,   551,
     552,   553,   554,   555,   556,   557,   558,   559,   560,   561,
     562,   563,   564,   565,   566,   567,   568,   569,   570,   571,
     572,   573,   574,   575,   576,   577,   578,   579,   581,   591,
     602,   603,   612,   613,   617,   622,   627,   632,   636,   640,
     644,   648,   652,   656,   660,   664,   669,   673,   677,   681,
     685,   690,   695,   699,   703,   707,   711,   725,   743,   754,
     762,   780,   792,   800,   821,   839,   855,   873,   884,   885,
     897,   913,   925,   937,   938,   942,   943,   951,   952,   953,
     954,   955,   956,   970,   976,   982,   990,   997,  1004,  1012,
    1019,  1026,  1034,  1041,  1048,  1058,  1059,  1071,  1072,  1075,
    1076,  1079,  1081,  1083,  1087,  1088,  1089,  1090,  1094,  1095,
    1096,  1097,  1109,  1113,  1117,  1130,  1157,  1158,  1159,  1163,
    1164,  1165,  1169,  1170,  1175,  1181,  1187,  1193,  1199,  1208,
    1214,  1220,  1226,  1232,  1242,  1252,  1253,  1257,  1258,  1266,
    1281,  1289,  1300,  1301,  1307,  1315,  1324,  1332,  1340,  1349,
    1358,  1370,  1380,  1388,  1397,  1404,  1411,  1419,  1427,  1435,
    1442,  1449,  1457,  1464,  1470,  1477,  1478,  1484,  1492,  1502,
    1510,  1514,  1515,  1516,  1520,  1521,  1534,  1553,  1577,  1578,
    1587,  1588,  1589,  1595,  1596,  1601,  1605,  1609,  1613,  1617,
    1621,  1625,  1629,  1633,  1637,  1646,  1650,  1654,  1658,  1663,
    1667,  1671,  1672,  1683,  1697,  1723,  1724,  1725,  1726,  1727,
    1728,  1729,  1733,  1734,  1738,  1742,  1749,  1750,  1751,  1754,
    1766,  1767,  1771,  1792,  1793,  1812,  1823,  1834,  1845,  1856,
    1867,  1886,  1914,  1920,  1926,  1932,  1950,  1961,  1962,  1963,
    1972,  1993,  1997,  2007,  2018,  2029,  2047,  2048,  2052,  2053,
    2056,  2062,  2066,  2073,  2078,  2090,  2092,  2094,  2096,  2099,
    2102,  2105,  2109,  2110,  2111,  2112,  2113,  2116,  2117,  2121,
    2122,  2123,  2126,  2127,  2128,  2129,  2132,  2133,  2136,  2137,
    2147,  2176,  2177,  2178,  2182,  2183,  2187,  2188,  2192,  2218,
    2229,  2238,  2239,  2242,  2246,  2250,  2254,  2258,  2262,  2266,
    2270,  2274,  2278,  2284,  2285,  2289,  2290,  2293,  2294,  2301,
    2313,  2323,  2337,  2338,  2346,  2347,  2351,  2352,  2356,  2357,
    2361,  2371,  2372,  2382,  2392,  2393,  2406,  2423,  2441,  2465,
    2466,  2470,  2476,  2482,  2491,  2492,  2493,  2497,  2502,  2512,
    2513,  2517,  2518,  2522,  2523,  2524,  2528,  2534,  2535,  2536,
    2537,  2538,  2542,  2543,  2547,  2549,  2557,  2564,  2573,  2577,
    2578,  2582,  2583,  2588,  2609,  2628,  2651,  2659,  2667,  2675,
    2711,  2714,  2715,  2718,  2722,  2729,  2730,  2731,  2732,  2745,
    2759,  2760,  2764,  2774,  2784,  2793,  2802,  2803,  2806,  2807,
    2812,  2831,  2841,  2842,  2843,  2844,  2845,  2846,  2847,  2848,
    2852,  2853,  2856,  2857,  2860,  2862,  2875,  2901,  2910,  2920,
    2929,  2939,  2948,  2957,  2967,  2976,  2985,  2994,  3003,  3015,
    3016,  3017,  3018,  3019,  3020,  3021,  3022,  3023,  3024,  3028,
    3029,  3039,  3046,  3055,  3062,  3075,  3081,  3088,  3095,  3102,
    3109,  3116,  3123,  3130,  3137,  3144,  3151,  3158,  3165,  3172,
    3181,  3182,  3192,  3207,  3220,  3246,  3248,  3250,  3254,  3256,
    3260,  3261,  3262,  3263,  3271,  3278,  3285,  3292,  3299,  3306,
    3313,  3324,  3325,  3328,  3338,  3352,  3353,  3357,  3358,  3363,
    3379,  3392,  3402,  3414,  3415,  3418,  3419,  3434,  3450,  3451,
    3455,  3456,  3459,  3460,  3468,  3475,  3482,  3491,  3492,  3493,
    3508,  3520,  3535,  3536,  3539,  3540,  3544,  3545,  3559,  3567,
    3575,  3583,  3591,  3602,  3603,  3604,  3605,  3611,  3615,  3629,
    3630,  3637,  3650,  3651,  3658,  3662,  3666,  3670,  3674,  3678,
    3682,  3686,  3690,  3694,  3701,  3705,  3709,  3715,  3716,  3723,
    3724,  3736,  3747,  3748,  3753,  3754,  3769,  3780,  3791,  3792,
    3796,  3807,  3814,  3816,  3818,  3823,  3825,  3836,  3846,  3858,
    3859,  3860,  3864,  3886,  3894,  3904,  3917,  3918,  3921,  3922,
    3932,  3941,  3949,  3957,  3966,  3974,  3982,  3991,  3999,  4008,
    4017,  4026,  4035,  4043,  4051,  4061,  4062,  4072,  4081,  4089,
    4098,  4106,  4114,  4130,  4139,  4147,  4155,  4163,  4172,  4181,
    4190,  4198,  4206,  4224,  4223,  4242,  4243,  4244,  4249,  4255,
    4264,  4265,  4266,  4267,  4268,  4272,  4273,  4277,  4278,  4279,
    4280,  4284,  4285,  4286,  4291,  4311,  4319,  4328,  4334,  4355,
    4362,  4369,  4376,  4383,  4390,  4397,  4405,  4413,  4421,  4429,
    4437,  4444,  4451,  4460,  4461,  4462,  4466,  4469,  4472,  4479,
    4481,  4483,  4488,  4490,  4501,  4512,  4532,  4548,  4558,  4559,
    4563,  4567,  4571,  4575,  4579,  4583,  4587,  4591,  4595,  4599,
    4603,  4607,  4617,  4618,  4629,  4639,  4647,  4659,  4660,  4664,
    4678,  4694,  4706,  4715,  4723,  4731,  4740,  4751,  4752,  4766,
    4789,  4796,  4803,  4820,  4832,  4844,  4856,  4868,  4883,  4884,
    4888,  4889,  4892,  4893,  4896,  4897,  4901,  4902,  4913,  4924,
    4925,  4926,  4927,  4928,  4929,  4933,  4934,  4944,  4954,  4955,
    4958,  4959,  4964,  4965,  4966,  4967,  4977,  4985,  5001,  5002,
    5012,  5018,  5034,  5042,  5049,  5056,  5063,  5070,  5080,  5082,
    5087,  5104,  5115,  5116,  5119,  5130,  5131,  5134,  5135,  5136,
    5137,  5138,  5139,  5140,  5141,  5144,  5145,  5156,  5177,  5189,
    5190,  5191,  5192,  5193,  5196,  5197,  5198,  5246,  5247,  5251,
    5252,  5262,  5263,  5269,  5275,  5284,  5285,  5312,  5328,  5332,
    5336,  5343,  5344,  5352,  5357,  5362,  5367,  5372,  5377,  5382,
    5387,  5394,  5395,  5398,  5399,  5400,  5407,  5408,  5409,  5410,
    5414,  5415,  5419,  5423,  5424,  5427,  5434,  5441,  5448,  5459,
    5461,  5463,  5465,  5467,  5478,  5480,  5484,  5485,  5495,  5499,
    5500,  5504,  5505,  5509,  5517,  5525,  5529,  5530,  5534,  5535,
    5547,  5548,  5552,  5553,  5563,  5567,  5572,  5579,  5587,  5594,
    5604,  5614,  5633,  5640,  5644,  5670,  5674,  5686,  5700,  5713,
    5727,  5738,  5753,  5759,  5764,  5770,  5777,  5778,  5779,  5780,
    5784,  5785,  5797,  5798,  5803,  5810,  5817,  5824,  5834,  5839,
    5840,  5845,  5849,  5855,  5876,  5881,  5887,  5893,  5903,  5905,
    5908,  5920,  5921,  5922,  5923,  5924,  5925,  5931,  5949,  5967,
    5968,  5969,  5970,  5971,  5975,  5986,  5990,  5994,  5998,  6002,
    6006,  6010,  6014,  6019,  6024,  6029,  6035,  6051,  6057,  6072,
    6083,  6090,  6105,  6116,  6127,  6131,  6139,  6143,  6151,  6172,
    6193,  6197,  6203,  6207,  6220,  6255,  6278,  6280,  6282,  6284,
    6286,  6288,  6293,  6294,  6298,  6299,  6303,  6329,  6340,  6362,
    6372,  6376,  6377,  6378,  6382,  6383,  6384,  6385,  6386,  6387,
    6388,  6390,  6392,  6395,  6398,  6400,  6403,  6405,  6431,  6432,
    6434,  6452,  6454,  6456,  6458,  6460,  6462,  6464,  6466,  6468,
    6470,  6472,  6475,  6477,  6479,  6482,  6484,  6486,  6489,  6491,
    6500,  6502,  6511,  6513,  6522,  6524,  6534,  6545,  6554,  6565,
    6584,  6596,  6608,  6620,  6632,  6636,  6643,  6650,  6657,  6664,
    6671,  6678,  6682,  6686,  6690,  6696,  6702,  6712,  6722,  6752,
    6784,  6796,  6803,  6829,  6831,  6833,  6835,  6837,  6839,  6841,
    6843,  6845,  6847,  6849,  6851,  6853,  6855,  6857,  6859,  6861,
    6865,  6869,  6883,  6884,  6885,  6899,  6911,  6913,  6915,  6924,
    6933,  6942,  6944,  6961,  6970,  6979,  6992,  7001,  7026,  7054,
    7071,  7101,  7114,  7145,  7162,  7192,  7209,  7240,  7249,  7258,
    7267,  7276,  7278,  7287,  7301,  7311,  7323,  7339,  7351,  7360,
    7369,  7378,  7392,  7401,  7405,  7411,  7418,  7437,  7438,  7439,
    7442,  7443,  7444,  7447,  7448,  7451,  7452,  7453,  7454,  7455,
    7456,  7457,  7458,  7459,  7462,  7464,  7469,  7471,  7476,  7478,
    7480,  7482,  7484,  7486,  7498,  7502,  7509,  7516,  7519,  7523,
    7529,  7531,  7535,  7541,  7554,  7555,  7556,  7557,  7558,  7559,
    7560,  7561,  7570,  7574,  7581,  7588,  7589,  7605,  7609,  7614,
    7618,  7633,  7638,  7642,  7645,  7648,  7649,  7650,  7653,  7660,
    7670,  7683,  7684,  7688,  7698,  7699,  7702,  7703,  7711,  7715,
    7722,  7726,  7730,  7737,  7747,  7748,  7752,  7753,  7756,  7757,
    7768,  7769,  7773,  7780,  7787,  7800,  7801,  7805,  7812,  7823,
    7824,  7828,  7835,  7852,  7853,  7857,  7858,  7869,  7876,  7902,
    7904,  7909,  7912,  7915,  7917,  7919,  7921,  7931,  7933,  7941,
    7948,  7955,  7962,  7969,  7981,  7989,  8000,  8023,  8027,  8031,
    8039,  8040,  8041,  8043,  8044,  8060,  8061,  8062,  8067,  8068,
    8074,  8075,  8076,  8082,  8083,  8084,  8085,  8086,  8102,  8103,
    8104,  8105,  8106,  8107,  8108,  8109,  8110,  8111,  8112,  8113,
    8114,  8115,  8116,  8117,  8118,  8119,  8120,  8121,  8122,  8123,
    8124,  8125,  8126,  8127,  8128,  8129,  8130,  8131,  8132,  8133,
    8134,  8135,  8136,  8137,  8138,  8139,  8140,  8141,  8142,  8143,
    8144,  8145,  8146,  8147,  8148,  8149,  8150,  8151,  8152,  8153,
    8154,  8155,  8156,  8157,  8158,  8159,  8160,  8161,  8162,  8163,
    8164,  8165,  8166,  8167,  8168,  8169,  8170,  8171,  8172,  8173,
    8174,  8175,  8176,  8177,  8178,  8179,  8180,  8181,  8182,  8183,
    8184,  8185,  8186,  8187,  8188,  8189,  8190,  8191,  8192,  8193,
    8194,  8195,  8196,  8197,  8198,  8199,  8200,  8201,  8202,  8203,
    8204,  8205,  8206,  8207,  8208,  8209,  8210,  8211,  8212,  8213,
    8214,  8215,  8216,  8217,  8218,  8219,  8220,  8221,  8222,  8223,
    8224,  8225,  8226,  8227,  8228,  8229,  8230,  8231,  8232,  8233,
    8234,  8235,  8236,  8237,  8238,  8239,  8240,  8241,  8242,  8243,
    8244,  8245,  8246,  8247,  8248,  8249,  8250,  8251,  8252,  8253,
    8254,  8255,  8256,  8257,  8258,  8259,  8260,  8261,  8262,  8263,
    8264,  8265,  8266,  8267,  8268,  8269,  8270,  8271,  8272,  8273,
    8274,  8275,  8276,  8277,  8278,  8279,  8280,  8281,  8282,  8283,
    8284,  8285,  8286,  8287,  8288,  8289,  8290,  8291,  8292,  8293,
    8294,  8295,  8296,  8297,  8298,  8299,  8300,  8301,  8302,  8303,
    8304,  8305,  8319,  8320,  8321,  8322,  8323,  8324,  8325,  8326,
    8327,  8328,  8329,  8330,  8331,  8332,  8333,  8334,  8335,  8336,
    8337,  8338,  8339,  8340,  8341,  8342,  8343,  8344,  8345,  8346,
    8347,  8348,  8349,  8350,  8351,  8352,  8353,  8354,  8355,  8369,
    8370,  8371,  8372,  8373,  8374,  8375,  8376,  8377,  8378,  8379,
    8380,  8381,  8382,  8383,  8384,  8385,  8386,  8387,  8388,  8398,
    8399,  8400,  8401,  8402,  8403,  8404,  8405,  8406,  8407,  8408,
    8409,  8410,  8411,  8412,  8413,  8414,  8415,  8416,  8417,  8418,
    8419,  8420,  8421,  8422,  8423,  8424,  8425,  8426,  8427,  8428,
    8429,  8430,  8431,  8432,  8433,  8434,  8435,  8436,  8437,  8438,
    8439,  8440,  8441,  8442,  8443,  8444,  8445,  8446,  8447,  8448,
    8449,  8450,  8451,  8452,  8453,  8454,  8455,  8456,  8457,  8458,
    8459,  8460,  8461,  8462,  8463,  8464,  8465,  8466,  8467,  8468,
    8469,  8470,  8475,  8484
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ABORT_P", "ABSOLUTE_P", "ACCESS", 
  "ACTION", "ADD", "ADMIN", "AFTER", "AGGREGATE", "ALL", "ALSO", "ALTER", 
  "ANALYSE", "ANALYZE", "AND", "ANY", "ARRAY", "AS", "ASC", "ASSERTION", 
  "ASSIGNMENT", "ASYMMETRIC", "AT", "AUTHORIZATION", "BACKWARD", "BEFORE", 
  "BEGIN_P", "BETWEEN", "BIGINT", "BINARY", "BIT", "BOOLEAN_P", "BOTH", 
  "BY", "CACHE", "CALLED", "CASCADE", "CASE", "CAST", "CHAIN", "CHAR_P", 
  "CHARACTER", "CHARACTERISTICS", "CHECK", "CHECKPOINT", "CLASS", "CLOSE", 
  "CLUSTER", "COALESCE", "COLLATE", "COLUMN", "COMMENT", "COMMIT", 
  "COMMITTED", "CONNECTION", "CONSTRAINT", "CONSTRAINTS", "CONVERSION_P", 
  "CONVERT", "COPY", "CREATE", "CREATEDB", "CREATEROLE", "CREATEUSER", 
  "CROSS", "CSV", "CURRENT_DATE", "CURRENT_ROLE", "CURRENT_TIME", 
  "CURRENT_TIMESTAMP", "CURRENT_USER", "CURSOR", "CYCLE", "DATABASE", 
  "DAY_P", "DEALLOCATE", "DEC", "DECIMAL_P", "DECLARE", "DEFAULT", 
  "DEFAULTS", "DEFERRABLE", "DEFERRED", "DEFINER", "DELETE_P", 
  "DELIMITER", "DELIMITERS", "DESC", "DISABLE_P", "DISTINCT", "DO", 
  "DOMAIN_P", "DOUBLE_P", "DROP", "EACH", "ELSE", "ENABLE_P", "ENCODING", 
  "ENCRYPTED", "END_P", "ESCAPE", "EXCEPT", "EXCLUDING", "EXCLUSIVE", 
  "EXECUTE", "EXISTS", "EXPLAIN", "EXTERNAL", "EXTRACT", "FALSE_P", 
  "FETCH", "FIRST_P", "FLOAT_P", "FOR", "FORCE", "FOREIGN", "FORWARD", 
  "FREEZE", "FROM", "FULL", "FUNCTION", "GLOBAL", "GRANT", "GRANTED", 
  "GREATEST", "GROUP_P", "HANDLER", "HAVING", "HEADER", "HOLD", "HOUR_P", 
  "ILIKE", "IMMEDIATE", "IMMUTABLE", "IMPLICIT_P", "IN_P", "INCLUDING", 
  "INCREMENT", "INDEX", "INHERIT", "INHERITS", "INITIALLY", "INNER_P", 
  "INOUT", "INPUT_P", "INSENSITIVE", "INSERT", "INSTEAD", "INT_P", 
  "INTEGER", "INTERSECT", "INTERVAL", "INTO", "INVOKER", "IS", "ISNULL", 
  "ISOLATION", "JOIN", "KEY", "LANCOMPILER", "LANGUAGE", "LARGE_P", 
  "LAST_P", "LEADING", "LEAST", "LEFT", "LEVEL", "LIKE", "LIMIT", 
  "LISTEN", "LOAD", "LOCAL", "LOCALTIME", "LOCALTIMESTAMP", "LOCATION", 
  "LOCK_P", "LOGIN_P", "MATCH", "MAXVALUE", "MINUTE_P", "MINVALUE", 
  "MODE", "MONTH_P", "MOVE", "NAMES", "NATIONAL", "NATURAL", "NCHAR", 
  "NEW", "NEXT", "NO", "NOCREATEDB", "NOCREATEROLE", "NOCREATEUSER", 
  "NOINHERIT", "NOLOGIN_P", "NONE", "NOSUPERUSER", "NOT", "NOTHING", 
  "NOTIFY", "NOTNULL", "NOWAIT", "NULL_P", "NULLIF", "NUMERIC", 
  "OBJECT_P", "OF", "OFF", "OFFSET", "OIDS", "OLD", "ON", "ONLY", 
  "OPERATOR", "OPTION", "OR", "ORDER", "OUT_P", "OUTER_P", "OVERLAPS", 
  "OVERLAY", "OWNER", "PARTIAL", "PASSWORD", "PLACING", "POSITION", 
  "PRECISION", "PRESERVE", "PREPARE", "PREPARED", "PRIMARY", "PRIOR", 
  "PRIVILEGES", "PROCEDURAL", "PROCEDURE", "QUOTE", "READ", "REAL", 
  "RECHECK", "REFERENCES", "REINDEX", "RELATIVE_P", "RELEASE", "RENAME", 
  "REPEATABLE", "REPLACE", "RESET", "RESTART", "RESTRICT", "RETURNS", 
  "REVOKE", "RIGHT", "ROLE", "ROLLBACK", "ROW", "ROWS", "RULE", 
  "SAVEPOINT", "SCHEMA", "SCROLL", "SECOND_P", "SECURITY", "SELECT", 
  "SEQUENCE", "SERIALIZABLE", "SESSION", "SESSION_USER", "SET", "SETOF", 
  "SHARE", "SHOW", "SIMILAR", "SIMPLE", "SMALLINT", "SOME", "STABLE", 
  "START", "STATEMENT", "STATISTICS", "STDIN", "STDOUT", "STORAGE", 
  "STRICT_P", "SUBSTRING", "SUPERUSER_P", "SYMMETRIC", "SYSID", 
  "SYSTEM_P", "TABLE", "TABLESPACE", "TEMP", "TEMPLATE", "TEMPORARY", 
  "THEN", "TIME", "TIMESTAMP", "TO", "TOAST", "TRAILING", "TRANSACTION", 
  "TREAT", "TRIGGER", "TRIM", "TRUE_P", "TRUNCATE", "TRUSTED", "TYPE_P", 
  "UNCOMMITTED", "UNENCRYPTED", "UNION", "UNIQUE", "UNKNOWN", "UNLISTEN", 
  "UNTIL", "UPDATE", "USER", "USING", "VACUUM", "VALID", "VALIDATOR", 
  "VALUES", "VARCHAR", "VARYING", "VERBOSE", "VIEW", "VOLATILE", "WHEN", 
  "WHERE", "WITH", "WITHOUT", "WORK", "WRITE", "YEAR_P", "ZONE", 
  "UNIONJOIN", "IDENT", "FCONST", "SCONST", "BCONST", "XCONST", "Op", 
  "ICONST", "PARAM", "'='", "'<'", "'>'", "POSTFIXOP", "'+'", "'-'", 
  "'*'", "'/'", "'%'", "'^'", "UMINUS", "'['", "']'", "'('", "')'", 
  "TYPECAST", "'.'", "';'", "','", "':'", "$accept", "stmtblock", 
  "stmtmulti", "stmt", "CreateRoleStmt", "opt_with", "OptRoleList", 
  "OptRoleElem", "CreateUserStmt", "AlterRoleStmt", "AlterRoleSetStmt", 
  "AlterUserStmt", "AlterUserSetStmt", "DropRoleStmt", "DropUserStmt", 
  "CreateGroupStmt", "AlterGroupStmt", "add_drop", "DropGroupStmt", 
  "CreateSchemaStmt", "OptSchemaName", "OptSchemaEltList", "schema_stmt", 
  "VariableSetStmt", "set_rest", "var_name", "var_list_or_default", 
  "var_list", "var_value", "iso_level", "opt_boolean", "zone_value", 
  "opt_encoding", "ColId_or_Sconst", "VariableShowStmt", 
  "VariableResetStmt", "ConstraintsSetStmt", "constraints_set_list", 
  "constraints_set_mode", "CheckPointStmt", "AlterTableStmt", 
  "alter_table_cmds", "alter_table_cmd", "alter_rel_cmds", 
  "alter_rel_cmd", "alter_column_default", "opt_drop_behavior", 
  "alter_using", "ClosePortalStmt", "CopyStmt", "copy_from", 
  "copy_file_name", "copy_opt_list", "copy_opt_item", "opt_binary", 
  "opt_oids", "copy_delimiter", "opt_using", "CreateStmt", "OptTemp", 
  "OptTableElementList", "TableElementList", "TableElement", "columnDef", 
  "ColQualList", "ColConstraint", "ColConstraintElem", "ConstraintAttr", 
  "TableLikeClause", "like_including_defaults", "TableConstraint", 
  "ConstraintElem", "opt_column_list", "columnList", "columnElem", 
  "key_match", "key_actions", "key_update", "key_delete", "key_action", 
  "OptInherit", "OptWithOids", "OnCommitOption", "OptTableSpace", 
  "OptConsTableSpace", "CreateAsStmt", "WithOidsAs", "OptCreateAs", 
  "CreateAsList", "CreateAsElement", "CreateSeqStmt", "AlterSeqStmt", 
  "OptSeqList", "OptSeqElem", "opt_by", "NumericOnly", "FloatOnly", 
  "IntegerOnly", "CreatePLangStmt", "opt_trusted", "handler_name", 
  "opt_validator", "opt_lancompiler", "DropPLangStmt", "opt_procedural", 
  "CreateTableSpaceStmt", "OptTableSpaceOwner", "DropTableSpaceStmt", 
  "CreateTrigStmt", "TriggerActionTime", "TriggerEvents", 
  "TriggerOneEvent", "TriggerForSpec", "TriggerForOpt", "TriggerForType", 
  "TriggerFuncArgs", "TriggerFuncArg", "OptConstrFromTable", 
  "ConstraintAttributeSpec", "ConstraintDeferrabilitySpec", 
  "ConstraintTimeSpec", "DropTrigStmt", "CreateAssertStmt", 
  "DropAssertStmt", "DefineStmt", "definition", "def_list", "def_elem", 
  "def_arg", "CreateOpClassStmt", "opclass_item_list", "opclass_item", 
  "opt_default", "opt_recheck", "DropOpClassStmt", "DropStmt", 
  "drop_type", "any_name_list", "any_name", "attrs", "TruncateStmt", 
  "CommentStmt", "comment_type", "comment_text", "FetchStmt", 
  "fetch_direction", "from_in", "GrantStmt", "RevokeStmt", "privileges", 
  "privilege_list", "privilege", "privilege_target", "grantee_list", 
  "grantee", "opt_grant_grant_option", "function_with_argtypes_list", 
  "function_with_argtypes", "GrantRoleStmt", "RevokeRoleStmt", 
  "opt_grant_admin_option", "opt_granted_by", "IndexStmt", 
  "index_opt_unique", "access_method_clause", "index_params", 
  "index_elem", "opt_class", "CreateFunctionStmt", "opt_or_replace", 
  "func_args", "func_args_list", "func_arg", "arg_class", "param_name", 
  "func_return", "func_type", "createfunc_opt_list", 
  "common_func_opt_item", "createfunc_opt_item", "func_as", 
  "opt_definition", "AlterFunctionStmt", "alterfunc_opt_list", 
  "opt_restrict", "RemoveFuncStmt", "RemoveAggrStmt", "aggr_argtype", 
  "RemoveOperStmt", "oper_argtypes", "any_operator", "CreateCastStmt", 
  "cast_context", "DropCastStmt", "ReindexStmt", "reindex_type", 
  "opt_force", "RenameStmt", "opt_column", "AlterObjectSchemaStmt", 
  "AlterOwnerStmt", "RuleStmt", "@1", "RuleActionList", "RuleActionMulti", 
  "RuleActionStmt", "RuleActionStmtOrEmpty", "event", "opt_instead", 
  "DropRuleStmt", "NotifyStmt", "ListenStmt", "UnlistenStmt", 
  "TransactionStmt", "opt_transaction", "transaction_mode_item", 
  "transaction_mode_list", "transaction_mode_list_or_empty", "ViewStmt", 
  "LoadStmt", "CreatedbStmt", "createdb_opt_list", "createdb_opt_item", 
  "opt_equal", "AlterDatabaseStmt", "AlterDatabaseSetStmt", 
  "alterdb_opt_list", "alterdb_opt_item", "DropdbStmt", 
  "CreateDomainStmt", "AlterDomainStmt", "opt_as", "CreateConversionStmt", 
  "ClusterStmt", "VacuumStmt", "AnalyzeStmt", "analyze_keyword", 
  "opt_verbose", "opt_full", "opt_freeze", "opt_name_list", "ExplainStmt", 
  "ExplainableStmt", "opt_analyze", "PrepareStmt", "prep_type_clause", 
  "prep_type_list", "PreparableStmt", "ExecuteStmt", 
  "execute_param_clause", "DeallocateStmt", "InsertStmt", "insert_rest", 
  "insert_column_list", "insert_column_item", "DeleteStmt", 
  "using_clause", "LockStmt", "opt_lock", "lock_type", "opt_nowait", 
  "UpdateStmt", "DeclareCursorStmt", "cursor_options", "opt_hold", 
  "SelectStmt", "select_with_parens", "select_no_parens", "select_clause", 
  "simple_select", "into_clause", "OptTempTableName", "opt_table", 
  "opt_all", "opt_distinct", "opt_sort_clause", "sort_clause", 
  "sortby_list", "sortby", "select_limit", "opt_select_limit", 
  "select_limit_value", "select_offset_value", "group_clause", 
  "having_clause", "for_locking_clause", "opt_for_locking_clause", 
  "locked_rels_list", "from_clause", "from_list", "table_ref", 
  "joined_table", "alias_clause", "join_type", "join_outer", "join_qual", 
  "relation_expr", "func_table", "where_clause", "TableFuncElementList", 
  "TableFuncElement", "Typename", "opt_array_bounds", "SimpleTypename", 
  "ConstTypename", "GenericType", "Numeric", "opt_float", "opt_numeric", 
  "opt_decimal", "Bit", "ConstBit", "BitWithLength", "BitWithoutLength", 
  "Character", "ConstCharacter", "CharacterWithLength", 
  "CharacterWithoutLength", "character", "opt_varying", "opt_charset", 
  "ConstDatetime", "ConstInterval", "opt_timezone", "opt_interval", 
  "a_expr", "b_expr", "c_expr", "func_expr", "row", "sub_type", "all_Op", 
  "MathOp", "qual_Op", "qual_all_Op", "subquery_Op", "expr_list", 
  "extract_list", "type_list", "array_expr_list", "array_expr", 
  "extract_arg", "overlay_list", "overlay_placing", "position_list", 
  "substr_list", "substr_from", "substr_for", "trim_list", "in_expr", 
  "case_expr", "when_clause_list", "when_clause", "case_default", 
  "case_arg", "columnref", "indirection_el", "indirection", 
  "opt_indirection", "opt_asymmetric", "target_list", "target_el", 
  "update_target_list", "update_target_el", "insert_target_list", 
  "insert_target_el", "relation_name", "qualified_name_list", 
  "qualified_name", "name_list", "name", "database_name", "access_method", 
  "attr_name", "index_name", "file_name", "func_name", "AexprConst", 
  "Iconst", "Sconst", "RoleId", "SignedIconst", "ColId", "type_name", 
  "function_name", "ColLabel", "unreserved_keyword", "col_name_keyword", 
  "func_name_keyword", "reserved_keyword", "SpecialRuleRelation", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     585,   586,   587,   588,   589,   590,   591,   592,   593,   594,
     595,   596,   597,   598,   599,   600,    61,    60,    62,   601,
      43,    45,    42,    47,    37,    94,   602,    91,    93,    40,
      41,   603,    46,    59,    44,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   366,   367,   368,   368,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   369,
     369,   369,   369,   369,   369,   369,   369,   369,   369,   370,
     371,   371,   372,   372,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   374,   375,   376,
     376,   377,   378,   378,   379,   380,   381,   382,   383,   383,
     384,   385,   385,   386,   386,   387,   387,   388,   388,   388,
     388,   388,   388,   389,   389,   389,   390,   390,   390,   390,
     390,   390,   390,   390,   390,   391,   391,   392,   392,   393,
     393,   394,   394,   394,   395,   395,   395,   395,   396,   396,
     396,   396,   397,   397,   397,   397,   397,   397,   397,   398,
     398,   398,   399,   399,   400,   400,   400,   400,   400,   401,
     401,   401,   401,   401,   402,   403,   403,   404,   404,   405,
     406,   406,   407,   407,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   408,   408,   408,   408,   408,
     408,   408,   408,   408,   408,   409,   409,   410,   410,   411,
     411,   412,   412,   412,   413,   413,   414,   415,   416,   416,
     417,   417,   417,   418,   418,   419,   419,   419,   419,   419,
     419,   419,   419,   419,   419,   420,   420,   421,   421,   422,
     422,   423,   423,   424,   424,   425,   425,   425,   425,   425,
     425,   425,   426,   426,   427,   427,   428,   428,   428,   429,
     430,   430,   431,   431,   431,   432,   432,   432,   432,   432,
     432,   432,   433,   433,   433,   433,   434,   435,   435,   435,
     436,   436,   437,   437,   437,   437,   438,   438,   439,   439,
     440,   441,   441,   441,   441,   442,   442,   442,   442,   442,
     443,   444,   445,   445,   445,   445,   445,   446,   446,   447,
     447,   447,   448,   448,   448,   448,   449,   449,   450,   450,
     451,   452,   452,   452,   453,   453,   454,   454,   455,   456,
     457,   458,   458,   459,   459,   459,   459,   459,   459,   459,
     459,   459,   459,   460,   460,   461,   461,   462,   462,   463,
     464,   464,   465,   465,   466,   466,   467,   467,   468,   468,
     469,   470,   470,   471,   472,   472,   473,   474,   474,   475,
     475,   476,   476,   476,   477,   477,   477,   478,   478,   479,
     479,   480,   480,   481,   481,   481,   482,   482,   482,   482,
     482,   482,   483,   483,   484,   484,   484,   484,   484,   485,
     485,   486,   486,   487,   488,   489,   490,   490,   490,   490,
     491,   492,   492,   493,   493,   494,   494,   494,   494,   495,
     496,   496,   497,   497,   497,   497,   498,   498,   499,   499,
     500,   501,   502,   502,   502,   502,   502,   502,   502,   502,
     503,   503,   504,   504,   505,   505,   506,   507,   507,   507,
     507,   507,   507,   507,   507,   507,   507,   507,   507,   508,
     508,   508,   508,   508,   508,   508,   508,   508,   508,   509,
     509,   510,   510,   510,   510,   511,   511,   511,   511,   511,
     511,   511,   511,   511,   511,   511,   511,   511,   511,   511,
     512,   512,   513,   514,   514,   515,   515,   515,   516,   516,
     517,   517,   517,   517,   518,   518,   518,   518,   518,   518,
     518,   519,   519,   520,   520,   521,   521,   522,   522,   523,
     524,   525,   525,   526,   526,   527,   527,   528,   529,   529,
     530,   530,   531,   531,   532,   532,   532,   533,   533,   533,
     534,   534,   535,   535,   536,   536,   537,   537,   538,   538,
     538,   538,   538,   539,   539,   539,   539,   540,   541,   542,
     542,   542,   543,   543,   544,   544,   544,   544,   544,   544,
     544,   544,   544,   544,   545,   545,   545,   546,   546,   547,
     547,   548,   549,   549,   550,   550,   551,   552,   553,   553,
     554,   555,   555,   555,   555,   556,   556,   557,   557,   558,
     558,   558,   559,   560,   560,   560,   561,   561,   562,   562,
     563,   563,   563,   563,   563,   563,   563,   563,   563,   563,
     563,   563,   563,   563,   563,   564,   564,   565,   565,   565,
     565,   565,   565,   566,   566,   566,   566,   566,   566,   566,
     566,   566,   566,   568,   567,   569,   569,   569,   570,   570,
     571,   571,   571,   571,   571,   572,   572,   573,   573,   573,
     573,   574,   574,   574,   575,   576,   577,   578,   578,   579,
     579,   579,   579,   579,   579,   579,   579,   579,   579,   579,
     579,   579,   579,   580,   580,   580,   581,   581,   581,   582,
     582,   582,   583,   583,   584,   584,   585,   586,   587,   587,
     588,   588,   588,   588,   588,   588,   588,   588,   588,   588,
     588,   588,   589,   589,   590,   591,   591,   592,   592,   593,
     594,   595,   596,   596,   596,   596,   596,   597,   597,   598,
     599,   599,   599,   600,   600,   600,   601,   601,   602,   602,
     603,   603,   604,   604,   605,   605,   606,   606,   607,   608,
     608,   608,   608,   608,   608,   609,   609,   610,   611,   611,
     612,   612,   613,   613,   613,   613,   614,   614,   615,   615,
     616,   616,   617,   618,   618,   618,   618,   618,   619,   619,
     620,   621,   622,   622,   623,   624,   624,   625,   625,   625,
     625,   625,   625,   625,   625,   626,   626,   627,   628,   629,
     629,   629,   629,   629,   630,   630,   630,   631,   631,   632,
     632,   633,   633,   633,   633,   634,   634,   635,   635,   635,
     635,   636,   636,   637,   637,   637,   637,   637,   637,   637,
     637,   638,   638,   639,   639,   639,   640,   640,   640,   640,
     641,   641,   642,   643,   643,   644,   644,   644,   644,   645,
     645,   645,   645,   645,   646,   646,   647,   647,   648,   649,
     649,   650,   650,   651,   651,   651,   652,   652,   653,   653,
     654,   654,   655,   655,   656,   656,   656,   656,   656,   656,
     656,   656,   656,   656,   656,   657,   657,   657,   657,   657,
     657,   657,   658,   658,   658,   658,   659,   659,   659,   659,
     660,   660,   661,   661,   662,   662,   662,   662,   663,   664,
     664,   665,   665,   666,   667,   667,   667,   667,   668,   668,
     668,   669,   669,   669,   669,   669,   669,   669,   669,   670,
     670,   670,   670,   670,   671,   672,   672,   672,   672,   672,
     672,   672,   672,   672,   672,   672,   673,   673,   674,   674,
     674,   675,   675,   675,   676,   676,   677,   677,   678,   679,
     680,   680,   681,   681,   682,   683,   684,   684,   684,   684,
     684,   684,   685,   685,   686,   686,   687,   687,   687,   687,
     688,   689,   689,   689,   690,   690,   690,   690,   690,   690,
     690,   690,   690,   690,   690,   690,   690,   690,   691,   691,
     691,   691,   691,   691,   691,   691,   691,   691,   691,   691,
     691,   691,   691,   691,   691,   691,   691,   691,   691,   691,
     691,   691,   691,   691,   691,   691,   691,   691,   691,   691,
     691,   691,   691,   691,   691,   691,   691,   691,   691,   691,
     691,   691,   691,   691,   691,   691,   691,   691,   691,   691,
     691,   691,   691,   692,   692,   692,   692,   692,   692,   692,
     692,   692,   692,   692,   692,   692,   692,   692,   692,   692,
     692,   692,   693,   693,   693,   693,   693,   693,   693,   693,
     693,   693,   693,   694,   694,   694,   694,   694,   694,   694,
     694,   694,   694,   694,   694,   694,   694,   694,   694,   694,
     694,   694,   694,   694,   694,   694,   694,   694,   694,   694,
     694,   694,   694,   694,   694,   694,   694,   695,   695,   695,
     696,   696,   696,   697,   697,   698,   698,   698,   698,   698,
     698,   698,   698,   698,   699,   699,   700,   700,   701,   701,
     701,   701,   701,   701,   702,   702,   703,   703,   704,   704,
     705,   705,   706,   706,   707,   707,   707,   707,   707,   707,
     707,   707,   708,   708,   709,   710,   710,   711,   711,   711,
     711,   711,   711,   712,   713,   714,   714,   714,   715,   715,
     716,   717,   717,   718,   719,   719,   720,   720,   721,   721,
     722,   722,   722,   722,   723,   723,   724,   724,   725,   725,
     726,   726,   727,   727,   727,   728,   728,   729,   729,   730,
     730,   731,   731,   732,   732,   733,   733,   734,   734,   735,
     735,   736,   737,   738,   739,   740,   741,   742,   742,   743,
     743,   743,   743,   743,   743,   743,   743,   743,   743,   743,
     744,   745,   746,   747,   747,   748,   748,   748,   749,   749,
     750,   750,   750,   751,   751,   751,   751,   751,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   752,   752,   752,   752,   752,   752,   752,   752,
     752,   752,   753,   753,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   753,
     753,   753,   753,   753,   753,   753,   753,   753,   753,   754,
     754,   754,   754,   754,   754,   754,   754,   754,   754,   754,
     754,   754,   754,   754,   754,   754,   754,   754,   754,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   755,   755,   755,   755,   755,   755,   755,   755,
     755,   755,   756,   756
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     5,
       1,     0,     2,     0,     2,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     2,     2,     2,     2,     3,     3,     5,     5,     5,
       4,     5,     5,     4,     3,     3,     5,     6,     1,     1,
       3,     6,     4,     1,     0,     2,     0,     1,     1,     1,
       1,     1,     1,     2,     3,     3,     3,     3,     3,     2,
       5,     2,     2,     3,     3,     1,     3,     1,     1,     1,
       3,     1,     1,     1,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     3,     6,     1,     1,     1,     1,
       1,     0,     1,     1,     2,     3,     4,     3,     2,     2,
       3,     4,     3,     2,     4,     1,     1,     1,     1,     1,
       4,     4,     1,     3,     3,     4,     6,     6,     6,     6,
       4,     6,     2,     4,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     1,     1,     3,     3,     3,     3,
       2,     1,     1,     0,     2,     0,     2,    10,     1,     1,
       1,     1,     1,     2,     0,     1,     1,     3,     3,     1,
       1,     3,     3,     3,     4,     1,     0,     2,     0,     3,
       0,     1,     0,    11,    12,     1,     1,     2,     2,     2,
       2,     0,     1,     0,     1,     3,     1,     1,     1,     3,
       2,     0,     3,     1,     1,     2,     1,     2,     3,     4,
       2,     5,     1,     2,     2,     2,     3,     2,     2,     0,
       3,     1,     4,     5,     6,    11,     3,     0,     1,     3,
       1,     2,     2,     2,     0,     1,     1,     2,     2,     0,
       3,     3,     2,     1,     1,     2,     2,     4,     0,     2,
       2,     0,     3,     4,     4,     0,     2,     0,     4,     0,
       7,     3,     3,     1,     3,     0,     1,     3,     1,     5,
       4,     2,     0,     2,     1,     2,     3,     2,     2,     2,
       2,     3,     3,     1,     0,     1,     1,     1,     2,     1,
       5,     9,     1,     0,     1,     2,     2,     0,     2,     0,
       5,     1,     0,     6,     2,     0,     3,    14,    19,     1,
       1,     1,     3,     5,     1,     1,     1,     3,     0,     1,
       0,     1,     1,     1,     3,     0,     1,     1,     1,     1,
       1,     1,     2,     0,     1,     2,     1,     2,     0,     2,
       1,     2,     2,     6,     8,     4,     4,     4,     4,     7,
       3,     1,     3,     3,     1,     1,     1,     1,     1,    12,
       1,     3,     4,     7,     4,     2,     1,     0,     1,     0,
       7,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     2,     2,     3,     3,     6,     9,     7,
       9,     8,     8,     6,     8,     9,     7,    10,     7,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     2,     4,     2,     0,     1,     1,     1,     1,
       2,     2,     1,     1,     1,     2,     2,     1,     2,     2,
       1,     1,     7,     7,    10,     1,     1,     2,     1,     3,
       1,     1,     1,     1,     1,     2,     2,     2,     2,     2,
       2,     1,     3,     1,     2,     3,     0,     1,     3,     2,
       6,     6,     9,     3,     0,     3,     0,    12,     1,     0,
       2,     0,     1,     3,     2,     2,     4,     1,     2,     0,
       9,     7,     2,     0,     3,     2,     1,     3,     3,     3,
       2,     2,     1,     1,     1,     1,     2,     1,     1,     1,
       4,     5,     1,     2,     4,     5,     1,     1,     1,     1,
       3,     3,     2,     2,     2,     2,     1,     1,     3,     2,
       0,     5,     1,     2,     1,     0,     5,     7,     1,     1,
       7,     1,     3,     3,     3,     1,     3,    11,    10,     2,
       2,     0,     8,     4,     4,     4,     1,     1,     1,     0,
       9,     6,     6,     7,     6,     6,     9,     6,     6,     6,
       8,     8,     6,     6,     6,     1,     0,     9,     6,     7,
       6,     6,     6,     9,     6,     6,     6,     7,     9,     9,
       6,     6,     6,     0,    14,     1,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     1,     1,
       1,     1,     1,     0,     6,     2,     2,     2,     2,     2,
       3,     3,     2,     2,     2,     2,     3,     2,     5,     4,
       3,     3,     3,     1,     1,     0,     3,     2,     2,     1,
       3,     2,     1,     0,     7,     9,     2,     5,     2,     0,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     4,
       3,     3,     1,     0,     5,     5,     4,     2,     0,     4,
       3,     6,     4,     6,     6,     5,     7,     1,     0,    10,
       4,     2,     1,     4,     5,     5,     2,     4,     1,     1,
       1,     0,     1,     0,     1,     0,     3,     0,     4,     1,
       1,     1,     1,     1,     1,     1,     0,     5,     3,     0,
       1,     3,     1,     1,     1,     1,     3,     9,     3,     0,
       2,     3,     4,     4,     2,     1,     7,     4,     1,     3,
       2,     5,     2,     0,     5,     3,     0,     2,     2,     2,
       3,     1,     3,     1,     2,     1,     0,     6,     7,     0,
       3,     2,     2,     2,     0,     2,     2,     1,     1,     3,
       3,     1,     2,     4,     4,     1,     1,     8,     4,     4,
       4,     2,     0,     3,     3,     4,     4,     4,     4,     2,
       1,     1,     0,     1,     1,     0,     1,     5,     1,     0,
       1,     0,     3,     1,     3,     3,     2,     2,     1,     4,
       4,     2,     2,     4,     1,     0,     1,     1,     1,     3,
       0,     2,     0,     4,     4,     3,     1,     0,     2,     0,
       2,     0,     1,     3,     1,     2,     1,     2,     5,     6,
       5,     1,     2,     1,     4,     3,     4,     3,     5,     4,
       5,     4,     5,     2,     4,     1,     2,     2,     2,     1,
       1,     0,     4,     2,     1,     2,     2,     4,     1,     2,
       0,     1,     3,     2,     2,     3,     5,     6,     3,     4,
       0,     1,     1,     1,     1,     1,     2,     5,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     1,     3,     0,     5,     3,
       0,     5,     3,     0,     1,     1,     1,     1,     5,     2,
       1,     1,     1,     1,     5,     2,     2,     2,     1,     3,
       3,     2,     1,     0,     3,     0,     5,     2,     5,     2,
       1,     3,     3,     0,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     0,     1,     3,
       5,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     3,     3,     2,     3,     5,
       4,     6,     3,     5,     4,     6,     4,     6,     5,     7,
       2,     3,     2,     4,     3,     3,     4,     3,     4,     3,
       4,     5,     6,     7,     6,     7,     6,     7,     3,     4,
       4,     6,     2,     1,     3,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     2,     5,
       6,     7,     1,     1,     2,     4,     1,     1,     1,     2,
       2,     2,     1,     3,     4,     5,     5,     4,     1,     1,
       4,     1,     4,     1,     4,     1,     4,     1,     1,     1,
       1,     6,     4,     4,     4,     4,     6,     5,     5,     5,
       4,     6,     4,     6,     4,     4,     4,     4,     3,     5,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     1,     4,     1,     4,
       1,     2,     1,     2,     1,     3,     3,     0,     3,     1,
       1,     3,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     3,     2,     3,     0,     3,     3,     2,
       2,     1,     0,     2,     2,     3,     2,     1,     1,     3,
       5,     1,     2,     4,     2,     0,     1,     0,     1,     2,
       2,     2,     3,     5,     1,     2,     0,     2,     1,     0,
       1,     3,     3,     1,     1,     1,     3,     4,     4,     1,
       3,     1,     1,     1,     1,     1,     3,     1,     2,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       1,     1,     1,     1,     2,     3,     6,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
      88,   705,     0,   759,   758,   705,   199,     0,   752,     0,
     705,   256,   271,     0,     0,     0,   382,   705,     0,   776,
     495,     0,     0,     0,     0,   852,   495,     0,     0,     0,
       0,     0,     0,   705,     0,   859,     0,     0,     0,   852,
       0,     0,   763,     0,     0,     2,     4,    39,    40,    15,
      14,    17,    16,    55,    56,    31,     9,    48,    34,    85,
      86,    84,    23,    19,    13,    20,    24,    36,    25,    35,
      12,    33,    50,    37,    53,    38,    54,    26,    46,    44,
      32,    49,    52,    80,    22,    60,    61,    75,    62,    76,
      63,    30,     8,    72,    71,    73,    27,    47,    70,    74,
      10,    11,    77,    51,    68,    65,    81,    79,    87,    66,
      41,     5,     6,    57,    29,     7,    28,    21,    83,    18,
     761,    59,    69,    58,    42,    64,    45,    67,    82,    43,
      78,   836,   827,   861,   835,   704,   703,   689,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   713,  1278,  1279,  1280,  1281,
    1282,  1283,  1284,  1285,  1286,  1287,  1288,  1289,  1290,  1291,
    1292,  1293,  1482,  1483,  1484,  1294,  1295,  1296,  1297,  1298,
    1485,  1486,  1299,  1300,  1301,  1302,  1303,  1487,  1304,  1305,
    1306,  1307,  1308,  1309,  1488,  1310,  1311,  1312,  1313,  1314,
    1315,  1316,  1317,  1318,  1319,  1489,  1490,  1320,  1321,  1322,
    1323,  1324,  1325,  1326,  1327,  1328,  1329,  1330,  1331,  1332,
    1333,  1334,  1335,  1336,  1337,  1338,  1491,  1339,  1340,  1492,
    1341,  1342,  1493,  1343,  1344,  1345,  1346,  1347,  1494,  1348,
    1349,  1350,  1351,  1352,  1353,  1354,  1355,  1356,  1357,  1358,
    1359,  1495,  1360,  1361,  1362,  1363,  1496,  1497,  1498,  1364,
    1365,  1366,  1367,  1368,  1369,  1370,  1499,  1371,  1372,  1373,
    1374,  1375,  1376,  1377,  1378,  1379,  1380,  1381,  1382,  1383,
    1384,  1385,  1500,  1501,  1386,  1387,  1388,  1389,  1390,  1391,
    1392,  1502,  1393,  1394,  1395,  1396,  1503,  1504,  1397,  1398,
    1399,  1400,  1401,  1505,  1506,  1402,  1403,  1404,  1507,  1508,
    1407,  1405,  1406,  1408,  1409,  1410,  1411,  1412,  1413,  1509,
    1414,  1415,  1416,  1417,  1418,  1419,  1420,  1421,  1422,  1423,
    1424,  1425,  1426,  1427,  1510,  1428,  1429,  1430,  1431,  1432,
    1433,  1434,  1435,  1436,  1437,  1438,  1511,  1439,  1440,  1441,
    1512,  1442,  1443,  1444,  1445,  1446,  1447,  1448,  1452,  1513,
    1449,  1450,  1451,  1453,  1454,  1455,  1456,  1514,  1515,  1457,
    1458,  1516,  1459,  1517,  1460,  1461,  1462,  1463,  1464,  1465,
    1466,  1467,  1468,  1469,  1470,  1471,  1472,  1518,  1473,  1474,
    1475,  1476,  1477,  1478,  1479,  1480,  1481,  1265,   236,  1241,
    1266,  1267,  1613,  1612,  1237,   751,     0,  1234,  1233,   382,
       0,   692,   255,     0,     0,     0,     0,     0,     0,   446,
       0,     0,     0,     0,     0,     0,     0,   134,     0,   266,
     265,     0,   372,     0,   548,     0,     0,   382,     0,     0,
       0,  1405,   790,   819,     0,     0,     0,     0,   458,     0,
     457,     0,     0,   455,     0,   381,     0,     0,   459,   453,
     452,     0,     0,   456,     0,   454,     0,     0,   693,   789,
     775,   761,  1279,   503,   507,   498,   504,   499,   496,   497,
    1416,  1263,     0,     0,   492,   502,   516,   522,   521,   520,
       0,   515,   518,   523,     0,   686,  1234,  1261,   716,  1246,
     851,     0,     0,   494,   685,  1458,   779,     0,   626,     0,
     627,     0,  1430,   697,   193,  1437,  1514,  1458,   189,   155,
    1283,     0,     0,   515,     0,   694,   695,   858,   856,     0,
    1308,  1374,  1385,  1426,  1437,  1514,  1458,   143,     0,   188,
    1437,  1514,  1458,   184,   713,     0,   688,   687,     0,     0,
     924,   762,   765,   836,     0,     1,    88,   760,   756,   855,
     855,     0,   855,     0,   832,  1519,  1520,  1521,  1522,  1523,
    1524,  1525,  1526,  1527,  1528,  1529,  1530,  1531,  1532,  1533,
    1534,  1535,  1536,  1537,  1538,  1265,     0,     0,  1247,  1266,
    1272,     0,   462,    91,  1242,     0,     0,     0,     0,  1262,
       0,     0,  1301,  1143,  1153,  1151,  1152,  1145,  1146,  1147,
    1148,  1149,  1150,     0,   615,  1144,     0,    91,     0,     0,
     352,     0,     0,     0,     0,    91,     0,     0,   709,   712,
     690,     0,     0,  1214,  1238,     0,     0,     0,   479,     0,
     488,   480,   485,     0,   482,     0,     0,     0,   481,   483,
     484,     0,   486,   487,     0,     0,   701,   307,     0,     0,
       0,     0,    91,   748,   270,   269,    91,   268,   267,  1301,
       0,   562,    91,     0,   136,   385,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,   791,     0,   803,
       0,   233,     0,   740,     0,   130,  1239,  1301,     0,   124,
       0,   386,     0,   125,     0,   233,   460,     0,   786,     0,
     500,   509,   508,   506,   505,   501,  1264,   510,   511,     0,
     517,     0,     0,     0,     0,   806,  1235,     0,   700,     0,
       0,   629,   629,   629,   696,   192,   190,     0,     0,     0,
       0,     0,     0,   702,     0,     0,     0,  1482,  1483,  1484,
    1207,     0,  1485,  1486,  1487,  1488,  1108,  1117,  1109,  1111,
    1118,  1489,  1490,  1329,  1491,  1492,  1258,  1493,  1494,  1496,
    1497,  1498,  1499,  1113,  1115,  1500,  1501,     0,  1259,  1503,
    1504,  1400,  1506,  1507,  1509,  1510,  1119,  1512,  1513,  1514,
    1515,  1516,  1517,  1257,     0,  1120,  1518,  1265,  1250,  1252,
    1253,  1154,  1260,  1216,     0,     0,  1224,     0,  1098,     0,
     949,   950,   951,   976,   977,   952,   982,   983,   995,   953,
       0,  1223,  1018,  1097,  1102,     0,  1096,  1092,   842,  1220,
    1208,     0,  1093,  1249,  1251,   954,  1266,   195,     0,   196,
    1437,   144,   180,   151,   179,   183,   152,   182,     0,  1299,
     145,     0,   149,     0,     0,   187,   185,     0,   691,   466,
       0,   926,     0,   925,   764,   761,   830,   829,     3,   767,
     853,   854,     0,     0,     0,     0,     0,     0,     0,   887,
     875,  1248,     0,     0,     0,     0,   463,     0,     0,     0,
      90,   738,   736,     0,     0,     0,     0,   742,     0,     0,
     587,     0,     0,   588,   586,   589,   602,   605,     0,   539,
     128,   129,     0,     0,     0,     0,     0,   201,   225,     0,
       0,     0,     0,     0,     0,    93,   120,     0,     0,     0,
     350,   646,   646,     0,     0,     0,   646,     0,   646,     0,
     200,   202,   224,     0,     0,     0,     0,     0,     0,     0,
      93,   123,     0,   707,   708,     0,   711,     0,  1539,  1540,
    1541,  1542,  1543,  1544,  1545,  1546,  1547,  1548,  1549,  1550,
    1551,  1552,  1553,  1554,  1555,  1556,  1557,  1558,  1559,  1560,
    1561,  1562,  1563,  1564,  1565,  1566,  1567,  1568,  1569,  1570,
    1571,  1572,  1573,  1574,  1575,  1576,  1577,  1578,  1579,  1580,
    1581,  1582,  1583,  1584,  1585,  1586,  1587,  1588,  1589,  1590,
    1591,  1592,  1593,  1594,  1595,  1596,  1597,  1598,  1599,  1600,
    1601,  1602,  1603,  1604,  1605,  1606,  1607,  1608,  1609,  1610,
    1611,  1273,  1211,  1210,  1244,  1274,  1275,  1276,  1277,  1215,
     750,     0,     0,     0,     0,     0,  1301,     0,     0,     0,
       0,     0,     0,   258,     0,   426,     0,   958,   993,   965,
     993,   993,   973,   973,   967,   955,   956,  1000,     0,   993,
     970,   959,     0,   957,  1003,  1003,   988,  1268,     0,   940,
     941,   942,   943,   974,   975,   944,   980,   981,   945,  1017,
     954,  1269,     0,   719,   747,     0,    93,   447,   427,     0,
      93,     0,   132,     0,     0,   390,   389,     0,     0,   428,
      93,   352,   345,   307,     0,     0,     0,  1245,     0,     0,
     822,   824,   823,     0,   821,     0,   930,     0,   231,   232,
     425,     0,   233,     0,     0,     0,     0,     0,   233,     0,
     451,  1164,     0,   271,   768,   774,   770,   772,   771,   773,
     769,   491,  1317,  1345,  1368,  1431,     0,  1453,     0,   524,
     544,   519,     0,     0,     0,   792,   795,     0,     0,   816,
     493,     0,   780,     0,   628,   625,   624,   623,   191,   156,
       0,     0,     0,   546,  1430,   699,     0,     0,  1100,  1101,
     992,   979,  1206,     0,     0,   987,   986,     0,     0,     0,
       0,     0,   963,   962,   961,  1099,  1167,     0,   960,     0,
       0,     0,     0,   993,   993,   991,  1037,     0,     0,   964,
       0,     0,  1186,     0,  1192,     0,     0,     0,   999,     0,
     997,     0,     0,  1072,  1094,  1021,  1022,  1098,  1164,     0,
    1254,     0,     0,   985,     0,  1017,     0,     0,     0,  1219,
    1162,     0,     0,  1050,  1160,     0,  1052,     0,     0,     0,
    1154,  1153,  1151,  1152,  1145,  1146,  1147,  1148,  1149,  1150,
       0,  1158,  1034,     0,     0,  1033,     0,     0,   891,  1209,
       0,   197,   198,   194,     0,   154,   153,     0,   177,   178,
     173,   367,     0,   148,   176,   365,   366,     0,   172,   369,
     158,   169,   171,   170,   168,   146,   157,   159,   161,   162,
     163,   147,   186,     0,   891,  1225,  1216,   753,     0,   757,
     836,   840,   835,   839,   862,   863,   868,   838,     0,   889,
     889,   877,   871,   876,   872,   878,   886,   834,   874,   833,
     609,     0,   608,     0,     0,   464,     0,     0,     0,   735,
     734,     0,     0,     0,     0,     0,   745,   301,     0,   230,
       0,     0,     0,     0,     0,     0,     0,     0,   592,   593,
     604,   603,   601,   573,   575,   574,     0,  1270,   565,     0,
     566,     0,     0,   572,   579,   954,   577,  1271,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   611,   616,     0,   119,   118,     0,     0,     0,     0,
     354,   364,     0,     0,     0,    91,    91,   351,   645,   212,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   122,
     121,     0,     0,   167,   706,   710,  1212,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   308,   310,     0,     0,     0,   431,   434,     0,   940,
       0,     0,   934,  1006,  1007,  1008,  1005,  1009,  1004,     0,
     946,   948,     0,   717,   281,   126,     0,     0,    89,   136,
     271,     0,   135,   137,   139,   140,   141,   138,   142,   384,
       0,   395,   394,   396,     0,   391,     0,   117,   349,     0,
     273,     0,     0,   370,     0,     0,     0,   663,     0,     0,
       0,   820,     0,   901,   802,   892,   903,   894,   896,   928,
    1237,     0,   801,     0,     0,   606,  1240,     0,     0,   233,
     233,   380,   461,   788,     0,     0,   527,   526,   537,     0,
     528,   529,   525,   530,     0,     0,   546,   794,     0,     0,
     798,  1216,     0,   813,     0,   811,     0,  1236,   815,   804,
     778,     0,   777,   783,   785,   784,   782,     0,     0,   515,
       0,     0,   233,   698,     0,     0,     0,  1170,     0,     0,
    1205,  1201,     0,     0,  1164,     0,     0,     0,     0,  1177,
    1178,  1179,  1176,  1180,  1175,  1174,  1181,     0,     0,     0,
       0,     0,     0,     0,   990,   989,     0,     0,     0,     0,
       0,     0,     0,     0,  1073,  1102,     0,     0,  1138,     0,
    1164,  1191,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1164,  1197,     0,  1217,  1216,     0,     0,     0,
       0,  1255,  1035,  1222,     0,  1218,     0,     0,  1042,     0,
    1198,  1068,     0,  1057,     0,  1051,     0,  1055,  1059,  1038,
    1219,  1163,     0,  1161,     0,     0,  1036,     0,  1031,  1029,
    1030,  1023,  1024,  1025,  1026,  1027,  1028,  1019,  1032,  1142,
    1140,  1141,     0,     0,     0,  1054,  1346,  1374,     0,   852,
     852,   841,   850,  1221,     0,   930,     0,     0,     0,  1103,
       0,     0,   368,     0,  1017,     0,   927,     0,   930,     0,
     755,   754,     0,     0,   866,   867,     0,   885,     0,   816,
     816,     0,     0,     0,     0,   654,   631,   465,   655,   632,
       0,   737,     0,     0,     0,     0,     0,   233,   743,   656,
     229,   744,   648,     0,   590,   591,     0,   576,   954,   564,
       0,     0,   571,     0,   570,   948,     0,     0,     0,   634,
     127,   227,   639,   228,   226,   635,     0,  1243,     0,     0,
       0,   642,     0,     0,   101,   103,   105,     0,     0,    99,
     107,   102,   104,   106,   100,   108,    98,     0,     0,    97,
       0,     0,     0,     0,    92,   660,   637,   650,   353,   363,
       0,   357,   358,   355,   359,   360,     0,     0,   204,     0,
       0,   216,   215,   222,   223,   221,   233,   233,   219,   220,
     218,   638,     0,   651,   217,   214,     0,   203,   662,   644,
       0,   661,   652,   643,   165,   164,   166,     0,     0,     0,
       0,     0,     0,     0,     0,   490,   473,   489,     0,     0,
       0,   467,   306,     0,   257,   238,   239,     0,   430,     0,
       0,     0,     0,   935,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   733,   733,   733,   733,   733,   718,
     741,     0,   307,   131,     0,     0,   383,     0,     0,     0,
     931,     0,     0,     0,     0,   272,   274,   276,   277,   278,
       0,   346,   348,   343,     0,     0,     0,     0,     0,     0,
     551,     0,     0,     0,   600,   596,   582,     0,   825,   826,
       0,   901,     0,   903,     0,   902,   915,     0,     0,   921,
     919,     0,   921,     0,   921,     0,     0,   895,     0,   897,
     915,  1238,   929,   233,     0,   233,   233,   684,   423,  1165,
       0,     0,   539,     0,   536,   531,   533,     0,   540,  1232,
    1231,     0,  1229,     0,     0,   800,   814,   807,   809,   808,
       0,     0,   805,   781,     0,     0,   233,     0,   541,   857,
    1172,  1173,     0,     0,     0,     0,  1202,     0,     0,  1134,
       0,  1132,  1110,  1112,   972,     0,  1122,     0,   966,  1135,
    1136,  1114,  1116,     0,   969,     0,  1155,     0,     0,  1123,
    1075,  1076,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1088,  1087,  1124,  1137,     0,
       0,  1189,  1190,  1125,  1001,  1002,  1003,  1003,     0,     0,
    1196,     0,     0,     0,  1130,  1095,  1165,   994,   995,     0,
       0,     0,     0,     0,     0,     0,  1058,  1053,     0,  1056,
    1060,     0,     0,     0,     0,  1044,  1069,  1040,     0,     0,
    1046,     0,  1070,   852,   852,   852,   852,   849,     0,     0,
     890,   880,     0,     0,  1107,  1104,   150,     0,   174,   160,
    1226,   817,     0,   766,   864,     0,  1156,   865,   888,   884,
     883,   869,   873,   870,     0,     0,     0,   733,     0,   300,
       0,     0,     0,   746,   584,     0,   948,   567,   568,   569,
       0,   657,   633,   649,     0,     0,   613,     0,   614,   612,
     113,     0,     0,     0,     0,    94,   114,   112,     0,   111,
       0,   356,   362,   361,   281,     0,     0,     0,   205,   213,
     210,     0,     0,  1213,     0,     0,     0,   469,   476,     0,
       0,     0,     0,   478,   309,   241,   242,   260,   240,   432,
    1400,  1146,   437,   433,   435,   436,   438,   418,     0,     0,
       0,   938,     0,  1011,  1012,  1013,  1014,  1015,  1016,  1010,
    1017,     0,   733,   732,     0,     0,     0,     0,     0,     0,
       0,     0,   292,     0,     0,   286,     0,     0,   339,   280,
     283,   284,     0,     0,   271,     0,   398,   392,   429,     0,
     933,   273,   299,   328,     0,   344,     0,     0,     0,     0,
     340,   714,   377,   374,     0,     0,     0,   594,   597,   595,
       0,   578,     0,   583,   561,     0,   818,   905,   913,     0,
     893,     0,   920,   916,     0,   917,     0,     0,   918,   907,
       0,     0,   913,     0,   607,   233,   450,   610,   345,   538,
     534,     0,     0,   512,   543,   793,     0,     0,   797,   799,
     812,   810,   546,     0,   513,   545,  1171,   978,     0,  1204,
    1200,     0,     0,     0,  1166,     0,     0,  1184,  1183,  1185,
       0,     0,     0,  1085,  1083,  1084,  1077,  1078,  1079,  1080,
    1081,  1082,  1074,  1086,  1194,  1193,  1187,  1188,   998,   996,
       0,  1127,  1128,  1129,  1195,  1139,   984,  1017,  1020,     0,
       0,  1043,  1199,  1061,     0,  1169,     0,  1039,     0,     0,
       0,     0,  1048,  1155,     0,     0,     0,     0,     0,     0,
     844,   843,     0,   882,  1105,  1106,     0,  1228,  1227,     0,
       0,     0,     0,     0,   302,     0,     0,   339,   585,     0,
     580,     0,     0,     0,   109,    95,   116,   115,    96,   110,
     279,     0,     0,     0,     0,   235,   640,   641,     0,     0,
     471,     0,     0,   472,   474,   261,    91,     0,   420,     0,
       0,   424,   414,   416,     0,     0,     0,   936,   939,   947,
     413,     0,   728,   727,   726,   723,   722,   731,   730,   721,
     720,   725,   724,     0,     0,   290,   294,   295,   293,   285,
     339,   307,     0,   287,     0,     0,     0,   400,     0,     0,
     932,     0,     0,     0,   296,     0,   331,   275,   347,   348,
     789,   341,   342,     0,   379,   375,     0,   550,     0,     0,
     600,   599,   679,   680,   677,   678,     0,   904,     0,     0,
     906,     0,     0,   909,   911,     0,     0,     0,     0,     0,
    1241,   622,     0,     0,     0,   532,  1230,     0,   233,     0,
    1203,  1121,  1131,   971,  1133,   968,  1182,     0,     0,     0,
    1126,  1256,  1066,  1064,     0,  1062,     0,     0,     0,  1045,
    1041,     0,  1047,  1071,   848,   847,   846,   845,     0,     0,
     837,  1017,     0,   653,   630,   647,   739,     0,   339,   303,
     581,   659,   636,   658,   206,   207,   208,   209,     0,   211,
     468,     0,   475,   470,   244,     0,   422,   421,   419,   415,
     417,   937,     0,   621,     0,   418,   729,     0,     0,   282,
     288,   314,     0,     0,   715,   273,   399,     0,     0,   393,
     331,   298,   297,     0,     0,     0,   335,   787,   376,     0,
     371,     0,     0,     0,   552,   559,   559,   598,   560,     0,
       0,   914,   923,     0,   910,   908,   898,     0,   900,     0,
     535,     0,   542,   233,  1089,     0,     0,  1063,  1168,  1067,
    1065,  1049,   879,   881,   175,  1157,     0,   304,   234,   477,
     237,   259,   621,     0,   618,   412,     0,   289,     0,   319,
       0,     0,   401,   402,   397,     0,   335,     0,   329,   330,
       0,   337,   378,   749,     0,   337,     0,     0,   557,   555,
     554,   930,   912,     0,   899,   796,   514,     0,  1090,   307,
     245,   249,   748,   748,     0,   250,   748,   246,   748,   243,
     617,   620,   619,     0,   311,   312,   313,     0,   291,   315,
     316,   338,     0,     0,   337,   327,     0,     0,   263,   559,
     930,   553,   558,     0,   922,  1091,   314,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   317,     0,   318,
       0,     0,     0,   439,   440,   405,   264,     0,   332,     0,
     336,   556,   547,   683,   319,   247,   252,     0,   253,   248,
     251,     0,   324,     0,   323,     0,   321,   320,     0,     0,
     445,     0,   407,   409,   410,   406,     0,   403,   408,   411,
     333,   334,   682,   681,     0,   418,   254,     0,   322,   326,
     325,     0,   449,   441,   387,     0,   665,   676,   664,   666,
     674,   671,   673,   672,   670,   305,     0,   444,   448,     0,
     442,   404,     0,   675,   669,   836,   827,     0,     0,   667,
     676,   405,   449,   668,     0,   443,   388
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    44,    45,    46,    47,   891,  1405,  1804,    48,    49,
      50,    51,    52,    53,    54,    55,    56,   913,    57,    58,
     673,  1102,  1492,    59,   537,   538,  1305,  1306,  1307,  1444,
    1308,  1293,   843,  1309,    60,    61,    62,   838,  1283,    63,
      64,   940,   941,   917,   942,   897,  1130,  2549,    65,    66,
    1867,  2177,  2630,  2679,   413,  1464,  2406,  2407,    67,  1099,
    1904,  1905,  1906,  1907,  1890,  2219,  2220,  2221,  1908,  2454,
    1909,  1357,  1053,  1460,  1461,  2639,  2688,  2689,  2690,  2746,
    2456,  2586,  2651,  2698,  2443,    68,  1916,  1511,  1910,  1911,
      69,    70,   930,  1417,  1810,  1310,  1295,  1296,    71,   437,
    2242,  2464,  2590,    72,   466,    73,  1104,    74,    75,  1107,
    1504,  1505,  2448,  2577,  2644,  2756,  2757,  2565,  2411,  2412,
    2413,    76,    77,    78,    79,  1055,  1465,  1466,  2183,    80,
    2723,  2724,   438,  2790,    81,    82,   467,   705,  2658,   886,
      83,    84,   655,  1856,    85,   483,   719,    86,    87,   490,
    1579,   492,  1158,  1964,  1965,  2283,  1547,   596,    88,    89,
    1556,  1582,    90,   439,  2246,  2593,  2594,  2659,    91,   440,
     909,  1379,  1380,  1381,  1382,  2250,  1383,  1924,  1925,  1926,
    2247,  2254,    92,   907,  1372,    93,    94,  1341,    95,  1400,
     613,    96,  2634,    97,    98,   511,  1175,    99,  1420,   100,
     101,   102,  1927,  2778,  2792,  2793,  2794,  2476,  2764,   103,
    2780,   105,   106,   107,   137,   628,   629,   630,   108,   109,
     110,  1483,  1889,  2204,   111,   112,  1350,  1741,   113,   114,
     115,  1095,   116,   117,   118,   119,   120,   558,   552,   865,
    1319,   121,  1144,   471,   122,   730,  1171,  1572,   123,   708,
     124,  2781,  1165,  1559,  1560,  2782,  1126,   127,  1169,  1566,
    1569,  2783,   129,   688,  1520,  2784,   808,   132,   133,   134,
    1278,  1701,   501,   872,   529,   563,   564,  1324,  1325,   879,
    1339,  1332,  1334,  2363,  2530,   880,  1337,  1729,  1705,  1524,
    1525,  1526,  1935,  1946,  2263,  2483,  1527,  1528,  1532,  1899,
    1900,  1384,  1472,  1079,   809,   810,   811,  1208,  1219,  1202,
    1082,   812,   813,   814,  1085,   815,   816,   817,   818,  1191,
    1243,   819,   820,  1228,  1480,  1141,  1623,   822,   823,   824,
    1692,  1271,   615,   825,  2107,  1273,  1643,  1607,  2346,  1586,
    1189,  1608,  1620,  2018,  1627,  1632,  2041,  2042,  1644,  1661,
     826,  1590,  1591,  1997,  1193,   827,   633,   634,  1234,  1657,
     828,   829,  1314,  1315,  1971,  1972,   830,  1159,   550,  2479,
     696,   593,  1776,  1033,   406,   498,   831,   832,   833,   834,
    1966,  1299,   496,   835,   588,  1034,   400,   401,   590,  1038,
     408
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -2400
static const int yypact[] =
{
    6310,   515,  2674, -2400, -2400,   515, -2400, 29904, 25200,    14,
     597,   263, 34592, 30240, 29904,   492, 34550,   515, 29904,   268,
   19110, 22848,   474, 25200,   340,   369, 19110, 25200, 30576,   529,
   30912, 26208, 22512,   629, 29904,   251, 31248, 26544,   476,   369,
   17740, 23520,   747,    27,   797,   531, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
     608, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400,   117, -2400,   516,   207, -2400, -2400, -2400, 21504, 29904,
   29904, 29904, 21504, 29904, 23520, 29904, 15286, 29904, 29904, 23520,
   23520, 29904, 29904, 29904, 29904,   106, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400,   338, -2400,   664,   765, -2400,  4443,
     340, -2400, -2400, 25200, 21504, 29904,   643,   726, 29904, -2400,
   29904,   631, 29904,   971, 15639,   844, 29904, 29904, 29904, -2400,
   -2400, 29904, -2400, 29904, -2400, 29904,   543,   838,  1079,  1027,
      90, 29904, -2400, -2400, 23520, 21504, 29904,   738, -2400, 29904,
   -2400, 21504, 29904, -2400, 15992, -2400, 29904, 29904, -2400, -2400,
   -2400, 29904, 29904, -2400, 29904, -2400,  1013, 29904, -2400,   843,
   -2400,   608,   712, -2400,   101,   123,   103,   130,   133,   138,
     712, -2400,   861,   946, -2400, -2400,   994, -2400, -2400, -2400,
    1022,   349, -2400, -2400, 25200, -2400, -2400, -2400, -2400, -2400,
   -2400, 25200,   946, -2400, -2400,   340,   895, 29904, -2400, 29904,
   -2400, 25200, 29904, -2400, -2400,  1274,   966,  1147,   949, -2400,
    1097,  1123,  1131,    38,   340,  1073, -2400, -2400,  1178,  9343,
   26880, 31584,   173, 20830, 27216,  1026,   106, -2400,   417, -2400,
    1378,  1072,  1253,   949,   106, 25200, -2400, -2400, 14278,  1146,
    1093, -2400,  1302,  1064,  1094, -2400,  6310, -2400, 25200,   582,
     582,  1404,   582,   508,   522, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400,  1116,   338,  1127, -2400,  1133,
   -2400,   609,  1099,   676, -2400,   153,  1078,  1145,   154, -2400,
     718,  1259, 29904, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400,  1151, -2400, -2400,  1149,   418,   776,  1242,
     -15,  1624,   819,  1307,    47,   857,  1364,     2, -2400,   367,
   -2400, 11842, 17404, -2400,   338, 25200, 21504,  1189, -2400, 29904,
   -2400, -2400, -2400, 21504, -2400,  1351, 16345, 29904, -2400, -2400,
   -2400, 29904, -2400, -2400,  1400, 29904, -2400,  1206,  1214,  1522,
   33600, 29904,  1243,  1556, -2400, -2400,  1243, -2400, -2400, 29904,
    1214,    64,  1243,  1551,  1552,  1354,   768,    83,  1243, 25200,
   25200, 25200,  1417, 29904, 29904, 21504, 29904, -2400,   652,  1263,
    1225,   125, 33600, -2400,  1145,  1221, -2400, 29904,  1228,  1221,
    1380, -2400,  1381,  1221, 20830,    60, -2400, 11842, -2400,   545,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, 29904,
   -2400, 23856, 29904, 24192,    37,    25, -2400, 29904, -2400, 33600,
    1573,  1480,  1480,  1480, -2400, -2400, -2400,  1430, 29904,  1485,
    1486, 23856, 29904, -2400, 31920,  1245,  1040,  1265,   500,  1266,
   11842,  1250,   254,   254,  1251,  1252, -2400, -2400,  1254,  1255,
   -2400,   -60,   -60,  1386,  1257,  1258, -2400,   431,  1267,  1278,
    1282,   632,  1268,  1270,  1271,  1087,   254, 11842, -2400,  1273,
     722,  1275,  1276,  1277,  1284,  1279, -2400,  1293,  1280,   781,
     791,  1283,  1286, -2400,  1257, -2400,   171,   737, -2400, -2400,
   -2400, -2400, -2400, -2400, 11842, 11842, -2400,  9700, -2400,   340,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,   113, -2400,
     746,  5185, -2400, -2400,  1403, 11842, -2400, -2400,    24, -2400,
     338,  1292, -2400, -2400, -2400, -2400,   778, -2400,   182,  1221,
    1126, -2400, -2400, -2400, -2400, -2400, -2400, -2400, 20492,  1622,
   -2400,   600,   367, 18412, 18412, -2400, -2400,  1476, -2400,  1291,
   25200, -2400, 29904, -2400, -2400,   608, -2400, -2400, -2400,  1295,
   -2400, -2400,    27,    27, 11842,    27,   471, 10057, 11842,  1541,
     438,   338, 18076,  1358,  1359, 21168,  1298,  1363,  1365, 31584,
   -2400, -2400, -2400,   107,   589,  1367,    87, -2400,  1454,  1405,
   -2400,  1466,   550, -2400, -2400, -2400, -2400,   116,  8649,   753,
   -2400, -2400,  1371,  1356,  1379,  1383,  1385,  1315, -2400,  1389,
    1370, 32592, 16698,  1391, 31584, -2400, -2400,  1394,  1397,  1423,
     829,    92,  1643,  1483,  1398,  1395,   786,  1407,   163,   637,
    1338, -2400, -2400,  1408,  1409, 23520,  1413,  1452,  1416, 31584,
   -2400, -2400,   669, -2400, -2400,   106, -2400,  2852, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400,  1357, 33600,  1503,  1145,   894, 29904,  1372,   433,  1509,
   29904,  1577, 29904,  1406, 21168, -2400,  1382, -2400,  1420, -2400,
    1420,  1420,  1402,  1402,  1410, -2400, -2400, -2400,  1087,  1420,
    1424, -2400, 34272, -2400,   729,   802, -2400, -2400,  1710,  1720,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,   473,
    1099, -2400,  1744, -2400, -2400, 33600, -2400,  1674, -2400,  1429,
   -2400, 29904,   552, 29904,  1583, -2400, -2400,   479,  1425, -2400,
   -2400, -2400,    -5,  1206, 20830,  1647,  1549, -2400,  1145,  1745,
   -2400,   811, -2400,  1505, -2400, 13606,  1435, 18076, -2400, -2400,
   -2400,  1749,   125, 29904,  1453, 32592, 25200, 25200,   125, 29904,
   -2400, 19791,   832,   457, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, 29904, 21504, 29904, 29904, 25200, 29904,  1477,  1291,
     -61, -2400,  1462,  1428, 14614, -2400, -2400,   126, 25200,  1586,
   -2400,   854, -2400,    62, -2400, -2400, -2400, -2400, -2400, -2400,
   24192, 22848,  1671,   187, 29904, -2400, 11842, 10414, -2400, -2400,
   -2400,  1433, 19791,  1467, 11842, -2400, -2400, 11842, 11842,  1456,
    1456,  1456, -2400, -2400, -2400, -2400,   978,  1456, -2400, 11842,
   11842,  1456,  1456,  1420,  1420, -2400,  2361, 11842,  1456, -2400,
   16698, 11842, 12199,  7954, 11842,  1498,  1510,  1456, -2400,  1456,
   -2400, 11842,  8986, -2400,   338,  1460,  1460,   808,  5663,  1449,
   -2400,  1544,  1456, -2400,  1456,   553, 11842, 21168,  1525,   111,
   11842,  1470,  1285, -2400, 11842,   528, -2400,  1471, 11842,  1532,
     115, 11842, 11842, 11842, 11842, 11842, 11842, 11842, 11842, 11842,
   33600, -2400, 12556,   148,   -35,    82, 24528,  9343,  1704,   863,
    7596, -2400, -2400, -2400,  1622, -2400, -2400,  1530, -2400, -2400,
   -2400, -2400,   622, -2400, -2400, -2400, -2400,   813, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400,  1469, -2400, -2400, -2400,
   -2400, -2400, -2400,  1474,    54, -2400, -2400, 23184, 29904, -2400,
   -2400,  1683, -2400, -2400,  1472, -2400,  4841,  1683,  1623,  1630,
    1630, -2400,   -34, 19791,  1670, 19791, -2400, -2400, -2400, -2400,
   -2400,  1484, -2400, 29904, 29904, -2400, 21168, 29904, 29904, -2400,
    1787,  1487, 29904,  1687,  1689,  1491, -2400, -2400, 29904, -2400,
    1648, 29904, 11842,  1649, 29904,  1650,   614,  1638, -2400, -2400,
   -2400, -2400, -2400,  1636, -2400, -2400, 34272,    89, -2400,   879,
   -2400, 24864, 32256, -2400, -2400,  1099, -2400,   224,  1559,  1561,
    1603, 29904, 29904, 29904, 29904, 29904,    51, 29904, 29904,  1504,
    1507,  1508, -2400, 29904, -2400,  3274, 29904, 29904, 29904,   894,
   -2400,  1834,   894,   894,   800,  1243,  1243, -2400, -2400, -2400,
   29904, 29904, 29904,  1582, 25536, 29904, 29904, 25872, 29904, 29904,
   29904,   144,  1142, 29904, 29904,  1628, 29904, 29904, 29904, -2400,
    3274,    96,  1637, -2400, -2400, -2400, -2400, 11842, 18076,  1858,
   29904,  1722,  1725,  1563, 32592,    41, 29904, 29904,  1727,    41,
     892, -2400, -2400,  1672,   199,   893, -2400,  1543, 11842,  1869,
   33600,  1534,  1538,  1598,  1600,  1601, -2400, -2400,  1602,  1456,
   -2400,  1298,   479,   615, -2400,  3274,  1788, 25200,  3274, -2400,
    1390, 22848, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
     340, -2400, -2400, -2400,  1688,  1686, 29904,  3274,   829, 25200,
   21840,   215,  1888,  1780,   340, 25200,   850, -2400,  1778,  1779,
    1796, -2400, 13270, 27552,  1548,   119, -2400, 27552, 27888, -2400,
     338, 11842, -2400,  1553, 33600, -2400, -2400, 29904,  1554,   125,
     125, -2400, -2400, -2400, 11842,  1627,  1221,  1555, -2400,  1145,
    1221,  1221,  1291,  1221, 28224,  1907,  1795, -2400, 10771,   909,
   -2400, -2400,    93, -2400,    97,    17,  1738, -2400, -2400, -2400,
   -2400, 33600, -2400, -2400, -2400, -2400, -2400,   145,  1708,  1562,
   28224,  1892,   125, -2400,   911,   275,   656, -2400,  1456, 11842,
      74, -2400,  5714,   919,  5826,   940,  1568,  1569,   959, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400,  1572,  1803,  1574,
     962,   965,  1579,  1584, -2400, -2400,  1864,   972,  1587,  6435,
    1590, 12199, 12199,  1451, -2400, -2400, 12199,  1591, -2400,   974,
    2880,  1578,  1593,  1597,  1610,  1594,  1596,  6499, 11128, 11842,
   11128, 11128,  6674,  1578,  1599, -2400, -2400, 11842, 29904,  1604,
    1607, -2400,  2361, -2400,  1626, -2400, 12199, 12199,  3140,  9700,
   -2400, -2400,  1840, -2400,   596, -2400,  1609, -2400, -2400,  3639,
     124, 11842,  1470, 11842,  1664, 16698,  4593, 11842,  3111,  3413,
    3413,   208,   208,   168,   168,   168,    80, -2400,    82, -2400,
   -2400, -2400,  1616,  1279, 11842, -2400,  1038,  1107, 25200,  1290,
    1828, -2400, -2400, -2400, 13606,  1435, 11842, 11842,  1618, -2400,
     983,   106, -2400,  1456,   553, 18761, -2400, 29904,  1435,   869,
   -2400, -2400,   986, 11842, -2400, -2400,  1770, -2400, 29904,  1586,
    1586, 11842, 11842, 10057,   824, -2400, -2400, -2400, -2400, -2400,
    1799, -2400, 11842,   298,  1617,  1620, 29904,   125, -2400, -2400,
   19791, -2400, -2400,  1836, -2400, -2400,  1782, -2400,  1099, -2400,
   22176, 33936, -2400, 33936, -2400,   468, 29904, 29904, 29904, -2400,
    1221, -2400, -2400, -2400, -2400, -2400,   821, -2400, 33600,  1759,
   32928, -2400, 29904,  1814, -2400, -2400, -2400,  1762,   108, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400,   340, 29904, -2400,
    1456,  1763, 29904,  1669, -2400, -2400, -2400, -2400, -2400, -2400,
     894, -2400, -2400, -2400, -2400, -2400,   894,   894, -2400, 33600,
      81, -2400, -2400, -2400, -2400, -2400,   125,   125, -2400, -2400,
   -2400, -2400,  1691, -2400, -2400, -2400,   352, -2400, -2400, -2400,
    1693, -2400, -2400, -2400, -2400, -2400, -2400,  6711,  1633, 33600,
    1838,    41,    41, 29904,  1635, -2400, -2400, -2400,  1843,  1844,
      41, -2400, -2400, 29904, -2400, -2400, -2400,   607, -2400, 21168,
   17051,  6728,  1639,  1538,  1642,  1456,   626,   443,   390,  1741,
    1821,  1651,  1793,  1839,  1666,  1666,  1666,  1666,  1666, -2400,
    1215,  1701,  1206,   552,  1765,   646, -2400, 25200,   479,   991,
   -2400, 33600,  1655, 25200,  1656,  1654, -2400, -2400, -2400, -2400,
    1005, -2400, 33600,  1909,  1807,  1812,    27,    27, 29904,  1724,
    1706,   340, 20830, 33264,   964, -2400, -2400,  1815, -2400, -2400,
      27,  8312,   119,  1668, 29904, -2400,  1667, 13606,  1872,  1813,
   -2400, 13606,  1813,    85,  1813, 13606,  1873, -2400, 14950, -2400,
    1676,   863, 19791,   125,  1677,   125,   125, -2400, -2400, 19791,
   25200, 21504, -2400, 29904,   391, -2400, -2400,  1819, -2400, -2400,
   19791,  1034, -2400,   320, 29904,   338, -2400, -2400, -2400, -2400,
    1933,  1935, -2400, -2400, 29904, 23856,    67, 29904, -2400, -2400,
   -2400, -2400,  1685,  1690, 19382, 11842, -2400,  1945, 33600, -2400,
   29904, -2400, -2400, -2400, -2400,  1456, -2400, 11842, -2400, -2400,
   -2400, -2400, -2400, 11842, -2400,  1456, -2400, 11842,  1928, -2400,
    1692,  1692, 12199,   487,  1275, 12199, 12199, 12199, 12199, 12199,
   12199, 12199, 12199, 12199, 33600, 12913,  1028, -2400, -2400, 11842,
   11842,  1934,  1928, -2400, -2400, -2400,   915,   915, 33600,  1695,
    1578,  1696,  1703, 11842, -2400,   338, 19423, -2400,  2008,   340,
   11842,   499,   602, 11842,  1049, 11842, -2400, -2400,  1711, -2400,
   -2400, 33600, 11842, 12199, 12199,  4119, -2400,  4870, 11842,  1705,
    5235,  9700, -2400,   369,   369,   369,   369, -2400, 25200, 25200,
    1548,  1925,  1063,  1066, -2400, -2400,   367,  1709, -2400, -2400,
   -2400, -2400, 11485, -2400, -2400,  1712, -2400, -2400,  1221, -2400,
   -2400, -2400, -2400, -2400,  1773,  1776,  1816,  1666, 19450, -2400,
   29904, 29904,  1069, -2400, -2400,  1932,   565, -2400, -2400, -2400,
    1775, -2400, -2400, -2400,  1792,  1800, -2400,  1801, -2400, -2400,
    1221,   712,   340, 29904, 29904, -2400,  1221, -2400,   340,  1221,
     340, -2400, -2400, -2400, -2400,   462,   458, 33600, -2400, -2400,
   -2400, 29904, 29904, -2400,  1930,  1755,    41, -2400, -2400,  1931,
    1942,    41,    41, -2400, -2400, -2400, -2400,   218, -2400, -2400,
    1712,   622, -2400, -2400, -2400, -2400, -2400,   162,  1456,   957,
    1768, -2400,  1772, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
     553, 25200,  1666, -2400,    69,   120, 28560, 28896, 29232,  1743,
   29904, 12199, -2400,   189,   150, -2400,  1968, 25200,  1817, -2400,
   -2400, -2400, 33600,  2113,   457, 25200,  2018,  1916, -2400, 29904,
   -2400, 21840,   143,  1993, 21840, -2400, 29904, 29904,  2118,  2120,
   -2400, -2400,  1822,  1099,   340, 29904,  1784, -2400,  1777, -2400,
    1039, -2400,  1214, -2400, -2400,    68, -2400, 27552,  1790, 29904,
     119, 13606, -2400, -2400,   931, -2400, 13606,  1987, -2400, -2400,
   13606, 29904,  1791, 29904, -2400,   125, -2400, -2400,  1797, -2400,
   -2400,  2023, 28224, -2400, -2400, -2400, 10771,  1798, -2400, -2400,
   -2400, -2400,   187,  2031, -2400, -2400, -2400, -2400, 11842, 19791,
   -2400,  1794,  1802,  1805, 19791, 19471,  1806, 19791,  1934,  1742,
    2039,  1952,  1808,  1742,  1396,  1396,   932,   932,   822,   822,
     822,  1692, -2400,  1028, 19791, 19791, -2400, -2400, -2400, -2400,
    1810, -2400, -2400, -2400,  1578, -2400, -2400,   553,  1460, 12199,
   12199,  3582, -2400,   248, 33600, -2400,  1092,  3582,   685,  1427,
   11842, 11842,  6094,   250, 11842, 19498, 25200, 25200, 25200, 25200,
   -2400, -2400,  2133,  2043, -2400, -2400,   340, -2400, 19791, 16698,
   29904, 29904, 29904,   712, -2400,  1095,  1102,  1817, -2400,  1865,
   -2400, 29904, 29904, 29904, -2400, -2400,  1221,  1221, -2400, -2400,
    1215,  1972,  1973,   712, 29904,  1854, -2400, -2400,    41,  2024,
   -2400,    41,    41, -2400, -2400, -2400,  1243,  2091, -2400,   538,
    2100, -2400,  2041,   175,  1831,  2070,  2071, -2400, -2400, -2400,
    2074,   712, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -2400, -2400, 11842,   593,  1742, -2400, -2400, -2400, -2400,
    1817,  1206,  2055, -2400,  1878,    27,    -1,  2102,  2093,   479,
   -2400,  1842,  2121,  2124, -2400,  1841,  1053, -2400, -2400, -2400,
     843, -2400, -2400, 29904,  2047,  1298,  2089, -2400, 13942,   340,
     964, -2400, -2400, -2400, -2400, -2400,  1921, -2400, 29904,  1114,
   -2400, 11842,  1863, -2400, -2400, 13606,   931,  1124, 29904,  1138,
   33600, -2400, 29904,  2205,  2009, -2400, -2400, 10771,   125, 28224,
   19791, -2400, -2400, -2400, -2400, -2400, -2400, 12199,  1868, 33600,
   -2400, -2400,  1465,  1465,  1160, -2400, 33600, 12199, 12199,  3582,
    3582, 11842,  3582, -2400, -2400, -2400, -2400, -2400, 11842, 11842,
   -2400,   553,  1870, -2400, -2400, -2400, -2400,  1989,  1817, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, 11842, -2400,
   -2400,    41, -2400, -2400, -2400,   340, -2400, -2400, -2400, -2400,
   -2400, -2400, 21504,  2213, 25200,   162, -2400, 19539,  2028, -2400,
   -2400,  2056,  1944, 29904, -2400, 21840, -2400,   406,  1997, -2400,
    1053, -2400, -2400, 25200,  2025,  2026,  2027, -2400, -2400,   340,
   -2400, 29904, 11842,  1167, -2400, 29568,  7254, -2400, -2400, 25200,
    1169, -2400, 19791, 29904, -2400, -2400, -2400,  1177, -2400,  1909,
   -2400,  1180, -2400,    67,  1164, 33600,  1183, -2400, -2400,  1465,
    1465,  3582,  1578, 19791, -2400, -2400, 25200, -2400, 19791, -2400,
    1132, -2400,  2213,   246, -2400, -2400,  2125, -2400,   466,  2029,
   29904,  2220, -2400, -2400, -2400, 21504,  2027,  1191, -2400, -2400,
    2188,  1953, -2400, -2400, 19765,  1953, 13942, 29904, -2400, -2400,
   -2400,  1435, -2400,  1194, -2400, -2400, -2400,  1197, -2400,  1206,
   -2400, -2400,  1556,  1556,   568, -2400,  1556, -2400,  1556, -2400,
   -2400, -2400, -2400,  2150, -2400, -2400, -2400,   255, -2400,  2033,
    2034, -2400,    77,  1893,  1953, -2400,   141, 29904, -2400, 29568,
    1435, -2400, -2400,  2162, -2400, -2400,  2056,   340,   340,  2051,
   29904,   340,   340,  2001,   510,   510,  2173, -2400,  1943, -2400,
    1456,  1456, 33600,  1898, -2400, 20150, -2400,  2005, -2400,  2006,
   -2400, -2400, -2400,   201,  2029, -2400, -2400, 29904,  1904, -2400,
   -2400,  2163, -2400,  2265, -2400,   142, -2400, -2400, 21504, 16698,
   -2400,    77, -2400, -2400, -2400, -2400,  1200, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400,    55,   162,  1904,  2038, -2400, -2400,
   -2400,  1145,   -30, -2400, -2400, 20150, -2400,    57, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, 21504, -2400, -2400, 32592,
   -2400, -2400,   744, -2400, -2400,   826,  1094,  1917,  1918, -2400,
      57, 20150,  2040, -2400,  1208, -2400, -2400
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
   -2400, -2400, -2400,  1726, -2400,  -579,  -324, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400,   794, -2400, -2400,  -421,   956,  1431, -2400,   569, -2400,
   -2400, -2400, -2400,  -504, -2400,   378, -2400, -2400, -2400, -2400,
   -2400, -2400,   855, -2400,  -530,   469,  -647, -2400, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400, -1017,    -7,
      61, -2400,    52,   871,   139, -2400,  -139, -2400, -2400, -2400,
    -118,   551, -1100, -1693,   434,  -410,  -436,  -391,  -388,  -412,
   -2400,  -275,  -340, -2379, -1781, -2400, -2400,    32, -2400,    73,
   -1016, -2400,  1201, -2400, -2400,  -792, -2400,   -80, -2400, -2400,
    -146, -2400, -2400, -2400,   504, -2400, -2400, -2400, -1014, -2400,
     837, -1807, -2400, -2400, -2400,  -481,  -452, -2400, -2399,   -91,
     -88, -2400, -2400, -2400, -2400,  -663, -2400,   459, -2400, -2400,
   -2400,  -425,  1230,  -470, -2400, -2400, -2400, -2400,  -129, -1050,
   -2400, -2400, -2400, -1412, -2400,  2307,  1832, -1013, -2400,     9,
      23,  1612,  -715, -1518,    56, -2400, -2400, -1137, -2400, -2400,
   -2400, -1480, -1008, -2400, -2400, -2400,  -319, -2346, -2400, -2400,
    -692, -2400,   590,   969,   976, -2400, -1273,   102,  -485, -1834,
   -2400,  -112, -2400, -2400, -2400, -2400, -2400,  -991, -2400, -1126,
    -420, -2400,  -273, -2400, -2400, -2400,   478, -2400,   222, -2400,
   -2400, -2400, -2400, -2400, -2400,  -404,  -437, -2400, -2400, -2400,
      84, -2400, -2400, -2400,  1054,  -609,  -505,  1820, -1007, -2400,
   -2400, -2400, -2400, -1667, -2400, -2400, -2400, -2400, -2400, -2400,
   -2400, -1507, -2400, -2400, -2400,  1050,  2347,  -371, -2400, -2400,
   -2400, -2400, -2400, -2400, -2400, -2400, -2400, -2400,  1659,   -90,
   -2400,    35, -2400, -2400,   395,    36, -2400, -2400, -2400, -2400,
    -298,    39,  1662, -2400, -2400,     8,     0,   -40,   386,   502,
   -2400, -2400,   -24,   716, -2400, -2400, -2400, -2400,   650,  1494,
   -2400,   648,  -235, -2400, -2400,  1497, -2400,  1052,  1065,   679,
   -1403,   862, -1446,   446,  -453,  -100,    31, -2400, -1653, -2104,
     164,   -26,   922,  -957, -2400,  2245,  2889, -2400, -2400,  -646,
   -2400, -2400,  2985,  3257, -2400, -2400,  3437,  3604, -2400,  -631,
     334,  3647,  1811,  -765, -1196,  4360,  1148,   314, -1114,    79,
   -2400,  -140, -2400,  4651,   524, -2400,  -665, -2400, -2220, -2400,
   -1109, -2400, -2400, -2400, -2400, -2400, -1741, -1921,  -281,   723,
   -2400, -2400,   806, -2400, -2400, -2400,  -602,  -532, -1191,   727,
   -2400,  1121, -2400,   682,   -96,   118,   370,  -500,  1045,  -434,
    1714,  -352, -1489,  -793,  1718, -2400,  -119, -2400, -1072,   206,
    -126,     7,    43,  3886,  -840,  -987,  2166,  -595,  -572, -2400,
   -2400
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1457
static const short yytable[] =
{
     131,   725,  1132,   554,   670,   436,   614,  1098,   130,  1538,
     591,  1529,   595,  1512,  1109,   545,  1548,   598,   695,   587,
     956,   617,   699,   597,   624,  1230,  1182,   485,   625,   846,
     703,   852,  1039,   485,   698,   125,   126,  1036,   925,   128,
    1481,   522,  1142,   553,   491,   859,   950,  1861,  1955,  1651,
     399,   407,  2091,  2122,   881,   523,   399,   399,  1140,  1294,
    1037,   399,  1986,   399,   493,  2101,   662,  1467,  1386,   399,
     918,   399,   549,   513,   519,   493,  1968,   526,  1587,   519,
     519,  1947,  1949,  1093,   104,  1493,  1494,  1096,  1495,  1496,
    2253,  2227,  1345,  1100,  1497,  1498,   839,   693,  1128,  1110,
     709, -1291,  1108, -1344,  1248,  1128,  1248, -1268,  1762,  1764,
     841,   906,   711,   850,   713,  1469,  1203,  -828,  1162,  1932,
    2326,  1195,  1196, -1342,  2514,  1719, -1143,  1596,  1597,  1598,
   -1370,  1562, -1143, -1386,  1655,  1609,  1533,  1351, -1408,  1612,
    1613,    15,  1239,    15,  1418,  1215,  1617,  1655,    15,  1352,
    2422,  1844,  1351,   898,  2472,  1635,  1241,  1636,   742,  1689,
     893,   910,  1167,  1128,  1352,  1690,  2636,  2487,  1362,  2489,
    1649,  1995,  1650,  -181,  1704,   600,  2155,  1731,  1276,  -828,
     619,   621,   592,   594,   592,  1938,   599,   421,   399,   616,
     599,   399,  1248,  1834,   399,   399,   592,   599,  1976,  2720,
    1138,  2425,  1978,    22,  1509,    22,  1939,  -831,  1509,  1353,
      22,  2788,   685,  2762,  -988,  1418,  2473,   953,  2205,  2206,
    2207,  2208,  1693,  2769,  1353,   899,  1047,  2727,   409,  1940,
     499,  1563,  1248,  2438,  1913,  2143,  2728,   423,  1252,  1253,
    1939,  -828, -1269,   956,  2266,  2408,  1855,  2452,   894,   911,
    2660,   900,  1942,  1452,   842,  -924,  2776,    27,  2408,    27,
    1653, -1159,   527,  1940,   626,  1984,  1281, -1159,  2681,  -831,
    1555,   946,  1248,  2436,  1980,   914,  2700,  2308,  1941,  1039,
    -973,  2453,     3,     4,   614,  1256,  1942,  1363,  1160,  2616,
    1036,   663,    35,  2721,   412,   658,   666,  -993,  1279,  1201,
     672,  2327,    35,  1133,   677,  2409,  -262,  1943,  1183,   678,
    1230,  1129,  1581,  1037,   614,  2726,  1282,   947,  1129,  1865,
      35,   916,    35,  2437,  1694,  1354,   690,    35,  1854,  2789,
    1732,  -831,   694,  2474,  1981,  1765,   954,   863,   706,  1944,
    1354,  2714,   528,  1351,  1286,   627,  1445,  2770,  1364,   686,
    2763,  2156,  1451,  2731,  1510,  2439,  1835,   429,  2575,   430,
    1163,  2722,  2410,  2144,  2169,  1977,  2785,  1370,   901,  1979,
    1975,  2729,    41,  1944,    41,  2410,  1129,   895,   404,    41,
     902,   497,  2682,  1564,  2607,  2475,    43,  2506,  1277,  1168,
    2157,  -271, -1143,   404,   903,  2667,  1164,   404,  1565,  1656,
     912,   904,   723,  1589, -1457,  1355,  1845,  1881,  1529,   497,
     404,   404,  2073,   802,  2777,  1353,    43,  1203,  1717,  1469,
    1355,    43,  1371,   896,  1139,  1691,  1516,  2375,  2376,  1195,
    1196,  2282,  1264,  1265,  1266,  1267,  1268,  1269,  1215,  2167,
    2168,  1270,  1054,  1270,   905,   481, -1268,   481,  2173, -1268,
    2373, -1268,   482, -1268,   482,  2055,  1945,  1848,   399,  1036,
     497,   594,  1428,   592, -1291,   599, -1344,   616,  1349,   599,
     674,   399,  1242,   920,   399,   689,   592,  -828,   599,   710,
    -828,   712,  1037,   714,   399,  1535, -1342,   715,  2128,   399,
    2129,  1541,   594, -1370,  1317,   399, -1386,   616,  1866,   399,
     399, -1408,  1402,  1404,   399,   399,   614,   399,   586,   723,
     592,  -988,   586,   497,   404,  2339,  1993,  1041,  2098,   404,
     404,  1584,  1585,  1269,  1044,   626,  1051, -1159,  1439,  1270,
    -988,  1354,  1593,  1595,  2260,  2421,  -181,  2405,  2264,  1362,
    1097,  1386,  2269,  1359,  1610,  1611,  1914,  1915,  2742,  1473,
     399,  1133,   399,  1737,  1115,   734,   131,  1670,  1629,  1631,
    1266,  1267,  1268,  1269,   130,  1501,  1118,  -831,  1134,  1270,
    -831,  2196,  2715,   399,   519,  2193,   847,   519,  2310,  1190,
     421, -1269,  1614,  1615, -1269,    35, -1269,  2684, -1269,  1455,
    1529,   125,   126,   870,  -993,   128,  2539,  2184,  1264,  1265,
    1266,  1267,  1268,  1269,   507,  1474,   627,  1143,   877,  1270,
    1513,  1355,   444,  -993,  1490,  1710,   656,  1808,  2340,   559,
    1811,  1812,  2556,   876,  2194,    14,  1440,  1502,   494,  1473,
     423,    15,  1645,  1990,  1078,  1368,  2253,  -860,  2209,  1544,
     104,  2097,  2579,  2287,  1395,   592,  1358,  1456,   722,   878,
    2251,    18,  1036,  2197,  1475,  2023,  1552,  1476,  2392,  2570,
     500,  1671,  2391,  2642,   923,  1672,  1131,    31,   560,   508,
    1359,  1883,  2557,   871,  2211,  1037,  1491,  1039,   877,    43,
     497,  1288,   399,  1120,  1431,  1474,  2643,  2311,   924,   616,
     399,  2685,  -860,    22,   399,   631,  2312,  1673,   592,  1754,
     632,  2517,  2743,  1172,   399,  1369,  2195,  2066,  2126,   131,
    1328,   728,   592,   723,  1884,  2024,   853,  1150,  1546,   878,
    1550,  1551,  2281,  1553,   131,  1121,   592,  1117,  2147,   399,
     743,   955,  1166,  -860,  1475,   561,  1477,  1476,   844,  2393,
     592,  2686,  2394,  1329,  1146,  1147,  1188,   847,  1148,   890,
     429,  1036,   430,  1067,  2400,  2282,  2467,  2627,  2023,  2403,
    2404,  2744,   399,   854,  1205,   399,   493,   554,  2709,  1755,
     399,  -967,  1485,  1289,  1037,  1356,  1488,  1105,   544,   738,
    2745,  1179,   614,   404,   586,   399,  1507,  1185,  1330,  1360,
    1207,  1885,  2223,  2568,  1233,  1106,  1503,   555,  2215,  1122,
    1618,  2067,  1674,  2190,  2192,  2068,  2710,  1237,  1478,   679,
      35,  2477,  2498,  1419,   404,   586,  1477,   135,  2024,   509,
     510,   586,  2130,  1529,  2279,  1190,  2216,  1529,   562,   410,
    1346,  1529,  1479,   883,   680,  2217,  1816,  1817,  1418,  1886,
    -993,  2023,   801,  1425,  1123,  2025,  2026,  2027,   136,  2028,
    2029,  2030,  2031,  2032,  2033,   884,  1342,  1962,  2480,  -993,
    2034,   524,    41,  2484,   404,  1409,  1774,  2486,   551,  1921,
     681,   404,  1320,  1320,  1813,  1320,  1493,  1494,   635,  1495,
    1496,   404,  1467,  2296,  1722,  1497,  1498,   898,  1478,  2175,
    2176,   847,  1957,  1958,   556,  1401,   847,   847,  1430,   135,
     887,  2024,  2069,  1410,    43,  1316,  2218,  1887,  1441,  1888,
    2070,  -836,   679,   654,  1124,   404,  1442,  1453,   404,  2379,
    1386,  1458,   888,  -836,   664,    31,   665,  1346,   404,  1395,
     136,   135,   519,  2303,   557,  1988,  1443,  2225,  1290,  1291,
     497,   682,   914,  2306,   481,   801,   889,   497,  2025,  2026,
    2027,  1292,  2028,  2029,  2030,  2031,  2032,  2033,  1770,   899,
    -836,  1712,   136,  2034,   915,   616,   716,   519,  1411,  1431,
     802,   892, -1000,   681,  2050,  1489,  1435,  1388,  -836, -1245,
    1814,  2613,  1815,  1921,  2191,   900,  2550,   518,   916,  2552,
    2553, -1000,   519,   543,  2064,   926,  1739,  1938,  1951,  1389,
     927,   898,   660,   951,  2419,   404,   586,   890,  2703,  1412,
    1542,  1413,  1922,   586,  1991,  1240,  1449,  2738,  2151,  -836,
    1992,  1414,   928,  1390,  2152,  2153,  1245,  -836,   801,  1239,
     661,  2025,  2026,  2027,  1549,  2028,  2029,  2030,  2031,  2032,
    2033,  2092,  2093,   943,  2766,  2134,  2034,  2732,  2114,   404,
     404,   404,  1939,   405,  1599,   586,   481,  1298,  1921,   155,
    1225,  1226,  -970,   482,   411,   944,   717,  2135,   495,  1484,
    2115,   468,   504,   899,   455,  1940,   898, -1268,  2182,  1415,
     614,  1218,  2604,   718,  2641,   547,   497,   525,  1227,   592,
    1941,   404,   671,   592,  2116,  1462, -1270,   692,  1942,   900,
    2123,  1342,  1923,   948,  2799,  1244,    31,  2800,  1416,  1401,
    1600,   404,  1225,  1226,   902,   898,  2414,  1645, -1269,  1943,
    -836, -1003,  1225,  1226,   554,  1523,  1922,   949,   903,  1213,
    1214, -1003,  2423,  1225,  1226,   904,  1545, -1271,   683,  2629,
    1227,  2511,  1518,  1519,   599,  2481,   399,  1529,   899,   931,
    1229,   848,  1529,   497,  1421,   932,  1529,   847,  1426,  1601,
    1429,  1229,  1602,  2670,   553,  2707,  2708,   684,   866,  2711,
    1284,  2712,  1713,   131,   900,   704,   399,  2033,   905,  2159,
    2160,  1576,   592,  2034,  2023,  1944,   866,   899,   890,  -828,
    1578,   933,  1543,  2465,   620,   399,  1544,   399,   399,  2671,
     399,  1922,   707,  1577,   934,   716,  2096,  1561,  1573,  1574,
    1176,  1177,  1575,   900,  1570,  2102,   901,  1735,  1571,  2672,
     631,  1738, -1248,   493,   493,   632,   631,  1583,   902,   720,
     404,   632,   935,  1291,  2673,  1749,   721,   936,   481,  1759,
     937,  1603,   903,  1760,  1687,  1292,  1225,  1226,  2674,   904,
    2482,  1660,  1862,  1868,   729,  2079,  1863,  1869,  1321,  1323,
    2209,  1327,  2675,   616,   667,  1769,   668,  1771,  1945,  1973,
    2293,  1989,  2210,  1974,  1036,  1544,   873,  1781,   875,  1999,
    1805,  2328,  2329,  1544,  2030,  2031,  2032,  2033,  2415,  2416,
   -1454,   901,   905,  2034,  2108,  2252,  2211,  1037,  2212,   735,
    2001,  1625,   736,   902,  1544,   737,  2274,  1838,  2276,  2277,
    1841,   738,  1843,  1604,   739,   404,  1605,   903,  1606,  2004,
   -1457,  1850,  2009,  2005,   904,  2010,  1544,  1858,  1859,  1544,
     901,  2083,  2014,  2084,  2038,  2624,  2015,  2676,  1544,  2294,
     740,  2571,   902,  2095,  2677,   741,  2103,  1544,  2140,  1039,
    1133,  2228, -1454,  1695,  2595,  2229,   903,  2049,  2213,  2051,
    2052,   399,   851,   904,  2146,  2235,   914,   905,  2149,  2236,
    2678,  1529,   744,  1645,  1322,  1322,  1662,  1322,  2028,  2029,
    2030,  2031,  2032,  2033,  2584,  2585,   599,   399,  2334,  2034,
     599,   594,   745, -1454,  2285,   399,  1663,  1187,  2286,    43,
    2085,   399,  2086,   855,   599, -1454,   905,   399,   856,  2342,
   -1454,   857,  1836,  1544, -1454,  2214,   862, -1454,  2249, -1454,
    2215,   864,  1342,  2364,   866,  2632,  2365,  1544,  1401,  2377,
    1544,  2109,  2110,  1863,   599,   399,   599,   399,   399,   874,
     399,  1777, -1454,  2518,  1874,   863,   599,   417,  2216,   599,
     399,   399,  2515,  1645,   867,  2537,  2516,  2217,   657,  1863,
   -1454,   885,  2538,  1819,  1820,   399,  1863,   399,   399,  1827,
     399,   399,   399,   399,  2601, -1270,   599,   399,  1133,   599,
     399,   599,   554,  1895,  2606,  1664,   882,   956,  2229,  2265,
    1665,  2268, -1271,   592,  1666,  1530,  2111,  2112,  2608,   592,
     592, -1454,  2229,  1714,   908,   919,   404,   404,  1954, -1454,
     921,   922,   929,   421,  2028,  2029,  2030,  2031,  2032,  2033,
    2617,   945,  1931,   586,  2516,  2034,   404,  2655,  2218,  2662,
    -549,  2656,   952,  1133,   493,   614,  1624,  2664,   404,   724,
    2665,  2229,  2595,  2668,  2286,  1983,   726,  2516,  1042,  1901,
    2292,  2695,  2023,  1912,  2704,  1168,   733,  2705,  1133,  1045,
    2774,  2516,  1050,   423,  2775,  1052,  1936,  1056,  2806,  2706,
    1936,  1950,  2775,  1054,   890,  1094,  1101,  -133,  1103,  1114,
    1777,   500,  1125,  2023,  1127,  1133,  2106,  1135,  2022,  2471,
     726,  1667,  1173,   861,  1136,  1137,  1174,   599,  1178,  1668,
    1180,  1181, -1454,   869,  1186,  -958,  -965,  2023,  1894,  1194,
    1197,  1198,  2024,  1199,  1200,  1204,    43,  1206,  -955,   554,
   -1454,  2023,  -956,   599,  -959,  1274,  1209,  1210,  2491,  1211,
    1212,   931,  1217,  -957,  1220,  1221,  1222,   932,  1223,  1224,
    2131,  1287,  1231,  2024,  1312,  1232,   404, -1454,  2748,  2749,
   -1454,  1280, -1454, -1454,  1318,  1168,   876,  1343,  1344,  1237,
    1346,  1857,  1347,  2798,  1348,  1857,  1361,  2024,  1365,  1366,
    1391,  1367,  1660,   933,  1392,  2088,  2089,  1395,  1393,  1396,
    1040,  2024,  1394,   429,  1408,   430,   934,   404,  1397,  1398,
    1403,  2057,  2082,  1406,   431,  1418,  1407,  1422,  1423,  1424,
    1625,  1625,  1432,   434,  1523,  1625,  1896,  1433,  1434,  2386,
    2387,  1427,  1436,  1437,   935,  1438,  1448,  1450,   616,   936,
    1919,   398,   937,  1457,  1111,  1112,  1113,   442,   443,  1470,
    2106,  1454,   469,  1459,   484,  1625,  1625,  1463,  1471,   801,
     503,  1468,   506, -1457, -1457,  1190,  2028,  2029,  2030,  2031,
    2032,  2033,  2136,  1482,  2139,   419,  1487,  2034,   847,  1500,
    1316,  1201,  1514,  1515,  1517,  1531,   726,  1521,  1534,  1207,
     801,   399,  1537,  2025,  2026,  2027,  1554,  2028,  2029,  2030,
    2031,  2032,  2033,  1218,  1506,  1557,   726,  1558,  2034,  1462,
    1568,  1580,  1588,  2154,   801,  1633,  1589,  2025,  2026,  2027,
     802,  2028,  2029,  2030,  2031,  2032,  2033,  1634,   801,   599,
     399,   399,  2034,  1647,  1648,  2028,  2029,  2030,  2031,  2032,
    2033,  1270,  1654,  2165,  1704,   399,  2034,  2554, -1456,  1659,
    1675,  1677,  1711,  1715,  1716,   560,  1723,  2280,  1727,  1728,
    1733,   399,  1549,  1740,  1734,   399,  1742,  1744,   914,  1745,
    1746,  2612,  1756,  1748,  1751,  1753,  1757,   404,  1766,   601,
    1767,  2295,   618,  2622,  1768,   622,   623,  1779,  1778,  1809,
     938,  2302,  1780,  1822,  1840,  2230,  1846,  1849,  1851,   404,
    1246,  1852,  1853,  1860,  1864,   404,  2154,  1872,  1248,  1870,
   -1456,  1875,  1530,  1249,   939,  1876,  1777,  1877,  2023,  1878,
    1879,  1880,  1897,  1891,  1898,  1313,  1462,  1917,  1918,  1928,
    1929,  1930,  1937,  1953,  1956,  1967,   131,   131,  1960,  1961,
    1581,  1982,  1985,  2007,  2240,  2241,   723,  1987,  2002,  2003,
     131, -1456,  2006,  2044,  2008,  1624,  1624,  1523,  2256,  2011,
    1624,  1523,  1544, -1456,  2012,  1523,  2045,  2016, -1456,  2532,
    2019,  2037, -1456,  2043,  2046, -1456,  2047, -1456,  2024,  2054,
    2065,   399,  2060,  2078,  2058,   847,  2666,  2059,  2071,  2117,
    1624,  1624,  2301,   131,  1936,  2081,  2120,  2258,  2094,  2121,
   -1456,  2288,  2124,  2137,  2141,  2150,  2105,  2125,  2142,  2148,
    2161,  2272,  2162,  2164,  2166,  2170,  2188,  1250, -1456,  2171,
    2172,  1251,  2189,  2145,  2198,  2199,   599,  2201,  2322,  2202,
    2222,  2200,  2203,  2224,  2231,  2237,  2233,  1561,  2234,  2238,
    1252,  1253,  2330,  2244,  2239,  2245,  2259,   399,  2257,  2255,
     599,  2261,  2270,  1254,  2262,  2273,  2284,  2275,  2290, -1456,
    2291,   554,  1187,   592,  2600,  2345,  2300, -1456,  2040,  2039,
    2297,  1241,  2362,  2034,  2600,  2331,  2332,  1857,  1857,  2356,
    2357,  2358,  2359,  2333,  1255,  2353,  1857,  1256,   404,  2366,
    2344,  2369,  2370,  2178,  1530,  2371,  2186,  2372,  2378,  2787,
    1257,  1237,  1258,  2647,  2380,   801,  2398,  2401,  2025,  2026,
    2027,  2381,  2028,  2029,  2030,  2031,  2032,  2033,  2402,  2382,
    2383,  1625,  2433,  2034,  1625,  1625,  1625,  1625,  1625,  1625,
    1625,  1625,  1625,   603,  1625,  2399,   604,   605,   606,   500,
     607,   608,   609,   610,   611,   612,  2417,  2248,  2440,   659,
    2418,  2395,  2445,  2447,  2449,  2455,  2442,  2461,  1259,  2462,
   -1456,  2469,   675,  2468,  2463,   676,  2485,  2494,  2384,  2478,
    2488,  2499,  1625,  1625,  2501,   687,  2492,  2497, -1456,  2507,
     691,  2508,  2502,  1462,  1462,  2503,  2505,  2509,  2528,  2663,
    2510,   700,  2529,  2548,  2540,   701,   702,  2544,  2545,  2555,
    2551,  1539,  1540,  2558,  2409, -1456,   399,   399, -1456,  2561,
   -1456, -1456,  2562,  2563,  2564,  2572,  2444,  2573,  2576,  2578,
    2583,   726,  2580,  2581,   399,   399,  2582,  1260,  2589,  2591,
    1261,  1262,  1263,  1567,  1264,  1265,  1266,  1267,  1268,  1269,
    2599,   731,  2603,   732,  2609,  1270,  2610,  2615,  2013,   614,
    2625,  2626,  2633,  2439,  2645,  2638,  2640,  2648,  2649,  2692,
    2683,  2650,  2696,  2687,  2533,  2697,  2713,  2716,  2718,   399,
     399,   399,  2725,   399,  2733,  2541,  2737,  2543,  2741,  2714,
    2715,  1523,  2751,  2760,  2761,  2337,  1523,   404,  1863,  2767,
    1523,  2768,  1901,   404,  1819,  2786,  2801,  1819,  2802,  2459,
     399,  2788,   868,  1893,  2099,  1311,  2457,  1837,  1777,  2158,
    1625,  1818,  2451,  2390,  2119,  2569,  2734,  2174,  2765,  2719,
    1936,  2717,   399,  2747,   589,  2646,  2694,  1530,   589,  2458,
    2493,  1530,  1508,  2546,  1901,  1530,  2490,  2588,  2345,  1882,
    2804,  1702,  2560,  2791,  2559,   599,  2773,  1486,  2179,  2772,
     404,   586,  2805,   502,   727,  1161,  1624,  2701,  2495,  1624,
    1624,  1624,  1624,  1624,  1624,  1624,  1624,  1624,  2385,  1624,
    2127,  1763,  2470,  1043,  2388,   404,  2389,  1761,  2598,  2680,
    2779,  1048,  1721,  2803,   858,  1049,   470,  1720,  1145,  2289,
    2587,  1149,  1857,  2104,  1338,  1092,  1336,  1857,  1857,  1718,
    2536,  2113,  1730,  2090,  1933,  1248,  2605,  1624,  1624,  2267,
    1249,  1873,  2336,  2450,  2185,  2076,  1996,  2074,  1703,  2100,
    1119,  2611,  1116,     0,  2496,     0,     0,     0,     0,     0,
    2424,  2426,   616,   599,   399,   399,     0,     0,  1625,  1625,
       0,     0,     0,     0,   599,   399,   599,     0,  2566,     0,
       0,     0,     0,  1151,     0,     0,     0,  2547,     0,     0,
       0,  1170,     0,  1549,     0,   131,     0,     0,     0,     0,
    2466,     0,     0,  2574,     0,     0,     0,     0,   404,   404,
       0,     0,  2653,     0,  2230,     0,     0,     0,     0,     0,
       0,  1089,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  2345,     0,  1523,     0,     0,     0,     0,
    2618,     0,     0,     0,  1250,     0,     0,     0,  1251,     0,
       0,     0,     0,  1089,     0,     0,   399,     0,     0,     0,
       0,  2596,     0,     0,     0,     0,     0,  1252,  1253,     0,
       0,   399,     0,     0,     0,  1624,  2693,     0,  2702,     0,
    1254,  2490,  1892,     0,     0,  2459,     0,     0,     0,     0,
    1089,     0,   599,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1902,     0,     0,     0,     0,     0,
    1920,  1255,     0,     0,  1256,     0,     0,     0,     0,     0,
       0,   404,  2531,     0,     0,     0,     0,  1257,     0,     0,
     589,     0,     0,     0,     0,     0,  1625,   404,     0,  2345,
       0,     0,     0,     0,     0,   404,  1625,  1625,     0,     0,
       0,     0,     0,     0,  1857,     0,     0,  1857,  1857,   614,
       0,   589,     0,     0,     0,     0,  1777,   589,  1819,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  2771,
       0,  1530,     0,     0,   592,  1259,  1530,     0,   592,   592,
    1530,     0,     0,     0,     0,     0,   399,     0,     0,     0,
       0,     0,     0,  1624,  1624,     0,     0,     0,     0,     0,
       0,     0,  1297,     0,     0,     0,     0,  2797,     0,     0,
       0,     0,     0,     0,     0,  2597,     0,     0,     0,     0,
       0,     0,     0,   399,   138,     0,     0,     0,     0,     0,
       0,     0,     0,  1089,     0,   836,  2750,     0,     0,  2596,
     592,     0,     0,     0,  1260,     0,     0,  1261,  1262,  1263,
       0,  1264,  1265,  1266,  1267,  1268,  1269,     0,     0,  1089,
       0,     0,  1270,     0,     0,     0,   404,   404,   404,   404,
       0,     0,  1089,   139,     0,     0,     0,  2796,     0,     0,
     399,     0,   592,  2087,     0,     0,     0,     0,     0,   140,
       0,     0,     0,  1462,     0,     0,     0,  1857,     0,     0,
       0,  2631,     0,  1401,   131,     0,     0,   141,  2759,  2020,
    2021,     0,     0,     0,  2036,     0,     0,  2795,     0,     0,
    1462,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   616,     0,     0,  2652,   142,   836,  1035,     0,
     131,   143,   589,     0,  2061,  2062,     0,     0,     0,   589,
       0,     0,     0,     0,   144,     0,     0,  1499,  2759,     0,
       0,  1624,     0,     0,     0,     0,  1091,     0,     0,     0,
       0,  1624,  1624,     0,     0,     0,   145,     0,   586,     0,
       0,     0,     0,     0,  2759,     0,     0,  1536,     0,     0,
       0,   589,     0,  1089,     0,  1530,     0,     0,  1091,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1246,     0,
       0,     0,     0,   836,     0,     0,  1248,     0,     0,     0,
       0,  1249,     0,  1089,     0,     0,     0,     0,     0,     0,
     146,     0,     0,     0,     0,  1091,  1246,     0,     0,     0,
       0,     0,     0,     0,  1248,  1080,  1089,     0,     0,  1249,
       0,     0,     0,  2735,  2736,     0,   836,  2739,  2740,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   147,
       0,  2758,   586,     0,   404,   148,     0,  1080,  1089,     0,
     149,     0,  2226,   836,     0,     0,  1089,     0,  2232,     0,
       0,     0,     0,   404,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   150,   151,     0,     0,   404,
     836,   836,     0,   836,  1080,     0,     0,     0,   152,     0,
       0,  2758,     0,   153,     0,  1250,     0,     0,     0,  1251,
       0,   836,   154,     0,     0,  2039,   404,     0,     0,     0,
    2040,     0,     0,     0,     0,  2278,     0,  2758,  1252,  1253,
       0,     0,     0,  1250,     0,   586,     0,  1251,     0,     0,
       0,  1254,     0,     0,     0,     0,   586,     0,     0,     0,
     726,     0,     0,     0,     0,     0,  1252,  1253,     0,     0,
     836,     0,     0,   836,   836,     0,     0,     0,  1091,  1254,
       0,  1035,  1255,     0,     0,  1256,     0,     0,  1736,     0,
       0,     0,     0,     0,     0,     0,  1743,     0,  1257,     0,
    1258,     0,  1747,     0,  1387,     0,     0,     0,  1752,     0,
    1255,  1089,     0,  1256,     0,     0,     0,  1091,     0,     0,
       0,     0,     0,     0,     0,     0,  1257,     0,  1258,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1772,  1773,
       0,  1775,     0,     0,     0,     0,     0,     0,   586,     0,
       0,  1806,  1807,     0,     0,     0,  1259,  1080,     0,     0,
       0,     0,     0,  2360,  2361,  1248,  1821,     0,  1825,  1826,
    1249,  1830,  1831,  1832,  1833,     0,     0,     0,  1839,     0,
       0,  1842,     0,  1080,  1259,     0,   586,     0,     0,     0,
       0,     0,     0,     0,  1248,     0,  1080,     0,     0,  1249,
    2309,     0,     0,  2313,  2314,  2315,  2316,  2317,  2318,  2319,
    2320,  2321,     0,  2323,     0,     0,     0,  1089,     0,     0,
       0,     0,  1089,  1089,     0,  1260,     0,     0,  1261,  1262,
    1263,     0,  1264,  1265,  1266,  1267,  1268,  1269,  1091,     0,
    1446,     0,     0,  1270,     0,     0,     0,  1447,     0,     0,
    1035,  2348,  2349,  1260,     0,     0,  1261,  1262,  1263,     0,
    1264,  1265,  1266,  1267,  1268,  1269,     0,     0,  1091,     0,
       0,  1270,  2063,     0,  1250,     0,  2420,     0,  1251,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1089,
       0,  1091,  2441,     0,     0,  1089,     0,  1252,  1253,     0,
    2446,     0,     0, -1457,     0,     0,     0,  1251,     0,     0,
    1254,  1089,  1782,     0,     0,     0,     0,  1080,     0,     0,
       0,   589,     0,  1091,     0,     0,  1252,  1253,     0,     0,
       0,  1091,     0,     0,     0,     0,     0,     0,     0, -1457,
       0,     0,     0,     0,  1256,     0,     0,  1080,     0,   589,
       0,     0,     0,     0,     0,     0,     0,  1257,     0,     0,
    1783,     0,     0,     0,     0,     0,     0,  1784,  1785,  1786,
    1080,     0,     0,  1256,     0,  1089,     0,     0,     0,     0,
       0,     0,   836,   836,     0,     0,  1257,     0,     0,  2435,
     836,     0,     0,   836,   836,     0,     0,     0,     0,     0,
       0,     0,  1080,     0,  1787,   836,   836,     0,     0,     0,
    1080,     0,  1089,   836,     0,  1259,     0,   836,   836,   836,
     836,     0,     0,     0,     0,     0,     0,   836,   836,     0,
       0,  2524,  2525,  2526,  2527,     0,     0,     0,     0,     0,
       0,  1788,   836,  1035, -1457,  1789,   836,     0,     0,     0,
     836,     0,     0,     0,   836,     0,     0,   836,   836,   836,
     836,   836,   836,   836,   836,   836,  1091,  1248,   836,     0,
       0,     0,  1249,   836,     0,     0,   836,     0,     0,     0,
       0,     0,  1790,     0,  1260,     0,     0,  1261,  1262,  1263,
       0,  1264,  1265,  1266,  1267,  1268,  1269,  1791,  1792,  1793,
    1794,  1795,  1270,  1796,     0,     0,     0,     0,     0,     0,
       0,  2132,  2133,  1260,     0,     0,     0,  2512,  2513,     0,
    1264,  1265,  1266,  1267,  1268,  1269,     0,     0,     0,     0,
    1797,  1270,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1035,     0,     0,  1080,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   836,  1798,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1091,     0,     0,     0,  1250,  1387,  1091,  1081,
    1251,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1799,     0,  1800,     0,     0,     0,     0,     0,  1252,
    1253,  1089,  1089,     0,  1089,     0,     0,     0,     0,     0,
       0,  1081,  1254,     0,     0,  1801,     0,     0,     0,  1089,
       0,  1089,  1802,     0,     0,  1803,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1248,     0,     0,  2635,
       0,  1249,     0,   836,  1091,     0,  1256,     0,  1081,     0,
    1091,  1080,     0,     0,     0,     0,  1080,  1080,   726,  1257,
    1089,     0,  2243,     0,   836,     0,  1091,     0,     0,     0,
       0,     0,     0,     0,  2661,  1083,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  2614,     0,     0,     0,     0,
    1089,     0,     0,  1248,     0,  2619,  2620,     0,  1249,     0,
       0,  2669,     0,     0,     0,     0,     0,  1083,     0,     0,
       0,  1089,     0,     0,     0,     0,     0,  1259,   589,     0,
       0,     0,     0,  1080,     0,     0,     0,   836,     0,  1080,
    1091,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     836,     0,  1089,     0,  1083,  1080,     0,     0,     0,  1251,
       0,     0,     0,  1089,   836,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1089,     0,     0,  1091,  1252,  1253,
       0,  2072,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   836,  1260,     0,     0,     0,
   -1457, -1457,     0,  1264,  1265,  1266,  1267,  1268,  1269,     0,
       0,  1081, -1457,     0,  1270,     0,  1251,     0,     0,  1080,
       0,     0,     0,     0,     0,  1256,     0,   836,   836,     0,
       0,     0,   836,     0,     0,  1252,  1253,  1081,  1257,     0,
       0,     0,     0,     0,   836,   836,   836,   836, -1457,  1089,
    1081,     0,     0,   836,     0,     0,  1080,     0,     0,     0,
       0,     0,   836,   836,     0,   836,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   836,     0,   836,
       0,     0,  1256,   836,     0,  1089,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1257,     0,     0,     0,  1089,
     836,     0,     0,     0,     0,     0,     0,  1083,     0,     0,
     589,     0,   836,   836,     0,  2396,  2397,     0,     0,     0,
       0,     0,  1089,     0,     0,     0,     0,     0,     0,   836,
       0,     0,     0,  1083,     0,     0,     0,   836,   836,   836,
       0,     0,     0,     0,     0,     0,  1083,     0,   836,     0,
       0,     0,     0, -1457,     0,     0,     0,  1084,     0,     0,
    2428,  2430,  2432,     0,  2434,  1260,  1387,  1091,     0,  1091,
       0,  1081,  1264,  1265,  1266,  1267,  1268,  1269,     0,     0,
       0,     0,     0,  1270,  1091,     0,  1091,     0,     0,  1084,
       0,  2460,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1081,     0,     0,     0,     0,     0,     0,  1089,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1260,     0,  1081,  1091,  1084,     0,     0,  1264,
    1265,  1266,  1267,  1268,  1269,     0,     0,     0,     0,     0,
    1270,     0,     0,     0,     0,  1080,  1080,     0,  1080,     0,
       0,     0,     0,     0,     0,  1091,  1081,     0,     0,     0,
       0,     0,     0,  1080,  1081,  1080,     0,  1083,     0,     0,
       0,     0,     0,  1089,     0,  1035,  1091,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1083,     0,     0,
       0,     0,     0,     0,  1080,     0,     0,  1091,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1091,     0,
    1083,     0,     0,     0,     0,  2534,  2535,     0,     0,  1091,
       0,     0,     0,     0,  1080,     0,  2542,  1086,     0,     0,
       0,     0,     0,   589,     0,     0,     0,   589,     0,     0,
       0,   589,  1083,     0,     0,  1080,     0,     0,     0,     0,
    1083,     0,     0,     0,     0,     0,     0,   589,     0,  1086,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1084,
       0,     0,     0,  1248,     0,     0,  1080,     0,  1249,     0,
       0,     0,     0,     0,     0,  1089,     0,  1080,     0,  1081,
       0,   836,     0,     0,  1091,  1084,  1086,     0,  1080,     0,
       0,     0,     0,   836,     0,     0,     0,  2243,  1084,   836,
       0,     0,     0,   836,     0,     0,     0,     0,   836,     0,
       0,   836,   836,   836,   836,   836,   836,   836,   836,   836,
    1091,   836,     0,     0,     0,   836,   836,     0,     0,     0,
       0,     0,     0,     0,  1091,     0,     0,     0,     0,   836,
       0,  2350,     0,     0,     0,     0,   836,     0,     0,   836,
       0,   836,     0,     0,     0,     0,     0,  1091,   836,   836,
     836,     0,     0,  1080,   836,     0,     0,   836,     0,     0,
       0,     0, -1457,     0,     0,  1083,  1251,     0,     0,     0,
       0,     0,     0,     0,  1087,  1081,     0,     0,   836,     0,
    1081,  1081,     0,     0,     0,  1252,  1253,     0,     0,  1080,
       0,     0,     0,     0,     0,     0,     0,     0, -1457,     0,
       0,     0,     0,  1080,     0,     0,  1087,     0,     0,  1084,
       0,  1089,     0,     0,     0,     0,     0,  1088,     0,     0,
       0,     0,     0,     0,     0,     0,  1080,     0,     0,  1086,
    1089,     0,  1256,  1091,     0,     0,     0,  1089,     0,  1084,
       0,     0,     0,  1087,     0,  1257,     0,  1081,     0,  1088,
       0,     0,     0,  1081,     0,  1086,     0,     0,     0,     0,
       0,     0,  1084,     0,  2691,     0,     0,     0,  1086,  1081,
       0,  1083,     0,     0,     0,     0,  1083,  1083,     0,     0,
       0,     0,     0,     0,     0,     0,  1088,   836,     0,     0,
       0,     0,     0,     0,  1084,     0,     0,     0,  1091,     0,
       0,     0,  1084, -1457,     0,     0,     0,     0,     0,     0,
       0,     0,  1080,     0,     0,     0,     0,     0,     0,     0,
       0,  2730,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1081,     0,     0,  1089,   589,     0,     0,
       0,     0,   589,  1083,     0,     0,   589,     0,     0,  1083,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   836,   636,     0,  1083,     0,     0,     0,     0,
    1081,     0,  1260,     0,   836,     0,     0,  1080,     0,  1264,
    1265,  1266,  1267,  1268,  1269,     0,     0,     0,     0,  1086,
    1270,     0,     0,   637,     0,     0,  1087,     0,     0,     0,
       0,     0,     0,     0,     0,   638,     0,     0,     0,     0,
     639,     0,   640,     0,     0,   836,   836,     0,     0,  1086,
    1091,     0,  1087,     0,     0,     0,   836,   836,   641,  1083,
     836,     0,     0,     0,     0,  1087,     0,  1084,     0,  1088,
       0,     0,  1086,  1089,     0,     0,   642,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1090,     0,     0,     0,
       0,     0,     0,     0,     0,  1088,  1083,     0,     0,     0,
       0,     0,     0,     0,  1086,   643,     0,     0,  1088,     0,
       0,     0,  1086,     0,     0,     0,     0,     0,  1090,     0,
       0,     0,     0,   644,     0,     0,     0,     0,     0,  1080,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   836,
    1089,     0,     0,     0,     0,     0,   645,     0,     0,  1246,
       0,     0,     0,     0,     0,  1090,     0,  1248,     0,     0,
       0,     0,  1249,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1084,   589,     0,     0,     0,  1084,  1084,
       0,     0,     0,     0,     0,     0,  1087,   836,     0,  1081,
    1081,   589,  1081,     0,     0,     0,  1091,     0,     0,   646,
       0,     0,     0,   836,     0,     0,     0,  1081,     0,  1081,
       0,     0,     0,   836,     0,  1091,  1087,     0,     0,   455,
       0,     0,  1091,   836,   836,     0,     0,   836,     0,  1088,
       0,     0,     0,     0,   836,   836,     0,     0,     0,  1087,
       0,     0,   647,     0,   648,  1084,     0,  1086,  1081,   649,
       0,  1084,     0,     0,   836,     0,     0,     0,     0,  1088,
       0,     0,     0,     0,     0,     0,  1250,  1084,   589,     0,
    1251,  1087,     0,     0,   650,  1080,     0,     0,  1081,  1087,
       0,     0,  1088,     0,     0,  1083,  1083,   651,  1083,  1252,
    1253,     0,   652,     0,  1080,     0,     0,     0,   836,  1081,
       0,  1080,  1254,  1083,     0,  1083,     0,     0,  1090,     0,
     653,     0,     0,     0,  1088,     0,     0,     0,     0,     0,
       0,  1091,  1088,     0,     0,     0,     0,     0,     0,     0,
    1081,  1084,     0,  1255,  1385,     0,  1256,     0,     0,     0,
       0,  1081,     0,     0,  1083,     0,     0,  1090,     0,  1257,
       0,   589,  1081,  1086,     0,     0,     0,     0,  1086,  1086,
       0,     0,   589,     0,     0,     0,     0,     0,  1084,     0,
       0,     0,     0,     0,  1083,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1083,     0,  1246,     0,     0,
    1080,  1724,     0,     0,     0,  1248,     0,  1259,     0,     0,
    1249,     0,     0,     0,  1087,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1086,  1083,  1081,  1091,   821,
       0,  1086,     0,     0,  1248,     0,     0,  1083,     0,  1249,
       0,     0,     0,     0,     0,     0,     0,  1086,  1083,     0,
       0,     0,     0,     0,   589,     0,     0,  1088,     0,     0,
       0,     0,     0,  1081,     0,     0,     0,     0,  1090,     0,
    1725,     0,     0,     0,     0,     0,  1260,  1081,     0,  1261,
    1262,  1263,     0,  1264,  1265,  1266,  1267,  1268,  1269,     0,
       0,     0,   589,     0,  1270,  1091,     0,     0,  1090,     0,
    1081,     0,     0,     0,     0,     0,     0,  1080,     0,     0,
       0,  1086,  2351,     0,  1250,     0,     0,     0,  1251,     0,
    1087,  1090,     0,  1083,     0,  1087,  1087,     0,     0,     0,
       0,   957,     0,     0,     0,     0,     0,  1252,  1253,     0,
       0,     0,     0, -1457,     0,     0,     0,  1251,  1086,     0,
    1254,     0,     0,  1090,     0,     0,     0,  1084,  1084,  1083,
    1084,  1090,     0,  1088,     0,     0,  1252,  1253,  1088,  1088,
       0,     0,     0,  1083,  1080,  1084,     0,  1084,     0, -1457,
       0,  1255,     0,     0,  1256,     0,  1081,     0,     0,     0,
       0,     0,  1087,     0,     0,     0,  1083,  1257,  1087,  1258,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1256,  1087,     0,  1084,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1257,     0,     0,     0,
       0,     0,     0,     0,     0,  1088,     0,     0,     0,     0,
       0,  1088,     0,     0,     0,     0,  1084,     0,     0,     0,
    1192,  1081,     0,     0,     0,  1259,     0,  1088,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1084,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1216,  1087,     0,
       0,     0,  1083,     0, -1457,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1090,     0,  1084,     0,
    1726,     0,     0,     0,  1235,  1236,     0,  1238,     0,  1084,
       0,     0,     0,     0,     0,  1087,     0,     0,     0,     0,
    1084,  1088,     0,     0,  1260,  1275,     0,  1261,  1262,  1263,
       0,  1264,  1265,  1266,  1267,  1268,  1269,  1086,  1086,     0,
    1086,  1246,  1270,     0,  1247,     0,     0,  1083,     0,  1248,
       0,     0,     0,  1260,  1249,  1086,     0,  1086,  1088,     0,
    1264,  1265,  1266,  1267,  1268,  1269,     0,     0,     0,     0,
       0,  1270,     0,  1081,  1326,     0,     0,  1333,  1335,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1084,  1086,     0,     0,  1248,
       0,     0,  1758,     0,  1249,     0,     0,  1385,  1385,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1086,     0,     0,     0,
       0,  1084,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1084,     0,  1086,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1250,     0,
       0,     0,  1251,     0,     0,     0,     0,     0,  1084,  1083,
       0,     0,     0,     0,  1090,     0,     0,  2354,  1086,     0,
    1090,  1252,  1253,     0,     0,     0,     0,     0,     0,  1086,
       0,     0,     0,     0,  1254,     0,  1090,     0,     0,     0,
    1086,     0,     0,     0,  1087,  1087,     0,  1087, -1457,     0,
       0,     0,  1251,     0,     0,     0,     0,     0,     0,  1081,
       0,     0,  1087,     0,  1087,  1255,     0,     0,  1256,     0,
       0,  1252,  1253,     0,     0,     0,     0,     0,  1081,     0,
       0,  1257,     0,  1258, -1457,  1081,     0,  1088,  1088,     0,
    1088,     0,     0,     0,  1084,     0,     0,     0,     0,     0,
    1090,     0,     0,  1087,     0,  1088,     0,  1088,     0,     0,
       0,     0,     0,     0,     0,  1086,     0,     0,  1256,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1257,     0,  1087,     0,     0,     0,  1090,     0,  1259,
       0,     0,     0,     0,     0,     0,  1088,     0,     0,     0,
       0,  1086,  1272,     0,  1087,  1083,     0,     0,     0,  1084,
       0,     0,     0,     0,     0,  1086,     0,     0,     0,     0,
       0,     0,     0,     0,  1083,     0,  1088,     0,     0,     0,
       0,  1083,     0,     0,  1081,  1087,     0,     0,  1086, -1457,
       0,     0,     0,     0,     0,     0,  1087,  1088,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1087,  1260,     0,
       0,  1261,  1262,  1263,     0,  1264,  1265,  1266,  1267,  1268,
    1269,     0,     0,     0,     0,     0,  1270,     0,  1088,     0,
       0,     0,     0,     0,  1592,     0,     0,     0,  1594,  1088,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1088,     0,     0,     0,     0,     0,     0,  1616,  1260,     0,
       0,  1619,     0,     0,  1630,  1264,  1265,  1266,  1267,  1268,
    1269,  1637,  1642,     0,  1086,     0,  1270,     0,     0,     0,
    1083,  1084,  1087,     0,     0,     0,  1652,     0,  1272,     0,
    1658,  1081,     0,     0,  1669,     0,     0,     0,  1676,     0,
       0,  1678,  1679,  1680,  1681,  1682,  1683,  1684,  1685,  1686,
       0,     0,  1688,     0,     0,     0,     0,   821,  1087,     0,
       0,     0,     0,     0,     0,  1088,  1385,  1385,     0,  1385,
       0,     0,  1087,     0,     0,     0,     0,     0,     0,  1086,
       0,     0,     0,     0,  1090,     0,  1090,     0,     0,     0,
       0,     0,     0,     0,     0,  1087,     0,     0,  1081,  1246,
       0,  1088,     0,     0,     0,     0,     0,  1248,     0,     0,
       0,     0,  1249,     0,     0,  1088,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1090,     0,  1083,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1088,     0,
       0,     0,  1750,     0,     0,     0,     0,     0,     0,     0,
    1246,     0,     0,  1998,     0,  1090,     0,     0,  1248,     0,
       0,     0,     0,  1249,     0,     0,     0,  1084,     0,     0,
       0,     0,     0,     0,     0,     0,  1385,     0,     0,     0,
       0,  1087,     0,     0,     0,     0,  1084,     0,     0,     0,
       0,     0,     0,  1084,  1083,     0,     0,     0,     0,     0,
       0,  1086,     0,     0,     0,     0,     0,  1090,     0,     0,
       0,     0,  1272,     0,     0,     0,  1250,     0,  1090,     0,
    1251,     0,     0,     0,  1088,     0,     0,  1847,     0,  1385,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1252,
    1253,     0,     0,     0,     0,     0,  1087,     0,  1871,     0,
       0,     0,  1254,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1246,  1272,     0,     0,     0,  1250,     0,     0,
    1248,  1251,     0,     0,     0,  1249,     0,     0,     0,     0,
       0,     0,     0,  1255,     0,     0,  1256,  1272,     0,  1088,
    1252,  1253,  1084,  1626,     0,     0,     0,     0,     0,  1257,
       0,  1258,     0,  1254,  1090,     0,  1272,  1272,     0,  1272,
       0,  1952,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1959,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1255,     0,     0,  1256,  1970,     0,
    1090,     0,     0,     0,     0,     0,  1272,  1086,     0,     0,
    1257,     0,  1258,     0,  1090,     0,     0,  1259,     0,     0,
       0,     0,     0,     0,     0,     0,  1086,     0,  1087,  1994,
       0,     0,     0,  1086,     0,     0,     0,  1090,     0,  1250,
       0,     0,     0,  1251,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1272,     0,  1084,
       0,     0,  1252,  1253,  1272,     0,  1272,     0,  1259,     0,
       0,  1088,     0,     0,     0,  1254,     0,     0,  1642,     0,
    1642,  1642,     0,     0,     0,     0,  1260,  2056,     0,  1261,
    1262,  1263,     0,  1264,  1265,  1266,  1267,  1268,  1269,     0,
       0,     0,     0,  1646,  1270,     0,  1255,     0,     0,  1256,
       0,  2075,     0,  2077,     0,     0,     0,  2080,     0,     0,
       0,     0,  1257,  1090,  1258,     0,  1084,     0,     0,     0,
       0,     0,  1086,     0,     0,     0,     0,  1260,     0,     0,
    1261,  1262,  1263,     0,  1264,  1265,  1266,  1267,  1268,  1269,
       0,     0,     0,     0,     0,  1270,     0,     0,     0,     0,
       0,     0,     0,  1326,     0,     0,     0,     0,     0,     0,
       0,  1335,  1335,  1333,  1087,     0,     0,     0,     0,     0,
    1259,     0,  2118,     0,     0,     0,     0,     0,  1090,     0,
       0,     0,     0,  1087,     0,     0,     0,     0,  1248,     0,
    1087,     0,     0,  1249,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1088,     0,     0,
       0,     0,     0,     0,     0,  2000,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1088,     0,     0,  1086,
       0,     0,     0,  1088,     0,     0,     0,     0,     0,  1260,
       0,     0,  1261,  1262,  1263,     0,  1264,  1265,  1266,  1267,
    1268,  1269,     0,     0,     0,     0,     0,  1270,     0,     0,
       0,     0,     0,     0,     0,     0,  2521,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1087,
       0,     0,     0,     0,     0,     0,  1086, -1457,     0,     0,
    1090,  1251,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1272,     0,  1272,     0,     0,     0,     0,
    1252,  1253,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1088, -1457,     0,     0,     0,  1272,     0,     0,
    1272,     0,  1626,  1626,  2035,     0,     0,  1626,     0,     0,
       0,  1272,     0,     0,     0,     0,     0,     0,  1272,     0,
       0,     0,     0,  1272,     0,     0,     0,  1256,     0,     0,
       0,     0,     0,  1272,     0,     0,     0,  1626,  1626,  1272,
    1257,     0,     0,     1,     0,     0,     0,     0,     0,     0,
    1272,     0,     0,     2,     3,     4,  1087,  1272,     0,  1272,
    1272,  1272,  1272,  1272,  1272,  1272,  1272,  1272,     5,  1272,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  2299,     6,     0,     7,     8,
       0,     0,     0,     9,    10,     0,     0,  2304, -1457,  1088,
       0,    11,    12,  2305,     0,     0,  1090,  2307,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,     0,     0,
      14,     0,     0,  1087,     0,  1090,    15,     0,     0,  2324,
    2325,  1272,  1090,     0,     0,    16,     0,     0,     0,     0,
       0,    17,     0,     0,     0,     0,    18,     0,    19,     0,
    2338,     0,    20,  2341,     0,  2343,     0,     0,     0,     0,
       0,     0,  2347,     0,    21,     0,  1088,  1260,  2352,     0,
       0,  2355,     0,     0,  1264,  1265,  1266,  1267,  1268,  1269,
       0,  1246,     0,     0,     0,  1270,     0,     0,    22,  1248,
       0,     0,  2368,     0,  1249,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    23,    24,     0,     0,     0,     0,    25,     0,     0,
       0,     0,     0,     0,     0,    26,     0,     0,  1272,     0,
       0,  1090,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    27,     0,     0,  1246,     0,     0,  2048,     0,
       0,     0,  1272,  1248,     0,     0,     0,     0,  1249,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    29,     0,    30,     0,     0,     0,    31,
       0,     0,     0,    32,     0,     0,    33,     0,  1250,     0,
      34,     0,  1251,     0,     0,    35,     0,     0,     0,     0,
      36,     0,     0,    37,     0,     0,     0,     0,     0,    38,
       0,  1252,  1253,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1272,  1254,     0,     0,     0,  1090,     0,
    1272,     0,     0,     0,     0,     0,     0,    39,     0,     0,
       0,  1272,     0,     0,     0,    40,     0,    41,     0,     0,
      42,     0,  1250,     0,     0,  1255,  1251,     0,  1256,     0,
       0,     0,     0,     0,     0,  1272,  1970,     0,     0,     0,
       0,  1257,     0,  1258,     0,  1252,  1253,     0,  2500,     0,
       0,     0,  2017,     0,     0,     0,     0,     0,  1254,    43,
       0,  2035,  2035,  1626,     0,  1090,  1626,  1626,  1626,  1626,
    1626,  1626,  1626,  1626,  1626,     0,  1626,  2035,     0,     0,
    1246,     0,     0,     0,     0,     0,     0,     0,  1248,  1255,
       0,     0,  1256,  1249,     0,     0,     0,  1272,     0,  1259,
    2519,  2520,  2035,  2035,  2522,  1257,     0,  1258,     0,     0,
       0,     0,     0,     0,  1626,  1626,  1272,  1246,  1272,     0,
       0,  1272,     0,     0,     0,  1248,     0,     0,     0,     0,
    1249,     0,     0,     0,  1246,     0,     0,     0,     0,     0,
       0,     0,  1248,     0,     0,     0,     0,  1249,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1272,
       0,     0,     0,  1259,     0,     0,     0,     0,  1260,     0,
       0,  1261,  1262,  1263,     0,  1264,  1265,  1266,  1267,  1268,
    1269,     0,     0,  2567,  2053,     0,  1270,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1250,     0,     0,
       0,  1251,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1252,  1253,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  2602,  1260,  1254,  1250,  1261,  1262,  1263,  1251,  1264,
    1265,  1266,  1267,  1268,  1269,     0,     0,  1970,     0,     0,
    1270,  1250,  1626,     0,     0,  1251,     0,  1252,  1253,     0,
       0,     0,     0,     0,  1255,     0,     0,  1256,     0,     0,
    1254,  2621,     0,     0,  1252,  1253,     0,     0,     0,  2623,
    1257,     0,  1258,     0,     0,     0,     0,  1254,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  2628,     0,
       0,  1255,     0,     0,  1256,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1257,  1255,  1258,
       0,  1256,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1257,     0,  1258,     0,  1259,     0,
    1272,     0,  2654,     0,     0,  1272,  1272,     0,  1272,     0,
    2035,     0,     0,     0,  2035,  2035,  2035,  2035,  2035,  2035,
    2035,  2035,  2035,     0,  2035,  1272,  1272,     0,     0,     0,
       0,     0,     0,     0,     0,  1259,     0,     0,     0,  1272,
    1626,  1626,  1272,     0,  1272,     0,     0,     0,  1272,  2035,
    2035,     0,  1259,  1272,     0,     0,  1272,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1260,     0,  1272,
    1261,  1262,  1263,     0,  1264,  1265,  1266,  1267,  1268,  1269,
       0,     0,     0,     0,     0,  1270,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1260,     0,     0,  1261,  1262,  1263,
       0,  1264,  1265,  1266,  1267,  1268,  1269,     0,     0,  2163,
       0,  1260,  1270,     0,  1261,  1262,  1263,     0,  1264,  1265,
    1266,  1267,  1268,  1269,     0,     0,  2035,     0,  2187,  1270,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1272,     0,     0,     0,     0,     0,     0,  1626,     0,
       0,     0,     0,  2035,  2035,     0,     0,     0,  1626,  1626,
    1272,  1272,     0,  1272,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1272,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1272,     0,     0,     0,   156,   157,   158,
     159,   160,   161,   162,   163,  2035,   164,   165,     0,     0,
    2035,  2035,  1272,     0,  1272,   166,   167,     0,   168,  1272,
     169,   170,   171,     0,   172,     0,   173,   174,     0,   175,
     176,   177,   178,     0,     0,   179,   180,   181,   182,     0,
     183,   184,   185,   186,   187,  1272,     0,   188,   189,   190,
     191,     0,   192,   193,   194,   195,     0,   196,   197,   198,
       0,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,   205,   206,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   216,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,     0,
     233,     0,   234,     0,     0,     0,   235,   236,     0,   237,
     238,     0,   239,     0,   240,   241,   242,     0,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,     0,   251,
     252,   253,   254,   255,   256,   257,     0,   258,     0,   259,
       0,     0,   260,     0,   261,   262,   263,   264,   265,     0,
     266,     0,   267,     0,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,     0,   283,     0,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,   293,   294,     0,   295,     0,
     296,   297,   298,   299,     0,     0,   300,     0,     0,     0,
     301,   302,     0,     0,   303,     0,     0,   304,   305,   306,
     307,     0,   308,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,     0,   345,   346,   347,   348,     0,   349,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   367,   368,     0,   369,     0,   370,   371,   372,   373,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,  2657,   383,   384,   385,   386,   387,   388,
       0,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   397,     0,     0,     0,     0,     0,     0,   156,
     157,   158,   159,   160,   161,   162,   163,  1706,   164,   165,
       0, -1234,     0,     0,   746,     0, -1234,   166,   167,     0,
     168,   565,   169,   170,   171,   566,   747,   567,   748,   749,
       0,   175,   176,   177,   178,   750,   751,   179,   752,   753,
     182,     0,   183,   184,   185,   186,   754,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   755,   195,     0,   196,
     197,   198,   568,   199,   756,   757,   758,   759,   760,   200,
     201,   202,   203,   204,   761,   762,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,  1707,     0,   215,
     763,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   764,   227,   228,   765,   766,   230,   231,
     767,     0,   233,     0,   234,   569,     0,   570,   235,   236,
       0,   237,   768,     0,   239,     0,   240,   241,   242,   571,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
     572,   251,   252,   253,   254,   255,   769,   770,     0,   771,
       0,   259,   573,   574,   260,   575,   261,   262,   263,   264,
     265,     0,   772,   576,   267,   577,     0,   268,   269,   270,
     773,   774,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   775,   578,   776,   402,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   777,   293,   294,   579,
     295,   778,   779,   780,   298,   299,     0,     0,   300,   403,
       0,     0,   781,   302,     0,     0,   303,   580,   581,   782,
     305,   306,   307,     0,   783,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   784,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     582,   332,   333,   785,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,   786,   345,   346,   347,   348,
     583,   349,   787,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   788,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   789,   790,     0,   369,     0,   370,   791,
     372,   792,   793,   374,   375,   376,   377,   378,     0,   794,
     379,   380,   381,   382,   795,     0,   383,   384,   385,   386,
     796,   388,   584,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   797,   798,   497,   799,   800,   801,
     802,   803,     0,     0,     0,     0,   804,   805,  1708,     0,
       0,     0,     0,     0,     0,   807,  1709,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,   746,     0,     0,   166,   167,     0,   168,   565,
     169,   170,   171,   566,   747,   567,   748,   749,     0,   175,
     176,   177,   178,   750,   751,   179,   752,   753,   182,     0,
     183,   184,   185,   186,   754,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   755,   195,     0,   196,   197,   198,
     568,   199,   756,   757,   758,   759,   760,   200,   201,   202,
     203,   204,   761,   762,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   763,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   764,   227,   228,   765,   766,   230,   231,   767,     0,
     233,     0,   234,   569,     0,   570,   235,   236,     0,   237,
     768,     0,   239,     0,   240,   241,   242,   571,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,   572,   251,
     252,   253,   254,   255,   769,   770,     0,   771,     0,   259,
     573,   574,   260,   575,   261,   262,   263,   264,   265,     0,
     772,   576,   267,   577,     0,   268,   269,   270,   773,   774,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   775,   578,   776,   402,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   777,   293,   294,   579,   295,   778,
     779,   780,   298,   299,     0,     0,   300,   403,     0,     0,
     781,   302,     0,     0,   303,   580,   581,   782,   305,   306,
     307,     0,   783,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   784,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   582,   332,
     333,   785,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,   786,   345,   346,   347,   348,   583,   349,
     787,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     788,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   789,   790,     0,   369,     0,   370,   791,   372,   792,
     793,   374,   375,   376,   377,   378,     0,   794,   379,   380,
     381,   382,   795,     0,   383,   384,   385,   386,   796,   388,
     584,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   797,   798,   497,   799,   800,   801,   802,   803,
       0,     0,     0,     0,   804,   805,     0,     0,     0,     0,
       0,     0,     0,   807,  1628,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,  1934,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,  -836,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,  -836,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,  -836,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,  -836,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,  -836,   300,     0,     0,     0,   301,   302,
       0,  -836,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,  -836,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,     0,   156,   157,   158,   159,   160,   161,   162,   163,
       0,   164,   165,     0,     0,     0,     0,     0,     0,     0,
     166,   167,   866,   168,   565,   169,   170,   171,   566,  1057,
     567,  1058,  1059,     0,   175,   176,   177,   178,     0,     0,
     179,  1060,  1061,   182,     0,   183,   184,   185,   186,     0,
       0,     0,   188,   189,   190,   191,     0,   192,   193,     0,
     195,     0,   196,   197,   198,   568,   199,     0,     0,     0,
       0,     0,   200,   201,   202,   203,   204,  1062,  1063,   207,
       0,   208,     0,   209,   210,   211,   212,   213,     0,   214,
       0,     0,   215,   763,   217,   218,     0,   219,   220,   221,
       0,   222,     0,   223,   224,   225,     0,   227,   228,     0,
       0,   230,   231,  1064,     0,   233,     0,   234,   569,     0,
     570,   235,   236,     0,   237,     0,     0,   239,     0,   240,
     241,   242,   571,   243,   244,   245,  1373,   246,   247,   248,
     249,   250,     0,   572,  1374,   252,   253,   254,   255,  1065,
    1066,     0,  1067,     0,   259,   573,   574,   260,   575,   261,
     262,   263,   264,   265,     0,     0,   576,   267,   577,     0,
     268,   269,   270,     0,     0,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,  1068,   578,  1069,     0,
     284,   285,   286,   287,   288,   289,   290,     0,   292,     0,
     293,   294,   579,   295,     0,     0,  1070,   298,   299,     0,
       0,   300,     0,     0,     0,   301,   302,     0,     0,  1375,
     580,   581,     0,   305,   306,   307,     0,     0,     0,   310,
     311,   312,     0,   313,   314,   315,   316,   317,   318,  1071,
     320,     0,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   582,   332,   333,     0,   335,   336,   337,
     338,   339,   340,   341,     0,   342,   343,   344,     0,   345,
    1376,   347,   348,   583,   349,  1073,     0,   351,   352,   353,
     354,   355,   356,   357,   358,     0,   360,     0,   361,   362,
       0,   363,   364,   365,   366,     0,  1074,  1075,     0,   369,
       0,   370,     0,   372,     0,     0,   374,   375,   376,   377,
     378,     0,     0,   379,   380,   381,   382,     0,     0,   383,
     384,   385,   386,  1076,   388,   584,   389,   390,     0,     0,
     391,   392,   393,   394,   395,   396,     0,  1377,     0,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,   746,     0,     0,   166,   167,  1378,
     168,   565,   169,   170,   171,   566,   747,   567,   748,   749,
    1638,   175,   176,   177,   178,   750,   751,   179,   752,   753,
     182,     0,   183,   184,   185,   186,   754,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   755,   195,     0,   196,
     197,   198,   568,   199,   756,   757,   758,   759,   760,   200,
     201,   202,   203,   204,   761,   762,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     763,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   764,   227,   228,   765,   766,   230,   231,
     767,     0,   233,     0,   234,   569,  1639,   570,   235,   236,
       0,   237,   768,     0,   239,     0,   240,   241,   242,   571,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
     572,   251,   252,   253,   254,   255,   769,   770,     0,   771,
       0,   259,   573,   574,   260,   575,   261,   262,   263,   264,
     265,  1640,   772,   576,   267,   577,     0,   268,   269,   270,
     773,   774,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   775,   578,   776,   402,   284,   285,   286,
     287,   288,   289,   290,   291,   292,   777,   293,   294,   579,
     295,   778,   779,   780,   298,   299,     0,     0,   300,   403,
       0,     0,   781,   302,     0,     0,   303,   580,   581,   782,
     305,   306,   307,     0,   783,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   784,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     582,   332,   333,   785,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,   786,   345,   346,   347,   348,
     583,   349,   787,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   788,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   789,   790,     0,   369,  1641,   370,   791,
     372,   792,   793,   374,   375,   376,   377,   378,     0,   794,
     379,   380,   381,   382,   795,     0,   383,   384,   385,   386,
     796,   388,   584,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   797,   798,   497,   799,   800,   801,
     802,   803,     0,     0,     0,     0,   804,   805,     0,     0,
       0,     0,     0,     0,     0,   807,   156,   157,   158,   159,
     160,   161,   162,   163,     0,   164,   165,     0,     0,     0,
       0,   746,     0,     0,   166,   167,     0,   168,   565,   169,
     170,   171,   566,   747,   567,   748,   749,     0,   175,   176,
     177,   178,   750,   751,   179,   752,   753,   182,     0,   183,
     184,   185,   186,   754,     0,     0,   188,   189,   190,   191,
       0,   192,   193,   755,   195,     0,   196,   197,   198,   568,
     199,   756,   757,   758,   759,   760,   200,   201,   202,   203,
     204,   761,   762,   207,     0,   208,     0,   209,   210,   211,
     212,   213,     0,   214,     0,     0,   215,   763,   217,   218,
       0,   219,   220,   221,     0,   222,     0,   223,   224,   225,
     764,   227,   228,   765,   766,   230,   231,   767,     0,   233,
       0,   234,   569,     0,   570,   235,   236,     0,   237,   768,
       0,   239,     0,   240,   241,   242,   571,   243,   244,   245,
       0,   246,   247,   248,   249,   250,     0,   572,   251,   252,
     253,   254,   255,   769,   770,     0,   771,     0,   259,   573,
     574,   260,   575,   261,   262,   263,   264,   265,     0,   772,
     576,   267,   577,     0,   268,   269,   270,   773,   774,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     775,   578,   776,   402,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   777,   293,   294,   579,   295,   778,   779,
     780,   298,   299,     0,     0,   300,   403,     0,     0,   781,
     302,     0,     0,   303,   580,   581,   782,   305,   306,   307,
       0,   783,   309,   310,   311,   312,     0,   313,   314,   315,
     316,   317,   318,   784,   320,     0,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   582,   332,   333,
     785,   335,   336,   337,   338,   339,   340,   341,     0,   342,
     343,   344,   786,   345,   346,   347,   348,   583,   349,   787,
       0,   351,   352,   353,   354,   355,   356,   357,   358,   788,
     360,     0,   361,   362,     0,   363,   364,   365,   366,     0,
     789,   790,     0,   369,     0,   370,   791,   372,   792,   793,
     374,   375,   376,   377,   378,     0,   794,   379,   380,   381,
     382,   795,     0,   383,   384,   385,   386,   796,   388,   584,
     389,   390,     0,     0,   391,   392,   393,   394,   395,   396,
       0,   797,   798,   497,   799,   800,   801,   802,   803,     0,
       0,     0,     0,   804,   805,   806,     0,     0,     0,     0,
       0,     0,   807,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,   746,     0,
       0,   166,   167,     0,   168,   565,   169,   170,   171,   566,
     747,   567,   748,   749,     0,   175,   176,   177,   178,   750,
     751,   179,   752,   753,   182,     0,   183,   184,   185,   186,
     754,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     755,   195,     0,   196,   197,   198,   568,   199,   756,   757,
     758,   759,   760,   200,   201,   202,   203,   204,   761,   762,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   763,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   764,   227,   228,
     765,   766,   230,   231,   767,     0,   233,     0,   234,   569,
       0,   570,   235,   236,     0,   237,   768,     0,   239,     0,
     240,   241,   242,   571,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,   572,   251,   252,   253,   254,   255,
     769,   770,     0,   771,     0,   259,   573,   574,   260,   575,
     261,   262,   263,   264,   265,     0,   772,   576,   267,   577,
       0,   268,   269,   270,   773,   774,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   775,   578,   776,
     402,   284,   285,   286,   287,   288,   289,   290,   291,   292,
     777,   293,   294,   579,   295,   778,   779,   780,   298,   299,
       0,     0,   300,   403,     0,     0,   781,   302,     0,     0,
     303,   580,   581,   782,   305,   306,   307,     0,   783,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     784,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   582,   332,   333,   785,   335,   336,
     337,   338,   339,   340,   341,    35,   342,   343,   344,   786,
     345,   346,   347,   348,   583,   349,   787,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   788,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   789,   790,     0,
     369,     0,   370,   791,   372,   792,   793,   374,   375,   376,
     377,   378,     0,   794,   379,   380,   381,   382,   795,     0,
     383,   384,   385,   386,   796,   388,   584,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   797,   798,
     497,   799,   800,   801,   802,   803,     0,     0,     0,     0,
     804,   805,     0,     0,     0,     0,     0,     0,     0,   807,
     156,   157,   158,   159,   160,   161,   162,   163,  1331,   164,
     165,     0,     0,     0,     0,   746,     0,     0,   166,   167,
       0,   168,   565,   169,   170,   171,   566,   747,   567,   748,
     749,     0,   175,   176,   177,   178,   750,   751,   179,   752,
     753,   182,     0,   183,   184,   185,   186,   754,     0,     0,
     188,   189,   190,   191,     0,   192,   193,   755,   195,     0,
     196,   197,   198,   568,   199,   756,   757,   758,   759,   760,
     200,   201,   202,   203,   204,   761,   762,   207,     0,   208,
       0,   209,   210,   211,   212,   213,     0,   214,     0,     0,
     215,   763,   217,   218,     0,   219,   220,   221,     0,   222,
       0,   223,   224,   225,   764,   227,   228,   765,   766,   230,
     231,   767,     0,   233,     0,   234,   569,     0,   570,   235,
     236,     0,   237,   768,     0,   239,     0,   240,   241,   242,
     571,   243,   244,   245,     0,   246,   247,   248,   249,   250,
       0,   572,   251,   252,   253,   254,   255,   769,   770,     0,
     771,     0,   259,   573,   574,   260,   575,   261,   262,   263,
     264,   265,     0,   772,   576,   267,   577,     0,   268,   269,
     270,   773,   774,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   775,   578,   776,   402,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   777,   293,   294,
     579,   295,   778,   779,   780,   298,   299,     0,     0,   300,
     403,     0,     0,   781,   302,     0,     0,   303,   580,   581,
     782,   305,   306,   307,     0,   783,   309,   310,   311,   312,
       0,   313,   314,   315,   316,   317,   318,   784,   320,     0,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   582,   332,   333,   785,   335,   336,   337,   338,   339,
     340,   341,     0,   342,   343,   344,   786,   345,   346,   347,
     348,   583,   349,   787,     0,   351,   352,   353,   354,   355,
     356,   357,   358,   788,   360,     0,   361,   362,     0,   363,
     364,   365,   366,     0,   789,   790,     0,   369,     0,   370,
     791,   372,   792,   793,   374,   375,   376,   377,   378,     0,
     794,   379,   380,   381,   382,   795,     0,   383,   384,   385,
     386,   796,   388,   584,   389,   390,     0,     0,   391,   392,
     393,   394,   395,   396,     0,   797,   798,   497,   799,   800,
     801,   802,   803,     0,     0,     0,     0,   804,   805,     0,
       0,     0,     0,     0,     0,     0,   807,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,   746,     0,     0,   166,   167,     0,   168,   565,
     169,   170,   171,   566,   747,   567,   748,   749,     0,   175,
     176,   177,   178,   750,   751,   179,   752,   753,   182,     0,
     183,   184,   185,   186,   754,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   755,   195,     0,   196,   197,   198,
     568,   199,   756,   757,   758,   759,   760,   200,   201,   202,
     203,   204,   761,   762,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   763,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   764,   227,   228,   765,   766,   230,   231,   767,     0,
     233,     0,   234,   569,     0,   570,   235,   236,     0,   237,
     768,     0,   239,     0,   240,   241,   242,   571,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,   572,   251,
     252,   253,   254,   255,   769,   770,     0,   771,     0,   259,
     573,   574,   260,   575,   261,   262,   263,   264,   265,     0,
     772,   576,   267,   577,     0,   268,   269,   270,   773,   774,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   775,   578,   776,   402,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   777,   293,   294,   579,   295,   778,
     779,   780,   298,   299,     0,     0,   300,   403,     0,     0,
     781,   302,     0,     0,   303,   580,   581,   782,   305,   306,
     307,     0,   783,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   784,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   582,   332,
     333,   785,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,   786,   345,   346,   347,   348,   583,   349,
     787,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     788,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   789,   790,     0,   369,     0,   370,   791,   372,   792,
     793,   374,   375,   376,   377,   378,     0,   794,   379,   380,
     381,   382,   795,     0,   383,   384,   385,   386,   796,   388,
     584,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   797,   798,   497,   799,   800,   801,   802,   803,
       0,     0,     0,     0,   804,   805,     0,     0,     0,     0,
       0,  1187,     0,   807,   156,   157,   158,   159,   160,   161,
     162,   163,     0,   164,   165,     0,     0,     0,     0,   746,
       0,     0,   166,   167,     0,   168,   565,   169,   170,   171,
     566,   747,   567,   748,   749,     0,   175,   176,   177,   178,
     750,   751,   179,   752,   753,   182,     0,   183,   184,   185,
     186,   754,     0,     0,   188,   189,   190,   191,     0,   192,
     193,   755,   195,     0,   196,   197,   198,   568,   199,   756,
     757,   758,   759,   760,   200,   201,   202,   203,   204,   761,
     762,   207,  1969,   208,     0,   209,   210,   211,   212,   213,
       0,   214,     0,     0,   215,   763,   217,   218,     0,   219,
     220,   221,     0,   222,     0,   223,   224,   225,   764,   227,
     228,   765,   766,   230,   231,   767,     0,   233,     0,   234,
     569,     0,   570,   235,   236,     0,   237,   768,     0,   239,
       0,   240,   241,   242,   571,   243,   244,   245,     0,   246,
     247,   248,   249,   250,     0,   572,   251,   252,   253,   254,
     255,   769,   770,     0,   771,     0,   259,   573,   574,   260,
     575,   261,   262,   263,   264,   265,     0,   772,   576,   267,
     577,     0,   268,   269,   270,   773,   774,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   775,   578,
     776,   402,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   777,   293,   294,   579,   295,   778,   779,   780,   298,
     299,     0,     0,   300,   403,     0,     0,   781,   302,     0,
       0,   303,   580,   581,   782,   305,   306,   307,     0,   783,
     309,   310,   311,   312,     0,   313,   314,   315,   316,   317,
     318,   784,   320,     0,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   582,   332,   333,   785,   335,
     336,   337,   338,   339,   340,   341,     0,   342,   343,   344,
     786,   345,   346,   347,   348,   583,   349,   787,     0,   351,
     352,   353,   354,   355,   356,   357,   358,   788,   360,     0,
     361,   362,     0,   363,   364,   365,   366,     0,   789,   790,
       0,   369,     0,   370,   791,   372,   792,   793,   374,   375,
     376,   377,   378,     0,   794,   379,   380,   381,   382,   795,
       0,   383,   384,   385,   386,   796,   388,   584,   389,   390,
       0,     0,   391,   392,   393,   394,   395,   396,     0,   797,
     798,   497,   799,   800,   801,   802,   803,     0,     0,     0,
       0,   804,   805,     0,     0,     0,     0,     0,     0,     0,
     807,   156,   157,   158,   159,   160,   161,   162,   163,     0,
     164,   165,     0,     0,     0,     0,   746,     0,     0,   166,
     167,     0,   168,   565,   169,   170,   171,   566,   747,   567,
     748,   749,     0,   175,   176,   177,   178,   750,   751,   179,
     752,   753,   182,     0,   183,   184,   185,   186,   754,     0,
       0,   188,   189,   190,   191,     0,   192,   193,   755,   195,
       0,   196,   197,   198,   568,   199,   756,   757,   758,   759,
     760,   200,   201,   202,   203,   204,   761,   762,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   763,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   764,   227,   228,   765,   766,
     230,   231,   767,     0,   233,     0,   234,   569,  1639,   570,
     235,   236,     0,   237,   768,     0,   239,     0,   240,   241,
     242,   571,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,   572,   251,   252,   253,   254,   255,   769,   770,
       0,   771,     0,   259,   573,   574,   260,   575,   261,   262,
     263,   264,   265,     0,   772,   576,   267,   577,     0,   268,
     269,   270,   773,   774,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   775,   578,   776,   402,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   777,   293,
     294,   579,   295,   778,   779,   780,   298,   299,     0,     0,
     300,   403,     0,     0,   781,   302,     0,     0,   303,   580,
     581,   782,   305,   306,   307,     0,   783,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   784,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   582,   332,   333,   785,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   344,   786,   345,   346,
     347,   348,   583,   349,   787,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   788,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,   789,   790,     0,   369,     0,
     370,   791,   372,   792,   793,   374,   375,   376,   377,   378,
       0,   794,   379,   380,   381,   382,   795,     0,   383,   384,
     385,   386,   796,   388,   584,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   797,   798,   497,   799,
     800,   801,   802,   803,     0,     0,     0,     0,   804,   805,
       0,     0,     0,     0,     0,     0,     0,   807,   156,   157,
     158,   159,   160,   161,   162,   163,     0,   164,   165,     0,
       0,     0,     0,   746,     0,     0,   166,   167,     0,   168,
     565,   169,   170,   171,   566,   747,   567,   748,   749,     0,
     175,   176,   177,   178,   750,   751,   179,   752,   753,   182,
       0,   183,   184,   185,   186,   754,     0,     0,   188,   189,
     190,   191,     0,   192,   193,   755,   195,     0,   196,   197,
     198,   568,   199,   756,   757,   758,   759,   760,   200,   201,
     202,   203,   204,   761,   762,   207,  2367,   208,     0,   209,
     210,   211,   212,   213,     0,   214,     0,     0,   215,   763,
     217,   218,     0,   219,   220,   221,     0,   222,     0,   223,
     224,   225,   764,   227,   228,   765,   766,   230,   231,   767,
       0,   233,     0,   234,   569,     0,   570,   235,   236,     0,
     237,   768,     0,   239,     0,   240,   241,   242,   571,   243,
     244,   245,     0,   246,   247,   248,   249,   250,     0,   572,
     251,   252,   253,   254,   255,   769,   770,     0,   771,     0,
     259,   573,   574,   260,   575,   261,   262,   263,   264,   265,
       0,   772,   576,   267,   577,     0,   268,   269,   270,   773,
     774,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   775,   578,   776,   402,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   777,   293,   294,   579,   295,
     778,   779,   780,   298,   299,     0,     0,   300,   403,     0,
       0,   781,   302,     0,     0,   303,   580,   581,   782,   305,
     306,   307,     0,   783,   309,   310,   311,   312,     0,   313,
     314,   315,   316,   317,   318,   784,   320,     0,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   582,
     332,   333,   785,   335,   336,   337,   338,   339,   340,   341,
       0,   342,   343,   344,   786,   345,   346,   347,   348,   583,
     349,   787,     0,   351,   352,   353,   354,   355,   356,   357,
     358,   788,   360,     0,   361,   362,     0,   363,   364,   365,
     366,     0,   789,   790,     0,   369,     0,   370,   791,   372,
     792,   793,   374,   375,   376,   377,   378,     0,   794,   379,
     380,   381,   382,   795,     0,   383,   384,   385,   386,   796,
     388,   584,   389,   390,     0,     0,   391,   392,   393,   394,
     395,   396,     0,   797,   798,   497,   799,   800,   801,   802,
     803,     0,     0,     0,     0,   804,   805,     0,     0,     0,
       0,     0,     0,     0,   807,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
     746,     0,     0,   166,   167,     0,   168,   565,   169,   170,
     171,   566,   747,   567,   748,   749,     0,   175,   176,   177,
     178,   750,   751,   179,   752,   753,   182,     0,   183,   184,
     185,   186,   754,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   755,   195,     0,   196,   197,   198,   568,   199,
     756,   757,   758,   759,   760,   200,   201,   202,   203,   204,
     761,   762,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   763,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   764,
     227,   228,   765,   766,   230,   231,   767,     0,   233,     0,
     234,   569,     0,   570,   235,   236,     0,   237,   768,     0,
     239,     0,   240,   241,   242,   571,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,   572,   251,   252,   253,
     254,   255,   769,   770,     0,   771,     0,   259,   573,   574,
     260,   575,   261,   262,   263,   264,   265,     0,   772,   576,
     267,   577,     0,   268,   269,   270,   773,   774,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   775,
     578,   776,   402,   284,   285,   286,   287,   288,   289,   290,
     291,   292,   777,   293,   294,   579,   295,   778,   779,   780,
     298,   299,     0,     0,   300,   403,     0,     0,   781,   302,
       0,     0,   303,   580,   581,   782,   305,   306,   307,     0,
     783,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   784,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   582,   332,   333,   785,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,   786,   345,   346,   347,   348,   583,   349,   787,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   788,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   789,
     790,     0,   369,     0,   370,   791,   372,   792,   793,   374,
     375,   376,   377,   378,     0,   794,   379,   380,   381,   382,
     795,     0,   383,   384,   385,   386,   796,   388,   584,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     797,   798,   497,   799,   800,   801,   802,   803,     0,     0,
       0,     0,   804,   805,     0,     0,     0,     0,     0,     0,
       0,   807,   156,   157,   158,   159,   160,   161,   162,   163,
       0,   164,   165,     0,     0,     0,     0,   746,     0,     0,
     166,   167,     0,   168,   565,   169,   170,   171,   566,   747,
     567,   748,   749,     0,   175,   176,   177,   178,   750,   751,
     179,   752,   753,   182,     0,   183,   184,   185,   186,   754,
       0,     0,   188,   189,   190,   191,     0,   192,   193,   755,
     195,     0,   196,   197,   198,   568,   199,   756,   757,   758,
     759,   760,   200,   201,   202,   203,   204,   761,   762,   207,
       0,   208,     0,   209,   210,   211,   212,   213,     0,   214,
       0,     0,   215,   763,   217,   218,     0,   219,   220,   221,
       0,   222,     0,   223,   224,   225,   764,   227,   228,   765,
     766,   230,   231,   767,     0,   233,     0,   234,   569,     0,
     570,   235,   236,     0,   237,   768,     0,   239,     0,   240,
     241,   242,   571,   243,   244,   245,     0,   246,   247,   248,
     249,   250,     0,   572,   251,   252,   253,   254,   255,   769,
     770,     0,   771,     0,   259,   573,   574,   260,   575,   261,
     262,   263,   264,   265,     0,   772,   576,   267,   577,     0,
     268,   269,   270,   773,   774,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   775,   578,   776,   402,
     284,   285,   286,   287,   288,   289,   290,   291,   292,     0,
     293,   294,   579,   295,   778,   779,   780,   298,   299,     0,
       0,   300,   403,     0,     0,   781,   302,     0,     0,   303,
     580,   581,   782,   305,   306,   307,     0,   783,   309,   310,
     311,   312,     0,   313,   314,   315,   316,   317,   318,   784,
     320,     0,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   582,   332,   333,   785,   335,   336,   337,
     338,   339,   340,   341,     0,   342,   343,   344,   786,   345,
     346,   347,   348,   583,   349,   787,     0,   351,   352,   353,
     354,   355,   356,   357,   358,   788,   360,     0,   361,   362,
       0,   363,   364,   365,   366,     0,   789,   790,     0,   369,
       0,   370,   791,   372,   792,   793,   374,   375,   376,   377,
     378,     0,     0,   379,   380,   381,   382,   795,     0,   383,
     384,   385,   386,   796,   388,   584,   389,   390,     0,     0,
     391,   392,   393,   394,   395,   396,     0,   797,   798,   497,
     799,   800,   801,   802,   803,     0,     0,     0,     0,  1621,
    1622,     0,     0,     0,     0,     0,     0,     0,   807,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,   746,     0,     0,   166,   167,     0,
     168,   565,   169,   170,   171,     0,   747,   567,   748,   749,
       0,   175,   176,   177,   178,   750,   751,   179,   752,   753,
     182,     0,   183,   184,   185,   186,   754,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   755,   195,     0,   196,
     197,   198,   568,   199,   756,   757,   758,   759,   760,   200,
     201,   202,   203,   204,   761,   762,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     763,   217,   218,     0,   219,   220,   221,     0,     0,     0,
     223,   224,   225,   764,   227,   228,   765,   766,   230,   231,
     767,     0,   233,     0,   234,   569,     0,   570,   235,   236,
       0,   237,   768,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
     572,   251,   252,   253,   254,   255,   769,   770,     0,   771,
       0,   259,   573,   574,   260,   575,   261,   262,   263,   264,
     265,     0,   772,   576,   267,     0,     0,   268,   269,   270,
     773,   774,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   775,   578,   776,   402,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,   293,   294,   579,
     295,   778,   779,   780,   298,   299,     0,     0,   300,   403,
       0,     0,   781,   302,     0,     0,   303,   580,   581,   782,
     305,   306,   307,     0,   783,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   784,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     582,   332,   333,   785,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,   786,   345,   346,   347,   348,
       0,   349,   787,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   788,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   789,   790,     0,   369,     0,   370,   791,
     372,   792,   793,   374,   375,   376,   377,   378,     0,   794,
     379,   380,   381,   382,   795,     0,   383,   384,   385,   386,
     796,   388,   584,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   797,   798,   497,   799,   800,   801,
     802,   803,     0,     0,     0,     0,   804,   805,     0,     0,
       0,     0,     0,     0,     0,   807,   156,   157,   158,   159,
     160,   161,   162,   163,     0,   164,   165,     0,     0,     0,
       0,   746,     0,     0,   166,   167,     0,   168,   565,   169,
     170,   171,     0,   747,   567,   748,   749,     0,   175,   176,
     177,   178,   750,   751,   179,   752,   753,   182,     0,   183,
     184,   185,   186,   754,     0,     0,   188,   189,   190,   191,
       0,   192,   193,   755,   195,     0,   196,   197,   198,   568,
     199,   756,   757,   758,   759,   760,   200,   201,   202,   203,
     204,   761,   762,   207,     0,   208,     0,   209,   210,   211,
     212,   213,     0,   214,     0,     0,   215,   763,   217,   218,
       0,   219,   220,   221,     0,     0,     0,   223,   224,   225,
     764,   227,   228,   765,   766,   230,   231,   767,     0,   233,
       0,   234,   569,     0,   570,   235,   236,     0,   237,   768,
       0,   239,     0,   240,   241,   242,     0,   243,   244,   245,
       0,   246,   247,   248,   249,   250,     0,   572,   251,   252,
     253,   254,   255,   769,   770,     0,   771,     0,   259,   573,
     574,   260,   575,   261,   262,   263,   264,   265,     0,   772,
     576,   267,     0,     0,   268,   269,   270,   773,   774,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     775,   578,   776,   402,   284,   285,   286,   287,   288,   289,
     290,   291,   292,     0,   293,   294,   579,   295,   778,   779,
     780,   298,   299,     0,     0,   300,   403,     0,     0,   781,
     302,     0,     0,   303,   580,   581,   782,   305,   306,   307,
       0,   783,   309,   310,   311,   312,     0,   313,   314,   315,
     316,   317,   318,   784,   320,     0,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   582,   332,   333,
     785,   335,   336,   337,   338,   339,   340,   341,     0,   342,
     343,   344,   786,   345,   346,   347,   348,     0,   349,   787,
       0,   351,   352,   353,   354,   355,   356,   357,   358,   788,
     360,     0,   361,   362,     0,   363,   364,   365,   366,     0,
     789,   790,     0,   369,     0,   370,   791,   372,   792,   793,
     374,   375,   376,   377,   378,     0,     0,   379,   380,   381,
     382,   795,     0,   383,   384,   385,   386,   796,   388,   584,
     389,   390,     0,     0,   391,   392,   393,   394,   395,   396,
       0,   797,   798,   497,   799,   800,   801,   802,   803,     0,
       0,     0,     0,  1621,  1622,     0,     0,     0,     0,     0,
       0,     0,   807,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,   565,   169,   170,   171,   566,
     172,   567,   173,   174,     0,   175,   176,   177,   178,     0,
     751,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     754,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     755,   195,     0,   196,   197,   198,   568,   199,   756,   757,
     758,   759,   760,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     765,     0,   230,   231,   232,     0,   233,     0,   234,   569,
       0,   570,   235,   236,     0,   237,   768,     0,   239,     0,
     240,   241,   242,   571,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,   572,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,   573,   574,   260,   575,
     261,   262,   263,   264,   265,     0,   772,   576,   267,   577,
       0,   268,   269,   270,   773,   774,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   578,   283,
     402,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,   579,   295,     0,   779,   297,   298,   299,
       0,     0,   300,   403,     0,   548,   301,   302,     0,     0,
     303,   580,   581,   782,   305,   306,   307,     0,   783,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   582,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,    35,   342,   343,   344,   786,
     345,   346,   347,   348,   583,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   788,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   791,   372,   792,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,   795,     0,
     383,   384,   385,   386,   387,   388,   584,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   585,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,  1522,
     168,   565,   169,   170,   171,   566,   172,   567,   173,   174,
       0,   175,   176,   177,   178,     0,   751,   179,   180,   181,
     182,     0,   183,   184,   185,   186,   754,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   755,   195,     0,   196,
     197,   198,   568,   199,   756,   757,   758,   759,   760,   200,
     201,   202,   203,   204,   205,   206,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     216,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   226,   227,   228,   765,     0,   230,   231,
     232,     0,   233,     0,   234,   569,     0,   570,   235,   236,
       0,   237,   768,     0,   239,     0,   240,   241,   242,   571,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
     572,   251,   252,   253,   254,   255,   256,   257,     0,   258,
       0,   259,   573,   574,   260,   575,   261,   262,   263,   264,
     265,     0,   772,   576,   267,   577,     0,   268,   269,   270,
     773,   774,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   578,   283,   402,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,   293,   294,   579,
     295,     0,   779,   297,   298,   299,     0,     0,   300,   403,
       0,   548,   301,   302,     0,     0,   303,   580,   581,   782,
     305,   306,   307,     0,   783,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     582,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,   786,   345,   346,   347,   348,
     583,   349,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   788,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   367,   368,     0,   369,     0,   370,   791,
     372,   792,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,   795,     0,   383,   384,   385,   386,
     387,   388,   584,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   585,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,  1522,   168,   565,   169,   170,
     171,   566,   172,   567,   173,   174,     0,   175,   176,   177,
     178,     0,   751,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   754,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   755,   195,     0,   196,   197,   198,   568,   199,
     756,   757,   758,   759,   760,   200,   201,   202,   203,   204,
     205,   206,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   765,     0,   230,   231,   232,     0,   233,     0,
     234,   569,     0,   570,   235,   236,     0,   237,   768,     0,
     239,     0,   240,   241,   242,   571,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,   572,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,   573,   574,
     260,   575,   261,   262,   263,   264,   265,     0,   772,   576,
     267,   577,     0,   268,   269,   270,   773,   774,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     578,   283,   402,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,   579,   295,     0,   779,   297,
     298,   299,     0,     0,   300,   403,     0,     0,   301,   302,
       0,     0,   303,   580,   581,   782,   305,   306,   307,     0,
     783,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   582,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,   786,   345,   346,   347,   348,   583,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   788,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   791,   372,   792,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
     795,     0,   383,   384,   385,   386,   387,   388,   584,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     585,   156,   157,   158,   159,   160,   161,   162,   163,     0,
     164,   165,     0,     0,     0,     0,     0,     0,     0,   166,
     167,  2592,   168,     0,   169,   170,   171,     0,   172,     0,
     173,   174,     0,   175,   176,   177,   178,     0,     0,   179,
     180,   181,   182,     0,   183,   184,   185,   186,   187,     0,
       0,   188,   189,   190,   191,     0,   192,   193,   194,   195,
       0,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,   205,   206,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   216,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,     0,   233,     0,   234,     0,     0,     0,
     235,   236,     0,   237,   238,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   258,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,   266,     0,   267,     0,     0,   268,
     269,   270,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,     0,   283,   402,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,   293,
     294,     0,   295,     0,   296,   297,   298,   299,     0,     0,
     300,   403,     0,     0,   301,   302,     0,     0,   303,     0,
       0,   304,   305,   306,   307,     0,   308,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   319,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   344,     0,   345,   346,
     347,   348,     0,   349,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,   367,   368,     0,   369,     0,
     370,   371,   372,   373,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,     0,   383,   384,
     385,   386,   387,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   397,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,     0,     0,     0,   166,   167,   860,   168,     0,
     169,   170,   171,     0,   172,     0,   173,   174,     0,   175,
     176,   177,   178,     0,     0,   179,   180,   181,   182,     0,
     183,   184,   185,   186,   187,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   194,   195,     0,   196,   197,   198,
       0,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,   205,   206,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   216,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,     0,
     233,     0,   234,     0,     0,     0,   235,   236,     0,   237,
     238,     0,   239,     0,   240,   241,   242,     0,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,     0,   251,
     252,   253,   254,   255,   256,   257,     0,   258,     0,   259,
       0,     0,   260,     0,   261,   262,   263,   264,   265,     0,
     266,     0,   267,     0,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,     0,   283,     0,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,   293,   294,     0,   295,     0,
     296,   297,   298,   299,     0,     0,   300,     0,     0,     0,
     301,   302,     0,     0,   303,     0,     0,   304,   305,   306,
     307,     0,   308,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,    35,
     342,   343,   344,     0,   345,   346,   347,   348,     0,   349,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   367,   368,     0,   369,     0,   370,   371,   372,   373,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,   387,   388,
       0,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   397,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,    43,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,  2271,
     168,     0,   169,   170,   171,     0,   172,     0,   173,   174,
       0,   175,   176,   177,   178,     0,     0,   179,   180,   181,
     182,     0,   183,   602,   185,   186,   187,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   194,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,   205,   206,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     216,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,     0,   233,     0,   234,     0,     0,     0,   235,   236,
       0,   237,   238,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
       0,   251,   252,   253,   254,   255,   256,   257,     0,   258,
       0,   259,     0,     0,   260,     0,   261,   262,   263,   264,
     265,     0,   266,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,     0,   283,     0,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,   293,   294,     0,
     295,     0,   296,   297,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,   303,     0,     0,   304,
     305,   306,   307,     0,   308,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,     0,   345,   346,   347,   348,
       0,   349,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   367,   368,     0,   369,     0,   370,   371,
     372,   373,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
     387,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   397,     0,     0,     0,     0,   603,
       0,     0,   604,   605,   606,     0,   607,   608,   609,   610,
     611,   612,   156,   157,   158,   159,   160,   161,   162,   163,
       0,   164,   165,     0,     0,     0,     0,     0,     0,     0,
     166,   167,     0,   168,     0,   169,   170,   171,     0,   172,
       0,   173,   174,     0,   175,   176,   177,   178,     0,     0,
     179,   180,   181,   182,     0,   183,   669,   185,   186,   187,
       0,     0,   188,   189,   190,   191,     0,   192,   193,   194,
     195,     0,   196,   197,   198,     0,   199,     0,     0,     0,
       0,     0,   200,   201,   202,   203,   204,   205,   206,   207,
       0,   208,     0,   209,   210,   211,   212,   213,     0,   214,
       0,     0,   215,   216,   217,   218,     0,   219,   220,   221,
       0,   222,     0,   223,   224,   225,   226,   227,   228,   229,
       0,   230,   231,   232,     0,   233,     0,   234,     0,     0,
       0,   235,   236,     0,   237,   238,     0,   239,     0,   240,
     241,   242,     0,   243,   244,   245,     0,   246,   247,   248,
     249,   250,     0,     0,   251,   252,   253,   254,   255,   256,
     257,     0,   258,     0,   259,     0,     0,   260,     0,   261,
     262,   263,   264,   265,     0,   266,     0,   267,     0,     0,
     268,   269,   270,     0,     0,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,     0,   283,     0,
     284,   285,   286,   287,   288,   289,   290,   291,   292,     0,
     293,   294,     0,   295,     0,   296,   297,   298,   299,     0,
       0,   300,     0,     0,     0,   301,   302,     0,     0,   303,
       0,     0,   304,   305,   306,   307,     0,   308,   309,   310,
     311,   312,     0,   313,   314,   315,   316,   317,   318,   319,
     320,     0,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,     0,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,     0,   342,   343,   344,     0,   345,
     346,   347,   348,     0,   349,   350,     0,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,     0,   361,   362,
       0,   363,   364,   365,   366,     0,   367,   368,     0,   369,
       0,   370,   371,   372,   373,     0,   374,   375,   376,   377,
     378,     0,     0,   379,   380,   381,   382,     0,     0,   383,
     384,   385,   386,   387,   388,     0,   389,   390,     0,     0,
     391,   392,   393,   394,   395,   396,     0,   397,     0,     0,
       0,     0,   603,     0,     0,   604,   605,   606,     0,   607,
     608,   609,   610,   611,   612,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   697,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,     0,     0,     0,     0,   603,     0,     0,   604,   605,
     606,     0,   607,   608,   609,   610,   611,   612,   156,   157,
     158,   159,   160,   161,   162,   163,     0,   164,   165,     0,
       0,     0,     0,     0,     0,     0,   166,   167,     0,   168,
       0,   169,   170,   171,     0,   172,     0,   173,   174,     0,
     175,   176,   177,   178,     0,     0,   179,   180,   181,   182,
       0,   183,  1046,   185,   186,   187,     0,     0,   188,   189,
     190,   191,     0,   192,   193,   194,   195,     0,   196,   197,
     198,     0,   199,     0,     0,     0,     0,     0,   200,   201,
     202,   203,   204,   205,   206,   207,     0,   208,     0,   209,
     210,   211,   212,   213,     0,   214,     0,     0,   215,   216,
     217,   218,     0,   219,   220,   221,     0,   222,     0,   223,
     224,   225,   226,   227,   228,   229,     0,   230,   231,   232,
       0,   233,     0,   234,     0,     0,     0,   235,   236,     0,
     237,   238,     0,   239,     0,   240,   241,   242,     0,   243,
     244,   245,     0,   246,   247,   248,   249,   250,     0,     0,
     251,   252,   253,   254,   255,   256,   257,     0,   258,     0,
     259,     0,     0,   260,     0,   261,   262,   263,   264,   265,
       0,   266,     0,   267,     0,     0,   268,   269,   270,     0,
       0,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,     0,   283,     0,   284,   285,   286,   287,
     288,   289,   290,   291,   292,     0,   293,   294,     0,   295,
       0,   296,   297,   298,   299,     0,     0,   300,     0,     0,
       0,   301,   302,     0,     0,   303,     0,     0,   304,   305,
     306,   307,     0,   308,   309,   310,   311,   312,     0,   313,
     314,   315,   316,   317,   318,   319,   320,     0,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,     0,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
       0,   342,   343,   344,     0,   345,   346,   347,   348,     0,
     349,   350,     0,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,     0,   361,   362,     0,   363,   364,   365,
     366,     0,   367,   368,     0,   369,     0,   370,   371,   372,
     373,     0,   374,   375,   376,   377,   378,     0,     0,   379,
     380,   381,   382,     0,     0,   383,   384,   385,   386,   387,
     388,     0,   389,   390,     0,     0,   391,   392,   393,   394,
     395,   396,     0,   397,     0,     0,     0,     0,   603,     0,
       0,   604,   605,   606,     0,   607,   608,   609,   610,   611,
     612,   156,   157,   158,   159,   160,   161,   162,   163,     0,
     164,   165,     0,     0,     0,     0,     0,     0,     0,   166,
     167,     0,   168,     0,   169,   170,   171,     0,   172,     0,
     173,   174,     0,   175,   176,   177,   178,     0,     0,   179,
     180,   181,   182,     0,   183,   184,   185,   186,   187,     0,
       0,   188,   189,   190,   191,     0,   192,   193,   194,   195,
       0,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,   205,   206,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   216,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,     0,   233,     0,   234,     0,     0,     0,
     235,   236,     0,   237,   238,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   258,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,   266,     0,   267,     0,     0,   268,
     269,   270,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,     0,   283,     0,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,   293,
     294,     0,   295,     0,   296,   297,   298,   299,     0,     0,
     300,     0,     0,     0,   301,   302,     0,     0,   303,     0,
       0,   304,   305,   306,   307,     0,   308,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   319,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   344,     0,   345,   346,
     347,   348,     0,   349,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,   367,   368,     0,   369,     0,
     370,   371,   372,   373,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,     0,   383,   384,
     385,   386,   387,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   397,     0,     0,     0,
       0,   603,     0,     0,   604,   605,   606,     0,   607,   608,
     609,   610,   611,   612,   156,   157,   158,   159,   160,   161,
     162,   163,     0,   164,   165,     0,     0,     0,     0,     0,
       0,     0,   166,   167,     0,   168,     0,   169,   170,   171,
       0,  1057,     0,  1058,  1059,     0,   175,   176,   177,   178,
       0,     0,   179,  1060,  1061,   182,     0,   183,   184,   185,
     186,     0,     0,     0,   188,   189,   190,   191,     0,   192,
     193,     0,   195,     0,   196,   197,   198,     0,   199,     0,
       0,     0,     0,     0,   200,   201,   202,   203,   204,  1062,
    1063,   207,     0,   208,     0,   209,   210,   211,   212,   213,
       0,   214,     0,     0,   215,   763,   217,   218,     0,   219,
     220,   221,     0,   222,     0,   223,   224,   225,     0,   227,
     228,     0,     0,   230,   231,  1064,     0,   233,     0,   234,
       0,     0,     0,   235,   236,     0,   237,     0,     0,   239,
       0,   240,   241,   242,     0,   243,   244,   245,     0,   246,
     247,   248,   249,   250,     0,     0,     0,   252,   253,   254,
     255,  1065,  1066,     0,  1067,     0,   259,     0,     0,   260,
       0,   261,   262,   263,   264,   265,     0,     0,     0,   267,
       0,     0,   268,   269,   270,     0,     0,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,  1068,     0,
    1069,     0,   284,   285,   286,   287,   288,   289,   290,     0,
     292,     0,   293,   294,     0,   295,     0,     0,  1070,   298,
     299,     0,     0,   300,     0,     0,     0,  2180,   302,     0,
       0,     0,     0,     0,     0,   305,   306,   307,     0,     0,
       0,   310,   311,   312,     0,   313,   314,   315,   316,   317,
     318,  1071,   320,     0,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,     0,   332,   333,     0,   335,
     336,   337,   338,   339,   340,   341,     0,   342,   343,   344,
       0,   345,  1376,   347,   348,     0,   349,  1073,     0,   351,
     352,   353,   354,   355,   356,   357,   358,     0,   360,     0,
     361,   362,     0,   363,   364,   365,   366,     0,  1074,  1075,
       0,   369,     0,   370,     0,   372,     0,     0,   374,   375,
     376,   377,   378,     0,     0,   379,   380,   381,   382,     0,
       0,   383,   384,   385,   386,  1076,   388,     0,   389,   390,
       0,     0,   391,   392,   393,   394,   395,   396,     0,  1077,
    1291,   497,     0,     0,   603,   481,     0,   604,   605,   606,
       0,   607,  2181,   609,   610,   611,   612,   156,   157,   158,
     159,   160,   161,   162,   163,   958,   164,   165,   959,   960,
     961,   962,   963,   964,   965,   166,   167,   966,   168,   565,
     169,   170,   171,   566,   172,   567,   173,   174,   967,   175,
     176,   177,   178,   968,   969,   179,   180,   181,   182,   970,
     183,   184,   185,   186,   187,   971,   972,   188,   189,   190,
     191,   973,   192,   193,   194,   195,   974,   196,   197,   198,
     568,   199,   975,   976,   977,   978,   979,   200,   201,   202,
     203,   204,   205,   206,   207,   980,   208,   981,   209,   210,
     211,   212,   213,   982,   214,   983,   984,   215,   216,   217,
     218,   985,   219,   220,   221,   986,   222,   987,   223,   224,
     225,   226,   227,   228,   229,   988,   230,   231,   232,   989,
     233,   990,   234,   569,   991,   570,   235,   236,   992,   237,
     238,   993,   239,   994,   240,   241,   242,   571,   243,   244,
     245,   995,   246,   247,   248,   249,   250,   996,   572,   251,
     252,   253,   254,   255,   256,   257,   997,   258,   998,   259,
     573,   574,   260,   575,   261,   262,   263,   264,   265,   999,
     266,   576,   267,   577,  1000,   268,   269,   270,  1001,  1002,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   578,   283,  1003,   284,   285,   286,   287,   288,
     289,   290,   291,   292,  1004,   293,   294,   579,   295,  1005,
     296,   297,   298,   299,  1006,  1007,   300,  1008,  1009,  1010,
     301,   302,  1011,  1012,   303,   580,   581,   304,   305,   306,
     307,  1013,   308,   309,   310,   311,   312,  1014,   313,   314,
     315,   316,   317,   318,   319,   320,  1015,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   582,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,  1016,
     342,   343,   344,  1017,   345,   346,   347,   348,   583,   349,
     350,  1018,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,  1019,   361,   362,  1020,   363,   364,   365,   366,
    1021,   367,   368,  1022,   369,  1023,   370,   371,   372,   373,
    1024,   374,   375,   376,   377,   378,  1025,  1026,   379,   380,
     381,   382,  1027,  1028,   383,   384,   385,   386,   387,   388,
     584,   389,   390,  1029,  1030,   391,   392,   393,   394,   395,
     396,     0,  1031,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,  1032,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
     402,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,   403,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,   546,     0,     0,     0,     0,   166,   167,     0,
     168,     0,   169,   170,   171,     0,  1057,     0,  1058,  1059,
       0,   175,   176,   177,   178,     0,     0,   179,  1060,  1061,
     182,     0,   183,   184,   185,   186,     0,     0,     0,   188,
     189,   190,   191,     0,   192,   193,     0,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,  1062,  1063,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     763,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,     0,   227,   228,     0,     0,   230,   231,
    1064,     0,   233,     0,   234,     0,     0,     0,   235,   236,
       0,   237,     0,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
       0,     0,   252,   253,   254,   255,  1065,  1066,     0,  1067,
       0,   259,     0,     0,   260,     0,   261,   262,   263,   264,
     265,     0,     0,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,  1068,     0,  1069,     0,   284,   285,   286,
     287,   288,   289,   290,     0,   292,     0,   293,   294,     0,
     295,     0,     0,  1070,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,     0,     0,     0,     0,
     305,   306,   307,     0,     0,     0,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,  1071,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,   333,     0,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,     0,   345,  1072,   347,   348,
       0,   349,  1073,     0,   351,   352,   353,   354,   355,   356,
     357,   358,     0,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,  1074,  1075,     0,   369,     0,   370,     0,
     372,     0,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
    1076,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,  1077,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,  1340,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,  1300,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,  1301,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,  1302,     0,   300,     0,  1303,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,  1304,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,  1291,   845,     0,     0,     0,   481,     0,     0,     0,
       0,     0,     0,  1292,   156,   157,   158,   159,   160,   161,
     162,   163,     0,   164,   165,     0,     0,     0,     0,     0,
       0,     0,   166,   167,     0,   168,     0,   169,   170,   171,
       0,   172,     0,   173,   174,     0,   175,   176,   177,   178,
       0,     0,   179,   180,   181,   182,     0,   183,   184,   185,
     186,   187,     0,     0,   188,   189,   190,   191,     0,   192,
     193,   194,   195,     0,   196,   197,   198,     0,   199,     0,
       0,     0,     0,     0,   200,   201,   202,   203,   204,   205,
     206,   207,     0,   208,     0,   209,   210,   211,   212,   213,
       0,   214,     0,     0,   215,   216,   217,   218,     0,   219,
     220,   221,     0,   222,     0,   223,   224,   225,   226,   227,
     228,   229,  1301,   230,   231,   232,     0,   233,     0,   234,
       0,     0,     0,   235,   236,     0,   237,   238,     0,   239,
       0,   240,   241,   242,     0,   243,   244,   245,     0,   246,
     247,   248,   249,   250,     0,     0,   251,   252,   253,   254,
     255,   256,   257,     0,   258,     0,   259,     0,     0,   260,
       0,   261,   262,   263,   264,   265,     0,   266,     0,   267,
       0,     0,   268,   269,   270,     0,     0,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,     0,
     283,     0,   284,   285,   286,   287,   288,   289,   290,   291,
     292,     0,   293,   294,     0,   295,     0,   296,   297,   298,
     299,  1302,     0,   300,     0,  1303,     0,   301,   302,     0,
       0,   303,     0,     0,   304,   305,   306,   307,     0,   308,
     309,   310,   311,   312,     0,   313,   314,   315,   316,   317,
     318,   319,   320,     0,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,     0,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,     0,   342,   343,   344,
       0,   345,   346,   347,   348,     0,   349,   350,     0,   351,
     352,   353,   354,   355,   356,   357,   358,   359,   360,     0,
     361,   362,     0,   363,   364,   365,   366,     0,   367,   368,
       0,   369,     0,   370,   371,   372,   373,  1304,   374,   375,
     376,   377,   378,     0,     0,   379,   380,   381,   382,     0,
       0,   383,   384,   385,   386,   387,   388,     0,   389,   390,
       0,     0,   391,   392,   393,   394,   395,   396,     0,   397,
    1291,   845,     0,     0,     0,   481,     0,     0,     0,     0,
       0,     0,  1292,   156,   472,   158,   159,   160,   161,   162,
     163,   473,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   474,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   475,   232,     0,   233,     0,   476,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   477,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   478,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   479,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   480,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,  1246,   361,
     362,     0,   363,   364,   365,   366,  1248,   367,   368,     0,
     369,  1249,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,  1246,
       0,   391,   392,   393,   394,   395,   396,  1248,   397,     0,
       0,     0,  1249,     0,   481,     0,     0,     0,     0,     0,
       0,   482,     0,     0,     0,     0,  1246,     0,     0,     0,
       0,     0,     0,     0,  1248,     0,     0,     0,     0,  1249,
       0,     0,     0,     0,     0,     0,     0,  1246,     0,     0,
       0,     0,     0,     0,     0,  1248,     0,     0,     0,     0,
    1249,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1246,  1250,     0,     0,     0,  1251,
       0,     0,  1248,     0,     0,     0,     0,  1249,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1252,  1253,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1254,     0,     0,     0,  1246,  1250,     0,     0,     0,
    1251,     0,     0,  1248,     0,     0,     0,     0,  1249,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1252,
    1253,     0,  1255,  1250,     0,  1256,     0,  1251,     0,     0,
       0,     0,  1254,     0,     0,     0,     0,     0,  1257,     0,
    1258,     0,     0,     0,  1250,     0,  1252,  1253,  1251,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1254,
       0,     0,     0,  1255,     0,     0,  1256,  1252,  1253,     0,
       0,  1250,     0,     0,     0,  1251,     0,     0,     0,  1257,
    1254,  1258,     0,     0,     0,     0,     0,     0,     0,     0,
    1255,     0,     0,  1256,  1252,  1253,  1259,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1257,  1254,  1258,     0,
       0,  1255,  1250,     0,  1256,     0,  1251,     0,  2298,     0,
       0,     0,     0,     0,     0,     0,     0,  1257,     0,  1258,
       0,     0,     0,     0,     0,  1252,  1253,  1259,  1255,     0,
       0,  1256,     0,     0,     0,     0,     0,     0,  1254,     0,
       0,     0,     0,     0,  1257,     0,  1258,     0,     0,     0,
       0,     0,     0,     0,  1259,  1260,     0,     0,  1261,  1262,
    1263,     0,  1264,  1265,  1266,  1267,  1268,  1269,     0,  1255,
       0,     0,  1256,  1270,     0,  1259,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1257,     0,  1258,     0,     0,
       0,     0,     0,     0,     0,     0,  1260,     0,     0,  1261,
    1262,  1263,  1259,  1264,  1265,  1266,  1267,  1268,  1269,     0,
       0,  1246,     0,  2335,  1270,     0,     0,     0,     0,  1248,
       0,     0,     0,  1260,  1249,     0,  1261,  1262,  1263,     0,
    1264,  1265,  1266,  1267,  1268,  1269,     0,  1246,     0,     0,
    2374,  1270,     0,  1259,  1260,  1248,     0,  1261,  1262,  1263,
    1249,  1264,  1265,  1266,  1267,  1268,  1269,     0,     0,     0,
       0,  2504,  1270,     0,     0,     0,     0,     0,     0,     0,
       0,  1260,     0,     0,  1261,  1262,  1263,     0,  1264,  1265,
    1266,  1267,  1268,  1269,     0,     0,     0,     0,  2523,  1270,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1260,     0,     0,  1261,  1262,  1263,     0,  1264,
    1265,  1266,  1267,  1268,  1269,     0,     0,     0,  1250,  2637,
    1270,     0,  1251,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1252,  1253,     0,  1250,     0,     0,     0,  1251,     0,
       0,     0,     0,     0,  1254,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1252,  1253,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1254,     0,     0,     0,     0,  1255,     0,     0,  1256,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1257,     0,  1258,     0,     0,     0,     0,     0,     0,
       0,  1255,     0,     0,  1256,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1257,     0,  1258,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1259,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1259,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1260,     0,
       0,  1261,  1262,  1263,     0,  1264,  1265,  1266,  1267,  1268,
    1269,     0,     0,     0,     0,  2699,  1270,     0,     0,     0,
       0,     0,     0,     0,  1260,     0,     0,  1261,  1262,  1263,
       0,  1264,  1265,  1266,  1267,  1268,  1269,     0,     0,     0,
       0,     0,  1270,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,  2752,
     497,  2753,  2754,     0,  2755,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,  1285,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,     0,   845,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,     0,
     845,   156,   157,   158,   159,   160,   161,   162,   163,   958,
     164,   165,   959,   960,   961,   962,   963,   964,   965,   166,
     167,   966,   168,   565,   169,   170,   171,   566,   172,   567,
     173,   174,   967,   175,   176,   177,   178,   968,   969,   179,
     180,   181,   182,   970,   183,   184,   185,   186,   187,   971,
     972,   188,   189,   190,   191,   973,   192,   193,   194,   195,
     974,   196,   197,   198,   568,   199,   975,   976,   977,   978,
     979,   200,   201,   202,   203,   204,   205,   206,   207,   980,
     208,   981,   209,   210,   211,   212,   213,   982,   214,   983,
     984,   215,   216,   217,   218,   985,   219,   220,   221,   986,
     222,   987,   223,   224,   225,   226,   227,   228,   229,   988,
     230,   231,   232,   989,   233,   990,   234,   569,   991,   570,
     235,   236,   992,   237,   238,   993,   239,   994,   240,   241,
     242,   571,   243,   244,   245,   995,   246,   247,   248,   249,
     250,   996,   572,   251,   252,   253,   254,   255,   256,   257,
     997,   258,   998,   259,   573,   574,   260,   575,   261,   262,
     263,   264,   265,   999,   266,   576,   267,   577,  1000,   268,
     269,   270,  1001,  1002,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   578,   283,  1003,   284,
     285,   286,   287,   288,   289,   290,   291,   292,  1004,   293,
     294,   579,   295,  1005,   296,   297,   298,   299,  1006,  1007,
     300,  1008,  1009,  1010,   301,   302,  1011,  1012,   303,   580,
     581,   304,   305,   306,   307,  1013,   308,   309,   310,   311,
     312,  1014,   313,   314,   315,   316,   317,   318,   319,   320,
    1015,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   582,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,  1016,   342,   343,   344,  1017,   345,   346,
     347,   348,   583,   349,   350,  1018,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,  1019,   361,   362,  1020,
     363,   364,   365,   366,  1021,   367,   368,  1022,   369,  1023,
     370,   371,   372,   373,  1024,   374,   375,   376,   377,   378,
    1025,  1026,   379,   380,   381,   382,  1027,  1028,   383,   384,
     385,   386,   387,   388,   584,   389,   390,  1029,  1030,   391,
     392,   393,   394,   395,   396,     0,  1031,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,     0,     0,     0,   166,   167,     0,   168,   565,
     169,   170,   171,   566,   172,   567,   173,   174,     0,   175,
     176,   177,   178,     0,     0,   179,   180,   181,   182,     0,
     183,   184,   185,   186,   187,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   194,   195,     0,   196,   197,   198,
     568,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,   205,   206,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   216,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,     0,
     233,     0,   234,   569,     0,   570,   235,   236,     0,   237,
     238,     0,   239,     0,   240,   241,   242,   571,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,   572,   251,
     252,   253,   254,   255,   256,   257,     0,   258,     0,   259,
     573,   574,   260,   575,   261,   262,   263,   264,   265,     0,
     266,   576,   267,   577,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   578,   283,   402,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,   293,   294,   579,   295,     0,
     296,   297,   298,   299,     0,     0,   300,   403,     0,     0,
     301,   302,     0,     0,   303,   580,   581,   304,   305,   306,
     307,     0,   308,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   582,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,     0,   345,   346,   347,   348,   583,   349,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   367,   368,     0,   369,     0,   370,   371,   372,   373,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,   387,   388,
     584,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   585,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,  1351,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,  1352,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,  1353,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,  1903,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,  1354,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,  1355,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,     0,
     168,   565,   169,   170,   171,   566,  1057,   567,  1058,  1059,
       0,   175,   176,   177,   178,     0,     0,   179,  1060,  1061,
     182,     0,   183,   184,   185,   186,     0,     0,     0,   188,
     189,   190,   191,     0,   192,   193,     0,   195,     0,   196,
     197,   198,   568,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,  1062,  1063,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     763,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,     0,   227,   228,     0,     0,   230,   231,
    1064,     0,   233,     0,   234,   569,     0,   570,   235,   236,
       0,   237,     0,     0,   239,     0,   240,   241,   242,   571,
     243,   244,   245,  1373,   246,   247,   248,   249,   250,     0,
     572,  1374,   252,   253,   254,   255,  1065,  1066,     0,  1067,
       0,   259,   573,   574,   260,   575,   261,   262,   263,   264,
     265,     0,     0,   576,   267,   577,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,  1068,   578,  1069,     0,   284,   285,   286,
     287,   288,   289,   290,     0,   292,     0,   293,   294,   579,
     295,     0,     0,  1070,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,  1375,   580,   581,     0,
     305,   306,   307,     0,     0,     0,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,  1071,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     582,   332,   333,     0,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,     0,   345,  1376,   347,   348,
     583,   349,  1073,     0,   351,   352,   353,   354,   355,   356,
     357,   358,     0,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,  1074,  1075,     0,   369,     0,   370,     0,
     372,     0,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
    1076,   388,   584,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,  1377,   156,   157,   158,   159,   160,
     520,   162,   163,   486,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,   487,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,   521,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,   488,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   489,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,   156,   157,   158,   159,   160,   161,   162,   163,   486,
     164,   165,     0,     0,     0,     0,     0,     0,     0,   166,
     167,     0,   168,     0,   169,   170,   171,     0,   172,     0,
     173,   174,     0,   175,   176,   177,   178,     0,     0,   179,
     180,   181,   182,     0,   183,   184,   185,   186,   187,     0,
       0,   188,   189,   190,   191,     0,   192,   193,   194,   195,
     487,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,   205,   206,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   216,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,     0,   233,     0,   234,     0,     0,     0,
     235,   236,     0,   237,   238,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   258,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,   266,     0,   267,     0,     0,   268,
     269,   270,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,     0,   283,     0,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,   293,
     294,     0,   295,     0,   296,   297,   298,   299,     0,     0,
     300,     0,     0,     0,   301,   302,     0,     0,   303,     0,
       0,   304,   305,   306,   307,     0,   308,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   319,   320,
     488,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   489,   342,   343,   344,     0,   345,   346,
     347,   348,     0,   349,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,   367,   368,     0,   369,     0,
     370,   371,   372,   373,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,     0,   383,   384,
     385,   386,   387,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   397,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     3,     4,
       0,     0,     0,     0,     0,   166,   167,     0,   168,     0,
     169,   170,   171,     0,   172,     0,   173,   174,     0,   175,
     176,   177,   178,     0,     0,   179,   180,   181,   182,     0,
     183,   184,   185,   186,   187,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   194,   195,     0,   196,   197,   198,
       0,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,   205,   206,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   216,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,     0,
     233,     0,   234,     0,     0,     0,   235,   236,     0,   237,
     238,     0,   239,     0,   240,   241,   242,     0,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,     0,   251,
     252,   253,   254,   255,   256,   257,     0,   258,     0,   259,
       0,     0,   260,     0,   261,   262,   263,   264,   265,     0,
     266,     0,   267,     0,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,     0,   283,   402,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,   293,   294,     0,   295,     0,
     296,   297,   298,   299,     0,     0,   300,   403,     0,     0,
     301,   302,     0,     0,   303,     0,     0,   304,   305,   306,
     307,     0,   308,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,     0,   345,   346,   347,   348,     0,   349,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   367,   368,     0,   369,     0,   370,   371,   372,   373,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,   387,   388,
       0,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   397,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
     402,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,   403,     0,   548,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,     0,
     168,     0,   169,   170,   171,     0,   172,     0,   173,   174,
       0,   175,   176,   177,   178,     0,     0,   179,   180,   181,
     182,     0,   183,   184,   185,   186,   187,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   194,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,  1152,   203,   204,   205,   206,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     216,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,     0,   233,     0,   234,     0,     0,     0,  1153,   236,
       0,   237,   238,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
       0,   251,   252,   253,   254,   255,   256,   257,     0,   258,
       0,   259,     0,     0,   260,     0,   261,   262,  1154,   264,
     265,     0,   266,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,     0,   283,   402,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,   293,   294,     0,
     295,     0,   296,   297,   298,   299,     0,     0,   300,   403,
       0,     0,   301,   302,     0,     0,   303,     0,     0,   304,
     305,   306,   307,     0,   308,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,   333,   334,   335,   336,   337,  1155,   339,   340,
     341,     0,   342,   343,   344,     0,   345,   346,   347,   348,
       0,   349,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   362,  1156,  1157,   364,
     365,   366,     0,   367,   368,     0,   369,     0,   370,   371,
     372,   373,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
     387,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   397,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,   487,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,   488,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   489,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,   156,   157,   158,   159,   160,   161,   162,   163,     0,
     164,   165,     0,     0,     0,     0,     0,     0,     0,   166,
     167,     0,   168,     0,   169,   170,   171,     0,   172,     0,
     173,   174,     0,   175,   176,   177,   178,     0,     0,   179,
     180,   181,   182,     0,   183,   184,   185,   186,   187,     0,
       0,   188,   189,   190,   191,     0,   192,   193,   194,   195,
       0,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,   205,   206,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   216,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,     0,   233,     0,   234,     0,     0,     0,
     235,  1696,     0,   237,   238,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   258,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,   266,     0,   267,     0,     0,   268,
     269,  1697,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,     0,   283,   402,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,   293,
     294,     0,   295,     0,   296,   297,   298,   299,     0,     0,
     300,   403,     0,     0,   301,   302,     0,     0,   303,     0,
       0,   304,   305,   306,   307,     0,   308,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   319,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   344,     0,   345,   346,
     347,   348,     0,   349,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,   362,  1698,
     363,  1699,   365,  1700,     0,   367,   368,     0,   369,     0,
     370,   371,   372,   373,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,     0,   383,   384,
     385,   386,   387,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   397,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,     0,     0,     0,   166,   167,     0,   168,   565,
     169,   170,   171,   566,  1057,   567,  1058,  1059,     0,   175,
     176,   177,   178,     0,     0,   179,  1060,  1061,   182,     0,
     183,   184,   185,   186,     0,     0,     0,   188,   189,   190,
     191,     0,   192,   193,     0,   195,     0,   196,   197,   198,
     568,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,  1062,  1063,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   763,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,     0,   227,   228,     0,     0,   230,   231,  1064,     0,
     233,     0,   234,   569,     0,   570,   235,   236,     0,   237,
       0,     0,   239,     0,   240,   241,   242,   571,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,   572,     0,
     252,   253,   254,   255,  1065,  1066,     0,  1067,     0,   259,
     573,   574,   260,   575,   261,   262,   263,   264,   265,     0,
       0,   576,   267,   577,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,  1068,   578,  1069,     0,   284,   285,   286,   287,   288,
     289,   290,     0,   292,     0,   293,   294,   579,   295,     0,
       0,  1070,   298,   299,     0,     0,   300,     0,     0,     0,
     301,   302,     0,     0,     0,   580,   581,     0,   305,   306,
     307,     0,     0,     0,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,  1071,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   582,   332,
     333,     0,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,     0,   345,  1376,   347,   348,   583,   349,
    1073,     0,   351,   352,   353,   354,   355,   356,   357,   358,
       0,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,  1074,  1075,     0,   369,     0,   370,     0,   372,     0,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,  1076,   388,
     584,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,  1377,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
     402,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,   403,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,  1823,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,     0,
     168,     0,   169,   170,   171,     0,   172,     0,   173,   174,
       0,   175,   176,   177,   178,     0,     0,   179,   180,   181,
     182,     0,   183,   184,   185,   186,   187,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   194,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,   205,   206,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     216,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,     0,   233,     0,   234,     0,     0,     0,   235,   236,
       0,   237,   238,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
       0,   251,   252,   253,   254,   255,   256,   257,     0,   258,
       0,   259,     0,     0,   260,     0,   261,   262,   263,   264,
     265,     0,   266,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,     0,   283,     0,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,   293,   294,     0,
     295,     0,   296,   297,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,   303,     0,     0,   304,
     305,   306,   307,     0,   308,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,     0,   345,   346,   347,   348,
       0,   349,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   367,   368,     0,   369,     0,   370,   371,
     372,   373,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,  1824,     0,   383,   384,   385,   386,
     387,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   397,   156,   157,   158,   159,   160,
     161,   162,   163,  1828,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
    1829,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,   156,   157,   158,   159,   160,   161,   162,   163,   514,
     164,   165,     0,     0,     0,     0,     0,     0,     0,   166,
     167,     0,   168,     0,   169,   170,   171,     0,   172,     0,
     173,   174,     0,   175,   176,   177,   178,     0,     0,   179,
     180,   181,   182,     0,   183,   184,   185,   186,   187,     0,
       0,   188,   189,   190,   191,     0,   192,   193,   194,   195,
       0,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,   205,   206,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   216,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,     0,   233,     0,   234,     0,     0,     0,
     235,   236,     0,   237,   238,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   258,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,   266,     0,   267,     0,     0,   268,
     269,   270,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,     0,   283,     0,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,   293,
     294,     0,   295,     0,   296,   297,   298,   299,     0,     0,
     300,     0,     0,     0,   301,   302,     0,     0,   303,     0,
       0,   304,   305,   306,   307,     0,   308,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   319,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   515,     0,   345,   346,
     347,   348,     0,   349,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,   516,   368,     0,   369,     0,
     517,   371,   372,   373,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,     0,   383,   384,
     385,   386,   387,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   397,   156,   157,   158,
     159,   160,   161,   162,   163,   539,   164,   165,     0,     0,
       0,     0,     0,     0,     0,   166,   167,     0,   168,     0,
     169,   170,   171,     0,   172,     0,   173,   174,     0,   175,
     176,   177,   178,     0,     0,   179,   180,   181,   182,     0,
     183,   184,   185,   186,   187,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   194,   195,     0,   196,   197,   198,
       0,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,   205,   206,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   216,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,     0,
     233,     0,   234,     0,     0,     0,   235,   236,     0,   237,
     238,     0,   239,     0,   240,   241,   242,     0,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,     0,   251,
     252,   253,   254,   255,   256,   257,     0,   258,     0,   259,
       0,     0,   260,     0,   261,   262,   263,   264,   265,     0,
     266,     0,   267,     0,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,     0,   283,     0,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,   293,   294,     0,   295,     0,
     296,   297,   298,   299,     0,     0,   300,     0,     0,     0,
     301,   302,     0,     0,   303,     0,     0,   304,   305,   306,
     307,     0,   308,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   540,     0,   345,   346,   347,   348,     0,   349,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   541,   368,     0,   369,     0,   542,   371,   372,   373,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,   387,   388,
       0,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   397,   156,   157,   158,   159,   160,   161,   162,
     163,   837,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,     0,
     168,   848,   169,   170,   171,     0,   172,     0,   173,   174,
       0,   175,   176,   177,   178,     0,     0,   179,   180,   181,
     849,     0,   183,   184,   185,   186,   187,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   194,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,   205,   206,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     216,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,     0,   233,     0,   234,     0,     0,     0,   235,   236,
       0,   237,   238,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
       0,   251,   252,   253,   254,   255,   256,   257,     0,   258,
       0,   259,     0,     0,   260,     0,   261,   262,   263,   264,
     265,     0,   266,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   532,   282,     0,   283,     0,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,   293,   294,     0,
     295,     0,   296,   297,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,   303,     0,     0,   304,
     305,   306,   307,     0,   308,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   533,   333,   334,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   840,     0,   345,   346,   347,   348,
       0,   349,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   535,   368,     0,   369,     0,   536,   371,
     372,   373,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
     387,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   397,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,  1934,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,   156,   157,   158,   159,   160,   161,   162,   163,     0,
     164,   165,     0,     0,     0,     0,     0,  1948,     0,   166,
     167,     0,   168,     0,   169,   170,   171,     0,   172,     0,
     173,   174,     0,   175,   176,   177,   178,     0,     0,   179,
     180,   181,   182,     0,   183,   184,   185,   186,   187,     0,
       0,   188,   189,   190,   191,     0,   192,   193,   194,   195,
       0,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,   205,   206,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   216,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,     0,   233,     0,   234,     0,     0,     0,
     235,   236,     0,   237,   238,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   258,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,   266,     0,   267,     0,     0,   268,
     269,   270,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,     0,   283,     0,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,   293,
     294,     0,   295,     0,   296,   297,   298,   299,     0,     0,
     300,     0,     0,     0,   301,   302,     0,     0,   303,     0,
       0,   304,   305,   306,   307,     0,   308,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   319,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   344,     0,   345,   346,
     347,   348,     0,   349,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,   367,   368,     0,   369,     0,
     370,   371,   372,   373,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,     0,   383,   384,
     385,   386,   387,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   397,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,     0,     0,     0,   166,   167,     0,   168,     0,
     169,   170,   171,     0,   172,     0,   173,   174,     0,   175,
     176,   177,   178,     0,     0,   179,   180,   181,   182,     0,
     183,   184,   185,   186,   187,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   194,   195,     0,   196,   197,   198,
       0,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,   205,   206,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   216,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,     0,
     233,     0,   234,     0,     0,     0,   235,   236,     0,   237,
     238,  1963,   239,     0,   240,   241,   242,     0,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,     0,   251,
     252,   253,   254,   255,   256,   257,     0,   258,     0,   259,
       0,     0,   260,     0,   261,   262,   263,   264,   265,     0,
     266,     0,   267,     0,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,     0,   283,     0,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,   293,   294,     0,   295,     0,
     296,   297,   298,   299,     0,     0,   300,     0,     0,     0,
     301,   302,     0,     0,   303,     0,     0,   304,   305,   306,
     307,     0,   308,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,     0,   345,   346,   347,   348,     0,   349,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   367,   368,     0,   369,     0,   370,   371,   372,   373,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,   387,   388,
       0,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   397,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,  2427,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,     0,
     168,     0,   169,   170,   171,     0,   172,     0,   173,   174,
       0,   175,   176,   177,   178,     0,     0,   179,   180,   181,
     182,     0,   183,   184,   185,   186,   187,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   194,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,   205,   206,   207,  2429,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     216,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,     0,   233,     0,   234,     0,     0,     0,   235,   236,
       0,   237,   238,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
       0,   251,   252,   253,   254,   255,   256,   257,     0,   258,
       0,   259,     0,     0,   260,     0,   261,   262,   263,   264,
     265,     0,   266,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,     0,   283,     0,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,   293,   294,     0,
     295,     0,   296,   297,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,   303,     0,     0,   304,
     305,   306,   307,     0,   308,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,     0,   345,   346,   347,   348,
       0,   349,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   367,   368,     0,   369,     0,   370,   371,
     372,   373,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
     387,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   397,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,  2431,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,   156,   157,   158,   159,   160,   161,   162,   163,     0,
     164,   165,     0,     0,     0,     0,     0,     0,     0,   166,
     167,     0,   168,     0,   169,   170,   171,     0,   172,     0,
     173,   174,     0,   175,   176,   177,   178,     0,     0,   179,
     180,   181,   182,     0,   183,   184,   185,   186,   187,     0,
       0,   188,   189,   190,   191,     0,   192,   193,   194,   195,
       0,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,   205,   206,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   216,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,     0,   233,     0,   234,     0,     0,     0,
     235,   236,     0,   237,   238,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   258,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,   266,     0,   267,     0,     0,   268,
     269,   270,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,     0,   283,     0,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,   293,
     294,     0,   295,     0,   296,   297,   298,   299,     0,     0,
     300,     0,     0,     0,   301,   302,     0,     0,   303,     0,
       0,   304,   305,   306,   307,     0,   308,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   319,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   344,     0,   345,   346,
     347,   348,     0,   349,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,   367,   368,     0,   369,     0,
     370,   371,   372,   373,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,  2657,   383,   384,
     385,   386,   387,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   397,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,     0,     0,     0,   166,   167,     0,   168,     0,
     169,   170,   171,     0,   172,     0,   173,   174,     0,   175,
     176,   177,   178,     0,     0,   179,   180,   181,   182,     0,
     183,   184,   185,   186,   187,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   194,   195,     0,   196,   197,   198,
       0,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,   205,   206,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   216,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,     0,
     233,     0,   234,     0,     0,     0,   235,   236,     0,   237,
     238,     0,   239,     0,   240,   241,   242,     0,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,     0,   251,
     252,   253,   254,   255,   256,   257,     0,   258,     0,   259,
       0,     0,   260,     0,   261,   262,   263,   264,   265,     0,
     266,     0,   267,     0,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,     0,   283,     0,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,   293,   294,     0,   295,     0,
     296,   297,   298,   299,     0,     0,   300,     0,     0,     0,
     301,   302,     0,     0,   303,     0,     0,   304,   305,   306,
     307,     0,   308,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,     0,   345,   346,   347,   348,     0,   349,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   367,   368,     0,   369,     0,   370,   371,   372,   373,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,   387,   388,
       0,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   397,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   441,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,     0,
     168,     0,   169,   170,   171,     0,   172,     0,   173,   174,
       0,   175,   176,   177,   178,     0,     0,   179,   180,   181,
     182,     0,   183,   184,   185,   186,   187,     0,     0,   188,
     189,   190,   191,     0,   192,   193,   194,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,   205,   206,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     216,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,   226,   227,   228,   229,     0,   230,   231,
     232,     0,   233,     0,   234,     0,     0,     0,   235,   236,
       0,   237,   238,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
       0,   251,   252,   253,   254,   255,   256,   257,     0,   258,
       0,   259,     0,     0,   260,     0,   261,   262,   263,   264,
     265,     0,   266,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,     0,   283,     0,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,   293,   294,     0,
     295,     0,   296,   297,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,   303,     0,     0,   304,
     305,   306,   307,     0,   308,   309,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,   319,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,     0,   345,   346,   347,   348,
       0,   349,   350,     0,   351,   352,   353,   354,   355,   356,
     357,   358,   359,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,   367,   368,     0,   369,     0,   505,   371,
     372,   373,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
     387,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,   397,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,   172,     0,   173,   174,     0,   175,   176,   177,
     178,     0,     0,   179,   180,   181,   182,     0,   183,   184,
     185,   186,   187,     0,     0,   188,   189,   190,   191,     0,
     192,   193,   194,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
     205,   206,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   216,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,   226,
     227,   228,   229,     0,   230,   231,   232,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,   238,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,   251,   252,   253,
     254,   255,   256,   257,     0,   258,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,   266,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
       0,   283,     0,   284,   285,   286,   287,   288,   289,   290,
     291,   292,     0,   293,   294,     0,   295,     0,   296,   297,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,   303,     0,     0,   304,   305,   306,   307,     0,
     308,   309,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,   319,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,   334,
     335,   336,   512,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,   346,   347,   348,     0,   349,   350,     0,
     351,   352,   353,   354,   355,   356,   357,   358,   359,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,   367,
     368,     0,   369,     0,   370,   371,   372,   373,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,   387,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
     397,   156,   157,   158,   159,   160,   161,   162,   163,     0,
     164,   165,     0,     0,     0,     0,     0,     0,     0,   166,
     167,     0,   168,     0,   169,   170,   171,     0,   172,     0,
     173,   174,     0,   175,   176,   177,   178,     0,     0,   179,
     180,   181,   182,     0,   183,   184,   185,   186,   187,     0,
       0,   188,   189,   190,   191,     0,   530,   193,   194,   195,
       0,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,   205,   206,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   216,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,   226,   227,   228,   229,     0,
     230,   231,   232,     0,   233,     0,   234,     0,     0,     0,
     235,   236,     0,   237,   238,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,   251,   252,   253,   254,   255,   256,   257,
       0,   258,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,   266,     0,   267,     0,     0,   268,
     269,   531,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   532,   282,     0,   283,     0,   284,
     285,   286,   287,   288,   289,   290,   291,   292,     0,   293,
     294,     0,   295,     0,   296,   297,   298,   299,     0,     0,
     300,     0,     0,     0,   301,   302,     0,     0,   303,     0,
       0,   304,   305,   306,   307,     0,   308,   309,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,   319,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   533,   333,   334,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   534,     0,   345,   346,
     347,   348,     0,   349,   350,     0,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,   535,   368,     0,   369,     0,
     536,   371,   372,   373,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,     0,   383,   384,
     385,   386,   387,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,   397,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,     0,     0,     0,   166,   167,     0,   168,     0,
     169,   170,   171,     0,   172,     0,   173,   174,     0,   175,
     176,   177,   178,     0,     0,   179,   180,   181,   182,     0,
     183,   184,   185,   186,   187,     0,     0,   188,   189,   190,
     191,     0,   192,   193,   194,   195,     0,   196,   197,   198,
       0,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,   205,   206,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   216,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,   226,   227,   228,   229,     0,   230,   231,   232,     0,
     233,     0,   234,     0,     0,     0,   235,   236,     0,   237,
     238,     0,   239,     0,   240,   241,   242,     0,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,     0,   251,
     252,   253,   254,   255,   256,   257,     0,   258,     0,   259,
       0,     0,   260,     0,   261,   262,   263,   264,   265,     0,
     266,     0,   267,     0,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     532,   282,     0,   283,     0,   284,   285,   286,   287,   288,
     289,   290,   291,   292,     0,   293,   294,     0,   295,     0,
     296,   297,   298,   299,     0,     0,   300,     0,     0,     0,
     301,   302,     0,     0,   303,     0,     0,   304,   305,   306,
     307,     0,   308,   309,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,   319,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   533,
     333,   334,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   840,     0,   345,   346,   347,   348,     0,   349,
     350,     0,   351,   352,   353,   354,   355,   356,   357,   358,
     359,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,   535,   368,     0,   369,     0,   536,   371,   372,   373,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,   387,   388,
       0,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,   397,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
     172,     0,   173,   174,     0,   175,   176,   177,   178,     0,
       0,   179,   180,   181,   182,     0,   183,   184,   185,   186,
     187,     0,     0,   188,   189,   190,   191,     0,   192,   193,
     194,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,   205,   206,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   216,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,   226,   227,   228,
     229,     0,   230,   231,   232,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,   238,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,   251,   252,   253,   254,   255,
     256,   257,     0,   258,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,   266,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,     0,   283,
       0,   284,   285,   286,   287,   288,   289,   290,   291,   292,
       0,   293,   294,     0,   295,     0,   296,   297,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
     303,     0,     0,   304,   305,   306,   307,     0,   308,   309,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
     319,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,   334,   335,   336,
    1184,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,   346,   347,   348,     0,   349,   350,     0,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,   367,   368,     0,
     369,     0,   370,   371,   372,   373,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,   387,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,   397,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,     0,
     168,     0,   169,   170,   171,     0,  1057,     0,  1058,  1059,
       0,   175,   176,   177,   178,     0,     0,   179,  1060,  1061,
     182,     0,   183,   184,   185,   186,     0,     0,     0,   188,
     189,   190,   191,     0,   192,   193,     0,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,  1062,  1063,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     763,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,     0,   227,   228,     0,     0,   230,   231,
    1064,     0,   233,     0,   234,     0,     0,     0,   235,   236,
       0,   237,     0,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,  1373,   246,   247,   248,   249,   250,     0,
       0,  1374,   252,   253,   254,   255,  1065,  1066,     0,  1067,
       0,   259,     0,     0,   260,     0,   261,   262,   263,   264,
     265,     0,     0,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,  1068,     0,  1069,     0,   284,   285,   286,
     287,   288,   289,   290,     0,   292,     0,   293,   294,     0,
     295,     0,     0,  1070,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,  1375,     0,     0,     0,
     305,   306,   307,     0,     0,     0,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,  1071,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,   333,     0,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,     0,   345,  1376,   347,   348,
       0,   349,  1073,     0,   351,   352,   353,   354,   355,   356,
     357,   358,     0,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,  1074,  1075,     0,   369,     0,   370,     0,
     372,     0,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
    1076,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,  1077,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,  1057,     0,  1058,  1059,     0,   175,   176,   177,
     178,     0,     0,   179,  1060,  1061,   182,     0,   183,   184,
     185,   186,     0,     0,     0,   188,   189,   190,   191,     0,
     192,   193,     0,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
    1062,  1063,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   763,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,     0,
     227,   228,     0,     0,   230,   231,  1064,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,     0,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,     0,   252,   253,
     254,   255,  1065,  1066,     0,  1067,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,     0,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,  1068,
       0,  1069,     0,   284,   285,   286,   287,   288,   289,   290,
    1399,   292,     0,   293,   294,     0,   295,     0,     0,  1070,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,     0,     0,     0,     0,   305,   306,   307,     0,
       0,     0,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,  1071,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,     0,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,  1072,   347,   348,     0,   349,  1073,     0,
     351,   352,   353,   354,   355,   356,   357,   358,     0,   360,
       0,   361,   362,     0,   363,   364,   365,   366,     0,  1074,
    1075,     0,   369,     0,   370,     0,   372,     0,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
       0,     0,   383,   384,   385,   386,  1076,   388,     0,   389,
     390,     0,     0,   391,   392,   393,   394,   395,   396,     0,
    1077,   156,   157,   158,   159,   160,   161,   162,   163,     0,
     164,   165,     0,     0,     0,     0,     0,     0,     0,   166,
     167,     0,   168,     0,   169,   170,   171,     0,  1057,     0,
    1058,  1059,     0,   175,   176,   177,   178,     0,     0,   179,
    1060,  1061,   182,     0,   183,   184,   185,   186,     0,     0,
       0,   188,   189,   190,   191,     0,   192,   193,     0,   195,
       0,   196,   197,   198,     0,   199,     0,     0,     0,     0,
       0,   200,   201,   202,   203,   204,  1062,  1063,   207,     0,
     208,     0,   209,   210,   211,   212,   213,     0,   214,     0,
       0,   215,   763,   217,   218,     0,   219,   220,   221,     0,
     222,     0,   223,   224,   225,     0,   227,   228,     0,     0,
     230,   231,  1064,     0,   233,     0,   234,     0,     0,     0,
     235,   236,     0,   237,     0,     0,   239,     0,   240,   241,
     242,     0,   243,   244,   245,     0,   246,   247,   248,   249,
     250,     0,     0,     0,   252,   253,   254,   255,  1065,  1066,
       0,  1067,     0,   259,     0,     0,   260,     0,   261,   262,
     263,   264,   265,     0,     0,     0,   267,     0,     0,   268,
     269,   270,     0,     0,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,  1068,     0,  1069,     0,   284,
     285,   286,   287,   288,   289,   290,  2138,   292,     0,   293,
     294,     0,   295,     0,     0,  1070,   298,   299,     0,     0,
     300,     0,     0,     0,   301,   302,     0,     0,     0,     0,
       0,     0,   305,   306,   307,     0,     0,     0,   310,   311,
     312,     0,   313,   314,   315,   316,   317,   318,  1071,   320,
       0,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,     0,   332,   333,     0,   335,   336,   337,   338,
     339,   340,   341,     0,   342,   343,   344,     0,   345,  1072,
     347,   348,     0,   349,  1073,     0,   351,   352,   353,   354,
     355,   356,   357,   358,     0,   360,     0,   361,   362,     0,
     363,   364,   365,   366,     0,  1074,  1075,     0,   369,     0,
     370,     0,   372,     0,     0,   374,   375,   376,   377,   378,
       0,     0,   379,   380,   381,   382,     0,     0,   383,   384,
     385,   386,  1076,   388,     0,   389,   390,     0,     0,   391,
     392,   393,   394,   395,   396,     0,  1077,   156,   157,   158,
     159,   160,   161,   162,   163,     0,   164,   165,     0,     0,
       0,     0,     0,     0,     0,   166,   167,     0,   168,     0,
     169,   170,   171,     0,  1057,     0,  1058,  1059,     0,   175,
     176,   177,   178,     0,     0,   179,  1060,  1061,   182,     0,
     183,   184,   185,   186,     0,     0,     0,   188,   189,   190,
     191,     0,   192,   193,     0,   195,     0,   196,   197,   198,
       0,   199,     0,     0,     0,     0,     0,   200,   201,   202,
     203,   204,  1062,  1063,   207,     0,   208,     0,   209,   210,
     211,   212,   213,     0,   214,     0,     0,   215,   763,   217,
     218,     0,   219,   220,   221,     0,   222,     0,   223,   224,
     225,     0,   227,   228,     0,     0,   230,   231,  1064,     0,
     233,     0,   234,     0,     0,     0,   235,   236,     0,   237,
       0,     0,   239,     0,   240,   241,   242,     0,   243,   244,
     245,     0,   246,   247,   248,   249,   250,     0,     0,     0,
     252,   253,   254,   255,  1065,  1066,     0,  1067,     0,   259,
       0,     0,   260,     0,   261,   262,   263,   264,   265,     0,
       0,     0,   267,     0,     0,   268,   269,   270,     0,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,  1068,     0,  1069,     0,   284,   285,   286,   287,   288,
     289,   290,     0,   292,     0,   293,   294,     0,   295,  1367,
       0,  1070,   298,   299,     0,     0,   300,     0,     0,     0,
     301,   302,     0,     0,     0,     0,     0,     0,   305,   306,
     307,     0,     0,     0,   310,   311,   312,     0,   313,   314,
     315,   316,   317,   318,  1071,   320,     0,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,     0,   332,
     333,     0,   335,   336,   337,   338,   339,   340,   341,     0,
     342,   343,   344,     0,   345,  1376,   347,   348,     0,   349,
    1073,     0,   351,   352,   353,   354,   355,   356,   357,   358,
       0,   360,     0,   361,   362,     0,   363,   364,   365,   366,
       0,  1074,  1075,     0,   369,     0,   370,     0,   372,     0,
       0,   374,   375,   376,   377,   378,     0,     0,   379,   380,
     381,   382,     0,     0,   383,   384,   385,   386,  1076,   388,
       0,   389,   390,     0,     0,   391,   392,   393,   394,   395,
     396,     0,  1077,   156,   157,   158,   159,   160,   161,   162,
     163,     0,   164,   165,     0,     0,     0,     0,     0,     0,
       0,   166,   167,     0,   168,     0,   169,   170,   171,     0,
    1057,     0,  1058,  1059,     0,   175,   176,   177,   178,     0,
       0,   179,  1060,  1061,   182,     0,   183,   184,   185,   186,
       0,     0,     0,   188,   189,   190,   191,     0,   192,   193,
       0,   195,     0,   196,   197,   198,     0,   199,     0,     0,
       0,     0,     0,   200,   201,   202,   203,   204,  1062,  1063,
     207,     0,   208,     0,   209,   210,   211,   212,   213,     0,
     214,     0,     0,   215,   763,   217,   218,     0,   219,   220,
     221,     0,   222,     0,   223,   224,   225,     0,   227,   228,
       0,     0,   230,   231,  1064,     0,   233,     0,   234,     0,
       0,     0,   235,   236,     0,   237,     0,     0,   239,     0,
     240,   241,   242,     0,   243,   244,   245,     0,   246,   247,
     248,   249,   250,     0,     0,     0,   252,   253,   254,   255,
    1065,  1066,     0,  1067,     0,   259,     0,     0,   260,     0,
     261,   262,   263,   264,   265,     0,     0,     0,   267,     0,
       0,   268,   269,   270,     0,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,  1068,     0,  1069,
       0,   284,   285,   286,   287,   288,   289,   290,     0,   292,
       0,   293,   294,     0,   295,     0,     0,  1070,   298,   299,
       0,     0,   300,     0,     0,     0,   301,   302,     0,     0,
       0,     0,     0,     0,   305,   306,   307,     0,     0,     0,
     310,   311,   312,     0,   313,   314,   315,   316,   317,   318,
    1071,   320,     0,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,     0,   332,   333,     0,   335,   336,
     337,   338,   339,   340,   341,     0,   342,   343,   344,     0,
     345,  1072,   347,   348,     0,   349,  1073,     0,   351,   352,
     353,   354,   355,   356,   357,   358,     0,   360,     0,   361,
     362,     0,   363,   364,   365,   366,     0,  1074,  1075,     0,
     369,     0,   370,     0,   372,     0,     0,   374,   375,   376,
     377,   378,     0,     0,   379,   380,   381,   382,     0,     0,
     383,   384,   385,   386,  1076,   388,     0,   389,   390,     0,
       0,   391,   392,   393,   394,   395,   396,     0,  1077,   156,
     157,   158,   159,   160,   161,   162,   163,     0,   164,   165,
       0,     0,     0,     0,     0,     0,     0,   166,   167,     0,
     168,     0,   169,   170,   171,     0,  1057,     0,  1058,  1059,
       0,   175,   176,   177,   178,     0,     0,   179,  1060,  1061,
     182,     0,   183,   184,   185,   186,     0,     0,     0,   188,
     189,   190,   191,     0,   192,   193,     0,   195,     0,   196,
     197,   198,     0,   199,     0,     0,     0,     0,     0,   200,
     201,   202,   203,   204,  1062,  1063,   207,     0,   208,     0,
     209,   210,   211,   212,   213,     0,   214,     0,     0,   215,
     763,   217,   218,     0,   219,   220,   221,     0,   222,     0,
     223,   224,   225,     0,   227,   228,     0,     0,   230,   231,
    1064,     0,   233,     0,   234,     0,     0,     0,   235,   236,
       0,   237,     0,     0,   239,     0,   240,   241,   242,     0,
     243,   244,   245,     0,   246,   247,   248,   249,   250,     0,
       0,     0,   252,   253,   254,   255,  1065,  1066,     0,  1067,
       0,   259,     0,     0,   260,     0,   261,   262,   263,   264,
     265,     0,     0,     0,   267,     0,     0,   268,   269,   270,
       0,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,  1068,     0,  1069,     0,   284,   285,   286,
     287,   288,   289,   290,     0,   292,     0,   293,   294,     0,
     295,     0,     0,  1070,   298,   299,     0,     0,   300,     0,
       0,     0,   301,   302,     0,     0,     0,     0,     0,     0,
     305,   306,   307,     0,     0,     0,   310,   311,   312,     0,
     313,   314,   315,   316,   317,   318,  1071,   320,     0,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
       0,   332,   333,     0,   335,   336,   337,   338,   339,   340,
     341,     0,   342,   343,   344,     0,   345,  1376,   347,   348,
       0,   349,  1073,     0,   351,   352,   353,   354,   355,   356,
     357,   358,     0,   360,     0,   361,   362,     0,   363,   364,
     365,   366,     0,  1074,  1075,     0,   369,     0,   370,     0,
     372,     0,     0,   374,   375,   376,   377,   378,     0,     0,
     379,   380,   381,   382,     0,     0,   383,   384,   385,   386,
    1076,   388,     0,   389,   390,     0,     0,   391,   392,   393,
     394,   395,   396,     0,  1077,   156,   157,   158,   159,   160,
     161,   162,   163,     0,   164,   165,     0,     0,     0,     0,
       0,     0,     0,   166,   167,     0,   168,     0,   169,   170,
     171,     0,  1057,     0,  1058,  1059,     0,   175,   176,   177,
     178,     0,     0,   179,  1060,  1061,   182,     0,   183,   184,
     185,   186,     0,     0,     0,   188,   189,   190,   191,     0,
     192,   193,     0,   195,     0,   196,   197,   198,     0,   199,
       0,     0,     0,     0,     0,   200,   201,   202,   203,   204,
    1062,  1063,   207,     0,   208,     0,   209,   210,   211,   212,
     213,     0,   214,     0,     0,   215,   763,   217,   218,     0,
     219,   220,   221,     0,   222,     0,   223,   224,   225,     0,
     227,   228,     0,     0,   230,   231,  1064,     0,   233,     0,
     234,     0,     0,     0,   235,   236,     0,   237,     0,     0,
     239,     0,   240,   241,   242,     0,   243,   244,   245,     0,
     246,   247,   248,   249,   250,     0,     0,     0,   252,   253,
     254,   255,  1065,  1066,     0,  1067,     0,   259,     0,     0,
     260,     0,   261,   262,   263,   264,   265,     0,     0,     0,
     267,     0,     0,   268,   269,   270,     0,     0,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,  1068,
       0,  1069,     0,   284,   285,   286,   287,   288,   289,   290,
       0,   292,     0,   293,   294,     0,   295,     0,     0,  1070,
     298,   299,     0,     0,   300,     0,     0,     0,   301,   302,
       0,     0,     0,     0,     0,     0,   305,   306,   307,     0,
       0,     0,   310,   311,   312,     0,   313,   314,   315,   316,
     317,   318,  1071,   320,     0,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,     0,   332,   333,     0,
     335,   336,   337,   338,   339,   340,   341,     0,   342,   343,
     344,     0,   345,     0,   347,   348,     0,   349,  1073,     0,
     351,   352,   353,   354,   355,   356,   357,   358,     0,   360,
     445,   361,   362,     0,   363,   364,   365,   366,     0,  1074,
    1075,   446,   369,     0,   370,     0,   372,     0,     0,   374,
     375,   376,   377,   378,     0,     0,   379,   380,   381,   382,
     447,     0,   383,   384,   385,   386,  1076,   388,     0,   389,
     390,     0,   414,   391,   392,   393,   394,   395,   396,   448,
    1077,     0,     0,   415,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   449,     0,     0,     0,     0,
       0,     0,   416,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   450,     0,     0,     0,     0,     0,   417,
       0,  -447,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   418,     0,     0,
       0,     0,   451,   419,     0,     0,     0,   452,     0,     0,
       0,     0,     0,     0,     0,   420,     0,     0,     0,     0,
     453,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -563,   421,     0,     0,     0,   422,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  -549,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -373,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   423,   454,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   455,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   456,     0,     0,   424,   457,
     425,   458,     0,     0,     0,     0,   459,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  -373,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   460,   461,     0,     0,     0,     0,   426,     0,     0,
       0,  -563,     0,   427,   462,     0,     0,     0,     0,   463,
       0,     0,     0,     0,     0,     0,     0,     0,   464,     0,
       0,     0,     0,     0,     0,     0,     0,   465,     0,     0,
       0,     0,     0,     0,   428,   429,     0,   430,     0,     0,
       0,     0,     0,     0,     0,     0,   431,     0,     0,     0,
     432,   433,     0,     0,     0,   434,     0,     0,     0,     0,
     435
};

static const short yycheck[] =
{
       0,   501,   694,    43,   424,    12,   146,   670,     0,  1135,
     139,  1125,   141,  1113,   677,    39,  1153,   143,   452,   138,
     629,   147,   456,   142,   153,   790,   741,    20,   154,   533,
     464,   536,   634,    26,   454,     0,     0,   632,   617,     0,
    1090,    32,   707,    43,    21,   545,   625,  1459,  1537,  1245,
       7,     8,  1705,  1746,   586,    32,    13,    14,   705,   851,
     632,    18,  1580,    20,    21,  1718,   418,  1054,   908,    26,
     600,    28,    41,    30,    31,    32,  1556,    34,  1187,    36,
      37,  1527,  1528,   662,     0,  1102,  1102,   666,  1102,  1102,
    1924,  1898,   885,   672,  1102,  1102,   530,   449,    38,   678,
     471,     0,    19,     0,    24,    38,    24,    18,  1381,  1382,
     531,   596,    11,   534,    11,  1072,   762,     0,    81,  1522,
    2041,   752,   753,     0,  2344,  1316,    11,  1199,  1200,  1201,
       0,     5,    17,     0,    23,  1207,  1127,    45,     0,  1211,
    1212,    86,   807,    86,    52,   776,  1218,    23,    86,    57,
      81,    55,    45,    37,    86,  1227,    43,  1229,   120,    11,
       7,     7,   137,    38,    57,    17,  2565,  2271,    81,  2273,
    1242,    97,  1244,     0,   120,   144,    95,   211,   154,    62,
     149,   150,   139,   140,   141,    66,   143,   123,   145,   146,
     147,   148,    24,    49,   151,   152,   153,   154,   105,   122,
     704,    81,   105,   148,   209,   148,   121,     0,   209,   117,
     148,   241,   122,    12,    43,    52,   148,   215,  1885,  1886,
    1887,  1888,   257,    81,   117,   109,   646,    86,   214,   144,
      24,   105,    24,    83,    19,   127,    95,   173,   156,   157,
     121,   124,    18,   852,   159,    83,   205,   104,    95,    95,
    2596,   135,   167,  1045,    81,   270,   201,   202,    83,   202,
    1247,    11,    11,   144,   158,   120,    84,    17,    22,    62,
     331,   224,    24,    84,   257,   224,  2655,  2018,   159,   881,
     340,   138,    14,    15,   424,   203,   167,   200,   722,  2509,
     885,   420,   265,   216,    31,   414,   422,    43,   830,   359,
     426,  2042,   265,   364,   433,   143,    88,   188,   742,   435,
    1075,   251,   125,   885,   454,  2694,   134,   270,   251,   120,
     265,   270,   265,   134,   359,   233,   445,   265,  1454,   359,
     364,   124,   451,   265,   317,  1385,   334,   352,   467,   254,
     233,    86,    91,    45,   848,   239,   955,   205,   261,   259,
     149,   270,  1044,  2699,   359,   205,   212,   293,   359,   295,
     323,   284,   200,   255,  1853,   272,  2765,   251,   252,   272,
    1561,   230,   317,   254,   317,   200,   251,   224,     8,   317,
     264,   340,   136,   257,  2488,   317,   359,  2308,   364,   364,
     309,   327,   277,    23,   278,  2615,   359,    27,   272,   288,
     246,   285,   364,   329,   156,   313,   310,  1479,  1522,   340,
      40,    41,   288,   344,   359,   117,   359,  1063,   364,  1376,
     313,   359,   907,   270,   364,   277,  1118,  2120,  2121,  1060,
    1061,   364,   350,   351,   352,   353,   354,   355,  1069,  1851,
    1852,   361,   359,   361,   328,   344,   357,   344,  1860,   360,
    2117,   362,   351,   364,   351,  1646,   337,  1448,   415,  1054,
     340,   418,   299,   420,   363,   422,   363,   424,   889,   426,
     427,   428,   359,   602,   431,   444,   433,   360,   435,   472,
     363,   474,  1054,   476,   441,  1132,   363,   480,  1761,   446,
    1763,  1138,   449,   363,   865,   452,   363,   454,   299,   456,
     457,   363,   922,   924,   461,   462,   646,   464,   138,   364,
     467,   340,   142,   340,   144,    16,  1588,   636,  1714,   149,
     150,  1186,  1187,   355,   643,   158,   655,   277,   949,   361,
     359,   233,  1197,  1198,  1937,  2202,   363,   319,  1941,    81,
     669,  1381,  1945,    81,  1209,  1210,   331,   332,    38,    76,
     507,   364,   509,  1346,   683,   512,   556,    29,  1223,  1224,
     352,   353,   354,   355,   556,    86,   685,   360,   697,   361,
     363,   181,   317,   530,   531,   132,   533,   534,    91,   325,
     123,   357,  1213,  1214,   360,   265,   362,   121,   364,   156,
    1704,   556,   556,    11,   340,   556,  2377,  1870,   350,   351,
     352,   353,   354,   355,    75,   132,   239,    62,   170,   361,
    1114,   313,   120,   359,    62,  1280,   410,  1409,    16,   103,
    1412,  1413,    84,   115,   181,    80,   950,   148,   154,    76,
     173,    86,  1234,   358,   660,    85,  2470,   115,    45,   364,
     556,  1713,  2449,   323,   292,   602,    57,   214,   299,   211,
    1923,   106,  1247,   263,   181,   156,  1156,   184,   200,  2440,
     291,   133,   200,   257,   246,   137,   692,   249,   152,   140,
      81,    56,   134,    91,    81,  1247,   124,  1279,   170,   359,
     340,    81,   639,    31,   332,   132,   280,   200,   270,   646,
     647,   225,   170,   148,   651,   357,   209,   169,   655,    85,
     362,    16,   192,   729,   661,   155,   263,   111,  1758,   709,
     239,   505,   669,   364,    99,   216,   299,   709,  1152,   211,
    1154,  1155,   331,  1157,   724,    73,   683,   684,  1800,   686,
     524,   364,   724,   211,   181,   219,   263,   184,   532,   281,
     697,   275,   284,   272,   709,   709,   746,   704,   709,   331,
     293,  1346,   295,   153,  2166,   364,  2245,  2538,   156,  2171,
    2172,   251,   719,   346,   764,   722,   723,   807,   200,   155,
     727,   340,  1096,   173,  1346,   893,  1100,     9,   302,   362,
     270,   738,   922,   413,   414,   742,  1110,   744,   317,   200,
     359,   176,  1892,   200,   794,    27,   317,     0,   205,   147,
    1220,   205,   274,  1875,  1876,   209,   238,   807,   335,   266,
     265,  2257,  2292,   931,   444,   445,   263,   302,   216,   290,
     291,   451,   354,  1937,  1961,   325,   233,  1941,   312,   232,
     362,  1945,   359,   224,   291,   242,  1415,  1416,    52,   224,
     340,   156,   343,    57,   192,   346,   347,   348,   333,   350,
     351,   352,   353,   354,   355,   246,   882,  1549,  2261,   359,
     361,   232,   317,  2266,   494,    36,  1396,  2270,   121,    19,
     327,   501,   872,   873,    74,   875,  1893,  1893,   214,  1893,
    1893,   511,  1869,  1992,  1318,  1893,  1893,    37,   335,   282,
     283,   848,  1539,  1540,   363,   921,   853,   854,   261,   302,
     224,   216,   306,    74,   359,   862,   313,   292,   239,   294,
     314,   103,   266,   409,   262,   545,   247,  1046,   548,   354,
    1760,  1050,   246,   115,   293,   249,   295,   362,   558,   292,
     333,   302,   889,  2005,   326,  1582,   267,   291,   338,   339,
     340,   437,   224,  2015,   344,   343,   270,   340,   346,   347,
     348,   351,   350,   351,   352,   353,   354,   355,  1392,   109,
     152,   339,   333,   361,   246,   922,   344,   924,   139,   332,
     344,   593,   340,   327,  1639,  1101,   945,   224,   170,   214,
     180,  2499,   182,    19,   358,   135,  2398,    31,   270,  2401,
    2402,   359,   949,    37,  1659,   617,  1348,    66,  1530,   246,
     224,    37,   359,   625,  2200,   635,   636,   331,  2661,   180,
    1139,   182,   162,   643,   358,   809,  1042,  2710,  1810,   211,
     364,   192,   246,   270,  1816,  1817,   820,   219,   343,  1694,
     304,   346,   347,   348,  1153,   350,   351,   352,   353,   354,
     355,  1706,  1707,   224,  2737,   224,   361,  2700,   224,   679,
     680,   681,   121,     8,    76,   685,   344,   851,    19,     5,
     331,   332,   340,   351,    10,   246,   120,   246,    23,  1095,
     246,    17,    27,   109,   236,   144,    37,   340,  1870,   250,
    1220,   359,  2485,   137,  2573,    40,   340,    33,   359,  1046,
     159,   721,   248,  1050,   270,  1052,   359,   359,   167,   135,
    1747,  1127,   252,   246,   360,   359,   249,   363,   279,  1135,
     132,   741,   331,   332,   264,    37,  2188,  1719,   340,   188,
     312,   340,   331,   332,  1164,  1125,   162,   270,   278,    42,
      43,   340,  2204,   331,   332,   285,  1143,   359,    59,  2551,
     359,  2337,   331,   332,  1101,   214,  1103,  2261,   109,     7,
     359,    25,  2266,   340,   932,    13,  2270,  1114,   936,   181,
     938,   359,   184,    31,  1164,  2672,  2673,   140,   360,  2676,
      44,  2678,   359,  1173,   135,   162,  1133,   355,   328,  1826,
    1827,  1173,  1139,   361,   156,   254,   360,   109,   331,   363,
    1181,    49,   360,  2243,   149,  1152,   364,  1154,  1155,    67,
    1157,   162,   359,  1180,    62,   344,  1711,  1164,  1173,  1173,
     732,   733,  1173,   135,   360,   346,   252,  1343,   364,    87,
     357,  1347,   359,  1180,  1181,   362,   357,  1184,   264,   235,
     860,   362,    90,   339,   102,  1361,   214,    95,   344,   360,
      98,   263,   278,   364,  1270,   351,   331,   332,   116,   285,
     319,  1251,   360,   360,   359,  1675,   364,   364,   872,   873,
      45,   875,   130,  1220,   293,  1391,   295,  1393,   337,   360,
    1985,   360,    57,   364,  1869,   364,   560,  1403,   562,   360,
    1406,  2046,  2047,   364,   352,   353,   354,   355,   331,   332,
       0,   252,   328,   361,  1728,   331,    81,  1869,    83,    25,
     360,  1222,   336,   264,   364,   158,  1953,  1433,  1955,  1956,
    1436,   362,  1438,   335,   217,   945,   338,   278,   340,   360,
     156,  1450,   360,   364,   285,   360,   364,  1456,  1457,   364,
     252,   293,   360,   295,   360,  2531,   364,   205,   364,  1986,
     217,  2441,   264,   360,   212,   214,   360,   364,  1782,  1951,
     364,   360,    62,  1274,  2468,   364,   278,  1638,   143,  1640,
    1641,  1318,   336,   285,  1798,   360,   224,   328,  1802,   364,
     238,  2485,   299,  1975,   872,   873,    91,   875,   350,   351,
     352,   353,   354,   355,   331,   332,  1343,  1344,  2053,   361,
    1347,  1348,   214,   103,   360,  1352,   111,   357,   364,   359,
     293,  1358,   295,    25,  1361,   115,   328,  1364,   336,   360,
     120,   158,   270,   364,   124,   200,   270,   127,  1922,   129,
     205,   119,  1448,   360,   360,  2562,   360,   364,  1454,   360,
     364,  1729,  1730,   364,  1391,  1392,  1393,  1394,  1395,    35,
    1397,  1398,   152,    16,  1470,   352,  1403,    57,   233,  1406,
    1407,  1408,   360,  2055,   360,   360,   364,   242,   413,   364,
     170,   362,   360,  1420,  1421,  1422,   364,  1424,  1425,  1426,
    1427,  1428,  1429,  1430,   360,   359,  1433,  1434,   364,  1436,
    1437,  1438,  1522,  1490,   360,   200,   359,  2096,   364,  1942,
     205,  1944,   359,  1450,   209,  1125,  1731,  1732,   360,  1456,
    1457,   211,   364,  1297,   359,   246,  1136,  1137,  1534,   219,
     359,   362,   270,   123,   350,   351,   352,   353,   354,   355,
     360,   214,  1522,  1153,   364,   361,  1156,   360,   313,   360,
     140,   364,   168,   364,  1491,  1675,  1222,   360,  1168,   494,
     360,   364,  2656,   360,   364,  1571,   501,   364,   359,  1506,
    1984,   360,   156,  1510,   360,   364,   511,   360,   364,   208,
     360,   364,   162,   173,   364,   359,  1523,    45,   360,  2669,
    1527,  1528,   364,   359,   331,    19,    25,    25,   224,   162,
    1537,   291,   319,   156,   359,   364,  1726,   359,   137,  2252,
     545,   306,    19,   548,   214,   214,   116,  1554,   168,   314,
     115,   115,   312,   558,   359,   340,   340,   156,   218,   359,
     359,   359,   216,   359,   359,   229,   359,   359,   340,  1659,
     330,   156,   340,  1580,   340,   222,   359,   359,  2275,   359,
     359,     7,   359,   340,   359,   359,   359,    13,   359,   359,
    1766,    19,   359,   216,   168,   359,  1276,   357,  2720,  2721,
     360,   359,   362,   363,   359,   364,   115,   299,   299,  1659,
     362,  1455,   299,  2789,   299,  1459,   299,   216,   214,   264,
     299,   205,  1672,    49,   318,  1699,  1700,   292,   299,   364,
     635,   216,   299,   293,   261,   295,    62,  1317,   299,   319,
     299,  1648,  1692,   299,   304,    52,   299,   214,   300,   304,
    1621,  1622,   364,   313,  1704,  1626,  1500,   299,   299,  2143,
    2144,   304,   299,   261,    90,   299,   359,   214,  1675,    95,
    1514,     7,    98,   214,   679,   680,   681,    13,    14,    19,
    1870,   359,    18,   156,    20,  1656,  1657,   331,    18,   343,
      26,   359,    28,   347,   348,   325,   350,   351,   352,   353,
     354,   355,  1778,     9,  1780,    81,   327,   361,  1715,   176,
    1717,   359,   115,   214,    19,   330,   721,   262,    19,   359,
     343,  1728,   319,   346,   347,   348,   299,   350,   351,   352,
     353,   354,   355,   359,   359,   323,   741,   359,   361,  1746,
     204,   120,   359,  1819,   343,   297,   329,   346,   347,   348,
     344,   350,   351,   352,   353,   354,   355,   297,   343,  1766,
    1767,  1768,   361,   364,   270,   350,   351,   352,   353,   354,
     355,   361,   297,  1849,   120,  1782,   361,  2406,     0,   359,
     359,   299,   302,   364,   360,   152,   364,  1963,   215,   209,
     170,  1798,  1961,    56,   360,  1802,   359,   160,   224,   160,
     359,  2498,   214,   205,   205,   205,   220,  1487,   299,   145,
     299,  1987,   148,  2528,   261,   151,   152,   360,   364,    35,
     246,  2000,   364,   291,   246,  1901,   239,    19,   156,  1509,
      16,   156,   319,   156,   212,  1515,  1912,    18,    24,   346,
      62,   357,  1522,    29,   270,   357,  1853,   299,   156,   299,
     299,   299,   214,   115,   218,   860,  1863,    19,   128,   131,
     131,   115,   364,   360,   360,     8,  1916,  1917,   291,   364,
     125,   183,   214,   120,  1916,  1917,   364,    35,   360,   360,
    1930,   103,   360,   336,   360,  1621,  1622,  1937,  1930,   360,
    1626,  1941,   364,   115,   360,  1945,   336,   360,   120,  2369,
     360,   360,   124,   360,   360,   127,   360,   129,   216,   360,
     120,  1918,   336,   299,   360,  1922,  2613,   360,   359,   170,
    1656,  1657,  1998,  1973,  1931,   359,   359,  1934,   360,   359,
     152,  1973,   146,   224,   170,   316,   216,   205,   226,   226,
     299,  1948,   299,   360,   156,   360,   357,   133,   170,   156,
     156,   137,   360,  1797,   263,   184,  1963,   214,  2034,   170,
     309,   360,   346,   248,   359,   106,   360,  1974,   364,   212,
     156,   157,  2048,   299,   212,   319,   359,  1984,   360,   214,
    1987,   159,   159,   169,   221,   359,   217,   360,   105,   211,
     105,  2081,   357,  2000,  2478,  2071,   101,   219,   120,   115,
     360,    43,   127,   361,  2488,   360,   360,  1851,  1852,  2083,
    2084,  2085,  2086,   360,   200,   360,  1860,   203,  1698,   360,
     359,   359,   299,  1867,  1704,   299,  1870,   261,   146,  2771,
     216,  2081,   218,  2583,   309,   343,   156,   156,   346,   347,
     348,   299,   350,   351,   352,   353,   354,   355,   156,   299,
     299,  2022,   359,   361,  2025,  2026,  2027,  2028,  2029,  2030,
    2031,  2032,  2033,   343,  2035,   360,   346,   347,   348,   291,
     350,   351,   352,   353,   354,   355,   358,  1921,   160,   415,
     358,  2157,    19,   115,   218,   142,   319,    19,   274,    19,
     312,   364,   428,   359,   322,   431,   159,   124,  2141,   359,
     359,   120,  2073,  2074,   360,   441,   359,   359,   330,   120,
     446,   209,   360,  2120,  2121,   360,   360,   359,    35,  2603,
     360,   457,   129,   319,   309,   461,   462,   205,   205,    88,
     156,  1136,  1137,    83,   143,   357,  2143,  2144,   360,   358,
     362,   363,   122,   122,   120,   140,  2222,   319,    96,   106,
     359,  1156,   360,    82,  2161,  2162,    82,   343,   161,   120,
     346,   347,   348,  1168,   350,   351,   352,   353,   354,   355,
     299,   507,   359,   509,    19,   361,   217,   359,   364,  2369,
     360,   242,    19,   205,   237,   179,   292,   212,   212,    19,
     115,   214,    54,   214,  2370,   292,    96,   214,   214,  2206,
    2207,  2208,   359,  2210,    92,  2381,   205,  2383,   257,    86,
     317,  2261,   364,   258,   258,  2059,  2266,  1897,   364,   106,
    2270,     6,  2229,  1903,  2231,   237,   359,  2234,   360,  2236,
    2237,   241,   556,  1489,  1715,   854,  2234,  1432,  2245,  1820,
    2211,  1420,  2231,  2154,  1743,  2434,  2706,  1863,  2734,  2690,
    2257,  2689,  2259,  2715,   138,  2580,  2646,  1937,   142,  2236,
    2278,  1941,  1111,  2393,  2271,  1945,  2273,  2463,  2344,  1482,
    2801,  1276,  2413,  2775,  2412,  2282,  2751,  1097,  1869,  2749,
    1960,  1961,  2802,    26,   502,   723,  2022,  2656,  2282,  2025,
    2026,  2027,  2028,  2029,  2030,  2031,  2032,  2033,  2142,  2035,
    1760,  1382,  2250,   639,  2148,  1985,  2150,  1381,  2470,  2632,
    2764,   647,  1317,  2800,   544,   651,    19,  1317,   709,  1974,
    2460,   709,  2166,  1723,   880,   661,   879,  2171,  2172,  1314,
    2373,  1733,  1330,  1704,  1522,    24,  2486,  2073,  2074,  1943,
      29,  1469,  2058,  2229,  1870,  1672,  1590,  1670,  1277,  1717,
     686,  2497,   684,    -1,  2286,    -1,    -1,    -1,    -1,    -1,
    2204,  2205,  2369,  2370,  2371,  2372,    -1,    -1,  2339,  2340,
      -1,    -1,    -1,    -1,  2381,  2382,  2383,    -1,  2421,    -1,
      -1,    -1,    -1,   719,    -1,    -1,    -1,  2394,    -1,    -1,
      -1,   727,    -1,  2562,    -1,  2445,    -1,    -1,    -1,    -1,
    2244,    -1,    -1,  2445,    -1,    -1,    -1,    -1,  2088,  2089,
      -1,    -1,  2591,    -1,  2490,    -1,    -1,    -1,    -1,    -1,
      -1,   660,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2509,    -1,  2485,    -1,    -1,    -1,    -1,
    2516,    -1,    -1,    -1,   133,    -1,    -1,    -1,   137,    -1,
      -1,    -1,    -1,   692,    -1,    -1,  2463,    -1,    -1,    -1,
      -1,  2468,    -1,    -1,    -1,    -1,    -1,   156,   157,    -1,
      -1,  2478,    -1,    -1,    -1,  2211,  2645,    -1,  2657,    -1,
     169,  2488,  1487,    -1,    -1,  2492,    -1,    -1,    -1,    -1,
     729,    -1,  2499,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1509,    -1,    -1,    -1,    -1,    -1,
    1515,   200,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,
      -1,  2201,  2366,    -1,    -1,    -1,    -1,   216,    -1,    -1,
     414,    -1,    -1,    -1,    -1,    -1,  2507,  2217,    -1,  2615,
      -1,    -1,    -1,    -1,    -1,  2225,  2517,  2518,    -1,    -1,
      -1,    -1,    -1,    -1,  2398,    -1,    -1,  2401,  2402,  2749,
      -1,   445,    -1,    -1,    -1,    -1,  2573,   451,  2575,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2748,
      -1,  2261,    -1,    -1,  2591,   274,  2266,    -1,  2595,  2596,
    2270,    -1,    -1,    -1,    -1,    -1,  2603,    -1,    -1,    -1,
      -1,    -1,    -1,  2339,  2340,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   851,    -1,    -1,    -1,    -1,  2786,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2469,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2640,    10,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   882,    -1,   529,  2722,    -1,    -1,  2656,
    2657,    -1,    -1,    -1,   343,    -1,    -1,   346,   347,   348,
      -1,   350,   351,   352,   353,   354,   355,    -1,    -1,   908,
      -1,    -1,   361,    -1,    -1,    -1,  2356,  2357,  2358,  2359,
      -1,    -1,   921,    59,    -1,    -1,    -1,  2777,    -1,    -1,
    2697,    -1,  2699,  1698,    -1,    -1,    -1,    -1,    -1,    75,
      -1,    -1,    -1,  2710,    -1,    -1,    -1,  2551,    -1,    -1,
      -1,  2555,    -1,  2789,  2764,    -1,    -1,    93,  2725,  1621,
    1622,    -1,    -1,    -1,  1626,    -1,    -1,  2777,    -1,    -1,
    2737,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  2749,    -1,    -1,  2589,   122,   631,   632,    -1,
    2800,   127,   636,    -1,  1656,  1657,    -1,    -1,    -1,   643,
      -1,    -1,    -1,    -1,   140,    -1,    -1,  1103,  2775,    -1,
      -1,  2507,    -1,    -1,    -1,    -1,   660,    -1,    -1,    -1,
      -1,  2517,  2518,    -1,    -1,    -1,   162,    -1,  2468,    -1,
      -1,    -1,    -1,    -1,  2801,    -1,    -1,  1133,    -1,    -1,
      -1,   685,    -1,  1042,    -1,  2485,    -1,    -1,   692,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,
      -1,    -1,    -1,   707,    -1,    -1,    24,    -1,    -1,    -1,
      -1,    29,    -1,  1072,    -1,    -1,    -1,    -1,    -1,    -1,
     216,    -1,    -1,    -1,    -1,   729,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,   660,  1095,    -1,    -1,    29,
      -1,    -1,    -1,  2707,  2708,    -1,   750,  2711,  2712,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   255,
      -1,  2725,  2562,    -1,  2564,   261,    -1,   692,  1127,    -1,
     266,    -1,  1897,   777,    -1,    -1,  1135,    -1,  1903,    -1,
      -1,    -1,    -1,  2583,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   291,   292,    -1,    -1,  2599,
     804,   805,    -1,   807,   729,    -1,    -1,    -1,   304,    -1,
      -1,  2775,    -1,   309,    -1,   133,    -1,    -1,    -1,   137,
      -1,   825,   318,    -1,    -1,   115,  2626,    -1,    -1,    -1,
     120,    -1,    -1,    -1,    -1,  1960,    -1,  2801,   156,   157,
      -1,    -1,    -1,   133,    -1,  2645,    -1,   137,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    -1,  2656,    -1,    -1,    -1,
    1985,    -1,    -1,    -1,    -1,    -1,   156,   157,    -1,    -1,
     874,    -1,    -1,   877,   878,    -1,    -1,    -1,   882,   169,
      -1,   885,   200,    -1,    -1,   203,    -1,    -1,  1344,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1352,    -1,   216,    -1,
     218,    -1,  1358,    -1,   908,    -1,    -1,    -1,  1364,    -1,
     200,  1270,    -1,   203,    -1,    -1,    -1,   921,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,   218,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1394,  1395,
      -1,  1397,    -1,    -1,    -1,    -1,    -1,    -1,  2748,    -1,
      -1,  1407,  1408,    -1,    -1,    -1,   274,   882,    -1,    -1,
      -1,    -1,    -1,  2088,  2089,    24,  1422,    -1,  1424,  1425,
      29,  1427,  1428,  1429,  1430,    -1,    -1,    -1,  1434,    -1,
      -1,  1437,    -1,   908,   274,    -1,  2786,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    -1,   921,    -1,    -1,    29,
    2022,    -1,    -1,  2025,  2026,  2027,  2028,  2029,  2030,  2031,
    2032,  2033,    -1,  2035,    -1,    -1,    -1,  1376,    -1,    -1,
      -1,    -1,  1381,  1382,    -1,   343,    -1,    -1,   346,   347,
     348,    -1,   350,   351,   352,   353,   354,   355,  1042,    -1,
     358,    -1,    -1,   361,    -1,    -1,    -1,   365,    -1,    -1,
    1054,  2073,  2074,   343,    -1,    -1,   346,   347,   348,    -1,
     350,   351,   352,   353,   354,   355,    -1,    -1,  1072,    -1,
      -1,   361,   102,    -1,   133,    -1,  2201,    -1,   137,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1448,
      -1,  1095,  2217,    -1,    -1,  1454,    -1,   156,   157,    -1,
    2225,    -1,    -1,   133,    -1,    -1,    -1,   137,    -1,    -1,
     169,  1470,     8,    -1,    -1,    -1,    -1,  1042,    -1,    -1,
      -1,  1125,    -1,  1127,    -1,    -1,   156,   157,    -1,    -1,
      -1,  1135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   203,    -1,    -1,  1072,    -1,  1153,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    65,
    1095,    -1,    -1,   203,    -1,  1534,    -1,    -1,    -1,    -1,
      -1,    -1,  1186,  1187,    -1,    -1,   216,    -1,    -1,  2211,
    1194,    -1,    -1,  1197,  1198,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1127,    -1,   100,  1209,  1210,    -1,    -1,    -1,
    1135,    -1,  1571,  1217,    -1,   274,    -1,  1221,  1222,  1223,
    1224,    -1,    -1,    -1,    -1,    -1,    -1,  1231,  1232,    -1,
      -1,  2356,  2357,  2358,  2359,    -1,    -1,    -1,    -1,    -1,
      -1,   137,  1246,  1247,   274,   141,  1250,    -1,    -1,    -1,
    1254,    -1,    -1,    -1,  1258,    -1,    -1,  1261,  1262,  1263,
    1264,  1265,  1266,  1267,  1268,  1269,  1270,    24,  1272,    -1,
      -1,    -1,    29,  1277,    -1,    -1,  1280,    -1,    -1,    -1,
      -1,    -1,   178,    -1,   343,    -1,    -1,   346,   347,   348,
      -1,   350,   351,   352,   353,   354,   355,   193,   194,   195,
     196,   197,   361,   199,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1767,  1768,   343,    -1,    -1,    -1,  2339,  2340,    -1,
     350,   351,   352,   353,   354,   355,    -1,    -1,    -1,    -1,
     226,   361,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1346,    -1,    -1,  1270,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1362,   255,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1376,    -1,    -1,    -1,   133,  1381,  1382,   660,
     137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   287,    -1,   289,    -1,    -1,    -1,    -1,    -1,   156,
     157,  1760,  1761,    -1,  1763,    -1,    -1,    -1,    -1,    -1,
      -1,   692,   169,    -1,    -1,   311,    -1,    -1,    -1,  1778,
      -1,  1780,   318,    -1,    -1,   321,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,  2564,
      -1,    29,    -1,  1447,  1448,    -1,   203,    -1,   729,    -1,
    1454,  1376,    -1,    -1,    -1,    -1,  1381,  1382,  2583,   216,
    1819,    -1,  1918,    -1,  1468,    -1,  1470,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2599,   660,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2507,    -1,    -1,    -1,    -1,
    1849,    -1,    -1,    24,    -1,  2517,  2518,    -1,    29,    -1,
      -1,  2626,    -1,    -1,    -1,    -1,    -1,   692,    -1,    -1,
      -1,  1870,    -1,    -1,    -1,    -1,    -1,   274,  1522,    -1,
      -1,    -1,    -1,  1448,    -1,    -1,    -1,  1531,    -1,  1454,
    1534,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1544,    -1,  1901,    -1,   729,  1470,    -1,    -1,    -1,   137,
      -1,    -1,    -1,  1912,  1558,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1923,    -1,    -1,  1571,   156,   157,
      -1,   102,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1589,   343,    -1,    -1,    -1,
     347,   348,    -1,   350,   351,   352,   353,   354,   355,    -1,
      -1,   882,   133,    -1,   361,    -1,   137,    -1,    -1,  1534,
      -1,    -1,    -1,    -1,    -1,   203,    -1,  1621,  1622,    -1,
      -1,    -1,  1626,    -1,    -1,   156,   157,   908,   216,    -1,
      -1,    -1,    -1,    -1,  1638,  1639,  1640,  1641,   169,  1998,
     921,    -1,    -1,  1647,    -1,    -1,  1571,    -1,    -1,    -1,
      -1,    -1,  1656,  1657,    -1,  1659,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1671,    -1,  1673,
      -1,    -1,   203,  1677,    -1,  2034,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,  2048,
    1694,    -1,    -1,    -1,    -1,    -1,    -1,   882,    -1,    -1,
    1704,    -1,  1706,  1707,    -1,  2161,  2162,    -1,    -1,    -1,
      -1,    -1,  2071,    -1,    -1,    -1,    -1,    -1,    -1,  1723,
      -1,    -1,    -1,   908,    -1,    -1,    -1,  1731,  1732,  1733,
      -1,    -1,    -1,    -1,    -1,    -1,   921,    -1,  1742,    -1,
      -1,    -1,    -1,   274,    -1,    -1,    -1,   660,    -1,    -1,
    2206,  2207,  2208,    -1,  2210,   343,  1760,  1761,    -1,  1763,
      -1,  1042,   350,   351,   352,   353,   354,   355,    -1,    -1,
      -1,    -1,    -1,   361,  1778,    -1,  1780,    -1,    -1,   692,
      -1,  2237,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1072,    -1,    -1,    -1,    -1,    -1,    -1,  2157,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   343,    -1,  1095,  1819,   729,    -1,    -1,   350,
     351,   352,   353,   354,   355,    -1,    -1,    -1,    -1,    -1,
     361,    -1,    -1,    -1,    -1,  1760,  1761,    -1,  1763,    -1,
      -1,    -1,    -1,    -1,    -1,  1849,  1127,    -1,    -1,    -1,
      -1,    -1,    -1,  1778,  1135,  1780,    -1,  1042,    -1,    -1,
      -1,    -1,    -1,  2222,    -1,  1869,  1870,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1072,    -1,    -1,
      -1,    -1,    -1,    -1,  1819,    -1,    -1,  1901,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1912,    -1,
    1095,    -1,    -1,    -1,    -1,  2371,  2372,    -1,    -1,  1923,
      -1,    -1,    -1,    -1,  1849,    -1,  2382,   660,    -1,    -1,
      -1,    -1,    -1,  1937,    -1,    -1,    -1,  1941,    -1,    -1,
      -1,  1945,  1127,    -1,    -1,  1870,    -1,    -1,    -1,    -1,
    1135,    -1,    -1,    -1,    -1,    -1,    -1,  1961,    -1,   692,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   882,
      -1,    -1,    -1,    24,    -1,    -1,  1901,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,  2344,    -1,  1912,    -1,  1270,
      -1,  1995,    -1,    -1,  1998,   908,   729,    -1,  1923,    -1,
      -1,    -1,    -1,  2007,    -1,    -1,    -1,  2463,   921,  2013,
      -1,    -1,    -1,  2017,    -1,    -1,    -1,    -1,  2022,    -1,
      -1,  2025,  2026,  2027,  2028,  2029,  2030,  2031,  2032,  2033,
    2034,  2035,    -1,    -1,    -1,  2039,  2040,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2048,    -1,    -1,    -1,    -1,  2053,
      -1,   102,    -1,    -1,    -1,    -1,  2060,    -1,    -1,  2063,
      -1,  2065,    -1,    -1,    -1,    -1,    -1,  2071,  2072,  2073,
    2074,    -1,    -1,  1998,  2078,    -1,    -1,  2081,    -1,    -1,
      -1,    -1,   133,    -1,    -1,  1270,   137,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   660,  1376,    -1,    -1,  2102,    -1,
    1381,  1382,    -1,    -1,    -1,   156,   157,    -1,    -1,  2034,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,  2048,    -1,    -1,   692,    -1,    -1,  1042,
      -1,  2490,    -1,    -1,    -1,    -1,    -1,   660,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2071,    -1,    -1,   882,
    2509,    -1,   203,  2157,    -1,    -1,    -1,  2516,    -1,  1072,
      -1,    -1,    -1,   729,    -1,   216,    -1,  1448,    -1,   692,
      -1,    -1,    -1,  1454,    -1,   908,    -1,    -1,    -1,    -1,
      -1,    -1,  1095,    -1,  2640,    -1,    -1,    -1,   921,  1470,
      -1,  1376,    -1,    -1,    -1,    -1,  1381,  1382,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   729,  2211,    -1,    -1,
      -1,    -1,    -1,    -1,  1127,    -1,    -1,    -1,  2222,    -1,
      -1,    -1,  1135,   274,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  2157,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2697,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1534,    -1,    -1,  2615,  2261,    -1,    -1,
      -1,    -1,  2266,  1448,    -1,    -1,  2270,    -1,    -1,  1454,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  2286,    10,    -1,  1470,    -1,    -1,    -1,    -1,
    1571,    -1,   343,    -1,  2298,    -1,    -1,  2222,    -1,   350,
     351,   352,   353,   354,   355,    -1,    -1,    -1,    -1,  1042,
     361,    -1,    -1,    40,    -1,    -1,   882,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      57,    -1,    59,    -1,    -1,  2339,  2340,    -1,    -1,  1072,
    2344,    -1,   908,    -1,    -1,    -1,  2350,  2351,    75,  1534,
    2354,    -1,    -1,    -1,    -1,   921,    -1,  1270,    -1,   882,
      -1,    -1,  1095,  2722,    -1,    -1,    93,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   660,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   908,  1571,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1127,   122,    -1,    -1,   921,    -1,
      -1,    -1,  1135,    -1,    -1,    -1,    -1,    -1,   692,    -1,
      -1,    -1,    -1,   140,    -1,    -1,    -1,    -1,    -1,  2344,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2433,
    2789,    -1,    -1,    -1,    -1,    -1,   163,    -1,    -1,    16,
      -1,    -1,    -1,    -1,    -1,   729,    -1,    24,    -1,    -1,
      -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1376,  2468,    -1,    -1,    -1,  1381,  1382,
      -1,    -1,    -1,    -1,    -1,    -1,  1042,  2481,    -1,  1760,
    1761,  2485,  1763,    -1,    -1,    -1,  2490,    -1,    -1,   216,
      -1,    -1,    -1,  2497,    -1,    -1,    -1,  1778,    -1,  1780,
      -1,    -1,    -1,  2507,    -1,  2509,  1072,    -1,    -1,   236,
      -1,    -1,  2516,  2517,  2518,    -1,    -1,  2521,    -1,  1042,
      -1,    -1,    -1,    -1,  2528,  2529,    -1,    -1,    -1,  1095,
      -1,    -1,   259,    -1,   261,  1448,    -1,  1270,  1819,   266,
      -1,  1454,    -1,    -1,  2548,    -1,    -1,    -1,    -1,  1072,
      -1,    -1,    -1,    -1,    -1,    -1,   133,  1470,  2562,    -1,
     137,  1127,    -1,    -1,   291,  2490,    -1,    -1,  1849,  1135,
      -1,    -1,  1095,    -1,    -1,  1760,  1761,   304,  1763,   156,
     157,    -1,   309,    -1,  2509,    -1,    -1,    -1,  2592,  1870,
      -1,  2516,   169,  1778,    -1,  1780,    -1,    -1,   882,    -1,
     327,    -1,    -1,    -1,  1127,    -1,    -1,    -1,    -1,    -1,
      -1,  2615,  1135,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1901,  1534,    -1,   200,   908,    -1,   203,    -1,    -1,    -1,
      -1,  1912,    -1,    -1,  1819,    -1,    -1,   921,    -1,   216,
      -1,  2645,  1923,  1376,    -1,    -1,    -1,    -1,  1381,  1382,
      -1,    -1,  2656,    -1,    -1,    -1,    -1,    -1,  1571,    -1,
      -1,    -1,    -1,    -1,  1849,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1870,    -1,    16,    -1,    -1,
    2615,    20,    -1,    -1,    -1,    24,    -1,   274,    -1,    -1,
      29,    -1,    -1,    -1,  1270,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1448,  1901,  1998,  2722,   529,
      -1,  1454,    -1,    -1,    24,    -1,    -1,  1912,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1470,  1923,    -1,
      -1,    -1,    -1,    -1,  2748,    -1,    -1,  1270,    -1,    -1,
      -1,    -1,    -1,  2034,    -1,    -1,    -1,    -1,  1042,    -1,
      89,    -1,    -1,    -1,    -1,    -1,   343,  2048,    -1,   346,
     347,   348,    -1,   350,   351,   352,   353,   354,   355,    -1,
      -1,    -1,  2786,    -1,   361,  2789,    -1,    -1,  1072,    -1,
    2071,    -1,    -1,    -1,    -1,    -1,    -1,  2722,    -1,    -1,
      -1,  1534,   102,    -1,   133,    -1,    -1,    -1,   137,    -1,
    1376,  1095,    -1,  1998,    -1,  1381,  1382,    -1,    -1,    -1,
      -1,   631,    -1,    -1,    -1,    -1,    -1,   156,   157,    -1,
      -1,    -1,    -1,   133,    -1,    -1,    -1,   137,  1571,    -1,
     169,    -1,    -1,  1127,    -1,    -1,    -1,  1760,  1761,  2034,
    1763,  1135,    -1,  1376,    -1,    -1,   156,   157,  1381,  1382,
      -1,    -1,    -1,  2048,  2789,  1778,    -1,  1780,    -1,   169,
      -1,   200,    -1,    -1,   203,    -1,  2157,    -1,    -1,    -1,
      -1,    -1,  1448,    -1,    -1,    -1,  2071,   216,  1454,   218,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   203,  1470,    -1,  1819,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1448,    -1,    -1,    -1,    -1,
      -1,  1454,    -1,    -1,    -1,    -1,  1849,    -1,    -1,    -1,
     750,  2222,    -1,    -1,    -1,   274,    -1,  1470,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1870,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   777,  1534,    -1,
      -1,    -1,  2157,    -1,   274,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1270,    -1,  1901,    -1,
     319,    -1,    -1,    -1,   804,   805,    -1,   807,    -1,  1912,
      -1,    -1,    -1,    -1,    -1,  1571,    -1,    -1,    -1,    -1,
    1923,  1534,    -1,    -1,   343,   825,    -1,   346,   347,   348,
      -1,   350,   351,   352,   353,   354,   355,  1760,  1761,    -1,
    1763,    16,   361,    -1,    19,    -1,    -1,  2222,    -1,    24,
      -1,    -1,    -1,   343,    29,  1778,    -1,  1780,  1571,    -1,
     350,   351,   352,   353,   354,   355,    -1,    -1,    -1,    -1,
      -1,   361,    -1,  2344,   874,    -1,    -1,   877,   878,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1998,  1819,    -1,    -1,    24,
      -1,    -1,  1376,    -1,    29,    -1,    -1,  1381,  1382,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1849,    -1,    -1,    -1,
      -1,  2034,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2048,    -1,  1870,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,
      -1,    -1,   137,    -1,    -1,    -1,    -1,    -1,  2071,  2344,
      -1,    -1,    -1,    -1,  1448,    -1,    -1,   102,  1901,    -1,
    1454,   156,   157,    -1,    -1,    -1,    -1,    -1,    -1,  1912,
      -1,    -1,    -1,    -1,   169,    -1,  1470,    -1,    -1,    -1,
    1923,    -1,    -1,    -1,  1760,  1761,    -1,  1763,   133,    -1,
      -1,    -1,   137,    -1,    -1,    -1,    -1,    -1,    -1,  2490,
      -1,    -1,  1778,    -1,  1780,   200,    -1,    -1,   203,    -1,
      -1,   156,   157,    -1,    -1,    -1,    -1,    -1,  2509,    -1,
      -1,   216,    -1,   218,   169,  2516,    -1,  1760,  1761,    -1,
    1763,    -1,    -1,    -1,  2157,    -1,    -1,    -1,    -1,    -1,
    1534,    -1,    -1,  1819,    -1,  1778,    -1,  1780,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1998,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   216,    -1,  1849,    -1,    -1,    -1,  1571,    -1,   274,
      -1,    -1,    -1,    -1,    -1,    -1,  1819,    -1,    -1,    -1,
      -1,  2034,   821,    -1,  1870,  2490,    -1,    -1,    -1,  2222,
      -1,    -1,    -1,    -1,    -1,  2048,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  2509,    -1,  1849,    -1,    -1,    -1,
      -1,  2516,    -1,    -1,  2615,  1901,    -1,    -1,  2071,   274,
      -1,    -1,    -1,    -1,    -1,    -1,  1912,  1870,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1923,   343,    -1,
      -1,   346,   347,   348,    -1,   350,   351,   352,   353,   354,
     355,    -1,    -1,    -1,    -1,    -1,   361,    -1,  1901,    -1,
      -1,    -1,    -1,    -1,  1194,    -1,    -1,    -1,  1198,  1912,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1923,    -1,    -1,    -1,    -1,    -1,    -1,  1217,   343,    -1,
      -1,  1221,    -1,    -1,  1224,   350,   351,   352,   353,   354,
     355,  1231,  1232,    -1,  2157,    -1,   361,    -1,    -1,    -1,
    2615,  2344,  1998,    -1,    -1,    -1,  1246,    -1,   957,    -1,
    1250,  2722,    -1,    -1,  1254,    -1,    -1,    -1,  1258,    -1,
      -1,  1261,  1262,  1263,  1264,  1265,  1266,  1267,  1268,  1269,
      -1,    -1,  1272,    -1,    -1,    -1,    -1,  1277,  2034,    -1,
      -1,    -1,    -1,    -1,    -1,  1998,  1760,  1761,    -1,  1763,
      -1,    -1,  2048,    -1,    -1,    -1,    -1,    -1,    -1,  2222,
      -1,    -1,    -1,    -1,  1778,    -1,  1780,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  2071,    -1,    -1,  2789,    16,
      -1,  2034,    -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,
      -1,    -1,    29,    -1,    -1,  2048,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1819,    -1,  2722,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2071,    -1,
      -1,    -1,  1362,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      16,    -1,    -1,    19,    -1,  1849,    -1,    -1,    24,    -1,
      -1,    -1,    -1,    29,    -1,    -1,    -1,  2490,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1870,    -1,    -1,    -1,
      -1,  2157,    -1,    -1,    -1,    -1,  2509,    -1,    -1,    -1,
      -1,    -1,    -1,  2516,  2789,    -1,    -1,    -1,    -1,    -1,
      -1,  2344,    -1,    -1,    -1,    -1,    -1,  1901,    -1,    -1,
      -1,    -1,  1141,    -1,    -1,    -1,   133,    -1,  1912,    -1,
     137,    -1,    -1,    -1,  2157,    -1,    -1,  1447,    -1,  1923,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,
     157,    -1,    -1,    -1,    -1,    -1,  2222,    -1,  1468,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    16,  1192,    -1,    -1,    -1,   133,    -1,    -1,
      24,   137,    -1,    -1,    -1,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   200,    -1,    -1,   203,  1216,    -1,  2222,
     156,   157,  2615,  1222,    -1,    -1,    -1,    -1,    -1,   216,
      -1,   218,    -1,   169,  1998,    -1,  1235,  1236,    -1,  1238,
      -1,  1531,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1544,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,   203,  1558,    -1,
    2034,    -1,    -1,    -1,    -1,    -1,  1275,  2490,    -1,    -1,
     216,    -1,   218,    -1,  2048,    -1,    -1,   274,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2509,    -1,  2344,  1589,
      -1,    -1,    -1,  2516,    -1,    -1,    -1,  2071,    -1,   133,
      -1,    -1,    -1,   137,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1326,    -1,  2722,
      -1,    -1,   156,   157,  1333,    -1,  1335,    -1,   274,    -1,
      -1,  2344,    -1,    -1,    -1,   169,    -1,    -1,  1638,    -1,
    1640,  1641,    -1,    -1,    -1,    -1,   343,  1647,    -1,   346,
     347,   348,    -1,   350,   351,   352,   353,   354,   355,    -1,
      -1,    -1,    -1,   360,   361,    -1,   200,    -1,    -1,   203,
      -1,  1671,    -1,  1673,    -1,    -1,    -1,  1677,    -1,    -1,
      -1,    -1,   216,  2157,   218,    -1,  2789,    -1,    -1,    -1,
      -1,    -1,  2615,    -1,    -1,    -1,    -1,   343,    -1,    -1,
     346,   347,   348,    -1,   350,   351,   352,   353,   354,   355,
      -1,    -1,    -1,    -1,    -1,   361,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1723,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1731,  1732,  1733,  2490,    -1,    -1,    -1,    -1,    -1,
     274,    -1,  1742,    -1,    -1,    -1,    -1,    -1,  2222,    -1,
      -1,    -1,    -1,  2509,    -1,    -1,    -1,    -1,    24,    -1,
    2516,    -1,    -1,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  2490,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   319,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  2509,    -1,    -1,  2722,
      -1,    -1,    -1,  2516,    -1,    -1,    -1,    -1,    -1,   343,
      -1,    -1,   346,   347,   348,    -1,   350,   351,   352,   353,
     354,   355,    -1,    -1,    -1,    -1,    -1,   361,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   102,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2615,
      -1,    -1,    -1,    -1,    -1,    -1,  2789,   133,    -1,    -1,
    2344,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1592,    -1,  1594,    -1,    -1,    -1,    -1,
     156,   157,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  2615,   169,    -1,    -1,    -1,  1616,    -1,    -1,
    1619,    -1,  1621,  1622,  1623,    -1,    -1,  1626,    -1,    -1,
      -1,  1630,    -1,    -1,    -1,    -1,    -1,    -1,  1637,    -1,
      -1,    -1,    -1,  1642,    -1,    -1,    -1,   203,    -1,    -1,
      -1,    -1,    -1,  1652,    -1,    -1,    -1,  1656,  1657,  1658,
     216,    -1,    -1,     3,    -1,    -1,    -1,    -1,    -1,    -1,
    1669,    -1,    -1,    13,    14,    15,  2722,  1676,    -1,  1678,
    1679,  1680,  1681,  1682,  1683,  1684,  1685,  1686,    28,  1688,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1995,    46,    -1,    48,    49,
      -1,    -1,    -1,    53,    54,    -1,    -1,  2007,   274,  2722,
      -1,    61,    62,  2013,    -1,    -1,  2490,  2017,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    77,    -1,    -1,
      80,    -1,    -1,  2789,    -1,  2509,    86,    -1,    -1,  2039,
    2040,  1750,  2516,    -1,    -1,    95,    -1,    -1,    -1,    -1,
      -1,   101,    -1,    -1,    -1,    -1,   106,    -1,   108,    -1,
    2060,    -1,   112,  2063,    -1,  2065,    -1,    -1,    -1,    -1,
      -1,    -1,  2072,    -1,   124,    -1,  2789,   343,  2078,    -1,
      -1,  2081,    -1,    -1,   350,   351,   352,   353,   354,   355,
      -1,    16,    -1,    -1,    -1,   361,    -1,    -1,   148,    24,
      -1,    -1,  2102,    -1,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   171,   172,    -1,    -1,    -1,    -1,   177,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   185,    -1,    -1,  1847,    -1,
      -1,  2615,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   202,    -1,    -1,    16,    -1,    -1,    19,    -1,
      -1,    -1,  1871,    24,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   231,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   243,    -1,   245,    -1,    -1,    -1,   249,
      -1,    -1,    -1,   253,    -1,    -1,   256,    -1,   133,    -1,
     260,    -1,   137,    -1,    -1,   265,    -1,    -1,    -1,    -1,
     270,    -1,    -1,   273,    -1,    -1,    -1,    -1,    -1,   279,
      -1,   156,   157,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1952,   169,    -1,    -1,    -1,  2722,    -1,
    1959,    -1,    -1,    -1,    -1,    -1,    -1,   307,    -1,    -1,
      -1,  1970,    -1,    -1,    -1,   315,    -1,   317,    -1,    -1,
     320,    -1,   133,    -1,    -1,   200,   137,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,  1994,  2286,    -1,    -1,    -1,
      -1,   216,    -1,   218,    -1,   156,   157,    -1,  2298,    -1,
      -1,    -1,   227,    -1,    -1,    -1,    -1,    -1,   169,   359,
      -1,  2020,  2021,  2022,    -1,  2789,  2025,  2026,  2027,  2028,
    2029,  2030,  2031,  2032,  2033,    -1,  2035,  2036,    -1,    -1,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    24,   200,
      -1,    -1,   203,    29,    -1,    -1,    -1,  2056,    -1,   274,
    2350,  2351,  2061,  2062,  2354,   216,    -1,   218,    -1,    -1,
      -1,    -1,    -1,    -1,  2073,  2074,  2075,    16,  2077,    -1,
      -1,  2080,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2118,
      -1,    -1,    -1,   274,    -1,    -1,    -1,    -1,   343,    -1,
      -1,   346,   347,   348,    -1,   350,   351,   352,   353,   354,
     355,    -1,    -1,  2433,   120,    -1,   361,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,    -1,    -1,
      -1,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     156,   157,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2481,   343,   169,   133,   346,   347,   348,   137,   350,
     351,   352,   353,   354,   355,    -1,    -1,  2497,    -1,    -1,
     361,   133,  2211,    -1,    -1,   137,    -1,   156,   157,    -1,
      -1,    -1,    -1,    -1,   200,    -1,    -1,   203,    -1,    -1,
     169,  2521,    -1,    -1,   156,   157,    -1,    -1,    -1,  2529,
     216,    -1,   218,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2548,    -1,
      -1,   200,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   216,   200,   218,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   216,    -1,   218,    -1,   274,    -1,
    2299,    -1,  2592,    -1,    -1,  2304,  2305,    -1,  2307,    -1,
    2309,    -1,    -1,    -1,  2313,  2314,  2315,  2316,  2317,  2318,
    2319,  2320,  2321,    -1,  2323,  2324,  2325,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   274,    -1,    -1,    -1,  2338,
    2339,  2340,  2341,    -1,  2343,    -1,    -1,    -1,  2347,  2348,
    2349,    -1,   274,  2352,    -1,    -1,  2355,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   343,    -1,  2368,
     346,   347,   348,    -1,   350,   351,   352,   353,   354,   355,
      -1,    -1,    -1,    -1,    -1,   361,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   343,    -1,    -1,   346,   347,   348,
      -1,   350,   351,   352,   353,   354,   355,    -1,    -1,   358,
      -1,   343,   361,    -1,   346,   347,   348,    -1,   350,   351,
     352,   353,   354,   355,    -1,    -1,  2435,    -1,   360,   361,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  2500,    -1,    -1,    -1,    -1,    -1,    -1,  2507,    -1,
      -1,    -1,    -1,  2512,  2513,    -1,    -1,    -1,  2517,  2518,
    2519,  2520,    -1,  2522,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  2567,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  2602,    -1,    -1,    -1,     3,     4,     5,
       6,     7,     8,     9,    10,  2614,    12,    13,    -1,    -1,
    2619,  2620,  2621,    -1,  2623,    21,    22,    -1,    24,  2628,
      26,    27,    28,    -1,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,  2654,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,    -1,
     116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,    -1,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
      -1,    -1,   158,    -1,   160,   161,   162,   163,   164,    -1,
     166,    -1,   168,    -1,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,    -1,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,   198,   199,    -1,   201,   202,    -1,   204,    -1,
     206,   207,   208,   209,    -1,    -1,   212,    -1,    -1,    -1,
     216,   217,    -1,    -1,   220,    -1,    -1,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,    -1,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,   319,   320,   321,   322,   323,   324,   325,
      -1,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,   357,    -1,    -1,    18,    -1,   362,    21,    22,    -1,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    91,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,    -1,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
     144,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    -1,   166,   167,   168,   169,    -1,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,    -1,    -1,   212,   213,
      -1,    -1,   216,   217,    -1,    -1,   220,   221,   222,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,    -1,   313,
     314,   315,   316,   317,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,   339,   340,   341,   342,   343,
     344,   345,    -1,    -1,    -1,    -1,   350,   351,   352,    -1,
      -1,    -1,    -1,    -1,    -1,   359,   360,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    18,    -1,    -1,    21,    22,    -1,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    -1,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    -1,
     116,    -1,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,   133,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,   144,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,    -1,
     166,   167,   168,   169,    -1,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,    -1,    -1,   212,   213,    -1,    -1,
     216,   217,    -1,    -1,   220,   221,   222,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,    -1,   313,   314,   315,
     316,   317,   318,    -1,   320,   321,   322,   323,   324,   325,
     326,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,   339,   340,   341,   342,   343,   344,   345,
      -1,    -1,    -1,    -1,   350,   351,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   359,   360,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    19,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,   103,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,   115,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,   152,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,   170,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,   211,   212,    -1,    -1,    -1,   216,   217,
      -1,   219,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,   312,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,    -1,     3,     4,     5,     6,     7,     8,     9,    10,
      -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      21,    22,   360,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    -1,    35,    36,    37,    38,    -1,    -1,
      41,    42,    43,    44,    -1,    46,    47,    48,    49,    -1,
      -1,    -1,    53,    54,    55,    56,    -1,    58,    59,    -1,
      61,    -1,    63,    64,    65,    66,    67,    -1,    -1,    -1,
      -1,    -1,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    82,    -1,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    96,    -1,    98,    99,   100,
      -1,   102,    -1,   104,   105,   106,    -1,   108,   109,    -1,
      -1,   112,   113,   114,    -1,   116,    -1,   118,   119,    -1,
     121,   122,   123,    -1,   125,    -1,    -1,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,    -1,   144,   145,   146,   147,   148,   149,   150,
     151,    -1,   153,    -1,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,    -1,    -1,   167,   168,   169,    -1,
     171,   172,   173,    -1,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,   192,   193,   194,   195,   196,   197,    -1,   199,    -1,
     201,   202,   203,   204,    -1,    -1,   207,   208,   209,    -1,
      -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,   220,
     221,   222,    -1,   224,   225,   226,    -1,    -1,    -1,   230,
     231,   232,    -1,   234,   235,   236,   237,   238,   239,   240,
     241,    -1,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,    -1,   258,   259,   260,
     261,   262,   263,   264,    -1,   266,   267,   268,    -1,   270,
     271,   272,   273,   274,   275,   276,    -1,   278,   279,   280,
     281,   282,   283,   284,   285,    -1,   287,    -1,   289,   290,
      -1,   292,   293,   294,   295,    -1,   297,   298,    -1,   300,
      -1,   302,    -1,   304,    -1,    -1,   307,   308,   309,   310,
     311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,
     321,   322,   323,   324,   325,   326,   327,   328,    -1,    -1,
     331,   332,   333,   334,   335,   336,    -1,   338,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    18,    -1,    -1,    21,    22,   360,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,    -1,   118,   119,   120,   121,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
     144,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,    -1,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   208,   209,    -1,    -1,   212,   213,
      -1,    -1,   216,   217,    -1,    -1,   220,   221,   222,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,    -1,   313,
     314,   315,   316,   317,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,   339,   340,   341,   342,   343,
     344,   345,    -1,    -1,    -1,    -1,   350,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   359,     3,     4,     5,     6,
       7,     8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,
      -1,    18,    -1,    -1,    21,    22,    -1,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    -1,    46,
      47,    48,    49,    50,    -1,    -1,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    -1,    82,    -1,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    95,    96,
      -1,    98,    99,   100,    -1,   102,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    -1,   116,
      -1,   118,   119,    -1,   121,   122,   123,    -1,   125,   126,
      -1,   128,    -1,   130,   131,   132,   133,   134,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,   144,   145,   146,
     147,   148,   149,   150,   151,    -1,   153,    -1,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,    -1,   166,
     167,   168,   169,    -1,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   200,   201,   202,   203,   204,   205,   206,
     207,   208,   209,    -1,    -1,   212,   213,    -1,    -1,   216,
     217,    -1,    -1,   220,   221,   222,   223,   224,   225,   226,
      -1,   228,   229,   230,   231,   232,    -1,   234,   235,   236,
     237,   238,   239,   240,   241,    -1,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,    -1,   266,
     267,   268,   269,   270,   271,   272,   273,   274,   275,   276,
      -1,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,    -1,   289,   290,    -1,   292,   293,   294,   295,    -1,
     297,   298,    -1,   300,    -1,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,    -1,   313,   314,   315,   316,
     317,   318,    -1,   320,   321,   322,   323,   324,   325,   326,
     327,   328,    -1,    -1,   331,   332,   333,   334,   335,   336,
      -1,   338,   339,   340,   341,   342,   343,   344,   345,    -1,
      -1,    -1,    -1,   350,   351,   352,    -1,    -1,    -1,    -1,
      -1,    -1,   359,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    18,    -1,
      -1,    21,    22,    -1,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,    -1,   116,    -1,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,   133,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,   144,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,    -1,   166,   167,   168,   169,
      -1,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   202,   203,   204,   205,   206,   207,   208,   209,
      -1,    -1,   212,   213,    -1,    -1,   216,   217,    -1,    -1,
     220,   221,   222,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,    -1,   313,   314,   315,   316,   317,   318,    -1,
     320,   321,   322,   323,   324,   325,   326,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,   339,
     340,   341,   342,   343,   344,   345,    -1,    -1,    -1,    -1,
     350,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    -1,    18,    -1,    -1,    21,    22,
      -1,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    -1,    46,    47,    48,    49,    50,    -1,    -1,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    -1,    82,
      -1,    84,    85,    86,    87,    88,    -1,    90,    -1,    -1,
      93,    94,    95,    96,    -1,    98,    99,   100,    -1,   102,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,    -1,   116,    -1,   118,   119,    -1,   121,   122,
     123,    -1,   125,   126,    -1,   128,    -1,   130,   131,   132,
     133,   134,   135,   136,    -1,   138,   139,   140,   141,   142,
      -1,   144,   145,   146,   147,   148,   149,   150,   151,    -1,
     153,    -1,   155,   156,   157,   158,   159,   160,   161,   162,
     163,   164,    -1,   166,   167,   168,   169,    -1,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   192,
     193,   194,   195,   196,   197,   198,   199,   200,   201,   202,
     203,   204,   205,   206,   207,   208,   209,    -1,    -1,   212,
     213,    -1,    -1,   216,   217,    -1,    -1,   220,   221,   222,
     223,   224,   225,   226,    -1,   228,   229,   230,   231,   232,
      -1,   234,   235,   236,   237,   238,   239,   240,   241,    -1,
     243,   244,   245,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,    -1,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,    -1,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,    -1,   289,   290,    -1,   292,
     293,   294,   295,    -1,   297,   298,    -1,   300,    -1,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,    -1,
     313,   314,   315,   316,   317,   318,    -1,   320,   321,   322,
     323,   324,   325,   326,   327,   328,    -1,    -1,   331,   332,
     333,   334,   335,   336,    -1,   338,   339,   340,   341,   342,
     343,   344,   345,    -1,    -1,    -1,    -1,   350,   351,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   359,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    18,    -1,    -1,    21,    22,    -1,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    -1,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,    -1,
     116,    -1,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,   133,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,   144,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,    -1,
     166,   167,   168,   169,    -1,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,    -1,    -1,   212,   213,    -1,    -1,
     216,   217,    -1,    -1,   220,   221,   222,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,    -1,   313,   314,   315,
     316,   317,   318,    -1,   320,   321,   322,   323,   324,   325,
     326,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,   339,   340,   341,   342,   343,   344,   345,
      -1,    -1,    -1,    -1,   350,   351,    -1,    -1,    -1,    -1,
      -1,   357,    -1,   359,     3,     4,     5,     6,     7,     8,
       9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,    18,
      -1,    -1,    21,    22,    -1,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    -1,    46,    47,    48,
      49,    50,    -1,    -1,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    -1,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    -1,    84,    85,    86,    87,    88,
      -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,    98,
      99,   100,    -1,   102,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    -1,   116,    -1,   118,
     119,    -1,   121,   122,   123,    -1,   125,   126,    -1,   128,
      -1,   130,   131,   132,   133,   134,   135,   136,    -1,   138,
     139,   140,   141,   142,    -1,   144,   145,   146,   147,   148,
     149,   150,   151,    -1,   153,    -1,   155,   156,   157,   158,
     159,   160,   161,   162,   163,   164,    -1,   166,   167,   168,
     169,    -1,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     209,    -1,    -1,   212,   213,    -1,    -1,   216,   217,    -1,
      -1,   220,   221,   222,   223,   224,   225,   226,    -1,   228,
     229,   230,   231,   232,    -1,   234,   235,   236,   237,   238,
     239,   240,   241,    -1,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,    -1,   266,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,    -1,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,    -1,
     289,   290,    -1,   292,   293,   294,   295,    -1,   297,   298,
      -1,   300,    -1,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    -1,   313,   314,   315,   316,   317,   318,
      -1,   320,   321,   322,   323,   324,   325,   326,   327,   328,
      -1,    -1,   331,   332,   333,   334,   335,   336,    -1,   338,
     339,   340,   341,   342,   343,   344,   345,    -1,    -1,    -1,
      -1,   350,   351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     359,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    -1,    -1,    -1,    -1,    18,    -1,    -1,    21,
      22,    -1,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,    -1,   116,    -1,   118,   119,   120,   121,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,   133,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,   144,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,    -1,   166,   167,   168,   169,    -1,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,    -1,    -1,
     212,   213,    -1,    -1,   216,   217,    -1,    -1,   220,   221,
     222,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
      -1,   313,   314,   315,   316,   317,   318,    -1,   320,   321,
     322,   323,   324,   325,   326,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,   339,   340,   341,
     342,   343,   344,   345,    -1,    -1,    -1,    -1,   350,   351,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,     3,     4,
       5,     6,     7,     8,     9,    10,    -1,    12,    13,    -1,
      -1,    -1,    -1,    18,    -1,    -1,    21,    22,    -1,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      -1,    46,    47,    48,    49,    50,    -1,    -1,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    -1,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    -1,    84,
      85,    86,    87,    88,    -1,    90,    -1,    -1,    93,    94,
      95,    96,    -1,    98,    99,   100,    -1,   102,    -1,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
      -1,   116,    -1,   118,   119,    -1,   121,   122,   123,    -1,
     125,   126,    -1,   128,    -1,   130,   131,   132,   133,   134,
     135,   136,    -1,   138,   139,   140,   141,   142,    -1,   144,
     145,   146,   147,   148,   149,   150,   151,    -1,   153,    -1,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
      -1,   166,   167,   168,   169,    -1,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,    -1,    -1,   212,   213,    -1,
      -1,   216,   217,    -1,    -1,   220,   221,   222,   223,   224,
     225,   226,    -1,   228,   229,   230,   231,   232,    -1,   234,
     235,   236,   237,   238,   239,   240,   241,    -1,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
      -1,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,    -1,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,    -1,   289,   290,    -1,   292,   293,   294,
     295,    -1,   297,   298,    -1,   300,    -1,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,    -1,   313,   314,
     315,   316,   317,   318,    -1,   320,   321,   322,   323,   324,
     325,   326,   327,   328,    -1,    -1,   331,   332,   333,   334,
     335,   336,    -1,   338,   339,   340,   341,   342,   343,   344,
     345,    -1,    -1,    -1,    -1,   350,   351,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   359,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      18,    -1,    -1,    21,    22,    -1,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    -1,   116,    -1,
     118,   119,    -1,   121,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,   133,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,   144,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,    -1,   166,   167,
     168,   169,    -1,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   204,   205,   206,   207,
     208,   209,    -1,    -1,   212,   213,    -1,    -1,   216,   217,
      -1,    -1,   220,   221,   222,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,    -1,   313,   314,   315,   316,   317,
     318,    -1,   320,   321,   322,   323,   324,   325,   326,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,   339,   340,   341,   342,   343,   344,   345,    -1,    -1,
      -1,    -1,   350,   351,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   359,     3,     4,     5,     6,     7,     8,     9,    10,
      -1,    12,    13,    -1,    -1,    -1,    -1,    18,    -1,    -1,
      21,    22,    -1,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    -1,    46,    47,    48,    49,    50,
      -1,    -1,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    82,    -1,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    96,    -1,    98,    99,   100,
      -1,   102,    -1,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,    -1,   116,    -1,   118,   119,    -1,
     121,   122,   123,    -1,   125,   126,    -1,   128,    -1,   130,
     131,   132,   133,   134,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,   144,   145,   146,   147,   148,   149,   150,
     151,    -1,   153,    -1,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,    -1,   166,   167,   168,   169,    -1,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   196,   197,   198,   199,    -1,
     201,   202,   203,   204,   205,   206,   207,   208,   209,    -1,
      -1,   212,   213,    -1,    -1,   216,   217,    -1,    -1,   220,
     221,   222,   223,   224,   225,   226,    -1,   228,   229,   230,
     231,   232,    -1,   234,   235,   236,   237,   238,   239,   240,
     241,    -1,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,    -1,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,    -1,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,    -1,   289,   290,
      -1,   292,   293,   294,   295,    -1,   297,   298,    -1,   300,
      -1,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,    -1,    -1,   314,   315,   316,   317,   318,    -1,   320,
     321,   322,   323,   324,   325,   326,   327,   328,    -1,    -1,
     331,   332,   333,   334,   335,   336,    -1,   338,   339,   340,
     341,   342,   343,   344,   345,    -1,    -1,    -1,    -1,   350,
     351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   359,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    18,    -1,    -1,    21,    22,    -1,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,    -1,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,    -1,   116,    -1,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
     144,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    -1,   166,   167,   168,    -1,    -1,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,    -1,   201,   202,   203,
     204,   205,   206,   207,   208,   209,    -1,    -1,   212,   213,
      -1,    -1,   216,   217,    -1,    -1,   220,   221,   222,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,   269,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,    -1,   313,
     314,   315,   316,   317,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,   339,   340,   341,   342,   343,
     344,   345,    -1,    -1,    -1,    -1,   350,   351,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   359,     3,     4,     5,     6,
       7,     8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,
      -1,    18,    -1,    -1,    21,    22,    -1,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    -1,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    -1,    46,
      47,    48,    49,    50,    -1,    -1,    53,    54,    55,    56,
      -1,    58,    59,    60,    61,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    -1,    82,    -1,    84,    85,    86,
      87,    88,    -1,    90,    -1,    -1,    93,    94,    95,    96,
      -1,    98,    99,   100,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    -1,   116,
      -1,   118,   119,    -1,   121,   122,   123,    -1,   125,   126,
      -1,   128,    -1,   130,   131,   132,    -1,   134,   135,   136,
      -1,   138,   139,   140,   141,   142,    -1,   144,   145,   146,
     147,   148,   149,   150,   151,    -1,   153,    -1,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,    -1,   166,
     167,   168,    -1,    -1,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   193,   194,   195,   196,
     197,   198,   199,    -1,   201,   202,   203,   204,   205,   206,
     207,   208,   209,    -1,    -1,   212,   213,    -1,    -1,   216,
     217,    -1,    -1,   220,   221,   222,   223,   224,   225,   226,
      -1,   228,   229,   230,   231,   232,    -1,   234,   235,   236,
     237,   238,   239,   240,   241,    -1,   243,   244,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,    -1,   266,
     267,   268,   269,   270,   271,   272,   273,    -1,   275,   276,
      -1,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,    -1,   289,   290,    -1,   292,   293,   294,   295,    -1,
     297,   298,    -1,   300,    -1,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,    -1,    -1,   314,   315,   316,
     317,   318,    -1,   320,   321,   322,   323,   324,   325,   326,
     327,   328,    -1,    -1,   331,   332,   333,   334,   335,   336,
      -1,   338,   339,   340,   341,   342,   343,   344,   345,    -1,
      -1,    -1,    -1,   350,   351,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   359,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    35,    36,    37,    38,    -1,
      40,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,   119,
      -1,   121,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,   133,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,   144,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,    -1,   166,   167,   168,   169,
      -1,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,   203,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,   213,    -1,   215,   216,   217,    -1,    -1,
     220,   221,   222,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   271,   272,   273,   274,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,   318,    -1,
     320,   321,   322,   323,   324,   325,   326,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,   359,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    35,    36,    37,    38,    -1,    40,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,    -1,   112,   113,
     114,    -1,   116,    -1,   118,   119,    -1,   121,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,   133,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
     144,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    -1,   166,   167,   168,   169,    -1,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,    -1,   201,   202,   203,
     204,    -1,   206,   207,   208,   209,    -1,    -1,   212,   213,
      -1,   215,   216,   217,    -1,    -1,   220,   221,   222,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,   318,    -1,   320,   321,   322,   323,
     324,   325,   326,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,   359,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    -1,    35,    36,    37,
      38,    -1,    40,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,   119,    -1,   121,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,   133,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,   144,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,   156,   157,
     158,   159,   160,   161,   162,   163,   164,    -1,   166,   167,
     168,   169,    -1,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,   203,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,   213,    -1,    -1,   216,   217,
      -1,    -1,   220,   221,   222,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
     318,    -1,   320,   321,   322,   323,   324,   325,   326,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,   359,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,   206,   207,   208,   209,    -1,    -1,
     212,   213,    -1,    -1,   216,   217,    -1,    -1,   220,    -1,
      -1,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,   359,    24,    -1,
      26,    27,    28,    -1,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,    -1,
     116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,    -1,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
      -1,    -1,   158,    -1,   160,   161,   162,   163,   164,    -1,
     166,    -1,   168,    -1,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,    -1,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,   198,   199,    -1,   201,   202,    -1,   204,    -1,
     206,   207,   208,   209,    -1,    -1,   212,    -1,    -1,    -1,
     216,   217,    -1,    -1,   220,    -1,    -1,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,    -1,   270,   271,   272,   273,    -1,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
      -1,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,   359,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,   359,
      24,    -1,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,   166,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,    -1,   201,   202,    -1,
     204,    -1,   206,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,   220,    -1,    -1,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,    -1,    -1,    -1,    -1,   343,
      -1,    -1,   346,   347,   348,    -1,   350,   351,   352,   353,
     354,   355,     3,     4,     5,     6,     7,     8,     9,    10,
      -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      21,    22,    -1,    24,    -1,    26,    27,    28,    -1,    30,
      -1,    32,    33,    -1,    35,    36,    37,    38,    -1,    -1,
      41,    42,    43,    44,    -1,    46,    47,    48,    49,    50,
      -1,    -1,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    73,    74,    75,    76,    77,    78,    79,    80,
      -1,    82,    -1,    84,    85,    86,    87,    88,    -1,    90,
      -1,    -1,    93,    94,    95,    96,    -1,    98,    99,   100,
      -1,   102,    -1,   104,   105,   106,   107,   108,   109,   110,
      -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,
      -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,   130,
     131,   132,    -1,   134,   135,   136,    -1,   138,   139,   140,
     141,   142,    -1,    -1,   145,   146,   147,   148,   149,   150,
     151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,
     161,   162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,
     171,   172,   173,    -1,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,    -1,   189,    -1,
     191,   192,   193,   194,   195,   196,   197,   198,   199,    -1,
     201,   202,    -1,   204,    -1,   206,   207,   208,   209,    -1,
      -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,   220,
      -1,    -1,   223,   224,   225,   226,    -1,   228,   229,   230,
     231,   232,    -1,   234,   235,   236,   237,   238,   239,   240,
     241,    -1,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,    -1,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,    -1,   266,   267,   268,    -1,   270,
     271,   272,   273,    -1,   275,   276,    -1,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,    -1,   289,   290,
      -1,   292,   293,   294,   295,    -1,   297,   298,    -1,   300,
      -1,   302,   303,   304,   305,    -1,   307,   308,   309,   310,
     311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,
     321,   322,   323,   324,   325,    -1,   327,   328,    -1,    -1,
     331,   332,   333,   334,   335,   336,    -1,   338,    -1,    -1,
      -1,    -1,   343,    -1,    -1,   346,   347,   348,    -1,   350,
     351,   352,   353,   354,   355,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,    -1,    -1,    -1,    -1,   343,    -1,    -1,   346,   347,
     348,    -1,   350,   351,   352,   353,   354,   355,     3,     4,
       5,     6,     7,     8,     9,    10,    -1,    12,    13,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,
      -1,    26,    27,    28,    -1,    30,    -1,    32,    33,    -1,
      35,    36,    37,    38,    -1,    -1,    41,    42,    43,    44,
      -1,    46,    47,    48,    49,    50,    -1,    -1,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    -1,    63,    64,
      65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    79,    80,    -1,    82,    -1,    84,
      85,    86,    87,    88,    -1,    90,    -1,    -1,    93,    94,
      95,    96,    -1,    98,    99,   100,    -1,   102,    -1,   104,
     105,   106,   107,   108,   109,   110,    -1,   112,   113,   114,
      -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,
     125,   126,    -1,   128,    -1,   130,   131,   132,    -1,   134,
     135,   136,    -1,   138,   139,   140,   141,   142,    -1,    -1,
     145,   146,   147,   148,   149,   150,   151,    -1,   153,    -1,
     155,    -1,    -1,   158,    -1,   160,   161,   162,   163,   164,
      -1,   166,    -1,   168,    -1,    -1,   171,   172,   173,    -1,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,    -1,   189,    -1,   191,   192,   193,   194,
     195,   196,   197,   198,   199,    -1,   201,   202,    -1,   204,
      -1,   206,   207,   208,   209,    -1,    -1,   212,    -1,    -1,
      -1,   216,   217,    -1,    -1,   220,    -1,    -1,   223,   224,
     225,   226,    -1,   228,   229,   230,   231,   232,    -1,   234,
     235,   236,   237,   238,   239,   240,   241,    -1,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,    -1,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
      -1,   266,   267,   268,    -1,   270,   271,   272,   273,    -1,
     275,   276,    -1,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,    -1,   289,   290,    -1,   292,   293,   294,
     295,    -1,   297,   298,    -1,   300,    -1,   302,   303,   304,
     305,    -1,   307,   308,   309,   310,   311,    -1,    -1,   314,
     315,   316,   317,    -1,    -1,   320,   321,   322,   323,   324,
     325,    -1,   327,   328,    -1,    -1,   331,   332,   333,   334,
     335,   336,    -1,   338,    -1,    -1,    -1,    -1,   343,    -1,
      -1,   346,   347,   348,    -1,   350,   351,   352,   353,   354,
     355,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,   206,   207,   208,   209,    -1,    -1,
     212,    -1,    -1,    -1,   216,   217,    -1,    -1,   220,    -1,
      -1,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,    -1,    -1,    -1,
      -1,   343,    -1,    -1,   346,   347,   348,    -1,   350,   351,
     352,   353,   354,   355,     3,     4,     5,     6,     7,     8,
       9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,    28,
      -1,    30,    -1,    32,    33,    -1,    35,    36,    37,    38,
      -1,    -1,    41,    42,    43,    44,    -1,    46,    47,    48,
      49,    -1,    -1,    -1,    53,    54,    55,    56,    -1,    58,
      59,    -1,    61,    -1,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      79,    80,    -1,    82,    -1,    84,    85,    86,    87,    88,
      -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,    98,
      99,   100,    -1,   102,    -1,   104,   105,   106,    -1,   108,
     109,    -1,    -1,   112,   113,   114,    -1,   116,    -1,   118,
      -1,    -1,    -1,   122,   123,    -1,   125,    -1,    -1,   128,
      -1,   130,   131,   132,    -1,   134,   135,   136,    -1,   138,
     139,   140,   141,   142,    -1,    -1,    -1,   146,   147,   148,
     149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,
      -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,   168,
      -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,    -1,
     189,    -1,   191,   192,   193,   194,   195,   196,   197,    -1,
     199,    -1,   201,   202,    -1,   204,    -1,    -1,   207,   208,
     209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,
      -1,    -1,    -1,    -1,    -1,   224,   225,   226,    -1,    -1,
      -1,   230,   231,   232,    -1,   234,   235,   236,   237,   238,
     239,   240,   241,    -1,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,    -1,   255,   256,    -1,   258,
     259,   260,   261,   262,   263,   264,    -1,   266,   267,   268,
      -1,   270,   271,   272,   273,    -1,   275,   276,    -1,   278,
     279,   280,   281,   282,   283,   284,   285,    -1,   287,    -1,
     289,   290,    -1,   292,   293,   294,   295,    -1,   297,   298,
      -1,   300,    -1,   302,    -1,   304,    -1,    -1,   307,   308,
     309,   310,   311,    -1,    -1,   314,   315,   316,   317,    -1,
      -1,   320,   321,   322,   323,   324,   325,    -1,   327,   328,
      -1,    -1,   331,   332,   333,   334,   335,   336,    -1,   338,
     339,   340,    -1,    -1,   343,   344,    -1,   346,   347,   348,
      -1,   350,   351,   352,   353,   354,   355,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,   352,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,   213,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,   352,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    -1,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    -1,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    -1,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,    -1,   108,   109,    -1,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,    -1,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,    -1,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,    -1,   199,    -1,   201,   202,    -1,
     204,    -1,    -1,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,    -1,    -1,    -1,    -1,
     224,   225,   226,    -1,    -1,    -1,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,    -1,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,    -1,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,    -1,
     304,    -1,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,   352,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,   210,    -1,   212,    -1,   214,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,   339,   340,    -1,    -1,    -1,   344,    -1,    -1,    -1,
      -1,    -1,    -1,   351,     3,     4,     5,     6,     7,     8,
       9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,    28,
      -1,    30,    -1,    32,    33,    -1,    35,    36,    37,    38,
      -1,    -1,    41,    42,    43,    44,    -1,    46,    47,    48,
      49,    50,    -1,    -1,    53,    54,    55,    56,    -1,    58,
      59,    60,    61,    -1,    63,    64,    65,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      79,    80,    -1,    82,    -1,    84,    85,    86,    87,    88,
      -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,    98,
      99,   100,    -1,   102,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,    -1,   116,    -1,   118,
      -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,
      -1,   130,   131,   132,    -1,   134,   135,   136,    -1,   138,
     139,   140,   141,   142,    -1,    -1,   145,   146,   147,   148,
     149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,
      -1,   160,   161,   162,   163,   164,    -1,   166,    -1,   168,
      -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,    -1,
     189,    -1,   191,   192,   193,   194,   195,   196,   197,   198,
     199,    -1,   201,   202,    -1,   204,    -1,   206,   207,   208,
     209,   210,    -1,   212,    -1,   214,    -1,   216,   217,    -1,
      -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,   228,
     229,   230,   231,   232,    -1,   234,   235,   236,   237,   238,
     239,   240,   241,    -1,   243,   244,   245,   246,   247,   248,
     249,   250,   251,   252,   253,    -1,   255,   256,   257,   258,
     259,   260,   261,   262,   263,   264,    -1,   266,   267,   268,
      -1,   270,   271,   272,   273,    -1,   275,   276,    -1,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,    -1,
     289,   290,    -1,   292,   293,   294,   295,    -1,   297,   298,
      -1,   300,    -1,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,    -1,    -1,   314,   315,   316,   317,    -1,
      -1,   320,   321,   322,   323,   324,   325,    -1,   327,   328,
      -1,    -1,   331,   332,   333,   334,   335,   336,    -1,   338,
     339,   340,    -1,    -1,    -1,   344,    -1,    -1,    -1,    -1,
      -1,    -1,   351,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    16,   289,
     290,    -1,   292,   293,   294,   295,    24,   297,   298,    -1,
     300,    29,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    16,
      -1,   331,   332,   333,   334,   335,   336,    24,   338,    -1,
      -1,    -1,    29,    -1,   344,    -1,    -1,    -1,    -1,    -1,
      -1,   351,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,    29,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    24,    -1,    -1,    -1,    -1,
      29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    16,   133,    -1,    -1,    -1,   137,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,   157,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   169,    -1,    -1,    -1,    16,   133,    -1,    -1,    -1,
     137,    -1,    -1,    24,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,
     157,    -1,   200,   133,    -1,   203,    -1,   137,    -1,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,   216,    -1,
     218,    -1,    -1,    -1,   133,    -1,   156,   157,   137,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,   200,    -1,    -1,   203,   156,   157,    -1,
      -1,   133,    -1,    -1,    -1,   137,    -1,    -1,    -1,   216,
     169,   218,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     200,    -1,    -1,   203,   156,   157,   274,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   216,   169,   218,    -1,
      -1,   200,   133,    -1,   203,    -1,   137,    -1,   296,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,   218,
      -1,    -1,    -1,    -1,    -1,   156,   157,   274,   200,    -1,
      -1,   203,    -1,    -1,    -1,    -1,    -1,    -1,   169,    -1,
      -1,    -1,    -1,    -1,   216,    -1,   218,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   274,   343,    -1,    -1,   346,   347,
     348,    -1,   350,   351,   352,   353,   354,   355,    -1,   200,
      -1,    -1,   203,   361,    -1,   274,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   216,    -1,   218,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   343,    -1,    -1,   346,
     347,   348,   274,   350,   351,   352,   353,   354,   355,    -1,
      -1,    16,    -1,   360,   361,    -1,    -1,    -1,    -1,    24,
      -1,    -1,    -1,   343,    29,    -1,   346,   347,   348,    -1,
     350,   351,   352,   353,   354,   355,    -1,    16,    -1,    -1,
     360,   361,    -1,   274,   343,    24,    -1,   346,   347,   348,
      29,   350,   351,   352,   353,   354,   355,    -1,    -1,    -1,
      -1,   360,   361,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   343,    -1,    -1,   346,   347,   348,    -1,   350,   351,
     352,   353,   354,   355,    -1,    -1,    -1,    -1,   360,   361,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   343,    -1,    -1,   346,   347,   348,    -1,   350,
     351,   352,   353,   354,   355,    -1,    -1,    -1,   133,   360,
     361,    -1,   137,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   156,   157,    -1,   133,    -1,    -1,    -1,   137,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   156,   157,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     169,    -1,    -1,    -1,    -1,   200,    -1,    -1,   203,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   216,    -1,   218,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   200,    -1,    -1,   203,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   216,    -1,   218,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   274,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   274,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   343,    -1,
      -1,   346,   347,   348,    -1,   350,   351,   352,   353,   354,
     355,    -1,    -1,    -1,    -1,   360,   361,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   343,    -1,    -1,   346,   347,   348,
      -1,   350,   351,   352,   353,   354,   355,    -1,    -1,    -1,
      -1,    -1,   361,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,   339,
     340,   341,   342,    -1,   344,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,    -1,   340,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,    -1,
     340,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   153,   154,   155,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   223,   224,   225,   226,   227,   228,   229,   230,   231,
     232,   233,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      66,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,    -1,
     116,    -1,   118,   119,    -1,   121,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,   133,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,   144,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,    -1,
     166,   167,   168,   169,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,    -1,   201,   202,   203,   204,    -1,
     206,   207,   208,   209,    -1,    -1,   212,   213,    -1,    -1,
     216,   217,    -1,    -1,   220,   221,   222,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,   274,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
     326,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,   117,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,   169,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,   313,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    -1,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    -1,    61,    -1,    63,
      64,    65,    66,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,    -1,   108,   109,    -1,    -1,   112,   113,
     114,    -1,   116,    -1,   118,   119,    -1,   121,   122,   123,
      -1,   125,    -1,    -1,   128,    -1,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   139,   140,   141,   142,    -1,
     144,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,    -1,    -1,   167,   168,   169,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,    -1,   199,    -1,   201,   202,   203,
     204,    -1,    -1,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,   220,   221,   222,    -1,
     224,   225,   226,    -1,    -1,    -1,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,    -1,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
     274,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,    -1,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,    -1,
     304,    -1,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,   326,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,   124,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,   206,   207,   208,   209,    -1,    -1,
     212,    -1,    -1,    -1,   216,   217,    -1,    -1,   220,    -1,
      -1,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
     242,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    14,    15,
      -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,    -1,
      26,    27,    28,    -1,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,    -1,
     116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,    -1,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
      -1,    -1,   158,    -1,   160,   161,   162,   163,   164,    -1,
     166,    -1,   168,    -1,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,    -1,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,    -1,   201,   202,    -1,   204,    -1,
     206,   207,   208,   209,    -1,    -1,   212,   213,    -1,    -1,
     216,   217,    -1,    -1,   220,    -1,    -1,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,    -1,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
      -1,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,   213,    -1,   215,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    -1,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,   166,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,   190,   191,   192,   193,
     194,   195,   196,   197,   198,   199,    -1,   201,   202,    -1,
     204,    -1,   206,   207,   208,   209,    -1,    -1,   212,   213,
      -1,    -1,   216,   217,    -1,    -1,   220,    -1,    -1,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,   291,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,   265,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,   190,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,   206,   207,   208,   209,    -1,    -1,
     212,   213,    -1,    -1,   216,   217,    -1,    -1,   220,    -1,
      -1,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,   291,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    -1,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    -1,    61,    -1,    63,    64,    65,
      66,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,    -1,   108,   109,    -1,    -1,   112,   113,   114,    -1,
     116,    -1,   118,   119,    -1,   121,   122,   123,    -1,   125,
      -1,    -1,   128,    -1,   130,   131,   132,   133,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,   144,    -1,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,    -1,
      -1,   167,   168,   169,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,    -1,   199,    -1,   201,   202,   203,   204,    -1,
      -1,   207,   208,   209,    -1,    -1,   212,    -1,    -1,    -1,
     216,   217,    -1,    -1,    -1,   221,   222,    -1,   224,   225,
     226,    -1,    -1,    -1,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,    -1,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,   274,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
      -1,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,    -1,   304,    -1,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
     326,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,   213,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    -1,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,   166,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,    -1,   201,   202,    -1,
     204,    -1,   206,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,   220,    -1,    -1,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,   318,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
     318,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,   206,   207,   208,   209,    -1,    -1,
     212,    -1,    -1,    -1,   216,   217,    -1,    -1,   220,    -1,
      -1,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,    -1,
      26,    27,    28,    -1,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,    -1,
     116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,    -1,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
      -1,    -1,   158,    -1,   160,   161,   162,   163,   164,    -1,
     166,    -1,   168,    -1,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,    -1,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,   198,   199,    -1,   201,   202,    -1,   204,    -1,
     206,   207,   208,   209,    -1,    -1,   212,    -1,    -1,    -1,
     216,   217,    -1,    -1,   220,    -1,    -1,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,    -1,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
      -1,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    25,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,   166,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,    -1,   201,   202,    -1,
     204,    -1,   206,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,   220,    -1,    -1,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    19,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    -1,    -1,    -1,    -1,    -1,    19,    -1,    21,
      22,    -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,   206,   207,   208,   209,    -1,    -1,
     212,    -1,    -1,    -1,   216,   217,    -1,    -1,   220,    -1,
      -1,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,    -1,
      26,    27,    28,    -1,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,    -1,
     116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,   125,
     126,   127,   128,    -1,   130,   131,   132,    -1,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
      -1,    -1,   158,    -1,   160,   161,   162,   163,   164,    -1,
     166,    -1,   168,    -1,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,    -1,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,   198,   199,    -1,   201,   202,    -1,   204,    -1,
     206,   207,   208,   209,    -1,    -1,   212,    -1,    -1,    -1,
     216,   217,    -1,    -1,   220,    -1,    -1,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,    -1,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
      -1,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    -1,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,   166,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,    -1,   201,   202,    -1,
     204,    -1,   206,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,   220,    -1,    -1,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,   206,   207,   208,   209,    -1,    -1,
     212,    -1,    -1,    -1,   216,   217,    -1,    -1,   220,    -1,
      -1,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,   319,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,    -1,
      26,    27,    28,    -1,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,    -1,
     116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,    -1,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
      -1,    -1,   158,    -1,   160,   161,   162,   163,   164,    -1,
     166,    -1,   168,    -1,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,    -1,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,   198,   199,    -1,   201,   202,    -1,   204,    -1,
     206,   207,   208,   209,    -1,    -1,   212,    -1,    -1,    -1,
     216,   217,    -1,    -1,   220,    -1,    -1,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,    -1,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
      -1,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    -1,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    50,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,   107,   108,   109,   110,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,   126,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,   166,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,   198,   199,    -1,   201,   202,    -1,
     204,    -1,   206,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,   220,    -1,    -1,   223,
     224,   225,   226,    -1,   228,   229,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,   303,
     304,   305,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    50,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,   107,
     108,   109,   110,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,   126,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,   145,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,   166,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,   206,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,   220,    -1,    -1,   223,   224,   225,   226,    -1,
     228,   229,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,   257,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,   303,   304,   305,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    50,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    60,    61,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,   107,   108,   109,   110,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,   126,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,   145,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,   166,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,   206,   207,   208,   209,    -1,    -1,
     212,    -1,    -1,    -1,   216,   217,    -1,    -1,   220,    -1,
      -1,   223,   224,   225,   226,    -1,   228,   229,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,   303,   304,   305,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,    -1,
      26,    27,    28,    -1,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    50,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    60,    61,    -1,    63,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,   107,   108,   109,   110,    -1,   112,   113,   114,    -1,
     116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,   125,
     126,    -1,   128,    -1,   130,   131,   132,    -1,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,   145,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
      -1,    -1,   158,    -1,   160,   161,   162,   163,   164,    -1,
     166,    -1,   168,    -1,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,    -1,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,   198,   199,    -1,   201,   202,    -1,   204,    -1,
     206,   207,   208,   209,    -1,    -1,   212,    -1,    -1,    -1,
     216,   217,    -1,    -1,   220,    -1,    -1,   223,   224,   225,
     226,    -1,   228,   229,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,    -1,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,    -1,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,   303,   304,   305,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
      -1,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      50,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,   107,   108,   109,
     110,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,   126,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,   145,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,   166,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,   198,   199,
      -1,   201,   202,    -1,   204,    -1,   206,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
     220,    -1,    -1,   223,   224,   225,   226,    -1,   228,   229,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,   257,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,   303,   304,   305,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    -1,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    -1,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    -1,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,    -1,   108,   109,    -1,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,    -1,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,   137,   138,   139,   140,   141,   142,    -1,
      -1,   145,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,    -1,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,    -1,   199,    -1,   201,   202,    -1,
     204,    -1,    -1,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,   220,    -1,    -1,    -1,
     224,   225,   226,    -1,    -1,    -1,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,    -1,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,    -1,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,    -1,
     304,    -1,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    -1,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    -1,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,    -1,
     108,   109,    -1,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,    -1,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,    -1,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
     198,   199,    -1,   201,   202,    -1,   204,    -1,    -1,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,    -1,    -1,    -1,    -1,   224,   225,   226,    -1,
      -1,    -1,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,    -1,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,   271,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,    -1,   287,
      -1,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    -1,   300,    -1,   302,    -1,   304,    -1,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      -1,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    -1,   331,   332,   333,   334,   335,   336,    -1,
     338,     3,     4,     5,     6,     7,     8,     9,    10,    -1,
      12,    13,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,
      22,    -1,    24,    -1,    26,    27,    28,    -1,    30,    -1,
      32,    33,    -1,    35,    36,    37,    38,    -1,    -1,    41,
      42,    43,    44,    -1,    46,    47,    48,    49,    -1,    -1,
      -1,    53,    54,    55,    56,    -1,    58,    59,    -1,    61,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    -1,
      82,    -1,    84,    85,    86,    87,    88,    -1,    90,    -1,
      -1,    93,    94,    95,    96,    -1,    98,    99,   100,    -1,
     102,    -1,   104,   105,   106,    -1,   108,   109,    -1,    -1,
     112,   113,   114,    -1,   116,    -1,   118,    -1,    -1,    -1,
     122,   123,    -1,   125,    -1,    -1,   128,    -1,   130,   131,
     132,    -1,   134,   135,   136,    -1,   138,   139,   140,   141,
     142,    -1,    -1,    -1,   146,   147,   148,   149,   150,   151,
      -1,   153,    -1,   155,    -1,    -1,   158,    -1,   160,   161,
     162,   163,   164,    -1,    -1,    -1,   168,    -1,    -1,   171,
     172,   173,    -1,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,    -1,   189,    -1,   191,
     192,   193,   194,   195,   196,   197,   198,   199,    -1,   201,
     202,    -1,   204,    -1,    -1,   207,   208,   209,    -1,    -1,
     212,    -1,    -1,    -1,   216,   217,    -1,    -1,    -1,    -1,
      -1,    -1,   224,   225,   226,    -1,    -1,    -1,   230,   231,
     232,    -1,   234,   235,   236,   237,   238,   239,   240,   241,
      -1,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,    -1,   255,   256,    -1,   258,   259,   260,   261,
     262,   263,   264,    -1,   266,   267,   268,    -1,   270,   271,
     272,   273,    -1,   275,   276,    -1,   278,   279,   280,   281,
     282,   283,   284,   285,    -1,   287,    -1,   289,   290,    -1,
     292,   293,   294,   295,    -1,   297,   298,    -1,   300,    -1,
     302,    -1,   304,    -1,    -1,   307,   308,   309,   310,   311,
      -1,    -1,   314,   315,   316,   317,    -1,    -1,   320,   321,
     322,   323,   324,   325,    -1,   327,   328,    -1,    -1,   331,
     332,   333,   334,   335,   336,    -1,   338,     3,     4,     5,
       6,     7,     8,     9,    10,    -1,    12,    13,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    21,    22,    -1,    24,    -1,
      26,    27,    28,    -1,    30,    -1,    32,    33,    -1,    35,
      36,    37,    38,    -1,    -1,    41,    42,    43,    44,    -1,
      46,    47,    48,    49,    -1,    -1,    -1,    53,    54,    55,
      56,    -1,    58,    59,    -1,    61,    -1,    63,    64,    65,
      -1,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    -1,    82,    -1,    84,    85,
      86,    87,    88,    -1,    90,    -1,    -1,    93,    94,    95,
      96,    -1,    98,    99,   100,    -1,   102,    -1,   104,   105,
     106,    -1,   108,   109,    -1,    -1,   112,   113,   114,    -1,
     116,    -1,   118,    -1,    -1,    -1,   122,   123,    -1,   125,
      -1,    -1,   128,    -1,   130,   131,   132,    -1,   134,   135,
     136,    -1,   138,   139,   140,   141,   142,    -1,    -1,    -1,
     146,   147,   148,   149,   150,   151,    -1,   153,    -1,   155,
      -1,    -1,   158,    -1,   160,   161,   162,   163,   164,    -1,
      -1,    -1,   168,    -1,    -1,   171,   172,   173,    -1,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,    -1,   189,    -1,   191,   192,   193,   194,   195,
     196,   197,    -1,   199,    -1,   201,   202,    -1,   204,   205,
      -1,   207,   208,   209,    -1,    -1,   212,    -1,    -1,    -1,
     216,   217,    -1,    -1,    -1,    -1,    -1,    -1,   224,   225,
     226,    -1,    -1,    -1,   230,   231,   232,    -1,   234,   235,
     236,   237,   238,   239,   240,   241,    -1,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,    -1,   255,
     256,    -1,   258,   259,   260,   261,   262,   263,   264,    -1,
     266,   267,   268,    -1,   270,   271,   272,   273,    -1,   275,
     276,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
      -1,   287,    -1,   289,   290,    -1,   292,   293,   294,   295,
      -1,   297,   298,    -1,   300,    -1,   302,    -1,   304,    -1,
      -1,   307,   308,   309,   310,   311,    -1,    -1,   314,   315,
     316,   317,    -1,    -1,   320,   321,   322,   323,   324,   325,
      -1,   327,   328,    -1,    -1,   331,   332,   333,   334,   335,
     336,    -1,   338,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    12,    13,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    21,    22,    -1,    24,    -1,    26,    27,    28,    -1,
      30,    -1,    32,    33,    -1,    35,    36,    37,    38,    -1,
      -1,    41,    42,    43,    44,    -1,    46,    47,    48,    49,
      -1,    -1,    -1,    53,    54,    55,    56,    -1,    58,    59,
      -1,    61,    -1,    63,    64,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    -1,    82,    -1,    84,    85,    86,    87,    88,    -1,
      90,    -1,    -1,    93,    94,    95,    96,    -1,    98,    99,
     100,    -1,   102,    -1,   104,   105,   106,    -1,   108,   109,
      -1,    -1,   112,   113,   114,    -1,   116,    -1,   118,    -1,
      -1,    -1,   122,   123,    -1,   125,    -1,    -1,   128,    -1,
     130,   131,   132,    -1,   134,   135,   136,    -1,   138,   139,
     140,   141,   142,    -1,    -1,    -1,   146,   147,   148,   149,
     150,   151,    -1,   153,    -1,   155,    -1,    -1,   158,    -1,
     160,   161,   162,   163,   164,    -1,    -1,    -1,   168,    -1,
      -1,   171,   172,   173,    -1,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,    -1,   189,
      -1,   191,   192,   193,   194,   195,   196,   197,    -1,   199,
      -1,   201,   202,    -1,   204,    -1,    -1,   207,   208,   209,
      -1,    -1,   212,    -1,    -1,    -1,   216,   217,    -1,    -1,
      -1,    -1,    -1,    -1,   224,   225,   226,    -1,    -1,    -1,
     230,   231,   232,    -1,   234,   235,   236,   237,   238,   239,
     240,   241,    -1,   243,   244,   245,   246,   247,   248,   249,
     250,   251,   252,   253,    -1,   255,   256,    -1,   258,   259,
     260,   261,   262,   263,   264,    -1,   266,   267,   268,    -1,
     270,   271,   272,   273,    -1,   275,   276,    -1,   278,   279,
     280,   281,   282,   283,   284,   285,    -1,   287,    -1,   289,
     290,    -1,   292,   293,   294,   295,    -1,   297,   298,    -1,
     300,    -1,   302,    -1,   304,    -1,    -1,   307,   308,   309,
     310,   311,    -1,    -1,   314,   315,   316,   317,    -1,    -1,
     320,   321,   322,   323,   324,   325,    -1,   327,   328,    -1,
      -1,   331,   332,   333,   334,   335,   336,    -1,   338,     3,
       4,     5,     6,     7,     8,     9,    10,    -1,    12,    13,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    21,    22,    -1,
      24,    -1,    26,    27,    28,    -1,    30,    -1,    32,    33,
      -1,    35,    36,    37,    38,    -1,    -1,    41,    42,    43,
      44,    -1,    46,    47,    48,    49,    -1,    -1,    -1,    53,
      54,    55,    56,    -1,    58,    59,    -1,    61,    -1,    63,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    -1,    82,    -1,
      84,    85,    86,    87,    88,    -1,    90,    -1,    -1,    93,
      94,    95,    96,    -1,    98,    99,   100,    -1,   102,    -1,
     104,   105,   106,    -1,   108,   109,    -1,    -1,   112,   113,
     114,    -1,   116,    -1,   118,    -1,    -1,    -1,   122,   123,
      -1,   125,    -1,    -1,   128,    -1,   130,   131,   132,    -1,
     134,   135,   136,    -1,   138,   139,   140,   141,   142,    -1,
      -1,    -1,   146,   147,   148,   149,   150,   151,    -1,   153,
      -1,   155,    -1,    -1,   158,    -1,   160,   161,   162,   163,
     164,    -1,    -1,    -1,   168,    -1,    -1,   171,   172,   173,
      -1,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,    -1,   189,    -1,   191,   192,   193,
     194,   195,   196,   197,    -1,   199,    -1,   201,   202,    -1,
     204,    -1,    -1,   207,   208,   209,    -1,    -1,   212,    -1,
      -1,    -1,   216,   217,    -1,    -1,    -1,    -1,    -1,    -1,
     224,   225,   226,    -1,    -1,    -1,   230,   231,   232,    -1,
     234,   235,   236,   237,   238,   239,   240,   241,    -1,   243,
     244,   245,   246,   247,   248,   249,   250,   251,   252,   253,
      -1,   255,   256,    -1,   258,   259,   260,   261,   262,   263,
     264,    -1,   266,   267,   268,    -1,   270,   271,   272,   273,
      -1,   275,   276,    -1,   278,   279,   280,   281,   282,   283,
     284,   285,    -1,   287,    -1,   289,   290,    -1,   292,   293,
     294,   295,    -1,   297,   298,    -1,   300,    -1,   302,    -1,
     304,    -1,    -1,   307,   308,   309,   310,   311,    -1,    -1,
     314,   315,   316,   317,    -1,    -1,   320,   321,   322,   323,
     324,   325,    -1,   327,   328,    -1,    -1,   331,   332,   333,
     334,   335,   336,    -1,   338,     3,     4,     5,     6,     7,
       8,     9,    10,    -1,    12,    13,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    21,    22,    -1,    24,    -1,    26,    27,
      28,    -1,    30,    -1,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    41,    42,    43,    44,    -1,    46,    47,
      48,    49,    -1,    -1,    -1,    53,    54,    55,    56,    -1,
      58,    59,    -1,    61,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    -1,    82,    -1,    84,    85,    86,    87,
      88,    -1,    90,    -1,    -1,    93,    94,    95,    96,    -1,
      98,    99,   100,    -1,   102,    -1,   104,   105,   106,    -1,
     108,   109,    -1,    -1,   112,   113,   114,    -1,   116,    -1,
     118,    -1,    -1,    -1,   122,   123,    -1,   125,    -1,    -1,
     128,    -1,   130,   131,   132,    -1,   134,   135,   136,    -1,
     138,   139,   140,   141,   142,    -1,    -1,    -1,   146,   147,
     148,   149,   150,   151,    -1,   153,    -1,   155,    -1,    -1,
     158,    -1,   160,   161,   162,   163,   164,    -1,    -1,    -1,
     168,    -1,    -1,   171,   172,   173,    -1,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
      -1,   189,    -1,   191,   192,   193,   194,   195,   196,   197,
      -1,   199,    -1,   201,   202,    -1,   204,    -1,    -1,   207,
     208,   209,    -1,    -1,   212,    -1,    -1,    -1,   216,   217,
      -1,    -1,    -1,    -1,    -1,    -1,   224,   225,   226,    -1,
      -1,    -1,   230,   231,   232,    -1,   234,   235,   236,   237,
     238,   239,   240,   241,    -1,   243,   244,   245,   246,   247,
     248,   249,   250,   251,   252,   253,    -1,   255,   256,    -1,
     258,   259,   260,   261,   262,   263,   264,    -1,   266,   267,
     268,    -1,   270,    -1,   272,   273,    -1,   275,   276,    -1,
     278,   279,   280,   281,   282,   283,   284,   285,    -1,   287,
      10,   289,   290,    -1,   292,   293,   294,   295,    -1,   297,
     298,    21,   300,    -1,   302,    -1,   304,    -1,    -1,   307,
     308,   309,   310,   311,    -1,    -1,   314,   315,   316,   317,
      40,    -1,   320,   321,   322,   323,   324,   325,    -1,   327,
     328,    -1,    10,   331,   332,   333,   334,   335,   336,    59,
     338,    -1,    -1,    21,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    93,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    59,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,
      -1,    -1,   122,    81,    -1,    -1,    -1,   127,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,    -1,    -1,    -1,    -1,
     140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   122,   123,    -1,    -1,    -1,   127,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   140,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   173,   216,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   236,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   255,    -1,    -1,   216,   259,
     218,   261,    -1,    -1,    -1,    -1,   266,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   236,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   291,   292,    -1,    -1,    -1,    -1,   255,    -1,    -1,
      -1,   259,    -1,   261,   304,    -1,    -1,    -1,    -1,   309,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   318,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   327,    -1,    -1,
      -1,    -1,    -1,    -1,   292,   293,    -1,   295,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   304,    -1,    -1,    -1,
     308,   309,    -1,    -1,    -1,   313,    -1,    -1,    -1,    -1,
     318
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,     3,    13,    14,    15,    28,    46,    48,    49,    53,
      54,    61,    62,    77,    80,    86,    95,   101,   106,   108,
     112,   124,   148,   171,   172,   177,   185,   202,   231,   243,
     245,   249,   253,   256,   260,   265,   270,   273,   279,   307,
     315,   317,   320,   359,   367,   368,   369,   370,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   384,   385,   389,
     400,   401,   402,   405,   406,   414,   415,   424,   451,   456,
     457,   464,   469,   471,   473,   474,   487,   488,   489,   490,
     495,   500,   501,   506,   507,   510,   513,   514,   524,   525,
     528,   534,   548,   551,   552,   554,   557,   559,   560,   563,
     565,   566,   567,   575,   576,   577,   578,   579,   584,   585,
     586,   590,   591,   594,   595,   596,   598,   599,   600,   601,
     602,   607,   610,   614,   616,   617,   621,   623,   627,   628,
     631,   632,   633,   634,   635,   302,   333,   580,    10,    59,
      75,    93,   122,   127,   140,   162,   216,   255,   261,   266,
     291,   292,   304,   309,   318,   580,     3,     4,     5,     6,
       7,     8,     9,    10,    12,    13,    21,    22,    24,    26,
      27,    28,    30,    32,    33,    35,    36,    37,    38,    41,
      42,    43,    44,    46,    47,    48,    49,    50,    53,    54,
      55,    56,    58,    59,    60,    61,    63,    64,    65,    67,
      73,    74,    75,    76,    77,    78,    79,    80,    82,    84,
      85,    86,    87,    88,    90,    93,    94,    95,    96,    98,
      99,   100,   102,   104,   105,   106,   107,   108,   109,   110,
     112,   113,   114,   116,   118,   122,   123,   125,   126,   128,
     130,   131,   132,   134,   135,   136,   138,   139,   140,   141,
     142,   145,   146,   147,   148,   149,   150,   151,   153,   155,
     158,   160,   161,   162,   163,   164,   166,   168,   171,   172,
     173,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   189,   191,   192,   193,   194,   195,   196,
     197,   198,   199,   201,   202,   204,   206,   207,   208,   209,
     212,   216,   217,   220,   223,   224,   225,   226,   228,   229,
     230,   231,   232,   234,   235,   236,   237,   238,   239,   240,
     241,   243,   244,   245,   246,   247,   248,   249,   250,   251,
     252,   253,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   266,   267,   268,   270,   271,   272,   273,   275,
     276,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   289,   290,   292,   293,   294,   295,   297,   298,   300,
     302,   303,   304,   305,   307,   308,   309,   310,   311,   314,
     315,   316,   317,   320,   321,   322,   323,   324,   325,   327,
     328,   331,   332,   333,   334,   335,   336,   338,   736,   748,
     752,   753,   190,   213,   732,   734,   740,   748,   756,   214,
     232,   580,    31,   420,    10,    21,    40,    57,    75,    81,
      93,   123,   127,   173,   216,   218,   255,   261,   292,   293,
     295,   304,   308,   309,   313,   318,   425,   465,   498,   529,
     535,   231,   736,   736,   120,    10,    21,    40,    59,    75,
      93,   122,   127,   140,   216,   236,   255,   259,   261,   266,
     291,   292,   304,   309,   318,   327,   470,   502,   580,   736,
     602,   609,     4,    11,    26,   113,   118,   164,   191,   234,
     244,   344,   351,   511,   736,   747,    11,    62,   242,   265,
     515,   516,   517,   748,   154,   734,   748,   340,   741,   745,
     291,   638,   511,   736,   734,   302,   736,    75,   140,   290,
     291,   561,   260,   748,    11,   268,   297,   302,   391,   748,
       8,   124,   515,   516,   232,   580,   748,    11,    91,   640,
      58,   173,   186,   255,   268,   297,   302,   390,   391,    11,
     268,   297,   302,   391,   302,   638,   352,   734,   215,   662,
     734,   121,   604,   632,   633,     0,   363,   326,   603,   103,
     152,   219,   312,   641,   642,    25,    29,    31,    66,   119,
     121,   133,   144,   156,   157,   159,   167,   169,   188,   203,
     221,   222,   254,   274,   326,   338,   732,   742,   750,   752,
     754,   504,   748,   737,   748,   504,   523,   742,   746,   748,
     662,   736,    47,   343,   346,   347,   348,   350,   351,   352,
     353,   354,   355,   556,   697,   698,   748,   746,   736,   662,
     734,   662,   736,   736,   504,   746,   158,   239,   581,   582,
     583,   357,   362,   722,   723,   214,    10,    40,    52,    57,
      59,    75,    93,   122,   140,   163,   216,   259,   261,   266,
     291,   304,   309,   327,   470,   508,   745,   734,   742,   736,
     359,   304,   737,   504,   293,   295,   746,   293,   295,    47,
     556,   248,   746,   386,   748,   736,   736,   504,   746,   266,
     291,   327,   470,    59,   140,   122,   259,   736,   629,   662,
     742,   736,   359,   737,   742,   735,   736,    47,   556,   735,
     736,   736,   736,   735,   162,   503,   504,   359,   615,   603,
     747,    11,   747,    11,   747,   747,   344,   120,   137,   512,
     235,   214,   299,   364,   734,   733,   734,   512,   745,   359,
     611,   736,   736,   734,   748,    25,   336,   158,   362,   217,
     217,   214,   120,   745,   299,   214,    18,    30,    32,    33,
      39,    40,    42,    43,    50,    60,    68,    69,    70,    71,
      72,    78,    79,    94,   107,   110,   111,   114,   126,   150,
     151,   153,   166,   174,   175,   187,   189,   200,   205,   206,
     207,   216,   223,   228,   240,   257,   269,   276,   286,   297,
     298,   303,   305,   306,   313,   318,   324,   338,   339,   341,
     342,   343,   344,   345,   350,   351,   352,   359,   632,   670,
     671,   672,   677,   678,   679,   681,   682,   683,   684,   687,
     688,   691,   693,   694,   695,   699,   716,   721,   726,   727,
     732,   742,   743,   744,   745,   749,   752,    11,   403,   735,
     268,   390,    81,   398,   745,   340,   399,   748,    25,    44,
     390,   336,   582,   299,   346,    25,   336,   158,   583,   733,
     359,   734,   270,   352,   119,   605,   360,   360,   369,   734,
      11,    91,   639,   639,    35,   639,   115,   170,   211,   645,
     651,   723,   359,   224,   246,   362,   505,   224,   246,   270,
     331,   371,   401,     7,    95,   224,   270,   411,    37,   109,
     135,   252,   264,   278,   285,   328,   544,   549,   359,   536,
       7,    95,   246,   383,   224,   246,   270,   409,   410,   246,
     504,   359,   362,   246,   270,   371,   401,   224,   246,   270,
     458,     7,    13,    49,    62,    90,    95,    98,   246,   270,
     407,   408,   410,   224,   246,   214,   224,   270,   246,   270,
     371,   401,   168,   215,   334,   364,   581,   691,    11,    14,
      15,    16,    17,    18,    19,    20,    23,    34,    39,    40,
      45,    51,    52,    57,    62,    68,    69,    70,    71,    72,
      81,    83,    89,    91,    92,    97,   101,   103,   111,   115,
     117,   120,   124,   127,   129,   137,   143,   152,   154,   165,
     170,   174,   175,   190,   200,   205,   210,   211,   213,   214,
     215,   218,   219,   227,   233,   242,   265,   269,   277,   288,
     291,   296,   299,   301,   306,   312,   313,   318,   319,   329,
     330,   338,   352,   739,   751,   752,   753,   754,   755,   722,
     734,   742,   359,   736,   742,   208,    47,   556,   736,   736,
     162,   504,   359,   438,   359,   491,    45,    30,    32,    33,
      42,    43,    78,    79,   114,   150,   151,   153,   187,   189,
     207,   240,   271,   276,   297,   298,   324,   338,   667,   669,
     671,   672,   676,   678,   679,   680,   682,   683,   687,   688,
     749,   752,   736,   371,    19,   597,   371,   504,   491,   425,
     371,    25,   387,   224,   472,     9,    27,   475,    19,   491,
     371,   734,   734,   734,   162,   504,   740,   748,   742,   736,
      31,    73,   147,   192,   262,   319,   622,   359,    38,   251,
     412,   667,   536,   364,   504,   359,   214,   214,   399,   364,
     412,   691,   702,    62,   608,   614,   617,   621,   627,   628,
     631,   736,    75,   122,   162,   261,   291,   292,   518,   733,
     735,   517,    81,   323,   359,   618,   631,   137,   364,   624,
     736,   612,   667,    19,   116,   562,   562,   562,   168,   748,
     115,   115,   518,   735,   260,   748,   359,   357,   632,   706,
     325,   685,   691,   720,   359,   685,   685,   359,   359,   359,
     359,   359,   675,   675,   229,   632,   359,   359,   673,   359,
     359,   359,   359,    42,    43,   685,   691,   359,   359,   674,
     359,   359,   359,   359,   359,   331,   332,   359,   689,   359,
     689,   359,   359,   632,   724,   691,   691,   632,   691,   702,
     745,    43,   359,   686,   359,   745,    16,    19,    24,    29,
     133,   137,   156,   157,   169,   200,   203,   216,   218,   274,
     343,   346,   347,   348,   350,   351,   352,   353,   354,   355,
     361,   697,   699,   701,   222,   691,   154,   364,   636,   723,
     359,    84,   134,   404,    44,    81,   399,    19,    81,   173,
     338,   339,   351,   397,   461,   462,   463,   688,   745,   747,
      81,   111,   210,   214,   306,   392,   393,   394,   396,   399,
     461,   392,   168,   734,   728,   729,   748,   603,   359,   606,
     632,   634,   635,   634,   643,   644,   691,   634,   239,   272,
     317,    11,   647,   691,   648,   691,   651,   652,   645,   646,
     352,   553,   667,   299,   299,   739,   362,   299,   299,   390,
     592,    45,    57,   117,   233,   313,   436,   437,    57,    81,
     200,   299,    81,   200,   261,   214,   264,   205,    85,   155,
     251,   544,   550,   137,   145,   220,   271,   338,   360,   537,
     538,   539,   540,   542,   667,   749,   750,   752,   224,   246,
     270,   299,   318,   299,   299,   292,   364,   299,   319,   198,
     555,   667,   556,   299,   390,   372,   299,   299,   261,    36,
      74,   139,   180,   182,   192,   250,   279,   459,    52,   436,
     564,   564,   214,   300,   304,    57,   564,   304,   299,   564,
     261,   332,   364,   299,   299,   662,   299,   261,   299,   390,
     372,   239,   247,   267,   395,   581,   358,   365,   359,   667,
     214,   536,   461,   504,   359,   156,   214,   214,   504,   156,
     439,   440,   748,   331,   421,   492,   493,   751,   359,   669,
      19,    18,   668,    76,   132,   181,   184,   263,   335,   359,
     690,   505,     9,   587,   667,   372,   498,   327,   372,   746,
      62,   124,   388,   424,   456,   474,   513,   528,   584,   736,
     176,    86,   148,   317,   476,   477,   359,   372,   458,   209,
     359,   453,   438,   399,   115,   214,   536,    19,   331,   332,
     630,   262,   359,   632,   655,   656,   657,   662,   663,   694,
     732,   330,   664,   553,    19,   412,   736,   319,   555,   734,
     734,   412,   504,   360,   364,   425,   735,   522,   523,   742,
     735,   735,   733,   735,   299,   331,   526,   323,   359,   619,
     620,   748,     5,   105,   257,   272,   625,   734,   204,   626,
     360,   364,   613,   617,   621,   627,   631,   516,   515,   516,
     120,   125,   527,   748,   702,   702,   705,   706,   359,   329,
     717,   718,   691,   702,   691,   702,   744,   744,   744,    76,
     132,   181,   184,   263,   335,   338,   340,   703,   707,   744,
     702,   702,   744,   744,   685,   685,   691,   744,   556,   691,
     708,   350,   351,   692,   693,   695,   699,   710,   360,   702,
     691,   702,   711,   297,   297,   744,   744,   691,    34,   120,
     165,   301,   691,   702,   714,   722,   360,   364,   270,   744,
     744,   690,   691,   751,   297,    23,   288,   725,   691,   359,
     632,   715,    91,   111,   200,   205,   209,   306,   314,   691,
      29,   133,   137,   169,   274,   359,   691,   299,   691,   691,
     691,   691,   691,   691,   691,   691,   691,   667,   691,    11,
      17,   277,   696,   257,   359,   695,   123,   173,   291,   293,
     295,   637,   734,   727,   120,   654,    11,    91,   352,   360,
     702,   302,   339,   359,   745,   364,   360,   364,   654,   724,
     601,   734,   735,   364,    20,    89,   319,   215,   209,   653,
     653,   211,   364,   170,   360,   746,   736,   739,   746,   737,
      56,   593,   359,   736,   160,   160,   359,   736,   205,   746,
     691,   205,   736,   205,    85,   155,   214,   220,   749,   360,
     364,   540,   542,   539,   542,   505,   299,   299,   261,   746,
     735,   746,   736,   736,   410,   736,   738,   748,   364,   360,
     364,   746,     8,    56,    63,    64,    65,   100,   137,   141,
     178,   193,   194,   195,   196,   197,   199,   226,   255,   287,
     289,   311,   318,   321,   373,   746,   736,   736,   461,    35,
     460,   461,   461,    74,   180,   182,   371,   371,   429,   748,
     748,   736,   291,    11,   318,   736,   736,   748,    11,   318,
     736,   736,   736,   736,    49,   212,   270,   408,   746,   736,
     246,   746,   736,   746,    55,   310,   239,   691,   553,    19,
     504,   156,   156,   319,   555,   205,   509,   745,   504,   504,
     156,   509,   360,   364,   212,   120,   299,   416,   360,   364,
     346,   691,    18,   668,   667,   357,   357,   299,   299,   299,
     299,   744,   476,    56,    99,   176,   224,   292,   294,   588,
     430,   115,   734,   387,   218,   425,   745,   214,   218,   665,
     666,   748,   734,   169,   426,   427,   428,   429,   434,   436,
     454,   455,   748,    19,   331,   332,   452,    19,   128,   745,
     734,    19,   162,   252,   543,   544,   545,   568,   131,   131,
     115,   632,   656,   657,    19,   658,   748,   364,    66,   121,
     144,   159,   167,   188,   254,   337,   659,   658,    19,   658,
     748,   723,   691,   360,   667,   738,   360,   412,   412,   691,
     291,   364,   536,   127,   519,   520,   746,     8,   527,    81,
     691,   730,   731,   360,   364,   724,   105,   272,   105,   272,
     257,   317,   183,   667,   120,   214,   519,    35,   412,   360,
     358,   358,   364,   744,   691,    97,   718,   719,    19,   360,
     319,   360,   360,   360,   360,   364,   360,   120,   360,   360,
     360,   360,   360,   364,   360,   364,   360,   227,   709,   360,
     692,   692,   137,   156,   216,   346,   347,   348,   350,   351,
     352,   353,   354,   355,   361,   699,   692,   360,   360,   115,
     120,   712,   713,   360,   336,   336,   360,   360,    19,   714,
     702,   714,   714,   120,   360,   724,   691,   748,   360,   360,
     336,   692,   692,   102,   702,   120,   111,   205,   209,   306,
     314,   359,   102,   288,   725,   691,   715,   691,   299,   556,
     691,   359,   632,   293,   295,   293,   295,   734,   638,   638,
     655,   664,   702,   702,   360,   360,   582,   744,   690,   394,
     729,   664,   346,   360,   644,   216,   697,   700,   735,   626,
     626,   648,   648,   647,   224,   246,   270,   170,   691,   437,
     359,   359,   439,   412,   146,   205,   505,   538,   542,   542,
     354,   746,   736,   736,   224,   246,   667,   224,   198,   667,
     735,   170,   226,   127,   255,   745,   735,   744,   226,   735,
     316,   461,   461,   461,   667,    95,   270,   309,   411,   412,
     412,   299,   299,   358,   360,   667,   156,   509,   509,   738,
     360,   156,   156,   509,   440,   282,   283,   417,   745,   493,
     216,   351,   461,   494,   542,   700,   745,   360,   357,   360,
     744,   358,   744,   132,   181,   263,   181,   263,   263,   184,
     360,   214,   170,   346,   589,   589,   589,   589,   589,    45,
      57,    81,    83,   143,   200,   205,   233,   242,   313,   431,
     432,   433,   309,   438,   248,   291,   734,   477,   360,   364,
     667,   359,   734,   360,   364,   360,   364,   106,   212,   212,
     631,   631,   466,   736,   299,   319,   530,   546,   745,   399,
     541,   542,   331,   545,   547,   214,   631,   360,   748,   359,
     656,   159,   221,   660,   656,   660,   159,   659,   660,   656,
     159,   359,   748,   359,   412,   360,   412,   412,   734,   523,
     746,   331,   364,   521,   217,   360,   364,   323,   631,   620,
     105,   105,   735,   518,   412,   746,   706,   360,   296,   691,
     101,   667,   504,   744,   691,   691,   744,   691,   712,   692,
      91,   200,   209,   692,   692,   692,   692,   692,   692,   692,
     692,   692,   667,   692,   691,   691,   713,   712,   689,   689,
     667,   360,   360,   360,   702,   360,   686,   745,   691,    16,
      16,   691,   360,   691,   359,   667,   704,   691,   692,   692,
     102,   102,   691,   360,   102,   691,   638,   638,   638,   638,
     734,   734,   127,   649,   360,   360,   360,    81,   691,   359,
     299,   299,   261,   589,   360,   439,   439,   360,   146,   354,
     309,   299,   299,   299,   747,   745,   735,   735,   745,   745,
     430,   200,   200,   281,   284,   667,   736,   736,   156,   360,
     509,   156,   156,   509,   509,   319,   422,   423,    83,   143,
     200,   484,   485,   486,   744,   331,   332,   358,   358,   690,
     734,   589,    81,   744,   745,    81,   745,    81,   736,    81,
     736,    81,   736,   359,   736,   692,    84,   134,    83,   205,
     160,   734,   319,   450,   667,    19,   734,   115,   478,   218,
     666,   426,   104,   138,   435,   142,   446,   428,   455,   748,
     736,    19,    19,   322,   467,   505,   745,   738,   359,   364,
     543,   491,    86,   148,   265,   317,   573,   658,   359,   735,
     656,   214,   319,   661,   656,   159,   656,   665,   359,   665,
     748,   412,   359,   453,   124,   520,   731,   359,   527,   120,
     691,   360,   360,   360,   360,   360,   713,   120,   209,   359,
     360,   690,   692,   692,   704,   360,   364,    16,    16,   691,
     691,   102,   691,   360,   734,   734,   734,   734,    35,   129,
     650,   745,   556,   746,   736,   736,   747,   360,   360,   450,
     309,   746,   736,   746,   205,   205,   463,   748,   319,   413,
     509,   156,   509,   509,   371,    88,    84,   134,    83,   486,
     485,   358,   122,   122,   120,   483,   747,   691,   200,   432,
     450,   438,   140,   319,   631,   359,    96,   479,   106,   477,
     360,    82,    82,   359,   331,   332,   447,   615,   466,   161,
     468,   120,   359,   531,   532,   694,   748,   745,   547,   299,
     735,   360,   691,   359,   656,   661,   360,   665,   360,    19,
     217,   730,   412,   519,   692,   359,   704,   360,   667,   692,
     692,   691,   702,   691,   690,   360,   242,   450,   691,   509,
     418,   745,   523,    19,   558,   734,   484,   360,   179,   441,
     292,   738,   257,   280,   480,   237,   447,   733,   212,   212,
     214,   448,   745,   504,   691,   360,   364,   319,   504,   533,
     533,   734,   360,   735,   360,   360,   412,   704,   360,   734,
      31,    67,    87,   102,   116,   130,   205,   212,   238,   419,
     558,    22,   136,   115,   121,   225,   275,   214,   442,   443,
     444,   736,    19,   742,   448,   360,    54,   292,   449,   360,
     449,   532,   504,   664,   360,   360,   438,   597,   597,   200,
     238,   597,   597,    96,    86,   317,   214,   444,   214,   443,
     122,   216,   284,   496,   497,   359,   449,    86,    95,   230,
     736,   533,   664,    92,   441,   745,   745,   205,   439,   745,
     745,   257,    38,   192,   251,   270,   445,   445,   744,   744,
     667,   364,   339,   341,   342,   344,   481,   482,   745,   748,
     258,   258,    12,   149,   574,   442,   439,   106,     6,    81,
     205,   742,   556,   497,   360,   364,   201,   359,   569,   571,
     576,   617,   621,   627,   631,   484,   237,   536,   241,   359,
     499,   482,   570,   571,   572,   632,   633,   742,   555,   360,
     363,   359,   360,   572,   481,   499,   360
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 478 "gram.y"
    { parsetree = yyvsp[0].list; }
    break;

  case 3:
#line 483 "gram.y"
    { if (yyvsp[0].node != NULL)
					yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node);
				  else
					yyval.list = yyvsp[-2].list;
				}
    break;

  case 4:
#line 489 "gram.y"
    { if (yyvsp[0].node != NULL)
						yyval.list = list_make1(yyvsp[0].node);
					  else
						yyval.list = NIL;
					}
    break;

  case 88:
#line 581 "gram.y"
    { yyval.node = NULL; }
    break;

  case 89:
#line 592 "gram.y"
    {
					CreateRoleStmt *n = makeNode(CreateRoleStmt);
					n->stmt_type = ROLESTMT_ROLE;
					n->role = yyvsp[-2].str;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 90:
#line 602 "gram.y"
    {}
    break;

  case 91:
#line 603 "gram.y"
    {}
    break;

  case 92:
#line 612 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].defelt); }
    break;

  case 93:
#line 613 "gram.y"
    { yyval.list = NIL; }
    break;

  case 94:
#line 618 "gram.y"
    {
					yyval.defelt = makeDefElem("password",
									 (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 95:
#line 623 "gram.y"
    {
					yyval.defelt = makeDefElem("encryptedPassword",
									 (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 96:
#line 628 "gram.y"
    {
					yyval.defelt = makeDefElem("unencryptedPassword",
									 (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 97:
#line 633 "gram.y"
    {
					yyval.defelt = makeDefElem("superuser", (Node *)makeInteger(TRUE));
				}
    break;

  case 98:
#line 637 "gram.y"
    {
					yyval.defelt = makeDefElem("superuser", (Node *)makeInteger(FALSE));
				}
    break;

  case 99:
#line 641 "gram.y"
    {
					yyval.defelt = makeDefElem("inherit", (Node *)makeInteger(TRUE));
				}
    break;

  case 100:
#line 645 "gram.y"
    {
					yyval.defelt = makeDefElem("inherit", (Node *)makeInteger(FALSE));
				}
    break;

  case 101:
#line 649 "gram.y"
    {
					yyval.defelt = makeDefElem("createdb", (Node *)makeInteger(TRUE));
				}
    break;

  case 102:
#line 653 "gram.y"
    {
					yyval.defelt = makeDefElem("createdb", (Node *)makeInteger(FALSE));
				}
    break;

  case 103:
#line 657 "gram.y"
    {
					yyval.defelt = makeDefElem("createrole", (Node *)makeInteger(TRUE));
				}
    break;

  case 104:
#line 661 "gram.y"
    {
					yyval.defelt = makeDefElem("createrole", (Node *)makeInteger(FALSE));
				}
    break;

  case 105:
#line 665 "gram.y"
    {
					/* For backwards compatibility, synonym for SUPERUSER */
					yyval.defelt = makeDefElem("superuser", (Node *)makeInteger(TRUE));
				}
    break;

  case 106:
#line 670 "gram.y"
    {
					yyval.defelt = makeDefElem("superuser", (Node *)makeInteger(FALSE));
				}
    break;

  case 107:
#line 674 "gram.y"
    {
					yyval.defelt = makeDefElem("canlogin", (Node *)makeInteger(TRUE));
				}
    break;

  case 108:
#line 678 "gram.y"
    {
					yyval.defelt = makeDefElem("canlogin", (Node *)makeInteger(FALSE));
				}
    break;

  case 109:
#line 682 "gram.y"
    {
					yyval.defelt = makeDefElem("connectionlimit", (Node *)makeInteger(yyvsp[0].ival));
				}
    break;

  case 110:
#line 686 "gram.y"
    {
					yyval.defelt = makeDefElem("validUntil", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 111:
#line 691 "gram.y"
    {
					yyval.defelt = makeDefElem("rolemembers", (Node *)yyvsp[0].list);
				}
    break;

  case 112:
#line 696 "gram.y"
    {
					yyval.defelt = makeDefElem("sysid", (Node *)makeInteger(yyvsp[0].ival));
				}
    break;

  case 113:
#line 700 "gram.y"
    {
					yyval.defelt = makeDefElem("adminmembers", (Node *)yyvsp[0].list);
				}
    break;

  case 114:
#line 704 "gram.y"
    {
					yyval.defelt = makeDefElem("rolemembers", (Node *)yyvsp[0].list);
				}
    break;

  case 115:
#line 708 "gram.y"
    {
					yyval.defelt = makeDefElem("addroleto", (Node *)yyvsp[0].list);
				}
    break;

  case 116:
#line 712 "gram.y"
    {
					yyval.defelt = makeDefElem("addroleto", (Node *)yyvsp[0].list);
				}
    break;

  case 117:
#line 726 "gram.y"
    {
					CreateRoleStmt *n = makeNode(CreateRoleStmt);
					n->stmt_type = ROLESTMT_USER;
					n->role = yyvsp[-2].str;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 118:
#line 744 "gram.y"
    {
					AlterRoleStmt *n = makeNode(AlterRoleStmt);
					n->role = yyvsp[-2].str;
					n->action = +1;	/* add, if there are members */
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				 }
    break;

  case 119:
#line 755 "gram.y"
    {
					AlterRoleSetStmt *n = makeNode(AlterRoleSetStmt);
					n->role = yyvsp[-2].str;
					n->variable = yyvsp[0].vsetstmt->name;
					n->value = yyvsp[0].vsetstmt->args;
					yyval.node = (Node *)n;
				}
    break;

  case 120:
#line 763 "gram.y"
    {
					AlterRoleSetStmt *n = makeNode(AlterRoleSetStmt);
					n->role = yyvsp[-1].str;
					n->variable = ((VariableResetStmt *)yyvsp[0].node)->name;
					n->value = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 121:
#line 781 "gram.y"
    {
					AlterRoleStmt *n = makeNode(AlterRoleStmt);
					n->role = yyvsp[-2].str;
					n->action = +1;	/* add, if there are members */
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				 }
    break;

  case 122:
#line 793 "gram.y"
    {
					AlterRoleSetStmt *n = makeNode(AlterRoleSetStmt);
					n->role = yyvsp[-2].str;
					n->variable = yyvsp[0].vsetstmt->name;
					n->value = yyvsp[0].vsetstmt->args;
					yyval.node = (Node *)n;
				}
    break;

  case 123:
#line 801 "gram.y"
    {
					AlterRoleSetStmt *n = makeNode(AlterRoleSetStmt);
					n->role = yyvsp[-1].str;
					n->variable = ((VariableResetStmt *)yyvsp[0].node)->name;
					n->value = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 124:
#line 822 "gram.y"
    {
					DropRoleStmt *n = makeNode(DropRoleStmt);
					n->roles = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 125:
#line 840 "gram.y"
    {
					DropRoleStmt *n = makeNode(DropRoleStmt);
					n->roles = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 126:
#line 856 "gram.y"
    {
					CreateRoleStmt *n = makeNode(CreateRoleStmt);
					n->stmt_type = ROLESTMT_GROUP;
					n->role = yyvsp[-2].str;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 127:
#line 874 "gram.y"
    {
					AlterRoleStmt *n = makeNode(AlterRoleStmt);
					n->role = yyvsp[-3].str;
					n->action = yyvsp[-2].ival;
					n->options = list_make1(makeDefElem("rolemembers",
														(Node *)yyvsp[0].list));
					yyval.node = (Node *)n;
				}
    break;

  case 128:
#line 884 "gram.y"
    { yyval.ival = +1; }
    break;

  case 129:
#line 885 "gram.y"
    { yyval.ival = -1; }
    break;

  case 130:
#line 898 "gram.y"
    {
					DropRoleStmt *n = makeNode(DropRoleStmt);
					n->roles = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 131:
#line 914 "gram.y"
    {
					CreateSchemaStmt *n = makeNode(CreateSchemaStmt);
					/* One can omit the schema name or the authorization id. */
					if (yyvsp[-3].str != NULL)
						n->schemaname = yyvsp[-3].str;
					else
						n->schemaname = yyvsp[-1].str;
					n->authid = yyvsp[-1].str;
					n->schemaElts = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 132:
#line 926 "gram.y"
    {
					CreateSchemaStmt *n = makeNode(CreateSchemaStmt);
					/* ...but not both */
					n->schemaname = yyvsp[-1].str;
					n->authid = NULL;
					n->schemaElts = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 133:
#line 937 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 134:
#line 938 "gram.y"
    { yyval.str = NULL; }
    break;

  case 135:
#line 942 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].node); }
    break;

  case 136:
#line 943 "gram.y"
    { yyval.list = NIL; }
    break;

  case 143:
#line 971 "gram.y"
    {
					VariableSetStmt *n = yyvsp[0].vsetstmt;
					n->is_local = false;
					yyval.node = (Node *) n;
				}
    break;

  case 144:
#line 977 "gram.y"
    {
					VariableSetStmt *n = yyvsp[0].vsetstmt;
					n->is_local = true;
					yyval.node = (Node *) n;
				}
    break;

  case 145:
#line 983 "gram.y"
    {
					VariableSetStmt *n = yyvsp[0].vsetstmt;
					n->is_local = false;
					yyval.node = (Node *) n;
				}
    break;

  case 146:
#line 991 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = yyvsp[-2].str;
					n->args = yyvsp[0].list;
					yyval.vsetstmt = n;
				}
    break;

  case 147:
#line 998 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = yyvsp[-2].str;
					n->args = yyvsp[0].list;
					yyval.vsetstmt = n;
				}
    break;

  case 148:
#line 1005 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = "timezone";
					if (yyvsp[0].node != NULL)
						n->args = list_make1(yyvsp[0].node);
					yyval.vsetstmt = n;
				}
    break;

  case 149:
#line 1013 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = "TRANSACTION";
					n->args = yyvsp[0].list;
					yyval.vsetstmt = n;
				}
    break;

  case 150:
#line 1020 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = "SESSION CHARACTERISTICS";
					n->args = yyvsp[0].list;
					yyval.vsetstmt = n;
				}
    break;

  case 151:
#line 1027 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = "client_encoding";
					if (yyvsp[0].str != NULL)
						n->args = list_make1(makeStringConst(yyvsp[0].str, NULL));
					yyval.vsetstmt = n;
				}
    break;

  case 152:
#line 1035 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = "role";
					n->args = list_make1(makeStringConst(yyvsp[0].str, NULL));
					yyval.vsetstmt = n;
				}
    break;

  case 153:
#line 1042 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = "session_authorization";
					n->args = list_make1(makeStringConst(yyvsp[0].str, NULL));
					yyval.vsetstmt = n;
				}
    break;

  case 154:
#line 1049 "gram.y"
    {
					VariableSetStmt *n = makeNode(VariableSetStmt);
					n->name = "session_authorization";
					n->args = NIL;
					yyval.vsetstmt = n;
				}
    break;

  case 155:
#line 1058 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 156:
#line 1060 "gram.y"
    {
					int qLen = strlen(yyvsp[-2].str);
					char* qualName = palloc(qLen + strlen(yyvsp[0].str) + 2);
					strcpy(qualName, yyvsp[-2].str);
					qualName[qLen] = '.';
					strcpy(qualName + qLen + 1, yyvsp[0].str);
					yyval.str = qualName;
				}
    break;

  case 157:
#line 1071 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 158:
#line 1072 "gram.y"
    { yyval.list = NIL; }
    break;

  case 159:
#line 1075 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 160:
#line 1076 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node); }
    break;

  case 161:
#line 1080 "gram.y"
    { yyval.node = makeStringConst(yyvsp[0].str, NULL); }
    break;

  case 162:
#line 1082 "gram.y"
    { yyval.node = makeStringConst(yyvsp[0].str, NULL); }
    break;

  case 163:
#line 1084 "gram.y"
    { yyval.node = makeAConst(yyvsp[0].value); }
    break;

  case 164:
#line 1087 "gram.y"
    { yyval.str = "read uncommitted"; }
    break;

  case 165:
#line 1088 "gram.y"
    { yyval.str = "read committed"; }
    break;

  case 166:
#line 1089 "gram.y"
    { yyval.str = "repeatable read"; }
    break;

  case 167:
#line 1090 "gram.y"
    { yyval.str = "serializable"; }
    break;

  case 168:
#line 1094 "gram.y"
    { yyval.str = "true"; }
    break;

  case 169:
#line 1095 "gram.y"
    { yyval.str = "false"; }
    break;

  case 170:
#line 1096 "gram.y"
    { yyval.str = "on"; }
    break;

  case 171:
#line 1097 "gram.y"
    { yyval.str = "off"; }
    break;

  case 172:
#line 1110 "gram.y"
    {
					yyval.node = makeStringConst(yyvsp[0].str, NULL);
				}
    break;

  case 173:
#line 1114 "gram.y"
    {
					yyval.node = makeStringConst(yyvsp[0].str, NULL);
				}
    break;

  case 174:
#line 1118 "gram.y"
    {
					A_Const *n = (A_Const *) makeStringConst(yyvsp[-1].str, yyvsp[-2].typnam);
					if (yyvsp[0].ival != INTERVAL_FULL_RANGE)
					{
						if ((yyvsp[0].ival & ~(INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE))) != 0)
							ereport(ERROR,
									(errcode(ERRCODE_SYNTAX_ERROR),
									 errmsg("time zone interval must be HOUR or HOUR TO MINUTE")));
						n->typename->typmod = INTERVAL_TYPMOD(INTERVAL_FULL_PRECISION, yyvsp[0].ival);
					}
					yyval.node = (Node *)n;
				}
    break;

  case 175:
#line 1131 "gram.y"
    {
					A_Const *n = (A_Const *) makeStringConst(yyvsp[-1].str, yyvsp[-5].typnam);
					if (yyvsp[-3].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("INTERVAL(%d) precision must not be negative",
										yyvsp[-3].ival)));
					if (yyvsp[-3].ival > MAX_INTERVAL_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("INTERVAL(%d) precision reduced to maximum allowed, %d",
										yyvsp[-3].ival, MAX_INTERVAL_PRECISION)));
						yyvsp[-3].ival = MAX_INTERVAL_PRECISION;
					}

					if ((yyvsp[0].ival != INTERVAL_FULL_RANGE)
						&& ((yyvsp[0].ival & ~(INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE))) != 0))
						ereport(ERROR,
								(errcode(ERRCODE_SYNTAX_ERROR),
								 errmsg("time zone interval must be HOUR or HOUR TO MINUTE")));

					n->typename->typmod = INTERVAL_TYPMOD(yyvsp[-3].ival, yyvsp[0].ival);

					yyval.node = (Node *)n;
				}
    break;

  case 176:
#line 1157 "gram.y"
    { yyval.node = makeAConst(yyvsp[0].value); }
    break;

  case 177:
#line 1158 "gram.y"
    { yyval.node = NULL; }
    break;

  case 178:
#line 1159 "gram.y"
    { yyval.node = NULL; }
    break;

  case 179:
#line 1163 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 180:
#line 1164 "gram.y"
    { yyval.str = NULL; }
    break;

  case 181:
#line 1165 "gram.y"
    { yyval.str = NULL; }
    break;

  case 182:
#line 1169 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 183:
#line 1170 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 184:
#line 1176 "gram.y"
    {
					VariableShowStmt *n = makeNode(VariableShowStmt);
					n->name = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 185:
#line 1182 "gram.y"
    {
					VariableShowStmt *n = makeNode(VariableShowStmt);
					n->name = "timezone";
					yyval.node = (Node *) n;
				}
    break;

  case 186:
#line 1188 "gram.y"
    {
					VariableShowStmt *n = makeNode(VariableShowStmt);
					n->name = "transaction_isolation";
					yyval.node = (Node *) n;
				}
    break;

  case 187:
#line 1194 "gram.y"
    {
					VariableShowStmt *n = makeNode(VariableShowStmt);
					n->name = "session_authorization";
					yyval.node = (Node *) n;
				}
    break;

  case 188:
#line 1200 "gram.y"
    {
					VariableShowStmt *n = makeNode(VariableShowStmt);
					n->name = "all";
					yyval.node = (Node *) n;
				}
    break;

  case 189:
#line 1209 "gram.y"
    {
					VariableResetStmt *n = makeNode(VariableResetStmt);
					n->name = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 190:
#line 1215 "gram.y"
    {
					VariableResetStmt *n = makeNode(VariableResetStmt);
					n->name = "timezone";
					yyval.node = (Node *) n;
				}
    break;

  case 191:
#line 1221 "gram.y"
    {
					VariableResetStmt *n = makeNode(VariableResetStmt);
					n->name = "transaction_isolation";
					yyval.node = (Node *) n;
				}
    break;

  case 192:
#line 1227 "gram.y"
    {
					VariableResetStmt *n = makeNode(VariableResetStmt);
					n->name = "session_authorization";
					yyval.node = (Node *) n;
				}
    break;

  case 193:
#line 1233 "gram.y"
    {
					VariableResetStmt *n = makeNode(VariableResetStmt);
					n->name = "all";
					yyval.node = (Node *) n;
				}
    break;

  case 194:
#line 1243 "gram.y"
    {
					ConstraintsSetStmt *n = makeNode(ConstraintsSetStmt);
					n->constraints = yyvsp[-1].list;
					n->deferred    = yyvsp[0].boolean;
					yyval.node = (Node *) n;
				}
    break;

  case 195:
#line 1252 "gram.y"
    { yyval.list = NIL; }
    break;

  case 196:
#line 1253 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 197:
#line 1257 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 198:
#line 1258 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 199:
#line 1267 "gram.y"
    {
					CheckPointStmt *n = makeNode(CheckPointStmt);
					yyval.node = (Node *)n;
				}
    break;

  case 200:
#line 1282 "gram.y"
    {
					AlterTableStmt *n = makeNode(AlterTableStmt);
					n->relation = yyvsp[-1].range;
					n->cmds = yyvsp[0].list;
					n->relkind = OBJECT_TABLE;
					yyval.node = (Node *)n;
				}
    break;

  case 201:
#line 1290 "gram.y"
    {
					AlterTableStmt *n = makeNode(AlterTableStmt);
					n->relation = yyvsp[-1].range;
					n->cmds = yyvsp[0].list;
					n->relkind = OBJECT_INDEX;
					yyval.node = (Node *)n;
				}
    break;

  case 202:
#line 1300 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 203:
#line 1301 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node); }
    break;

  case 204:
#line 1308 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_AddColumn;
					n->def = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 205:
#line 1316 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_ColumnDefault;
					n->name = yyvsp[-1].str;
					n->def = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 206:
#line 1325 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_DropNotNull;
					n->name = yyvsp[-3].str;
					yyval.node = (Node *)n;
				}
    break;

  case 207:
#line 1333 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_SetNotNull;
					n->name = yyvsp[-3].str;
					yyval.node = (Node *)n;
				}
    break;

  case 208:
#line 1341 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_SetStatistics;
					n->name = yyvsp[-3].str;
					n->def = (Node *) yyvsp[0].value;
					yyval.node = (Node *)n;
				}
    break;

  case 209:
#line 1350 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_SetStorage;
					n->name = yyvsp[-3].str;
					n->def = (Node *) makeString(yyvsp[0].str);
					yyval.node = (Node *)n;
				}
    break;

  case 210:
#line 1359 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_DropColumn;
					n->name = yyvsp[-1].str;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 211:
#line 1371 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_AlterColumnType;
					n->name = yyvsp[-3].str;
					n->def = (Node *) yyvsp[-1].typnam;
					n->transform = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 212:
#line 1381 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_AddConstraint;
					n->def = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 213:
#line 1389 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_DropConstraint;
					n->name = yyvsp[-1].str;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 214:
#line 1398 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_DropOids;
					yyval.node = (Node *)n;
				}
    break;

  case 215:
#line 1405 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_ToastTable;
					yyval.node = (Node *)n;
				}
    break;

  case 216:
#line 1412 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_ClusterOn;
					n->name = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 217:
#line 1420 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_DropCluster;
					n->name = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 218:
#line 1428 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_EnableTrig;
					n->name = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 219:
#line 1436 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_EnableTrigAll;
					yyval.node = (Node *)n;
				}
    break;

  case 220:
#line 1443 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_EnableTrigUser;
					yyval.node = (Node *)n;
				}
    break;

  case 221:
#line 1450 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_DisableTrig;
					n->name = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 222:
#line 1458 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_DisableTrigAll;
					yyval.node = (Node *)n;
				}
    break;

  case 223:
#line 1465 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_DisableTrigUser;
					yyval.node = (Node *)n;
				}
    break;

  case 224:
#line 1471 "gram.y"
    {
					yyval.node = yyvsp[0].node;
				}
    break;

  case 225:
#line 1477 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 226:
#line 1478 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node); }
    break;

  case 227:
#line 1485 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_ChangeOwner;
					n->name = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 228:
#line 1493 "gram.y"
    {
					AlterTableCmd *n = makeNode(AlterTableCmd);
					n->subtype = AT_SetTableSpace;
					n->name = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 229:
#line 1503 "gram.y"
    {
					/* Treat SET DEFAULT NULL the same as DROP DEFAULT */
					if (exprIsNullConstant(yyvsp[0].node))
						yyval.node = NULL;
					else
						yyval.node = yyvsp[0].node;
				}
    break;

  case 230:
#line 1510 "gram.y"
    { yyval.node = NULL; }
    break;

  case 231:
#line 1514 "gram.y"
    { yyval.dbehavior = DROP_CASCADE; }
    break;

  case 232:
#line 1515 "gram.y"
    { yyval.dbehavior = DROP_RESTRICT; }
    break;

  case 233:
#line 1516 "gram.y"
    { yyval.dbehavior = DROP_RESTRICT; /* default */ }
    break;

  case 234:
#line 1520 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 235:
#line 1521 "gram.y"
    { yyval.node = NULL; }
    break;

  case 236:
#line 1535 "gram.y"
    {
					ClosePortalStmt *n = makeNode(ClosePortalStmt);
					n->portalname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 237:
#line 1555 "gram.y"
    {
					CopyStmt *n = makeNode(CopyStmt);
					n->relation = yyvsp[-7].range;
					n->attlist = yyvsp[-6].list;
					n->is_from = yyvsp[-4].boolean;
					n->filename = yyvsp[-3].str;

					n->options = NIL;
					/* Concatenate user-supplied flags */
					if (yyvsp[-8].defelt)
						n->options = lappend(n->options, yyvsp[-8].defelt);
					if (yyvsp[-5].defelt)
						n->options = lappend(n->options, yyvsp[-5].defelt);
					if (yyvsp[-2].defelt)
						n->options = lappend(n->options, yyvsp[-2].defelt);
					if (yyvsp[0].list)
						n->options = list_concat(n->options, yyvsp[0].list);
					yyval.node = (Node *)n;
				}
    break;

  case 238:
#line 1577 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 239:
#line 1578 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 240:
#line 1587 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 241:
#line 1588 "gram.y"
    { yyval.str = NULL; }
    break;

  case 242:
#line 1589 "gram.y"
    { yyval.str = NULL; }
    break;

  case 243:
#line 1595 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].defelt); }
    break;

  case 244:
#line 1596 "gram.y"
    { yyval.list = NIL; }
    break;

  case 245:
#line 1602 "gram.y"
    {
					yyval.defelt = makeDefElem("binary", (Node *)makeInteger(TRUE));
				}
    break;

  case 246:
#line 1606 "gram.y"
    {
					yyval.defelt = makeDefElem("oids", (Node *)makeInteger(TRUE));
				}
    break;

  case 247:
#line 1610 "gram.y"
    {
					yyval.defelt = makeDefElem("delimiter", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 248:
#line 1614 "gram.y"
    {
					yyval.defelt = makeDefElem("null", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 249:
#line 1618 "gram.y"
    {
					yyval.defelt = makeDefElem("csv", (Node *)makeInteger(TRUE));
				}
    break;

  case 250:
#line 1622 "gram.y"
    {
					yyval.defelt = makeDefElem("header", (Node *)makeInteger(TRUE));
				}
    break;

  case 251:
#line 1626 "gram.y"
    {
					yyval.defelt = makeDefElem("quote", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 252:
#line 1630 "gram.y"
    {
					yyval.defelt = makeDefElem("escape", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 253:
#line 1634 "gram.y"
    {
					yyval.defelt = makeDefElem("force_quote", (Node *)yyvsp[0].list);
				}
    break;

  case 254:
#line 1638 "gram.y"
    {
					yyval.defelt = makeDefElem("force_notnull", (Node *)yyvsp[0].list);
				}
    break;

  case 255:
#line 1647 "gram.y"
    {
					yyval.defelt = makeDefElem("binary", (Node *)makeInteger(TRUE));
				}
    break;

  case 256:
#line 1650 "gram.y"
    { yyval.defelt = NULL; }
    break;

  case 257:
#line 1655 "gram.y"
    {
					yyval.defelt = makeDefElem("oids", (Node *)makeInteger(TRUE));
				}
    break;

  case 258:
#line 1658 "gram.y"
    { yyval.defelt = NULL; }
    break;

  case 259:
#line 1664 "gram.y"
    {
					yyval.defelt = makeDefElem("delimiter", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 260:
#line 1667 "gram.y"
    { yyval.defelt = NULL; }
    break;

  case 261:
#line 1671 "gram.y"
    {}
    break;

  case 262:
#line 1672 "gram.y"
    {}
    break;

  case 263:
#line 1685 "gram.y"
    {
					CreateStmt *n = makeNode(CreateStmt);
					yyvsp[-7].range->istemp = yyvsp[-9].boolean;
					n->relation = yyvsp[-7].range;
					n->tableElts = yyvsp[-5].list;
					n->inhRelations = yyvsp[-3].list;
					n->constraints = NIL;
					n->hasoids = yyvsp[-2].withoids;
					n->oncommit = yyvsp[-1].oncommit;
					n->tablespacename = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 264:
#line 1699 "gram.y"
    {
					/* SQL99 CREATE TABLE OF <UDT> (cols) seems to be satisfied
					 * by our inheritance capabilities. Let's try it...
					 */
					CreateStmt *n = makeNode(CreateStmt);
					yyvsp[-8].range->istemp = yyvsp[-10].boolean;
					n->relation = yyvsp[-8].range;
					n->tableElts = yyvsp[-4].list;
					n->inhRelations = list_make1(yyvsp[-6].range);
					n->constraints = NIL;
					n->hasoids = yyvsp[-2].withoids;
					n->oncommit = yyvsp[-1].oncommit;
					n->tablespacename = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 265:
#line 1723 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 266:
#line 1724 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 267:
#line 1725 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 268:
#line 1726 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 269:
#line 1727 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 270:
#line 1728 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 271:
#line 1729 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 272:
#line 1733 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 273:
#line 1734 "gram.y"
    { yyval.list = NIL; }
    break;

  case 274:
#line 1739 "gram.y"
    {
					yyval.list = list_make1(yyvsp[0].node);
				}
    break;

  case 275:
#line 1743 "gram.y"
    {
					yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node);
				}
    break;

  case 276:
#line 1749 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 277:
#line 1750 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 278:
#line 1751 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 279:
#line 1755 "gram.y"
    {
					ColumnDef *n = makeNode(ColumnDef);
					n->colname = yyvsp[-2].str;
					n->typename = yyvsp[-1].typnam;
					n->constraints = yyvsp[0].list;
					n->is_local = true;
					yyval.node = (Node *)n;
				}
    break;

  case 280:
#line 1766 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].node); }
    break;

  case 281:
#line 1767 "gram.y"
    { yyval.list = NIL; }
    break;

  case 282:
#line 1772 "gram.y"
    {
					switch (nodeTag(yyvsp[0].node))
					{
						case T_Constraint:
							{
								Constraint *n = (Constraint *)yyvsp[0].node;
								n->name = yyvsp[-1].str;
							}
							break;
						case T_FkConstraint:
							{
								FkConstraint *n = (FkConstraint *)yyvsp[0].node;
								n->constr_name = yyvsp[-1].str;
							}
							break;
						default:
							break;
					}
					yyval.node = yyvsp[0].node;
				}
    break;

  case 283:
#line 1792 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 284:
#line 1793 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 285:
#line 1813 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_NOTNULL;
					n->name = NULL;
					n->raw_expr = NULL;
					n->cooked_expr = NULL;
					n->keys = NULL;
					n->indexspace = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 286:
#line 1824 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_NULL;
					n->name = NULL;
					n->raw_expr = NULL;
					n->cooked_expr = NULL;
					n->keys = NULL;
					n->indexspace = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 287:
#line 1835 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_UNIQUE;
					n->name = NULL;
					n->raw_expr = NULL;
					n->cooked_expr = NULL;
					n->keys = NULL;
					n->indexspace = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 288:
#line 1846 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_PRIMARY;
					n->name = NULL;
					n->raw_expr = NULL;
					n->cooked_expr = NULL;
					n->keys = NULL;
					n->indexspace = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 289:
#line 1857 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_CHECK;
					n->name = NULL;
					n->raw_expr = yyvsp[-1].node;
					n->cooked_expr = NULL;
					n->keys = NULL;
					n->indexspace = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 290:
#line 1868 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_DEFAULT;
					n->name = NULL;
					if (exprIsNullConstant(yyvsp[0].node))
					{
						/* DEFAULT NULL should be reported as empty expr */
						n->raw_expr = NULL;
					}
					else
					{
						n->raw_expr = yyvsp[0].node;
					}
					n->cooked_expr = NULL;
					n->keys = NULL;
					n->indexspace = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 291:
#line 1887 "gram.y"
    {
					FkConstraint *n = makeNode(FkConstraint);
					n->constr_name		= NULL;
					n->pktable			= yyvsp[-3].range;
					n->fk_attrs			= NIL;
					n->pk_attrs			= yyvsp[-2].list;
					n->fk_matchtype		= yyvsp[-1].ival;
					n->fk_upd_action	= (char) (yyvsp[0].ival >> 8);
					n->fk_del_action	= (char) (yyvsp[0].ival & 0xFF);
					n->deferrable		= FALSE;
					n->initdeferred		= FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 292:
#line 1915 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_ATTR_DEFERRABLE;
					yyval.node = (Node *)n;
				}
    break;

  case 293:
#line 1921 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_ATTR_NOT_DEFERRABLE;
					yyval.node = (Node *)n;
				}
    break;

  case 294:
#line 1927 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_ATTR_DEFERRED;
					yyval.node = (Node *)n;
				}
    break;

  case 295:
#line 1933 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_ATTR_IMMEDIATE;
					yyval.node = (Node *)n;
				}
    break;

  case 296:
#line 1951 "gram.y"
    {
					InhRelation *n = makeNode(InhRelation);
					n->relation = yyvsp[-1].range;
					n->including_defaults = yyvsp[0].boolean;

					yyval.node = (Node *)n;
				}
    break;

  case 297:
#line 1961 "gram.y"
    { yyval.boolean = true; }
    break;

  case 298:
#line 1962 "gram.y"
    { yyval.boolean = false; }
    break;

  case 299:
#line 1963 "gram.y"
    { yyval.boolean = false; }
    break;

  case 300:
#line 1973 "gram.y"
    {
					switch (nodeTag(yyvsp[0].node))
					{
						case T_Constraint:
							{
								Constraint *n = (Constraint *)yyvsp[0].node;
								n->name = yyvsp[-1].str;
							}
							break;
						case T_FkConstraint:
							{
								FkConstraint *n = (FkConstraint *)yyvsp[0].node;
								n->constr_name = yyvsp[-1].str;
							}
							break;
						default:
							break;
					}
					yyval.node = yyvsp[0].node;
				}
    break;

  case 301:
#line 1993 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 302:
#line 1998 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_CHECK;
					n->name = NULL;
					n->raw_expr = yyvsp[-1].node;
					n->cooked_expr = NULL;
					n->indexspace = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 303:
#line 2008 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_UNIQUE;
					n->name = NULL;
					n->raw_expr = NULL;
					n->cooked_expr = NULL;
					n->keys = yyvsp[-2].list;
					n->indexspace = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 304:
#line 2019 "gram.y"
    {
					Constraint *n = makeNode(Constraint);
					n->contype = CONSTR_PRIMARY;
					n->name = NULL;
					n->raw_expr = NULL;
					n->cooked_expr = NULL;
					n->keys = yyvsp[-2].list;
					n->indexspace = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 305:
#line 2031 "gram.y"
    {
					FkConstraint *n = makeNode(FkConstraint);
					n->constr_name		= NULL;
					n->pktable			= yyvsp[-4].range;
					n->fk_attrs			= yyvsp[-7].list;
					n->pk_attrs			= yyvsp[-3].list;
					n->fk_matchtype		= yyvsp[-2].ival;
					n->fk_upd_action	= (char) (yyvsp[-1].ival >> 8);
					n->fk_del_action	= (char) (yyvsp[-1].ival & 0xFF);
					n->deferrable		= (yyvsp[0].ival & 1) != 0;
					n->initdeferred		= (yyvsp[0].ival & 2) != 0;
					yyval.node = (Node *)n;
				}
    break;

  case 306:
#line 2047 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 307:
#line 2048 "gram.y"
    { yyval.list = NIL; }
    break;

  case 308:
#line 2052 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 309:
#line 2053 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node); }
    break;

  case 310:
#line 2057 "gram.y"
    {
					yyval.node = (Node *) makeString(yyvsp[0].str);
				}
    break;

  case 311:
#line 2063 "gram.y"
    {
				yyval.ival = FKCONSTR_MATCH_FULL;
			}
    break;

  case 312:
#line 2067 "gram.y"
    {
				ereport(ERROR,
						(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						 errmsg("MATCH PARTIAL not yet implemented")));
				yyval.ival = FKCONSTR_MATCH_PARTIAL;
			}
    break;

  case 313:
#line 2074 "gram.y"
    {
				yyval.ival = FKCONSTR_MATCH_UNSPECIFIED;
			}
    break;

  case 314:
#line 2078 "gram.y"
    {
				yyval.ival = FKCONSTR_MATCH_UNSPECIFIED;
			}
    break;

  case 315:
#line 2091 "gram.y"
    { yyval.ival = (yyvsp[0].ival << 8) | (FKCONSTR_ACTION_NOACTION & 0xFF); }
    break;

  case 316:
#line 2093 "gram.y"
    { yyval.ival = (FKCONSTR_ACTION_NOACTION << 8) | (yyvsp[0].ival & 0xFF); }
    break;

  case 317:
#line 2095 "gram.y"
    { yyval.ival = (yyvsp[-1].ival << 8) | (yyvsp[0].ival & 0xFF); }
    break;

  case 318:
#line 2097 "gram.y"
    { yyval.ival = (yyvsp[0].ival << 8) | (yyvsp[-1].ival & 0xFF); }
    break;

  case 319:
#line 2099 "gram.y"
    { yyval.ival = (FKCONSTR_ACTION_NOACTION << 8) | (FKCONSTR_ACTION_NOACTION & 0xFF); }
    break;

  case 320:
#line 2102 "gram.y"
    { yyval.ival = yyvsp[0].ival; }
    break;

  case 321:
#line 2105 "gram.y"
    { yyval.ival = yyvsp[0].ival; }
    break;

  case 322:
#line 2109 "gram.y"
    { yyval.ival = FKCONSTR_ACTION_NOACTION; }
    break;

  case 323:
#line 2110 "gram.y"
    { yyval.ival = FKCONSTR_ACTION_RESTRICT; }
    break;

  case 324:
#line 2111 "gram.y"
    { yyval.ival = FKCONSTR_ACTION_CASCADE; }
    break;

  case 325:
#line 2112 "gram.y"
    { yyval.ival = FKCONSTR_ACTION_SETNULL; }
    break;

  case 326:
#line 2113 "gram.y"
    { yyval.ival = FKCONSTR_ACTION_SETDEFAULT; }
    break;

  case 327:
#line 2116 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 328:
#line 2117 "gram.y"
    { yyval.list = NIL; }
    break;

  case 329:
#line 2121 "gram.y"
    { yyval.withoids = MUST_HAVE_OIDS; }
    break;

  case 330:
#line 2122 "gram.y"
    { yyval.withoids = MUST_NOT_HAVE_OIDS; }
    break;

  case 331:
#line 2123 "gram.y"
    { yyval.withoids = DEFAULT_OIDS; }
    break;

  case 332:
#line 2126 "gram.y"
    { yyval.oncommit = ONCOMMIT_DROP; }
    break;

  case 333:
#line 2127 "gram.y"
    { yyval.oncommit = ONCOMMIT_DELETE_ROWS; }
    break;

  case 334:
#line 2128 "gram.y"
    { yyval.oncommit = ONCOMMIT_PRESERVE_ROWS; }
    break;

  case 335:
#line 2129 "gram.y"
    { yyval.oncommit = ONCOMMIT_NOOP; }
    break;

  case 336:
#line 2132 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 337:
#line 2133 "gram.y"
    { yyval.str = NULL; }
    break;

  case 338:
#line 2136 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 339:
#line 2137 "gram.y"
    { yyval.str = NULL; }
    break;

  case 340:
#line 2148 "gram.y"
    {
					/*
					 * When the SelectStmt is a set-operation tree, we must
					 * stuff the INTO information into the leftmost component
					 * Select, because that's where analyze.c will expect
					 * to find it.	Similarly, the output column names must
					 * be attached to that Select's target list.
					 */
					SelectStmt *n = findLeftmostSelect((SelectStmt *) yyvsp[0].node);
					if (n->into != NULL)
						ereport(ERROR,
								(errcode(ERRCODE_SYNTAX_ERROR),
								 errmsg("CREATE TABLE AS may not specify INTO")));
					yyvsp[-3].range->istemp = yyvsp[-5].boolean;
					n->into = yyvsp[-3].range;
					n->intoColNames = yyvsp[-2].list;
					n->intoHasOids = yyvsp[-1].withoids;
					yyval.node = yyvsp[0].node;
				}
    break;

  case 341:
#line 2176 "gram.y"
    { yyval.withoids = MUST_HAVE_OIDS; }
    break;

  case 342:
#line 2177 "gram.y"
    { yyval.withoids = MUST_NOT_HAVE_OIDS; }
    break;

  case 343:
#line 2178 "gram.y"
    { yyval.withoids = DEFAULT_OIDS; }
    break;

  case 344:
#line 2182 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 345:
#line 2183 "gram.y"
    { yyval.list = NIL; }
    break;

  case 346:
#line 2187 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 347:
#line 2188 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node); }
    break;

  case 348:
#line 2193 "gram.y"
    {
					ColumnDef *n = makeNode(ColumnDef);
					n->colname = yyvsp[0].str;
					n->typename = NULL;
					n->inhcount = 0;
					n->is_local = true;
					n->is_not_null = false;
					n->raw_default = NULL;
					n->cooked_default = NULL;
					n->constraints = NIL;
					n->support = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 349:
#line 2219 "gram.y"
    {
					CreateSeqStmt *n = makeNode(CreateSeqStmt);
					yyvsp[-1].range->istemp = yyvsp[-3].boolean;
					n->sequence = yyvsp[-1].range;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 350:
#line 2230 "gram.y"
    {
					AlterSeqStmt *n = makeNode(AlterSeqStmt);
					n->sequence = yyvsp[-1].range;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 351:
#line 2238 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].defelt); }
    break;

  case 352:
#line 2239 "gram.y"
    { yyval.list = NIL; }
    break;

  case 353:
#line 2243 "gram.y"
    {
					yyval.defelt = makeDefElem("cache", (Node *)yyvsp[0].value);
				}
    break;

  case 354:
#line 2247 "gram.y"
    {
					yyval.defelt = makeDefElem("cycle", (Node *)makeInteger(TRUE));
				}
    break;

  case 355:
#line 2251 "gram.y"
    {
					yyval.defelt = makeDefElem("cycle", (Node *)makeInteger(FALSE));
				}
    break;

  case 356:
#line 2255 "gram.y"
    {
					yyval.defelt = makeDefElem("increment", (Node *)yyvsp[0].value);
				}
    break;

  case 357:
#line 2259 "gram.y"
    {
					yyval.defelt = makeDefElem("maxvalue", (Node *)yyvsp[0].value);
				}
    break;

  case 358:
#line 2263 "gram.y"
    {
					yyval.defelt = makeDefElem("minvalue", (Node *)yyvsp[0].value);
				}
    break;

  case 359:
#line 2267 "gram.y"
    {
					yyval.defelt = makeDefElem("maxvalue", NULL);
				}
    break;

  case 360:
#line 2271 "gram.y"
    {
					yyval.defelt = makeDefElem("minvalue", NULL);
				}
    break;

  case 361:
#line 2275 "gram.y"
    {
					yyval.defelt = makeDefElem("start", (Node *)yyvsp[0].value);
				}
    break;

  case 362:
#line 2279 "gram.y"
    {
					yyval.defelt = makeDefElem("restart", (Node *)yyvsp[0].value);
				}
    break;

  case 363:
#line 2284 "gram.y"
    {}
    break;

  case 364:
#line 2285 "gram.y"
    {}
    break;

  case 365:
#line 2289 "gram.y"
    { yyval.value = yyvsp[0].value; }
    break;

  case 366:
#line 2290 "gram.y"
    { yyval.value = yyvsp[0].value; }
    break;

  case 367:
#line 2293 "gram.y"
    { yyval.value = makeFloat(yyvsp[0].str); }
    break;

  case 368:
#line 2295 "gram.y"
    {
					yyval.value = makeFloat(yyvsp[0].str);
					doNegateFloat(yyval.value);
				}
    break;

  case 369:
#line 2301 "gram.y"
    { yyval.value = makeInteger(yyvsp[0].ival); }
    break;

  case 370:
#line 2314 "gram.y"
    {
				CreatePLangStmt *n = makeNode(CreatePLangStmt);
				n->plname = yyvsp[0].str;
				/* parameters are all to be supplied by system */
				n->plhandler = NIL;
				n->plvalidator = NIL;
				n->pltrusted = false;
				yyval.node = (Node *)n;
			}
    break;

  case 371:
#line 2325 "gram.y"
    {
				CreatePLangStmt *n = makeNode(CreatePLangStmt);
				n->plname = yyvsp[-4].str;
				n->plhandler = yyvsp[-2].list;
				n->plvalidator = yyvsp[-1].list;
				n->pltrusted = yyvsp[-7].boolean;
				/* LANCOMPILER is now ignored entirely */
				yyval.node = (Node *)n;
			}
    break;

  case 372:
#line 2337 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 373:
#line 2338 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 374:
#line 2346 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 375:
#line 2347 "gram.y"
    { yyval.list = lcons(makeString(yyvsp[-1].str), yyvsp[0].list); }
    break;

  case 376:
#line 2351 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 377:
#line 2352 "gram.y"
    { yyval.list = NIL; }
    break;

  case 378:
#line 2356 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 379:
#line 2357 "gram.y"
    { yyval.str = NULL; }
    break;

  case 380:
#line 2362 "gram.y"
    {
					DropPLangStmt *n = makeNode(DropPLangStmt);
					n->plname = yyvsp[-1].str;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 381:
#line 2371 "gram.y"
    {}
    break;

  case 382:
#line 2372 "gram.y"
    {}
    break;

  case 383:
#line 2383 "gram.y"
    {
					CreateTableSpaceStmt *n = makeNode(CreateTableSpaceStmt);
					n->tablespacename = yyvsp[-3].str;
					n->owner = yyvsp[-2].str;
					n->location = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 384:
#line 2392 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 385:
#line 2393 "gram.y"
    { yyval.str = NULL; }
    break;

  case 386:
#line 2407 "gram.y"
    {
					DropTableSpaceStmt *n = makeNode(DropTableSpaceStmt);
					n->tablespacename = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 387:
#line 2426 "gram.y"
    {
					CreateTrigStmt *n = makeNode(CreateTrigStmt);
					n->trigname = yyvsp[-11].str;
					n->relation = yyvsp[-7].range;
					n->funcname = yyvsp[-3].list;
					n->args = yyvsp[-1].list;
					n->before = yyvsp[-10].boolean;
					n->row = yyvsp[-6].boolean;
					memcpy(n->actions, yyvsp[-9].str, 4);
					n->isconstraint  = FALSE;
					n->deferrable	 = FALSE;
					n->initdeferred  = FALSE;
					n->constrrel = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 388:
#line 2446 "gram.y"
    {
					CreateTrigStmt *n = makeNode(CreateTrigStmt);
					n->trigname = yyvsp[-15].str;
					n->relation = yyvsp[-11].range;
					n->funcname = yyvsp[-3].list;
					n->args = yyvsp[-1].list;
					n->before = FALSE;
					n->row = TRUE;
					memcpy(n->actions, yyvsp[-13].str, 4);
					n->isconstraint  = TRUE;
					n->deferrable = (yyvsp[-9].ival & 1) != 0;
					n->initdeferred = (yyvsp[-9].ival & 2) != 0;

					n->constrrel = yyvsp[-10].range;
					yyval.node = (Node *)n;
				}
    break;

  case 389:
#line 2465 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 390:
#line 2466 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 391:
#line 2471 "gram.y"
    {
					char *e = palloc(4);
					e[0] = yyvsp[0].chr; e[1] = '\0';
					yyval.str = e;
				}
    break;

  case 392:
#line 2477 "gram.y"
    {
					char *e = palloc(4);
					e[0] = yyvsp[-2].chr; e[1] = yyvsp[0].chr; e[2] = '\0';
					yyval.str = e;
				}
    break;

  case 393:
#line 2483 "gram.y"
    {
					char *e = palloc(4);
					e[0] = yyvsp[-4].chr; e[1] = yyvsp[-2].chr; e[2] = yyvsp[0].chr; e[3] = '\0';
					yyval.str = e;
				}
    break;

  case 394:
#line 2491 "gram.y"
    { yyval.chr = 'i'; }
    break;

  case 395:
#line 2492 "gram.y"
    { yyval.chr = 'd'; }
    break;

  case 396:
#line 2493 "gram.y"
    { yyval.chr = 'u'; }
    break;

  case 397:
#line 2498 "gram.y"
    {
					yyval.boolean = yyvsp[0].boolean;
				}
    break;

  case 398:
#line 2502 "gram.y"
    {
					/*
					 * If ROW/STATEMENT not specified, default to
					 * STATEMENT, per SQL
					 */
					yyval.boolean = FALSE;
				}
    break;

  case 399:
#line 2512 "gram.y"
    {}
    break;

  case 400:
#line 2513 "gram.y"
    {}
    break;

  case 401:
#line 2517 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 402:
#line 2518 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 403:
#line 2522 "gram.y"
    { yyval.list = list_make1(yyvsp[0].value); }
    break;

  case 404:
#line 2523 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].value); }
    break;

  case 405:
#line 2524 "gram.y"
    { yyval.list = NIL; }
    break;

  case 406:
#line 2529 "gram.y"
    {
					char buf[64];
					snprintf(buf, sizeof(buf), "%d", yyvsp[0].ival);
					yyval.value = makeString(pstrdup(buf));
				}
    break;

  case 407:
#line 2534 "gram.y"
    { yyval.value = makeString(yyvsp[0].str); }
    break;

  case 408:
#line 2535 "gram.y"
    { yyval.value = makeString(yyvsp[0].str); }
    break;

  case 409:
#line 2536 "gram.y"
    { yyval.value = makeString(yyvsp[0].str); }
    break;

  case 410:
#line 2537 "gram.y"
    { yyval.value = makeString(yyvsp[0].str); }
    break;

  case 411:
#line 2538 "gram.y"
    { yyval.value = makeString(yyvsp[0].str); }
    break;

  case 412:
#line 2542 "gram.y"
    { yyval.range = yyvsp[0].range; }
    break;

  case 413:
#line 2543 "gram.y"
    { yyval.range = NULL; }
    break;

  case 414:
#line 2548 "gram.y"
    { yyval.ival = yyvsp[0].ival; }
    break;

  case 415:
#line 2550 "gram.y"
    {
					if (yyvsp[-1].ival == 0 && yyvsp[0].ival != 0)
						ereport(ERROR,
								(errcode(ERRCODE_SYNTAX_ERROR),
								 errmsg("constraint declared INITIALLY DEFERRED must be DEFERRABLE")));
					yyval.ival = yyvsp[-1].ival | yyvsp[0].ival;
				}
    break;

  case 416:
#line 2558 "gram.y"
    {
					if (yyvsp[0].ival != 0)
						yyval.ival = 3;
					else
						yyval.ival = 0;
				}
    break;

  case 417:
#line 2565 "gram.y"
    {
					if (yyvsp[0].ival == 0 && yyvsp[-1].ival != 0)
						ereport(ERROR,
								(errcode(ERRCODE_SYNTAX_ERROR),
								 errmsg("constraint declared INITIALLY DEFERRED must be DEFERRABLE")));
					yyval.ival = yyvsp[-1].ival | yyvsp[0].ival;
				}
    break;

  case 418:
#line 2573 "gram.y"
    { yyval.ival = 0; }
    break;

  case 419:
#line 2577 "gram.y"
    { yyval.ival = 0; }
    break;

  case 420:
#line 2578 "gram.y"
    { yyval.ival = 1; }
    break;

  case 421:
#line 2582 "gram.y"
    { yyval.ival = 0; }
    break;

  case 422:
#line 2583 "gram.y"
    { yyval.ival = 2; }
    break;

  case 423:
#line 2589 "gram.y"
    {
					DropPropertyStmt *n = makeNode(DropPropertyStmt);
					n->relation = yyvsp[-1].range;
					n->property = yyvsp[-3].str;
					n->behavior = yyvsp[0].dbehavior;
					n->removeType = OBJECT_TRIGGER;
					yyval.node = (Node *) n;
				}
    break;

  case 424:
#line 2611 "gram.y"
    {
					CreateTrigStmt *n = makeNode(CreateTrigStmt);
					n->trigname = yyvsp[-5].str;
					n->args = list_make1(yyvsp[-2].node);
					n->isconstraint  = TRUE;
					n->deferrable = (yyvsp[0].ival & 1) != 0;
					n->initdeferred = (yyvsp[0].ival & 2) != 0;

					ereport(ERROR,
							(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
							 errmsg("CREATE ASSERTION is not yet implemented")));

					yyval.node = (Node *)n;
				}
    break;

  case 425:
#line 2629 "gram.y"
    {
					DropPropertyStmt *n = makeNode(DropPropertyStmt);
					n->relation = NULL;
					n->property = yyvsp[-1].str;
					n->behavior = yyvsp[0].dbehavior;
					n->removeType = OBJECT_TRIGGER; /* XXX */
					ereport(ERROR,
							(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
							 errmsg("DROP ASSERTION is not yet implemented")));
					yyval.node = (Node *) n;
				}
    break;

  case 426:
#line 2652 "gram.y"
    {
					DefineStmt *n = makeNode(DefineStmt);
					n->kind = OBJECT_AGGREGATE;
					n->defnames = yyvsp[-1].list;
					n->definition = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 427:
#line 2660 "gram.y"
    {
					DefineStmt *n = makeNode(DefineStmt);
					n->kind = OBJECT_OPERATOR;
					n->defnames = yyvsp[-1].list;
					n->definition = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 428:
#line 2668 "gram.y"
    {
					DefineStmt *n = makeNode(DefineStmt);
					n->kind = OBJECT_TYPE;
					n->defnames = yyvsp[-1].list;
					n->definition = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 429:
#line 2676 "gram.y"
    {
					CompositeTypeStmt *n = makeNode(CompositeTypeStmt);
					RangeVar *r = makeNode(RangeVar);

					/* can't use qualified_name, sigh */
					switch (list_length(yyvsp[-4].list))
					{
						case 1:
							r->catalogname = NULL;
							r->schemaname = NULL;
							r->relname = strVal(linitial(yyvsp[-4].list));
							break;
						case 2:
							r->catalogname = NULL;
							r->schemaname = strVal(linitial(yyvsp[-4].list));
							r->relname = strVal(lsecond(yyvsp[-4].list));
							break;
						case 3:
							r->catalogname = strVal(linitial(yyvsp[-4].list));
							r->schemaname = strVal(lsecond(yyvsp[-4].list));
							r->relname = strVal(lthird(yyvsp[-4].list));
							break;
						default:
							ereport(ERROR,
									(errcode(ERRCODE_SYNTAX_ERROR),
									 errmsg("improper qualified name (too many dotted names): %s",
											NameListToString(yyvsp[-4].list))));
							break;
					}
					n->typevar = r;
					n->coldeflist = yyvsp[-1].list;
					yyval.node = (Node *)n;
				}
    break;

  case 430:
#line 2711 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 431:
#line 2714 "gram.y"
    { yyval.list = list_make1(yyvsp[0].defelt); }
    break;

  case 432:
#line 2715 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].defelt); }
    break;

  case 433:
#line 2719 "gram.y"
    {
					yyval.defelt = makeDefElem(yyvsp[-2].str, (Node *)yyvsp[0].node);
				}
    break;

  case 434:
#line 2723 "gram.y"
    {
					yyval.defelt = makeDefElem(yyvsp[0].str, NULL);
				}
    break;

  case 435:
#line 2729 "gram.y"
    { yyval.node = (Node *)yyvsp[0].typnam; }
    break;

  case 436:
#line 2730 "gram.y"
    { yyval.node = (Node *)yyvsp[0].list; }
    break;

  case 437:
#line 2731 "gram.y"
    { yyval.node = (Node *)yyvsp[0].value; }
    break;

  case 438:
#line 2732 "gram.y"
    { yyval.node = (Node *)makeString(yyvsp[0].str); }
    break;

  case 439:
#line 2747 "gram.y"
    {
					CreateOpClassStmt *n = makeNode(CreateOpClassStmt);
					n->opclassname = yyvsp[-8].list;
					n->isDefault = yyvsp[-7].boolean;
					n->datatype = yyvsp[-4].typnam;
					n->amname = yyvsp[-2].str;
					n->items = yyvsp[0].list;
					yyval.node = (Node *) n;
				}
    break;

  case 440:
#line 2759 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 441:
#line 2760 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node); }
    break;

  case 442:
#line 2765 "gram.y"
    {
					CreateOpClassItem *n = makeNode(CreateOpClassItem);
					n->itemtype = OPCLASS_ITEM_OPERATOR;
					n->name = yyvsp[-1].list;
					n->args = NIL;
					n->number = yyvsp[-2].ival;
					n->recheck = yyvsp[0].boolean;
					yyval.node = (Node *) n;
				}
    break;

  case 443:
#line 2775 "gram.y"
    {
					CreateOpClassItem *n = makeNode(CreateOpClassItem);
					n->itemtype = OPCLASS_ITEM_OPERATOR;
					n->name = yyvsp[-4].list;
					n->args = yyvsp[-2].list;
					n->number = yyvsp[-5].ival;
					n->recheck = yyvsp[0].boolean;
					yyval.node = (Node *) n;
				}
    break;

  case 444:
#line 2785 "gram.y"
    {
					CreateOpClassItem *n = makeNode(CreateOpClassItem);
					n->itemtype = OPCLASS_ITEM_FUNCTION;
					n->name = yyvsp[-1].list;
					n->args = extractArgTypes(yyvsp[0].list);
					n->number = yyvsp[-2].ival;
					yyval.node = (Node *) n;
				}
    break;

  case 445:
#line 2794 "gram.y"
    {
					CreateOpClassItem *n = makeNode(CreateOpClassItem);
					n->itemtype = OPCLASS_ITEM_STORAGETYPE;
					n->storedtype = yyvsp[0].typnam;
					yyval.node = (Node *) n;
				}
    break;

  case 446:
#line 2802 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 447:
#line 2803 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 448:
#line 2806 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 449:
#line 2807 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 450:
#line 2813 "gram.y"
    {
					RemoveOpClassStmt *n = makeNode(RemoveOpClassStmt);
					n->opclassname = yyvsp[-3].list;
					n->amname = yyvsp[-1].str;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *) n;
				}
    break;

  case 451:
#line 2832 "gram.y"
    {
					DropStmt *n = makeNode(DropStmt);
					n->removeType = yyvsp[-2].objtype;
					n->objects = yyvsp[-1].list;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 452:
#line 2841 "gram.y"
    { yyval.objtype = OBJECT_TABLE; }
    break;

  case 453:
#line 2842 "gram.y"
    { yyval.objtype = OBJECT_SEQUENCE; }
    break;

  case 454:
#line 2843 "gram.y"
    { yyval.objtype = OBJECT_VIEW; }
    break;

  case 455:
#line 2844 "gram.y"
    { yyval.objtype = OBJECT_INDEX; }
    break;

  case 456:
#line 2845 "gram.y"
    { yyval.objtype = OBJECT_TYPE; }
    break;

  case 457:
#line 2846 "gram.y"
    { yyval.objtype = OBJECT_DOMAIN; }
    break;

  case 458:
#line 2847 "gram.y"
    { yyval.objtype = OBJECT_CONVERSION; }
    break;

  case 459:
#line 2848 "gram.y"
    { yyval.objtype = OBJECT_SCHEMA; }
    break;

  case 460:
#line 2852 "gram.y"
    { yyval.list = list_make1(yyvsp[0].list); }
    break;

  case 461:
#line 2853 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].list); }
    break;

  case 462:
#line 2856 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 463:
#line 2857 "gram.y"
    { yyval.list = lcons(makeString(yyvsp[-1].str), yyvsp[0].list); }
    break;

  case 464:
#line 2861 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 465:
#line 2863 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, makeString(yyvsp[0].str)); }
    break;

  case 466:
#line 2876 "gram.y"
    {
					TruncateStmt *n = makeNode(TruncateStmt);
					n->relations = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 467:
#line 2902 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = yyvsp[-3].objtype;
					n->objname = yyvsp[-2].list;
					n->objargs = NIL;
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 468:
#line 2912 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_AGGREGATE;
					n->objname = yyvsp[-5].list;
					n->objargs = list_make1(yyvsp[-3].typnam);
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 469:
#line 2921 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_FUNCTION;
					n->objname = yyvsp[-3].list;
					n->objargs = extractArgTypes(yyvsp[-2].list);
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 470:
#line 2931 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_OPERATOR;
					n->objname = yyvsp[-5].list;
					n->objargs = yyvsp[-3].list;
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 471:
#line 2940 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_CONSTRAINT;
					n->objname = lappend(yyvsp[-2].list, makeString(yyvsp[-4].str));
					n->objargs = NIL;
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 472:
#line 2949 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_RULE;
					n->objname = lappend(yyvsp[-2].list, makeString(yyvsp[-4].str));
					n->objargs = NIL;
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 473:
#line 2958 "gram.y"
    {
					/* Obsolete syntax supported for awhile for compatibility */
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_RULE;
					n->objname = list_make1(makeString(yyvsp[-2].str));
					n->objargs = NIL;
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 474:
#line 2968 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_TRIGGER;
					n->objname = lappend(yyvsp[-2].list, makeString(yyvsp[-4].str));
					n->objargs = NIL;
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 475:
#line 2977 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_OPCLASS;
					n->objname = yyvsp[-4].list;
					n->objargs = list_make1(makeString(yyvsp[-2].str));
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 476:
#line 2986 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_LARGEOBJECT;
					n->objname = list_make1(yyvsp[-2].value);
					n->objargs = NIL;
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 477:
#line 2995 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_CAST;
					n->objname = list_make1(yyvsp[-5].typnam);
					n->objargs = list_make1(yyvsp[-3].typnam);
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 478:
#line 3004 "gram.y"
    {
					CommentStmt *n = makeNode(CommentStmt);
					n->objtype = OBJECT_LANGUAGE;
					n->objname = yyvsp[-2].list;
					n->objargs = NIL;
					n->comment = yyvsp[0].str;
					yyval.node = (Node *) n;
				}
    break;

  case 479:
#line 3015 "gram.y"
    { yyval.objtype = OBJECT_COLUMN; }
    break;

  case 480:
#line 3016 "gram.y"
    { yyval.objtype = OBJECT_DATABASE; }
    break;

  case 481:
#line 3017 "gram.y"
    { yyval.objtype = OBJECT_SCHEMA; }
    break;

  case 482:
#line 3018 "gram.y"
    { yyval.objtype = OBJECT_INDEX; }
    break;

  case 483:
#line 3019 "gram.y"
    { yyval.objtype = OBJECT_SEQUENCE; }
    break;

  case 484:
#line 3020 "gram.y"
    { yyval.objtype = OBJECT_TABLE; }
    break;

  case 485:
#line 3021 "gram.y"
    { yyval.objtype = OBJECT_TYPE; }
    break;

  case 486:
#line 3022 "gram.y"
    { yyval.objtype = OBJECT_TYPE; }
    break;

  case 487:
#line 3023 "gram.y"
    { yyval.objtype = OBJECT_VIEW; }
    break;

  case 488:
#line 3024 "gram.y"
    { yyval.objtype = OBJECT_CONVERSION; }
    break;

  case 489:
#line 3028 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 490:
#line 3029 "gram.y"
    { yyval.str = NULL; }
    break;

  case 491:
#line 3040 "gram.y"
    {
					FetchStmt *n = (FetchStmt *) yyvsp[-2].node;
					n->portalname = yyvsp[0].str;
					n->ismove = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 492:
#line 3047 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = 1;
					n->portalname = yyvsp[0].str;
					n->ismove = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 493:
#line 3056 "gram.y"
    {
					FetchStmt *n = (FetchStmt *) yyvsp[-2].node;
					n->portalname = yyvsp[0].str;
					n->ismove = TRUE;
					yyval.node = (Node *)n;
				}
    break;

  case 494:
#line 3063 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = 1;
					n->portalname = yyvsp[0].str;
					n->ismove = TRUE;
					yyval.node = (Node *)n;
				}
    break;

  case 495:
#line 3075 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = 1;
					yyval.node = (Node *)n;
				}
    break;

  case 496:
#line 3082 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = 1;
					yyval.node = (Node *)n;
				}
    break;

  case 497:
#line 3089 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_BACKWARD;
					n->howMany = 1;
					yyval.node = (Node *)n;
				}
    break;

  case 498:
#line 3096 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_ABSOLUTE;
					n->howMany = 1;
					yyval.node = (Node *)n;
				}
    break;

  case 499:
#line 3103 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_ABSOLUTE;
					n->howMany = -1;
					yyval.node = (Node *)n;
				}
    break;

  case 500:
#line 3110 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_ABSOLUTE;
					n->howMany = yyvsp[0].ival;
					yyval.node = (Node *)n;
				}
    break;

  case 501:
#line 3117 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_RELATIVE;
					n->howMany = yyvsp[0].ival;
					yyval.node = (Node *)n;
				}
    break;

  case 502:
#line 3124 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = yyvsp[0].ival;
					yyval.node = (Node *)n;
				}
    break;

  case 503:
#line 3131 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = FETCH_ALL;
					yyval.node = (Node *)n;
				}
    break;

  case 504:
#line 3138 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = 1;
					yyval.node = (Node *)n;
				}
    break;

  case 505:
#line 3145 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = yyvsp[0].ival;
					yyval.node = (Node *)n;
				}
    break;

  case 506:
#line 3152 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_FORWARD;
					n->howMany = FETCH_ALL;
					yyval.node = (Node *)n;
				}
    break;

  case 507:
#line 3159 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_BACKWARD;
					n->howMany = 1;
					yyval.node = (Node *)n;
				}
    break;

  case 508:
#line 3166 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_BACKWARD;
					n->howMany = yyvsp[0].ival;
					yyval.node = (Node *)n;
				}
    break;

  case 509:
#line 3173 "gram.y"
    {
					FetchStmt *n = makeNode(FetchStmt);
					n->direction = FETCH_BACKWARD;
					n->howMany = FETCH_ALL;
					yyval.node = (Node *)n;
				}
    break;

  case 510:
#line 3181 "gram.y"
    {}
    break;

  case 511:
#line 3182 "gram.y"
    {}
    break;

  case 512:
#line 3194 "gram.y"
    {
					GrantStmt *n = makeNode(GrantStmt);
					n->is_grant = true;
					n->privileges = yyvsp[-5].list;
					n->objtype = (yyvsp[-3].privtarget)->objtype;
					n->objects = (yyvsp[-3].privtarget)->objs;
					n->grantees = yyvsp[-1].list;
					n->grant_option = yyvsp[0].boolean;
					yyval.node = (Node*)n;
				}
    break;

  case 513:
#line 3209 "gram.y"
    {
					GrantStmt *n = makeNode(GrantStmt);
					n->is_grant = false;
					n->grant_option = false;
					n->privileges = yyvsp[-5].list;
					n->objtype = (yyvsp[-3].privtarget)->objtype;
					n->objects = (yyvsp[-3].privtarget)->objs;
					n->grantees = yyvsp[-1].list;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 514:
#line 3222 "gram.y"
    {
					GrantStmt *n = makeNode(GrantStmt);
					n->is_grant = false;
					n->grant_option = true;
					n->privileges = yyvsp[-5].list;
					n->objtype = (yyvsp[-3].privtarget)->objtype;
					n->objects = (yyvsp[-3].privtarget)->objs;
					n->grantees = yyvsp[-1].list;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 515:
#line 3247 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 516:
#line 3249 "gram.y"
    { yyval.list = NIL; }
    break;

  case 517:
#line 3251 "gram.y"
    { yyval.list = NIL; }
    break;

  case 518:
#line 3255 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 519:
#line 3257 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, makeString(yyvsp[0].str)); }
    break;

  case 520:
#line 3260 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 521:
#line 3261 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 522:
#line 3262 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 523:
#line 3263 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 524:
#line 3272 "gram.y"
    {
					PrivTarget *n = makeNode(PrivTarget);
					n->objtype = ACL_OBJECT_RELATION;
					n->objs = yyvsp[0].list;
					yyval.privtarget = n;
				}
    break;

  case 525:
#line 3279 "gram.y"
    {
					PrivTarget *n = makeNode(PrivTarget);
					n->objtype = ACL_OBJECT_RELATION;
					n->objs = yyvsp[0].list;
					yyval.privtarget = n;
				}
    break;

  case 526:
#line 3286 "gram.y"
    {
					PrivTarget *n = makeNode(PrivTarget);
					n->objtype = ACL_OBJECT_FUNCTION;
					n->objs = yyvsp[0].list;
					yyval.privtarget = n;
				}
    break;

  case 527:
#line 3293 "gram.y"
    {
					PrivTarget *n = makeNode(PrivTarget);
					n->objtype = ACL_OBJECT_DATABASE;
					n->objs = yyvsp[0].list;
					yyval.privtarget = n;
				}
    break;

  case 528:
#line 3300 "gram.y"
    {
					PrivTarget *n = makeNode(PrivTarget);
					n->objtype = ACL_OBJECT_LANGUAGE;
					n->objs = yyvsp[0].list;
					yyval.privtarget = n;
				}
    break;

  case 529:
#line 3307 "gram.y"
    {
					PrivTarget *n = makeNode(PrivTarget);
					n->objtype = ACL_OBJECT_NAMESPACE;
					n->objs = yyvsp[0].list;
					yyval.privtarget = n;
				}
    break;

  case 530:
#line 3314 "gram.y"
    {
					PrivTarget *n = makeNode(PrivTarget);
					n->objtype = ACL_OBJECT_TABLESPACE;
					n->objs = yyvsp[0].list;
					yyval.privtarget = n;
				}
    break;

  case 531:
#line 3324 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 532:
#line 3325 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node); }
    break;

  case 533:
#line 3329 "gram.y"
    {
					PrivGrantee *n = makeNode(PrivGrantee);
					/* This hack lets us avoid reserving PUBLIC as a keyword*/
					if (strcmp(yyvsp[0].str, "public") == 0)
						n->rolname = NULL;
					else
						n->rolname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 534:
#line 3339 "gram.y"
    {
					PrivGrantee *n = makeNode(PrivGrantee);
					/* Treat GROUP PUBLIC as a synonym for PUBLIC */
					if (strcmp(yyvsp[0].str, "public") == 0)
						n->rolname = NULL;
					else
						n->rolname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 535:
#line 3352 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 536:
#line 3353 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 537:
#line 3357 "gram.y"
    { yyval.list = list_make1(yyvsp[0].funwithargs); }
    break;

  case 538:
#line 3359 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].funwithargs); }
    break;

  case 539:
#line 3364 "gram.y"
    {
					FuncWithArgs *n = makeNode(FuncWithArgs);
					n->funcname = yyvsp[-1].list;
					n->funcargs = extractArgTypes(yyvsp[0].list);
					yyval.funwithargs = n;
				}
    break;

  case 540:
#line 3380 "gram.y"
    {
					GrantRoleStmt *n = makeNode(GrantRoleStmt);
					n->is_grant = true;
					n->granted_roles = yyvsp[-4].list;
					n->grantee_roles = yyvsp[-2].list;
					n->admin_opt = yyvsp[-1].boolean;
					n->grantor = yyvsp[0].str;
					yyval.node = (Node*)n;
				}
    break;

  case 541:
#line 3393 "gram.y"
    {
					GrantRoleStmt *n = makeNode(GrantRoleStmt);
					n->is_grant = false;
					n->admin_opt = false;
					n->granted_roles = yyvsp[-4].list;
					n->grantee_roles = yyvsp[-2].list;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node*)n;
				}
    break;

  case 542:
#line 3403 "gram.y"
    {
					GrantRoleStmt *n = makeNode(GrantRoleStmt);
					n->is_grant = false;
					n->admin_opt = true;
					n->granted_roles = yyvsp[-4].list;
					n->grantee_roles = yyvsp[-2].list;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node*)n;
				}
    break;

  case 543:
#line 3414 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 544:
#line 3415 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 545:
#line 3418 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 546:
#line 3419 "gram.y"
    { yyval.str = NULL; }
    break;

  case 547:
#line 3436 "gram.y"
    {
					IndexStmt *n = makeNode(IndexStmt);
					n->unique = yyvsp[-10].boolean;
					n->idxname = yyvsp[-8].str;
					n->relation = yyvsp[-6].range;
					n->accessMethod = yyvsp[-5].str;
					n->indexParams = yyvsp[-3].list;
					n->tableSpace = yyvsp[-1].str;
					n->whereClause = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 548:
#line 3450 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 549:
#line 3451 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 550:
#line 3455 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 551:
#line 3456 "gram.y"
    { yyval.str = DEFAULT_INDEX_TYPE; }
    break;

  case 552:
#line 3459 "gram.y"
    { yyval.list = list_make1(yyvsp[0].ielem); }
    break;

  case 553:
#line 3460 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].ielem); }
    break;

  case 554:
#line 3469 "gram.y"
    {
					yyval.ielem = makeNode(IndexElem);
					yyval.ielem->name = yyvsp[-1].str;
					yyval.ielem->expr = NULL;
					yyval.ielem->opclass = yyvsp[0].list;
				}
    break;

  case 555:
#line 3476 "gram.y"
    {
					yyval.ielem = makeNode(IndexElem);
					yyval.ielem->name = NULL;
					yyval.ielem->expr = yyvsp[-1].node;
					yyval.ielem->opclass = yyvsp[0].list;
				}
    break;

  case 556:
#line 3483 "gram.y"
    {
					yyval.ielem = makeNode(IndexElem);
					yyval.ielem->name = NULL;
					yyval.ielem->expr = yyvsp[-2].node;
					yyval.ielem->opclass = yyvsp[0].list;
				}
    break;

  case 557:
#line 3491 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 558:
#line 3492 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 559:
#line 3493 "gram.y"
    { yyval.list = NIL; }
    break;

  case 560:
#line 3510 "gram.y"
    {
					CreateFunctionStmt *n = makeNode(CreateFunctionStmt);
					n->replace = yyvsp[-7].boolean;
					n->funcname = yyvsp[-5].list;
					n->parameters = yyvsp[-4].list;
					n->returnType = yyvsp[-2].typnam;
					n->options = yyvsp[-1].list;
					n->withClause = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 561:
#line 3522 "gram.y"
    {
					CreateFunctionStmt *n = makeNode(CreateFunctionStmt);
					n->replace = yyvsp[-5].boolean;
					n->funcname = yyvsp[-3].list;
					n->parameters = yyvsp[-2].list;
					n->returnType = NULL;
					n->options = yyvsp[-1].list;
					n->withClause = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 562:
#line 3535 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 563:
#line 3536 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 564:
#line 3539 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 565:
#line 3540 "gram.y"
    { yyval.list = NIL; }
    break;

  case 566:
#line 3544 "gram.y"
    { yyval.list = list_make1(yyvsp[0].fun_param); }
    break;

  case 567:
#line 3545 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].fun_param); }
    break;

  case 568:
#line 3560 "gram.y"
    {
					FunctionParameter *n = makeNode(FunctionParameter);
					n->name = yyvsp[-1].str;
					n->argType = yyvsp[0].typnam;
					n->mode = yyvsp[-2].fun_param_mode;
					yyval.fun_param = n;
				}
    break;

  case 569:
#line 3568 "gram.y"
    {
					FunctionParameter *n = makeNode(FunctionParameter);
					n->name = yyvsp[-2].str;
					n->argType = yyvsp[0].typnam;
					n->mode = yyvsp[-1].fun_param_mode;
					yyval.fun_param = n;
				}
    break;

  case 570:
#line 3576 "gram.y"
    {
					FunctionParameter *n = makeNode(FunctionParameter);
					n->name = yyvsp[-1].str;
					n->argType = yyvsp[0].typnam;
					n->mode = FUNC_PARAM_IN;
					yyval.fun_param = n;
				}
    break;

  case 571:
#line 3584 "gram.y"
    {
					FunctionParameter *n = makeNode(FunctionParameter);
					n->name = NULL;
					n->argType = yyvsp[0].typnam;
					n->mode = yyvsp[-1].fun_param_mode;
					yyval.fun_param = n;
				}
    break;

  case 572:
#line 3592 "gram.y"
    {
					FunctionParameter *n = makeNode(FunctionParameter);
					n->name = NULL;
					n->argType = yyvsp[0].typnam;
					n->mode = FUNC_PARAM_IN;
					yyval.fun_param = n;
				}
    break;

  case 573:
#line 3602 "gram.y"
    { yyval.fun_param_mode = FUNC_PARAM_IN; }
    break;

  case 574:
#line 3603 "gram.y"
    { yyval.fun_param_mode = FUNC_PARAM_OUT; }
    break;

  case 575:
#line 3604 "gram.y"
    { yyval.fun_param_mode = FUNC_PARAM_INOUT; }
    break;

  case 576:
#line 3605 "gram.y"
    { yyval.fun_param_mode = FUNC_PARAM_INOUT; }
    break;

  case 578:
#line 3616 "gram.y"
    {
					/* We can catch over-specified results here if we want to,
					 * but for now better to silently swallow typmod, etc.
					 * - thomas 2000-03-22
					 */
					yyval.typnam = yyvsp[0].typnam;
				}
    break;

  case 579:
#line 3629 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 580:
#line 3631 "gram.y"
    {
					yyval.typnam = makeNode(TypeName);
					yyval.typnam->names = lcons(makeString(yyvsp[-3].str), yyvsp[-2].list);
					yyval.typnam->pct_type = true;
					yyval.typnam->typmod = -1;
				}
    break;

  case 581:
#line 3638 "gram.y"
    {
					yyval.typnam = makeNode(TypeName);
					yyval.typnam->names = lcons(makeString(yyvsp[-3].str), yyvsp[-2].list);
					yyval.typnam->pct_type = true;
					yyval.typnam->typmod = -1;
					yyval.typnam->setof = TRUE;
				}
    break;

  case 582:
#line 3650 "gram.y"
    { yyval.list = list_make1(yyvsp[0].defelt); }
    break;

  case 583:
#line 3651 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].defelt); }
    break;

  case 584:
#line 3659 "gram.y"
    {
					yyval.defelt = makeDefElem("strict", (Node *)makeInteger(FALSE));
				}
    break;

  case 585:
#line 3663 "gram.y"
    {
					yyval.defelt = makeDefElem("strict", (Node *)makeInteger(TRUE));
				}
    break;

  case 586:
#line 3667 "gram.y"
    {
					yyval.defelt = makeDefElem("strict", (Node *)makeInteger(TRUE));
				}
    break;

  case 587:
#line 3671 "gram.y"
    {
					yyval.defelt = makeDefElem("volatility", (Node *)makeString("immutable"));
				}
    break;

  case 588:
#line 3675 "gram.y"
    {
					yyval.defelt = makeDefElem("volatility", (Node *)makeString("stable"));
				}
    break;

  case 589:
#line 3679 "gram.y"
    {
					yyval.defelt = makeDefElem("volatility", (Node *)makeString("volatile"));
				}
    break;

  case 590:
#line 3683 "gram.y"
    {
					yyval.defelt = makeDefElem("security", (Node *)makeInteger(TRUE));
				}
    break;

  case 591:
#line 3687 "gram.y"
    {
					yyval.defelt = makeDefElem("security", (Node *)makeInteger(FALSE));
				}
    break;

  case 592:
#line 3691 "gram.y"
    {
					yyval.defelt = makeDefElem("security", (Node *)makeInteger(TRUE));
				}
    break;

  case 593:
#line 3695 "gram.y"
    {
					yyval.defelt = makeDefElem("security", (Node *)makeInteger(FALSE));
				}
    break;

  case 594:
#line 3702 "gram.y"
    {
					yyval.defelt = makeDefElem("as", (Node *)yyvsp[0].list);
				}
    break;

  case 595:
#line 3706 "gram.y"
    {
					yyval.defelt = makeDefElem("language", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 596:
#line 3710 "gram.y"
    {
					yyval.defelt = yyvsp[0].defelt;
				}
    break;

  case 597:
#line 3715 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 598:
#line 3717 "gram.y"
    {
					yyval.list = list_make2(makeString(yyvsp[-2].str), makeString(yyvsp[0].str));
				}
    break;

  case 599:
#line 3723 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 600:
#line 3724 "gram.y"
    { yyval.list = NIL; }
    break;

  case 601:
#line 3737 "gram.y"
    {
					AlterFunctionStmt *n = makeNode(AlterFunctionStmt);
					n->func = yyvsp[-2].funwithargs;
					n->actions = yyvsp[-1].list;
					yyval.node = (Node *) n;
				}
    break;

  case 602:
#line 3747 "gram.y"
    { yyval.list = list_make1(yyvsp[0].defelt); }
    break;

  case 603:
#line 3748 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].defelt); }
    break;

  case 606:
#line 3770 "gram.y"
    {
					RemoveFuncStmt *n = makeNode(RemoveFuncStmt);
					n->funcname = yyvsp[-2].list;
					n->args = extractArgTypes(yyvsp[-1].list);
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 607:
#line 3781 "gram.y"
    {
						RemoveAggrStmt *n = makeNode(RemoveAggrStmt);
						n->aggname = yyvsp[-4].list;
						n->aggtype = yyvsp[-2].typnam;
						n->behavior = yyvsp[0].dbehavior;
						yyval.node = (Node *)n;
				}
    break;

  case 608:
#line 3791 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 609:
#line 3792 "gram.y"
    { yyval.typnam = NULL; }
    break;

  case 610:
#line 3797 "gram.y"
    {
					RemoveOperStmt *n = makeNode(RemoveOperStmt);
					n->opname = yyvsp[-4].list;
					n->args = yyvsp[-2].list;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 611:
#line 3808 "gram.y"
    {
				   ereport(ERROR,
						   (errcode(ERRCODE_SYNTAX_ERROR),
							errmsg("missing argument"),
							errhint("Use NONE to denote the missing argument of a unary operator.")));
				}
    break;

  case 612:
#line 3815 "gram.y"
    { yyval.list = list_make2(yyvsp[-2].typnam, yyvsp[0].typnam); }
    break;

  case 613:
#line 3817 "gram.y"
    { yyval.list = list_make2(NULL, yyvsp[0].typnam); }
    break;

  case 614:
#line 3819 "gram.y"
    { yyval.list = list_make2(yyvsp[-2].typnam, NULL); }
    break;

  case 615:
#line 3824 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 616:
#line 3826 "gram.y"
    { yyval.list = lcons(makeString(yyvsp[-2].str), yyvsp[0].list); }
    break;

  case 617:
#line 3838 "gram.y"
    {
					CreateCastStmt *n = makeNode(CreateCastStmt);
					n->sourcetype = yyvsp[-7].typnam;
					n->targettype = yyvsp[-5].typnam;
					n->func = yyvsp[-1].funwithargs;
					n->context = (CoercionContext) yyvsp[0].ival;
					yyval.node = (Node *)n;
				}
    break;

  case 618:
#line 3848 "gram.y"
    {
					CreateCastStmt *n = makeNode(CreateCastStmt);
					n->sourcetype = yyvsp[-6].typnam;
					n->targettype = yyvsp[-4].typnam;
					n->func = NULL;
					n->context = (CoercionContext) yyvsp[0].ival;
					yyval.node = (Node *)n;
				}
    break;

  case 619:
#line 3858 "gram.y"
    { yyval.ival = COERCION_IMPLICIT; }
    break;

  case 620:
#line 3859 "gram.y"
    { yyval.ival = COERCION_ASSIGNMENT; }
    break;

  case 621:
#line 3860 "gram.y"
    { yyval.ival = COERCION_EXPLICIT; }
    break;

  case 622:
#line 3865 "gram.y"
    {
					DropCastStmt *n = makeNode(DropCastStmt);
					n->sourcetype = yyvsp[-4].typnam;
					n->targettype = yyvsp[-2].typnam;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 623:
#line 3887 "gram.y"
    {
					ReindexStmt *n = makeNode(ReindexStmt);
					n->kind = yyvsp[-2].objtype;
					n->relation = yyvsp[-1].range;
					n->name = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 624:
#line 3895 "gram.y"
    {
					ReindexStmt *n = makeNode(ReindexStmt);
					n->kind = OBJECT_DATABASE;
					n->name = yyvsp[-1].str;
					n->relation = NULL;
					n->do_system = true;
					n->do_user = false;
					yyval.node = (Node *)n;
				}
    break;

  case 625:
#line 3905 "gram.y"
    {
					ReindexStmt *n = makeNode(ReindexStmt);
					n->kind = OBJECT_DATABASE;
					n->name = yyvsp[-1].str;
					n->relation = NULL;
					n->do_system = true;
					n->do_user = true;
					yyval.node = (Node *)n;
				}
    break;

  case 626:
#line 3917 "gram.y"
    { yyval.objtype = OBJECT_INDEX; }
    break;

  case 627:
#line 3918 "gram.y"
    { yyval.objtype = OBJECT_TABLE; }
    break;

  case 628:
#line 3921 "gram.y"
    {  yyval.boolean = TRUE; }
    break;

  case 629:
#line 3922 "gram.y"
    {  yyval.boolean = FALSE; }
    break;

  case 630:
#line 3933 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_AGGREGATE;
					n->object = yyvsp[-6].list;
					n->objarg = list_make1(yyvsp[-4].typnam);
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 631:
#line 3942 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_CONVERSION;
					n->object = yyvsp[-3].list;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 632:
#line 3950 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_DATABASE;
					n->subname = yyvsp[-3].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 633:
#line 3958 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_FUNCTION;
					n->object = yyvsp[-4].list;
					n->objarg = extractArgTypes(yyvsp[-3].list);
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 634:
#line 3967 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_ROLE;
					n->subname = yyvsp[-3].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 635:
#line 3975 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_LANGUAGE;
					n->subname = yyvsp[-3].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 636:
#line 3983 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_OPCLASS;
					n->object = yyvsp[-5].list;
					n->subname = yyvsp[-3].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 637:
#line 3992 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_SCHEMA;
					n->subname = yyvsp[-3].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 638:
#line 4000 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_TABLE;
					n->relation = yyvsp[-3].range;
					n->subname = NULL;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 639:
#line 4009 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_INDEX;
					n->relation = yyvsp[-3].range;
					n->subname = NULL;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 640:
#line 4018 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_COLUMN;
					n->relation = yyvsp[-5].range;
					n->subname = yyvsp[-2].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 641:
#line 4027 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_TRIGGER;
					n->relation = yyvsp[-3].range;
					n->subname = yyvsp[-5].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 642:
#line 4036 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_ROLE;
					n->subname = yyvsp[-3].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 643:
#line 4044 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_ROLE;
					n->subname = yyvsp[-3].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 644:
#line 4052 "gram.y"
    {
					RenameStmt *n = makeNode(RenameStmt);
					n->renameType = OBJECT_TABLESPACE;
					n->subname = yyvsp[-3].str;
					n->newname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 645:
#line 4061 "gram.y"
    { yyval.ival = COLUMN; }
    break;

  case 646:
#line 4062 "gram.y"
    { yyval.ival = 0; }
    break;

  case 647:
#line 4073 "gram.y"
    {
					AlterObjectSchemaStmt *n = makeNode(AlterObjectSchemaStmt);
					n->objectType = OBJECT_AGGREGATE;
					n->object = yyvsp[-6].list;
					n->objarg = list_make1(yyvsp[-4].typnam);
					n->newschema = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 648:
#line 4082 "gram.y"
    {
					AlterObjectSchemaStmt *n = makeNode(AlterObjectSchemaStmt);
					n->objectType = OBJECT_DOMAIN;
					n->object = yyvsp[-3].list;
					n->newschema = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 649:
#line 4090 "gram.y"
    {
					AlterObjectSchemaStmt *n = makeNode(AlterObjectSchemaStmt);
					n->objectType = OBJECT_FUNCTION;
					n->object = yyvsp[-4].list;
					n->objarg = extractArgTypes(yyvsp[-3].list);
					n->newschema = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 650:
#line 4099 "gram.y"
    {
					AlterObjectSchemaStmt *n = makeNode(AlterObjectSchemaStmt);
					n->objectType = OBJECT_SEQUENCE;
					n->relation = yyvsp[-3].range;
					n->newschema = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 651:
#line 4107 "gram.y"
    {
					AlterObjectSchemaStmt *n = makeNode(AlterObjectSchemaStmt);
					n->objectType = OBJECT_TABLE;
					n->relation = yyvsp[-3].range;
					n->newschema = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 652:
#line 4115 "gram.y"
    {
					AlterObjectSchemaStmt *n = makeNode(AlterObjectSchemaStmt);
					n->objectType = OBJECT_TYPE;
					n->object = yyvsp[-3].list;
					n->newschema = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 653:
#line 4131 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_AGGREGATE;
					n->object = yyvsp[-6].list;
					n->objarg = list_make1(yyvsp[-4].typnam);
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 654:
#line 4140 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_CONVERSION;
					n->object = yyvsp[-3].list;
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 655:
#line 4148 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_DATABASE;
					n->object = list_make1(yyvsp[-3].str);
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 656:
#line 4156 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_DOMAIN;
					n->object = yyvsp[-3].list;
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 657:
#line 4164 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_FUNCTION;
					n->object = yyvsp[-4].list;
					n->objarg = extractArgTypes(yyvsp[-3].list);
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 658:
#line 4173 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_OPERATOR;
					n->object = yyvsp[-6].list;
					n->objarg = yyvsp[-4].list;
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 659:
#line 4182 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_OPCLASS;
					n->object = yyvsp[-5].list;
					n->addname = yyvsp[-3].str;
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 660:
#line 4191 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_SCHEMA;
					n->object = list_make1(yyvsp[-3].str);
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 661:
#line 4199 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_TYPE;
					n->object = yyvsp[-3].list;
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 662:
#line 4207 "gram.y"
    {
					AlterOwnerStmt *n = makeNode(AlterOwnerStmt);
					n->objectType = OBJECT_TABLESPACE;
					n->object = list_make1(yyvsp[-3].str);
					n->newowner = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 663:
#line 4224 "gram.y"
    { QueryIsRule=TRUE; }
    break;

  case 664:
#line 4227 "gram.y"
    {
					RuleStmt *n = makeNode(RuleStmt);
					n->replace = yyvsp[-12].boolean;
					n->relation = yyvsp[-4].range;
					n->rulename = yyvsp[-10].str;
					n->whereClause = yyvsp[-3].node;
					n->event = yyvsp[-6].ival;
					n->instead = yyvsp[-1].boolean;
					n->actions = yyvsp[0].list;
					yyval.node = (Node *)n;
					QueryIsRule=FALSE;
				}
    break;

  case 665:
#line 4242 "gram.y"
    { yyval.list = NIL; }
    break;

  case 666:
#line 4243 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 667:
#line 4244 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 668:
#line 4250 "gram.y"
    { if (yyvsp[0].node != NULL)
					yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node);
				  else
					yyval.list = yyvsp[-2].list;
				}
    break;

  case 669:
#line 4256 "gram.y"
    { if (yyvsp[0].node != NULL)
					yyval.list = list_make1(yyvsp[0].node);
				  else
					yyval.list = NIL;
				}
    break;

  case 675:
#line 4272 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 676:
#line 4273 "gram.y"
    { yyval.node = NULL; }
    break;

  case 677:
#line 4277 "gram.y"
    { yyval.ival = CMD_SELECT; }
    break;

  case 678:
#line 4278 "gram.y"
    { yyval.ival = CMD_UPDATE; }
    break;

  case 679:
#line 4279 "gram.y"
    { yyval.ival = CMD_DELETE; }
    break;

  case 680:
#line 4280 "gram.y"
    { yyval.ival = CMD_INSERT; }
    break;

  case 681:
#line 4284 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 682:
#line 4285 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 683:
#line 4286 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 684:
#line 4292 "gram.y"
    {
					DropPropertyStmt *n = makeNode(DropPropertyStmt);
					n->relation = yyvsp[-1].range;
					n->property = yyvsp[-3].str;
					n->behavior = yyvsp[0].dbehavior;
					n->removeType = OBJECT_RULE;
					yyval.node = (Node *) n;
				}
    break;

  case 685:
#line 4312 "gram.y"
    {
					NotifyStmt *n = makeNode(NotifyStmt);
					n->relation = yyvsp[0].range;
					yyval.node = (Node *)n;
				}
    break;

  case 686:
#line 4320 "gram.y"
    {
					ListenStmt *n = makeNode(ListenStmt);
					n->relation = yyvsp[0].range;
					yyval.node = (Node *)n;
				}
    break;

  case 687:
#line 4329 "gram.y"
    {
					UnlistenStmt *n = makeNode(UnlistenStmt);
					n->relation = yyvsp[0].range;
					yyval.node = (Node *)n;
				}
    break;

  case 688:
#line 4335 "gram.y"
    {
					UnlistenStmt *n = makeNode(UnlistenStmt);
					n->relation = makeNode(RangeVar);
					n->relation->relname = "*";
					n->relation->schemaname = NULL;
					yyval.node = (Node *)n;
				}
    break;

  case 689:
#line 4356 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_ROLLBACK;
					n->options = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 690:
#line 4363 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_BEGIN;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 691:
#line 4370 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_START;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 692:
#line 4377 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_COMMIT;
					n->options = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 693:
#line 4384 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_COMMIT;
					n->options = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 694:
#line 4391 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_ROLLBACK;
					n->options = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 695:
#line 4398 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_SAVEPOINT;
					n->options = list_make1(makeDefElem("savepoint_name",
														(Node *)makeString(yyvsp[0].str)));
					yyval.node = (Node *)n;
				}
    break;

  case 696:
#line 4406 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_RELEASE;
					n->options = list_make1(makeDefElem("savepoint_name",
														(Node *)makeString(yyvsp[0].str)));
					yyval.node = (Node *)n;
				}
    break;

  case 697:
#line 4414 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_RELEASE;
					n->options = list_make1(makeDefElem("savepoint_name",
														(Node *)makeString(yyvsp[0].str)));
					yyval.node = (Node *)n;
				}
    break;

  case 698:
#line 4422 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_ROLLBACK_TO;
					n->options = list_make1(makeDefElem("savepoint_name",
														(Node *)makeString(yyvsp[0].str)));
					yyval.node = (Node *)n;
				}
    break;

  case 699:
#line 4430 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_ROLLBACK_TO;
					n->options = list_make1(makeDefElem("savepoint_name",
														(Node *)makeString(yyvsp[0].str)));
					yyval.node = (Node *)n;
				}
    break;

  case 700:
#line 4438 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_PREPARE;
					n->gid = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 701:
#line 4445 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_COMMIT_PREPARED;
					n->gid = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 702:
#line 4452 "gram.y"
    {
					TransactionStmt *n = makeNode(TransactionStmt);
					n->kind = TRANS_STMT_ROLLBACK_PREPARED;
					n->gid = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 703:
#line 4460 "gram.y"
    {}
    break;

  case 704:
#line 4461 "gram.y"
    {}
    break;

  case 705:
#line 4462 "gram.y"
    {}
    break;

  case 706:
#line 4467 "gram.y"
    { yyval.defelt = makeDefElem("transaction_isolation",
									   makeStringConst(yyvsp[0].str, NULL)); }
    break;

  case 707:
#line 4470 "gram.y"
    { yyval.defelt = makeDefElem("transaction_read_only",
									   makeIntConst(TRUE)); }
    break;

  case 708:
#line 4473 "gram.y"
    { yyval.defelt = makeDefElem("transaction_read_only",
									   makeIntConst(FALSE)); }
    break;

  case 709:
#line 4480 "gram.y"
    { yyval.list = list_make1(yyvsp[0].defelt); }
    break;

  case 710:
#line 4482 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].defelt); }
    break;

  case 711:
#line 4484 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].defelt); }
    break;

  case 713:
#line 4490 "gram.y"
    { yyval.list = NIL; }
    break;

  case 714:
#line 4503 "gram.y"
    {
					ViewStmt *n = makeNode(ViewStmt);
					n->replace = false;
					n->view = yyvsp[-3].range;
					n->view->istemp = yyvsp[-5].boolean;
					n->aliases = yyvsp[-2].list;
					n->query = (Query *) yyvsp[0].node;
					yyval.node = (Node *) n;
				}
    break;

  case 715:
#line 4514 "gram.y"
    {
					ViewStmt *n = makeNode(ViewStmt);
					n->replace = true;
					n->view = yyvsp[-3].range;
					n->view->istemp = yyvsp[-5].boolean;
					n->aliases = yyvsp[-2].list;
					n->query = (Query *) yyvsp[0].node;
					yyval.node = (Node *) n;
				}
    break;

  case 716:
#line 4533 "gram.y"
    {
					LoadStmt *n = makeNode(LoadStmt);
					n->filename = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 717:
#line 4549 "gram.y"
    {
					CreatedbStmt *n = makeNode(CreatedbStmt);
					n->dbname = yyvsp[-2].str;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 718:
#line 4558 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].defelt); }
    break;

  case 719:
#line 4559 "gram.y"
    { yyval.list = NIL; }
    break;

  case 720:
#line 4564 "gram.y"
    {
					yyval.defelt = makeDefElem("tablespace", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 721:
#line 4568 "gram.y"
    {
					yyval.defelt = makeDefElem("tablespace", NULL);
				}
    break;

  case 722:
#line 4572 "gram.y"
    {
					yyval.defelt = makeDefElem("location", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 723:
#line 4576 "gram.y"
    {
					yyval.defelt = makeDefElem("location", NULL);
				}
    break;

  case 724:
#line 4580 "gram.y"
    {
					yyval.defelt = makeDefElem("template", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 725:
#line 4584 "gram.y"
    {
					yyval.defelt = makeDefElem("template", NULL);
				}
    break;

  case 726:
#line 4588 "gram.y"
    {
					yyval.defelt = makeDefElem("encoding", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 727:
#line 4592 "gram.y"
    {
					yyval.defelt = makeDefElem("encoding", (Node *)makeInteger(yyvsp[0].ival));
				}
    break;

  case 728:
#line 4596 "gram.y"
    {
					yyval.defelt = makeDefElem("encoding", NULL);
				}
    break;

  case 729:
#line 4600 "gram.y"
    {
					yyval.defelt = makeDefElem("connectionlimit", (Node *)makeInteger(yyvsp[0].ival));
				}
    break;

  case 730:
#line 4604 "gram.y"
    {
					yyval.defelt = makeDefElem("owner", (Node *)makeString(yyvsp[0].str));
				}
    break;

  case 731:
#line 4608 "gram.y"
    {
					yyval.defelt = makeDefElem("owner", NULL);
				}
    break;

  case 732:
#line 4617 "gram.y"
    {}
    break;

  case 733:
#line 4618 "gram.y"
    {}
    break;

  case 734:
#line 4630 "gram.y"
    {
					AlterDatabaseStmt *n = makeNode(AlterDatabaseStmt);
					n->dbname = yyvsp[-2].str;
					n->options = yyvsp[0].list;
					yyval.node = (Node *)n;
				 }
    break;

  case 735:
#line 4640 "gram.y"
    {
					AlterDatabaseSetStmt *n = makeNode(AlterDatabaseSetStmt);
					n->dbname = yyvsp[-2].str;
					n->variable = yyvsp[0].vsetstmt->name;
					n->value = yyvsp[0].vsetstmt->args;
					yyval.node = (Node *)n;
				}
    break;

  case 736:
#line 4648 "gram.y"
    {
					AlterDatabaseSetStmt *n = makeNode(AlterDatabaseSetStmt);
					n->dbname = yyvsp[-1].str;
					n->variable = ((VariableResetStmt *)yyvsp[0].node)->name;
					n->value = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 737:
#line 4659 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].defelt); }
    break;

  case 738:
#line 4660 "gram.y"
    { yyval.list = NIL; }
    break;

  case 739:
#line 4665 "gram.y"
    {
					yyval.defelt = makeDefElem("connectionlimit", (Node *)makeInteger(yyvsp[0].ival));
				}
    break;

  case 740:
#line 4679 "gram.y"
    {
					DropdbStmt *n = makeNode(DropdbStmt);
					n->dbname = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 741:
#line 4695 "gram.y"
    {
					CreateDomainStmt *n = makeNode(CreateDomainStmt);
					n->domainname = yyvsp[-3].list;
					n->typename = yyvsp[-1].typnam;
					n->constraints = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 742:
#line 4707 "gram.y"
    {
					AlterDomainStmt *n = makeNode(AlterDomainStmt);
					n->subtype = 'T';
					n->typename = yyvsp[-1].list;
					n->def = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 743:
#line 4716 "gram.y"
    {
					AlterDomainStmt *n = makeNode(AlterDomainStmt);
					n->subtype = 'N';
					n->typename = yyvsp[-3].list;
					yyval.node = (Node *)n;
				}
    break;

  case 744:
#line 4724 "gram.y"
    {
					AlterDomainStmt *n = makeNode(AlterDomainStmt);
					n->subtype = 'O';
					n->typename = yyvsp[-3].list;
					yyval.node = (Node *)n;
				}
    break;

  case 745:
#line 4732 "gram.y"
    {
					AlterDomainStmt *n = makeNode(AlterDomainStmt);
					n->subtype = 'C';
					n->typename = yyvsp[-2].list;
					n->def = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 746:
#line 4741 "gram.y"
    {
					AlterDomainStmt *n = makeNode(AlterDomainStmt);
					n->subtype = 'X';
					n->typename = yyvsp[-4].list;
					n->name = yyvsp[-1].str;
					n->behavior = yyvsp[0].dbehavior;
					yyval.node = (Node *)n;
				}
    break;

  case 747:
#line 4751 "gram.y"
    {}
    break;

  case 748:
#line 4752 "gram.y"
    {}
    break;

  case 749:
#line 4768 "gram.y"
    {
			  CreateConversionStmt *n = makeNode(CreateConversionStmt);
			  n->conversion_name = yyvsp[-6].list;
			  n->for_encoding_name = yyvsp[-4].str;
			  n->to_encoding_name = yyvsp[-2].str;
			  n->func_name = yyvsp[0].list;
			  n->def = yyvsp[-8].boolean;
			  yyval.node = (Node *)n;
			}
    break;

  case 750:
#line 4790 "gram.y"
    {
				   ClusterStmt *n = makeNode(ClusterStmt);
				   n->relation = yyvsp[0].range;
				   n->indexname = yyvsp[-2].str;
				   yyval.node = (Node*)n;
				}
    break;

  case 751:
#line 4797 "gram.y"
    {
			       ClusterStmt *n = makeNode(ClusterStmt);
				   n->relation = yyvsp[0].range;
				   n->indexname = NULL;
				   yyval.node = (Node*)n;
				}
    break;

  case 752:
#line 4804 "gram.y"
    {
				   ClusterStmt *n = makeNode(ClusterStmt);
				   n->relation = NULL;
				   n->indexname = NULL;
				   yyval.node = (Node*)n;
				}
    break;

  case 753:
#line 4821 "gram.y"
    {
					VacuumStmt *n = makeNode(VacuumStmt);
					n->vacuum = true;
					n->analyze = false;
					n->full = yyvsp[-2].boolean;
					n->freeze = yyvsp[-1].boolean;
					n->verbose = yyvsp[0].boolean;
					n->relation = NULL;
					n->va_cols = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 754:
#line 4833 "gram.y"
    {
					VacuumStmt *n = makeNode(VacuumStmt);
					n->vacuum = true;
					n->analyze = false;
					n->full = yyvsp[-3].boolean;
					n->freeze = yyvsp[-2].boolean;
					n->verbose = yyvsp[-1].boolean;
					n->relation = yyvsp[0].range;
					n->va_cols = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 755:
#line 4845 "gram.y"
    {
					VacuumStmt *n = (VacuumStmt *) yyvsp[0].node;
					n->vacuum = true;
					n->full = yyvsp[-3].boolean;
					n->freeze = yyvsp[-2].boolean;
					n->verbose |= yyvsp[-1].boolean;
					yyval.node = (Node *)n;
				}
    break;

  case 756:
#line 4857 "gram.y"
    {
					VacuumStmt *n = makeNode(VacuumStmt);
					n->vacuum = false;
					n->analyze = true;
					n->full = false;
					n->freeze = false;
					n->verbose = yyvsp[0].boolean;
					n->relation = NULL;
					n->va_cols = NIL;
					yyval.node = (Node *)n;
				}
    break;

  case 757:
#line 4869 "gram.y"
    {
					VacuumStmt *n = makeNode(VacuumStmt);
					n->vacuum = false;
					n->analyze = true;
					n->full = false;
					n->freeze = false;
					n->verbose = yyvsp[-2].boolean;
					n->relation = yyvsp[-1].range;
					n->va_cols = yyvsp[0].list;
					yyval.node = (Node *)n;
				}
    break;

  case 758:
#line 4883 "gram.y"
    {}
    break;

  case 759:
#line 4884 "gram.y"
    {}
    break;

  case 760:
#line 4888 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 761:
#line 4889 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 762:
#line 4892 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 763:
#line 4893 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 764:
#line 4896 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 765:
#line 4897 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 766:
#line 4901 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 767:
#line 4902 "gram.y"
    { yyval.list = NIL; }
    break;

  case 768:
#line 4914 "gram.y"
    {
					ExplainStmt *n = makeNode(ExplainStmt);
					n->analyze = yyvsp[-2].boolean;
					n->verbose = yyvsp[-1].boolean;
					n->query = (Query*)yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 775:
#line 4933 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 776:
#line 4934 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 777:
#line 4945 "gram.y"
    {
					PrepareStmt *n = makeNode(PrepareStmt);
					n->name = yyvsp[-3].str;
					n->argtypes = yyvsp[-2].list;
					n->query = (Query *) yyvsp[0].node;
					yyval.node = (Node *) n;
				}
    break;

  case 778:
#line 4954 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 779:
#line 4955 "gram.y"
    { yyval.list = NIL; }
    break;

  case 780:
#line 4958 "gram.y"
    { yyval.list = list_make1(yyvsp[0].typnam); }
    break;

  case 781:
#line 4960 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].typnam); }
    break;

  case 786:
#line 4978 "gram.y"
    {
					ExecuteStmt *n = makeNode(ExecuteStmt);
					n->name = yyvsp[-1].str;
					n->params = yyvsp[0].list;
					n->into = NULL;
					yyval.node = (Node *) n;
				}
    break;

  case 787:
#line 4986 "gram.y"
    {
					ExecuteStmt *n = makeNode(ExecuteStmt);
					n->name = yyvsp[-1].str;
					n->params = yyvsp[0].list;
					yyvsp[-5].range->istemp = yyvsp[-7].boolean;
					n->into = yyvsp[-5].range;
					if (yyvsp[-4].list)
						ereport(ERROR,
								(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
								 errmsg("column name list not allowed in CREATE TABLE / AS EXECUTE")));
					/* ... because it's not implemented, but it could be */
					yyval.node = (Node *) n;
				}
    break;

  case 788:
#line 5001 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 789:
#line 5002 "gram.y"
    { yyval.list = NIL; }
    break;

  case 790:
#line 5013 "gram.y"
    {
						DeallocateStmt *n = makeNode(DeallocateStmt);
						n->name = yyvsp[0].str;
						yyval.node = (Node *) n;
					}
    break;

  case 791:
#line 5019 "gram.y"
    {
						DeallocateStmt *n = makeNode(DeallocateStmt);
						n->name = yyvsp[0].str;
						yyval.node = (Node *) n;
					}
    break;

  case 792:
#line 5035 "gram.y"
    {
					yyvsp[0].istmt->relation = yyvsp[-1].range;
					yyval.node = (Node *) yyvsp[0].istmt;
				}
    break;

  case 793:
#line 5043 "gram.y"
    {
					yyval.istmt = makeNode(InsertStmt);
					yyval.istmt->cols = NIL;
					yyval.istmt->targetList = yyvsp[-1].list;
					yyval.istmt->selectStmt = NULL;
				}
    break;

  case 794:
#line 5050 "gram.y"
    {
					yyval.istmt = makeNode(InsertStmt);
					yyval.istmt->cols = NIL;
					yyval.istmt->targetList = NIL;
					yyval.istmt->selectStmt = NULL;
				}
    break;

  case 795:
#line 5057 "gram.y"
    {
					yyval.istmt = makeNode(InsertStmt);
					yyval.istmt->cols = NIL;
					yyval.istmt->targetList = NIL;
					yyval.istmt->selectStmt = yyvsp[0].node;
				}
    break;

  case 796:
#line 5064 "gram.y"
    {
					yyval.istmt = makeNode(InsertStmt);
					yyval.istmt->cols = yyvsp[-5].list;
					yyval.istmt->targetList = yyvsp[-1].list;
					yyval.istmt->selectStmt = NULL;
				}
    break;

  case 797:
#line 5071 "gram.y"
    {
					yyval.istmt = makeNode(InsertStmt);
					yyval.istmt->cols = yyvsp[-2].list;
					yyval.istmt->targetList = NIL;
					yyval.istmt->selectStmt = yyvsp[0].node;
				}
    break;

  case 798:
#line 5081 "gram.y"
    { yyval.list = list_make1(yyvsp[0].target); }
    break;

  case 799:
#line 5083 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].target); }
    break;

  case 800:
#line 5088 "gram.y"
    {
					yyval.target = makeNode(ResTarget);
					yyval.target->name = yyvsp[-1].str;
					yyval.target->indirection = yyvsp[0].list;
					yyval.target->val = NULL;
				}
    break;

  case 801:
#line 5105 "gram.y"
    {
					DeleteStmt *n = makeNode(DeleteStmt);
					n->relation = yyvsp[-2].range;
					n->usingClause = yyvsp[-1].list;
					n->whereClause = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 802:
#line 5115 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 803:
#line 5116 "gram.y"
    { yyval.list = NIL; }
    break;

  case 804:
#line 5120 "gram.y"
    {
					LockStmt *n = makeNode(LockStmt);

					n->relations = yyvsp[-2].list;
					n->mode = yyvsp[-1].ival;
					n->nowait = yyvsp[0].boolean;
					yyval.node = (Node *)n;
				}
    break;

  case 805:
#line 5130 "gram.y"
    { yyval.ival = yyvsp[-1].ival; }
    break;

  case 806:
#line 5131 "gram.y"
    { yyval.ival = AccessExclusiveLock; }
    break;

  case 807:
#line 5134 "gram.y"
    { yyval.ival = AccessShareLock; }
    break;

  case 808:
#line 5135 "gram.y"
    { yyval.ival = RowShareLock; }
    break;

  case 809:
#line 5136 "gram.y"
    { yyval.ival = RowExclusiveLock; }
    break;

  case 810:
#line 5137 "gram.y"
    { yyval.ival = ShareUpdateExclusiveLock; }
    break;

  case 811:
#line 5138 "gram.y"
    { yyval.ival = ShareLock; }
    break;

  case 812:
#line 5139 "gram.y"
    { yyval.ival = ShareRowExclusiveLock; }
    break;

  case 813:
#line 5140 "gram.y"
    { yyval.ival = ExclusiveLock; }
    break;

  case 814:
#line 5141 "gram.y"
    { yyval.ival = AccessExclusiveLock; }
    break;

  case 815:
#line 5144 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 816:
#line 5145 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 817:
#line 5160 "gram.y"
    {
					UpdateStmt *n = makeNode(UpdateStmt);
					n->relation = yyvsp[-4].range;
					n->targetList = yyvsp[-2].list;
					n->fromClause = yyvsp[-1].list;
					n->whereClause = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 818:
#line 5178 "gram.y"
    {
					DeclareCursorStmt *n = makeNode(DeclareCursorStmt);
					n->portalname = yyvsp[-5].str;
					n->options = yyvsp[-4].ival;
					n->query = yyvsp[0].node;
					if (yyvsp[-2].boolean)
						n->options |= CURSOR_OPT_HOLD;
					yyval.node = (Node *)n;
				}
    break;

  case 819:
#line 5189 "gram.y"
    { yyval.ival = 0; }
    break;

  case 820:
#line 5190 "gram.y"
    { yyval.ival = yyvsp[-2].ival | CURSOR_OPT_NO_SCROLL; }
    break;

  case 821:
#line 5191 "gram.y"
    { yyval.ival = yyvsp[-1].ival | CURSOR_OPT_SCROLL; }
    break;

  case 822:
#line 5192 "gram.y"
    { yyval.ival = yyvsp[-1].ival | CURSOR_OPT_BINARY; }
    break;

  case 823:
#line 5193 "gram.y"
    { yyval.ival = yyvsp[-1].ival | CURSOR_OPT_INSENSITIVE; }
    break;

  case 824:
#line 5196 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 825:
#line 5197 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 826:
#line 5198 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 829:
#line 5251 "gram.y"
    { yyval.node = yyvsp[-1].node; }
    break;

  case 830:
#line 5252 "gram.y"
    { yyval.node = yyvsp[-1].node; }
    break;

  case 831:
#line 5262 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 832:
#line 5264 "gram.y"
    {
					insertSelectOptions((SelectStmt *) yyvsp[-1].node, yyvsp[0].list, NULL,
										NULL, NULL);
					yyval.node = yyvsp[-1].node;
				}
    break;

  case 833:
#line 5270 "gram.y"
    {
					insertSelectOptions((SelectStmt *) yyvsp[-3].node, yyvsp[-2].list, yyvsp[-1].node,
										list_nth(yyvsp[0].list, 0), list_nth(yyvsp[0].list, 1));
					yyval.node = yyvsp[-3].node;
				}
    break;

  case 834:
#line 5276 "gram.y"
    {
					insertSelectOptions((SelectStmt *) yyvsp[-3].node, yyvsp[-2].list, yyvsp[0].node,
										list_nth(yyvsp[-1].list, 0), list_nth(yyvsp[-1].list, 1));
					yyval.node = yyvsp[-3].node;
				}
    break;

  case 835:
#line 5284 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 836:
#line 5285 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 837:
#line 5315 "gram.y"
    {
					SelectStmt *n = makeNode(SelectStmt);
					n->distinctClause = yyvsp[-6].list;
					n->targetList = yyvsp[-5].list;
					n->into = yyvsp[-4].range;
					n->intoColNames = NIL;
					n->intoHasOids = DEFAULT_OIDS;
					n->fromClause = yyvsp[-3].list;
					n->whereClause = yyvsp[-2].node;
					n->groupClause = yyvsp[-1].list;
					n->havingClause = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 838:
#line 5329 "gram.y"
    {
					yyval.node = makeSetOp(SETOP_UNION, yyvsp[-1].boolean, yyvsp[-3].node, yyvsp[0].node);
				}
    break;

  case 839:
#line 5333 "gram.y"
    {
					yyval.node = makeSetOp(SETOP_INTERSECT, yyvsp[-1].boolean, yyvsp[-3].node, yyvsp[0].node);
				}
    break;

  case 840:
#line 5337 "gram.y"
    {
					yyval.node = makeSetOp(SETOP_EXCEPT, yyvsp[-1].boolean, yyvsp[-3].node, yyvsp[0].node);
				}
    break;

  case 841:
#line 5343 "gram.y"
    { yyval.range = yyvsp[0].range; }
    break;

  case 842:
#line 5344 "gram.y"
    { yyval.range = NULL; }
    break;

  case 843:
#line 5353 "gram.y"
    {
					yyval.range = yyvsp[0].range;
					yyval.range->istemp = true;
				}
    break;

  case 844:
#line 5358 "gram.y"
    {
					yyval.range = yyvsp[0].range;
					yyval.range->istemp = true;
				}
    break;

  case 845:
#line 5363 "gram.y"
    {
					yyval.range = yyvsp[0].range;
					yyval.range->istemp = true;
				}
    break;

  case 846:
#line 5368 "gram.y"
    {
					yyval.range = yyvsp[0].range;
					yyval.range->istemp = true;
				}
    break;

  case 847:
#line 5373 "gram.y"
    {
					yyval.range = yyvsp[0].range;
					yyval.range->istemp = true;
				}
    break;

  case 848:
#line 5378 "gram.y"
    {
					yyval.range = yyvsp[0].range;
					yyval.range->istemp = true;
				}
    break;

  case 849:
#line 5383 "gram.y"
    {
					yyval.range = yyvsp[0].range;
					yyval.range->istemp = false;
				}
    break;

  case 850:
#line 5388 "gram.y"
    {
					yyval.range = yyvsp[0].range;
					yyval.range->istemp = false;
				}
    break;

  case 851:
#line 5394 "gram.y"
    {}
    break;

  case 852:
#line 5395 "gram.y"
    {}
    break;

  case 853:
#line 5398 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 854:
#line 5399 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 855:
#line 5400 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 856:
#line 5407 "gram.y"
    { yyval.list = list_make1(NIL); }
    break;

  case 857:
#line 5408 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 858:
#line 5409 "gram.y"
    { yyval.list = NIL; }
    break;

  case 859:
#line 5410 "gram.y"
    { yyval.list = NIL; }
    break;

  case 860:
#line 5414 "gram.y"
    { yyval.list = yyvsp[0].list;}
    break;

  case 861:
#line 5415 "gram.y"
    { yyval.list = NIL; }
    break;

  case 862:
#line 5419 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 863:
#line 5423 "gram.y"
    { yyval.list = list_make1(yyvsp[0].sortby); }
    break;

  case 864:
#line 5424 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].sortby); }
    break;

  case 865:
#line 5428 "gram.y"
    {
					yyval.sortby = makeNode(SortBy);
					yyval.sortby->node = yyvsp[-2].node;
					yyval.sortby->sortby_kind = SORTBY_USING;
					yyval.sortby->useOp = yyvsp[0].list;
				}
    break;

  case 866:
#line 5435 "gram.y"
    {
					yyval.sortby = makeNode(SortBy);
					yyval.sortby->node = yyvsp[-1].node;
					yyval.sortby->sortby_kind = SORTBY_ASC;
					yyval.sortby->useOp = NIL;
				}
    break;

  case 867:
#line 5442 "gram.y"
    {
					yyval.sortby = makeNode(SortBy);
					yyval.sortby->node = yyvsp[-1].node;
					yyval.sortby->sortby_kind = SORTBY_DESC;
					yyval.sortby->useOp = NIL;
				}
    break;

  case 868:
#line 5449 "gram.y"
    {
					yyval.sortby = makeNode(SortBy);
					yyval.sortby->node = yyvsp[0].node;
					yyval.sortby->sortby_kind = SORTBY_ASC;	/* default */
					yyval.sortby->useOp = NIL;
				}
    break;

  case 869:
#line 5460 "gram.y"
    { yyval.list = list_make2(yyvsp[0].node, yyvsp[-2].node); }
    break;

  case 870:
#line 5462 "gram.y"
    { yyval.list = list_make2(yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 871:
#line 5464 "gram.y"
    { yyval.list = list_make2(NULL, yyvsp[0].node); }
    break;

  case 872:
#line 5466 "gram.y"
    { yyval.list = list_make2(yyvsp[0].node, NULL); }
    break;

  case 873:
#line 5468 "gram.y"
    {
					/* Disabled because it was too confusing, bjm 2002-02-18 */
					ereport(ERROR,
							(errcode(ERRCODE_SYNTAX_ERROR),
							 errmsg("LIMIT #,# syntax is not supported"),
							 errhint("Use separate LIMIT and OFFSET clauses.")));
				}
    break;

  case 874:
#line 5478 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 875:
#line 5480 "gram.y"
    { yyval.list = list_make2(NULL,NULL); }
    break;

  case 876:
#line 5484 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 877:
#line 5486 "gram.y"
    {
					/* LIMIT ALL is represented as a NULL constant */
					A_Const *n = makeNode(A_Const);
					n->val.type = T_Null;
					yyval.node = (Node *)n;
				}
    break;

  case 878:
#line 5495 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 879:
#line 5499 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 880:
#line 5500 "gram.y"
    { yyval.list = NIL; }
    break;

  case 881:
#line 5504 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 882:
#line 5505 "gram.y"
    { yyval.node = NULL; }
    break;

  case 883:
#line 5510 "gram.y"
    {
					LockingClause *n = makeNode(LockingClause);
					n->lockedRels = yyvsp[-1].list;
					n->forUpdate = TRUE;
					n->nowait = yyvsp[0].boolean;
					yyval.node = (Node *) n;
				}
    break;

  case 884:
#line 5518 "gram.y"
    {
					LockingClause *n = makeNode(LockingClause);
					n->lockedRels = yyvsp[-1].list;
					n->forUpdate = FALSE;
					n->nowait = yyvsp[0].boolean;
					yyval.node = (Node *) n;
				}
    break;

  case 885:
#line 5525 "gram.y"
    { yyval.node = NULL; }
    break;

  case 886:
#line 5529 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 887:
#line 5530 "gram.y"
    { yyval.node = NULL; }
    break;

  case 888:
#line 5534 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 889:
#line 5535 "gram.y"
    { yyval.list = NIL; }
    break;

  case 890:
#line 5547 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 891:
#line 5548 "gram.y"
    { yyval.list = NIL; }
    break;

  case 892:
#line 5552 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 893:
#line 5553 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node); }
    break;

  case 894:
#line 5564 "gram.y"
    {
					yyval.node = (Node *) yyvsp[0].range;
				}
    break;

  case 895:
#line 5568 "gram.y"
    {
					yyvsp[-1].range->alias = yyvsp[0].alias;
					yyval.node = (Node *) yyvsp[-1].range;
				}
    break;

  case 896:
#line 5573 "gram.y"
    {
					RangeFunction *n = makeNode(RangeFunction);
					n->funccallnode = yyvsp[0].node;
					n->coldeflist = NIL;
					yyval.node = (Node *) n;
				}
    break;

  case 897:
#line 5580 "gram.y"
    {
					RangeFunction *n = makeNode(RangeFunction);
					n->funccallnode = yyvsp[-1].node;
					n->alias = yyvsp[0].alias;
					n->coldeflist = NIL;
					yyval.node = (Node *) n;
				}
    break;

  case 898:
#line 5588 "gram.y"
    {
					RangeFunction *n = makeNode(RangeFunction);
					n->funccallnode = yyvsp[-4].node;
					n->coldeflist = yyvsp[-1].list;
					yyval.node = (Node *) n;
				}
    break;

  case 899:
#line 5595 "gram.y"
    {
					RangeFunction *n = makeNode(RangeFunction);
					Alias *a = makeNode(Alias);
					n->funccallnode = yyvsp[-5].node;
					a->aliasname = yyvsp[-3].str;
					n->alias = a;
					n->coldeflist = yyvsp[-1].list;
					yyval.node = (Node *) n;
				}
    break;

  case 900:
#line 5605 "gram.y"
    {
					RangeFunction *n = makeNode(RangeFunction);
					Alias *a = makeNode(Alias);
					n->funccallnode = yyvsp[-4].node;
					a->aliasname = yyvsp[-3].str;
					n->alias = a;
					n->coldeflist = yyvsp[-1].list;
					yyval.node = (Node *) n;
				}
    break;

  case 901:
#line 5615 "gram.y"
    {
					/*
					 * The SQL spec does not permit a subselect
					 * (<derived_table>) without an alias clause,
					 * so we don't either.  This avoids the problem
					 * of needing to invent a unique refname for it.
					 * That could be surmounted if there's sufficient
					 * popular demand, but for now let's just implement
					 * the spec and see if anyone complains.
					 * However, it does seem like a good idea to emit
					 * an error message that's better than "syntax error".
					 */
					ereport(ERROR,
							(errcode(ERRCODE_SYNTAX_ERROR),
							 errmsg("subquery in FROM must have an alias"),
							 errhint("For example, FROM (SELECT ...) [AS] foo.")));
					yyval.node = NULL;
				}
    break;

  case 902:
#line 5634 "gram.y"
    {
					RangeSubselect *n = makeNode(RangeSubselect);
					n->subquery = yyvsp[-1].node;
					n->alias = yyvsp[0].alias;
					yyval.node = (Node *) n;
				}
    break;

  case 903:
#line 5641 "gram.y"
    {
					yyval.node = (Node *) yyvsp[0].jexpr;
				}
    break;

  case 904:
#line 5645 "gram.y"
    {
					yyvsp[-2].jexpr->alias = yyvsp[0].alias;
					yyval.node = (Node *) yyvsp[-2].jexpr;
				}
    break;

  case 905:
#line 5671 "gram.y"
    {
					yyval.jexpr = yyvsp[-1].jexpr;
				}
    break;

  case 906:
#line 5675 "gram.y"
    {
					/* CROSS JOIN is same as unqualified inner join */
					JoinExpr *n = makeNode(JoinExpr);
					n->jointype = JOIN_INNER;
					n->isNatural = FALSE;
					n->larg = yyvsp[-3].node;
					n->rarg = yyvsp[0].node;
					n->using = NIL;
					n->quals = NULL;
					yyval.jexpr = n;
				}
    break;

  case 907:
#line 5687 "gram.y"
    {
					/* UNION JOIN is made into 1 token to avoid shift/reduce
					 * conflict against regular UNION keyword.
					 */
					JoinExpr *n = makeNode(JoinExpr);
					n->jointype = JOIN_UNION;
					n->isNatural = FALSE;
					n->larg = yyvsp[-2].node;
					n->rarg = yyvsp[0].node;
					n->using = NIL;
					n->quals = NULL;
					yyval.jexpr = n;
				}
    break;

  case 908:
#line 5701 "gram.y"
    {
					JoinExpr *n = makeNode(JoinExpr);
					n->jointype = yyvsp[-3].jtype;
					n->isNatural = FALSE;
					n->larg = yyvsp[-4].node;
					n->rarg = yyvsp[-1].node;
					if (yyvsp[0].node != NULL && IsA(yyvsp[0].node, List))
						n->using = (List *) yyvsp[0].node; /* USING clause */
					else
						n->quals = yyvsp[0].node; /* ON clause */
					yyval.jexpr = n;
				}
    break;

  case 909:
#line 5714 "gram.y"
    {
					/* letting join_type reduce to empty doesn't work */
					JoinExpr *n = makeNode(JoinExpr);
					n->jointype = JOIN_INNER;
					n->isNatural = FALSE;
					n->larg = yyvsp[-3].node;
					n->rarg = yyvsp[-1].node;
					if (yyvsp[0].node != NULL && IsA(yyvsp[0].node, List))
						n->using = (List *) yyvsp[0].node; /* USING clause */
					else
						n->quals = yyvsp[0].node; /* ON clause */
					yyval.jexpr = n;
				}
    break;

  case 910:
#line 5728 "gram.y"
    {
					JoinExpr *n = makeNode(JoinExpr);
					n->jointype = yyvsp[-2].jtype;
					n->isNatural = TRUE;
					n->larg = yyvsp[-4].node;
					n->rarg = yyvsp[0].node;
					n->using = NIL; /* figure out which columns later... */
					n->quals = NULL; /* fill later */
					yyval.jexpr = n;
				}
    break;

  case 911:
#line 5739 "gram.y"
    {
					/* letting join_type reduce to empty doesn't work */
					JoinExpr *n = makeNode(JoinExpr);
					n->jointype = JOIN_INNER;
					n->isNatural = TRUE;
					n->larg = yyvsp[-3].node;
					n->rarg = yyvsp[0].node;
					n->using = NIL; /* figure out which columns later... */
					n->quals = NULL; /* fill later */
					yyval.jexpr = n;
				}
    break;

  case 912:
#line 5754 "gram.y"
    {
					yyval.alias = makeNode(Alias);
					yyval.alias->aliasname = yyvsp[-3].str;
					yyval.alias->colnames = yyvsp[-1].list;
				}
    break;

  case 913:
#line 5760 "gram.y"
    {
					yyval.alias = makeNode(Alias);
					yyval.alias->aliasname = yyvsp[0].str;
				}
    break;

  case 914:
#line 5765 "gram.y"
    {
					yyval.alias = makeNode(Alias);
					yyval.alias->aliasname = yyvsp[-3].str;
					yyval.alias->colnames = yyvsp[-1].list;
				}
    break;

  case 915:
#line 5771 "gram.y"
    {
					yyval.alias = makeNode(Alias);
					yyval.alias->aliasname = yyvsp[0].str;
				}
    break;

  case 916:
#line 5777 "gram.y"
    { yyval.jtype = JOIN_FULL; }
    break;

  case 917:
#line 5778 "gram.y"
    { yyval.jtype = JOIN_LEFT; }
    break;

  case 918:
#line 5779 "gram.y"
    { yyval.jtype = JOIN_RIGHT; }
    break;

  case 919:
#line 5780 "gram.y"
    { yyval.jtype = JOIN_INNER; }
    break;

  case 920:
#line 5784 "gram.y"
    { yyval.node = NULL; }
    break;

  case 921:
#line 5785 "gram.y"
    { yyval.node = NULL; }
    break;

  case 922:
#line 5797 "gram.y"
    { yyval.node = (Node *) yyvsp[-1].list; }
    break;

  case 923:
#line 5798 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 924:
#line 5804 "gram.y"
    {
					/* default inheritance */
					yyval.range = yyvsp[0].range;
					yyval.range->inhOpt = INH_DEFAULT;
					yyval.range->alias = NULL;
				}
    break;

  case 925:
#line 5811 "gram.y"
    {
					/* inheritance query */
					yyval.range = yyvsp[-1].range;
					yyval.range->inhOpt = INH_YES;
					yyval.range->alias = NULL;
				}
    break;

  case 926:
#line 5818 "gram.y"
    {
					/* no inheritance */
					yyval.range = yyvsp[0].range;
					yyval.range->inhOpt = INH_NO;
					yyval.range->alias = NULL;
				}
    break;

  case 927:
#line 5825 "gram.y"
    {
					/* no inheritance, SQL99-style syntax */
					yyval.range = yyvsp[-1].range;
					yyval.range->inhOpt = INH_NO;
					yyval.range->alias = NULL;
				}
    break;

  case 928:
#line 5834 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 929:
#line 5839 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 930:
#line 5840 "gram.y"
    { yyval.node = NULL; }
    break;

  case 931:
#line 5846 "gram.y"
    {
					yyval.list = list_make1(yyvsp[0].node);
				}
    break;

  case 932:
#line 5850 "gram.y"
    {
					yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node);
				}
    break;

  case 933:
#line 5856 "gram.y"
    {
					ColumnDef *n = makeNode(ColumnDef);
					n->colname = yyvsp[-1].str;
					n->typename = yyvsp[0].typnam;
					n->constraints = NIL;
					n->is_local = true;
					yyval.node = (Node *)n;
				}
    break;

  case 934:
#line 5877 "gram.y"
    {
					yyval.typnam = yyvsp[-1].typnam;
					yyval.typnam->arrayBounds = yyvsp[0].list;
				}
    break;

  case 935:
#line 5882 "gram.y"
    {
					yyval.typnam = yyvsp[-1].typnam;
					yyval.typnam->arrayBounds = yyvsp[0].list;
					yyval.typnam->setof = TRUE;
				}
    break;

  case 936:
#line 5888 "gram.y"
    {
					/* SQL99's redundant syntax */
					yyval.typnam = yyvsp[-4].typnam;
					yyval.typnam->arrayBounds = list_make1(makeInteger(yyvsp[-1].ival));
				}
    break;

  case 937:
#line 5894 "gram.y"
    {
					/* SQL99's redundant syntax */
					yyval.typnam = yyvsp[-4].typnam;
					yyval.typnam->arrayBounds = list_make1(makeInteger(yyvsp[-1].ival));
					yyval.typnam->setof = TRUE;
				}
    break;

  case 938:
#line 5904 "gram.y"
    {  yyval.list = lappend(yyvsp[-2].list, makeInteger(-1)); }
    break;

  case 939:
#line 5906 "gram.y"
    {  yyval.list = lappend(yyvsp[-3].list, makeInteger(yyvsp[-1].ival)); }
    break;

  case 940:
#line 5908 "gram.y"
    {  yyval.list = NIL; }
    break;

  case 941:
#line 5920 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 942:
#line 5921 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 943:
#line 5922 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 944:
#line 5923 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 945:
#line 5924 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 946:
#line 5926 "gram.y"
    {
					yyval.typnam = yyvsp[-1].typnam;
					if (yyvsp[0].ival != INTERVAL_FULL_RANGE)
						yyval.typnam->typmod = INTERVAL_TYPMOD(INTERVAL_FULL_PRECISION, yyvsp[0].ival);
				}
    break;

  case 947:
#line 5932 "gram.y"
    {
					yyval.typnam = yyvsp[-4].typnam;
					if (yyvsp[-2].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("INTERVAL(%d) precision must not be negative",
										yyvsp[-2].ival)));
					if (yyvsp[-2].ival > MAX_INTERVAL_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("INTERVAL(%d) precision reduced to maximum allowed, %d",
										yyvsp[-2].ival, MAX_INTERVAL_PRECISION)));
						yyvsp[-2].ival = MAX_INTERVAL_PRECISION;
					}
					yyval.typnam->typmod = INTERVAL_TYPMOD(yyvsp[-2].ival, yyvsp[0].ival);
				}
    break;

  case 948:
#line 5950 "gram.y"
    {
					yyval.typnam = makeNode(TypeName);
					yyval.typnam->names = lcons(makeString(yyvsp[-1].str), yyvsp[0].list);
					yyval.typnam->typmod = -1;
				}
    break;

  case 949:
#line 5967 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 950:
#line 5968 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 951:
#line 5969 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 952:
#line 5970 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 953:
#line 5971 "gram.y"
    { yyval.typnam = yyvsp[0].typnam; }
    break;

  case 954:
#line 5976 "gram.y"
    {
					yyval.typnam = makeTypeName(yyvsp[0].str);
				}
    break;

  case 955:
#line 5987 "gram.y"
    {
					yyval.typnam = SystemTypeName("int4");
				}
    break;

  case 956:
#line 5991 "gram.y"
    {
					yyval.typnam = SystemTypeName("int4");
				}
    break;

  case 957:
#line 5995 "gram.y"
    {
					yyval.typnam = SystemTypeName("int2");
				}
    break;

  case 958:
#line 5999 "gram.y"
    {
					yyval.typnam = SystemTypeName("int8");
				}
    break;

  case 959:
#line 6003 "gram.y"
    {
					yyval.typnam = SystemTypeName("float4");
				}
    break;

  case 960:
#line 6007 "gram.y"
    {
					yyval.typnam = yyvsp[0].typnam;
				}
    break;

  case 961:
#line 6011 "gram.y"
    {
					yyval.typnam = SystemTypeName("float8");
				}
    break;

  case 962:
#line 6015 "gram.y"
    {
					yyval.typnam = SystemTypeName("numeric");
					yyval.typnam->typmod = yyvsp[0].ival;
				}
    break;

  case 963:
#line 6020 "gram.y"
    {
					yyval.typnam = SystemTypeName("numeric");
					yyval.typnam->typmod = yyvsp[0].ival;
				}
    break;

  case 964:
#line 6025 "gram.y"
    {
					yyval.typnam = SystemTypeName("numeric");
					yyval.typnam->typmod = yyvsp[0].ival;
				}
    break;

  case 965:
#line 6030 "gram.y"
    {
					yyval.typnam = SystemTypeName("bool");
				}
    break;

  case 966:
#line 6036 "gram.y"
    {
					if (yyvsp[-1].ival < 1)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("precision for type float must be at least 1 bit")));
					else if (yyvsp[-1].ival <= 24)
						yyval.typnam = SystemTypeName("float4");
					else if (yyvsp[-1].ival <= 53)
						yyval.typnam = SystemTypeName("float8");
					else
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("precision for type float must be less than 54 bits")));
				}
    break;

  case 967:
#line 6051 "gram.y"
    {
					yyval.typnam = SystemTypeName("float8");
				}
    break;

  case 968:
#line 6058 "gram.y"
    {
					if (yyvsp[-3].ival < 1 || yyvsp[-3].ival > NUMERIC_MAX_PRECISION)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("NUMERIC precision %d must be between 1 and %d",
										yyvsp[-3].ival, NUMERIC_MAX_PRECISION)));
					if (yyvsp[-1].ival < 0 || yyvsp[-1].ival > yyvsp[-3].ival)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("NUMERIC scale %d must be between 0 and precision %d",
										yyvsp[-1].ival, yyvsp[-3].ival)));

					yyval.ival = ((yyvsp[-3].ival << 16) | yyvsp[-1].ival) + VARHDRSZ;
				}
    break;

  case 969:
#line 6073 "gram.y"
    {
					if (yyvsp[-1].ival < 1 || yyvsp[-1].ival > NUMERIC_MAX_PRECISION)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("NUMERIC precision %d must be between 1 and %d",
										yyvsp[-1].ival, NUMERIC_MAX_PRECISION)));

					yyval.ival = (yyvsp[-1].ival << 16) + VARHDRSZ;
				}
    break;

  case 970:
#line 6083 "gram.y"
    {
					/* Insert "-1" meaning "no limit" */
					yyval.ival = -1;
				}
    break;

  case 971:
#line 6091 "gram.y"
    {
					if (yyvsp[-3].ival < 1 || yyvsp[-3].ival > NUMERIC_MAX_PRECISION)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("DECIMAL precision %d must be between 1 and %d",
										yyvsp[-3].ival, NUMERIC_MAX_PRECISION)));
					if (yyvsp[-1].ival < 0 || yyvsp[-1].ival > yyvsp[-3].ival)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("DECIMAL scale %d must be between 0 and precision %d",
										yyvsp[-1].ival, yyvsp[-3].ival)));

					yyval.ival = ((yyvsp[-3].ival << 16) | yyvsp[-1].ival) + VARHDRSZ;
				}
    break;

  case 972:
#line 6106 "gram.y"
    {
					if (yyvsp[-1].ival < 1 || yyvsp[-1].ival > NUMERIC_MAX_PRECISION)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("DECIMAL precision %d must be between 1 and %d",
										yyvsp[-1].ival, NUMERIC_MAX_PRECISION)));

					yyval.ival = (yyvsp[-1].ival << 16) + VARHDRSZ;
				}
    break;

  case 973:
#line 6116 "gram.y"
    {
					/* Insert "-1" meaning "no limit" */
					yyval.ival = -1;
				}
    break;

  case 974:
#line 6128 "gram.y"
    {
					yyval.typnam = yyvsp[0].typnam;
				}
    break;

  case 975:
#line 6132 "gram.y"
    {
					yyval.typnam = yyvsp[0].typnam;
				}
    break;

  case 976:
#line 6140 "gram.y"
    {
					yyval.typnam = yyvsp[0].typnam;
				}
    break;

  case 977:
#line 6144 "gram.y"
    {
					yyval.typnam = yyvsp[0].typnam;
					yyval.typnam->typmod = -1;
				}
    break;

  case 978:
#line 6152 "gram.y"
    {
					char *typname;

					typname = yyvsp[-3].boolean ? "varbit" : "bit";
					yyval.typnam = SystemTypeName(typname);
					if (yyvsp[-1].ival < 1)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("length for type %s must be at least 1",
										typname)));
					else if (yyvsp[-1].ival > (MaxAttrSize * BITS_PER_BYTE))
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("length for type %s cannot exceed %d",
										typname, MaxAttrSize * BITS_PER_BYTE)));
					yyval.typnam->typmod = yyvsp[-1].ival;
				}
    break;

  case 979:
#line 6173 "gram.y"
    {
					/* bit defaults to bit(1), varbit to no limit */
					if (yyvsp[0].boolean)
					{
						yyval.typnam = SystemTypeName("varbit");
						yyval.typnam->typmod = -1;
					}
					else
					{
						yyval.typnam = SystemTypeName("bit");
						yyval.typnam->typmod = 1;
					}
				}
    break;

  case 980:
#line 6194 "gram.y"
    {
					yyval.typnam = yyvsp[0].typnam;
				}
    break;

  case 981:
#line 6198 "gram.y"
    {
					yyval.typnam = yyvsp[0].typnam;
				}
    break;

  case 982:
#line 6204 "gram.y"
    {
					yyval.typnam = yyvsp[0].typnam;
				}
    break;

  case 983:
#line 6208 "gram.y"
    {
					/* Length was not specified so allow to be unrestricted.
					 * This handles problems with fixed-length (bpchar) strings
					 * which in column definitions must default to a length
					 * of one, but should not be constrained if the length
					 * was not specified.
					 */
					yyval.typnam = yyvsp[0].typnam;
					yyval.typnam->typmod = -1;
				}
    break;

  case 984:
#line 6221 "gram.y"
    {
					if ((yyvsp[0].str != NULL) && (strcmp(yyvsp[0].str, "sql_text") != 0))
					{
						char *type;

						type = palloc(strlen(yyvsp[-4].str) + 1 + strlen(yyvsp[0].str) + 1);
						strcpy(type, yyvsp[-4].str);
						strcat(type, "_");
						strcat(type, yyvsp[0].str);
						yyvsp[-4].str = type;
					}

					yyval.typnam = SystemTypeName(yyvsp[-4].str);

					if (yyvsp[-2].ival < 1)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("length for type %s must be at least 1",
										yyvsp[-4].str)));
					else if (yyvsp[-2].ival > MaxAttrSize)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("length for type %s cannot exceed %d",
										yyvsp[-4].str, MaxAttrSize)));

					/* we actually implement these like a varlen, so
					 * the first 4 bytes is the length. (the difference
					 * between these and "text" is that we blank-pad and
					 * truncate where necessary)
					 */
					yyval.typnam->typmod = VARHDRSZ + yyvsp[-2].ival;
				}
    break;

  case 985:
#line 6256 "gram.y"
    {
					if ((yyvsp[0].str != NULL) && (strcmp(yyvsp[0].str, "sql_text") != 0))
					{
						char *type;

						type = palloc(strlen(yyvsp[-1].str) + 1 + strlen(yyvsp[0].str) + 1);
						strcpy(type, yyvsp[-1].str);
						strcat(type, "_");
						strcat(type, yyvsp[0].str);
						yyvsp[-1].str = type;
					}

					yyval.typnam = SystemTypeName(yyvsp[-1].str);

					/* char defaults to char(1), varchar to no limit */
					if (strcmp(yyvsp[-1].str, "char") == 0)
						yyval.typnam->typmod = VARHDRSZ + 1;
					else
						yyval.typnam->typmod = -1;
				}
    break;

  case 986:
#line 6279 "gram.y"
    { yyval.str = yyvsp[0].boolean ? "varchar": "char"; }
    break;

  case 987:
#line 6281 "gram.y"
    { yyval.str = yyvsp[0].boolean ? "varchar": "char"; }
    break;

  case 988:
#line 6283 "gram.y"
    { yyval.str = "varchar"; }
    break;

  case 989:
#line 6285 "gram.y"
    { yyval.str = yyvsp[0].boolean ? "varchar": "char"; }
    break;

  case 990:
#line 6287 "gram.y"
    { yyval.str = yyvsp[0].boolean ? "varchar": "char"; }
    break;

  case 991:
#line 6289 "gram.y"
    { yyval.str = yyvsp[0].boolean ? "varchar": "char"; }
    break;

  case 992:
#line 6293 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 993:
#line 6294 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 994:
#line 6298 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 995:
#line 6299 "gram.y"
    { yyval.str = NULL; }
    break;

  case 996:
#line 6304 "gram.y"
    {
					if (yyvsp[0].boolean)
						yyval.typnam = SystemTypeName("timestamptz");
					else
						yyval.typnam = SystemTypeName("timestamp");
					/* XXX the timezone field seems to be unused
					 * - thomas 2001-09-06
					 */
					yyval.typnam->timezone = yyvsp[0].boolean;
					if (yyvsp[-2].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("TIMESTAMP(%d)%s precision must not be negative",
										yyvsp[-2].ival, (yyvsp[0].boolean ? " WITH TIME ZONE": ""))));
					if (yyvsp[-2].ival > MAX_TIMESTAMP_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("TIMESTAMP(%d)%s precision reduced to maximum allowed, %d",
										yyvsp[-2].ival, (yyvsp[0].boolean ? " WITH TIME ZONE": ""),
										MAX_TIMESTAMP_PRECISION)));
						yyvsp[-2].ival = MAX_TIMESTAMP_PRECISION;
					}
					yyval.typnam->typmod = yyvsp[-2].ival;
				}
    break;

  case 997:
#line 6330 "gram.y"
    {
					if (yyvsp[0].boolean)
						yyval.typnam = SystemTypeName("timestamptz");
					else
						yyval.typnam = SystemTypeName("timestamp");
					/* XXX the timezone field seems to be unused
					 * - thomas 2001-09-06
					 */
					yyval.typnam->timezone = yyvsp[0].boolean;
				}
    break;

  case 998:
#line 6341 "gram.y"
    {
					if (yyvsp[0].boolean)
						yyval.typnam = SystemTypeName("timetz");
					else
						yyval.typnam = SystemTypeName("time");
					if (yyvsp[-2].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("TIME(%d)%s precision must not be negative",
										yyvsp[-2].ival, (yyvsp[0].boolean ? " WITH TIME ZONE": ""))));
					if (yyvsp[-2].ival > MAX_TIME_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("TIME(%d)%s precision reduced to maximum allowed, %d",
										yyvsp[-2].ival, (yyvsp[0].boolean ? " WITH TIME ZONE": ""),
										MAX_TIME_PRECISION)));
						yyvsp[-2].ival = MAX_TIME_PRECISION;
					}
					yyval.typnam->typmod = yyvsp[-2].ival;
				}
    break;

  case 999:
#line 6363 "gram.y"
    {
					if (yyvsp[0].boolean)
						yyval.typnam = SystemTypeName("timetz");
					else
						yyval.typnam = SystemTypeName("time");
				}
    break;

  case 1000:
#line 6372 "gram.y"
    { yyval.typnam = SystemTypeName("interval"); }
    break;

  case 1001:
#line 6376 "gram.y"
    { yyval.boolean = TRUE; }
    break;

  case 1002:
#line 6377 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 1003:
#line 6378 "gram.y"
    { yyval.boolean = FALSE; }
    break;

  case 1004:
#line 6382 "gram.y"
    { yyval.ival = INTERVAL_MASK(YEAR); }
    break;

  case 1005:
#line 6383 "gram.y"
    { yyval.ival = INTERVAL_MASK(MONTH); }
    break;

  case 1006:
#line 6384 "gram.y"
    { yyval.ival = INTERVAL_MASK(DAY); }
    break;

  case 1007:
#line 6385 "gram.y"
    { yyval.ival = INTERVAL_MASK(HOUR); }
    break;

  case 1008:
#line 6386 "gram.y"
    { yyval.ival = INTERVAL_MASK(MINUTE); }
    break;

  case 1009:
#line 6387 "gram.y"
    { yyval.ival = INTERVAL_MASK(SECOND); }
    break;

  case 1010:
#line 6389 "gram.y"
    { yyval.ival = INTERVAL_MASK(YEAR) | INTERVAL_MASK(MONTH); }
    break;

  case 1011:
#line 6391 "gram.y"
    { yyval.ival = INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR); }
    break;

  case 1012:
#line 6393 "gram.y"
    { yyval.ival = INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR)
						| INTERVAL_MASK(MINUTE); }
    break;

  case 1013:
#line 6396 "gram.y"
    { yyval.ival = INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR)
						| INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND); }
    break;

  case 1014:
#line 6399 "gram.y"
    { yyval.ival = INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE); }
    break;

  case 1015:
#line 6401 "gram.y"
    { yyval.ival = INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE)
						| INTERVAL_MASK(SECOND); }
    break;

  case 1016:
#line 6404 "gram.y"
    { yyval.ival = INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND); }
    break;

  case 1017:
#line 6405 "gram.y"
    { yyval.ival = INTERVAL_FULL_RANGE; }
    break;

  case 1018:
#line 6431 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1019:
#line 6433 "gram.y"
    { yyval.node = makeTypeCast(yyvsp[-2].node, yyvsp[0].typnam); }
    break;

  case 1020:
#line 6435 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("timezone");
					n->args = list_make2(yyvsp[0].node, yyvsp[-4].node);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) n;
				}
    break;

  case 1021:
#line 6453 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "+", NULL, yyvsp[0].node); }
    break;

  case 1022:
#line 6455 "gram.y"
    { yyval.node = doNegate(yyvsp[0].node); }
    break;

  case 1023:
#line 6457 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "+", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1024:
#line 6459 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "-", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1025:
#line 6461 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "*", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1026:
#line 6463 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "/", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1027:
#line 6465 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "%", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1028:
#line 6467 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "^", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1029:
#line 6469 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "<", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1030:
#line 6471 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, ">", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1031:
#line 6473 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "=", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1032:
#line 6476 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_OP, yyvsp[-1].list, yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1033:
#line 6478 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_OP, yyvsp[-1].list, NULL, yyvsp[0].node); }
    break;

  case 1034:
#line 6480 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_OP, yyvsp[0].list, yyvsp[-1].node, NULL); }
    break;

  case 1035:
#line 6483 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_AND, NIL, yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1036:
#line 6485 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_OR, NIL, yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1037:
#line 6487 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_NOT, NIL, NULL, yyvsp[0].node); }
    break;

  case 1038:
#line 6490 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "~~", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1039:
#line 6492 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("like_escape");
					n->args = list_make2(yyvsp[-2].node, yyvsp[0].node);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "~~", yyvsp[-4].node, (Node *) n);
				}
    break;

  case 1040:
#line 6501 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "!~~", yyvsp[-3].node, yyvsp[0].node); }
    break;

  case 1041:
#line 6503 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("like_escape");
					n->args = list_make2(yyvsp[-2].node, yyvsp[0].node);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "!~~", yyvsp[-5].node, (Node *) n);
				}
    break;

  case 1042:
#line 6512 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "~~*", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1043:
#line 6514 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("like_escape");
					n->args = list_make2(yyvsp[-2].node, yyvsp[0].node);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "~~*", yyvsp[-4].node, (Node *) n);
				}
    break;

  case 1044:
#line 6523 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "!~~*", yyvsp[-3].node, yyvsp[0].node); }
    break;

  case 1045:
#line 6525 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("like_escape");
					n->args = list_make2(yyvsp[-2].node, yyvsp[0].node);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "!~~*", yyvsp[-5].node, (Node *) n);
				}
    break;

  case 1046:
#line 6535 "gram.y"
    {
					A_Const *c = makeNode(A_Const);
					FuncCall *n = makeNode(FuncCall);
					c->val.type = T_Null;
					n->funcname = SystemFuncName("similar_escape");
					n->args = list_make2(yyvsp[0].node, (Node *) c);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "~", yyvsp[-3].node, (Node *) n);
				}
    break;

  case 1047:
#line 6546 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("similar_escape");
					n->args = list_make2(yyvsp[-2].node, yyvsp[0].node);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "~", yyvsp[-5].node, (Node *) n);
				}
    break;

  case 1048:
#line 6555 "gram.y"
    {
					A_Const *c = makeNode(A_Const);
					FuncCall *n = makeNode(FuncCall);
					c->val.type = T_Null;
					n->funcname = SystemFuncName("similar_escape");
					n->args = list_make2(yyvsp[0].node, (Node *) c);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "!~", yyvsp[-4].node, (Node *) n);
				}
    break;

  case 1049:
#line 6566 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("similar_escape");
					n->args = list_make2(yyvsp[-2].node, yyvsp[0].node);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "!~", yyvsp[-6].node, (Node *) n);
				}
    break;

  case 1050:
#line 6585 "gram.y"
    {
					if (IsA(yyvsp[-1].node, RowExpr))
						yyval.node = makeRowNullTest(IS_NULL, (RowExpr *) yyvsp[-1].node);
					else
					{
						NullTest *n = makeNode(NullTest);
						n->arg = (Expr *) yyvsp[-1].node;
						n->nulltesttype = IS_NULL;
						yyval.node = (Node *)n;
					}
				}
    break;

  case 1051:
#line 6597 "gram.y"
    {
					if (IsA(yyvsp[-2].node, RowExpr))
						yyval.node = makeRowNullTest(IS_NULL, (RowExpr *) yyvsp[-2].node);
					else
					{
						NullTest *n = makeNode(NullTest);
						n->arg = (Expr *) yyvsp[-2].node;
						n->nulltesttype = IS_NULL;
						yyval.node = (Node *)n;
					}
				}
    break;

  case 1052:
#line 6609 "gram.y"
    {
					if (IsA(yyvsp[-1].node, RowExpr))
						yyval.node = makeRowNullTest(IS_NOT_NULL, (RowExpr *) yyvsp[-1].node);
					else
					{
						NullTest *n = makeNode(NullTest);
						n->arg = (Expr *) yyvsp[-1].node;
						n->nulltesttype = IS_NOT_NULL;
						yyval.node = (Node *)n;
					}
				}
    break;

  case 1053:
#line 6621 "gram.y"
    {
					if (IsA(yyvsp[-3].node, RowExpr))
						yyval.node = makeRowNullTest(IS_NOT_NULL, (RowExpr *) yyvsp[-3].node);
					else
					{
						NullTest *n = makeNode(NullTest);
						n->arg = (Expr *) yyvsp[-3].node;
						n->nulltesttype = IS_NOT_NULL;
						yyval.node = (Node *)n;
					}
				}
    break;

  case 1054:
#line 6633 "gram.y"
    {
					yyval.node = (Node *)makeOverlaps(yyvsp[-2].list, yyvsp[0].list);
				}
    break;

  case 1055:
#line 6637 "gram.y"
    {
					BooleanTest *b = makeNode(BooleanTest);
					b->arg = (Expr *) yyvsp[-2].node;
					b->booltesttype = IS_TRUE;
					yyval.node = (Node *)b;
				}
    break;

  case 1056:
#line 6644 "gram.y"
    {
					BooleanTest *b = makeNode(BooleanTest);
					b->arg = (Expr *) yyvsp[-3].node;
					b->booltesttype = IS_NOT_TRUE;
					yyval.node = (Node *)b;
				}
    break;

  case 1057:
#line 6651 "gram.y"
    {
					BooleanTest *b = makeNode(BooleanTest);
					b->arg = (Expr *) yyvsp[-2].node;
					b->booltesttype = IS_FALSE;
					yyval.node = (Node *)b;
				}
    break;

  case 1058:
#line 6658 "gram.y"
    {
					BooleanTest *b = makeNode(BooleanTest);
					b->arg = (Expr *) yyvsp[-3].node;
					b->booltesttype = IS_NOT_FALSE;
					yyval.node = (Node *)b;
				}
    break;

  case 1059:
#line 6665 "gram.y"
    {
					BooleanTest *b = makeNode(BooleanTest);
					b->arg = (Expr *) yyvsp[-2].node;
					b->booltesttype = IS_UNKNOWN;
					yyval.node = (Node *)b;
				}
    break;

  case 1060:
#line 6672 "gram.y"
    {
					BooleanTest *b = makeNode(BooleanTest);
					b->arg = (Expr *) yyvsp[-3].node;
					b->booltesttype = IS_NOT_UNKNOWN;
					yyval.node = (Node *)b;
				}
    break;

  case 1061:
#line 6679 "gram.y"
    {
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_DISTINCT, "=", yyvsp[-4].node, yyvsp[0].node);
				}
    break;

  case 1062:
#line 6683 "gram.y"
    {
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OF, "=", yyvsp[-5].node, (Node *) yyvsp[-1].list);
				}
    break;

  case 1063:
#line 6687 "gram.y"
    {
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OF, "!=", yyvsp[-6].node, (Node *) yyvsp[-1].list);
				}
    break;

  case 1064:
#line 6691 "gram.y"
    {
					yyval.node = (Node *) makeA_Expr(AEXPR_AND, NIL,
						(Node *) makeSimpleA_Expr(AEXPR_OP, ">=", yyvsp[-5].node, yyvsp[-2].node),
						(Node *) makeSimpleA_Expr(AEXPR_OP, "<=", yyvsp[-5].node, yyvsp[0].node));
				}
    break;

  case 1065:
#line 6697 "gram.y"
    {
					yyval.node = (Node *) makeA_Expr(AEXPR_OR, NIL,
						(Node *) makeSimpleA_Expr(AEXPR_OP, "<", yyvsp[-6].node, yyvsp[-2].node),
						(Node *) makeSimpleA_Expr(AEXPR_OP, ">", yyvsp[-6].node, yyvsp[0].node));
				}
    break;

  case 1066:
#line 6703 "gram.y"
    {
					yyval.node = (Node *) makeA_Expr(AEXPR_OR, NIL,
						(Node *) makeA_Expr(AEXPR_AND, NIL,
						    (Node *) makeSimpleA_Expr(AEXPR_OP, ">=", yyvsp[-5].node, yyvsp[-2].node),
						    (Node *) makeSimpleA_Expr(AEXPR_OP, "<=", yyvsp[-5].node, yyvsp[0].node)),
						(Node *) makeA_Expr(AEXPR_AND, NIL,
						    (Node *) makeSimpleA_Expr(AEXPR_OP, ">=", yyvsp[-5].node, yyvsp[0].node),
						    (Node *) makeSimpleA_Expr(AEXPR_OP, "<=", yyvsp[-5].node, yyvsp[-2].node)));
				}
    break;

  case 1067:
#line 6713 "gram.y"
    {
					yyval.node = (Node *) makeA_Expr(AEXPR_AND, NIL,
						(Node *) makeA_Expr(AEXPR_OR, NIL,
						    (Node *) makeSimpleA_Expr(AEXPR_OP, "<", yyvsp[-6].node, yyvsp[-2].node),
						    (Node *) makeSimpleA_Expr(AEXPR_OP, ">", yyvsp[-6].node, yyvsp[0].node)),
						(Node *) makeA_Expr(AEXPR_OR, NIL,
						    (Node *) makeSimpleA_Expr(AEXPR_OP, "<", yyvsp[-6].node, yyvsp[0].node),
						    (Node *) makeSimpleA_Expr(AEXPR_OP, ">", yyvsp[-6].node, yyvsp[-2].node)));
				}
    break;

  case 1068:
#line 6723 "gram.y"
    {
					/* in_expr returns a SubLink or a list of a_exprs */
					if (IsA(yyvsp[0].node, SubLink))
					{
							SubLink *n = (SubLink *)yyvsp[0].node;
							n->subLinkType = ANY_SUBLINK;
							if (IsA(yyvsp[-2].node, RowExpr))
								n->lefthand = ((RowExpr *) yyvsp[-2].node)->args;
							else
								n->lefthand = list_make1(yyvsp[-2].node);
							n->operName = list_make1(makeString("="));
							yyval.node = (Node *)n;
					}
					else
					{
						Node *n = NULL;
						ListCell *l;
						foreach(l, (List *) yyvsp[0].node)
						{
							Node *cmp;
							cmp = (Node *) makeSimpleA_Expr(AEXPR_OP, "=", yyvsp[-2].node, lfirst(l));
							if (n == NULL)
								n = cmp;
							else
								n = (Node *) makeA_Expr(AEXPR_OR, NIL, n, cmp);
						}
						yyval.node = n;
					}
				}
    break;

  case 1069:
#line 6753 "gram.y"
    {
					/* in_expr returns a SubLink or a list of a_exprs */
					if (IsA(yyvsp[0].node, SubLink))
					{
						/* Make an IN node */
						SubLink *n = (SubLink *)yyvsp[0].node;
						n->subLinkType = ANY_SUBLINK;
						if (IsA(yyvsp[-3].node, RowExpr))
							n->lefthand = ((RowExpr *) yyvsp[-3].node)->args;
						else
							n->lefthand = list_make1(yyvsp[-3].node);
						n->operName = list_make1(makeString("="));
						/* Stick a NOT on top */
						yyval.node = (Node *) makeA_Expr(AEXPR_NOT, NIL, NULL, (Node *) n);
					}
					else
					{
						Node *n = NULL;
						ListCell *l;
						foreach(l, (List *) yyvsp[0].node)
						{
							Node *cmp;
							cmp = (Node *) makeSimpleA_Expr(AEXPR_OP, "<>", yyvsp[-3].node, lfirst(l));
							if (n == NULL)
								n = cmp;
							else
								n = (Node *) makeA_Expr(AEXPR_AND, NIL, n, cmp);
						}
						yyval.node = n;
					}
				}
    break;

  case 1070:
#line 6785 "gram.y"
    {
					SubLink *n = makeNode(SubLink);
					n->subLinkType = yyvsp[-1].ival;
					if (IsA(yyvsp[-3].node, RowExpr))
						n->lefthand = ((RowExpr *) yyvsp[-3].node)->args;
					else
						n->lefthand = list_make1(yyvsp[-3].node);
					n->operName = yyvsp[-2].list;
					n->subselect = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 1071:
#line 6797 "gram.y"
    {
					if (yyvsp[-3].ival == ANY_SUBLINK)
						yyval.node = (Node *) makeA_Expr(AEXPR_OP_ANY, yyvsp[-4].list, yyvsp[-5].node, yyvsp[-1].node);
					else
						yyval.node = (Node *) makeA_Expr(AEXPR_OP_ALL, yyvsp[-4].list, yyvsp[-5].node, yyvsp[-1].node);
				}
    break;

  case 1072:
#line 6804 "gram.y"
    {
					/* Not sure how to get rid of the parentheses
					 * but there are lots of shift/reduce errors without them.
					 *
					 * Should be able to implement this by plopping the entire
					 * select into a node, then transforming the target expressions
					 * from whatever they are into count(*), and testing the
					 * entire result equal to one.
					 * But, will probably implement a separate node in the executor.
					 */
					ereport(ERROR,
							(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
							 errmsg("UNIQUE predicate is not yet implemented")));
				}
    break;

  case 1073:
#line 6830 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1074:
#line 6832 "gram.y"
    { yyval.node = makeTypeCast(yyvsp[-2].node, yyvsp[0].typnam); }
    break;

  case 1075:
#line 6834 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "+", NULL, yyvsp[0].node); }
    break;

  case 1076:
#line 6836 "gram.y"
    { yyval.node = doNegate(yyvsp[0].node); }
    break;

  case 1077:
#line 6838 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "+", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1078:
#line 6840 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "-", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1079:
#line 6842 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "*", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1080:
#line 6844 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "/", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1081:
#line 6846 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "%", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1082:
#line 6848 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "^", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1083:
#line 6850 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "<", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1084:
#line 6852 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, ">", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1085:
#line 6854 "gram.y"
    { yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OP, "=", yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1086:
#line 6856 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_OP, yyvsp[-1].list, yyvsp[-2].node, yyvsp[0].node); }
    break;

  case 1087:
#line 6858 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_OP, yyvsp[-1].list, NULL, yyvsp[0].node); }
    break;

  case 1088:
#line 6860 "gram.y"
    { yyval.node = (Node *) makeA_Expr(AEXPR_OP, yyvsp[0].list, yyvsp[-1].node, NULL); }
    break;

  case 1089:
#line 6862 "gram.y"
    {
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_DISTINCT, "=", yyvsp[-4].node, yyvsp[0].node);
				}
    break;

  case 1090:
#line 6866 "gram.y"
    {
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OF, "=", yyvsp[-5].node, (Node *) yyvsp[-1].list);
				}
    break;

  case 1091:
#line 6870 "gram.y"
    {
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_OF, "!=", yyvsp[-6].node, (Node *) yyvsp[-1].list);
				}
    break;

  case 1092:
#line 6883 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1093:
#line 6884 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1094:
#line 6886 "gram.y"
    {
					ParamRef *p = makeNode(ParamRef);
					p->number = yyvsp[-1].ival;
					if (yyvsp[0].list)
					{
						A_Indirection *n = makeNode(A_Indirection);
						n->arg = (Node *) p;
						n->indirection = yyvsp[0].list;
						yyval.node = (Node *) n;
					}
					else
						yyval.node = (Node *) p;
				}
    break;

  case 1095:
#line 6900 "gram.y"
    {
					if (yyvsp[0].list)
					{
						A_Indirection *n = makeNode(A_Indirection);
						n->arg = yyvsp[-2].node;
						n->indirection = yyvsp[0].list;
						yyval.node = (Node *)n;
					}
					else
						yyval.node = yyvsp[-2].node;
				}
    break;

  case 1096:
#line 6912 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1097:
#line 6914 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1098:
#line 6916 "gram.y"
    {
					SubLink *n = makeNode(SubLink);
					n->subLinkType = EXPR_SUBLINK;
					n->lefthand = NIL;
					n->operName = NIL;
					n->subselect = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 1099:
#line 6925 "gram.y"
    {
					SubLink *n = makeNode(SubLink);
					n->subLinkType = EXISTS_SUBLINK;
					n->lefthand = NIL;
					n->operName = NIL;
					n->subselect = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 1100:
#line 6934 "gram.y"
    {
					SubLink *n = makeNode(SubLink);
					n->subLinkType = ARRAY_SUBLINK;
					n->lefthand = NIL;
					n->operName = NIL;
					n->subselect = yyvsp[0].node;
					yyval.node = (Node *)n;
				}
    break;

  case 1101:
#line 6943 "gram.y"
    {	yyval.node = yyvsp[0].node;	}
    break;

  case 1102:
#line 6945 "gram.y"
    {
					RowExpr *r = makeNode(RowExpr);
					r->args = yyvsp[0].list;
					r->row_typeid = 0;	/* not analyzed yet */
					yyval.node = (Node *)r;
				}
    break;

  case 1103:
#line 6962 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = yyvsp[-2].list;
					n->args = NIL;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1104:
#line 6971 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = yyvsp[-3].list;
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1105:
#line 6980 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = yyvsp[-4].list;
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					/* Ideally we'd mark the FuncCall node to indicate
					 * "must be an aggregate", but there's no provision
					 * for that in FuncCall at the moment.
					 */
					yyval.node = (Node *)n;
				}
    break;

  case 1106:
#line 6993 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = yyvsp[-4].list;
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = TRUE;
					yyval.node = (Node *)n;
				}
    break;

  case 1107:
#line 7002 "gram.y"
    {
					/*
					 * For now, we transform AGGREGATE(*) into AGGREGATE(1).
					 *
					 * This does the right thing for COUNT(*) (in fact,
					 * any certainly-non-null expression would do for COUNT),
					 * and there are no other aggregates in SQL92 that accept
					 * '*' as parameter.
					 *
					 * The FuncCall node is also marked agg_star = true,
					 * so that later processing can detect what the argument
					 * really was.
					 */
					FuncCall *n = makeNode(FuncCall);
					A_Const *star = makeNode(A_Const);

					star->val.type = T_Integer;
					star->val.val.ival = 1;
					n->funcname = yyvsp[-3].list;
					n->args = list_make1(star);
					n->agg_star = TRUE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1108:
#line 7027 "gram.y"
    {
					/*
					 * Translate as "'now'::text::date".
					 *
					 * We cannot use "'now'::date" because coerce_type() will
					 * immediately reduce that to a constant representing
					 * today's date.  We need to delay the conversion until
					 * runtime, else the wrong things will happen when
					 * CURRENT_DATE is used in a column default value or rule.
					 *
					 * This could be simplified if we had a way to generate
					 * an expression tree representing runtime application
					 * of type-input conversion functions.  (As of PG 7.3
					 * that is actually possible, but not clear that we want
					 * to rely on it.)
					 */
					A_Const *s = makeNode(A_Const);
					TypeName *d;

					s->val.type = T_String;
					s->val.val.str = "now";
					s->typename = SystemTypeName("text");

					d = SystemTypeName("date");

					yyval.node = (Node *)makeTypeCast((Node *)s, d);
				}
    break;

  case 1109:
#line 7055 "gram.y"
    {
					/*
					 * Translate as "'now'::text::timetz".
					 * See comments for CURRENT_DATE.
					 */
					A_Const *s = makeNode(A_Const);
					TypeName *d;

					s->val.type = T_String;
					s->val.val.str = "now";
					s->typename = SystemTypeName("text");

					d = SystemTypeName("timetz");

					yyval.node = (Node *)makeTypeCast((Node *)s, d);
				}
    break;

  case 1110:
#line 7072 "gram.y"
    {
					/*
					 * Translate as "'now'::text::timetz(n)".
					 * See comments for CURRENT_DATE.
					 */
					A_Const *s = makeNode(A_Const);
					TypeName *d;

					s->val.type = T_String;
					s->val.val.str = "now";
					s->typename = SystemTypeName("text");
					d = SystemTypeName("timetz");
					if (yyvsp[-1].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("CURRENT_TIME(%d) precision must not be negative",
										yyvsp[-1].ival)));
					if (yyvsp[-1].ival > MAX_TIME_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("CURRENT_TIME(%d) precision reduced to maximum allowed, %d",
										yyvsp[-1].ival, MAX_TIME_PRECISION)));
						yyvsp[-1].ival = MAX_TIME_PRECISION;
					}
					d->typmod = yyvsp[-1].ival;

					yyval.node = (Node *)makeTypeCast((Node *)s, d);
				}
    break;

  case 1111:
#line 7102 "gram.y"
    {
					/*
					 * Translate as "now()", since we have a function that
					 * does exactly what is needed.
					 */
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("now");
					n->args = NIL;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1112:
#line 7115 "gram.y"
    {
					/*
					 * Translate as "'now'::text::timestamptz(n)".
					 * See comments for CURRENT_DATE.
					 */
					A_Const *s = makeNode(A_Const);
					TypeName *d;

					s->val.type = T_String;
					s->val.val.str = "now";
					s->typename = SystemTypeName("text");

					d = SystemTypeName("timestamptz");
					if (yyvsp[-1].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("CURRENT_TIMESTAMP(%d) precision must not be negative",
										yyvsp[-1].ival)));
					if (yyvsp[-1].ival > MAX_TIMESTAMP_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("CURRENT_TIMESTAMP(%d) precision reduced to maximum allowed, %d",
										yyvsp[-1].ival, MAX_TIMESTAMP_PRECISION)));
						yyvsp[-1].ival = MAX_TIMESTAMP_PRECISION;
					}
					d->typmod = yyvsp[-1].ival;

					yyval.node = (Node *)makeTypeCast((Node *)s, d);
				}
    break;

  case 1113:
#line 7146 "gram.y"
    {
					/*
					 * Translate as "'now'::text::time".
					 * See comments for CURRENT_DATE.
					 */
					A_Const *s = makeNode(A_Const);
					TypeName *d;

					s->val.type = T_String;
					s->val.val.str = "now";
					s->typename = SystemTypeName("text");

					d = SystemTypeName("time");

					yyval.node = (Node *)makeTypeCast((Node *)s, d);
				}
    break;

  case 1114:
#line 7163 "gram.y"
    {
					/*
					 * Translate as "'now'::text::time(n)".
					 * See comments for CURRENT_DATE.
					 */
					A_Const *s = makeNode(A_Const);
					TypeName *d;

					s->val.type = T_String;
					s->val.val.str = "now";
					s->typename = SystemTypeName("text");
					d = SystemTypeName("time");
					if (yyvsp[-1].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("LOCALTIME(%d) precision must not be negative",
										yyvsp[-1].ival)));
					if (yyvsp[-1].ival > MAX_TIME_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("LOCALTIME(%d) precision reduced to maximum allowed, %d",
										yyvsp[-1].ival, MAX_TIME_PRECISION)));
						yyvsp[-1].ival = MAX_TIME_PRECISION;
					}
					d->typmod = yyvsp[-1].ival;

					yyval.node = (Node *)makeTypeCast((Node *)s, d);
				}
    break;

  case 1115:
#line 7193 "gram.y"
    {
					/*
					 * Translate as "'now'::text::timestamp".
					 * See comments for CURRENT_DATE.
					 */
					A_Const *s = makeNode(A_Const);
					TypeName *d;

					s->val.type = T_String;
					s->val.val.str = "now";
					s->typename = SystemTypeName("text");

					d = SystemTypeName("timestamp");

					yyval.node = (Node *)makeTypeCast((Node *)s, d);
				}
    break;

  case 1116:
#line 7210 "gram.y"
    {
					/*
					 * Translate as "'now'::text::timestamp(n)".
					 * See comments for CURRENT_DATE.
					 */
					A_Const *s = makeNode(A_Const);
					TypeName *d;

					s->val.type = T_String;
					s->val.val.str = "now";
					s->typename = SystemTypeName("text");

					d = SystemTypeName("timestamp");
					if (yyvsp[-1].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("LOCALTIMESTAMP(%d) precision must not be negative",
										yyvsp[-1].ival)));
					if (yyvsp[-1].ival > MAX_TIMESTAMP_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("LOCALTIMESTAMP(%d) precision reduced to maximum allowed, %d",
										yyvsp[-1].ival, MAX_TIMESTAMP_PRECISION)));
						yyvsp[-1].ival = MAX_TIMESTAMP_PRECISION;
					}
					d->typmod = yyvsp[-1].ival;

					yyval.node = (Node *)makeTypeCast((Node *)s, d);
				}
    break;

  case 1117:
#line 7241 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("current_user");
					n->args = NIL;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1118:
#line 7250 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("current_user");
					n->args = NIL;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1119:
#line 7259 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("session_user");
					n->args = NIL;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1120:
#line 7268 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("current_user");
					n->args = NIL;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1121:
#line 7277 "gram.y"
    { yyval.node = makeTypeCast(yyvsp[-3].node, yyvsp[-1].typnam); }
    break;

  case 1122:
#line 7279 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("date_part");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1123:
#line 7288 "gram.y"
    {
					/* overlay(A PLACING B FROM C FOR D) is converted to
					 * substring(A, 1, C-1) || B || substring(A, C+1, C+D)
					 * overlay(A PLACING B FROM C) is converted to
					 * substring(A, 1, C-1) || B || substring(A, C+1, C+char_length(B))
					 */
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("overlay");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1124:
#line 7302 "gram.y"
    {
					/* position(A in B) is converted to position(B, A) */
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("position");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1125:
#line 7312 "gram.y"
    {
					/* substring(A from B for C) is converted to
					 * substring(A, B, C) - thomas 2000-11-28
					 */
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("substring");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1126:
#line 7324 "gram.y"
    {
					/* TREAT(expr AS target) converts expr of a particular type to target,
					 * which is defined to be a subtype of the original expression.
					 * In SQL99, this is intended for use with structured UDTs,
					 * but let's make this a generally useful form allowing stronger
					 * coersions than are handled by implicit casting.
					 */
					FuncCall *n = makeNode(FuncCall);
					/* Convert SystemTypeName() to SystemFuncName() even though
					 * at the moment they result in the same thing.
					 */
					n->funcname = SystemFuncName(((Value *)llast(yyvsp[-1].typnam->names))->val.str);
					n->args = list_make1(yyvsp[-3].node);
					yyval.node = (Node *)n;
				}
    break;

  case 1127:
#line 7340 "gram.y"
    {
					/* various trim expressions are defined in SQL92
					 * - thomas 1997-07-19
					 */
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("btrim");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1128:
#line 7352 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("ltrim");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1129:
#line 7361 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("rtrim");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1130:
#line 7370 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("btrim");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1131:
#line 7379 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					A_Const *c = makeNode(A_Const);

					c->val.type = T_String;
					c->val.val.str = NameListToQuotedString(yyvsp[-1].list);

					n->funcname = SystemFuncName("convert_using");
					n->args = list_make2(yyvsp[-3].node, c);
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1132:
#line 7393 "gram.y"
    {
					FuncCall *n = makeNode(FuncCall);
					n->funcname = SystemFuncName("convert");
					n->args = yyvsp[-1].list;
					n->agg_star = FALSE;
					n->agg_distinct = FALSE;
					yyval.node = (Node *)n;
				}
    break;

  case 1133:
#line 7402 "gram.y"
    {
					yyval.node = (Node *) makeSimpleA_Expr(AEXPR_NULLIF, "=", yyvsp[-3].node, yyvsp[-1].node);
				}
    break;

  case 1134:
#line 7406 "gram.y"
    {
					CoalesceExpr *c = makeNode(CoalesceExpr);
					c->args = yyvsp[-1].list;
					yyval.node = (Node *)c;
				}
    break;

  case 1135:
#line 7412 "gram.y"
    {
					MinMaxExpr *v = makeNode(MinMaxExpr);
					v->args = yyvsp[-1].list;
					v->op = IS_GREATEST;
					yyval.node = (Node *)v;
				}
    break;

  case 1136:
#line 7419 "gram.y"
    {
					MinMaxExpr *v = makeNode(MinMaxExpr);
					v->args = yyvsp[-1].list;
					v->op = IS_LEAST;
					yyval.node = (Node *)v;
				}
    break;

  case 1137:
#line 7437 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 1138:
#line 7438 "gram.y"
    { yyval.list = NIL; }
    break;

  case 1139:
#line 7439 "gram.y"
    { yyval.list = lappend(yyvsp[-3].list, yyvsp[-1].node); }
    break;

  case 1140:
#line 7442 "gram.y"
    { yyval.ival = ANY_SUBLINK; }
    break;

  case 1141:
#line 7443 "gram.y"
    { yyval.ival = ANY_SUBLINK; }
    break;

  case 1142:
#line 7444 "gram.y"
    { yyval.ival = ALL_SUBLINK; }
    break;

  case 1143:
#line 7447 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1144:
#line 7448 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1145:
#line 7451 "gram.y"
    { yyval.str = "+"; }
    break;

  case 1146:
#line 7452 "gram.y"
    { yyval.str = "-"; }
    break;

  case 1147:
#line 7453 "gram.y"
    { yyval.str = "*"; }
    break;

  case 1148:
#line 7454 "gram.y"
    { yyval.str = "/"; }
    break;

  case 1149:
#line 7455 "gram.y"
    { yyval.str = "%"; }
    break;

  case 1150:
#line 7456 "gram.y"
    { yyval.str = "^"; }
    break;

  case 1151:
#line 7457 "gram.y"
    { yyval.str = "<"; }
    break;

  case 1152:
#line 7458 "gram.y"
    { yyval.str = ">"; }
    break;

  case 1153:
#line 7459 "gram.y"
    { yyval.str = "="; }
    break;

  case 1154:
#line 7463 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 1155:
#line 7465 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 1156:
#line 7470 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 1157:
#line 7472 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 1158:
#line 7477 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 1159:
#line 7479 "gram.y"
    { yyval.list = yyvsp[-1].list; }
    break;

  case 1160:
#line 7481 "gram.y"
    { yyval.list = list_make1(makeString("~~")); }
    break;

  case 1161:
#line 7483 "gram.y"
    { yyval.list = list_make1(makeString("!~~")); }
    break;

  case 1162:
#line 7485 "gram.y"
    { yyval.list = list_make1(makeString("~~*")); }
    break;

  case 1163:
#line 7487 "gram.y"
    { yyval.list = list_make1(makeString("!~~*")); }
    break;

  case 1164:
#line 7499 "gram.y"
    {
					yyval.list = list_make1(yyvsp[0].node);
				}
    break;

  case 1165:
#line 7503 "gram.y"
    {
					yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node);
				}
    break;

  case 1166:
#line 7510 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->val.type = T_String;
					n->val.val.str = yyvsp[-2].str;
					yyval.list = list_make2((Node *) n, yyvsp[0].node);
				}
    break;

  case 1167:
#line 7516 "gram.y"
    { yyval.list = NIL; }
    break;

  case 1168:
#line 7520 "gram.y"
    {
					yyval.list = lappend(yyvsp[-2].list, yyvsp[0].typnam);
				}
    break;

  case 1169:
#line 7524 "gram.y"
    {
					yyval.list = list_make1(yyvsp[0].typnam);
				}
    break;

  case 1170:
#line 7530 "gram.y"
    {	yyval.list = list_make1(yyvsp[0].node);		}
    break;

  case 1171:
#line 7532 "gram.y"
    {	yyval.list = lappend(yyvsp[-2].list, yyvsp[0].node);	}
    break;

  case 1172:
#line 7536 "gram.y"
    {
					ArrayExpr *n = makeNode(ArrayExpr);
					n->elements = yyvsp[-1].list;
					yyval.node = (Node *)n;
				}
    break;

  case 1173:
#line 7542 "gram.y"
    {
					ArrayExpr *n = makeNode(ArrayExpr);
					n->elements = yyvsp[-1].list;
					yyval.node = (Node *)n;
				}
    break;

  case 1174:
#line 7554 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1175:
#line 7555 "gram.y"
    { yyval.str = "year"; }
    break;

  case 1176:
#line 7556 "gram.y"
    { yyval.str = "month"; }
    break;

  case 1177:
#line 7557 "gram.y"
    { yyval.str = "day"; }
    break;

  case 1178:
#line 7558 "gram.y"
    { yyval.str = "hour"; }
    break;

  case 1179:
#line 7559 "gram.y"
    { yyval.str = "minute"; }
    break;

  case 1180:
#line 7560 "gram.y"
    { yyval.str = "second"; }
    break;

  case 1181:
#line 7561 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1182:
#line 7571 "gram.y"
    {
					yyval.list = list_make4(yyvsp[-3].node, yyvsp[-2].node, yyvsp[-1].node, yyvsp[0].node);
				}
    break;

  case 1183:
#line 7575 "gram.y"
    {
					yyval.list = list_make3(yyvsp[-2].node, yyvsp[-1].node, yyvsp[0].node);
				}
    break;

  case 1184:
#line 7582 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1185:
#line 7588 "gram.y"
    { yyval.list = list_make2(yyvsp[0].node, yyvsp[-2].node); }
    break;

  case 1186:
#line 7589 "gram.y"
    { yyval.list = NIL; }
    break;

  case 1187:
#line 7606 "gram.y"
    {
					yyval.list = list_make3(yyvsp[-2].node, yyvsp[-1].node, yyvsp[0].node);
				}
    break;

  case 1188:
#line 7610 "gram.y"
    {
					/* not legal per SQL99, but might as well allow it */
					yyval.list = list_make3(yyvsp[-2].node, yyvsp[0].node, yyvsp[-1].node);
				}
    break;

  case 1189:
#line 7615 "gram.y"
    {
					yyval.list = list_make2(yyvsp[-1].node, yyvsp[0].node);
				}
    break;

  case 1190:
#line 7619 "gram.y"
    {
					/*
					 * Since there are no cases where this syntax allows
					 * a textual FOR value, we forcibly cast the argument
					 * to int4.  This is a kluge to avoid surprising results
					 * when the argument is, say, int8.  It'd be better if
					 * there were not an implicit cast from int8 to text ...
					 */
					A_Const *n = makeNode(A_Const);
					n->val.type = T_Integer;
					n->val.val.ival = 1;
					yyval.list = list_make3(yyvsp[-1].node, (Node *) n,
									makeTypeCast(yyvsp[0].node, SystemTypeName("int4")));
				}
    break;

  case 1191:
#line 7634 "gram.y"
    {
					yyval.list = yyvsp[0].list;
				}
    break;

  case 1192:
#line 7638 "gram.y"
    { yyval.list = NIL; }
    break;

  case 1193:
#line 7642 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1194:
#line 7645 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1195:
#line 7648 "gram.y"
    { yyval.list = lappend(yyvsp[0].list, yyvsp[-2].node); }
    break;

  case 1196:
#line 7649 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 1197:
#line 7650 "gram.y"
    { yyval.list = yyvsp[0].list; }
    break;

  case 1198:
#line 7654 "gram.y"
    {
					SubLink *n = makeNode(SubLink);
					n->subselect = yyvsp[0].node;
					/* other fields will be filled later */
					yyval.node = (Node *)n;
				}
    break;

  case 1199:
#line 7660 "gram.y"
    { yyval.node = (Node *)yyvsp[-1].list; }
    break;

  case 1200:
#line 7671 "gram.y"
    {
					CaseExpr *c = makeNode(CaseExpr);
					c->casetype = 0; /* not analyzed yet */
					c->arg = (Expr *) yyvsp[-3].node;
					c->args = yyvsp[-2].list;
					c->defresult = (Expr *) yyvsp[-1].node;
					yyval.node = (Node *)c;
				}
    break;

  case 1201:
#line 7683 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 1202:
#line 7684 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].node); }
    break;

  case 1203:
#line 7689 "gram.y"
    {
					CaseWhen *w = makeNode(CaseWhen);
					w->expr = (Expr *) yyvsp[-2].node;
					w->result = (Expr *) yyvsp[0].node;
					yyval.node = (Node *)w;
				}
    break;

  case 1204:
#line 7698 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1205:
#line 7699 "gram.y"
    { yyval.node = NULL; }
    break;

  case 1206:
#line 7702 "gram.y"
    { yyval.node = yyvsp[0].node; }
    break;

  case 1207:
#line 7703 "gram.y"
    { yyval.node = NULL; }
    break;

  case 1208:
#line 7712 "gram.y"
    {
					yyval.node = makeColumnRef(yyvsp[0].str, NIL);
				}
    break;

  case 1209:
#line 7716 "gram.y"
    {
					yyval.node = makeColumnRef(yyvsp[-1].str, yyvsp[0].list);
				}
    break;

  case 1210:
#line 7723 "gram.y"
    {
					yyval.node = (Node *) makeString(yyvsp[0].str);
				}
    break;

  case 1211:
#line 7727 "gram.y"
    {
					yyval.node = (Node *) makeString("*");
				}
    break;

  case 1212:
#line 7731 "gram.y"
    {
					A_Indices *ai = makeNode(A_Indices);
					ai->lidx = NULL;
					ai->uidx = yyvsp[-1].node;
					yyval.node = (Node *) ai;
				}
    break;

  case 1213:
#line 7738 "gram.y"
    {
					A_Indices *ai = makeNode(A_Indices);
					ai->lidx = yyvsp[-3].node;
					ai->uidx = yyvsp[-1].node;
					yyval.node = (Node *) ai;
				}
    break;

  case 1214:
#line 7747 "gram.y"
    { yyval.list = list_make1(yyvsp[0].node); }
    break;

  case 1215:
#line 7748 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].node); }
    break;

  case 1216:
#line 7752 "gram.y"
    { yyval.list = NIL; }
    break;

  case 1217:
#line 7753 "gram.y"
    { yyval.list = lappend(yyvsp[-1].list, yyvsp[0].node); }
    break;

  case 1220:
#line 7768 "gram.y"
    { yyval.list = list_make1(yyvsp[0].target); }
    break;

  case 1221:
#line 7769 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].target); }
    break;

  case 1222:
#line 7774 "gram.y"
    {
					yyval.target = makeNode(ResTarget);
					yyval.target->name = yyvsp[0].str;
					yyval.target->indirection = NIL;
					yyval.target->val = (Node *)yyvsp[-2].node;
				}
    break;

  case 1223:
#line 7781 "gram.y"
    {
					yyval.target = makeNode(ResTarget);
					yyval.target->name = NULL;
					yyval.target->indirection = NIL;
					yyval.target->val = (Node *)yyvsp[0].node;
				}
    break;

  case 1224:
#line 7788 "gram.y"
    {
					ColumnRef *n = makeNode(ColumnRef);
					n->fields = list_make1(makeString("*"));

					yyval.target = makeNode(ResTarget);
					yyval.target->name = NULL;
					yyval.target->indirection = NIL;
					yyval.target->val = (Node *)n;
				}
    break;

  case 1225:
#line 7800 "gram.y"
    { yyval.list = list_make1(yyvsp[0].target); }
    break;

  case 1226:
#line 7801 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list,yyvsp[0].target); }
    break;

  case 1227:
#line 7806 "gram.y"
    {
					yyval.target = makeNode(ResTarget);
					yyval.target->name = yyvsp[-3].str;
					yyval.target->indirection = yyvsp[-2].list;
					yyval.target->val = (Node *) yyvsp[0].node;
				}
    break;

  case 1228:
#line 7813 "gram.y"
    {
					yyval.target = makeNode(ResTarget);
					yyval.target->name = yyvsp[-3].str;
					yyval.target->indirection = yyvsp[-2].list;
					yyval.target->val = (Node *) makeNode(SetToDefault);
				}
    break;

  case 1229:
#line 7823 "gram.y"
    { yyval.list = list_make1(yyvsp[0].target); }
    break;

  case 1230:
#line 7824 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].target); }
    break;

  case 1231:
#line 7829 "gram.y"
    {
					yyval.target = makeNode(ResTarget);
					yyval.target->name = NULL;
					yyval.target->indirection = NIL;
					yyval.target->val = (Node *)yyvsp[0].node;
				}
    break;

  case 1232:
#line 7836 "gram.y"
    {
					yyval.target = makeNode(ResTarget);
					yyval.target->name = NULL;
					yyval.target->indirection = NIL;
					yyval.target->val = (Node *) makeNode(SetToDefault);
				}
    break;

  case 1233:
#line 7852 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1234:
#line 7853 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1235:
#line 7857 "gram.y"
    { yyval.list = list_make1(yyvsp[0].range); }
    break;

  case 1236:
#line 7858 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, yyvsp[0].range); }
    break;

  case 1237:
#line 7870 "gram.y"
    {
					yyval.range = makeNode(RangeVar);
					yyval.range->catalogname = NULL;
					yyval.range->schemaname = NULL;
					yyval.range->relname = yyvsp[0].str;
				}
    break;

  case 1238:
#line 7877 "gram.y"
    {
					check_qualified_name(yyvsp[0].list);
					yyval.range = makeNode(RangeVar);
					switch (list_length(yyvsp[0].list))
					{
						case 1:
							yyval.range->catalogname = NULL;
							yyval.range->schemaname = yyvsp[-1].str;
							yyval.range->relname = strVal(linitial(yyvsp[0].list));
							break;
						case 2:
							yyval.range->catalogname = yyvsp[-1].str;
							yyval.range->schemaname = strVal(linitial(yyvsp[0].list));
							yyval.range->relname = strVal(lsecond(yyvsp[0].list));
							break;
						default:
							ereport(ERROR,
									(errcode(ERRCODE_SYNTAX_ERROR),
									 errmsg("improper qualified name (too many dotted names): %s",
											NameListToString(lcons(makeString(yyvsp[-1].str), yyvsp[0].list)))));
							break;
					}
				}
    break;

  case 1239:
#line 7903 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 1240:
#line 7905 "gram.y"
    { yyval.list = lappend(yyvsp[-2].list, makeString(yyvsp[0].str)); }
    break;

  case 1241:
#line 7909 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1242:
#line 7912 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1243:
#line 7915 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1244:
#line 7917 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1245:
#line 7919 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1246:
#line 7921 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1247:
#line 7932 "gram.y"
    { yyval.list = list_make1(makeString(yyvsp[0].str)); }
    break;

  case 1248:
#line 7934 "gram.y"
    { yyval.list = check_func_name(lcons(makeString(yyvsp[-1].str), yyvsp[0].list)); }
    break;

  case 1249:
#line 7942 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->val.type = T_Integer;
					n->val.val.ival = yyvsp[0].ival;
					yyval.node = (Node *)n;
				}
    break;

  case 1250:
#line 7949 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->val.type = T_Float;
					n->val.val.str = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 1251:
#line 7956 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->val.type = T_String;
					n->val.val.str = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 1252:
#line 7963 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->val.type = T_BitString;
					n->val.val.str = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 1253:
#line 7970 "gram.y"
    {
					/* This is a bit constant per SQL99:
					 * Without Feature F511, "BIT data type",
					 * a <general literal> shall not be a
					 * <bit string literal> or a <hex string literal>.
					 */
					A_Const *n = makeNode(A_Const);
					n->val.type = T_BitString;
					n->val.val.str = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 1254:
#line 7982 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->typename = yyvsp[-1].typnam;
					n->val.type = T_String;
					n->val.val.str = yyvsp[0].str;
					yyval.node = (Node *)n;
				}
    break;

  case 1255:
#line 7990 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->typename = yyvsp[-2].typnam;
					n->val.type = T_String;
					n->val.val.str = yyvsp[-1].str;
					/* precision is not specified, but fields may be... */
					if (yyvsp[0].ival != INTERVAL_FULL_RANGE)
						n->typename->typmod = INTERVAL_TYPMOD(INTERVAL_FULL_PRECISION, yyvsp[0].ival);
					yyval.node = (Node *)n;
				}
    break;

  case 1256:
#line 8001 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->typename = yyvsp[-5].typnam;
					n->val.type = T_String;
					n->val.val.str = yyvsp[-1].str;
					/* precision specified, and fields may be... */
					if (yyvsp[-3].ival < 0)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("INTERVAL(%d) precision must not be negative",
										yyvsp[-3].ival)));
					if (yyvsp[-3].ival > MAX_INTERVAL_PRECISION)
					{
						ereport(WARNING,
								(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
								 errmsg("INTERVAL(%d) precision reduced to maximum allowed, %d",
										yyvsp[-3].ival, MAX_INTERVAL_PRECISION)));
						yyvsp[-3].ival = MAX_INTERVAL_PRECISION;
					}
					n->typename->typmod = INTERVAL_TYPMOD(yyvsp[-3].ival, yyvsp[0].ival);
					yyval.node = (Node *)n;
				}
    break;

  case 1257:
#line 8024 "gram.y"
    {
					yyval.node = (Node *)makeBoolAConst(TRUE);
				}
    break;

  case 1258:
#line 8028 "gram.y"
    {
					yyval.node = (Node *)makeBoolAConst(FALSE);
				}
    break;

  case 1259:
#line 8032 "gram.y"
    {
					A_Const *n = makeNode(A_Const);
					n->val.type = T_Null;
					yyval.node = (Node *)n;
				}
    break;

  case 1260:
#line 8039 "gram.y"
    { yyval.ival = yyvsp[0].ival; }
    break;

  case 1261:
#line 8040 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1262:
#line 8041 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1263:
#line 8043 "gram.y"
    { yyval.ival = yyvsp[0].ival; }
    break;

  case 1264:
#line 8044 "gram.y"
    { yyval.ival = - yyvsp[0].ival; }
    break;

  case 1265:
#line 8060 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1266:
#line 8061 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1267:
#line 8062 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1268:
#line 8067 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1269:
#line 8068 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1270:
#line 8074 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1271:
#line 8075 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1272:
#line 8076 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1273:
#line 8082 "gram.y"
    { yyval.str = yyvsp[0].str; }
    break;

  case 1274:
#line 8083 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1275:
#line 8084 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1276:
#line 8085 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1277:
#line 8086 "gram.y"
    { yyval.str = pstrdup(yyvsp[0].keyword); }
    break;

  case 1612:
#line 8476 "gram.y"
    {
					if (QueryIsRule)
						yyval.str = "*OLD*";
					else
						ereport(ERROR,
								(errcode(ERRCODE_SYNTAX_ERROR),
								 errmsg("OLD used in query that is not in a rule")));
				}
    break;

  case 1613:
#line 8485 "gram.y"
    {
					if (QueryIsRule)
						yyval.str = "*NEW*";
					else
						ereport(ERROR,
								(errcode(ERRCODE_SYNTAX_ERROR),
								 errmsg("NEW used in query that is not in a rule")));
				}
    break;


    }

/* Line 999 of yacc.c.  */
#line 20466 "gram.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 8495 "gram.y"


static Node *
makeColumnRef(char *relname, List *indirection)
{
	/*
	 * Generate a ColumnRef node, with an A_Indirection node added if there
	 * is any subscripting in the specified indirection list.  However,
	 * any field selection at the start of the indirection list must be
	 * transposed into the "fields" part of the ColumnRef node.
	 */
	ColumnRef  *c = makeNode(ColumnRef);
	int		nfields = 0;
	ListCell *l;

	foreach(l, indirection)
	{
		if (IsA(lfirst(l), A_Indices))
		{
			A_Indirection *i = makeNode(A_Indirection);

			if (nfields == 0)
			{
				/* easy case - all indirection goes to A_Indirection */
				c->fields = list_make1(makeString(relname));
				i->indirection = indirection;
			}
			else
			{
				/* got to split the list in two */
				i->indirection = list_copy_tail(indirection, nfields);
				indirection = list_truncate(indirection, nfields);
				c->fields = lcons(makeString(relname), indirection);
			}
			i->arg = (Node *) c;
			return (Node *) i;
		}
		nfields++;
	}
	/* No subscripting, so all indirection gets added to field list */
	c->fields = lcons(makeString(relname), indirection);
	return (Node *) c;
}

static Node *
makeTypeCast(Node *arg, TypeName *typename)
{
	/*
	 * Simply generate a TypeCast node.
	 *
	 * Earlier we would determine whether an A_Const would
	 * be acceptable, however Domains require coerce_type()
	 * to process them -- applying constraints as required.
	 */
	TypeCast *n = makeNode(TypeCast);
	n->arg = arg;
	n->typename = typename;
	return (Node *) n;
}

static Node *
makeStringConst(char *str, TypeName *typename)
{
	A_Const *n = makeNode(A_Const);

	n->val.type = T_String;
	n->val.val.str = str;
	n->typename = typename;

	return (Node *)n;
}

static Node *
makeIntConst(int val)
{
	A_Const *n = makeNode(A_Const);
	n->val.type = T_Integer;
	n->val.val.ival = val;
	n->typename = SystemTypeName("int4");

	return (Node *)n;
}

static Node *
makeFloatConst(char *str)
{
	A_Const *n = makeNode(A_Const);

	n->val.type = T_Float;
	n->val.val.str = str;
	n->typename = SystemTypeName("float8");

	return (Node *)n;
}

static Node *
makeAConst(Value *v)
{
	Node *n;

	switch (v->type)
	{
		case T_Float:
			n = makeFloatConst(v->val.str);
			break;

		case T_Integer:
			n = makeIntConst(v->val.ival);
			break;

		case T_String:
		default:
			n = makeStringConst(v->val.str, NULL);
			break;
	}

	return n;
}

/* makeDefElem()
 * Create a DefElem node and set contents.
 * Could be moved to nodes/makefuncs.c if this is useful elsewhere.
 */
static DefElem *
makeDefElem(char *name, Node *arg)
{
	DefElem *f = makeNode(DefElem);
	f->defname = name;
	f->arg = arg;
	return f;
}

/* makeBoolAConst()
 * Create an A_Const node and initialize to a boolean constant.
 */
static A_Const *
makeBoolAConst(bool state)
{
	A_Const *n = makeNode(A_Const);
	n->val.type = T_String;
	n->val.val.str = (state? "t": "f");
	n->typename = SystemTypeName("bool");
	return n;
}

/* makeRowNullTest()
 * Generate separate operator nodes for a single row descriptor test.
 *
 * Eventually this should be eliminated in favor of making the NullTest
 * node type capable of handling it directly.
 */
static Node *
makeRowNullTest(NullTestType test, RowExpr *row)
{
	Node		*result = NULL;
	ListCell	*arg;

	foreach(arg, row->args)
	{
		NullTest *n;

		n = makeNode(NullTest);
		n->arg = (Expr *) lfirst(arg);
		n->nulltesttype = test;

		if (result == NULL)
			result = (Node *) n;
		else if (test == IS_NOT_NULL)
			result = (Node *) makeA_Expr(AEXPR_OR, NIL, result, (Node *)n);
		else
			result = (Node *) makeA_Expr(AEXPR_AND, NIL, result, (Node *)n);
	}

	if (result == NULL)
	{
		/* zero-length rows?  Generate constant TRUE or FALSE */
		result = (Node *) makeBoolAConst(test == IS_NULL);
	}

	return result;
}

/* makeOverlaps()
 * Create and populate a FuncCall node to support the OVERLAPS operator.
 */
static FuncCall *
makeOverlaps(List *largs, List *rargs)
{
	FuncCall *n = makeNode(FuncCall);
	n->funcname = SystemFuncName("overlaps");
	if (list_length(largs) == 1)
		largs = lappend(largs, largs);
	else if (list_length(largs) != 2)
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("wrong number of parameters on left side of OVERLAPS expression")));
	if (list_length(rargs) == 1)
		rargs = lappend(rargs, rargs);
	else if (list_length(rargs) != 2)
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("wrong number of parameters on right side of OVERLAPS expression")));
	n->args = list_concat(largs, rargs);
	n->agg_star = FALSE;
	n->agg_distinct = FALSE;
	return n;
}

/* check_qualified_name --- check the result of qualified_name production
 *
 * It's easiest to let the grammar production for qualified_name allow
 * subscripts and '*', which we then must reject here.
 */
static void
check_qualified_name(List *names)
{
	ListCell   *i;

	foreach(i, names)
	{
		if (!IsA(lfirst(i), String))
			yyerror("syntax error");
		else if (strcmp(strVal(lfirst(i)), "*") == 0)
			yyerror("syntax error");
	}
}

/* check_func_name --- check the result of func_name production
 *
 * It's easiest to let the grammar production for func_name allow subscripts
 * and '*', which we then must reject here.
 */
static List *
check_func_name(List *names)
{
	ListCell   *i;

	foreach(i, names)
	{
		if (!IsA(lfirst(i), String))
			yyerror("syntax error");
		else if (strcmp(strVal(lfirst(i)), "*") == 0)
			yyerror("syntax error");
	}
	return names;
}

/* extractArgTypes()
 * Given a list of FunctionParameter nodes, extract a list of just the
 * argument types (TypeNames) for input parameters only.  This is what
 * is needed to look up an existing function, which is what is wanted by
 * the productions that use this call.
 */
static List *
extractArgTypes(List *parameters)
{
	List	   *result = NIL;
	ListCell   *i;

	foreach(i, parameters)
	{
		FunctionParameter *p = (FunctionParameter *) lfirst(i);

		if (p->mode != FUNC_PARAM_OUT)			/* keep if IN or INOUT */
			result = lappend(result, p->argType);
	}
	return result;
}

/* findLeftmostSelect()
 * Find the leftmost component SelectStmt in a set-operation parsetree.
 */
static SelectStmt *
findLeftmostSelect(SelectStmt *node)
{
	while (node && node->op != SETOP_NONE)
		node = node->larg;
	return node;
}

/* insertSelectOptions()
 * Insert ORDER BY, etc into an already-constructed SelectStmt.
 *
 * This routine is just to avoid duplicating code in SelectStmt productions.
 */
static void
insertSelectOptions(SelectStmt *stmt,
					List *sortClause, Node *lockingClause,
					Node *limitOffset, Node *limitCount)
{
	/*
	 * Tests here are to reject constructs like
	 *	(SELECT foo ORDER BY bar) ORDER BY baz
	 */
	if (sortClause)
	{
		if (stmt->sortClause)
			ereport(ERROR,
					(errcode(ERRCODE_SYNTAX_ERROR),
					 errmsg("multiple ORDER BY clauses not allowed")));
		stmt->sortClause = sortClause;
	}
	if (lockingClause)
	{
		if (stmt->lockingClause)
			ereport(ERROR,
					(errcode(ERRCODE_SYNTAX_ERROR),
					 errmsg("multiple FOR UPDATE/FOR SHARE clauses not allowed")));
		stmt->lockingClause = (LockingClause *) lockingClause;
	}
	if (limitOffset)
	{
		if (stmt->limitOffset)
			ereport(ERROR,
					(errcode(ERRCODE_SYNTAX_ERROR),
					 errmsg("multiple OFFSET clauses not allowed")));
		stmt->limitOffset = limitOffset;
	}
	if (limitCount)
	{
		if (stmt->limitCount)
			ereport(ERROR,
					(errcode(ERRCODE_SYNTAX_ERROR),
					 errmsg("multiple LIMIT clauses not allowed")));
		stmt->limitCount = limitCount;
	}
}

static Node *
makeSetOp(SetOperation op, bool all, Node *larg, Node *rarg)
{
	SelectStmt *n = makeNode(SelectStmt);

	n->op = op;
	n->all = all;
	n->larg = (SelectStmt *) larg;
	n->rarg = (SelectStmt *) rarg;
	return (Node *) n;
}

/* SystemFuncName()
 * Build a properly-qualified reference to a built-in function.
 */
List *
SystemFuncName(char *name)
{
	return list_make1(makeString(name));
}

/* SystemTypeName()
 * Build a properly-qualified reference to a built-in type.
 *
 * typmod is defaulted, but may be changed afterwards by caller.
 */
TypeName *
SystemTypeName(char *name)
{
	TypeName   *n = makeNode(TypeName);

	n->names = list_make1(makeString(name));
	n->typmod = -1;
	return n;
}

/* parser_init()
 * Initialize to parse one query string
 */
void
parser_init(void)
{
	QueryIsRule = FALSE;
}

/* exprIsNullConstant()
 * Test whether an a_expr is a plain NULL constant or not.
 */
bool
exprIsNullConstant(Node *arg)
{
	if (arg && IsA(arg, A_Const))
	{
		A_Const *con = (A_Const *) arg;

		if (con->val.type == T_Null &&
			con->typename == NULL)
			return TRUE;
	}
	return FALSE;
}

/* doNegate()
 * Handle negation of a numeric constant.
 *
 * Formerly, we did this here because the optimizer couldn't cope with
 * indexquals that looked like "var = -4" --- it wants "var = const"
 * and a unary minus operator applied to a constant didn't qualify.
 * As of Postgres 7.0, that problem doesn't exist anymore because there
 * is a constant-subexpression simplifier in the optimizer.  However,
 * there's still a good reason for doing this here, which is that we can
 * postpone committing to a particular internal representation for simple
 * negative constants.	It's better to leave "-123.456" in string form
 * until we know what the desired type is.
 */
static Node *
doNegate(Node *n)
{
	if (IsA(n, A_Const))
	{
		A_Const *con = (A_Const *)n;

		if (con->val.type == T_Integer)
		{
			con->val.val.ival = -con->val.val.ival;
			return n;
		}
		if (con->val.type == T_Float)
		{
			doNegateFloat(&con->val);
			return n;
		}
	}

	return (Node *) makeSimpleA_Expr(AEXPR_OP, "-", NULL, n);
}

static void
doNegateFloat(Value *v)
{
	char   *oldval = v->val.str;

	if (*oldval == '+')
		oldval++;
	if (*oldval == '-')
		v->val.str = oldval+1;	/* just strip the '-' */
	else
	{
		char   *newval = (char *) palloc(strlen(oldval) + 2);

		*newval = '-';
		strcpy(newval+1, oldval);
		v->val.str = newval;
	}
}

#undef yylex
#include "scan.c"

