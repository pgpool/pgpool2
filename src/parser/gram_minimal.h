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
    ASENSITIVE = 294,
    ASSERTION = 295,
    ASSIGNMENT = 296,
    ASYMMETRIC = 297,
    ATOMIC = 298,
    AT = 299,
    ATTACH = 300,
    ATTRIBUTE = 301,
    AUTHORIZATION = 302,
    BACKWARD = 303,
    BEFORE = 304,
    BEGIN_P = 305,
    BETWEEN = 306,
    BIGINT = 307,
    BINARY = 308,
    BIT = 309,
    BOOLEAN_P = 310,
    BOTH = 311,
    BREADTH = 312,
    BY = 313,
    CACHE = 314,
    CALL = 315,
    CALLED = 316,
    CASCADE = 317,
    CASCADED = 318,
    CASE = 319,
    CAST = 320,
    CATALOG_P = 321,
    CHAIN = 322,
    CHAR_P = 323,
    CHARACTER = 324,
    CHARACTERISTICS = 325,
    CHECK = 326,
    CHECKPOINT = 327,
    CLASS = 328,
    CLOSE = 329,
    CLUSTER = 330,
    COALESCE = 331,
    COLLATE = 332,
    COLLATION = 333,
    COLUMN = 334,
    COLUMNS = 335,
    COMMENT = 336,
    COMMENTS = 337,
    COMMIT = 338,
    COMMITTED = 339,
    COMPRESSION = 340,
    CONCURRENTLY = 341,
    CONFIGURATION = 342,
    CONFLICT = 343,
    CONNECTION = 344,
    CONSTRAINT = 345,
    CONSTRAINTS = 346,
    CONTENT_P = 347,
    CONTINUE_P = 348,
    CONVERSION_P = 349,
    COPY = 350,
    COST = 351,
    CREATE = 352,
    CROSS = 353,
    CSV = 354,
    CUBE = 355,
    CURRENT_P = 356,
    CURRENT_CATALOG = 357,
    CURRENT_DATE = 358,
    CURRENT_ROLE = 359,
    CURRENT_SCHEMA = 360,
    CURRENT_TIME = 361,
    CURRENT_TIMESTAMP = 362,
    CURRENT_USER = 363,
    CURSOR = 364,
    CYCLE = 365,
    DATA_P = 366,
    DATABASE = 367,
    DAY_P = 368,
    DEALLOCATE = 369,
    DEC = 370,
    DECIMAL_P = 371,
    DECLARE = 372,
    DEFAULT = 373,
    DEFAULTS = 374,
    DEFERRABLE = 375,
    DEFERRED = 376,
    DEFINER = 377,
    DELETE_P = 378,
    DELIMITER = 379,
    DELIMITERS = 380,
    DEPENDS = 381,
    DEPTH = 382,
    DESC = 383,
    DETACH = 384,
    DICTIONARY = 385,
    DISABLE_P = 386,
    DISCARD = 387,
    DISTINCT = 388,
    DO = 389,
    DOCUMENT_P = 390,
    DOMAIN_P = 391,
    DOUBLE_P = 392,
    DROP = 393,
    EACH = 394,
    ELSE = 395,
    ENABLE_P = 396,
    ENCODING = 397,
    ENCRYPTED = 398,
    END_P = 399,
    ENUM_P = 400,
    ESCAPE = 401,
    EVENT = 402,
    EXCEPT = 403,
    EXCLUDE = 404,
    EXCLUDING = 405,
    EXCLUSIVE = 406,
    EXECUTE = 407,
    EXISTS = 408,
    EXPLAIN = 409,
    EXPRESSION = 410,
    EXTENSION = 411,
    EXTERNAL = 412,
    EXTRACT = 413,
    FALSE_P = 414,
    FAMILY = 415,
    FETCH = 416,
    FILTER = 417,
    FINALIZE = 418,
    FIRST_P = 419,
    FLOAT_P = 420,
    FOLLOWING = 421,
    FOR = 422,
    FORCE = 423,
    FOREIGN = 424,
    FORWARD = 425,
    FREEZE = 426,
    FROM = 427,
    FULL = 428,
    FUNCTION = 429,
    FUNCTIONS = 430,
    GENERATED = 431,
    GLOBAL = 432,
    GRANT = 433,
    GRANTED = 434,
    GREATEST = 435,
    GROUP_P = 436,
    GROUPING = 437,
    GROUPS = 438,
    HANDLER = 439,
    HAVING = 440,
    HEADER_P = 441,
    HOLD = 442,
    HOUR_P = 443,
    IDENTITY_P = 444,
    IF_P = 445,
    ILIKE = 446,
    IMMEDIATE = 447,
    IMMUTABLE = 448,
    IMPLICIT_P = 449,
    IMPORT_P = 450,
    IN_P = 451,
    INCLUDE = 452,
    INCLUDING = 453,
    INCREMENT = 454,
    INDEX = 455,
    INDEXES = 456,
    INHERIT = 457,
    INHERITS = 458,
    INITIALLY = 459,
    INLINE_P = 460,
    INNER_P = 461,
    INOUT = 462,
    INPUT_P = 463,
    INSENSITIVE = 464,
    INSERT = 465,
    INSTEAD = 466,
    INT_P = 467,
    INTEGER = 468,
    INTERSECT = 469,
    INTERVAL = 470,
    INTO = 471,
    INVOKER = 472,
    IS = 473,
    ISNULL = 474,
    ISOLATION = 475,
    JOIN = 476,
    KEY = 477,
    LABEL = 478,
    LANGUAGE = 479,
    LARGE_P = 480,
    LAST_P = 481,
    LATERAL_P = 482,
    LEADING = 483,
    LEAKPROOF = 484,
    LEAST = 485,
    LEFT = 486,
    LEVEL = 487,
    LIKE = 488,
    LIMIT = 489,
    LISTEN = 490,
    LOAD = 491,
    LOCAL = 492,
    LOCALTIME = 493,
    LOCALTIMESTAMP = 494,
    LOCATION = 495,
    LOCK_P = 496,
    LOCKED = 497,
    LOGGED = 498,
    MAPPING = 499,
    MATCH = 500,
    MATERIALIZED = 501,
    MAXVALUE = 502,
    METHOD = 503,
    MINUTE_P = 504,
    MINVALUE = 505,
    MODE = 506,
    MONTH_P = 507,
    MOVE = 508,
    NAME_P = 509,
    NAMES = 510,
    NATIONAL = 511,
    NATURAL = 512,
    NCHAR = 513,
    NEW = 514,
    NEXT = 515,
    NFC = 516,
    NFD = 517,
    NFKC = 518,
    NFKD = 519,
    NO = 520,
    NONE = 521,
    NORMALIZE = 522,
    NORMALIZED = 523,
    NOT = 524,
    NOTHING = 525,
    NOTIFY = 526,
    NOTNULL = 527,
    NOWAIT = 528,
    NULL_P = 529,
    NULLIF = 530,
    NULLS_P = 531,
    NUMERIC = 532,
    OBJECT_P = 533,
    OF = 534,
    OFF = 535,
    OFFSET = 536,
    OIDS = 537,
    OLD = 538,
    ON = 539,
    ONLY = 540,
    OPERATOR = 541,
    OPTION = 542,
    OPTIONS = 543,
    OR = 544,
    ORDER = 545,
    ORDINALITY = 546,
    OTHERS = 547,
    OUT_P = 548,
    OUTER_P = 549,
    OVER = 550,
    OVERLAPS = 551,
    OVERLAY = 552,
    OVERRIDING = 553,
    OWNED = 554,
    OWNER = 555,
    PARALLEL = 556,
    PARSER = 557,
    PARTIAL = 558,
    PARTITION = 559,
    PASSING = 560,
    PASSWORD = 561,
    PGPOOL = 562,
    PLACING = 563,
    PLANS = 564,
    POLICY = 565,
    POSITION = 566,
    PRECEDING = 567,
    PRECISION = 568,
    PRESERVE = 569,
    PREPARE = 570,
    PREPARED = 571,
    PRIMARY = 572,
    PRIOR = 573,
    PRIVILEGES = 574,
    PROCEDURAL = 575,
    PROCEDURE = 576,
    PROCEDURES = 577,
    PROGRAM = 578,
    PUBLICATION = 579,
    QUOTE = 580,
    RANGE = 581,
    READ = 582,
    REAL = 583,
    REASSIGN = 584,
    RECHECK = 585,
    RECURSIVE = 586,
    REF = 587,
    REFERENCES = 588,
    REFERENCING = 589,
    REFRESH = 590,
    REINDEX = 591,
    RELATIVE_P = 592,
    RELEASE = 593,
    RENAME = 594,
    REPEATABLE = 595,
    REPLACE = 596,
    REPLICA = 597,
    RESET = 598,
    RESTART = 599,
    RESTRICT = 600,
    RETURN = 601,
    RETURNING = 602,
    RETURNS = 603,
    REVOKE = 604,
    RIGHT = 605,
    ROLE = 606,
    ROLLBACK = 607,
    ROLLUP = 608,
    ROUTINE = 609,
    ROUTINES = 610,
    ROW = 611,
    ROWS = 612,
    RULE = 613,
    SAVEPOINT = 614,
    SCHEMA = 615,
    SCHEMAS = 616,
    SCROLL = 617,
    SEARCH = 618,
    SECOND_P = 619,
    SECURITY = 620,
    SELECT = 621,
    SEQUENCE = 622,
    SEQUENCES = 623,
    SERIALIZABLE = 624,
    SERVER = 625,
    SESSION = 626,
    SESSION_USER = 627,
    SET = 628,
    SETS = 629,
    SETOF = 630,
    SHARE = 631,
    SHOW = 632,
    SIMILAR = 633,
    SIMPLE = 634,
    SKIP = 635,
    SMALLINT = 636,
    SNAPSHOT = 637,
    SOME = 638,
    SQL_P = 639,
    STABLE = 640,
    STANDALONE_P = 641,
    START = 642,
    STATEMENT = 643,
    STATISTICS = 644,
    STDIN = 645,
    STDOUT = 646,
    STORAGE = 647,
    STORED = 648,
    STRICT_P = 649,
    STRIP_P = 650,
    SUBSCRIPTION = 651,
    SUBSTRING = 652,
    SUPPORT = 653,
    SYMMETRIC = 654,
    SYSID = 655,
    SYSTEM_P = 656,
    TABLE = 657,
    TABLES = 658,
    TABLESAMPLE = 659,
    TABLESPACE = 660,
    TEMP = 661,
    TEMPLATE = 662,
    TEMPORARY = 663,
    TEXT_P = 664,
    THEN = 665,
    TIES = 666,
    TIME = 667,
    TIMESTAMP = 668,
    TO = 669,
    TRAILING = 670,
    TRANSACTION = 671,
    TRANSFORM = 672,
    TREAT = 673,
    TRIGGER = 674,
    TRIM = 675,
    TRUE_P = 676,
    TRUNCATE = 677,
    TRUSTED = 678,
    TYPE_P = 679,
    TYPES_P = 680,
    UESCAPE = 681,
    UNBOUNDED = 682,
    UNCOMMITTED = 683,
    UNENCRYPTED = 684,
    UNION = 685,
    UNIQUE = 686,
    UNKNOWN = 687,
    UNLISTEN = 688,
    UNLOGGED = 689,
    UNTIL = 690,
    UPDATE = 691,
    USER = 692,
    USING = 693,
    VACUUM = 694,
    VALID = 695,
    VALIDATE = 696,
    VALIDATOR = 697,
    VALUE_P = 698,
    VALUES = 699,
    VARCHAR = 700,
    VARIADIC = 701,
    VARYING = 702,
    VERBOSE = 703,
    VERSION_P = 704,
    VIEW = 705,
    VIEWS = 706,
    VOLATILE = 707,
    WHEN = 708,
    WHERE = 709,
    WHITESPACE_P = 710,
    WINDOW = 711,
    WITH = 712,
    WITHIN = 713,
    WITHOUT = 714,
    WORK = 715,
    WRAPPER = 716,
    WRITE = 717,
    XML_P = 718,
    XMLATTRIBUTES = 719,
    XMLCONCAT = 720,
    XMLELEMENT = 721,
    XMLEXISTS = 722,
    XMLFOREST = 723,
    XMLNAMESPACES = 724,
    XMLPARSE = 725,
    XMLPI = 726,
    XMLROOT = 727,
    XMLSERIALIZE = 728,
    XMLTABLE = 729,
    YEAR_P = 730,
    YES_P = 731,
    ZONE = 732,
    NOT_LA = 733,
    NULLS_LA = 734,
    WITH_LA = 735,
    MODE_TYPE_NAME = 736,
    MODE_PLPGSQL_EXPR = 737,
    MODE_PLPGSQL_ASSIGN1 = 738,
    MODE_PLPGSQL_ASSIGN2 = 739,
    MODE_PLPGSQL_ASSIGN3 = 740,
    UMINUS = 741
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
#define ASENSITIVE 294
#define ASSERTION 295
#define ASSIGNMENT 296
#define ASYMMETRIC 297
#define ATOMIC 298
#define AT 299
#define ATTACH 300
#define ATTRIBUTE 301
#define AUTHORIZATION 302
#define BACKWARD 303
#define BEFORE 304
#define BEGIN_P 305
#define BETWEEN 306
#define BIGINT 307
#define BINARY 308
#define BIT 309
#define BOOLEAN_P 310
#define BOTH 311
#define BREADTH 312
#define BY 313
#define CACHE 314
#define CALL 315
#define CALLED 316
#define CASCADE 317
#define CASCADED 318
#define CASE 319
#define CAST 320
#define CATALOG_P 321
#define CHAIN 322
#define CHAR_P 323
#define CHARACTER 324
#define CHARACTERISTICS 325
#define CHECK 326
#define CHECKPOINT 327
#define CLASS 328
#define CLOSE 329
#define CLUSTER 330
#define COALESCE 331
#define COLLATE 332
#define COLLATION 333
#define COLUMN 334
#define COLUMNS 335
#define COMMENT 336
#define COMMENTS 337
#define COMMIT 338
#define COMMITTED 339
#define COMPRESSION 340
#define CONCURRENTLY 341
#define CONFIGURATION 342
#define CONFLICT 343
#define CONNECTION 344
#define CONSTRAINT 345
#define CONSTRAINTS 346
#define CONTENT_P 347
#define CONTINUE_P 348
#define CONVERSION_P 349
#define COPY 350
#define COST 351
#define CREATE 352
#define CROSS 353
#define CSV 354
#define CUBE 355
#define CURRENT_P 356
#define CURRENT_CATALOG 357
#define CURRENT_DATE 358
#define CURRENT_ROLE 359
#define CURRENT_SCHEMA 360
#define CURRENT_TIME 361
#define CURRENT_TIMESTAMP 362
#define CURRENT_USER 363
#define CURSOR 364
#define CYCLE 365
#define DATA_P 366
#define DATABASE 367
#define DAY_P 368
#define DEALLOCATE 369
#define DEC 370
#define DECIMAL_P 371
#define DECLARE 372
#define DEFAULT 373
#define DEFAULTS 374
#define DEFERRABLE 375
#define DEFERRED 376
#define DEFINER 377
#define DELETE_P 378
#define DELIMITER 379
#define DELIMITERS 380
#define DEPENDS 381
#define DEPTH 382
#define DESC 383
#define DETACH 384
#define DICTIONARY 385
#define DISABLE_P 386
#define DISCARD 387
#define DISTINCT 388
#define DO 389
#define DOCUMENT_P 390
#define DOMAIN_P 391
#define DOUBLE_P 392
#define DROP 393
#define EACH 394
#define ELSE 395
#define ENABLE_P 396
#define ENCODING 397
#define ENCRYPTED 398
#define END_P 399
#define ENUM_P 400
#define ESCAPE 401
#define EVENT 402
#define EXCEPT 403
#define EXCLUDE 404
#define EXCLUDING 405
#define EXCLUSIVE 406
#define EXECUTE 407
#define EXISTS 408
#define EXPLAIN 409
#define EXPRESSION 410
#define EXTENSION 411
#define EXTERNAL 412
#define EXTRACT 413
#define FALSE_P 414
#define FAMILY 415
#define FETCH 416
#define FILTER 417
#define FINALIZE 418
#define FIRST_P 419
#define FLOAT_P 420
#define FOLLOWING 421
#define FOR 422
#define FORCE 423
#define FOREIGN 424
#define FORWARD 425
#define FREEZE 426
#define FROM 427
#define FULL 428
#define FUNCTION 429
#define FUNCTIONS 430
#define GENERATED 431
#define GLOBAL 432
#define GRANT 433
#define GRANTED 434
#define GREATEST 435
#define GROUP_P 436
#define GROUPING 437
#define GROUPS 438
#define HANDLER 439
#define HAVING 440
#define HEADER_P 441
#define HOLD 442
#define HOUR_P 443
#define IDENTITY_P 444
#define IF_P 445
#define ILIKE 446
#define IMMEDIATE 447
#define IMMUTABLE 448
#define IMPLICIT_P 449
#define IMPORT_P 450
#define IN_P 451
#define INCLUDE 452
#define INCLUDING 453
#define INCREMENT 454
#define INDEX 455
#define INDEXES 456
#define INHERIT 457
#define INHERITS 458
#define INITIALLY 459
#define INLINE_P 460
#define INNER_P 461
#define INOUT 462
#define INPUT_P 463
#define INSENSITIVE 464
#define INSERT 465
#define INSTEAD 466
#define INT_P 467
#define INTEGER 468
#define INTERSECT 469
#define INTERVAL 470
#define INTO 471
#define INVOKER 472
#define IS 473
#define ISNULL 474
#define ISOLATION 475
#define JOIN 476
#define KEY 477
#define LABEL 478
#define LANGUAGE 479
#define LARGE_P 480
#define LAST_P 481
#define LATERAL_P 482
#define LEADING 483
#define LEAKPROOF 484
#define LEAST 485
#define LEFT 486
#define LEVEL 487
#define LIKE 488
#define LIMIT 489
#define LISTEN 490
#define LOAD 491
#define LOCAL 492
#define LOCALTIME 493
#define LOCALTIMESTAMP 494
#define LOCATION 495
#define LOCK_P 496
#define LOCKED 497
#define LOGGED 498
#define MAPPING 499
#define MATCH 500
#define MATERIALIZED 501
#define MAXVALUE 502
#define METHOD 503
#define MINUTE_P 504
#define MINVALUE 505
#define MODE 506
#define MONTH_P 507
#define MOVE 508
#define NAME_P 509
#define NAMES 510
#define NATIONAL 511
#define NATURAL 512
#define NCHAR 513
#define NEW 514
#define NEXT 515
#define NFC 516
#define NFD 517
#define NFKC 518
#define NFKD 519
#define NO 520
#define NONE 521
#define NORMALIZE 522
#define NORMALIZED 523
#define NOT 524
#define NOTHING 525
#define NOTIFY 526
#define NOTNULL 527
#define NOWAIT 528
#define NULL_P 529
#define NULLIF 530
#define NULLS_P 531
#define NUMERIC 532
#define OBJECT_P 533
#define OF 534
#define OFF 535
#define OFFSET 536
#define OIDS 537
#define OLD 538
#define ON 539
#define ONLY 540
#define OPERATOR 541
#define OPTION 542
#define OPTIONS 543
#define OR 544
#define ORDER 545
#define ORDINALITY 546
#define OTHERS 547
#define OUT_P 548
#define OUTER_P 549
#define OVER 550
#define OVERLAPS 551
#define OVERLAY 552
#define OVERRIDING 553
#define OWNED 554
#define OWNER 555
#define PARALLEL 556
#define PARSER 557
#define PARTIAL 558
#define PARTITION 559
#define PASSING 560
#define PASSWORD 561
#define PGPOOL 562
#define PLACING 563
#define PLANS 564
#define POLICY 565
#define POSITION 566
#define PRECEDING 567
#define PRECISION 568
#define PRESERVE 569
#define PREPARE 570
#define PREPARED 571
#define PRIMARY 572
#define PRIOR 573
#define PRIVILEGES 574
#define PROCEDURAL 575
#define PROCEDURE 576
#define PROCEDURES 577
#define PROGRAM 578
#define PUBLICATION 579
#define QUOTE 580
#define RANGE 581
#define READ 582
#define REAL 583
#define REASSIGN 584
#define RECHECK 585
#define RECURSIVE 586
#define REF 587
#define REFERENCES 588
#define REFERENCING 589
#define REFRESH 590
#define REINDEX 591
#define RELATIVE_P 592
#define RELEASE 593
#define RENAME 594
#define REPEATABLE 595
#define REPLACE 596
#define REPLICA 597
#define RESET 598
#define RESTART 599
#define RESTRICT 600
#define RETURN 601
#define RETURNING 602
#define RETURNS 603
#define REVOKE 604
#define RIGHT 605
#define ROLE 606
#define ROLLBACK 607
#define ROLLUP 608
#define ROUTINE 609
#define ROUTINES 610
#define ROW 611
#define ROWS 612
#define RULE 613
#define SAVEPOINT 614
#define SCHEMA 615
#define SCHEMAS 616
#define SCROLL 617
#define SEARCH 618
#define SECOND_P 619
#define SECURITY 620
#define SELECT 621
#define SEQUENCE 622
#define SEQUENCES 623
#define SERIALIZABLE 624
#define SERVER 625
#define SESSION 626
#define SESSION_USER 627
#define SET 628
#define SETS 629
#define SETOF 630
#define SHARE 631
#define SHOW 632
#define SIMILAR 633
#define SIMPLE 634
#define SKIP 635
#define SMALLINT 636
#define SNAPSHOT 637
#define SOME 638
#define SQL_P 639
#define STABLE 640
#define STANDALONE_P 641
#define START 642
#define STATEMENT 643
#define STATISTICS 644
#define STDIN 645
#define STDOUT 646
#define STORAGE 647
#define STORED 648
#define STRICT_P 649
#define STRIP_P 650
#define SUBSCRIPTION 651
#define SUBSTRING 652
#define SUPPORT 653
#define SYMMETRIC 654
#define SYSID 655
#define SYSTEM_P 656
#define TABLE 657
#define TABLES 658
#define TABLESAMPLE 659
#define TABLESPACE 660
#define TEMP 661
#define TEMPLATE 662
#define TEMPORARY 663
#define TEXT_P 664
#define THEN 665
#define TIES 666
#define TIME 667
#define TIMESTAMP 668
#define TO 669
#define TRAILING 670
#define TRANSACTION 671
#define TRANSFORM 672
#define TREAT 673
#define TRIGGER 674
#define TRIM 675
#define TRUE_P 676
#define TRUNCATE 677
#define TRUSTED 678
#define TYPE_P 679
#define TYPES_P 680
#define UESCAPE 681
#define UNBOUNDED 682
#define UNCOMMITTED 683
#define UNENCRYPTED 684
#define UNION 685
#define UNIQUE 686
#define UNKNOWN 687
#define UNLISTEN 688
#define UNLOGGED 689
#define UNTIL 690
#define UPDATE 691
#define USER 692
#define USING 693
#define VACUUM 694
#define VALID 695
#define VALIDATE 696
#define VALIDATOR 697
#define VALUE_P 698
#define VALUES 699
#define VARCHAR 700
#define VARIADIC 701
#define VARYING 702
#define VERBOSE 703
#define VERSION_P 704
#define VIEW 705
#define VIEWS 706
#define VOLATILE 707
#define WHEN 708
#define WHERE 709
#define WHITESPACE_P 710
#define WINDOW 711
#define WITH 712
#define WITHIN 713
#define WITHOUT 714
#define WORK 715
#define WRAPPER 716
#define WRITE 717
#define XML_P 718
#define XMLATTRIBUTES 719
#define XMLCONCAT 720
#define XMLELEMENT 721
#define XMLEXISTS 722
#define XMLFOREST 723
#define XMLNAMESPACES 724
#define XMLPARSE 725
#define XMLPI 726
#define XMLROOT 727
#define XMLSERIALIZE 728
#define XMLTABLE 729
#define YEAR_P 730
#define YES_P 731
#define ZONE 732
#define NOT_LA 733
#define NULLS_LA 734
#define WITH_LA 735
#define MODE_TYPE_NAME 736
#define MODE_PLPGSQL_EXPR 737
#define MODE_PLPGSQL_ASSIGN1 738
#define MODE_PLPGSQL_ASSIGN2 739
#define MODE_PLPGSQL_ASSIGN3 740
#define UMINUS 741

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 248 "gram_minimal.y" /* yacc.c:1909  */

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
	StatsElem			*selem;
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
	SetQuantifier	 setquantifier;
	struct GroupClause  *groupclause;

#line 1074 "gram_minimal.h" /* yacc.c:1909  */
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
