/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_MINIMAL_BASE_YY_GRAM_MINIMAL_H_INCLUDED
# define YY_MINIMAL_BASE_YY_GRAM_MINIMAL_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int minimal_base_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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
    ATTACH = 296,
    ATTRIBUTE = 297,
    AUTHORIZATION = 298,
    BACKWARD = 299,
    BEFORE = 300,
    BEGIN_P = 301,
    BETWEEN = 302,
    BIGINT = 303,
    BINARY = 304,
    BIT = 305,
    BOOLEAN_P = 306,
    BOTH = 307,
    BY = 308,
    CACHE = 309,
    CALL = 310,
    CALLED = 311,
    CASCADE = 312,
    CASCADED = 313,
    CASE = 314,
    CAST = 315,
    CATALOG_P = 316,
    CHAIN = 317,
    CHAR_P = 318,
    CHARACTER = 319,
    CHARACTERISTICS = 320,
    CHECK = 321,
    CHECKPOINT = 322,
    CLASS = 323,
    CLOSE = 324,
    CLUSTER = 325,
    COALESCE = 326,
    COLLATE = 327,
    COLLATION = 328,
    COLUMN = 329,
    COLUMNS = 330,
    COMMENT = 331,
    COMMENTS = 332,
    COMMIT = 333,
    COMMITTED = 334,
    CONCURRENTLY = 335,
    CONFIGURATION = 336,
    CONFLICT = 337,
    CONNECTION = 338,
    CONSTRAINT = 339,
    CONSTRAINTS = 340,
    CONTENT_P = 341,
    CONTINUE_P = 342,
    CONVERSION_P = 343,
    COPY = 344,
    COST = 345,
    CREATE = 346,
    CROSS = 347,
    CSV = 348,
    CUBE = 349,
    CURRENT_P = 350,
    CURRENT_CATALOG = 351,
    CURRENT_DATE = 352,
    CURRENT_ROLE = 353,
    CURRENT_SCHEMA = 354,
    CURRENT_TIME = 355,
    CURRENT_TIMESTAMP = 356,
    CURRENT_USER = 357,
    CURSOR = 358,
    CYCLE = 359,
    DATA_P = 360,
    DATABASE = 361,
    DAY_P = 362,
    DEALLOCATE = 363,
    DEC = 364,
    DECIMAL_P = 365,
    DECLARE = 366,
    DEFAULT = 367,
    DEFAULTS = 368,
    DEFERRABLE = 369,
    DEFERRED = 370,
    DEFINER = 371,
    DELETE_P = 372,
    DELIMITER = 373,
    DELIMITERS = 374,
    DEPENDS = 375,
    DESC = 376,
    DETACH = 377,
    DICTIONARY = 378,
    DISABLE_P = 379,
    DISCARD = 380,
    DISTINCT = 381,
    DO = 382,
    DOCUMENT_P = 383,
    DOMAIN_P = 384,
    DOUBLE_P = 385,
    DROP = 386,
    EACH = 387,
    ELSE = 388,
    ENABLE_P = 389,
    ENCODING = 390,
    ENCRYPTED = 391,
    END_P = 392,
    ENUM_P = 393,
    ESCAPE = 394,
    EVENT = 395,
    EXCEPT = 396,
    EXCLUDE = 397,
    EXCLUDING = 398,
    EXCLUSIVE = 399,
    EXECUTE = 400,
    EXISTS = 401,
    EXPLAIN = 402,
    EXTENSION = 403,
    EXTERNAL = 404,
    EXTRACT = 405,
    FALSE_P = 406,
    FAMILY = 407,
    FETCH = 408,
    FILTER = 409,
    FIRST_P = 410,
    FLOAT_P = 411,
    FOLLOWING = 412,
    FOR = 413,
    FORCE = 414,
    FOREIGN = 415,
    FORWARD = 416,
    FREEZE = 417,
    FROM = 418,
    FULL = 419,
    FUNCTION = 420,
    FUNCTIONS = 421,
    GENERATED = 422,
    GLOBAL = 423,
    GRANT = 424,
    GRANTED = 425,
    GREATEST = 426,
    GROUP_P = 427,
    GROUPING = 428,
    GROUPS = 429,
    HANDLER = 430,
    HAVING = 431,
    HEADER_P = 432,
    HOLD = 433,
    HOUR_P = 434,
    IDENTITY_P = 435,
    IF_P = 436,
    ILIKE = 437,
    IMMEDIATE = 438,
    IMMUTABLE = 439,
    IMPLICIT_P = 440,
    IMPORT_P = 441,
    IN_P = 442,
    INCLUDE = 443,
    INCLUDING = 444,
    INCREMENT = 445,
    INDEX = 446,
    INDEXES = 447,
    INHERIT = 448,
    INHERITS = 449,
    INITIALLY = 450,
    INLINE_P = 451,
    INNER_P = 452,
    INOUT = 453,
    INPUT_P = 454,
    INSENSITIVE = 455,
    INSERT = 456,
    INSTEAD = 457,
    INT_P = 458,
    INTEGER = 459,
    INTERSECT = 460,
    INTERVAL = 461,
    INTO = 462,
    INVOKER = 463,
    IS = 464,
    ISNULL = 465,
    ISOLATION = 466,
    JOIN = 467,
    KEY = 468,
    LABEL = 469,
    LANGUAGE = 470,
    LARGE_P = 471,
    LAST_P = 472,
    LATERAL_P = 473,
    LEADING = 474,
    LEAKPROOF = 475,
    LEAST = 476,
    LEFT = 477,
    LEVEL = 478,
    LIKE = 479,
    LIMIT = 480,
    LISTEN = 481,
    LOAD = 482,
    LOCAL = 483,
    LOCALTIME = 484,
    LOCALTIMESTAMP = 485,
    LOCATION = 486,
    LOCK_P = 487,
    LOCKED = 488,
    LOGGED = 489,
    MAPPING = 490,
    MATCH = 491,
    MATERIALIZED = 492,
    MAXVALUE = 493,
    METHOD = 494,
    MINUTE_P = 495,
    MINVALUE = 496,
    MODE = 497,
    MONTH_P = 498,
    MOVE = 499,
    NAME_P = 500,
    NAMES = 501,
    NATIONAL = 502,
    NATURAL = 503,
    NCHAR = 504,
    NEW = 505,
    NEXT = 506,
    NO = 507,
    NONE = 508,
    NOT = 509,
    NOTHING = 510,
    NOTIFY = 511,
    NOTNULL = 512,
    NOWAIT = 513,
    NULL_P = 514,
    NULLIF = 515,
    NULLS_P = 516,
    NUMERIC = 517,
    OBJECT_P = 518,
    OF = 519,
    OFF = 520,
    OFFSET = 521,
    OIDS = 522,
    OLD = 523,
    ON = 524,
    ONLY = 525,
    OPERATOR = 526,
    OPTION = 527,
    OPTIONS = 528,
    OR = 529,
    ORDER = 530,
    ORDINALITY = 531,
    OTHERS = 532,
    OUT_P = 533,
    OUTER_P = 534,
    OVER = 535,
    OVERLAPS = 536,
    OVERLAY = 537,
    OVERRIDING = 538,
    OWNED = 539,
    OWNER = 540,
    PARALLEL = 541,
    PARSER = 542,
    PARTIAL = 543,
    PARTITION = 544,
    PASSING = 545,
    PASSWORD = 546,
    PGPOOL = 547,
    PLACING = 548,
    PLANS = 549,
    POLICY = 550,
    POSITION = 551,
    PRECEDING = 552,
    PRECISION = 553,
    PRESERVE = 554,
    PREPARE = 555,
    PREPARED = 556,
    PRIMARY = 557,
    PRIOR = 558,
    PRIVILEGES = 559,
    PROCEDURAL = 560,
    PROCEDURE = 561,
    PROCEDURES = 562,
    PROGRAM = 563,
    PUBLICATION = 564,
    QUOTE = 565,
    RANGE = 566,
    READ = 567,
    REAL = 568,
    REASSIGN = 569,
    RECHECK = 570,
    RECURSIVE = 571,
    REF = 572,
    REFERENCES = 573,
    REFERENCING = 574,
    REFRESH = 575,
    REINDEX = 576,
    RELATIVE_P = 577,
    RELEASE = 578,
    RENAME = 579,
    REPEATABLE = 580,
    REPLACE = 581,
    REPLICA = 582,
    RESET = 583,
    RESTART = 584,
    RESTRICT = 585,
    RETURNING = 586,
    RETURNS = 587,
    REVOKE = 588,
    RIGHT = 589,
    ROLE = 590,
    ROLLBACK = 591,
    ROLLUP = 592,
    ROUTINE = 593,
    ROUTINES = 594,
    ROW = 595,
    ROWS = 596,
    RULE = 597,
    SAVEPOINT = 598,
    SCHEMA = 599,
    SCHEMAS = 600,
    SCROLL = 601,
    SEARCH = 602,
    SECOND_P = 603,
    SECURITY = 604,
    SELECT = 605,
    SEQUENCE = 606,
    SEQUENCES = 607,
    SERIALIZABLE = 608,
    SERVER = 609,
    SESSION = 610,
    SESSION_USER = 611,
    SET = 612,
    SETS = 613,
    SETOF = 614,
    SHARE = 615,
    SHOW = 616,
    SIMILAR = 617,
    SIMPLE = 618,
    SKIP = 619,
    SMALLINT = 620,
    SNAPSHOT = 621,
    SOME = 622,
    SQL_P = 623,
    STABLE = 624,
    STANDALONE_P = 625,
    START = 626,
    STATEMENT = 627,
    STATISTICS = 628,
    STDIN = 629,
    STDOUT = 630,
    STORAGE = 631,
    STORED = 632,
    STRICT_P = 633,
    STRIP_P = 634,
    SUBSCRIPTION = 635,
    SUBSTRING = 636,
    SUPPORT = 637,
    SYMMETRIC = 638,
    SYSID = 639,
    SYSTEM_P = 640,
    TABLE = 641,
    TABLES = 642,
    TABLESAMPLE = 643,
    TABLESPACE = 644,
    TEMP = 645,
    TEMPLATE = 646,
    TEMPORARY = 647,
    TEXT_P = 648,
    THEN = 649,
    TIES = 650,
    TIME = 651,
    TIMESTAMP = 652,
    TO = 653,
    TRAILING = 654,
    TRANSACTION = 655,
    TRANSFORM = 656,
    TREAT = 657,
    TRIGGER = 658,
    TRIM = 659,
    TRUE_P = 660,
    TRUNCATE = 661,
    TRUSTED = 662,
    TYPE_P = 663,
    TYPES_P = 664,
    UNBOUNDED = 665,
    UNCOMMITTED = 666,
    UNENCRYPTED = 667,
    UNION = 668,
    UNIQUE = 669,
    UNKNOWN = 670,
    UNLISTEN = 671,
    UNLOGGED = 672,
    UNTIL = 673,
    UPDATE = 674,
    USER = 675,
    USING = 676,
    VACUUM = 677,
    VALID = 678,
    VALIDATE = 679,
    VALIDATOR = 680,
    VALUE_P = 681,
    VALUES = 682,
    VARCHAR = 683,
    VARIADIC = 684,
    VARYING = 685,
    VERBOSE = 686,
    VERSION_P = 687,
    VIEW = 688,
    VIEWS = 689,
    VOLATILE = 690,
    WHEN = 691,
    WHERE = 692,
    WHITESPACE_P = 693,
    WINDOW = 694,
    WITH = 695,
    WITHIN = 696,
    WITHOUT = 697,
    WORK = 698,
    WRAPPER = 699,
    WRITE = 700,
    XML_P = 701,
    XMLATTRIBUTES = 702,
    XMLCONCAT = 703,
    XMLELEMENT = 704,
    XMLEXISTS = 705,
    XMLFOREST = 706,
    XMLNAMESPACES = 707,
    XMLPARSE = 708,
    XMLPI = 709,
    XMLROOT = 710,
    XMLSERIALIZE = 711,
    XMLTABLE = 712,
    YEAR_P = 713,
    YES_P = 714,
    ZONE = 715,
    NOT_LA = 716,
    NULLS_LA = 717,
    WITH_LA = 718,
    POSTFIXOP = 719,
    UMINUS = 720
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
#define ATTACH 296
#define ATTRIBUTE 297
#define AUTHORIZATION 298
#define BACKWARD 299
#define BEFORE 300
#define BEGIN_P 301
#define BETWEEN 302
#define BIGINT 303
#define BINARY 304
#define BIT 305
#define BOOLEAN_P 306
#define BOTH 307
#define BY 308
#define CACHE 309
#define CALL 310
#define CALLED 311
#define CASCADE 312
#define CASCADED 313
#define CASE 314
#define CAST 315
#define CATALOG_P 316
#define CHAIN 317
#define CHAR_P 318
#define CHARACTER 319
#define CHARACTERISTICS 320
#define CHECK 321
#define CHECKPOINT 322
#define CLASS 323
#define CLOSE 324
#define CLUSTER 325
#define COALESCE 326
#define COLLATE 327
#define COLLATION 328
#define COLUMN 329
#define COLUMNS 330
#define COMMENT 331
#define COMMENTS 332
#define COMMIT 333
#define COMMITTED 334
#define CONCURRENTLY 335
#define CONFIGURATION 336
#define CONFLICT 337
#define CONNECTION 338
#define CONSTRAINT 339
#define CONSTRAINTS 340
#define CONTENT_P 341
#define CONTINUE_P 342
#define CONVERSION_P 343
#define COPY 344
#define COST 345
#define CREATE 346
#define CROSS 347
#define CSV 348
#define CUBE 349
#define CURRENT_P 350
#define CURRENT_CATALOG 351
#define CURRENT_DATE 352
#define CURRENT_ROLE 353
#define CURRENT_SCHEMA 354
#define CURRENT_TIME 355
#define CURRENT_TIMESTAMP 356
#define CURRENT_USER 357
#define CURSOR 358
#define CYCLE 359
#define DATA_P 360
#define DATABASE 361
#define DAY_P 362
#define DEALLOCATE 363
#define DEC 364
#define DECIMAL_P 365
#define DECLARE 366
#define DEFAULT 367
#define DEFAULTS 368
#define DEFERRABLE 369
#define DEFERRED 370
#define DEFINER 371
#define DELETE_P 372
#define DELIMITER 373
#define DELIMITERS 374
#define DEPENDS 375
#define DESC 376
#define DETACH 377
#define DICTIONARY 378
#define DISABLE_P 379
#define DISCARD 380
#define DISTINCT 381
#define DO 382
#define DOCUMENT_P 383
#define DOMAIN_P 384
#define DOUBLE_P 385
#define DROP 386
#define EACH 387
#define ELSE 388
#define ENABLE_P 389
#define ENCODING 390
#define ENCRYPTED 391
#define END_P 392
#define ENUM_P 393
#define ESCAPE 394
#define EVENT 395
#define EXCEPT 396
#define EXCLUDE 397
#define EXCLUDING 398
#define EXCLUSIVE 399
#define EXECUTE 400
#define EXISTS 401
#define EXPLAIN 402
#define EXTENSION 403
#define EXTERNAL 404
#define EXTRACT 405
#define FALSE_P 406
#define FAMILY 407
#define FETCH 408
#define FILTER 409
#define FIRST_P 410
#define FLOAT_P 411
#define FOLLOWING 412
#define FOR 413
#define FORCE 414
#define FOREIGN 415
#define FORWARD 416
#define FREEZE 417
#define FROM 418
#define FULL 419
#define FUNCTION 420
#define FUNCTIONS 421
#define GENERATED 422
#define GLOBAL 423
#define GRANT 424
#define GRANTED 425
#define GREATEST 426
#define GROUP_P 427
#define GROUPING 428
#define GROUPS 429
#define HANDLER 430
#define HAVING 431
#define HEADER_P 432
#define HOLD 433
#define HOUR_P 434
#define IDENTITY_P 435
#define IF_P 436
#define ILIKE 437
#define IMMEDIATE 438
#define IMMUTABLE 439
#define IMPLICIT_P 440
#define IMPORT_P 441
#define IN_P 442
#define INCLUDE 443
#define INCLUDING 444
#define INCREMENT 445
#define INDEX 446
#define INDEXES 447
#define INHERIT 448
#define INHERITS 449
#define INITIALLY 450
#define INLINE_P 451
#define INNER_P 452
#define INOUT 453
#define INPUT_P 454
#define INSENSITIVE 455
#define INSERT 456
#define INSTEAD 457
#define INT_P 458
#define INTEGER 459
#define INTERSECT 460
#define INTERVAL 461
#define INTO 462
#define INVOKER 463
#define IS 464
#define ISNULL 465
#define ISOLATION 466
#define JOIN 467
#define KEY 468
#define LABEL 469
#define LANGUAGE 470
#define LARGE_P 471
#define LAST_P 472
#define LATERAL_P 473
#define LEADING 474
#define LEAKPROOF 475
#define LEAST 476
#define LEFT 477
#define LEVEL 478
#define LIKE 479
#define LIMIT 480
#define LISTEN 481
#define LOAD 482
#define LOCAL 483
#define LOCALTIME 484
#define LOCALTIMESTAMP 485
#define LOCATION 486
#define LOCK_P 487
#define LOCKED 488
#define LOGGED 489
#define MAPPING 490
#define MATCH 491
#define MATERIALIZED 492
#define MAXVALUE 493
#define METHOD 494
#define MINUTE_P 495
#define MINVALUE 496
#define MODE 497
#define MONTH_P 498
#define MOVE 499
#define NAME_P 500
#define NAMES 501
#define NATIONAL 502
#define NATURAL 503
#define NCHAR 504
#define NEW 505
#define NEXT 506
#define NO 507
#define NONE 508
#define NOT 509
#define NOTHING 510
#define NOTIFY 511
#define NOTNULL 512
#define NOWAIT 513
#define NULL_P 514
#define NULLIF 515
#define NULLS_P 516
#define NUMERIC 517
#define OBJECT_P 518
#define OF 519
#define OFF 520
#define OFFSET 521
#define OIDS 522
#define OLD 523
#define ON 524
#define ONLY 525
#define OPERATOR 526
#define OPTION 527
#define OPTIONS 528
#define OR 529
#define ORDER 530
#define ORDINALITY 531
#define OTHERS 532
#define OUT_P 533
#define OUTER_P 534
#define OVER 535
#define OVERLAPS 536
#define OVERLAY 537
#define OVERRIDING 538
#define OWNED 539
#define OWNER 540
#define PARALLEL 541
#define PARSER 542
#define PARTIAL 543
#define PARTITION 544
#define PASSING 545
#define PASSWORD 546
#define PGPOOL 547
#define PLACING 548
#define PLANS 549
#define POLICY 550
#define POSITION 551
#define PRECEDING 552
#define PRECISION 553
#define PRESERVE 554
#define PREPARE 555
#define PREPARED 556
#define PRIMARY 557
#define PRIOR 558
#define PRIVILEGES 559
#define PROCEDURAL 560
#define PROCEDURE 561
#define PROCEDURES 562
#define PROGRAM 563
#define PUBLICATION 564
#define QUOTE 565
#define RANGE 566
#define READ 567
#define REAL 568
#define REASSIGN 569
#define RECHECK 570
#define RECURSIVE 571
#define REF 572
#define REFERENCES 573
#define REFERENCING 574
#define REFRESH 575
#define REINDEX 576
#define RELATIVE_P 577
#define RELEASE 578
#define RENAME 579
#define REPEATABLE 580
#define REPLACE 581
#define REPLICA 582
#define RESET 583
#define RESTART 584
#define RESTRICT 585
#define RETURNING 586
#define RETURNS 587
#define REVOKE 588
#define RIGHT 589
#define ROLE 590
#define ROLLBACK 591
#define ROLLUP 592
#define ROUTINE 593
#define ROUTINES 594
#define ROW 595
#define ROWS 596
#define RULE 597
#define SAVEPOINT 598
#define SCHEMA 599
#define SCHEMAS 600
#define SCROLL 601
#define SEARCH 602
#define SECOND_P 603
#define SECURITY 604
#define SELECT 605
#define SEQUENCE 606
#define SEQUENCES 607
#define SERIALIZABLE 608
#define SERVER 609
#define SESSION 610
#define SESSION_USER 611
#define SET 612
#define SETS 613
#define SETOF 614
#define SHARE 615
#define SHOW 616
#define SIMILAR 617
#define SIMPLE 618
#define SKIP 619
#define SMALLINT 620
#define SNAPSHOT 621
#define SOME 622
#define SQL_P 623
#define STABLE 624
#define STANDALONE_P 625
#define START 626
#define STATEMENT 627
#define STATISTICS 628
#define STDIN 629
#define STDOUT 630
#define STORAGE 631
#define STORED 632
#define STRICT_P 633
#define STRIP_P 634
#define SUBSCRIPTION 635
#define SUBSTRING 636
#define SUPPORT 637
#define SYMMETRIC 638
#define SYSID 639
#define SYSTEM_P 640
#define TABLE 641
#define TABLES 642
#define TABLESAMPLE 643
#define TABLESPACE 644
#define TEMP 645
#define TEMPLATE 646
#define TEMPORARY 647
#define TEXT_P 648
#define THEN 649
#define TIES 650
#define TIME 651
#define TIMESTAMP 652
#define TO 653
#define TRAILING 654
#define TRANSACTION 655
#define TRANSFORM 656
#define TREAT 657
#define TRIGGER 658
#define TRIM 659
#define TRUE_P 660
#define TRUNCATE 661
#define TRUSTED 662
#define TYPE_P 663
#define TYPES_P 664
#define UNBOUNDED 665
#define UNCOMMITTED 666
#define UNENCRYPTED 667
#define UNION 668
#define UNIQUE 669
#define UNKNOWN 670
#define UNLISTEN 671
#define UNLOGGED 672
#define UNTIL 673
#define UPDATE 674
#define USER 675
#define USING 676
#define VACUUM 677
#define VALID 678
#define VALIDATE 679
#define VALIDATOR 680
#define VALUE_P 681
#define VALUES 682
#define VARCHAR 683
#define VARIADIC 684
#define VARYING 685
#define VERBOSE 686
#define VERSION_P 687
#define VIEW 688
#define VIEWS 689
#define VOLATILE 690
#define WHEN 691
#define WHERE 692
#define WHITESPACE_P 693
#define WINDOW 694
#define WITH 695
#define WITHIN 696
#define WITHOUT 697
#define WORK 698
#define WRAPPER 699
#define WRITE 700
#define XML_P 701
#define XMLATTRIBUTES 702
#define XMLCONCAT 703
#define XMLELEMENT 704
#define XMLEXISTS 705
#define XMLFOREST 706
#define XMLNAMESPACES 707
#define XMLPARSE 708
#define XMLPI 709
#define XMLROOT 710
#define XMLSERIALIZE 711
#define XMLTABLE 712
#define YEAR_P 713
#define YES_P 714
#define ZONE 715
#define NOT_LA 716
#define NULLS_LA 717
#define WITH_LA 718
#define POSTFIXOP 719
#define UMINUS 720

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 239 "gram_minimal.y" /* yacc.c:1909  */

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
	ObjectWithArgs		*objwithargs;
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
	PartitionElem		*partelem;
	PartitionSpec		*partspec;
	PartitionBoundSpec	*partboundspec;
	RoleSpec			*rolespec;

#line 1028 "gram_minimal.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int minimal_base_yyparse (core_yyscan_t yyscanner);

#endif /* !YY_MINIMAL_BASE_YY_GRAM_MINIMAL_H_INCLUDED  */
