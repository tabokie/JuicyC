%{
#include <cstdlib>
#include <cstdio>
#include "front_env.h"

void yyerror(const char *s);
int yylex();

using juicyc::Terminal;
using juicyc::NonTerminal;
using juicyc::UnaryExpression;
using juicyc::BinaryExpression;
using juicyc::TernaryExpression;

template <typename SymType, typename YaccType, class ...YaccChild>
void BUILD(const char* name, YaccType& root, YaccType& first, YaccChild... childs) {
  SymType *tmp = new SymType();
  tmp->type = juicyc::FrontEnv::Tag(name);
  tmp->childs = first;
  juicyc::Symbol::MakeSibling(first, childs...);
  root = tmp;
}

template <typename SymType, typename YaccType>
void BUILD(const char* name, YaccType& root, YaccType& child) {
  SymType *tmp = new SymType();
  tmp->type = juicyc::FrontEnv::Tag(name);
  tmp->childs = child;
  root = tmp;
}

%}

// included in header
%code requires {
#include "juicyc/symbol.h"
}

%union{
  juicyc::Symbol* sym;
}

%token <sym> IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token <sym> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token <sym> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token <sym> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token <sym> XOR_ASSIGN OR_ASSIGN TYPE_NAME
%token <sym> ';' '{' '}' ',' ':' '=' '(' ')' '[' ']' '.' '&' '!' '~' '-'
%token <sym> '+' '*' '/' '%' '<' '>' '^' '|' '?'

%token <sym> TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
/* due to built-in macro on c-type,
 * add TOKEN_ prefix to several types
 * and quantifier.
 * by Tabokie
 */
%token <sym> TOKEN_CHAR TOKEN_SHORT TOKEN_INT TOKEN_LONG TOKEN_SIGNED TOKEN_UNSIGNED TOKEN_FLOAT TOKEN_DOUBLE
%token <sym> TOKEN_CONST VOLATILE TOKEN_VOID TOKEN_BOOL
// %token COMPLEX IMAGINARY
%token <sym> STRUCT UNION ENUM ELLIPSIS

%token <sym> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

/* eliminate reduce/shift conflict by
 * assign if-else higher priority than
 * if alone.
 * by Tabokie
 */
%token <sym> LOWER_THAN_ELSE
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%type <sym> primary_expression postfix_expression argument_expression_list unary_expression unary_operator
%type <sym> cast_expression multiplicative_expression additive_expression shift_expression
%type <sym> relational_expression equality_expression and_expression exclusive_or_expression
%type <sym> inclusive_or_expression logical_and_expression logical_or_expression conditional_expression
%type <sym> assignment_expression assignment_operator expression constant_expression declaration
%type <sym> declaration_specifiers init_declarator_list init_declarator storage_class_specifier
%type <sym> type_specifier struct_or_union_specifier struct_or_union struct_declaration_list
%type <sym> struct_declaration specifier_qualifier_list struct_declarator_list struct_declarator
%type <sym> enum_specifier enumerator_list enumerator type_qualifier function_specifier declarator
%type <sym> direct_declarator pointer type_qualifier_list parameter_type_list parameter_list
%type <sym> parameter_declaration identifier_list type_name abstract_declarator direct_abstract_declarator
%type <sym> initializer initializer_list designation designator_list designator statement
%type <sym> labeled_statement compound_statement block_item_list block_item expression_statement
%type <sym> selection_statement iteration_statement jump_statement translation_unit external_declaration
%type <sym> function_definition declaration_list

%start translation_unit
%%

primary_expression
  : IDENTIFIER {   NonTerminal* tmp = new NonTerminal(); tmp->type = juicyc::FrontEnv::Tag("primary_expression"); tmp->childs = $1; $$ = tmp;}
  | CONSTANT {BUILD<NonTerminal>("primary_expression", $$, $1);}
  | STRING_LITERAL {BUILD<NonTerminal>("primary_expression", $$, $1);}
  | '(' expression ')' {BUILD<NonTerminal>("primary_expression", $$, $1, $2, $3);}
  ;

