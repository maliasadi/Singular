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
     DOTDOT = 258,
     EQUAL_EQUAL = 259,
     GE = 260,
     LE = 261,
     MINUSMINUS = 262,
     NOT = 263,
     NOTEQUAL = 264,
     PLUSPLUS = 265,
     COLONCOLON = 266,
     GRING_CMD = 267,
     INTMAT_CMD = 268,
     PROC_CMD = 269,
     RING_CMD = 270,
     BEGIN_RING = 271,
     IDEAL_CMD = 272,
     MAP_CMD = 273,
     MATRIX_CMD = 274,
     MODUL_CMD = 275,
     NUMBER_CMD = 276,
     POLY_CMD = 277,
     RESOLUTION_CMD = 278,
     VECTOR_CMD = 279,
     BETTI_CMD = 280,
     COEFFS_CMD = 281,
     COEF_CMD = 282,
     CONTRACT_CMD = 283,
     DEGREE_CMD = 284,
     DEG_CMD = 285,
     DIFF_CMD = 286,
     DIM_CMD = 287,
     DIVISION_CMD = 288,
     ELIMINATION_CMD = 289,
     E_CMD = 290,
     FAREY_CMD = 291,
     FETCH_CMD = 292,
     FREEMODULE_CMD = 293,
     KEEPRING_CMD = 294,
     HILBERT_CMD = 295,
     HOMOG_CMD = 296,
     IMAP_CMD = 297,
     INDEPSET_CMD = 298,
     INTERRED_CMD = 299,
     INTERSECT_CMD = 300,
     JACOB_CMD = 301,
     JET_CMD = 302,
     KBASE_CMD = 303,
     KOSZUL_CMD = 304,
     LEADCOEF_CMD = 305,
     LEADEXP_CMD = 306,
     LEAD_CMD = 307,
     LEADMONOM_CMD = 308,
     LIFTSTD_CMD = 309,
     LIFT_CMD = 310,
     MAXID_CMD = 311,
     MINBASE_CMD = 312,
     MINOR_CMD = 313,
     MINRES_CMD = 314,
     MODULO_CMD = 315,
     MONOM_CMD = 316,
     MRES_CMD = 317,
     MULTIPLICITY_CMD = 318,
     ORD_CMD = 319,
     PAR_CMD = 320,
     PARDEG_CMD = 321,
     PREIMAGE_CMD = 322,
     QUOTIENT_CMD = 323,
     QHWEIGHT_CMD = 324,
     REDUCE_CMD = 325,
     REGULARITY_CMD = 326,
     RES_CMD = 327,
     SIMPLIFY_CMD = 328,
     SORTVEC_CMD = 329,
     SRES_CMD = 330,
     STD_CMD = 331,
     SUBST_CMD = 332,
     SYZYGY_CMD = 333,
     VAR_CMD = 334,
     VDIM_CMD = 335,
     WEDGE_CMD = 336,
     WEIGHT_CMD = 337,
     VALTVARS = 338,
     VMAXDEG = 339,
     VMAXMULT = 340,
     VNOETHER = 341,
     VMINPOLY = 342,
     END_RING = 343,
     CMD_1 = 344,
     CMD_2 = 345,
     CMD_3 = 346,
     CMD_12 = 347,
     CMD_13 = 348,
     CMD_23 = 349,
     CMD_123 = 350,
     CMD_M = 351,
     ROOT_DECL = 352,
     ROOT_DECL_LIST = 353,
     RING_DECL = 354,
     EXAMPLE_CMD = 355,
     EXPORT_CMD = 356,
     HELP_CMD = 357,
     KILL_CMD = 358,
     LIB_CMD = 359,
     LISTVAR_CMD = 360,
     SETRING_CMD = 361,
     TYPE_CMD = 362,
     STRINGTOK = 363,
     BLOCKTOK = 364,
     INT_CONST = 365,
     UNKNOWN_IDENT = 366,
     RINGVAR = 367,
     PROC_DEF = 368,
     BREAK_CMD = 369,
     CONTINUE_CMD = 370,
     ELSE_CMD = 371,
     EVAL = 372,
     QUOTE = 373,
     FOR_CMD = 374,
     IF_CMD = 375,
     SYS_BREAK = 376,
     WHILE_CMD = 377,
     RETURN = 378,
     PARAMETER = 379,
     SYSVAR = 380,
     UMINUS = 381
   };
