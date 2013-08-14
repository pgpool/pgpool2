/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
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
     ABORT_P = 269,
     ABSOLUTE_P = 270,
     ACCESS = 271,
     ACTION = 272,
     ADD_P = 273,
     ADMIN = 274,
     AFTER = 275,
     AGGREGATE = 276,
     ALL = 277,
     ALSO = 278,
     ALTER = 279,
     ALWAYS = 280,
     ANALYSE = 281,
     ANALYZE = 282,
     AND = 283,
     ANY = 284,
     ARRAY = 285,
     AS = 286,
     ASC = 287,
     ASSERTION = 288,
     ASSIGNMENT = 289,
     ASYMMETRIC = 290,
     AT = 291,
     ATTRIBUTE = 292,
     AUTHORIZATION = 293,
     BACKWARD = 294,
     BEFORE = 295,
     BEGIN_P = 296,
     BETWEEN = 297,
     BIGINT = 298,
     BINARY = 299,
     BIT = 300,
     BOOLEAN_P = 301,
     BOTH = 302,
     BY = 303,
     CACHE = 304,
     CALLED = 305,
     CASCADE = 306,
     CASCADED = 307,
     CASE = 308,
     CAST = 309,
     CATALOG_P = 310,
     CHAIN = 311,
     CHAR_P = 312,
     CHARACTER = 313,
     CHARACTERISTICS = 314,
     CHECK = 315,
     CHECKPOINT = 316,
     CLASS = 317,
     CLOSE = 318,
     CLUSTER = 319,
     COALESCE = 320,
     COLLATE = 321,
     COLLATION = 322,
     COLUMN = 323,
     COMMENT = 324,
     COMMENTS = 325,
     COMMIT = 326,
     COMMITTED = 327,
     CONCURRENTLY = 328,
     CONFIGURATION = 329,
     CONNECTION = 330,
     CONSTRAINT = 331,
     CONSTRAINTS = 332,
     CONTENT_P = 333,
     CONTINUE_P = 334,
     CONVERSION_P = 335,
     COPY = 336,
     COST = 337,
     CREATE = 338,
     CROSS = 339,
     CSV = 340,
     CURRENT_P = 341,
     CURRENT_CATALOG = 342,
     CURRENT_DATE = 343,
     CURRENT_ROLE = 344,
     CURRENT_SCHEMA = 345,
     CURRENT_TIME = 346,
     CURRENT_TIMESTAMP = 347,
     CURRENT_USER = 348,
     CURSOR = 349,
     CYCLE = 350,
     DATA_P = 351,
     DATABASE = 352,
     DAY_P = 353,
     DEALLOCATE = 354,
     DEC = 355,
     DECIMAL_P = 356,
     DECLARE = 357,
     DEFAULT = 358,
     DEFAULTS = 359,
     DEFERRABLE = 360,
     DEFERRED = 361,
     DEFINER = 362,
     DELETE_P = 363,
     DELIMITER = 364,
     DELIMITERS = 365,
     DESC = 366,
     DICTIONARY = 367,
     DISABLE_P = 368,
     DISCARD = 369,
     DISTINCT = 370,
     DO = 371,
     DOCUMENT_P = 372,
     DOMAIN_P = 373,
     DOUBLE_P = 374,
     DROP = 375,
     EACH = 376,
     ELSE = 377,
     ENABLE_P = 378,
     ENCODING = 379,
     ENCRYPTED = 380,
     END_P = 381,
     ENUM_P = 382,
     ESCAPE = 383,
     EXCEPT = 384,
     EXCLUDE = 385,
     EXCLUDING = 386,
     EXCLUSIVE = 387,
     EXECUTE = 388,
     EXISTS = 389,
     EXPLAIN = 390,
     EXTENSION = 391,
     EXTERNAL = 392,
     EXTRACT = 393,
     FALSE_P = 394,
     FAMILY = 395,
     FETCH = 396,
     FIRST_P = 397,
     FLOAT_P = 398,
     FOLLOWING = 399,
     FOR = 400,
     FORCE = 401,
     FOREIGN = 402,
     FORWARD = 403,
     FREEZE = 404,
     FROM = 405,
     FULL = 406,
     FUNCTION = 407,
     FUNCTIONS = 408,
     GLOBAL = 409,
     GRANT = 410,
     GRANTED = 411,
     GREATEST = 412,
     GROUP_P = 413,
     HANDLER = 414,
     HAVING = 415,
     HEADER_P = 416,
     HOLD = 417,
     HOUR_P = 418,
     IDENTITY_P = 419,
     IF_P = 420,
     ILIKE = 421,
     IMMEDIATE = 422,
     IMMUTABLE = 423,
     IMPLICIT_P = 424,
     IN_P = 425,
     INCLUDING = 426,
     INCREMENT = 427,
     INDEX = 428,
     INDEXES = 429,
     INHERIT = 430,
     INHERITS = 431,
     INITIALLY = 432,
     INLINE_P = 433,
     INNER_P = 434,
     INOUT = 435,
     INPUT_P = 436,
     INSENSITIVE = 437,
     INSERT = 438,
     INSTEAD = 439,
     INT_P = 440,
     INTEGER = 441,
     INTERSECT = 442,
     INTERVAL = 443,
     INTO = 444,
     INVOKER = 445,
     IS = 446,
     ISNULL = 447,
     ISOLATION = 448,
     JOIN = 449,
     KEY = 450,
     LABEL = 451,
     LANGUAGE = 452,
     LARGE_P = 453,
     LAST_P = 454,
     LC_COLLATE_P = 455,
     LC_CTYPE_P = 456,
     LEADING = 457,
     LEAKPROOF = 458,
     LEAST = 459,
     LEFT = 460,
     LEVEL = 461,
     LIKE = 462,
     LIMIT = 463,
     LISTEN = 464,
     LOAD = 465,
     LOCAL = 466,
     LOCALTIME = 467,
     LOCALTIMESTAMP = 468,
     LOCATION = 469,
     LOCK_P = 470,
     MAPPING = 471,
     MATCH = 472,
     MAXVALUE = 473,
     MINUTE_P = 474,
     MINVALUE = 475,
     MODE = 476,
     MONTH_P = 477,
     MOVE = 478,
     NAME_P = 479,
     NAMES = 480,
     NATIONAL = 481,
     NATURAL = 482,
     NCHAR = 483,
     NEXT = 484,
     NO = 485,
     NONE = 486,
     NOT = 487,
     NOTHING = 488,
     NOTIFY = 489,
     NOTNULL = 490,
     NOWAIT = 491,
     NULL_P = 492,
     NULLIF = 493,
     NULLS_P = 494,
     NUMERIC = 495,
     OBJECT_P = 496,
     OF = 497,
     OFF = 498,
     OFFSET = 499,
     OIDS = 500,
     ON = 501,
     ONLY = 502,
     OPERATOR = 503,
     OPTION = 504,
     OPTIONS = 505,
     OR = 506,
     ORDER = 507,
     OUT_P = 508,
     OUTER_P = 509,
     OVER = 510,
     OVERLAPS = 511,
     OVERLAY = 512,
     OWNED = 513,
     OWNER = 514,
     PARSER = 515,
     PARTIAL = 516,
     PARTITION = 517,
     PASSING = 518,
     PASSWORD = 519,
     PLACING = 520,
     PLANS = 521,
     POSITION = 522,
     PRECEDING = 523,
     PRECISION = 524,
     PRESERVE = 525,
     PREPARE = 526,
     PREPARED = 527,
     PRIMARY = 528,
     PRIOR = 529,
     PRIVILEGES = 530,
     PROCEDURAL = 531,
     PROCEDURE = 532,
     QUOTE = 533,
     RANGE = 534,
     READ = 535,
     REAL = 536,
     REASSIGN = 537,
     RECHECK = 538,
     RECURSIVE = 539,
     REF = 540,
     REFERENCES = 541,
     REINDEX = 542,
     RELATIVE_P = 543,
     RELEASE = 544,
     RENAME = 545,
     REPEATABLE = 546,
     REPLACE = 547,
     REPLICA = 548,
     RESET = 549,
     RESTART = 550,
     RESTRICT = 551,
     RETURNING = 552,
     RETURNS = 553,
     REVOKE = 554,
     RIGHT = 555,
     ROLE = 556,
     ROLLBACK = 557,
     ROW = 558,
     ROWS = 559,
     RULE = 560,
     SAVEPOINT = 561,
     SCHEMA = 562,
     SCROLL = 563,
     SEARCH = 564,
     SECOND_P = 565,
     SECURITY = 566,
     SELECT = 567,
     SEQUENCE = 568,
     SEQUENCES = 569,
     SERIALIZABLE = 570,
     SERVER = 571,
     SESSION = 572,
     SESSION_USER = 573,
     SET = 574,
     SETOF = 575,
     SHARE = 576,
     SHOW = 577,
     SIMILAR = 578,
     SIMPLE = 579,
     SMALLINT = 580,
     SNAPSHOT = 581,
     SOME = 582,
     STABLE = 583,
     STANDALONE_P = 584,
     START = 585,
     STATEMENT = 586,
     STATISTICS = 587,
     STDIN = 588,
     STDOUT = 589,
     STORAGE = 590,
     STRICT_P = 591,
     STRIP_P = 592,
     SUBSTRING = 593,
     SYMMETRIC = 594,
     SYSID = 595,
     SYSTEM_P = 596,
     TABLE = 597,
     TABLES = 598,
     TABLESPACE = 599,
     TEMP = 600,
     TEMPLATE = 601,
     TEMPORARY = 602,
     TEXT_P = 603,
     THEN = 604,
     TIME = 605,
     TIMESTAMP = 606,
     TO = 607,
     TRAILING = 608,
     TRANSACTION = 609,
     TREAT = 610,
     TRIGGER = 611,
     TRIM = 612,
     TRUE_P = 613,
     TRUNCATE = 614,
     TRUSTED = 615,
     TYPE_P = 616,
     TYPES_P = 617,
     UNBOUNDED = 618,
     UNCOMMITTED = 619,
     UNENCRYPTED = 620,
     UNION = 621,
     UNIQUE = 622,
     UNKNOWN = 623,
     UNLISTEN = 624,
     UNLOGGED = 625,
     UNTIL = 626,
     UPDATE = 627,
     USER = 628,
     USING = 629,
     VACUUM = 630,
     VALID = 631,
     VALIDATE = 632,
     VALIDATOR = 633,
     VALUE_P = 634,
     VALUES = 635,
     VARCHAR = 636,
     VARIADIC = 637,
     VARYING = 638,
     VERBOSE = 639,
     VERSION_P = 640,
     VIEW = 641,
     VOLATILE = 642,
     WHEN = 643,
     WHERE = 644,
     WHITESPACE_P = 645,
     WINDOW = 646,
     WITH = 647,
     WITHOUT = 648,
     WORK = 649,
     WRAPPER = 650,
     WRITE = 651,
     XML_P = 652,
     XMLATTRIBUTES = 653,
     XMLCONCAT = 654,
     XMLELEMENT = 655,
     XMLEXISTS = 656,
     XMLFOREST = 657,
     XMLPARSE = 658,
     XMLPI = 659,
     XMLROOT = 660,
     XMLSERIALIZE = 661,
     YEAR_P = 662,
     YES_P = 663,
     ZONE = 664,
     NULLS_FIRST = 665,
     NULLS_LAST = 666,
     WITH_TIME = 667,
     POSTFIXOP = 668,
     UMINUS = 669
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
#define ABORT_P 269
#define ABSOLUTE_P 270
#define ACCESS 271
#define ACTION 272
#define ADD_P 273
#define ADMIN 274
#define AFTER 275
#define AGGREGATE 276
#define ALL 277
#define ALSO 278
#define ALTER 279
#define ALWAYS 280
#define ANALYSE 281
#define ANALYZE 282
#define AND 283
#define ANY 284
#define ARRAY 285
#define AS 286
#define ASC 287
#define ASSERTION 288
#define ASSIGNMENT 289
#define ASYMMETRIC 290
#define AT 291
#define ATTRIBUTE 292
#define AUTHORIZATION 293
#define BACKWARD 294
#define BEFORE 295
#define BEGIN_P 296
#define BETWEEN 297
#define BIGINT 298
#define BINARY 299
#define BIT 300
#define BOOLEAN_P 301
#define BOTH 302
#define BY 303
#define CACHE 304
#define CALLED 305
#define CASCADE 306
#define CASCADED 307
#define CASE 308
#define CAST 309
#define CATALOG_P 310
#define CHAIN 311
#define CHAR_P 312
#define CHARACTER 313
#define CHARACTERISTICS 314
#define CHECK 315
#define CHECKPOINT 316
#define CLASS 317
#define CLOSE 318
#define CLUSTER 319
#define COALESCE 320
#define COLLATE 321
#define COLLATION 322
#define COLUMN 323
#define COMMENT 324
#define COMMENTS 325
#define COMMIT 326
#define COMMITTED 327
#define CONCURRENTLY 328
#define CONFIGURATION 329
#define CONNECTION 330
#define CONSTRAINT 331
#define CONSTRAINTS 332
#define CONTENT_P 333
#define CONTINUE_P 334
#define CONVERSION_P 335
#define COPY 336
#define COST 337
#define CREATE 338
#define CROSS 339
#define CSV 340
#define CURRENT_P 341
#define CURRENT_CATALOG 342
#define CURRENT_DATE 343
#define CURRENT_ROLE 344
#define CURRENT_SCHEMA 345
#define CURRENT_TIME 346
#define CURRENT_TIMESTAMP 347
#define CURRENT_USER 348
#define CURSOR 349
#define CYCLE 350
#define DATA_P 351
#define DATABASE 352
#define DAY_P 353
#define DEALLOCATE 354
#define DEC 355
#define DECIMAL_P 356
#define DECLARE 357
#define DEFAULT 358
#define DEFAULTS 359
#define DEFERRABLE 360
#define DEFERRED 361
#define DEFINER 362
#define DELETE_P 363
#define DELIMITER 364
#define DELIMITERS 365
#define DESC 366
#define DICTIONARY 367
#define DISABLE_P 368
#define DISCARD 369
#define DISTINCT 370
#define DO 371
#define DOCUMENT_P 372
#define DOMAIN_P 373
#define DOUBLE_P 374
#define DROP 375
#define EACH 376
#define ELSE 377
#define ENABLE_P 378
#define ENCODING 379
#define ENCRYPTED 380
#define END_P 381
#define ENUM_P 382
#define ESCAPE 383
#define EXCEPT 384
#define EXCLUDE 385
#define EXCLUDING 386
#define EXCLUSIVE 387
#define EXECUTE 388
#define EXISTS 389
#define EXPLAIN 390
#define EXTENSION 391
#define EXTERNAL 392
#define EXTRACT 393
#define FALSE_P 394
#define FAMILY 395
#define FETCH 396
#define FIRST_P 397
#define FLOAT_P 398
#define FOLLOWING 399
#define FOR 400
#define FORCE 401
#define FOREIGN 402
#define FORWARD 403
#define FREEZE 404
#define FROM 405
#define FULL 406
#define FUNCTION 407
#define FUNCTIONS 408
#define GLOBAL 409
#define GRANT 410
#define GRANTED 411
#define GREATEST 412
#define GROUP_P 413
#define HANDLER 414
#define HAVING 415
#define HEADER_P 416
#define HOLD 417
#define HOUR_P 418
#define IDENTITY_P 419
#define IF_P 420
#define ILIKE 421
#define IMMEDIATE 422
#define IMMUTABLE 423
#define IMPLICIT_P 424
#define IN_P 425
#define INCLUDING 426
#define INCREMENT 427
#define INDEX 428
#define INDEXES 429
#define INHERIT 430
#define INHERITS 431
#define INITIALLY 432
#define INLINE_P 433
#define INNER_P 434
#define INOUT 435
#define INPUT_P 436
#define INSENSITIVE 437
#define INSERT 438
#define INSTEAD 439
#define INT_P 440
#define INTEGER 441
#define INTERSECT 442
#define INTERVAL 443
#define INTO 444
#define INVOKER 445
#define IS 446
#define ISNULL 447
#define ISOLATION 448
#define JOIN 449
#define KEY 450
#define LABEL 451
#define LANGUAGE 452
#define LARGE_P 453
#define LAST_P 454
#define LC_COLLATE_P 455
#define LC_CTYPE_P 456
#define LEADING 457
#define LEAKPROOF 458
#define LEAST 459
#define LEFT 460
#define LEVEL 461
#define LIKE 462
#define LIMIT 463
#define LISTEN 464
#define LOAD 465
#define LOCAL 466
#define LOCALTIME 467
#define LOCALTIMESTAMP 468
#define LOCATION 469
#define LOCK_P 470
#define MAPPING 471
#define MATCH 472
#define MAXVALUE 473
#define MINUTE_P 474
#define MINVALUE 475
#define MODE 476
#define MONTH_P 477
#define MOVE 478
#define NAME_P 479
#define NAMES 480
#define NATIONAL 481
#define NATURAL 482
#define NCHAR 483
#define NEXT 484
#define NO 485
#define NONE 486
#define NOT 487
#define NOTHING 488
#define NOTIFY 489
#define NOTNULL 490
#define NOWAIT 491
#define NULL_P 492
#define NULLIF 493
#define NULLS_P 494
#define NUMERIC 495
#define OBJECT_P 496
#define OF 497
#define OFF 498
#define OFFSET 499
#define OIDS 500
#define ON 501
#define ONLY 502
#define OPERATOR 503
#define OPTION 504
#define OPTIONS 505
#define OR 506
#define ORDER 507
#define OUT_P 508
#define OUTER_P 509
#define OVER 510
#define OVERLAPS 511
#define OVERLAY 512
#define OWNED 513
#define OWNER 514
#define PARSER 515
#define PARTIAL 516
#define PARTITION 517
#define PASSING 518
#define PASSWORD 519
#define PLACING 520
#define PLANS 521
#define POSITION 522
#define PRECEDING 523
#define PRECISION 524
#define PRESERVE 525
#define PREPARE 526
#define PREPARED 527
#define PRIMARY 528
#define PRIOR 529
#define PRIVILEGES 530
#define PROCEDURAL 531
#define PROCEDURE 532
#define QUOTE 533
#define RANGE 534
#define READ 535
#define REAL 536
#define REASSIGN 537
#define RECHECK 538
#define RECURSIVE 539
#define REF 540
#define REFERENCES 541
#define REINDEX 542
#define RELATIVE_P 543
#define RELEASE 544
#define RENAME 545
#define REPEATABLE 546
#define REPLACE 547
#define REPLICA 548
#define RESET 549
#define RESTART 550
#define RESTRICT 551
#define RETURNING 552
#define RETURNS 553
#define REVOKE 554
#define RIGHT 555
#define ROLE 556
#define ROLLBACK 557
#define ROW 558
#define ROWS 559
#define RULE 560
#define SAVEPOINT 561
#define SCHEMA 562
#define SCROLL 563
#define SEARCH 564
#define SECOND_P 565
#define SECURITY 566
#define SELECT 567
#define SEQUENCE 568
#define SEQUENCES 569
#define SERIALIZABLE 570
#define SERVER 571
#define SESSION 572
#define SESSION_USER 573
#define SET 574
#define SETOF 575
#define SHARE 576
#define SHOW 577
#define SIMILAR 578
#define SIMPLE 579
#define SMALLINT 580
#define SNAPSHOT 581
#define SOME 582
#define STABLE 583
#define STANDALONE_P 584
#define START 585
#define STATEMENT 586
#define STATISTICS 587
#define STDIN 588
#define STDOUT 589
#define STORAGE 590
#define STRICT_P 591
#define STRIP_P 592
#define SUBSTRING 593
#define SYMMETRIC 594
#define SYSID 595
#define SYSTEM_P 596
#define TABLE 597
#define TABLES 598
#define TABLESPACE 599
#define TEMP 600
#define TEMPLATE 601
#define TEMPORARY 602
#define TEXT_P 603
#define THEN 604
#define TIME 605
#define TIMESTAMP 606
#define TO 607
#define TRAILING 608
#define TRANSACTION 609
#define TREAT 610
#define TRIGGER 611
#define TRIM 612
#define TRUE_P 613
#define TRUNCATE 614
#define TRUSTED 615
#define TYPE_P 616
#define TYPES_P 617
#define UNBOUNDED 618
#define UNCOMMITTED 619
#define UNENCRYPTED 620
#define UNION 621
#define UNIQUE 622
#define UNKNOWN 623
#define UNLISTEN 624
#define UNLOGGED 625
#define UNTIL 626
#define UPDATE 627
#define USER 628
#define USING 629
#define VACUUM 630
#define VALID 631
#define VALIDATE 632
#define VALIDATOR 633
#define VALUE_P 634
#define VALUES 635
#define VARCHAR 636
#define VARIADIC 637
#define VARYING 638
#define VERBOSE 639
#define VERSION_P 640
#define VIEW 641
#define VOLATILE 642
#define WHEN 643
#define WHERE 644
#define WHITESPACE_P 645
#define WINDOW 646
#define WITH 647
#define WITHOUT 648
#define WORK 649
#define WRAPPER 650
#define WRITE 651
#define XML_P 652
#define XMLATTRIBUTES 653
#define XMLCONCAT 654
#define XMLELEMENT 655
#define XMLEXISTS 656
#define XMLFOREST 657
#define XMLPARSE 658
#define XMLPI 659
#define XMLROOT 660
#define XMLSERIALIZE 661
#define YEAR_P 662
#define YES_P 663
#define ZONE 664
#define NULLS_FIRST 665
#define NULLS_LAST 666
#define WITH_TIME 667
#define POSTFIXOP 668
#define UMINUS 669




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 171 "gram.y"

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
	A_Indices			*aind;
	ResTarget			*target;
	struct PrivTarget	*privtarget;
	AccessPriv			*accesspriv;
	InsertStmt			*istmt;
	VariableSetStmt		*vsetstmt;



/* Line 1685 of yacc.c  */
#line 918 "gram.h"
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



