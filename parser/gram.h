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
/* Tokens.  */
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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 106 "gram.y"
{
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
}
/* Line 1529 of yacc.c.  */
#line 778 "gram.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

