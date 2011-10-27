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
     AUTHORIZATION = 292,
     BACKWARD = 293,
     BEFORE = 294,
     BEGIN_P = 295,
     BETWEEN = 296,
     BIGINT = 297,
     BINARY = 298,
     BIT = 299,
     BOOLEAN_P = 300,
     BOTH = 301,
     BY = 302,
     CACHE = 303,
     CALLED = 304,
     CASCADE = 305,
     CASCADED = 306,
     CASE = 307,
     CAST = 308,
     CATALOG_P = 309,
     CHAIN = 310,
     CHAR_P = 311,
     CHARACTER = 312,
     CHARACTERISTICS = 313,
     CHECK = 314,
     CHECKPOINT = 315,
     CLASS = 316,
     CLOSE = 317,
     CLUSTER = 318,
     COALESCE = 319,
     COLLATE = 320,
     COLUMN = 321,
     COMMENT = 322,
     COMMENTS = 323,
     COMMIT = 324,
     COMMITTED = 325,
     CONCURRENTLY = 326,
     CONFIGURATION = 327,
     CONNECTION = 328,
     CONSTRAINT = 329,
     CONSTRAINTS = 330,
     CONTENT_P = 331,
     CONTINUE_P = 332,
     CONVERSION_P = 333,
     COPY = 334,
     COST = 335,
     CREATE = 336,
     CREATEDB = 337,
     CREATEROLE = 338,
     CREATEUSER = 339,
     CROSS = 340,
     CSV = 341,
     CURRENT_P = 342,
     CURRENT_CATALOG = 343,
     CURRENT_DATE = 344,
     CURRENT_ROLE = 345,
     CURRENT_SCHEMA = 346,
     CURRENT_TIME = 347,
     CURRENT_TIMESTAMP = 348,
     CURRENT_USER = 349,
     CURSOR = 350,
     CYCLE = 351,
     DATA_P = 352,
     DATABASE = 353,
     DAY_P = 354,
     DEALLOCATE = 355,
     DEC = 356,
     DECIMAL_P = 357,
     DECLARE = 358,
     DEFAULT = 359,
     DEFAULTS = 360,
     DEFERRABLE = 361,
     DEFERRED = 362,
     DEFINER = 363,
     DELETE_P = 364,
     DELIMITER = 365,
     DELIMITERS = 366,
     DESC = 367,
     DICTIONARY = 368,
     DISABLE_P = 369,
     DISCARD = 370,
     DISTINCT = 371,
     DO = 372,
     DOCUMENT_P = 373,
     DOMAIN_P = 374,
     DOUBLE_P = 375,
     DROP = 376,
     EACH = 377,
     ELSE = 378,
     ENABLE_P = 379,
     ENCODING = 380,
     ENCRYPTED = 381,
     END_P = 382,
     ENUM_P = 383,
     ESCAPE = 384,
     EXCEPT = 385,
     EXCLUDE = 386,
     EXCLUDING = 387,
     EXCLUSIVE = 388,
     EXECUTE = 389,
     EXISTS = 390,
     EXPLAIN = 391,
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
     LANGUAGE = 451,
     LARGE_P = 452,
     LAST_P = 453,
     LC_COLLATE_P = 454,
     LC_CTYPE_P = 455,
     LEADING = 456,
     LEAST = 457,
     LEFT = 458,
     LEVEL = 459,
     LIKE = 460,
     LIMIT = 461,
     LISTEN = 462,
     LOAD = 463,
     LOCAL = 464,
     LOCALTIME = 465,
     LOCALTIMESTAMP = 466,
     LOCATION = 467,
     LOCK_P = 468,
     LOGIN_P = 469,
     MAPPING = 470,
     MATCH = 471,
     MAXVALUE = 472,
     MINUTE_P = 473,
     MINVALUE = 474,
     MODE = 475,
     MONTH_P = 476,
     MOVE = 477,
     NAME_P = 478,
     NAMES = 479,
     NATIONAL = 480,
     NATURAL = 481,
     NCHAR = 482,
     NEXT = 483,
     NO = 484,
     NOCREATEDB = 485,
     NOCREATEROLE = 486,
     NOCREATEUSER = 487,
     NOINHERIT = 488,
     NOLOGIN_P = 489,
     NONE = 490,
     NOSUPERUSER = 491,
     NOT = 492,
     NOTHING = 493,
     NOTIFY = 494,
     NOTNULL = 495,
     NOWAIT = 496,
     NULL_P = 497,
     NULLIF = 498,
     NULLS_P = 499,
     NUMERIC = 500,
     OBJECT_P = 501,
     OF = 502,
     OFF = 503,
     OFFSET = 504,
     OIDS = 505,
     ON = 506,
     ONLY = 507,
     OPERATOR = 508,
     OPTION = 509,
     OPTIONS = 510,
     OR = 511,
     ORDER = 512,
     OUT_P = 513,
     OUTER_P = 514,
     OVER = 515,
     OVERLAPS = 516,
     OVERLAY = 517,
     OWNED = 518,
     OWNER = 519,
     PARSER = 520,
     PARTIAL = 521,
     PARTITION = 522,
     PASSWORD = 523,
     PLACING = 524,
     PLANS = 525,
     POSITION = 526,
     PRECEDING = 527,
     PRECISION = 528,
     PRESERVE = 529,
     PREPARE = 530,
     PREPARED = 531,
     PRIMARY = 532,
     PRIOR = 533,
     PRIVILEGES = 534,
     PROCEDURAL = 535,
     PROCEDURE = 536,
     QUOTE = 537,
     RANGE = 538,
     READ = 539,
     REAL = 540,
     REASSIGN = 541,
     RECHECK = 542,
     RECURSIVE = 543,
     REFERENCES = 544,
     REINDEX = 545,
     RELATIVE_P = 546,
     RELEASE = 547,
     RENAME = 548,
     REPEATABLE = 549,
     REPLACE = 550,
     REPLICA = 551,
     RESET = 552,
     RESTART = 553,
     RESTRICT = 554,
     RETURNING = 555,
     RETURNS = 556,
     REVOKE = 557,
     RIGHT = 558,
     ROLE = 559,
     ROLLBACK = 560,
     ROW = 561,
     ROWS = 562,
     RULE = 563,
     SAVEPOINT = 564,
     SCHEMA = 565,
     SCROLL = 566,
     SEARCH = 567,
     SECOND_P = 568,
     SECURITY = 569,
     SELECT = 570,
     SEQUENCE = 571,
     SEQUENCES = 572,
     SERIALIZABLE = 573,
     SERVER = 574,
     SESSION = 575,
     SESSION_USER = 576,
     SET = 577,
     SETOF = 578,
     SHARE = 579,
     SHOW = 580,
     SIMILAR = 581,
     SIMPLE = 582,
     SMALLINT = 583,
     SOME = 584,
     STABLE = 585,
     STANDALONE_P = 586,
     START = 587,
     STATEMENT = 588,
     STATISTICS = 589,
     STDIN = 590,
     STDOUT = 591,
     STORAGE = 592,
     STRICT_P = 593,
     STRIP_P = 594,
     SUBSTRING = 595,
     SUPERUSER_P = 596,
     SYMMETRIC = 597,
     SYSID = 598,
     SYSTEM_P = 599,
     TABLE = 600,
     TABLES = 601,
     TABLESPACE = 602,
     TEMP = 603,
     TEMPLATE = 604,
     TEMPORARY = 605,
     TEXT_P = 606,
     THEN = 607,
     TIME = 608,
     TIMESTAMP = 609,
     TO = 610,
     TRAILING = 611,
     TRANSACTION = 612,
     TREAT = 613,
     TRIGGER = 614,
     TRIM = 615,
     TRUE_P = 616,
     TRUNCATE = 617,
     TRUSTED = 618,
     TYPE_P = 619,
     UNBOUNDED = 620,
     UNCOMMITTED = 621,
     UNENCRYPTED = 622,
     UNION = 623,
     UNIQUE = 624,
     UNKNOWN = 625,
     UNLISTEN = 626,
     UNTIL = 627,
     UPDATE = 628,
     USER = 629,
     USING = 630,
     VACUUM = 631,
     VALID = 632,
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
     XMLFOREST = 656,
     XMLPARSE = 657,
     XMLPI = 658,
     XMLROOT = 659,
     XMLSERIALIZE = 660,
     YEAR_P = 661,
     YES_P = 662,
     ZONE = 663,
     NULLS_FIRST = 664,
     NULLS_LAST = 665,
     WITH_TIME = 666,
     POSTFIXOP = 667,
     UMINUS = 668
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
#define AUTHORIZATION 292
#define BACKWARD 293
#define BEFORE 294
#define BEGIN_P 295
#define BETWEEN 296
#define BIGINT 297
#define BINARY 298
#define BIT 299
#define BOOLEAN_P 300
#define BOTH 301
#define BY 302
#define CACHE 303
#define CALLED 304
#define CASCADE 305
#define CASCADED 306
#define CASE 307
#define CAST 308
#define CATALOG_P 309
#define CHAIN 310
#define CHAR_P 311
#define CHARACTER 312
#define CHARACTERISTICS 313
#define CHECK 314
#define CHECKPOINT 315
#define CLASS 316
#define CLOSE 317
#define CLUSTER 318
#define COALESCE 319
#define COLLATE 320
#define COLUMN 321
#define COMMENT 322
#define COMMENTS 323
#define COMMIT 324
#define COMMITTED 325
#define CONCURRENTLY 326
#define CONFIGURATION 327
#define CONNECTION 328
#define CONSTRAINT 329
#define CONSTRAINTS 330
#define CONTENT_P 331
#define CONTINUE_P 332
#define CONVERSION_P 333
#define COPY 334
#define COST 335
#define CREATE 336
#define CREATEDB 337
#define CREATEROLE 338
#define CREATEUSER 339
#define CROSS 340
#define CSV 341
#define CURRENT_P 342
#define CURRENT_CATALOG 343
#define CURRENT_DATE 344
#define CURRENT_ROLE 345
#define CURRENT_SCHEMA 346
#define CURRENT_TIME 347
#define CURRENT_TIMESTAMP 348
#define CURRENT_USER 349
#define CURSOR 350
#define CYCLE 351
#define DATA_P 352
#define DATABASE 353
#define DAY_P 354
#define DEALLOCATE 355
#define DEC 356
#define DECIMAL_P 357
#define DECLARE 358
#define DEFAULT 359
#define DEFAULTS 360
#define DEFERRABLE 361
#define DEFERRED 362
#define DEFINER 363
#define DELETE_P 364
#define DELIMITER 365
#define DELIMITERS 366
#define DESC 367
#define DICTIONARY 368
#define DISABLE_P 369
#define DISCARD 370
#define DISTINCT 371
#define DO 372
#define DOCUMENT_P 373
#define DOMAIN_P 374
#define DOUBLE_P 375
#define DROP 376
#define EACH 377
#define ELSE 378
#define ENABLE_P 379
#define ENCODING 380
#define ENCRYPTED 381
#define END_P 382
#define ENUM_P 383
#define ESCAPE 384
#define EXCEPT 385
#define EXCLUDE 386
#define EXCLUDING 387
#define EXCLUSIVE 388
#define EXECUTE 389
#define EXISTS 390
#define EXPLAIN 391
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
#define LANGUAGE 451
#define LARGE_P 452
#define LAST_P 453
#define LC_COLLATE_P 454
#define LC_CTYPE_P 455
#define LEADING 456
#define LEAST 457
#define LEFT 458
#define LEVEL 459
#define LIKE 460
#define LIMIT 461
#define LISTEN 462
#define LOAD 463
#define LOCAL 464
#define LOCALTIME 465
#define LOCALTIMESTAMP 466
#define LOCATION 467
#define LOCK_P 468
#define LOGIN_P 469
#define MAPPING 470
#define MATCH 471
#define MAXVALUE 472
#define MINUTE_P 473
#define MINVALUE 474
#define MODE 475
#define MONTH_P 476
#define MOVE 477
#define NAME_P 478
#define NAMES 479
#define NATIONAL 480
#define NATURAL 481
#define NCHAR 482
#define NEXT 483
#define NO 484
#define NOCREATEDB 485
#define NOCREATEROLE 486
#define NOCREATEUSER 487
#define NOINHERIT 488
#define NOLOGIN_P 489
#define NONE 490
#define NOSUPERUSER 491
#define NOT 492
#define NOTHING 493
#define NOTIFY 494
#define NOTNULL 495
#define NOWAIT 496
#define NULL_P 497
#define NULLIF 498
#define NULLS_P 499
#define NUMERIC 500
#define OBJECT_P 501
#define OF 502
#define OFF 503
#define OFFSET 504
#define OIDS 505
#define ON 506
#define ONLY 507
#define OPERATOR 508
#define OPTION 509
#define OPTIONS 510
#define OR 511
#define ORDER 512
#define OUT_P 513
#define OUTER_P 514
#define OVER 515
#define OVERLAPS 516
#define OVERLAY 517
#define OWNED 518
#define OWNER 519
#define PARSER 520
#define PARTIAL 521
#define PARTITION 522
#define PASSWORD 523
#define PLACING 524
#define PLANS 525
#define POSITION 526
#define PRECEDING 527
#define PRECISION 528
#define PRESERVE 529
#define PREPARE 530
#define PREPARED 531
#define PRIMARY 532
#define PRIOR 533
#define PRIVILEGES 534
#define PROCEDURAL 535
#define PROCEDURE 536
#define QUOTE 537
#define RANGE 538
#define READ 539
#define REAL 540
#define REASSIGN 541
#define RECHECK 542
#define RECURSIVE 543
#define REFERENCES 544
#define REINDEX 545
#define RELATIVE_P 546
#define RELEASE 547
#define RENAME 548
#define REPEATABLE 549
#define REPLACE 550
#define REPLICA 551
#define RESET 552
#define RESTART 553
#define RESTRICT 554
#define RETURNING 555
#define RETURNS 556
#define REVOKE 557
#define RIGHT 558
#define ROLE 559
#define ROLLBACK 560
#define ROW 561
#define ROWS 562
#define RULE 563
#define SAVEPOINT 564
#define SCHEMA 565
#define SCROLL 566
#define SEARCH 567
#define SECOND_P 568
#define SECURITY 569
#define SELECT 570
#define SEQUENCE 571
#define SEQUENCES 572
#define SERIALIZABLE 573
#define SERVER 574
#define SESSION 575
#define SESSION_USER 576
#define SET 577
#define SETOF 578
#define SHARE 579
#define SHOW 580
#define SIMILAR 581
#define SIMPLE 582
#define SMALLINT 583
#define SOME 584
#define STABLE 585
#define STANDALONE_P 586
#define START 587
#define STATEMENT 588
#define STATISTICS 589
#define STDIN 590
#define STDOUT 591
#define STORAGE 592
#define STRICT_P 593
#define STRIP_P 594
#define SUBSTRING 595
#define SUPERUSER_P 596
#define SYMMETRIC 597
#define SYSID 598
#define SYSTEM_P 599
#define TABLE 600
#define TABLES 601
#define TABLESPACE 602
#define TEMP 603
#define TEMPLATE 604
#define TEMPORARY 605
#define TEXT_P 606
#define THEN 607
#define TIME 608
#define TIMESTAMP 609
#define TO 610
#define TRAILING 611
#define TRANSACTION 612
#define TREAT 613
#define TRIGGER 614
#define TRIM 615
#define TRUE_P 616
#define TRUNCATE 617
#define TRUSTED 618
#define TYPE_P 619
#define UNBOUNDED 620
#define UNCOMMITTED 621
#define UNENCRYPTED 622
#define UNION 623
#define UNIQUE 624
#define UNKNOWN 625
#define UNLISTEN 626
#define UNTIL 627
#define UPDATE 628
#define USER 629
#define USING 630
#define VACUUM 631
#define VALID 632
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
#define XMLFOREST 656
#define XMLPARSE 657
#define XMLPI 658
#define XMLROOT 659
#define XMLSERIALIZE 660
#define YEAR_P 661
#define YES_P 662
#define ZONE 663
#define NULLS_FIRST 664
#define NULLS_LAST 665
#define WITH_TIME 666
#define POSTFIXOP 667
#define UMINUS 668




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 155 "gram.y"

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
#line 916 "gram.h"
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