postfix_expression
  : primary_expression {BUILD<NonTerminal>("postfix_expression", $$, $1);}
  | postfix_expression '[' expression ']' {BUILD<NonTerminal>("postfix_expression", $$, $1, $2, $3, $4);}
  | postfix_expression '(' ')' {BUILD<NonTerminal>("postfix_expression", $$, $1, $2, $3);}
  | postfix_expression '(' argument_expression_list ')' {BUILD<NonTerminal>("postfix_expression", $$, $1, $2, $3, $4);}
  | postfix_expression '.' IDENTIFIER {BUILD<NonTerminal>("postfix_expression", $$, $1, $2, $3);}
  | postfix_expression PTR_OP IDENTIFIER {BUILD<NonTerminal>("postfix_expression", $$, $1, $2, $3);}
  | postfix_expression INC_OP {BUILD<NonTerminal>("postfix_expression", $$, $1, $2);}
  | postfix_expression DEC_OP {BUILD<NonTerminal>("postfix_expression", $$, $1, $2);}
  | '(' type_name ')' '{' initializer_list '}' {BUILD<NonTerminal>("postfix_expression", $$, $1, $2, $3, $4, $5, $6);}
  | '(' type_name ')' '{' initializer_list ',' '}' {BUILD<NonTerminal>("postfix_expression", $$, $1, $2, $3, $4, $5, $7);}
  ;

argument_expression_list
  : assignment_expression {BUILD<NonTerminal>("argument_expression_list", $$, $1);}
  | argument_expression_list ',' assignment_expression {BUILD<NonTerminal>("argument_expression_list", $$, $1, $2, $3);}
  ;

unary_expression
  : postfix_expression {BUILD<UnaryExpression>("unary_expression", $$, $1);}
  | INC_OP unary_expression {BUILD<UnaryExpression>("unary_expression", $$, $1, $2);}
  | DEC_OP unary_expression {BUILD<UnaryExpression>("unary_expression", $$, $1, $2);}
  | unary_operator cast_expression {BUILD<UnaryExpression>("unary_expression", $$, $1, $2);}
  | SIZEOF unary_expression {BUILD<UnaryExpression>("unary_expression", $$, $1, $2);}
  | SIZEOF '(' type_name ')' {BUILD<UnaryExpression>("unary_expression", $$, $1, $2, $3, $4);}
  ;

unary_operator
  : '&' {BUILD<NonTerminal>("unary_operator", $$, $1);}
  | '*' {BUILD<NonTerminal>("unary_operator", $$, $1);}
  | '+' {BUILD<NonTerminal>("unary_operator", $$, $1);}
  | '-' {BUILD<NonTerminal>("unary_operator", $$, $1);}
  | '~' {BUILD<NonTerminal>("unary_operator", $$, $1);}
  | '!' {BUILD<NonTerminal>("unary_operator", $$, $1);}
  ;

cast_expression
  : unary_expression {BUILD<NonTerminal>("cast_expression", $$, $1);}
  | '(' type_name ')' cast_expression {BUILD<NonTerminal>("cast_expression", $$, $1, $2, $3, $4);}
  ;

multiplicative_expression
  : cast_expression {BUILD<BinaryExpression>("multiplicative_expression", $$, $1);}
  | multiplicative_expression '*' cast_expression {BUILD<BinaryExpression>("multiplicative_expression", $$, $1, $2, $3);}
  | multiplicative_expression '/' cast_expression {BUILD<BinaryExpression>("multiplicative_expression", $$, $1, $2, $3);}
  | multiplicative_expression '%' cast_expression {BUILD<BinaryExpression>("multiplicative_expression", $$, $1, $2, $3);}
  ;

additive_expression
  : multiplicative_expression {BUILD<BinaryExpression>("additive_expression", $$, $1);}
  | additive_expression '+' multiplicative_expression {BUILD<BinaryExpression>("additive_expression", $$, $1, $2, $3);}
  | additive_expression '-' multiplicative_expression {BUILD<BinaryExpression>("additive_expression", $$, $1, $2, $3);}
  ;

shift_expression
  : additive_expression {BUILD<BinaryExpression>("shift_expression", $$, $1);}
  | shift_expression LEFT_OP additive_expression {BUILD<BinaryExpression>("shift_expression", $$, $1, $2, $3);}
  | shift_expression RIGHT_OP additive_expression {BUILD<BinaryExpression>("shift_expression", $$, $1, $2, $3);}
  ;

