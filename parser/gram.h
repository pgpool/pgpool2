/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
     ABORT_P = 258,
     ABSOLUTE_P = 259,
     ACCESS = 260,
     ACTION = 261,
     ADD_P = 262,
     ADMIN = 263,
     AFTER = 264,
     AGGREGATE = 265,
     ALL = 266,
     ALSO = 267,
     ALTER = 268,
     ALWAYS = 269,
     ANALYSE = 270,
     ANALYZE = 271,
     AND = 272,
     ANY = 273,
     ARRAY = 274,
     AS = 275,
     ASC = 276,
     ASSERTION = 277,
     ASSIGNMENT = 278,
     ASYMMETRIC = 279,
     AT = 280,
     AUTHORIZATION = 281,
     BACKWARD = 282,
     BEFORE = 283,
     BEGIN_P = 284,
     BETWEEN = 285,
     BIGINT = 286,
     BINARY = 287,
     BIT = 288,
     BOOLEAN_P = 289,
     BOTH = 290,
     BY = 291,
     CACHE = 292,
     CALLED = 293,
     CASCADE = 294,
     CASCADED = 295,
     CASE = 296,
     CAST = 297,
     CHAIN = 298,
     CHAR_P = 299,
     CHARACTER = 300,
     CHARACTERISTICS = 301,
     CHECK = 302,
     CHECKPOINT = 303,
     CLASS = 304,
     CLOSE = 305,
     CLUSTER = 306,
     COALESCE = 307,
     COLLATE = 308,
     COLUMN = 309,
     COMMENT = 310,
     COMMIT = 311,
     COMMITTED = 312,
     CONCURRENTLY = 313,
     CONFIGURATION = 314,
     CONNECTION = 315,
     CONSTRAINT = 316,
     CONSTRAINTS = 317,
     CONTENT_P = 318,
     CONVERSION_P = 319,
     COPY = 320,
     COST = 321,
     CREATE = 322,
     CREATEDB = 323,
     CREATEROLE = 324,
     CREATEUSER = 325,
     CROSS = 326,
     CSV = 327,
     CURRENT_P = 328,
     CURRENT_DATE = 329,
     CURRENT_ROLE = 330,
     CURRENT_TIME = 331,
     CURRENT_TIMESTAMP = 332,
     CURRENT_USER = 333,
     CURSOR = 334,
     CYCLE = 335,
     DATABASE = 336,
     DAY_P = 337,
     DEALLOCATE = 338,
     DEC = 339,
     DECIMAL_P = 340,
     DECLARE = 341,
     DEFAULT = 342,
     DEFAULTS = 343,
     DEFERRABLE = 344,
     DEFERRED = 345,
     DEFINER = 346,
     DELETE_P = 347,
     DELIMITER = 348,
     DELIMITERS = 349,
     DESC = 350,
     DICTIONARY = 351,
     DISABLE_P = 352,
     DISCARD = 353,
     DISTINCT = 354,
     DO = 355,
     DOCUMENT_P = 356,
     DOMAIN_P = 357,
     DOUBLE_P = 358,
     DROP = 359,
     EACH = 360,
     ELSE = 361,
     ENABLE_P = 362,
     ENCODING = 363,
     ENCRYPTED = 364,
     END_P = 365,
     ENUM_P = 366,
     ESCAPE = 367,
     EXCEPT = 368,
     EXCLUDING = 369,
     EXCLUSIVE = 370,
     EXECUTE = 371,
     EXISTS = 372,
     EXPLAIN = 373,
     EXTERNAL = 374,
     EXTRACT = 375,
     FALSE_P = 376,
     FAMILY = 377,
     FETCH = 378,
     FIRST_P = 379,
     FLOAT_P = 380,
     FOR = 381,
     FORCE = 382,
     FOREIGN = 383,
     FORWARD = 384,
     FREEZE = 385,
     FROM = 386,
     FULL = 387,
     FUNCTION = 388,
     GLOBAL = 389,
     GRANT = 390,
     GRANTED = 391,
     GREATEST = 392,
     GROUP_P = 393,
     HANDLER = 394,
     HAVING = 395,
     HEADER_P = 396,
     HOLD = 397,
     HOUR_P = 398,
     IF_P = 399,
     ILIKE = 400,
     IMMEDIATE = 401,
     IMMUTABLE = 402,
     IMPLICIT_P = 403,
     IN_P = 404,
     INCLUDING = 405,
     INCREMENT = 406,
     INDEX = 407,
     INDEXES = 408,
     INHERIT = 409,
     INHERITS = 410,
     INITIALLY = 411,
     INNER_P = 412,
     INOUT = 413,
     INPUT_P = 414,
     INSENSITIVE = 415,
     INSERT = 416,
     INSTEAD = 417,
     INT_P = 418,
     INTEGER = 419,
     INTERSECT = 420,
     INTERVAL = 421,
     INTO = 422,
     INVOKER = 423,
     IS = 424,
     ISNULL = 425,
     ISOLATION = 426,
     JOIN = 427,
     KEY = 428,
     LANCOMPILER = 429,
     LANGUAGE = 430,
     LARGE_P = 431,
     LAST_P = 432,
     LEADING = 433,
     LEAST = 434,
     LEFT = 435,
     LEVEL = 436,
     LIKE = 437,
     LIMIT = 438,
     LISTEN = 439,
     LOAD = 440,
     LOCAL = 441,
     LOCALTIME = 442,
     LOCALTIMESTAMP = 443,
     LOCATION = 444,
     LOCK_P = 445,
     LOGIN_P = 446,
     MAPPING = 447,
     MATCH = 448,
     MAXVALUE = 449,
     MINUTE_P = 450,
     MINVALUE = 451,
     MODE = 452,
     MONTH_P = 453,
     MOVE = 454,
     NAME_P = 455,
     NAMES = 456,
     NATIONAL = 457,
     NATURAL = 458,
     NCHAR = 459,
     NEW = 460,
     NEXT = 461,
     NO = 462,
     NOCREATEDB = 463,
     NOCREATEROLE = 464,
     NOCREATEUSER = 465,
     NOINHERIT = 466,
     NOLOGIN_P = 467,
     NONE = 468,
     NOSUPERUSER = 469,
     NOT = 470,
     NOTHING = 471,
     NOTIFY = 472,
     NOTNULL = 473,
     NOWAIT = 474,
     NULL_P = 475,
     NULLIF = 476,
     NULLS_P = 477,
     NUMERIC = 478,
     OBJECT_P = 479,
     OF = 480,
     OFF = 481,
     OFFSET = 482,
     OIDS = 483,
     OLD = 484,
     ON = 485,
     ONLY = 486,
     OPERATOR = 487,
     OPTION = 488,
     OR = 489,
     ORDER = 490,
     OUT_P = 491,
     OUTER_P = 492,
     OVERLAPS = 493,
     OVERLAY = 494,
     OWNED = 495,
     OWNER = 496,
     PARSER = 497,
     PARTIAL = 498,
     PASSWORD = 499,
     PLACING = 500,
     PLANS = 501,
     POSITION = 502,
     PRECISION = 503,
     PRESERVE = 504,
     PREPARE = 505,
     PREPARED = 506,
     PRIMARY = 507,
     PRIOR = 508,
     PRIVILEGES = 509,
     PROCEDURAL = 510,
     PROCEDURE = 511,
     QUOTE = 512,
     READ = 513,
     REAL = 514,
     REASSIGN = 515,
     RECHECK = 516,
     REFERENCES = 517,
     REINDEX = 518,
     RELATIVE_P = 519,
     RELEASE = 520,
     RENAME = 521,
     REPEATABLE = 522,
     REPLACE = 523,
     REPLICA = 524,
     RESET = 525,
     RESTART = 526,
     RESTRICT = 527,
     RETURNING = 528,
     RETURNS = 529,
     REVOKE = 530,
     RIGHT = 531,
     ROLE = 532,
     ROLLBACK = 533,
     ROW = 534,
     ROWS = 535,
     RULE = 536,
     SAVEPOINT = 537,
     SCHEMA = 538,
     SCROLL = 539,
     SEARCH = 540,
     SECOND_P = 541,
     SECURITY = 542,
     SELECT = 543,
     SEQUENCE = 544,
     SERIALIZABLE = 545,
     SESSION = 546,
     SESSION_USER = 547,
     SET = 548,
     SETOF = 549,
     SHARE = 550,
     SHOW = 551,
     SIMILAR = 552,
     SIMPLE = 553,
     SMALLINT = 554,
     SOME = 555,
     STABLE = 556,
     STANDALONE_P = 557,
     START = 558,
     STATEMENT = 559,
     STATISTICS = 560,
     STDIN = 561,
     STDOUT = 562,
     STORAGE = 563,
     STRICT_P = 564,
     STRIP_P = 565,
     SUBSTRING = 566,
     SUPERUSER_P = 567,
     SYMMETRIC = 568,
     SYSID = 569,
     SYSTEM_P = 570,
     TABLE = 571,
     TABLESPACE = 572,
     TEMP = 573,
     TEMPLATE = 574,
     TEMPORARY = 575,
     TEXT_P = 576,
     THEN = 577,
     TIME = 578,
     TIMESTAMP = 579,
     TO = 580,
     TRAILING = 581,
     TRANSACTION = 582,
     TREAT = 583,
     TRIGGER = 584,
     TRIM = 585,
     TRUE_P = 586,
     TRUNCATE = 587,
     TRUSTED = 588,
     TYPE_P = 589,
     UNCOMMITTED = 590,
     UNENCRYPTED = 591,
     UNION = 592,
     UNIQUE = 593,
     UNKNOWN = 594,
     UNLISTEN = 595,
     UNTIL = 596,
     UPDATE = 597,
     USER = 598,
     USING = 599,
     VACUUM = 600,
     VALID = 601,
     VALIDATOR = 602,
     VALUE_P = 603,
     VALUES = 604,
     VARCHAR = 605,
     VARYING = 606,
     VERBOSE = 607,
     VERSION_P = 608,
     VIEW = 609,
     VOLATILE = 610,
     WHEN = 611,
     WHERE = 612,
     WHITESPACE_P = 613,
     WITH = 614,
     WITHOUT = 615,
     WORK = 616,
     WRITE = 617,
     XML_P = 618,
     XMLATTRIBUTES = 619,
     XMLCONCAT = 620,
     XMLELEMENT = 621,
     XMLFOREST = 622,
     XMLPARSE = 623,
     XMLPI = 624,
     XMLROOT = 625,
     XMLSERIALIZE = 626,
     YEAR_P = 627,
     YES_P = 628,
     ZONE = 629,
     NULLS_FIRST = 630,
     NULLS_LAST = 631,
     WITH_CASCADED = 632,
     WITH_LOCAL = 633,
     WITH_CHECK = 634,
     IDENT = 635,
     FCONST = 636,
     SCONST = 637,
     BCONST = 638,
     XCONST = 639,
     Op = 640,
     ICONST = 641,
     PARAM = 642,
     POSTFIXOP = 643,
     UMINUS = 644,
     TYPECAST = 645
   };
