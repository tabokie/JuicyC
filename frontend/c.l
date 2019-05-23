D           [0-9]
L           [a-zA-Z_]
H           [a-fA-F0-9]
E           ([Ee][+-]?{D}+)
P           ([Pp][+-]?{D}+)
FS          (f|F|l|L)
IS          ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%{
#include <cstdio>
#include <iostream>
#include "gen_parse.hh"
#include "front_env.h"
#include "juicyc/symbol.h"

using namespace juicyc;

int line = 0;
int column = 0;
uint16_t file = 0;

// hook scanner input to
// customized preprocessor.
// At most one line once, good for resetting
// file and line number. 
#define YY_INPUT(buf, result, max_size)  \
  result = 0; \
  while (1) { \
    int c = FrontEnv::pp->get(); \
    if (FrontEnv::pp->eof()) { \
      break; \
    } \
    buf[result++] = c; \
    if (result == max_size || c == '\n') { \
      break; \
    } \
  }

#define MAKE_TOKEN(token) \
  Terminal* tmp = new Terminal(); \
  yylval.sym = tmp; \
  tmp->value = yytext; \
  tmp->type = token; \
  tmp->file = file; \
  tmp->line = line; \
  tmp->col = column; \
  return token;


void count(void);
void comment(void);
int check_type(void);
%}

/* if to use original flex on Windows,
 * please add following options:
 * %option nounistd
 * %option never-interactive
 * by Tabokie
 */

%%
"/*"            { comment(); }
"//"[^\n]*      { /* consume //-comment */ }


"auto"          { count(); MAKE_TOKEN(AUTO) }
"bool"          { count(); MAKE_TOKEN(TOKEN_BOOL) }
"break"         { count(); MAKE_TOKEN(BREAK) }
"case"          { count(); MAKE_TOKEN(CASE) }
"char"          { count(); MAKE_TOKEN(TOKEN_CHAR) }
"const"         { count(); MAKE_TOKEN(TOKEN_CONST) }
"continue"      { count(); MAKE_TOKEN(CONTINUE) }
"default"       { count(); MAKE_TOKEN(DEFAULT) }
"do"            { count(); MAKE_TOKEN(DO) }
"double"        { count(); MAKE_TOKEN(TOKEN_DOUBLE) }
"else"          { count(); MAKE_TOKEN(ELSE) }
"enum"          { count(); MAKE_TOKEN(ENUM) }
"extern"        { count(); MAKE_TOKEN(EXTERN) }
"float"         { count(); MAKE_TOKEN(TOKEN_FLOAT) }
"for"           { count(); MAKE_TOKEN(FOR) }
"goto"          { count(); MAKE_TOKEN(GOTO) }
"if"            { count(); MAKE_TOKEN(IF) }
"inline"        { count(); MAKE_TOKEN(INLINE) }
"int"           { count(); MAKE_TOKEN(TOKEN_INT) }
"long"          { count(); MAKE_TOKEN(TOKEN_LONG) }
"register"      { count(); MAKE_TOKEN(REGISTER) }
"restrict"      { count(); MAKE_TOKEN(RESTRICT) }
"return"        { count(); MAKE_TOKEN(RETURN) }
"short"         { count(); MAKE_TOKEN(TOKEN_SHORT) }
"signed"        { count(); MAKE_TOKEN(TOKEN_SIGNED) }
"sizeof"        { count(); MAKE_TOKEN(SIZEOF) }
"static"        { count(); MAKE_TOKEN(STATIC) }
"struct"        { count(); MAKE_TOKEN(STRUCT) }
"switch"        { count(); MAKE_TOKEN(SWITCH) }
"typedef"       { count(); MAKE_TOKEN(TYPEDEF) }
"union"         { count(); MAKE_TOKEN(UNION) }
"unsigned"      { count(); MAKE_TOKEN(TOKEN_UNSIGNED) }
"void"          { count(); MAKE_TOKEN(TOKEN_VOID) }
"volatile"      { count(); MAKE_TOKEN(VOLATILE) }
"while"         { count(); MAKE_TOKEN(WHILE) }

{L}({L}|{D})*       { count(); MAKE_TOKEN(check_type()) }

0[xX]{H}+{IS}?      { count(); MAKE_TOKEN(CONSTANT) }
0[0-7]*{IS}?        { count(); MAKE_TOKEN(CONSTANT) }
[1-9]{D}*{IS}?      { count(); MAKE_TOKEN(CONSTANT) }
L?'(\\.|[^\\'\n])+' { count(); MAKE_TOKEN(CONSTANT) }