relational_expression
  : shift_expression {BUILD<BinaryExpression>("relational_expression", $$, $1);}
  | relational_expression '<' shift_expression {BUILD<BinaryExpression>("relational_expression", $$, $1, $2, $3);}
  | relational_expression '>' shift_expression {BUILD<BinaryExpression>("relational_expression", $$, $1, $2, $3);}
  | relational_expression LE_OP shift_expression {BUILD<BinaryExpression>("relational_expression", $$, $1, $2, $3);}
  | relational_expression GE_OP shift_expression {BUILD<BinaryExpression>("relational_expression", $$, $1, $2, $3);}
  ;

equality_expression
  : relational_expression {BUILD<BinaryExpression>("equality_expression", $$, $1);}
  | equality_expression EQ_OP relational_expression {BUILD<BinaryExpression>("equality_expression", $$, $1, $2, $3);}
  | equality_expression NE_OP relational_expression {BUILD<BinaryExpression>("equality_expression", $$, $1, $2, $3);}
  ;

and_expression
  : equality_expression {BUILD<BinaryExpression>("and_expression", $$, $1);}
  | and_expression '&' equality_expression {BUILD<BinaryExpression>("and_expression", $$, $1, $2, $3);}
  ;

exclusive_or_expression
  : and_expression {BUILD<BinaryExpression>("exclusive_or_expression", $$, $1);}
  | exclusive_or_expression '^' and_expression {BUILD<BinaryExpression>("exclusive_or_expression", $$, $1, $2, $3);}
  ;

inclusive_or_expression
  : exclusive_or_expression {BUILD<BinaryExpression>("inclusive_or_expression", $$, $1);}
  | inclusive_or_expression '|' exclusive_or_expression {BUILD<BinaryExpression>("inclusive_or_expression", $$, $1, $2, $3);}
  ;

logical_and_expression
  : inclusive_or_expression {BUILD<BinaryExpression>("logical_and_expression", $$, $1);}
  | logical_and_expression AND_OP inclusive_or_expression {BUILD<BinaryExpression>("logical_and_expression", $$, $1, $2, $3);}
  ;

logical_or_expression
  : logical_and_expression {BUILD<BinaryExpression>("logical_or_expression", $$, $1);}
  | logical_or_expression OR_OP logical_and_expression {BUILD<BinaryExpression>("logical_or_expression", $$, $1, $2, $3);}
  ;

conditional_expression
  : logical_or_expression {BUILD<TernaryExpression>("conditional_expression", $$, $1);}
  | logical_or_expression '?' expression ':' conditional_expression {BUILD<TernaryExpression>("conditional_expression", $$, $1, $2, $3, $4, $5);}
  ;

assignment_expression
  : conditional_expression {BUILD<NonTerminal>("assignment_expression", $$, $1);}
  | unary_expression assignment_operator assignment_expression {BUILD<NonTerminal>("assignment_expression", $$, $1, $2, $3);}
  ;