#endif
/* Tokens.  */
#define DOTDOT 258
#define EQUAL_EQUAL 259
#define GE 260
#define LE 261
#define MINUSMINUS 262
#define NOT 263
#define NOTEQUAL 264
#define PLUSPLUS 265
#define COLONCOLON 266
#define GRING_CMD 267
#define INTMAT_CMD 268
#define PROC_CMD 269
#define RING_CMD 270
#define BEGIN_RING 271
#define IDEAL_CMD 272
#define MAP_CMD 273
#define MATRIX_CMD 274
#define MODUL_CMD 275
#define NUMBER_CMD 276
#define POLY_CMD 277
#define RESOLUTION_CMD 278
#define VECTOR_CMD 279
#define BETTI_CMD 280
#define COEFFS_CMD 281
#define COEF_CMD 282
#define CONTRACT_CMD 283
#define DEGREE_CMD 284
#define DEG_CMD 285
#define DIFF_CMD 286
#define DIM_CMD 287
#define DIVISION_CMD 288
#define ELIMINATION_CMD 289
#define E_CMD 290
#define FAREY_CMD 291
#define FETCH_CMD 292
#define FREEMODULE_CMD 293
#define KEEPRING_CMD 294
#define HILBERT_CMD 295
#define HOMOG_CMD 296
#define IMAP_CMD 297
#define INDEPSET_CMD 298
#define INTERRED_CMD 299
#define INTERSECT_CMD 300
#define JACOB_CMD 301
#define JET_CMD 302
#define KBASE_CMD 303
#define KOSZUL_CMD 304
#define LEADCOEF_CMD 305
#define LEADEXP_CMD 306
#define LEAD_CMD 307
#define LEADMONOM_CMD 308
#define LIFTSTD_CMD 309
#define LIFT_CMD 310
#define MAXID_CMD 311
#define MINBASE_CMD 312
#define MINOR_CMD 313
#define MINRES_CMD 314
#define MODULO_CMD 315
#define MONOM_CMD 316
#define MRES_CMD 317
#define MULTIPLICITY_CMD 318
#define ORD_CMD 319
#define PAR_CMD 320
#define PARDEG_CMD 321
#define PREIMAGE_CMD 322
#define QUOTIENT_CMD 323
#define QHWEIGHT_CMD 324
#define REDUCE_CMD 325
#define REGULARITY_CMD 326
#define RES_CMD 327
#define SIMPLIFY_CMD 328
#define SORTVEC_CMD 329
#define SRES_CMD 330
#define STD_CMD 331
#define SUBST_CMD 332
#define SYZYGY_CMD 333
#define VAR_CMD 334
#define VDIM_CMD 335
#define WEDGE_CMD 336
#define WEIGHT_CMD 337
#define VALTVARS 338
#define VMAXDEG 339
#define VMAXMULT 340
#define VNOETHER 341
#define VMINPOLY 342
#define END_RING 343
#define CMD_1 344
#define CMD_2 345
#define CMD_3 346
#define CMD_12 347
#define CMD_13 348
#define CMD_23 349
#define CMD_123 350
#define CMD_M 351
#define ROOT_DECL 352
#define ROOT_DECL_LIST 353
#define RING_DECL 354
#define EXAMPLE_CMD 355
#define EXPORT_CMD 356
#define HELP_CMD 357
#define KILL_CMD 358
#define LIB_CMD 359
#define LISTVAR_CMD 360
#define SETRING_CMD 361
#define TYPE_CMD 362
#define STRINGTOK 363
#define BLOCKTOK 364
#define INT_CONST 365
#define UNKNOWN_IDENT 366
#define RINGVAR 367
#define PROC_DEF 368
#define BREAK_CMD 369
#define CONTINUE_CMD 370
#define ELSE_CMD 371
#define EVAL 372
#define QUOTE 373
#define FOR_CMD 374
#define IF_CMD 375
#define SYS_BREAK 376
#define WHILE_CMD 377
#define RETURN 378
#define PARAMETER 379
#define SYSVAR 380
#define UMINUS 381




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



