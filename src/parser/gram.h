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

#ifndef YY_BASE_YY_GRAM_H_INCLUDED
# define YY_BASE_YY_GRAM_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int base_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENT = 258,
    UIDENT = 259,
    FCONST = 260,
    SCONST = 261,
    USCONST = 262,
    BCONST = 263,
    XCONST = 264,
    Op = 265,
    ICONST = 266,
    PARAM = 267,
    TYPECAST = 268,
    DOT_DOT = 269,
    COLON_EQUALS = 270,
    EQUALS_GREATER = 271,
    LESS_EQUALS = 272,
    GREATER_EQUALS = 273,
    NOT_EQUALS = 274,
    ABORT_P = 275,
    ABSOLUTE_P = 276,
    ACCESS = 277,
    ACTION = 278,
    ADD_P = 279,
    ADMIN = 280,
    AFTER = 281,
    AGGREGATE = 282,
    ALL = 283,
    ALSO = 284,
    ALTER = 285,
    ALWAYS = 286,
    ANALYSE = 287,
    ANALYZE = 288,
    AND = 289,
    ANY = 290,
    ARRAY = 291,
    AS = 292,
    ASC = 293,
    ASSERTION = 294,
    ASSIGNMENT = 295,
    ASYMMETRIC = 296,
    AT = 297,
    ATTACH = 298,
    ATTRIBUTE = 299,
    AUTHORIZATION = 300,
    BACKWARD = 301,
    BEFORE = 302,
    BEGIN_P = 303,
    BETWEEN = 304,
    BIGINT = 305,
    BINARY = 306,
    BIT = 307,
    BOOLEAN_P = 308,
    BOTH = 309,
    BY = 310,
    CACHE = 311,
    CALL = 312,
    CALLED = 313,
    CASCADE = 314,
    CASCADED = 315,
    CASE = 316,
    CAST = 317,
    CATALOG_P = 318,
    CHAIN = 319,
    CHAR_P = 320,
    CHARACTER = 321,
    CHARACTERISTICS = 322,
    CHECK = 323,
    CHECKPOINT = 324,
    CLASS = 325,
    CLOSE = 326,
    CLUSTER = 327,
    COALESCE = 328,
    COLLATE = 329,
    COLLATION = 330,
    COLUMN = 331,
    COLUMNS = 332,
    COMMENT = 333,
    COMMENTS = 334,
    COMMIT = 335,
    COMMITTED = 336,
    CONCURRENTLY = 337,
    CONFIGURATION = 338,
    CONFLICT = 339,
    CONNECTION = 340,
    CONSTRAINT = 341,
    CONSTRAINTS = 342,
    CONTENT_P = 343,
    CONTINUE_P = 344,
    CONVERSION_P = 345,
    COPY = 346,
    COST = 347,
    CREATE = 348,
    CROSS = 349,
    CSV = 350,
    CUBE = 351,
    CURRENT_P = 352,
    CURRENT_CATALOG = 353,
    CURRENT_DATE = 354,
    CURRENT_ROLE = 355,
    CURRENT_SCHEMA = 356,
    CURRENT_TIME = 357,
    CURRENT_TIMESTAMP = 358,
    CURRENT_USER = 359,
    CURSOR = 360,
    CYCLE = 361,
    DATA_P = 362,
    DATABASE = 363,
    DAY_P = 364,
    DEALLOCATE = 365,
    DEC = 366,
    DECIMAL_P = 367,
    DECLARE = 368,
    DEFAULT = 369,
    DEFAULTS = 370,
    DEFERRABLE = 371,
    DEFERRED = 372,
    DEFINER = 373,
    DELETE_P = 374,
    DELIMITER = 375,
    DELIMITERS = 376,
    DEPENDS = 377,
    DESC = 378,
    DETACH = 379,
    DICTIONARY = 380,
    DISABLE_P = 381,
    DISCARD = 382,
    DISTINCT = 383,
    DO = 384,
    DOCUMENT_P = 385,
    DOMAIN_P = 386,
    DOUBLE_P = 387,
    DROP = 388,
    EACH = 389,
    ELSE = 390,
    ENABLE_P = 391,
    ENCODING = 392,
    ENCRYPTED = 393,
    END_P = 394,
    ENUM_P = 395,
    ESCAPE = 396,
    EVENT = 397,
    EXCEPT = 398,
    EXCLUDE = 399,
    EXCLUDING = 400,
    EXCLUSIVE = 401,
    EXECUTE = 402,
    EXISTS = 403,
    EXPLAIN = 404,
    EXPRESSION = 405,
    EXTENSION = 406,
    EXTERNAL = 407,
    EXTRACT = 408,
    FALSE_P = 409,
    FAMILY = 410,
    FETCH = 411,
    FILTER = 412,
    FIRST_P = 413,
    FLOAT_P = 414,
    FOLLOWING = 415,
    FOR = 416,
    FORCE = 417,
    FOREIGN = 418,
    FORWARD = 419,
    FREEZE = 420,
    FROM = 421,
    FULL = 422,
    FUNCTION = 423,
    FUNCTIONS = 424,
    GENERATED = 425,
    GLOBAL = 426,
    GRANT = 427,
    GRANTED = 428,
    GREATEST = 429,
    GROUP_P = 430,
    GROUPING = 431,
    GROUPS = 432,
    HANDLER = 433,
    HAVING = 434,
    HEADER_P = 435,
    HOLD = 436,
    HOUR_P = 437,
    IDENTITY_P = 438,
    IF_P = 439,
    ILIKE = 440,
    IMMEDIATE = 441,
    IMMUTABLE = 442,
    IMPLICIT_P = 443,
    IMPORT_P = 444,
    IN_P = 445,
    INCLUDE = 446,
    INCLUDING = 447,
    INCREMENT = 448,
    INDEX = 449,
    INDEXES = 450,
    INHERIT = 451,
    INHERITS = 452,
    INITIALLY = 453,
    INLINE_P = 454,
    INNER_P = 455,
    INOUT = 456,
    INPUT_P = 457,
    INSENSITIVE = 458,
    INSERT = 459,
    INSTEAD = 460,
    INT_P = 461,
    INTEGER = 462,
    INTERSECT = 463,
    INTERVAL = 464,
    INTO = 465,
    INVOKER = 466,
    IS = 467,
    ISNULL = 468,
    ISOLATION = 469,
    JOIN = 470,
    KEY = 471,
    LABEL = 472,
    LANGUAGE = 473,
    LARGE_P = 474,
    LAST_P = 475,
    LATERAL_P = 476,
    LEADING = 477,
    LEAKPROOF = 478,
    LEAST = 479,
    LEFT = 480,
    LEVEL = 481,
    LIKE = 482,
    LIMIT = 483,
    LISTEN = 484,
    LOAD = 485,
    LOCAL = 486,
    LOCALTIME = 487,
    LOCALTIMESTAMP = 488,
    LOCATION = 489,
    LOCK_P = 490,
    LOCKED = 491,
    LOGGED = 492,
    MAPPING = 493,
    MATCH = 494,
    MATERIALIZED = 495,
    MAXVALUE = 496,
    METHOD = 497,
    MINUTE_P = 498,
    MINVALUE = 499,
    MODE = 500,
    MONTH_P = 501,
    MOVE = 502,
    NAME_P = 503,
    NAMES = 504,
    NATIONAL = 505,
    NATURAL = 506,
    NCHAR = 507,
    NEW = 508,
    NEXT = 509,
    NFC = 510,
    NFD = 511,
    NFKC = 512,
    NFKD = 513,
    NO = 514,
    NONE = 515,
    NORMALIZE = 516,
    NORMALIZED = 517,
    NOT = 518,
    NOTHING = 519,
    NOTIFY = 520,
    NOTNULL = 521,
    NOWAIT = 522,
    NULL_P = 523,
    NULLIF = 524,
    NULLS_P = 525,
    NUMERIC = 526,
    OBJECT_P = 527,
    OF = 528,
    OFF = 529,
    OFFSET = 530,
    OIDS = 531,
    OLD = 532,
    ON = 533,
    ONLY = 534,
    OPERATOR = 535,
    OPTION = 536,
    OPTIONS = 537,
    OR = 538,
    ORDER = 539,
    ORDINALITY = 540,
    OTHERS = 541,
    OUT_P = 542,
    OUTER_P = 543,
    OVER = 544,
    OVERLAPS = 545,
    OVERLAY = 546,
    OVERRIDING = 547,
    OWNED = 548,
    OWNER = 549,
    PARALLEL = 550,
    PARSER = 551,
    PARTIAL = 552,
    PARTITION = 553,
    PASSING = 554,
    PASSWORD = 555,
    PGPOOL = 556,
    PLACING = 557,
    PLANS = 558,
    POLICY = 559,
    POSITION = 560,
    PRECEDING = 561,
    PRECISION = 562,
    PRESERVE = 563,
    PREPARE = 564,
    PREPARED = 565,
    PRIMARY = 566,
    PRIOR = 567,
    PRIVILEGES = 568,
    PROCEDURAL = 569,
    PROCEDURE = 570,
    PROCEDURES = 571,
    PROGRAM = 572,
    PUBLICATION = 573,
    QUOTE = 574,
    RANGE = 575,
    READ = 576,
    REAL = 577,
    REASSIGN = 578,
    RECHECK = 579,
    RECURSIVE = 580,
    REF = 581,
    REFERENCES = 582,
    REFERENCING = 583,
    REFRESH = 584,
    REINDEX = 585,
    RELATIVE_P = 586,
    RELEASE = 587,
    RENAME = 588,
    REPEATABLE = 589,
    REPLACE = 590,
    REPLICA = 591,
    RESET = 592,
    RESTART = 593,
    RESTRICT = 594,
    RETURNING = 595,
    RETURNS = 596,
    REVOKE = 597,
    RIGHT = 598,
    ROLE = 599,
    ROLLBACK = 600,
    ROLLUP = 601,
    ROUTINE = 602,
    ROUTINES = 603,
    ROW = 604,
    ROWS = 605,
    RULE = 606,
    SAVEPOINT = 607,
    SCHEMA = 608,
    SCHEMAS = 609,
    SCROLL = 610,
    SEARCH = 611,
    SECOND_P = 612,
    SECURITY = 613,
    SELECT = 614,
    SEQUENCE = 615,
    SEQUENCES = 616,
    SERIALIZABLE = 617,
    SERVER = 618,
    SESSION = 619,
    SESSION_USER = 620,
    SET = 621,
    SETS = 622,
    SETOF = 623,
    SHARE = 624,
    SHOW = 625,
    SIMILAR = 626,
    SIMPLE = 627,
    SKIP = 628,
    SMALLINT = 629,
    SNAPSHOT = 630,
    SOME = 631,
    SQL_P = 632,
    STABLE = 633,
    STANDALONE_P = 634,
    START = 635,
    STATEMENT = 636,
    STATISTICS = 637,
    STDIN = 638,
    STDOUT = 639,
    STORAGE = 640,
    STORED = 641,
    STRICT_P = 642,
    STRIP_P = 643,
    SUBSCRIPTION = 644,
    SUBSTRING = 645,
    SUPPORT = 646,
    SYMMETRIC = 647,
    SYSID = 648,
    SYSTEM_P = 649,
    TABLE = 650,
    TABLES = 651,
    TABLESAMPLE = 652,
    TABLESPACE = 653,
    TEMP = 654,
    TEMPLATE = 655,
    TEMPORARY = 656,
    TEXT_P = 657,
    THEN = 658,
    TIES = 659,
    TIME = 660,
    TIMESTAMP = 661,
    TO = 662,
    TRAILING = 663,
    TRANSACTION = 664,
    TRANSFORM = 665,
    TREAT = 666,
    TRIGGER = 667,
    TRIM = 668,
    TRUE_P = 669,
    TRUNCATE = 670,
    TRUSTED = 671,
    TYPE_P = 672,
    TYPES_P = 673,
    UESCAPE = 674,
    UNBOUNDED = 675,
    UNCOMMITTED = 676,
    UNENCRYPTED = 677,
    UNION = 678,
    UNIQUE = 679,
    UNKNOWN = 680,
    UNLISTEN = 681,
    UNLOGGED = 682,
    UNTIL = 683,
    UPDATE = 684,
    USER = 685,
    USING = 686,
    VACUUM = 687,
    VALID = 688,
    VALIDATE = 689,
    VALIDATOR = 690,
    VALUE_P = 691,
    VALUES = 692,
    VARCHAR = 693,
    VARIADIC = 694,
    VARYING = 695,
    VERBOSE = 696,
    VERSION_P = 697,
    VIEW = 698,
    VIEWS = 699,
    VOLATILE = 700,
    WHEN = 701,
    WHERE = 702,
    WHITESPACE_P = 703,
    WINDOW = 704,
    WITH = 705,
    WITHIN = 706,
    WITHOUT = 707,
    WORK = 708,
    WRAPPER = 709,
    WRITE = 710,
    XML_P = 711,
    XMLATTRIBUTES = 712,
    XMLCONCAT = 713,
    XMLELEMENT = 714,
    XMLEXISTS = 715,
    XMLFOREST = 716,
    XMLNAMESPACES = 717,
    XMLPARSE = 718,
    XMLPI = 719,
    XMLROOT = 720,
    XMLSERIALIZE = 721,
    XMLTABLE = 722,
    YEAR_P = 723,
    YES_P = 724,
    ZONE = 725,
    NOT_LA = 726,
    NULLS_LA = 727,
    WITH_LA = 728,
    POSTFIXOP = 729,
    UMINUS = 730
  };