assignment_operator
  : '=' {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | MUL_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | DIV_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | MOD_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | ADD_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | SUB_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | LEFT_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | RIGHT_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | AND_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | XOR_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  | OR_ASSIGN {BUILD<NonTerminal>("assignment_operator", $$, $1);}
  ;

expression
  : assignment_expression {BUILD<NonTerminal>("expression", $$, $1);}
  | expression ',' assignment_expression {BUILD<NonTerminal>("expression", $$, $1, $2, $3);}
  ;

constant_expression
  : conditional_expression {BUILD<NonTerminal>("constant_expression", $$, $1);}
  ;

declaration
  : declaration_specifiers ';' {BUILD<NonTerminal>("declaration", $$, $1, $2);}
  | declaration_specifiers init_declarator_list ';' {BUILD<NonTerminal>("declaration", $$, $1, $2, $3);}
  ;

declaration_specifiers
  : storage_class_specifier {BUILD<NonTerminal>("declaration_specifiers", $$, $1);}
  | storage_class_specifier declaration_specifiers {BUILD<NonTerminal>("declaration_specifiers", $$, $1, $2);}
  | type_specifier {BUILD<NonTerminal>("declaration_specifiers", $$, $1);}
  | type_specifier declaration_specifiers {BUILD<NonTerminal>("declaration_specifiers", $$, $1, $2);}
  | type_qualifier {BUILD<NonTerminal>("declaration_specifiers", $$, $1);}
  | type_qualifier declaration_specifiers {BUILD<NonTerminal>("declaration_specifiers", $$, $1, $2);}
  | function_specifier {BUILD<NonTerminal>("declaration_specifiers", $$, $1);}
  | function_specifier declaration_specifiers {BUILD<NonTerminal>("declaration_specifiers", $$, $1, $2);}
  ;

init_declarator_list
  : init_declarator {BUILD<NonTerminal>("init_declarator_list", $$, $1);}
  | init_declarator_list ',' init_declarator {BUILD<NonTerminal>("init_declarator_list", $$, $1, $2, $3);}
  ;

init_declarator
  : declarator {BUILD<NonTerminal>("init_declarator", $$, $1);}
  | declarator '=' initializer {BUILD<NonTerminal>("init_declarator", $$, $1, $2, $3);}
  ;

storage_class_specifier
  : TYPEDEF {BUILD<NonTerminal>("storage_class_specifier", $$, $1);}
  | EXTERN {BUILD<NonTerminal>("storage_class_specifier", $$, $1);}
  | STATIC {BUILD<NonTerminal>("storage_class_specifier", $$, $1);}
  | AUTO {BUILD<NonTerminal>("storage_class_specifier", $$, $1);}
  | REGISTER {BUILD<NonTerminal>("storage_class_specifier", $$, $1);}
  ;

type_specifier
  : TOKEN_VOID {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_CHAR {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_SHORT {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_INT {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_LONG {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_FLOAT {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_DOUBLE {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_SIGNED {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_UNSIGNED {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TOKEN_BOOL {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | struct_or_union_specifier {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | enum_specifier {BUILD<NonTerminal>("type_specifier", $$, $1);}
  | TYPE_NAME {BUILD<NonTerminal>("type_specifier", $$, $1);}
  ;

struct_or_union_specifier
  : struct_or_union IDENTIFIER '{' struct_declaration_list '}' {BUILD<NonTerminal>("struct_or_union_specifier", $$, $1, $2, $3, $4, $5);}
  | struct_or_union '{' struct_declaration_list '}' {BUILD<NonTerminal>("struct_or_union_specifier", $$, $1, $2, $3, $4);}
  | struct_or_union IDENTIFIER {BUILD<NonTerminal>("struct_or_union_specifier", $$, $1, $2);}
  ;

struct_or_union
  : STRUCT {BUILD<NonTerminal>("struct_or_union", $$, $1);}
  | UNION {BUILD<NonTerminal>("struct_or_union", $$, $1);}
  ;

struct_declaration_list
  : struct_declaration {BUILD<NonTerminal>("struct_declaration_list", $$, $1);}
  | struct_declaration_list struct_declaration {BUILD<NonTerminal>("struct_declaration_list", $$, $1, $2);}
  ;

struct_declaration
  : specifier_qualifier_list struct_declarator_list ';' {BUILD<NonTerminal>("struct_declaration", $$, $1, $2, $3);}
  ;

specifier_qualifier_list
  : type_specifier specifier_qualifier_list {BUILD<NonTerminal>("specifier_qualifier_list", $$, $1, $2);}
  | type_specifier {BUILD<NonTerminal>("specifier_qualifier_list", $$, $1);}
  | type_qualifier specifier_qualifier_list {BUILD<NonTerminal>("specifier_qualifier_list", $$, $1, $2);}
  | type_qualifier {BUILD<NonTerminal>("specifier_qualifier_list", $$, $1);}
  ;

struct_declarator_list
  : struct_declarator {BUILD<NonTerminal>("struct_declarator_list", $$, $1);}
  | struct_declarator_list ',' struct_declarator {BUILD<NonTerminal>("struct_declarator_list", $$, $1, $2, $3);}
  ;

struct_declarator
  : declarator {BUILD<NonTerminal>("struct_declarator", $$, $1);}
  | ':' constant_expression {BUILD<NonTerminal>("struct_declarator", $$, $1, $2);}
  | declarator ':' constant_expression {BUILD<NonTerminal>("struct_declarator", $$, $1, $2, $3);}
  ;

enum_specifier
  : ENUM '{' enumerator_list '}' {BUILD<NonTerminal>("enum_specifier", $$, $1, $2, $3, $4);}
  | ENUM IDENTIFIER '{' enumerator_list '}' {BUILD<NonTerminal>("enum_specifier", $$, $1, $2, $3, $4, $5);}
  | ENUM '{' enumerator_list ',' '}' {BUILD<NonTerminal>("enum_specifier", $$, $1, $2, $3, $4, $5);}
  | ENUM IDENTIFIER '{' enumerator_list ',' '}' {BUILD<NonTerminal>("enum_specifier", $$, $1, $2, $3, $4, $5, $6);}
  | ENUM IDENTIFIER {BUILD<NonTerminal>("enum_specifier", $$, $1);}
  ;

enumerator_list
  : enumerator {BUILD<NonTerminal>("enumerator_list", $$, $1);}
  | enumerator_list ',' enumerator {BUILD<NonTerminal>("enumerator_list", $$, $1, $2, $3);}
  ;

enumerator
  : IDENTIFIER {BUILD<NonTerminal>("enumerator", $$, $1);}
  | IDENTIFIER '=' constant_expression {BUILD<NonTerminal>("enumerator", $$, $1, $2, $3);}
  ;

type_qualifier
  : TOKEN_CONST {BUILD<NonTerminal>("type_qualifier", $$, $1);}
  | RESTRICT {BUILD<NonTerminal>("type_qualifier", $$, $1);}
  | VOLATILE {BUILD<NonTerminal>("type_qualifier", $$, $1);}
  ;

function_specifier
  : INLINE {BUILD<NonTerminal>("function_specifier", $$, $1);}
  ;

declarator
  : pointer direct_declarator {BUILD<NonTerminal>("declarator", $$, $1, $2);}
  | direct_declarator {BUILD<NonTerminal>("declarator", $$, $1);}
  ;


direct_declarator
  : IDENTIFIER {BUILD<NonTerminal>("direct_declarator", $$, $1);}
  | '(' declarator ')' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3);}
  | direct_declarator '[' type_qualifier_list assignment_expression ']' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4, $5);}
  | direct_declarator '[' type_qualifier_list ']' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4);}
  | direct_declarator '[' assignment_expression ']' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4);}
  | direct_declarator '[' STATIC type_qualifier_list assignment_expression ']' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4, $5, $6);}
  | direct_declarator '[' type_qualifier_list STATIC assignment_expression ']' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4, $5, $6);}
  | direct_declarator '[' type_qualifier_list '*' ']' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4, $5);}
  | direct_declarator '[' '*' ']' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4);}
  | direct_declarator '[' ']' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3);}
  | direct_declarator '(' parameter_type_list ')' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4);}
  | direct_declarator '(' identifier_list ')' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3, $4);}
  | direct_declarator '(' ')' {BUILD<NonTerminal>("direct_declarator", $$, $1, $2, $3);}
  ;

