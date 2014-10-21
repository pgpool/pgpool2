/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
    EVENT = 384,
    EXCEPT = 385,
    EXCLUDE = 386,
    EXCLUDING = 387,
    EXCLUSIVE = 388,
    EXECUTE = 389,
    EXISTS = 390,
    EXPLAIN = 391,
    EXTENSION = 392,
    EXTERNAL = 393,
    EXTRACT = 394,
    FALSE_P = 395,
    FAMILY = 396,
    FETCH = 397,
    FILTER = 398,
    FIRST_P = 399,
    FLOAT_P = 400,
    FOLLOWING = 401,
    FOR = 402,
    FORCE = 403,
    FOREIGN = 404,
    FORWARD = 405,
    FREEZE = 406,
    FROM = 407,
    FULL = 408,
    FUNCTION = 409,
    FUNCTIONS = 410,
    GLOBAL = 411,
    GRANT = 412,
    GRANTED = 413,
    GREATEST = 414,
    GROUP_P = 415,
    HANDLER = 416,
    HAVING = 417,
    HEADER_P = 418,
    HOLD = 419,
    HOUR_P = 420,
    IDENTITY_P = 421,
    IF_P = 422,
    ILIKE = 423,
    IMMEDIATE = 424,
    IMMUTABLE = 425,
    IMPLICIT_P = 426,
    IN_P = 427,
    INCLUDING = 428,
    INCREMENT = 429,
    INDEX = 430,
    INDEXES = 431,
    INHERIT = 432,
    INHERITS = 433,
    INITIALLY = 434,
    INLINE_P = 435,
    INNER_P = 436,
    INOUT = 437,
    INPUT_P = 438,
    INSENSITIVE = 439,
    INSERT = 440,
    INSTEAD = 441,
    INT_P = 442,
    INTEGER = 443,
    INTERSECT = 444,
    INTERVAL = 445,
    INTO = 446,
    INVOKER = 447,
    IS = 448,
    ISNULL = 449,
    ISOLATION = 450,
    JOIN = 451,
    KEY = 452,
    LABEL = 453,
    LANGUAGE = 454,
    LARGE_P = 455,
    LAST_P = 456,
    LATERAL_P = 457,
    LC_COLLATE_P = 458,
    LC_CTYPE_P = 459,
    LEADING = 460,
    LEAKPROOF = 461,
    LEAST = 462,
    LEFT = 463,
    LEVEL = 464,
    LIKE = 465,
    LIMIT = 466,
    LISTEN = 467,
    LOAD = 468,
    LOCAL = 469,
    LOCALTIME = 470,
    LOCALTIMESTAMP = 471,
    LOCATION = 472,
    LOCK_P = 473,
    MAPPING = 474,
    MATCH = 475,
    MATERIALIZED = 476,
    MAXVALUE = 477,
    MINUTE_P = 478,
    MINVALUE = 479,
    MODE = 480,
    MONTH_P = 481,
    MOVE = 482,
    NAME_P = 483,
    NAMES = 484,
    NATIONAL = 485,
    NATURAL = 486,
    NCHAR = 487,
    NEXT = 488,
    NO = 489,
    NONE = 490,
    NOT = 491,
    NOTHING = 492,
    NOTIFY = 493,
    NOTNULL = 494,
    NOWAIT = 495,
    NULL_P = 496,
    NULLIF = 497,
    NULLS_P = 498,
    NUMERIC = 499,
    OBJECT_P = 500,
    OF = 501,
    OFF = 502,
    OFFSET = 503,
    OIDS = 504,
    ON = 505,
    ONLY = 506,
    OPERATOR = 507,
    OPTION = 508,
    OPTIONS = 509,
    OR = 510,
    ORDER = 511,
    ORDINALITY = 512,
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
    PASSING = 523,
    PASSWORD = 524,
    PLACING = 525,
    PLANS = 526,
    POSITION = 527,
    PRECEDING = 528,
    PRECISION = 529,
    PRESERVE = 530,
    PREPARE = 531,
    PREPARED = 532,
    PRIMARY = 533,
    PRIOR = 534,
    PRIVILEGES = 535,
    PROCEDURAL = 536,
    PROCEDURE = 537,
    PROGRAM = 538,
    QUOTE = 539,
    RANGE = 540,
    READ = 541,
    REAL = 542,
    REASSIGN = 543,
    RECHECK = 544,
    RECURSIVE = 545,
    REF = 546,
    REFERENCES = 547,
    REFRESH = 548,
    REINDEX = 549,
    RELATIVE_P = 550,
    RELEASE = 551,
    RENAME = 552,
    REPEATABLE = 553,
    REPLACE = 554,
    REPLICA = 555,
    RESET = 556,
    RESTART = 557,
    RESTRICT = 558,
    RETURNING = 559,
    RETURNS = 560,
    REVOKE = 561,
    RIGHT = 562,
    ROLE = 563,
    ROLLBACK = 564,
    ROW = 565,
    ROWS = 566,
    RULE = 567,
    SAVEPOINT = 568,
    SCHEMA = 569,
    SCROLL = 570,
    SEARCH = 571,
    SECOND_P = 572,
    SECURITY = 573,
    SELECT = 574,
    SEQUENCE = 575,
    SEQUENCES = 576,
    SERIALIZABLE = 577,
    SERVER = 578,
    SESSION = 579,
    SESSION_USER = 580,
    SET = 581,
    SETOF = 582,
    SHARE = 583,
    SHOW = 584,
    SIMILAR = 585,
    SIMPLE = 586,
    SMALLINT = 587,
    SNAPSHOT = 588,
    SOME = 589,
    STABLE = 590,
    STANDALONE_P = 591,
    START = 592,
    STATEMENT = 593,
    STATISTICS = 594,
    STDIN = 595,
    STDOUT = 596,
    STORAGE = 597,
    STRICT_P = 598,
    STRIP_P = 599,
    SUBSTRING = 600,
    SYMMETRIC = 601,
    SYSID = 602,
    SYSTEM_P = 603,
    TABLE = 604,
    TABLES = 605,
    TABLESPACE = 606,
    TEMP = 607,
    TEMPLATE = 608,
    TEMPORARY = 609,
    TEXT_P = 610,
    THEN = 611,
    TIME = 612,
    TIMESTAMP = 613,
    TO = 614,
    TRAILING = 615,
    TRANSACTION = 616,
    TREAT = 617,
    TRIGGER = 618,
    TRIM = 619,
    TRUE_P = 620,
    TRUNCATE = 621,
    TRUSTED = 622,
    TYPE_P = 623,
    TYPES_P = 624,
    UNBOUNDED = 625,
    UNCOMMITTED = 626,
    UNENCRYPTED = 627,
    UNION = 628,
    UNIQUE = 629,
    UNKNOWN = 630,
    UNLISTEN = 631,
    UNLOGGED = 632,
    UNTIL = 633,
    UPDATE = 634,
    USER = 635,
    USING = 636,
    VACUUM = 637,
    VALID = 638,
    VALIDATE = 639,
    VALIDATOR = 640,
    VALUE_P = 641,
    VALUES = 642,
    VARCHAR = 643,
    VARIADIC = 644,
    VARYING = 645,
    VERBOSE = 646,
    VERSION_P = 647,
    VIEW = 648,
    VIEWS = 649,
    VOLATILE = 650,
    WHEN = 651,
    WHERE = 652,
    WHITESPACE_P = 653,
    WINDOW = 654,
    WITH = 655,
    WITHIN = 656,
    WITHOUT = 657,
    WORK = 658,
    WRAPPER = 659,
    WRITE = 660,
    XML_P = 661,
    XMLATTRIBUTES = 662,
    XMLCONCAT = 663,
    XMLELEMENT = 664,
    XMLEXISTS = 665,
    XMLFOREST = 666,
    XMLPARSE = 667,
    XMLPI = 668,
    XMLROOT = 669,
    XMLSERIALIZE = 670,
    YEAR_P = 671,
    YES_P = 672,
    ZONE = 673,
    NULLS_FIRST = 674,
    NULLS_LAST = 675,
    WITH_ORDINALITY = 676,
    WITH_TIME = 677,
    POSTFIXOP = 678,
    UMINUS = 679
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
#define EVENT 384
#define EXCEPT 385
#define EXCLUDE 386
#define EXCLUDING 387
#define EXCLUSIVE 388
#define EXECUTE 389
#define EXISTS 390
#define EXPLAIN 391
#define EXTENSION 392
#define EXTERNAL 393
#define EXTRACT 394
#define FALSE_P 395
#define FAMILY 396
#define FETCH 397
#define FILTER 398
#define FIRST_P 399
#define FLOAT_P 400
#define FOLLOWING 401
#define FOR 402
#define FORCE 403
#define FOREIGN 404
#define FORWARD 405
#define FREEZE 406
#define FROM 407
#define FULL 408
#define FUNCTION 409
#define FUNCTIONS 410
#define GLOBAL 411
#define GRANT 412
#define GRANTED 413
#define GREATEST 414
#define GROUP_P 415
#define HANDLER 416
#define HAVING 417
#define HEADER_P 418
#define HOLD 419
#define HOUR_P 420
#define IDENTITY_P 421
#define IF_P 422
#define ILIKE 423
#define IMMEDIATE 424
#define IMMUTABLE 425
#define IMPLICIT_P 426
#define IN_P 427
#define INCLUDING 428
#define INCREMENT 429
#define INDEX 430
#define INDEXES 431
#define INHERIT 432
#define INHERITS 433
#define INITIALLY 434
#define INLINE_P 435
#define INNER_P 436
#define INOUT 437
#define INPUT_P 438
#define INSENSITIVE 439
#define INSERT 440
#define INSTEAD 441
#define INT_P 442
#define INTEGER 443
#define INTERSECT 444
#define INTERVAL 445
#define INTO 446
#define INVOKER 447
#define IS 448
#define ISNULL 449
#define ISOLATION 450
#define JOIN 451
#define KEY 452
#define LABEL 453
#define LANGUAGE 454
#define LARGE_P 455
#define LAST_P 456
#define LATERAL_P 457
#define LC_COLLATE_P 458
#define LC_CTYPE_P 459
#define LEADING 460
#define LEAKPROOF 461
#define LEAST 462
#define LEFT 463
#define LEVEL 464
#define LIKE 465
#define LIMIT 466
#define LISTEN 467
#define LOAD 468
#define LOCAL 469
#define LOCALTIME 470
#define LOCALTIMESTAMP 471
#define LOCATION 472
#define LOCK_P 473
#define MAPPING 474
#define MATCH 475
#define MATERIALIZED 476
#define MAXVALUE 477
#define MINUTE_P 478
#define MINVALUE 479
#define MODE 480
#define MONTH_P 481
#define MOVE 482
#define NAME_P 483
#define NAMES 484
#define NATIONAL 485
#define NATURAL 486
#define NCHAR 487
#define NEXT 488
#define NO 489
#define NONE 490
#define NOT 491
#define NOTHING 492
#define NOTIFY 493
#define NOTNULL 494
#define NOWAIT 495
#define NULL_P 496
#define NULLIF 497
#define NULLS_P 498
#define NUMERIC 499
#define OBJECT_P 500
#define OF 501
#define OFF 502
#define OFFSET 503
#define OIDS 504
#define ON 505
#define ONLY 506
#define OPERATOR 507
#define OPTION 508
#define OPTIONS 509
#define OR 510
#define ORDER 511
#define ORDINALITY 512
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
#define PASSING 523
#define PASSWORD 524
#define PLACING 525
#define PLANS 526
#define POSITION 527
#define PRECEDING 528
#define PRECISION 529
#define PRESERVE 530
#define PREPARE 531
#define PREPARED 532
#define PRIMARY 533
#define PRIOR 534
#define PRIVILEGES 535
#define PROCEDURAL 536
#define PROCEDURE 537
#define PROGRAM 538
#define QUOTE 539
#define RANGE 540
#define READ 541
#define REAL 542
#define REASSIGN 543
#define RECHECK 544
#define RECURSIVE 545
#define REF 546
#define REFERENCES 547
#define REFRESH 548
#define REINDEX 549
#define RELATIVE_P 550
#define RELEASE 551
#define RENAME 552
#define REPEATABLE 553
#define REPLACE 554
#define REPLICA 555
#define RESET 556
#define RESTART 557
#define RESTRICT 558
#define RETURNING 559
#define RETURNS 560
#define REVOKE 561
#define RIGHT 562
#define ROLE 563
#define ROLLBACK 564
#define ROW 565
#define ROWS 566
#define RULE 567
#define SAVEPOINT 568
#define SCHEMA 569
#define SCROLL 570
#define SEARCH 571
#define SECOND_P 572
#define SECURITY 573
#define SELECT 574
#define SEQUENCE 575
#define SEQUENCES 576
#define SERIALIZABLE 577
#define SERVER 578
#define SESSION 579
#define SESSION_USER 580
#define SET 581
#define SETOF 582
#define SHARE 583
#define SHOW 584
#define SIMILAR 585
#define SIMPLE 586
#define SMALLINT 587
#define SNAPSHOT 588
#define SOME 589
#define STABLE 590
#define STANDALONE_P 591
#define START 592
#define STATEMENT 593
#define STATISTICS 594
#define STDIN 595
#define STDOUT 596
#define STORAGE 597
#define STRICT_P 598
#define STRIP_P 599
#define SUBSTRING 600
#define SYMMETRIC 601
#define SYSID 602
#define SYSTEM_P 603
#define TABLE 604
#define TABLES 605
#define TABLESPACE 606
#define TEMP 607
#define TEMPLATE 608
#define TEMPORARY 609
#define TEXT_P 610
#define THEN 611
#define TIME 612
#define TIMESTAMP 613
#define TO 614
#define TRAILING 615
#define TRANSACTION 616
#define TREAT 617
#define TRIGGER 618
#define TRIM 619
#define TRUE_P 620
#define TRUNCATE 621
#define TRUSTED 622
#define TYPE_P 623
#define TYPES_P 624
#define UNBOUNDED 625
#define UNCOMMITTED 626
#define UNENCRYPTED 627
#define UNION 628
#define UNIQUE 629
#define UNKNOWN 630
#define UNLISTEN 631
#define UNLOGGED 632
#define UNTIL 633
#define UPDATE 634
#define USER 635
#define USING 636
#define VACUUM 637
#define VALID 638
#define VALIDATE 639
#define VALIDATOR 640
#define VALUE_P 641
#define VALUES 642
#define VARCHAR 643
#define VARIADIC 644
#define VARYING 645
#define VERBOSE 646
#define VERSION_P 647
#define VIEW 648
#define VIEWS 649
#define VOLATILE 650
#define WHEN 651
#define WHERE 652
#define WHITESPACE_P 653
#define WINDOW 654
#define WITH 655
#define WITHIN 656
#define WITHOUT 657
#define WORK 658
#define WRAPPER 659
#define WRITE 660
#define XML_P 661
#define XMLATTRIBUTES 662
#define XMLCONCAT 663
#define XMLELEMENT 664
#define XMLEXISTS 665
#define XMLFOREST 666
#define XMLPARSE 667
#define XMLPI 668
#define XMLROOT 669
#define XMLSERIALIZE 670
#define YEAR_P 671
#define YES_P 672
#define ZONE 673
#define NULLS_FIRST 674
#define NULLS_LAST 675
#define WITH_ORDINALITY 676
#define WITH_TIME 677
#define POSTFIXOP 678
#define UMINUS 679

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 211 "gram.y" /* yacc.c:1909  */

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

#line 939 "gram.h" /* yacc.c:1909  */
};
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
