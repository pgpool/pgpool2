/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_BASE_YY_GRAM_H_INCLUDED
# define YY_BASE_YY_GRAM_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int base_yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENT = 258,
     FCONST = 259,
     SCONST = 260,
     BCONST = 261,
     XCONST = 262,
     Op = 263,
     ICONST = 264,
     PARAM = 265,
     TYPECAST = 266,
     DOT_DOT = 267,
     COLON_EQUALS = 268,
     EQUALS_GREATER = 269,
     LESS_EQUALS = 270,
     GREATER_EQUALS = 271,
     NOT_EQUALS = 272,
     ABORT_P = 273,
     ABSOLUTE_P = 274,
     ACCESS = 275,
     ACTION = 276,
     ADD_P = 277,
     ADMIN = 278,
     AFTER = 279,
     AGGREGATE = 280,
     ALL = 281,
     ALSO = 282,
     ALTER = 283,
     ALWAYS = 284,
     ANALYSE = 285,
     ANALYZE = 286,
     AND = 287,
     ANY = 288,
     ARRAY = 289,
     AS = 290,
     ASC = 291,
     ASSERTION = 292,
     ASSIGNMENT = 293,
     ASYMMETRIC = 294,
     AT = 295,
     ATTRIBUTE = 296,
     AUTHORIZATION = 297,
     BACKWARD = 298,
     BEFORE = 299,
     BEGIN_P = 300,
     BETWEEN = 301,
     BIGINT = 302,
     BINARY = 303,
     BIT = 304,
     BOOLEAN_P = 305,
     BOTH = 306,
     BY = 307,
     CACHE = 308,
     CALLED = 309,
     CASCADE = 310,
     CASCADED = 311,
     CASE = 312,
     CAST = 313,
     CATALOG_P = 314,
     CHAIN = 315,
     CHAR_P = 316,
     CHARACTER = 317,
     CHARACTERISTICS = 318,
     CHECK = 319,
     CHECKPOINT = 320,
     CLASS = 321,
     CLOSE = 322,
     CLUSTER = 323,
     COALESCE = 324,
     COLLATE = 325,
     COLLATION = 326,
     COLUMN = 327,
     COMMENT = 328,
     COMMENTS = 329,
     COMMIT = 330,
     COMMITTED = 331,
     CONCURRENTLY = 332,
     CONFIGURATION = 333,
     CONFLICT = 334,
     CONNECTION = 335,
     CONSTRAINT = 336,
     CONSTRAINTS = 337,
     CONTENT_P = 338,
     CONTINUE_P = 339,
     CONVERSION_P = 340,
     COPY = 341,
     COST = 342,
     CREATE = 343,
     CROSS = 344,
     CSV = 345,
     CUBE = 346,
     CURRENT_P = 347,
     CURRENT_CATALOG = 348,
     CURRENT_DATE = 349,
     CURRENT_ROLE = 350,
     CURRENT_SCHEMA = 351,
     CURRENT_TIME = 352,
     CURRENT_TIMESTAMP = 353,
     CURRENT_USER = 354,
     CURSOR = 355,
     CYCLE = 356,
     DATA_P = 357,
     DATABASE = 358,
     DAY_P = 359,
     DEALLOCATE = 360,
     DEC = 361,
     DECIMAL_P = 362,
     DECLARE = 363,
     DEFAULT = 364,
     DEFAULTS = 365,
     DEFERRABLE = 366,
     DEFERRED = 367,
     DEFINER = 368,
     DELETE_P = 369,
     DELIMITER = 370,
     DELIMITERS = 371,
     DEPENDS = 372,
     DESC = 373,
     DICTIONARY = 374,
     DISABLE_P = 375,
     DISCARD = 376,
     DISTINCT = 377,
     DO = 378,
     DOCUMENT_P = 379,
     DOMAIN_P = 380,
     DOUBLE_P = 381,
     DROP = 382,
     EACH = 383,
     ELSE = 384,
     ENABLE_P = 385,
     ENCODING = 386,
     ENCRYPTED = 387,
     END_P = 388,
     ENUM_P = 389,
     ESCAPE = 390,
     EVENT = 391,
     EXCEPT = 392,
     EXCLUDE = 393,
     EXCLUDING = 394,
     EXCLUSIVE = 395,
     EXECUTE = 396,
     EXISTS = 397,
     EXPLAIN = 398,
     EXTENSION = 399,
     EXTERNAL = 400,
     EXTRACT = 401,
     FALSE_P = 402,
     FAMILY = 403,
     FETCH = 404,
     FILTER = 405,
     FIRST_P = 406,
     FLOAT_P = 407,
     FOLLOWING = 408,
     FOR = 409,
     FORCE = 410,
     FOREIGN = 411,
     FORWARD = 412,
     FREEZE = 413,
     FROM = 414,
     FULL = 415,
     FUNCTION = 416,
     FUNCTIONS = 417,
     GLOBAL = 418,
     GRANT = 419,
     GRANTED = 420,
     GREATEST = 421,
     GROUP_P = 422,
     GROUPING = 423,
     HANDLER = 424,
     HAVING = 425,
     HEADER_P = 426,
     HOLD = 427,
     HOUR_P = 428,
     IDENTITY_P = 429,
     IF_P = 430,
     ILIKE = 431,
     IMMEDIATE = 432,
     IMMUTABLE = 433,
     IMPLICIT_P = 434,
     IMPORT_P = 435,
     IN_P = 436,
     INCLUDING = 437,
     INCREMENT = 438,
     INDEX = 439,
     INDEXES = 440,
     INHERIT = 441,
     INHERITS = 442,
     INITIALLY = 443,
     INLINE_P = 444,
     INNER_P = 445,
     INOUT = 446,
     INPUT_P = 447,
     INSENSITIVE = 448,
     INSERT = 449,
     INSTEAD = 450,
     INT_P = 451,
     INTEGER = 452,
     INTERSECT = 453,
     INTERVAL = 454,
     INTO = 455,
     INVOKER = 456,
     IS = 457,
     ISNULL = 458,
     ISOLATION = 459,
     JOIN = 460,
     KEY = 461,
     LABEL = 462,
     LANGUAGE = 463,
     LARGE_P = 464,
     LAST_P = 465,
     LATERAL_P = 466,
     LEADING = 467,
     LEAKPROOF = 468,
     LEAST = 469,
     LEFT = 470,
     LEVEL = 471,
     LIKE = 472,
     LIMIT = 473,
     LISTEN = 474,
     LOAD = 475,
     LOCAL = 476,
     LOCALTIME = 477,
     LOCALTIMESTAMP = 478,
     LOCATION = 479,
     LOCK_P = 480,
     LOCKED = 481,
     LOGGED = 482,
     MAPPING = 483,
     MATCH = 484,
     MATERIALIZED = 485,
     MAXVALUE = 486,
     METHOD = 487,
     MINUTE_P = 488,
     MINVALUE = 489,
     MODE = 490,
     MONTH_P = 491,
     MOVE = 492,
     NAME_P = 493,
     NAMES = 494,
     NATIONAL = 495,
     NATURAL = 496,
     NCHAR = 497,
     NEXT = 498,
     NO = 499,
     NONE = 500,
     NOT = 501,
     NOTHING = 502,
     NOTIFY = 503,
     NOTNULL = 504,
     NOWAIT = 505,
     NULL_P = 506,
     NULLIF = 507,
     NULLS_P = 508,
     NUMERIC = 509,
     OBJECT_P = 510,
     OF = 511,
     OFF = 512,
     OFFSET = 513,
     OIDS = 514,
     ON = 515,
     ONLY = 516,
     OPERATOR = 517,
     OPTION = 518,
     OPTIONS = 519,
     OR = 520,
     ORDER = 521,
     ORDINALITY = 522,
     OUT_P = 523,
     OUTER_P = 524,
     OVER = 525,
     OVERLAPS = 526,
     OVERLAY = 527,
     OWNED = 528,
     OWNER = 529,
     PARALLEL = 530,
     PARSER = 531,
     PARTIAL = 532,
     PARTITION = 533,
     PASSING = 534,
     PASSWORD = 535,
     PGPOOL = 536,
     PLACING = 537,
     PLANS = 538,
     POLICY = 539,
     POSITION = 540,
     PRECEDING = 541,
     PRECISION = 542,
     PRESERVE = 543,
     PREPARE = 544,
     PREPARED = 545,
     PRIMARY = 546,
     PRIOR = 547,
     PRIVILEGES = 548,
     PROCEDURAL = 549,
     PROCEDURE = 550,
     PROGRAM = 551,
     QUOTE = 552,
     RANGE = 553,
     READ = 554,
     REAL = 555,
     REASSIGN = 556,
     RECHECK = 557,
     RECURSIVE = 558,
     REF = 559,
     REFERENCES = 560,
     REFRESH = 561,
     REINDEX = 562,
     RELATIVE_P = 563,
     RELEASE = 564,
     RENAME = 565,
     REPEATABLE = 566,
     REPLACE = 567,
     REPLICA = 568,
     RESET = 569,
     RESTART = 570,
     RESTRICT = 571,
     RETURNING = 572,
     RETURNS = 573,
     REVOKE = 574,
     RIGHT = 575,
     ROLE = 576,
     ROLLBACK = 577,
     ROLLUP = 578,
     ROW = 579,
     ROWS = 580,
     RULE = 581,
     SAVEPOINT = 582,
     SCHEMA = 583,
     SCROLL = 584,
     SEARCH = 585,
     SECOND_P = 586,
     SECURITY = 587,
     SELECT = 588,
     SEQUENCE = 589,
     SEQUENCES = 590,
     SERIALIZABLE = 591,
     SERVER = 592,
     SESSION = 593,
     SESSION_USER = 594,
     SET = 595,
     SETS = 596,
     SETOF = 597,
     SHARE = 598,
     SHOW = 599,
     SIMILAR = 600,
     SIMPLE = 601,
     SKIP = 602,
     SMALLINT = 603,
     SNAPSHOT = 604,
     SOME = 605,
     SQL_P = 606,
     STABLE = 607,
     STANDALONE_P = 608,
     START = 609,
     STATEMENT = 610,
     STATISTICS = 611,
     STDIN = 612,
     STDOUT = 613,
     STORAGE = 614,
     STRICT_P = 615,
     STRIP_P = 616,
     SUBSTRING = 617,
     SYMMETRIC = 618,
     SYSID = 619,
     SYSTEM_P = 620,
     TABLE = 621,
     TABLES = 622,
     TABLESAMPLE = 623,
     TABLESPACE = 624,
     TEMP = 625,
     TEMPLATE = 626,
     TEMPORARY = 627,
     TEXT_P = 628,
     THEN = 629,
     TIME = 630,
     TIMESTAMP = 631,
     TO = 632,
     TRAILING = 633,
     TRANSACTION = 634,
     TRANSFORM = 635,
     TREAT = 636,
     TRIGGER = 637,
     TRIM = 638,
     TRUE_P = 639,
     TRUNCATE = 640,
     TRUSTED = 641,
     TYPE_P = 642,
     TYPES_P = 643,
     UNBOUNDED = 644,
     UNCOMMITTED = 645,
     UNENCRYPTED = 646,
     UNION = 647,
     UNIQUE = 648,
     UNKNOWN = 649,
     UNLISTEN = 650,
     UNLOGGED = 651,
     UNTIL = 652,
     UPDATE = 653,
     USER = 654,
     USING = 655,
     VACUUM = 656,
     VALID = 657,
     VALIDATE = 658,
     VALIDATOR = 659,
     VALUE_P = 660,
     VALUES = 661,
     VARCHAR = 662,
     VARIADIC = 663,
     VARYING = 664,
     VERBOSE = 665,
     VERSION_P = 666,
     VIEW = 667,
     VIEWS = 668,
     VOLATILE = 669,
     WHEN = 670,
     WHERE = 671,
     WHITESPACE_P = 672,
     WINDOW = 673,
     WITH = 674,
     WITHIN = 675,
     WITHOUT = 676,
     WORK = 677,
     WRAPPER = 678,
     WRITE = 679,
     XML_P = 680,
     XMLATTRIBUTES = 681,
     XMLCONCAT = 682,
     XMLELEMENT = 683,
     XMLEXISTS = 684,
     XMLFOREST = 685,
     XMLPARSE = 686,
     XMLPI = 687,
     XMLROOT = 688,
     XMLSERIALIZE = 689,
     YEAR_P = 690,
     YES_P = 691,
     ZONE = 692,
     NOT_LA = 693,
     NULLS_LA = 694,
     WITH_LA = 695,
     POSTFIXOP = 696,
     UMINUS = 697
   };