pointer
  : '*' {BUILD<NonTerminal>("pointer", $$, $1);}
  | '*' type_qualifier_list {BUILD<NonTerminal>("pointer", $$, $1, $2);}
  | '*' pointer {BUILD<NonTerminal>("pointer", $$, $1, $2);}
  | '*' type_qualifier_list pointer {BUILD<NonTerminal>("pointer", $$, $1, $2, $3);}
  ;

type_qualifier_list
  : type_qualifier {BUILD<NonTerminal>("type_qualifier_list", $$, $1);}
  | type_qualifier_list type_qualifier {BUILD<NonTerminal>("type_qualifier_list", $$, $1, $2);}
  ;


parameter_type_list
  : parameter_list {BUILD<NonTerminal>("parameter_type_list", $$, $1);}
  | parameter_list ',' ELLIPSIS {BUILD<NonTerminal>("parameter_type_list", $$, $1, $2, $3);}
  ;

parameter_list
  : parameter_declaration {BUILD<NonTerminal>("parameter_list", $$, $1);}
  | parameter_list ',' parameter_declaration {BUILD<NonTerminal>("parameter_list", $$, $1, $2, $3);}
  ;

parameter_declaration
  : declaration_specifiers declarator {BUILD<NonTerminal>("parameter_declaration", $$, $1, $2);}
  | declaration_specifiers abstract_declarator {BUILD<NonTerminal>("parameter_declaration", $$, $1, $2);}
  | declaration_specifiers {BUILD<NonTerminal>("parameter_declaration", $$, $1);}
  ;