{D}+{E}{FS}?                { count(); MAKE_TOKEN(CONSTANT) }
{D}*"."{D}+{E}?{FS}?        { count(); MAKE_TOKEN(CONSTANT) }
{D}+"."{D}*{E}?{FS}?        { count(); MAKE_TOKEN(CONSTANT) }
0[xX]{H}+{P}{FS}?           { count(); MAKE_TOKEN(CONSTANT) }
0[xX]{H}*"."{H}+{P}{FS}?    { count(); MAKE_TOKEN(CONSTANT) }
0[xX]{H}+"."{H}*{P}{FS}?    { count(); MAKE_TOKEN(CONSTANT) }


L?\"(\\.|[^\\"\n])*\"   { count(); MAKE_TOKEN(STRING_LITERAL) }

"..."           { count(); MAKE_TOKEN(ELLIPSIS) }
">>="           { count(); MAKE_TOKEN(RIGHT_ASSIGN) }
"<<="           { count(); MAKE_TOKEN(LEFT_ASSIGN) }
"+="            { count(); MAKE_TOKEN(ADD_ASSIGN) }
"-="            { count(); MAKE_TOKEN(SUB_ASSIGN) }
"*="            { count(); MAKE_TOKEN(MUL_ASSIGN) }
"/="            { count(); MAKE_TOKEN(DIV_ASSIGN) }
"%="            { count(); MAKE_TOKEN(MOD_ASSIGN) }
"&="            { count(); MAKE_TOKEN(AND_ASSIGN) }
"^="            { count(); MAKE_TOKEN(XOR_ASSIGN) }
"|="            { count(); MAKE_TOKEN(OR_ASSIGN) }
">>"            { count(); MAKE_TOKEN(RIGHT_OP) }
"<<"            { count(); MAKE_TOKEN(LEFT_OP) }
"++"            { count(); MAKE_TOKEN(INC_OP) }
"--"            { count(); MAKE_TOKEN(DEC_OP) }
"->"            { count(); MAKE_TOKEN(PTR_OP) }
"&&"            { count(); MAKE_TOKEN(AND_OP) }
"||"            { count(); MAKE_TOKEN(OR_OP) }
"<="            { count(); MAKE_TOKEN(LE_OP) }
">="            { count(); MAKE_TOKEN(GE_OP) }
"=="            { count(); MAKE_TOKEN(EQ_OP) }
"!="            { count(); MAKE_TOKEN(NE_OP) }
";"             { count(); MAKE_TOKEN(';') }
("{"|"<%")      { count(); MAKE_TOKEN('{') }
("}"|"%>")      { count(); MAKE_TOKEN('}') }
","             { count(); MAKE_TOKEN(',') }
":"             { count(); MAKE_TOKEN(':') }
"="             { count(); MAKE_TOKEN('=') }
"("             { count(); MAKE_TOKEN('(') }
")"             { count(); MAKE_TOKEN(')') }
("["|"<:")      { count(); MAKE_TOKEN('[') }
("]"|":>")      { count(); MAKE_TOKEN(']') }
"."             { count(); MAKE_TOKEN('.') }
"&"             { count(); MAKE_TOKEN('&') }
"!"             { count(); MAKE_TOKEN('!') }
"~"             { count(); MAKE_TOKEN('~') }
"-"             { count(); MAKE_TOKEN('-') }
"+"             { count(); MAKE_TOKEN('+') }
"*"             { count(); MAKE_TOKEN('*') }
"/"             { count(); MAKE_TOKEN('/') }
"%"             { count(); MAKE_TOKEN('%') }
"<"             { count(); MAKE_TOKEN('<') }
">"             { count(); MAKE_TOKEN('>') }
"^"             { count(); MAKE_TOKEN('^') }
"|"             { count(); MAKE_TOKEN('|') }
"?"             { count(); MAKE_TOKEN('?') }

[ \t\v\n\f]     { count(); }
.               { /* Add code to complain about unmatched characters */ }

%%

int yywrap(void)
{
    return 1;
}


void comment(void)
{
    // char c, prev = 0;
  
    // while ((c = input()) != 0)      /* (EOF maps to 0) */
    // {
    //     if (c == '/' && prev == '*')
    //         return;
    //     prev = c;
    // }
    // yyerror("unterminated comment");
}

// called by preprocessor

void set_count(int l, int c) {
  line = l;
  column = c;
}

void set_file(int f) {
  file = f;
}

void count(void)
{
  int i;
  for (i = 0; yytext[i] != '\0'; i++)
    if (yytext[i] == '\n') {
      line ++;
      column = 0;
    }
    else if (yytext[i] == '\t')
      column += 8 - (column % 8);
    else
      column++;
}


int check_type(void)
{
/*
* pseudo code --- this is what it should check
*
*   if (yytext == type_name)
*       return TYPE_NAME;
*
*   return IDENTIFIER;
*/

/*
*   it actually will only return IDENTIFIER
*/

    return IDENTIFIER;
}