#endif
/* Tokens.  */
#define IDENT 258
#define UIDENT 259
#define FCONST 260
#define SCONST 261
#define USCONST 262
#define BCONST 263
#define XCONST 264
#define Op 265
#define ICONST 266
#define PARAM 267
#define TYPECAST 268
#define DOT_DOT 269
#define COLON_EQUALS 270
#define EQUALS_GREATER 271
#define LESS_EQUALS 272
#define GREATER_EQUALS 273
#define NOT_EQUALS 274
#define ABORT_P 275
#define ABSOLUTE_P 276
#define ACCESS 277
#define ACTION 278
#define ADD_P 279
#define ADMIN 280
#define AFTER 281
#define AGGREGATE 282
#define ALL 283
#define ALSO 284
#define ALTER 285
#define ALWAYS 286
#define ANALYSE 287
#define ANALYZE 288
#define AND 289
#define ANY 290
#define ARRAY 291
#define AS 292
#define ASC 293
#define ASSERTION 294
#define ASSIGNMENT 295
#define ASYMMETRIC 296
#define AT 297
#define ATTACH 298
#define ATTRIBUTE 299
#define AUTHORIZATION 300
#define BACKWARD 301
#define BEFORE 302
#define BEGIN_P 303
#define BETWEEN 304
#define BIGINT 305
#define BINARY 306
#define BIT 307
#define BOOLEAN_P 308
#define BOTH 309
#define BY 310
#define CACHE 311
#define CALL 312
#define CALLED 313
#define CASCADE 314
#define CASCADED 315
#define CASE 316
#define CAST 317
#define CATALOG_P 318
#define CHAIN 319
#define CHAR_P 320
#define CHARACTER 321
#define CHARACTERISTICS 322
#define CHECK 323
#define CHECKPOINT 324
#define CLASS 325
#define CLOSE 326
#define CLUSTER 327
#define COALESCE 328
#define COLLATE 329
#define COLLATION 330
#define COLUMN 331
#define COLUMNS 332
#define COMMENT 333
#define COMMENTS 334
#define COMMIT 335
#define COMMITTED 336
#define CONCURRENTLY 337
#define CONFIGURATION 338
#define CONFLICT 339
#define CONNECTION 340
#define CONSTRAINT 341
#define CONSTRAINTS 342
#define CONTENT_P 343
#define CONTINUE_P 344
#define CONVERSION_P 345
#define COPY 346
#define COST 347
#define CREATE 348
#define CROSS 349
#define CSV 350
#define CUBE 351
#define CURRENT_P 352
#define CURRENT_CATALOG 353
#define CURRENT_DATE 354
#define CURRENT_ROLE 355
#define CURRENT_SCHEMA 356
#define CURRENT_TIME 357
#define CURRENT_TIMESTAMP 358
#define CURRENT_USER 359
#define CURSOR 360
#define CYCLE 361
#define DATA_P 362
#define DATABASE 363
#define DAY_P 364
#define DEALLOCATE 365
#define DEC 366
#define DECIMAL_P 367
#define DECLARE 368
#define DEFAULT 369
#define DEFAULTS 370
#define DEFERRABLE 371
#define DEFERRED 372
#define DEFINER 373
#define DELETE_P 374
#define DELIMITER 375
#define DELIMITERS 376
#define DEPENDS 377
#define DESC 378
#define DETACH 379
#define DICTIONARY 380
#define DISABLE_P 381
#define DISCARD 382
#define DISTINCT 383
#define DO 384
#define DOCUMENT_P 385
#define DOMAIN_P 386
#define DOUBLE_P 387
#define DROP 388
#define EACH 389
#define ELSE 390
#define ENABLE_P 391
#define ENCODING 392
#define ENCRYPTED 393
#define END_P 394
#define ENUM_P 395
#define ESCAPE 396
#define EVENT 397
#define EXCEPT 398
#define EXCLUDE 399
#define EXCLUDING 400
#define EXCLUSIVE 401
#define EXECUTE 402
#define EXISTS 403
#define EXPLAIN 404
#define EXPRESSION 405
#define EXTENSION 406
#define EXTERNAL 407
#define EXTRACT 408
#define FALSE_P 409
#define FAMILY 410
#define FETCH 411
#define FILTER 412
#define FIRST_P 413
#define FLOAT_P 414
#define FOLLOWING 415
#define FOR 416
#define FORCE 417
#define FOREIGN 418
#define FORWARD 419
#define FREEZE 420
#define FROM 421
#define FULL 422
#define FUNCTION 423
#define FUNCTIONS 424
#define GENERATED 425
#define GLOBAL 426
#define GRANT 427
#define GRANTED 428
#define GREATEST 429
#define GROUP_P 430
#define GROUPING 431
#define GROUPS 432
#define HANDLER 433
#define HAVING 434
#define HEADER_P 435
#define HOLD 436
#define HOUR_P 437
#define IDENTITY_P 438
#define IF_P 439
#define ILIKE 440
#define IMMEDIATE 441
#define IMMUTABLE 442
#define IMPLICIT_P 443
#define IMPORT_P 444
#define IN_P 445
#define INCLUDE 446
#define INCLUDING 447
#define INCREMENT 448
#define INDEX 449
#define INDEXES 450
#define INHERIT 451
#define INHERITS 452
#define INITIALLY 453
#define INLINE_P 454
#define INNER_P 455
#define INOUT 456
#define INPUT_P 457
#define INSENSITIVE 458
#define INSERT 459
#define INSTEAD 460
#define INT_P 461
#define INTEGER 462
#define INTERSECT 463
#define INTERVAL 464
#define INTO 465
#define INVOKER 466
#define IS 467
#define ISNULL 468
#define ISOLATION 469
#define JOIN 470
#define KEY 471
#define LABEL 472
#define LANGUAGE 473
#define LARGE_P 474
#define LAST_P 475
#define LATERAL_P 476
#define LEADING 477
#define LEAKPROOF 478
#define LEAST 479
#define LEFT 480
#define LEVEL 481
#define LIKE 482
#define LIMIT 483
#define LISTEN 484
#define LOAD 485
#define LOCAL 486
#define LOCALTIME 487
#define LOCALTIMESTAMP 488
#define LOCATION 489
#define LOCK_P 490
#define LOCKED 491
#define LOGGED 492
#define MAPPING 493
#define MATCH 494
#define MATERIALIZED 495
#define MAXVALUE 496
#define METHOD 497
#define MINUTE_P 498
#define MINVALUE 499
#define MODE 500
#define MONTH_P 501
#define MOVE 502
#define NAME_P 503
#define NAMES 504
#define NATIONAL 505
#define NATURAL 506
#define NCHAR 507
#define NEW 508
#define NEXT 509
#define NFC 510
#define NFD 511
#define NFKC 512
#define NFKD 513
#define NO 514
#define NONE 515
#define NORMALIZE 516
#define NORMALIZED 517
#define NOT 518
#define NOTHING 519
#define NOTIFY 520
#define NOTNULL 521
#define NOWAIT 522
#define NULL_P 523
#define NULLIF 524
#define NULLS_P 525
#define NUMERIC 526
#define OBJECT_P 527
#define OF 528
#define OFF 529
#define OFFSET 530
#define OIDS 531
#define OLD 532
#define ON 533
#define ONLY 534
#define OPERATOR 535
#define OPTION 536
#define OPTIONS 537
#define OR 538
#define ORDER 539
#define ORDINALITY 540
#define OTHERS 541
#define OUT_P 542
#define OUTER_P 543
#define OVER 544
#define OVERLAPS 545
#define OVERLAY 546
#define OVERRIDING 547
#define OWNED 548
#define OWNER 549
#define PARALLEL 550
#define PARSER 551
#define PARTIAL 552
#define PARTITION 553
#define PASSING 554
#define PASSWORD 555
#define PGPOOL 556
#define PLACING 557
#define PLANS 558
#define POLICY 559
#define POSITION 560
#define PRECEDING 561
#define PRECISION 562
#define PRESERVE 563
#define PREPARE 564
#define PREPARED 565
#define PRIMARY 566
#define PRIOR 567
#define PRIVILEGES 568
#define PROCEDURAL 569
#define PROCEDURE 570
#define PROCEDURES 571
#define PROGRAM 572
#define PUBLICATION 573
#define QUOTE 574
#define RANGE 575
#define READ 576
#define REAL 577
#define REASSIGN 578
#define RECHECK 579
#define RECURSIVE 580
#define REF 581
#define REFERENCES 582
#define REFERENCING 583
#define REFRESH 584
#define REINDEX 585
#define RELATIVE_P 586
#define RELEASE 587
#define RENAME 588
#define REPEATABLE 589
#define REPLACE 590
#define REPLICA 591
#define RESET 592
#define RESTART 593
#define RESTRICT 594
#define RETURNING 595
#define RETURNS 596
#define REVOKE 597
#define RIGHT 598
#define ROLE 599
#define ROLLBACK 600
#define ROLLUP 601
#define ROUTINE 602
#define ROUTINES 603
#define ROW 604
#define ROWS 605
#define RULE 606
#define SAVEPOINT 607
#define SCHEMA 608
#define SCHEMAS 609
#define SCROLL 610
#define SEARCH 611
#define SECOND_P 612
#define SECURITY 613
#define SELECT 614
#define SEQUENCE 615
#define SEQUENCES 616
#define SERIALIZABLE 617
#define SERVER 618
#define SESSION 619
#define SESSION_USER 620
#define SET 621
#define SETS 622
#define SETOF 623
#define SHARE 624
#define SHOW 625
#define SIMILAR 626
#define SIMPLE 627
#define SKIP 628
#define SMALLINT 629
#define SNAPSHOT 630
#define SOME 631
#define SQL_P 632
#define STABLE 633
#define STANDALONE_P 634
#define START 635
#define STATEMENT 636
#define STATISTICS 637
#define STDIN 638
#define STDOUT 639
#define STORAGE 640
#define STORED 641
#define STRICT_P 642
#define STRIP_P 643
#define SUBSCRIPTION 644
#define SUBSTRING 645
#define SUPPORT 646
#define SYMMETRIC 647
#define SYSID 648
#define SYSTEM_P 649
#define TABLE 650
#define TABLES 651
#define TABLESAMPLE 652
#define TABLESPACE 653
#define TEMP 654
#define TEMPLATE 655
#define TEMPORARY 656
#define TEXT_P 657
#define THEN 658
#define TIES 659
#define TIME 660
#define TIMESTAMP 661
#define TO 662
#define TRAILING 663
#define TRANSACTION 664
#define TRANSFORM 665
#define TREAT 666
#define TRIGGER 667
#define TRIM 668
#define TRUE_P 669
#define TRUNCATE 670
#define TRUSTED 671
#define TYPE_P 672
#define TYPES_P 673
#define UESCAPE 674
#define UNBOUNDED 675
#define UNCOMMITTED 676
#define UNENCRYPTED 677
#define UNION 678
#define UNIQUE 679
#define UNKNOWN 680
#define UNLISTEN 681
#define UNLOGGED 682
#define UNTIL 683
#define UPDATE 684
#define USER 685
#define USING 686
#define VACUUM 687
#define VALID 688
#define VALIDATE 689
#define VALIDATOR 690
#define VALUE_P 691
#define VALUES 692
#define VARCHAR 693
#define VARIADIC 694
#define VARYING 695
#define VERBOSE 696
#define VERSION_P 697
#define VIEW 698
#define VIEWS 699
#define VOLATILE 700
#define WHEN 701
#define WHERE 702
#define WHITESPACE_P 703
#define WINDOW 704
#define WITH 705
#define WITHIN 706
#define WITHOUT 707
#define WORK 708
#define WRAPPER 709
#define WRITE 710
#define XML_P 711
#define XMLATTRIBUTES 712
#define XMLCONCAT 713
#define XMLELEMENT 714
#define XMLEXISTS 715
#define XMLFOREST 716
#define XMLNAMESPACES 717
#define XMLPARSE 718
#define XMLPI 719
#define XMLROOT 720
#define XMLSERIALIZE 721
#define XMLTABLE 722
#define YEAR_P 723
#define YES_P 724
#define ZONE 725
#define NOT_LA 726
#define NULLS_LA 727
#define WITH_LA 728
#define POSTFIXOP 729
#define UMINUS 730

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 247 "gram.y" /* yacc.c:1909  */

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
	struct SelectLimit	*selectlimit;

#line 1049 "gram.h" /* yacc.c:1909  */
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



int base_yyparse (core_yyscan_t yyscanner);

#endif /* !YY_BASE_YY_GRAM_H_INCLUDED  */