identifier_list
  : IDENTIFIER {BUILD<NonTerminal>("identifier_list", $$, $1);}
  | identifier_list ',' IDENTIFIER {BUILD<NonTerminal>("identifier_list", $$, $1, $2, $3);}
  ;

type_name
  : specifier_qualifier_list {BUILD<NonTerminal>("type_name", $$, $1);}
  | specifier_qualifier_list abstract_declarator {BUILD<NonTerminal>("type_name", $$, $1, $2);}
  ;

abstract_declarator
  : pointer {BUILD<NonTerminal>("abstract_declarator", $$, $1);}
  | direct_abstract_declarator {BUILD<NonTerminal>("abstract_declarator", $$, $1);}
  | pointer direct_abstract_declarator {BUILD<NonTerminal>("abstract_declarator", $$, $1, $2);}
  ;

direct_abstract_declarator
  : '(' abstract_declarator ')' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3);}
  | '[' ']' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2);}
  | '[' assignment_expression ']' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3);}
  | direct_abstract_declarator '[' ']' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3);}
  | direct_abstract_declarator '[' assignment_expression ']' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3, $4);}
  | '[' '*' ']' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3);}
  | direct_abstract_declarator '[' '*' ']' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3, $4);}
  | '(' ')' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2);}
  | '(' parameter_type_list ')' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3);}
  | direct_abstract_declarator '(' ')' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3);}
  | direct_abstract_declarator '(' parameter_type_list ')' {BUILD<NonTerminal>("direct_abstract_declarator", $$, $1, $2, $3, $4);}
  ;

initializer
  : assignment_expression {BUILD<NonTerminal>("initializer", $$, $1);}
  | '{' initializer_list '}' {BUILD<NonTerminal>("initializer", $$, $1, $2, $3);}
  | '{' initializer_list ',' '}' {BUILD<NonTerminal>("initializer", $$, $1, $2, $3, $4);}
  ;

initializer_list
  : initializer {BUILD<NonTerminal>("initializer_list", $$, $1);}
  | designation initializer {BUILD<NonTerminal>("initializer_list", $$, $1, $2);}
  | initializer_list ',' initializer {BUILD<NonTerminal>("initializer_list", $$, $1, $2, $3);}
  | initializer_list ',' designation initializer {BUILD<NonTerminal>("initializer_list", $$, $1, $2, $3, $4);}
  ;

designation
  : designator_list '=' {BUILD<NonTerminal>("designation", $$, $1, $2);}
  ;

designator_list
  : designator {BUILD<NonTerminal>("designator_list", $$, $1);}
  | designator_list designator {BUILD<NonTerminal>("designator_list", $$, $1, $2);}
  ;

designator
  : '[' constant_expression ']' {BUILD<NonTerminal>("designator", $$, $1, $2, $3);}
  | '.' IDENTIFIER {BUILD<NonTerminal>("designator", $$, $1, $2);}
  ;

statement
  : labeled_statement {BUILD<NonTerminal>("statement", $$, $1);}
  | compound_statement {BUILD<NonTerminal>("statement", $$, $1);}
  | expression_statement {BUILD<NonTerminal>("statement", $$, $1);}
  | selection_statement {BUILD<NonTerminal>("statement", $$, $1);}
  | iteration_statement {BUILD<NonTerminal>("statement", $$, $1);}
  | jump_statement {BUILD<NonTerminal>("statement", $$, $1);}
  ;

labeled_statement
  : IDENTIFIER ':' statement {BUILD<NonTerminal>("labeled_statement", $$, $1, $2, $3);}
  | CASE constant_expression ':' statement {BUILD<NonTerminal>("labeled_statement", $$, $1, $2, $3, $4);}
  | DEFAULT ':' statement {BUILD<NonTerminal>("labeled_statement", $$, $1, $2, $3);}
  ;