#endif
/* Tokens.  */
#define IDENT 258
#define FCONST 259
#define SCONST 260
#define BCONST 261
#define XCONST 262
#define Op 263
#define ICONST 264
#define PARAM 265
#define TYPECAST 266
#define DOT_DOT 267
#define COLON_EQUALS 268
#define EQUALS_GREATER 269
#define LESS_EQUALS 270
#define GREATER_EQUALS 271
#define NOT_EQUALS 272
#define ABORT_P 273
#define ABSOLUTE_P 274
#define ACCESS 275
#define ACTION 276
#define ADD_P 277
#define ADMIN 278
#define AFTER 279
#define AGGREGATE 280
#define ALL 281
#define ALSO 282
#define ALTER 283
#define ALWAYS 284
#define ANALYSE 285
#define ANALYZE 286
#define AND 287
#define ANY 288
#define ARRAY 289
#define AS 290
#define ASC 291
#define ASSERTION 292
#define ASSIGNMENT 293
#define ASYMMETRIC 294
#define AT 295
#define ATTRIBUTE 296
#define AUTHORIZATION 297
#define BACKWARD 298
#define BEFORE 299
#define BEGIN_P 300
#define BETWEEN 301
#define BIGINT 302
#define BINARY 303
#define BIT 304
#define BOOLEAN_P 305
#define BOTH 306
#define BY 307
#define CACHE 308
#define CALLED 309
#define CASCADE 310
#define CASCADED 311
#define CASE 312
#define CAST 313
#define CATALOG_P 314
#define CHAIN 315
#define CHAR_P 316
#define CHARACTER 317
#define CHARACTERISTICS 318
#define CHECK 319
#define CHECKPOINT 320
#define CLASS 321
#define CLOSE 322
#define CLUSTER 323
#define COALESCE 324
#define COLLATE 325
#define COLLATION 326
#define COLUMN 327
#define COMMENT 328
#define COMMENTS 329
#define COMMIT 330
#define COMMITTED 331
#define CONCURRENTLY 332
#define CONFIGURATION 333
#define CONFLICT 334
#define CONNECTION 335
#define CONSTRAINT 336
#define CONSTRAINTS 337
#define CONTENT_P 338
#define CONTINUE_P 339
#define CONVERSION_P 340
#define COPY 341
#define COST 342
#define CREATE 343
#define CROSS 344
#define CSV 345
#define CUBE 346
#define CURRENT_P 347
#define CURRENT_CATALOG 348
#define CURRENT_DATE 349
#define CURRENT_ROLE 350
#define CURRENT_SCHEMA 351
#define CURRENT_TIME 352
#define CURRENT_TIMESTAMP 353
#define CURRENT_USER 354
#define CURSOR 355
#define CYCLE 356
#define DATA_P 357
#define DATABASE 358
#define DAY_P 359
#define DEALLOCATE 360
#define DEC 361
#define DECIMAL_P 362
#define DECLARE 363
#define DEFAULT 364
#define DEFAULTS 365
#define DEFERRABLE 366
#define DEFERRED 367
#define DEFINER 368
#define DELETE_P 369
#define DELIMITER 370
#define DELIMITERS 371
#define DEPENDS 372
#define DESC 373
#define DICTIONARY 374
#define DISABLE_P 375
#define DISCARD 376
#define DISTINCT 377
#define DO 378
#define DOCUMENT_P 379
#define DOMAIN_P 380
#define DOUBLE_P 381
#define DROP 382
#define EACH 383
#define ELSE 384
#define ENABLE_P 385
#define ENCODING 386
#define ENCRYPTED 387
#define END_P 388
#define ENUM_P 389
#define ESCAPE 390
#define EVENT 391
#define EXCEPT 392
#define EXCLUDE 393
#define EXCLUDING 394
#define EXCLUSIVE 395
#define EXECUTE 396
#define EXISTS 397
#define EXPLAIN 398
#define EXTENSION 399
#define EXTERNAL 400
#define EXTRACT 401
#define FALSE_P 402
#define FAMILY 403
#define FETCH 404
#define FILTER 405
#define FIRST_P 406
#define FLOAT_P 407
#define FOLLOWING 408
#define FOR 409
#define FORCE 410
#define FOREIGN 411
#define FORWARD 412
#define FREEZE 413
#define FROM 414
#define FULL 415
#define FUNCTION 416
#define FUNCTIONS 417
#define GLOBAL 418
#define GRANT 419
#define GRANTED 420
#define GREATEST 421
#define GROUP_P 422
#define GROUPING 423
#define HANDLER 424
#define HAVING 425
#define HEADER_P 426
#define HOLD 427
#define HOUR_P 428
#define IDENTITY_P 429
#define IF_P 430
#define ILIKE 431
#define IMMEDIATE 432
#define IMMUTABLE 433
#define IMPLICIT_P 434
#define IMPORT_P 435
#define IN_P 436
#define INCLUDING 437
#define INCREMENT 438
#define INDEX 439
#define INDEXES 440
#define INHERIT 441
#define INHERITS 442
#define INITIALLY 443
#define INLINE_P 444
#define INNER_P 445
#define INOUT 446
#define INPUT_P 447
#define INSENSITIVE 448
#define INSERT 449
#define INSTEAD 450
#define INT_P 451
#define INTEGER 452
#define INTERSECT 453
#define INTERVAL 454
#define INTO 455
#define INVOKER 456
#define IS 457
#define ISNULL 458
#define ISOLATION 459
#define JOIN 460
#define KEY 461
#define LABEL 462
#define LANGUAGE 463
#define LARGE_P 464
#define LAST_P 465
#define LATERAL_P 466
#define LEADING 467
#define LEAKPROOF 468
#define LEAST 469
#define LEFT 470
#define LEVEL 471
#define LIKE 472
#define LIMIT 473
#define LISTEN 474
#define LOAD 475
#define LOCAL 476
#define LOCALTIME 477
#define LOCALTIMESTAMP 478
#define LOCATION 479
#define LOCK_P 480
#define LOCKED 481
#define LOGGED 482
#define MAPPING 483
#define MATCH 484
#define MATERIALIZED 485
#define MAXVALUE 486
#define METHOD 487
#define MINUTE_P 488
#define MINVALUE 489
#define MODE 490
#define MONTH_P 491
#define MOVE 492
#define NAME_P 493
#define NAMES 494
#define NATIONAL 495
#define NATURAL 496
#define NCHAR 497
#define NEXT 498
#define NO 499
#define NONE 500
#define NOT 501
#define NOTHING 502
#define NOTIFY 503
#define NOTNULL 504
#define NOWAIT 505
#define NULL_P 506
#define NULLIF 507
#define NULLS_P 508
#define NUMERIC 509
#define OBJECT_P 510
#define OF 511
#define OFF 512
#define OFFSET 513
#define OIDS 514
#define ON 515
#define ONLY 516
#define OPERATOR 517
#define OPTION 518
#define OPTIONS 519
#define OR 520
#define ORDER 521
#define ORDINALITY 522
#define OUT_P 523
#define OUTER_P 524
#define OVER 525
#define OVERLAPS 526
#define OVERLAY 527
#define OWNED 528
#define OWNER 529
#define PARALLEL 530
#define PARSER 531
#define PARTIAL 532
#define PARTITION 533
#define PASSING 534
#define PASSWORD 535
#define PGPOOL 536
#define PLACING 537
#define PLANS 538
#define POLICY 539
#define POSITION 540
#define PRECEDING 541
#define PRECISION 542
#define PRESERVE 543
#define PREPARE 544
#define PREPARED 545
#define PRIMARY 546
#define PRIOR 547
#define PRIVILEGES 548
#define PROCEDURAL 549
#define PROCEDURE 550
#define PROGRAM 551
#define QUOTE 552
#define RANGE 553
#define READ 554
#define REAL 555
#define REASSIGN 556
#define RECHECK 557
#define RECURSIVE 558
#define REF 559
#define REFERENCES 560
#define REFRESH 561
#define REINDEX 562
#define RELATIVE_P 563
#define RELEASE 564
#define RENAME 565
#define REPEATABLE 566
#define REPLACE 567
#define REPLICA 568
#define RESET 569
#define RESTART 570
#define RESTRICT 571
#define RETURNING 572
#define RETURNS 573
#define REVOKE 574
#define RIGHT 575
#define ROLE 576
#define ROLLBACK 577
#define ROLLUP 578
#define ROW 579
#define ROWS 580
#define RULE 581
#define SAVEPOINT 582
#define SCHEMA 583
#define SCROLL 584
#define SEARCH 585
#define SECOND_P 586
#define SECURITY 587
#define SELECT 588
#define SEQUENCE 589
#define SEQUENCES 590
#define SERIALIZABLE 591
#define SERVER 592
#define SESSION 593
#define SESSION_USER 594
#define SET 595
#define SETS 596
#define SETOF 597
#define SHARE 598
#define SHOW 599
#define SIMILAR 600
#define SIMPLE 601
#define SKIP 602
#define SMALLINT 603
#define SNAPSHOT 604
#define SOME 605
#define SQL_P 606
#define STABLE 607
#define STANDALONE_P 608
#define START 609
#define STATEMENT 610
#define STATISTICS 611
#define STDIN 612
#define STDOUT 613
#define STORAGE 614
#define STRICT_P 615
#define STRIP_P 616
#define SUBSTRING 617
#define SYMMETRIC 618
#define SYSID 619
#define SYSTEM_P 620
#define TABLE 621
#define TABLES 622
#define TABLESAMPLE 623
#define TABLESPACE 624
#define TEMP 625
#define TEMPLATE 626
#define TEMPORARY 627
#define TEXT_P 628
#define THEN 629
#define TIME 630
#define TIMESTAMP 631
#define TO 632
#define TRAILING 633
#define TRANSACTION 634
#define TRANSFORM 635
#define TREAT 636
#define TRIGGER 637
#define TRIM 638
#define TRUE_P 639
#define TRUNCATE 640
#define TRUSTED 641
#define TYPE_P 642
#define TYPES_P 643
#define UNBOUNDED 644
#define UNCOMMITTED 645
#define UNENCRYPTED 646
#define UNION 647
#define UNIQUE 648
#define UNKNOWN 649
#define UNLISTEN 650
#define UNLOGGED 651
#define UNTIL 652
#define UPDATE 653
#define USER 654
#define USING 655
#define VACUUM 656
#define VALID 657
#define VALIDATE 658
#define VALIDATOR 659
#define VALUE_P 660
#define VALUES 661
#define VARCHAR 662
#define VARIADIC 663
#define VARYING 664
#define VERBOSE 665
#define VERSION_P 666
#define VIEW 667
#define VIEWS 668
#define VOLATILE 669
#define WHEN 670
#define WHERE 671
#define WHITESPACE_P 672
#define WINDOW 673
#define WITH 674
#define WITHIN 675
#define WITHOUT 676
#define WORK 677
#define WRAPPER 678
#define WRITE 679
#define XML_P 680
#define XMLATTRIBUTES 681
#define XMLCONCAT 682
#define XMLELEMENT 683
#define XMLEXISTS 684
#define XMLFOREST 685
#define XMLPARSE 686
#define XMLPI 687
#define XMLROOT 688
#define XMLSERIALIZE 689
#define YEAR_P 690
#define YES_P 691
#define ZONE 692
#define NOT_LA 693
#define NULLS_LA 694
#define WITH_LA 695
#define POSTFIXOP 696
#define UMINUS 697



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 227 "gram.y"

	core_YYSTYPE		core_yystype;
	/* these fields must match core_YYSTYPE: */
	int					ival;
	char				*str;
	const char			*keyword;

	char				chr;
	bool				boolean;
	JoinType			jtype;
	DropBehavior		dbehavior;
	OnCommitAction		oncommit;
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
	WindowDef			*windef;
	JoinExpr			*jexpr;
	IndexElem			*ielem;
	Alias				*alias;
	RangeVar			*range;
	IntoClause			*into;
	WithClause			*with;
	InferClause			*infer;
	OnConflictClause	*onconflict;
	A_Indices			*aind;
	ResTarget			*target;
	struct PrivTarget	*privtarget;
	AccessPriv			*accesspriv;
	struct ImportQual	*importqual;
	InsertStmt			*istmt;
	VariableSetStmt		*vsetstmt;


/* Line 2058 of yacc.c  */
#line 982 "gram.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int base_yyparse (void *YYPARSE_PARAM);
#else
int base_yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int base_yyparse (core_yyscan_t yyscanner);
#else
int base_yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_BASE_YY_GRAM_H_INCLUDED  */
