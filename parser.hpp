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
     LSB = 258,
     RSB = 259,
     LMB = 260,
     RMB = 261,
     LLB = 262,
     RLB = 263,
     DOT = 264,
     COLON = 265,
     SEMI = 266,
     COMMA = 267,
     PLUS = 268,
     MINUS = 269,
     MUL = 270,
     DIV = 271,
     MOD = 272,
     XOR = 273,
     AND = 274,
     OR = 275,
     GT = 276,
     GE = 277,
     LT = 278,
     LE = 279,
     NE = 280,
     EQ = 281,
     ASSIGN = 282,
     QUOTE = 283,
     IF = 284,
     ELSE = 285,
     WHILE = 286,
     FOR = 287,
     DO = 288,
     BREAK = 289,
     CONTINUE = 290,
     SWITCH = 291,
     CASE = 292,
     DEFAULT = 293,
     INT = 294,
     CHAR = 295,
     DOUBLE = 296,
     FLOAT = 297,
     BOOLEAN = 298,
     CONST = 299,
     ENUM = 300,
     STRING = 301,
     NEW = 302,
     CLASS = 303,
     THIS = 304,
     TRY = 305,
     CATCH = 306,
     THROW = 307,
     PUBLIC = 308,
     PRIVATE = 309,
     PROTECTED = 310,
     SIZEOF = 311,
     VOID = 312,
     ID = 313
   };
#endif
/* Tokens.  */
#define LSB 258
#define RSB 259
#define LMB 260
#define RMB 261
#define LLB 262
#define RLB 263
#define DOT 264
#define COLON 265
#define SEMI 266
#define COMMA 267
#define PLUS 268
#define MINUS 269
#define MUL 270
#define DIV 271
#define MOD 272
#define XOR 273
#define AND 274
#define OR 275
#define GT 276
#define GE 277
#define LT 278
#define LE 279
#define NE 280
#define EQ 281
#define ASSIGN 282
#define QUOTE 283
#define IF 284
#define ELSE 285
#define WHILE 286
#define FOR 287
#define DO 288
#define BREAK 289
#define CONTINUE 290
#define SWITCH 291
#define CASE 292
#define DEFAULT 293
#define INT 294
#define CHAR 295
#define DOUBLE 296
#define FLOAT 297
#define BOOLEAN 298
#define CONST 299
#define ENUM 300
#define STRING 301
#define NEW 302
#define CLASS 303
#define THIS 304
#define TRY 305
#define CATCH 306
#define THROW 307
#define PUBLIC 308
#define PRIVATE 309
#define PROTECTED 310
#define SIZEOF 311
#define VOID 312
#define ID 313




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

