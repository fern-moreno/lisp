#ifndef __cilisp_h_
#define __cilisp_h_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "parser.h"
#include <regex.h>


#define NAN_RET_VAL (RET_VAL){DOUBLE_TYPE, NAN}
#define ZERO_RET_VAL (RET_VAL){INT_TYPE, 0}


#define BISON_FLEX_LOG_PATH "../src/bison-flex-output/bison_flex_log"
FILE* read_target;
FILE* flex_bison_log_file;


int yyparse(void);
int yylex(void);
void yyerror(char *, ...);
void warning(char*, ...);


typedef enum func_type {
    NEG_FUNC,
    ABS_FUNC,
    ADD_FUNC,
    SUB_FUNC,
    MULT_FUNC,
    DIV_FUNC,
    REM_FUNC,
    EXP_FUNC,
    EXP2_FUNC,
    POW_FUNC,
    LOG_FUNC,
    SQRT_FUNC,
    CBRT_FUNC,
    HYPOT_FUNC,
    MAX_FUNC,
    MIN_FUNC,
    LESS_FUNC,
    GREATER_FUNC,
    EQUAL_FUNC,
    PRINT_FUNC,
    RAND_FUNC,
    READ_FUNC,
    // TODO complete the enum
    CUSTOM_FUNC
} FUNC_TYPE;


FUNC_TYPE resolveFunc(char *);


typedef enum num_type {
    INT_TYPE,
    DOUBLE_TYPE,
    NO_TYPE
} NUM_TYPE;

NUM_TYPE resolveNTYPE(char *numTypeName);

typedef enum  {
    VAR_TYPE,
    LAMBDA_TYPE,
    ARG_TYPE
} SYMBOL_TYPE;

typedef struct {
    NUM_TYPE type;
    double value;
} AST_NUMBER;

typedef AST_NUMBER RET_VAL;


typedef struct ast_function {
    FUNC_TYPE func;
    struct ast_node *opList;
    char *id;
} AST_FUNCTION;

typedef struct ast_conditional{
    struct ast_node *condition;
    struct ast_node *ifTrue;
    struct ast_node *ifFalse;
} AST_CONDITIONAL;

typedef struct {
    char* id;
} AST_SYMBOL;


typedef struct {
    struct ast_node *child;
} AST_SCOPE;

typedef enum ast_node_type {
    NUM_NODE_TYPE,
    FUNC_NODE_TYPE,
    SYM_NODE_TYPE,
    SCOPE_NODE_TYPE,
    CONDITIONAL_NODE_TYPE
} AST_NODE_TYPE;


typedef struct ast_node {
    AST_NODE_TYPE type;
    struct ast_node *parent;
    struct symbol_table_node *symbolTable;
    struct symbol_table_node *argTable;
    union {
        AST_NUMBER number;
        AST_FUNCTION function;
        AST_SYMBOL symbol;
        AST_SCOPE scope;
        AST_CONDITIONAL conditional;
    } data;
    struct ast_node *next;
} AST_NODE;

typedef struct symbol_table_node {
    char *id;
    AST_NODE *value;
    SYMBOL_TYPE symbolType;
    NUM_TYPE ntype;
    struct stack_node *stack;
    struct symbol_table_node *next;
} SYMBOL_TABLE_NODE;

typedef struct stack_node {
    RET_VAL value;
    struct stack_node *next;
} STACK_NODE;


AST_NODE *createConditionalASTNode(AST_NODE *condition, AST_NODE *ifTrue, AST_NODE *ifFalse);
AST_NODE *createNumberNode(double value, NUM_TYPE type);
AST_NODE *createFunctionNode(FUNC_TYPE func, AST_NODE *opList, char *id);
AST_NODE *addExpressionToList(AST_NODE *newExpr, AST_NODE *exprList);
AST_NODE *createScopeASTNode(SYMBOL_TABLE_NODE *stn, AST_NODE *child);
AST_NODE *createSymbolASTNode(char *id);

SYMBOL_TABLE_NODE *linkLetElems(SYMBOL_TABLE_NODE *node1, SYMBOL_TABLE_NODE *node2);
SYMBOL_TABLE_NODE *linkArgs(char *id, SYMBOL_TABLE_NODE *node2);
SYMBOL_TABLE_NODE *createSymbolTableNode(NUM_TYPE ntype, char *id, AST_NODE *value, SYMBOL_TYPE st);

SYMBOL_TABLE_NODE *createLambda(NUM_TYPE ntype, char *id, AST_NODE *value, SYMBOL_TYPE st, SYMBOL_TABLE_NODE *args);


RET_VAL eval(AST_NODE *node);

void printRetVal(RET_VAL val);

void linkStuff(AST_NODE *scope);

void freeNode(AST_NODE *node);
void freeSymbolTableNode(SYMBOL_TABLE_NODE *node);

#endif