compound_statement
  : '{' '}' {BUILD<NonTerminal>("compound_statement", $$, $1, $2);}
  | '{' block_item_list '}' {BUILD<NonTerminal>("compound_statement", $$, $1, $2, $3);}
  ;

block_item_list
  : block_item {BUILD<NonTerminal>("block_item_list", $$, $1);}
  | block_item_list block_item {BUILD<NonTerminal>("block_item_list", $$, $1, $2);}
  ;

block_item
  : declaration {BUILD<NonTerminal>("block_item", $$, $1);}
  | statement {BUILD<NonTerminal>("block_item", $$, $1);}
  ;

expression_statement
  : ';' {BUILD<NonTerminal>("expression_statement", $$, $1);}
  | expression ';' {BUILD<NonTerminal>("expression_statement", $$, $1, $2);}
  ;

selection_statement
  : IF '(' expression ')' statement %prec LOWER_THAN_ELSE
  | IF '(' expression ')' statement ELSE statement {BUILD<NonTerminal>("selection_statement", $$, $1, $2, $3, $4, $5, $6, $7);}
  | SWITCH '(' expression ')' statement {BUILD<NonTerminal>("selection_statement", $$, $1, $2, $3, $4, $5);}
  ;

iteration_statement
  : WHILE '(' expression ')' statement {BUILD<NonTerminal>("iteration_statement", $$, $1, $2, $3, $4, $5);}
  | DO statement WHILE '(' expression ')' ';' {BUILD<NonTerminal>("iteration_statement", $$, $1, $2, $3, $4, $5, $6, $7);}
  | FOR '(' expression_statement expression_statement ')' statement {BUILD<NonTerminal>("iteration_statement", $$, $1, $2, $3, $4, $5, $6);}
  | FOR '(' expression_statement expression_statement expression ')' statement {BUILD<NonTerminal>("iteration_statement", $$, $1, $2, $3, $4, $5, $6, $7);}
  | FOR '(' declaration expression_statement ')' statement {BUILD<NonTerminal>("iteration_statement", $$, $1, $2, $3, $4, $5, $6);}
  | FOR '(' declaration expression_statement expression ')' statement {BUILD<NonTerminal>("iteration_statement", $$, $1, $2, $3, $4, $5, $6, $7);}
  ;

jump_statement
  : GOTO IDENTIFIER ';' {BUILD<NonTerminal>("jump_statement", $$, $1, $2);}
  | CONTINUE ';' {BUILD<NonTerminal>("jump_statement", $$, $1, $2);}
  | BREAK ';' {BUILD<NonTerminal>("jump_statement", $$, $1, $2);}
  | RETURN ';' {BUILD<NonTerminal>("jump_statement", $$, $1, $2);}
  | RETURN expression ';' {BUILD<NonTerminal>("jump_statement", $$, $1, $2);}
  ;

translation_unit
  : external_declaration {BUILD<NonTerminal>("translation_unit", $$, $1); juicyc::FrontEnv::root = $$;}
  | translation_unit external_declaration {BUILD<NonTerminal>("translation_unit", $$, $1, $2); juicyc::FrontEnv::root = $$;}
  ;

external_declaration
  : function_definition {BUILD<NonTerminal>("external_declaration", $$, $1);}
  | declaration {BUILD<NonTerminal>("external_declaration", $$, $1);}
  ;

function_definition
  : declaration_specifiers declarator declaration_list compound_statement {BUILD<NonTerminal>("function_definition", $$, $1, $2, $3, $4);}
  | declaration_specifiers declarator compound_statement {BUILD<NonTerminal>("function_definition", $$, $1, $2, $3);}
  ;

declaration_list
  : declaration {BUILD<NonTerminal>("declaration_list", $$, $1);}
  | declaration_list declaration {BUILD<NonTerminal>("declaration_list", $$, $1, $2);}
  ;

%%
extern char yytext[];
extern int column;

void yyerror(const char *s)
{
  fflush(stdout);
  printf("\n%*s\n%*s\n", column, "^", column, s);
}