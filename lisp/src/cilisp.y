%{
    #include "cilisp.h"
    #define ylog(r, p) {fprintf(flex_bison_log_file, "BISON: %s ::= %s \n", #r, #p); fflush(flex_bison_log_file);}
    int yylex();
    void yyerror(char*, ...);
%}

%union {
    double dval;
    int ival;
    struct ast_node *astNode;
    char *sval;
};

%token <ival> FUNC TYPE
%token <dval> INT FLOAT
%token <sval> SYMBOL
%token QUIT EOL EOFT LPAREN RPAREN LET COND LAMBDA

%type <astNode> number s_expr_list s_expr_section f_expr s_expr let_section let_list let_elem arg_list

%%

program:
    s_expr EOL {
        ylog(program, s_expr EOL);
        if ($1) {
            printRetVal(eval($1));
            freeNode($1);
        }
        YYACCEPT;
    }
    | s_expr EOFT {
        ylog(program, s_expr EOFT);
        if ($1) {
            printRetVal(eval($1));
            freeNode($1);
        }
        exit(EXIT_SUCCESS);
    }
    | EOL {
        ylog(program, EOL);
        YYACCEPT;  // paranoic; main skips blank lines
    }
    | EOFT {
        ylog(program, EOFT);
        exit(EXIT_SUCCESS);
    };


s_expr:
    f_expr {
        ylog(s_expr, f_expr);
        $$ = $1;
    }
    | SYMBOL {
        ylog(s_expr, SYMBOL);
        $$ = createSymbolASTNode($1);
    }
    | LPAREN let_section s_expr RPAREN {
        ylog(s_expr, LPAREN let_section s_expr RPAREN);
        $$ = createScopeASTNode($2, $3);
    }
    | LPAREN COND s_expr s_expr s_expr RPAREN {
        ylog(s_expr, LPAREN COND s_expr s_expr s_expr RPAREN);
        $$ = createConditionalASTNode($3, $4, $5);
    }
    | number {
         ylog(s_expr, number);
         $$ = $1;
    }
    | QUIT {
        ylog(s_expr, QUIT);
        exit(EXIT_SUCCESS);
    }
    | error {
        ylog(s_expr, error);
        yyerror("unexpected token");
        $$ = NULL;
    };

let_section:
    LPAREN LET let_list RPAREN {
        ylog(let_section, let_list);
        $$ = $3;
    };

let_list:
    let_elem {
        ylog(let_section, let_list);
        $$ = $1;
    }
    | let_elem let_list {
      ylog(let_list, let_list);
      $$ = linkLetElems($1, $2);

    };

let_elem:
    LPAREN SYMBOL s_expr RPAREN {
        ylog(let_elem, LPAREN SYMBOL s_expr RPAREN);
        $$ = createSymbolTableNode(NO_TYPE, $2, $3, VAR_TYPE);
    }
    | LPAREN TYPE SYMBOL s_expr RPAREN {
      ylog(let_elem, LPAREN TYPE SYMBOL s_expr RPAREN);
      $$ = createSymbolTableNode($2, $3, $4, VAR_TYPE);

    }
    | LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN {
        ylog(let_elem, LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN);
        $$ = createLambda(NO_TYPE, $2, $7, LAMBDA_TYPE, $5);
    }

    | LPAREN TYPE SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN {
        ylog(let_elem, LPAREN TYPE SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN);
        $$ = createLambda($2, $3, $8, LAMBDA_TYPE, $6);
    };

f_expr:
    LPAREN FUNC s_expr_section RPAREN {
        ylog(f_expr, LPAREN FUNC s_expr_section RPAREN);
        $$ = createFunctionNode($2, $3, NULL);
    }
    | LPAREN SYMBOL s_expr_section RPAREN {
        ylog(f_expr, LPAREN SYMBOL s_expr_section RPAREN);
        $$ = createFunctionNode($2, $3, $2);
    };

arg_list:
    SYMBOL {
        ylog(arg_list, SYMBOL);
        $$ = createSymbolTableNode(NO_TYPE, $1, NULL, ARG_TYPE);
    }
    | SYMBOL arg_list {
        ylog(arg_list, SYMBOL arg_list);
        $$ = linkArgs($1, $2);
    }
    | {
        $$ = NULL;
    };

s_expr_section:
    s_expr_list {
        ylog(s_expr_section, s_expr_list);
        $$ = $1;
    }
    | {
        $$ = NULL;
    };

s_expr_list:
    s_expr {
        ylog(s_expr_list, s_expr);
        $$ = $1;
    }
    | s_expr s_expr_list {
        ylog(s_expr_list, s_expr s_expr_list);
        $$ = addExpressionToList($1, $2);
    };

number:
    INT {
        ylog(number, INT);
        $$ = createNumberNode($1, INT_TYPE);
    }
    | FLOAT {
        ylog(number, FLOAT);
        $$ = createNumberNode($1, DOUBLE_TYPE);
    };

%%