#endif
/* Tokens.  */
#define ABORT_P 258
#define ABSOLUTE_P 259
#define ACCESS 260
#define ACTION 261
#define ADD_P 262
#define ADMIN 263
#define AFTER 264
#define AGGREGATE 265
#define ALL 266
#define ALSO 267
#define ALTER 268
#define ALWAYS 269
#define ANALYSE 270
#define ANALYZE 271
#define AND 272
#define ANY 273
#define ARRAY 274
#define AS 275
#define ASC 276
#define ASSERTION 277
#define ASSIGNMENT 278
#define ASYMMETRIC 279
#define AT 280
#define AUTHORIZATION 281
#define BACKWARD 282
#define BEFORE 283
#define BEGIN_P 284
#define BETWEEN 285
#define BIGINT 286
#define BINARY 287
#define BIT 288
#define BOOLEAN_P 289
#define BOTH 290
#define BY 291
#define CACHE 292
#define CALLED 293
#define CASCADE 294
#define CASCADED 295
#define CASE 296
#define CAST 297
#define CHAIN 298
#define CHAR_P 299
#define CHARACTER 300
#define CHARACTERISTICS 301
#define CHECK 302
#define CHECKPOINT 303
#define CLASS 304
#define CLOSE 305
#define CLUSTER 306
#define COALESCE 307
#define COLLATE 308
#define COLUMN 309
#define COMMENT 310
#define COMMIT 311
#define COMMITTED 312
#define CONCURRENTLY 313
#define CONFIGURATION 314
#define CONNECTION 315
#define CONSTRAINT 316
#define CONSTRAINTS 317
#define CONTENT_P 318
#define CONVERSION_P 319
#define COPY 320
#define COST 321
#define CREATE 322
#define CREATEDB 323
#define CREATEROLE 324
#define CREATEUSER 325
#define CROSS 326
#define CSV 327
#define CURRENT_P 328
#define CURRENT_DATE 329
#define CURRENT_ROLE 330
#define CURRENT_TIME 331
#define CURRENT_TIMESTAMP 332
#define CURRENT_USER 333
#define CURSOR 334
#define CYCLE 335
#define DATABASE 336
#define DAY_P 337
#define DEALLOCATE 338
#define DEC 339
#define DECIMAL_P 340
#define DECLARE 341
#define DEFAULT 342
#define DEFAULTS 343
#define DEFERRABLE 344
#define DEFERRED 345
#define DEFINER 346
#define DELETE_P 347
#define DELIMITER 348
#define DELIMITERS 349
#define DESC 350
#define DICTIONARY 351
#define DISABLE_P 352
#define DISCARD 353
#define DISTINCT 354
#define DO 355
#define DOCUMENT_P 356
#define DOMAIN_P 357
#define DOUBLE_P 358
#define DROP 359
#define EACH 360
#define ELSE 361
#define ENABLE_P 362
#define ENCODING 363
#define ENCRYPTED 364
#define END_P 365
#define ENUM_P 366
#define ESCAPE 367
#define EXCEPT 368
#define EXCLUDING 369
#define EXCLUSIVE 370
#define EXECUTE 371
#define EXISTS 372
#define EXPLAIN 373
#define EXTERNAL 374
#define EXTRACT 375
#define FALSE_P 376
#define FAMILY 377
#define FETCH 378
#define FIRST_P 379
#define FLOAT_P 380
#define FOR 381
#define FORCE 382
#define FOREIGN 383
#define FORWARD 384
#define FREEZE 385
#define FROM 386
#define FULL 387
#define FUNCTION 388
#define GLOBAL 389
#define GRANT 390
#define GRANTED 391
#define GREATEST 392
#define GROUP_P 393
#define HANDLER 394
#define HAVING 395
#define HEADER_P 396
#define HOLD 397
#define HOUR_P 398
#define IF_P 399
#define ILIKE 400
#define IMMEDIATE 401
#define IMMUTABLE 402
#define IMPLICIT_P 403
#define IN_P 404
#define INCLUDING 405
#define INCREMENT 406
#define INDEX 407
#define INDEXES 408
#define INHERIT 409
#define INHERITS 410
#define INITIALLY 411
#define INNER_P 412
#define INOUT 413
#define INPUT_P 414
#define INSENSITIVE 415
#define INSERT 416
#define INSTEAD 417
#define INT_P 418
#define INTEGER 419
#define INTERSECT 420
#define INTERVAL 421
#define INTO 422
#define INVOKER 423
#define IS 424
#define ISNULL 425
#define ISOLATION 426
#define JOIN 427
#define KEY 428
#define LANCOMPILER 429
#define LANGUAGE 430
#define LARGE_P 431
#define LAST_P 432
#define LEADING 433
#define LEAST 434
#define LEFT 435
#define LEVEL 436
#define LIKE 437
#define LIMIT 438
#define LISTEN 439
#define LOAD 440
#define LOCAL 441
#define LOCALTIME 442
#define LOCALTIMESTAMP 443
#define LOCATION 444
#define LOCK_P 445
#define LOGIN_P 446
#define MAPPING 447
#define MATCH 448
#define MAXVALUE 449
#define MINUTE_P 450
#define MINVALUE 451
#define MODE 452
#define MONTH_P 453
#define MOVE 454
#define NAME_P 455
#define NAMES 456
#define NATIONAL 457
#define NATURAL 458
#define NCHAR 459
#define NEW 460
#define NEXT 461
#define NO 462
#define NOCREATEDB 463
#define NOCREATEROLE 464
#define NOCREATEUSER 465
#define NOINHERIT 466
#define NOLOGIN_P 467
#define NONE 468
#define NOSUPERUSER 469
#define NOT 470
#define NOTHING 471
#define NOTIFY 472
#define NOTNULL 473
#define NOWAIT 474
#define NULL_P 475
#define NULLIF 476
#define NULLS_P 477
#define NUMERIC 478
#define OBJECT_P 479
#define OF 480
#define OFF 481
#define OFFSET 482
#define OIDS 483
#define OLD 484
#define ON 485
#define ONLY 486
#define OPERATOR 487
#define OPTION 488
#define OR 489
#define ORDER 490
#define OUT_P 491
#define OUTER_P 492
#define OVERLAPS 493
#define OVERLAY 494
#define OWNED 495
#define OWNER 496
#define PARSER 497
#define PARTIAL 498
#define PASSWORD 499
#define PLACING 500
#define PLANS 501
#define POSITION 502
#define PRECISION 503
#define PRESERVE 504
#define PREPARE 505
#define PREPARED 506
#define PRIMARY 507
#define PRIOR 508
#define PRIVILEGES 509
#define PROCEDURAL 510
#define PROCEDURE 511
#define QUOTE 512
#define READ 513
#define REAL 514
#define REASSIGN 515
#define RECHECK 516
#define REFERENCES 517
#define REINDEX 518
#define RELATIVE_P 519
#define RELEASE 520
#define RENAME 521
#define REPEATABLE 522
#define REPLACE 523
#define REPLICA 524
#define RESET 525
#define RESTART 526
#define RESTRICT 527
#define RETURNING 528
#define RETURNS 529
#define REVOKE 530
#define RIGHT 531
#define ROLE 532
#define ROLLBACK 533
#define ROW 534
#define ROWS 535
#define RULE 536
#define SAVEPOINT 537
#define SCHEMA 538
#define SCROLL 539
#define SEARCH 540
#define SECOND_P 541
#define SECURITY 542
#define SELECT 543
#define SEQUENCE 544
#define SERIALIZABLE 545
#define SESSION 546
#define SESSION_USER 547
#define SET 548
#define SETOF 549
#define SHARE 550
#define SHOW 551
#define SIMILAR 552
#define SIMPLE 553
#define SMALLINT 554
#define SOME 555
#define STABLE 556
#define STANDALONE_P 557
#define START 558
#define STATEMENT 559
#define STATISTICS 560
#define STDIN 561
#define STDOUT 562
#define STORAGE 563
#define STRICT_P 564
#define STRIP_P 565
#define SUBSTRING 566
#define SUPERUSER_P 567
#define SYMMETRIC 568
#define SYSID 569
#define SYSTEM_P 570
#define TABLE 571
#define TABLESPACE 572
#define TEMP 573
#define TEMPLATE 574
#define TEMPORARY 575
#define TEXT_P 576
#define THEN 577
#define TIME 578
#define TIMESTAMP 579
#define TO 580
#define TRAILING 581
#define TRANSACTION 582
#define TREAT 583
#define TRIGGER 584
#define TRIM 585
#define TRUE_P 586
#define TRUNCATE 587
#define TRUSTED 588
#define TYPE_P 589
#define UNCOMMITTED 590
#define UNENCRYPTED 591
#define UNION 592
#define UNIQUE 593
#define UNKNOWN 594
#define UNLISTEN 595
#define UNTIL 596
#define UPDATE 597
#define USER 598
#define USING 599
#define VACUUM 600
#define VALID 601
#define VALIDATOR 602
#define VALUE_P 603
#define VALUES 604
#define VARCHAR 605
#define VARYING 606
#define VERBOSE 607
#define VERSION_P 608
#define VIEW 609
#define VOLATILE 610
#define WHEN 611
#define WHERE 612
#define WHITESPACE_P 613
#define WITH 614
#define WITHOUT 615
#define WORK 616
#define WRITE 617
#define XML_P 618
#define XMLATTRIBUTES 619
#define XMLCONCAT 620
#define XMLELEMENT 621
#define XMLFOREST 622
#define XMLPARSE 623
#define XMLPI 624
#define XMLROOT 625
#define XMLSERIALIZE 626
#define YEAR_P 627
#define YES_P 628
#define ZONE 629
#define NULLS_FIRST 630
#define NULLS_LAST 631
#define WITH_CASCADED 632
#define WITH_LOCAL 633
#define WITH_CHECK 634
#define IDENT 635
#define FCONST 636
#define SCONST 637
#define BCONST 638
#define XCONST 639
#define Op 640
#define ICONST 641
#define PARAM 642
#define POSTFIXOP 643
#define UMINUS 644
#define TYPECAST 645




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 145 "gram.y"
{
	int					ival;
	char				chr;
	char				*str;
	const char			*keyword;
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
	JoinExpr			*jexpr;
	IndexElem			*ielem;
	Alias				*alias;
	RangeVar			*range;
	IntoClause			*into;
	A_Indices			*aind;
	ResTarget			*target;
	PrivTarget			*privtarget;

	InsertStmt			*istmt;
	VariableSetStmt		*vsetstmt;
}
/* Line 1489 of yacc.c.  */
#line 862 "gram.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE base_yylval;

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

extern YYLTYPE base_yylloc;
