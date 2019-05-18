%union
{
	NBlock* block;
	NExpression* expr;
	NStatement* stmt;
	NIdentifier* ident;
	NVariableDeclaration* var_decl;
	NArrayIndex* index;
	std::vector<shared_ptr<NVariableDeclaration>>* varvec;
	std::vector<shared_ptr<NExpression>>* exprvec;
	std::string* string;
	int token;
}

typedef struct abstract_syntax_tree {
	enum code op;
	int val;
	struct symbol *sym;
	struct abstract_syntax_tree *left, *right;
	char *str;
} AST;
%union {
	AST *val;
	int type;
}

%union { char * intval;
        char  charval;
        struct symtab *symp;
        //struct exval *test;
        }

%union {
	char* lexeme;
	int integer;
	int boolean;
	char character;
	int type;
	struct AstNode *astnode;
};

struct gramTree {
    string content;
    //string type;
    string name;
    int line;       //所在代码行数
    struct gramTree *left;
    struct gramTree *right;
   /* double double_value;
    int int_value;
    string string_value;*/
};
%union{
	struct gramTree* gt;
}