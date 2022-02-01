#include "cilisp.h"

#define RED             "\033[31m"
#define RESET_COLOR     "\033[0m"

// yyerror:
// Something went so wrong that the whole program should crash.
// This is basically printf, but red, with "\nERROR: " prepended, "\n" appended,

void yyerror(char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, 255, format, args);

    printf(RED "\nERROR: %s\nExiting...\n" RESET_COLOR, buffer);
    fflush(stdout);

    va_end (args);
    exit(1);
}

// warning:
// Something went mildly wrong (on the user-input level, probably)
void warning(char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, 255, format, args);

    printf(RED "WARNING: %s\n" RESET_COLOR, buffer);
    fflush(stdout);

    va_end (args);
}

FUNC_TYPE resolveFunc(char *funcName)
{
    // Array of string values for function names.
    // Must be in sync with members of the FUNC_TYPE enum in order for resolveFunc to work.
    // For example, funcNames[NEG_FUNC] should be "neg"
    char *funcNames[] = {
            "neg",
            "abs",
            "add",
            "sub",
            "mult",
            "div",
            "remainder",
            "exp",
            "exp2",
            "pow",
            "log",
            "sqrt",
            "cbrt",
            "hypot",
            "max",
            "min",
            "less",
            "greater",
            "equal",
            "print",
            "rand",
            "read"
            ""
    };
    int i = 0;
    while (funcNames[i][0] != '\0')
    {
        if (strcmp(funcNames[i], funcName) == 0)
        {
            return i;
        }
        i++;
    }
    return CUSTOM_FUNC;
}

NUM_TYPE resolveNTYPE(char *numTypeName)
{
    // Array of string values for number types.

    char *funcNames[] = {
            "int",
            "double",
    };
    int i = 0;
    while (funcNames[i][0] != '\0')
    {
        if (strcmp(funcNames[i], numTypeName) == 0)
        {
            return i;
        }
        i++;
    }
}

AST_NODE *createConditionalASTNode(AST_NODE *condition, AST_NODE *ifTrue, AST_NODE *ifFalse)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    node->type = CONDITIONAL_NODE_TYPE;
    node->symbolTable = NULL;
    node->next = NULL;

    node->data.conditional.condition = condition;
    node->data.conditional.condition->parent = node;

    node->data.conditional.ifTrue = ifTrue;
    node->data.conditional.ifTrue->parent = node;

    node->data.conditional.ifFalse = ifFalse;
    node->data.conditional.ifFalse->parent = node;

    return node;
}


AST_NODE *createNumberNode(double value, NUM_TYPE type)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    node->type = NUM_NODE_TYPE;
    node->data.number.value = value;
    node->data.number.type = type;
    node->parent = NULL;
    node->next = NULL;

    return node;
}


AST_NODE *createFunctionNode(FUNC_TYPE func, AST_NODE *opList, char *id)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    node->type = FUNC_NODE_TYPE;
    node->data.function.func = func;
    node->data.function.opList = opList;
    node->next = NULL;
    node->parent = NULL;
    node->argTable = NULL;
    node->symbolTable = NULL;

    while(opList != NULL)
    {
        opList->parent = node;
        opList = opList->next;
    }

    if (id != NULL)
    {
        node->data.function.id = id;
        node->data.function.func = CUSTOM_FUNC;
    }
    return node;
}

AST_NODE *addExpressionToList(AST_NODE *newExpr, AST_NODE *exprList)
{
    newExpr->next = exprList;
    return newExpr; 
}

AST_NODE *createScopeASTNode(SYMBOL_TABLE_NODE *symbolTableNode, AST_NODE *child)
{
    AST_NODE *scopeNode;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((scopeNode = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    scopeNode->parent = NULL;
    scopeNode->next = NULL;
    scopeNode->symbolTable = NULL;
    scopeNode->argTable = NULL;

    scopeNode->type = SCOPE_NODE_TYPE;
    scopeNode->data.scope.child = child;
    scopeNode->data.scope.child->symbolTable = symbolTableNode;

    scopeNode->data.scope.child->parent = scopeNode;

    if (symbolTableNode->symbolType != LAMBDA_TYPE )
    {
        linkStuff(scopeNode->data.scope.child);
    }


    return scopeNode;
}

void linkStuff(AST_NODE *child)
{
    // link symbol table node to AST node
    SYMBOL_TABLE_NODE *tempSymbolTable = child->symbolTable;

    while (tempSymbolTable != NULL)
    {
        if (tempSymbolTable->symbolType == LAMBDA_TYPE)
        {
            return;
        }
        AST_NODE *tempAST = tempSymbolTable->value;
        while (tempAST != NULL)
        {
            tempAST->parent = child;

            switch (tempAST->type)
            {
                case NUM_NODE_TYPE:
                    tempAST = tempAST->next;
                    break;

                case FUNC_NODE_TYPE:
                    tempAST = tempAST->data.function.opList;
                    break;

                case SYM_NODE_TYPE:
                    tempAST = tempAST->next;
                    break;
                case SCOPE_NODE_TYPE:
                    tempAST = tempAST->next;
                    break;
            }
        }
        tempSymbolTable = tempSymbolTable->next;
    }
}


AST_NODE *createSymbolASTNode(char *id)
{
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
    {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    node->type = SYM_NODE_TYPE;
    node->data.symbol.id = id;
    node->next = NULL;
    node->parent = NULL;
    node->argTable = NULL;
    node->symbolTable = NULL;

    return node;
}

SYMBOL_TABLE_NODE *createSymbolTableNode(NUM_TYPE ntype, char *id, AST_NODE *value, SYMBOL_TYPE st)
{
    SYMBOL_TABLE_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(SYMBOL_TABLE_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL) {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    node->id = id;
    node->value = value;
    node->next = NULL;
    node->ntype = ntype;
    node->symbolType = st;
    node->stack = NULL;

    return node;
}

SYMBOL_TABLE_NODE *createLambda(NUM_TYPE ntype, char *id, AST_NODE *value, SYMBOL_TYPE st, SYMBOL_TABLE_NODE *args)
{
    SYMBOL_TABLE_NODE *stnode;
    size_t nodeSize;

    nodeSize = sizeof(SYMBOL_TABLE_NODE);
    if ((stnode = calloc(nodeSize, 1)) == NULL) {
        yyerror("Memory allocation failed!");
        exit(1);
    }

    stnode->id = id;
    stnode->value = value;
    stnode->next = NULL;
    stnode->ntype = ntype;
    stnode->symbolType = st;
    //node->value->argTable = args;
    stnode->value->argTable = args;
    stnode->stack = NULL;

    return stnode;
}
SYMBOL_TABLE_NODE *linkLetElems(SYMBOL_TABLE_NODE *node1, SYMBOL_TABLE_NODE *node2)
{

    SYMBOL_TABLE_NODE *temp = node2;
    while(node2 != NULL)
    {
        if (strcmp(node2->id, node1->id) == 0)
        {
            if ((node2->symbolType == ARG_TYPE) && (node1->symbolType == ARG_TYPE)) {
                warning("Duplicate assignment to symbol %s detected in the same scope!\n"
                        "Only the first assignment is kept!");
                node2->value = node1->value;
            }
        }

        node2 = node2->next;
    }

    if (node2 == NULL)
    {
        node1->next = temp;
    }

    return node1;
}

SYMBOL_TABLE_NODE *linkArgs(char *id, SYMBOL_TABLE_NODE *node)
{
    SYMBOL_TABLE_NODE *arg = createSymbolTableNode(NO_TYPE, id, NULL, ARG_TYPE);
    arg->next = node;
    arg->stack = NULL;
    arg->value = NULL;
    return arg;
}


RET_VAL evalNeg(AST_NODE *oplist)
{
    RET_VAL val;

    if (oplist == NULL)
    {
        warning("WARNING: neg called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    if (oplist->next != NULL)
    {
        warning("WARNING: neg called with 2 operands! nan returned!");
        return NAN_RET_VAL;
    }

    val = eval(oplist);
    val.value = val.value * -1;

    return val;
}

RET_VAL evalAbs(AST_NODE *oplist)
{
    RET_VAL val;

    if (oplist == NULL)
    {
        warning("WARNING: abs called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    if (oplist->next != NULL)
    {
        warning("WARNING: abs called with 2 operands! nan returned!");
        return NAN_RET_VAL;
    }

    val = eval(oplist);
    val.value = fabs(val.value);

    return val;
}

RET_VAL evalAdd(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    if (oplist == NULL)
    {
        warning("WARNING: add called with no operands! 0 returned!");
        return ZERO_RET_VAL;
    }

    if (oplist->next == NULL)
    {
        val = eval(oplist);
        return val;
    }
    else
    {
        val.type = INT_TYPE;
        val.value = 0;
        while(oplist != NULL)
        {

            temp = eval(oplist);
            val.value += temp.value;

            if(temp.type == DOUBLE_TYPE)
            {
                val.type = DOUBLE_TYPE;
            }
            oplist = oplist->next;
        }
    }
    return val;
}

RET_VAL evalSub(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    int arg_count = 2;

    if (oplist == NULL)
    {
        warning("WARNING: sub called with no operands!");
        return NAN_RET_VAL;
    }

    if (oplist->next == NULL)
    {
        warning("WARNING: sub called with only one arg!");
        return NAN_RET_VAL;
    }
    else
    {
        val = eval(oplist);
        oplist = oplist->next;
        while(oplist != NULL)
        {
            if (arg_count > 2)
            {
                warning("WARNING: sub called with extra (ignored) operands!");
                break;
            }

            temp = eval(oplist);
            val.value -= temp.value;

            if(temp.type == DOUBLE_TYPE)
            {
                val.type = DOUBLE_TYPE;
            }
            oplist = oplist->next;
            arg_count +=1;
        }
    }
    return val;
}

RET_VAL evalMul(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    if (oplist == NULL)
    {
        warning("WARNING: mult called with no operands! 1 returned!");
        val.value = 1;
        val.type = INT_TYPE;
        return val;
    }

    if (oplist->next == NULL)
    {
        val = eval(oplist);
        return val;
    }
    else
    {
        val = eval(oplist);
        oplist = oplist->next;
        while(oplist != NULL)
        {
            temp = eval(oplist);
            val.value *= temp.value;

            if(temp.type == DOUBLE_TYPE)
            {
                val.type = DOUBLE_TYPE;
            }
            oplist = oplist->next;

        }
    }
    return val;
}

RET_VAL evalDiv(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    int arg_count = 2;

    if (oplist == NULL)
    {
        warning("WARNING: div called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    if (oplist->next == NULL)
    {
        warning("WARNING: div called with only one arg! nan returned!");
        return NAN_RET_VAL;
    }
    else
    {
        val = eval(oplist);
        oplist = oplist->next;
        while(oplist != NULL)
        {
            if (arg_count > 2)
            {
                warning("WARNING: div called with extra (ignored) operands!");
                break;
            }
            temp = eval(oplist);
            val.value /= temp.value;

            if(temp.type == DOUBLE_TYPE)
            {
                val.type = DOUBLE_TYPE;
            }
            oplist = oplist->next;

            arg_count += 1;
        }
    }
    if (val.type == INT_TYPE)
    {
        val.value = trunc(val.value);
    }

    return val;
}

RET_VAL evalRem(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    int arg_count = 2;

    if (oplist == NULL)
    {
        warning("WARNING: remainder called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    if (oplist->next == NULL)
    {
        warning("WARNING: remainder called with only one arg! nan returned!");
        return NAN_RET_VAL;
    }
    else
    {
        val = eval(oplist);
        oplist = oplist->next;
        while(oplist != NULL)
        {
            if (arg_count > 2)
            {
                warning("WARNING: sub called with extra (ignored) operands!");
                break;
            }

            temp = eval(oplist);
            //val.value = fmod(fmod(val.value, temp.value), temp.value);
            val.value = fmod(val.value, temp.value);

            if (val.value < 0)
            {
                val.value += abs(temp.value);
            }

            if(temp.type == DOUBLE_TYPE)
            {
                val.type = DOUBLE_TYPE;
            }
            oplist = oplist->next;
            arg_count +=1;
        }
    }

    if (val.value < 0)
    {
        val.value = fabs(val.value);
    }

    return val;
}

RET_VAL evalExp(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    int arg_count = 1;

    if (oplist == NULL)
    {
        warning("WARNING: exp called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    else
    {
        val = eval(oplist);
        val.value = exp(val.value);
        val.type = DOUBLE_TYPE;

        arg_count += 1;
        oplist = oplist->next;
        if (oplist != NULL)
        {
            if (arg_count > 1)
            {
                warning("WARNING: sub called with extra (ignored) operands!");
            }

        }
    }

    val.value = fabs(val.value);
    return val;
}

RET_VAL evalExp2(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    int arg_count = 1;

    if (oplist == NULL)
    {
        warning("WARNING: exp2 called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    else
    {
        val = eval(oplist);

        if(val.value < 0 || val.value < 0.0 || val.type == DOUBLE_TYPE)
        {
            val.type = DOUBLE_TYPE;
        }

        val.value = pow(2, val.value);

        arg_count += 1;
        oplist = oplist->next;
        if (oplist != NULL)
        {
            if (arg_count > 1)
            {
                warning("WARNING: sub called with extra (ignored) operands!");
            }

        }
    }
    return val;
}

RET_VAL evalPow(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL val2;

    if (oplist == NULL)
    {
        warning("WARNING: pow called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    if (oplist->next == NULL)
    {
        warning("WARNING:pow called with only one operand! nan returned!");
        return NAN_RET_VAL;
    }

    else
    {
        val = eval(oplist);
        val2 = eval(oplist->next);
        val.value = pow(val.value, val2.value);
        val.type = val.type || val2.type;

        oplist = oplist->next;
        if (oplist->next != NULL)
        {
            warning("WARNING: pow called with extra (ignored) operands!");
        }
    }
    return val;

}

RET_VAL evalLog(AST_NODE *oplist)
{
    RET_VAL val;

    if (oplist == NULL)
    {
        warning("WARNING: log called with no operands! nan returned!");
        return NAN_RET_VAL;
    }
    else
    {
        val = eval(oplist);
        val.value = log(val.value);
        val.type = DOUBLE_TYPE;

        if (oplist->next != NULL)
        {
            warning("WARNING: log called with extra (ignored) operands!");
        }
    }

    return val;

}

RET_VAL evalSqrt(AST_NODE *oplist)
{
    RET_VAL val;

    if (oplist == NULL)
    {
        warning("WARNING: sqrt called with no operands! nan returned!");
        return NAN_RET_VAL;
    }
    else
    {
        val = eval(oplist);
        val.value = sqrt(val.value);
        val.type = DOUBLE_TYPE;

        if (oplist->next != NULL)
        {
            warning("WARNING: sqrt called with extra (ignored) operands!");
        }
    }

    return val;
}

RET_VAL evalCbrt(AST_NODE *oplist)
{
    RET_VAL val;

    if (oplist == NULL)
    {
        warning("WARNING: cbrt called with no operands! nan returned!");
        return NAN_RET_VAL;
    }
    else
    {
        val = eval(oplist);
        val.value = cbrt(val.value);
        val.type = DOUBLE_TYPE;

        if (oplist->next != NULL)
        {
            warning("WARNING: cbrt called with extra (ignored) operands!");
        }
    }

    return val;
}

RET_VAL evalHypot(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    if (oplist == NULL)
    {
        warning("WARNING: hypot called with no operands! 0 returned!");
        val.value = 0;
        val.type = DOUBLE_TYPE;
        return val;
    }

    else
    {
        val.value = 0;
        val.type = DOUBLE_TYPE;
        temp = eval(oplist);
        while(oplist != NULL)
        {
            temp = eval(oplist);
            val.value += temp.value * temp.value;
            oplist = oplist->next;
        }
    }

    val.value = sqrt(val.value);

    return val;
}

RET_VAL evalMax(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    if (oplist == NULL)
    {
        warning("WARNING: max called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    else
    {
        val = eval(oplist);
        while(oplist != NULL)
        {
            temp = eval(oplist);
            if(temp.value > val.value)
            {
                val.value = temp.value;
                val.type = temp.type;
            }
            oplist = oplist->next;
        }
    }

    return val;
}

RET_VAL evalMin(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL temp;

    if (oplist == NULL)
    {
        warning("WARNING: min called with no operands! nan returned!");
        return NAN_RET_VAL;
    }

    else
    {
        val = eval(oplist);
        while(oplist != NULL)
        {
            temp = eval(oplist);
            if(temp.value < val.value)
            {
                val.value = temp.value;
                val.type = temp.type;
            }
            oplist = oplist->next;
        }
    }

    return val;
}

RET_VAL evalLess(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL val2;
    RET_VAL retVal;

    val = eval(oplist);
    val2 = eval(oplist->next);

    retVal.type = INT_TYPE;
    if (val.value < val2.value)
    {
        retVal.value = 1;
    }
    else
    {
        retVal.value = 0;
    }

    return retVal;

}

RET_VAL evalGreater(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL val2;
    RET_VAL retVal;

    val = eval(oplist);
    val2 = eval(oplist->next);

    retVal.type = INT_TYPE;
    if (val.value > val2.value)
    {
        retVal.value = 1;
    }
    else
    {
        retVal.value = 0;
    }



    return retVal;
}

RET_VAL evalEqual(AST_NODE *oplist)
{
    RET_VAL val;
    RET_VAL val2;
    RET_VAL retVal;

    val = eval(oplist);
    val2 = eval(oplist->next);

    retVal.type = INT_TYPE;
    if (val.value == val2.value)
    {
        retVal.value = 1;
    }
    else
    {
        retVal.value = 0;
    }

    return retVal;
}

RET_VAL evalPrint(AST_NODE *oplist)
{
    RET_VAL val;

    if (oplist == NULL)
    {
        warning("print called with no operands!");
        return NAN_RET_VAL;

    }


    if (oplist->next != NULL)
    {
        warning(("print called with extra (ignored) operands!"));
    }

    val = eval(oplist);
    printRetVal(val);
    return val;

}

RET_VAL evalRand()
{
    RET_VAL val;

    double d = (double)rand() / RAND_MAX;

    val.type = DOUBLE_TYPE;
    val.value = d;

    return val;
}

int match(const char *string, const char *pattern)
{
    /*
     * check if string matches regex pattern
     *
     */

    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) return 0;
    int status = regexec(&re, string, 0, NULL, 0);
    regfree(&re);
    if (status != 0) return 0;
    return 1;
}

RET_VAL evalRead()
{
    RET_VAL val;

    const char* regexIntPattern = "^\[+-]?[0-9]+$";
    const char* regexFloatPattern = "[+-]?[0-9]+\\.[0-9]*";
    char* input[20];

    // read input from user
    printf("read :: ");
    fscanf(read_target,"%s", input);

    // test if input matches int or double regex pattern
    int testInt = match(input, regexIntPattern);

    // input matches int regex pattern. convert string to INT
    if (testInt == 1)
    {
        val.type = INT_TYPE;
        val.value = atoi(input);
        return val;
    }

    // input matches double regex pattern. convert string to DOUBLE
    int testDouble = match(input, regexFloatPattern);
    if (testDouble == 1)
    {
        val.type = DOUBLE_TYPE;
        val.value = atof(input);
        return val;
    }

    // input is invalid.
    if (testInt == 0 & testDouble == 0)
    {
        warning("Invalid read entry! NAN returned!");
        return NAN_RET_VAL;
    }

}

RET_VAL typeCast(NUM_TYPE numtype, RET_VAL val)
{
    RET_VAL retVal;
    int temp_int;
    double temp_double;
    switch (numtype)
    {
        case INT_TYPE:
            switch (val.type)
            {
                case DOUBLE_TYPE:
                    retVal.type = INT_TYPE;
                    retVal.value = (int) val.value;
                    temp_int = retVal.value;
                    warning("Precision lost on int cast from %lf to %d", val.value, temp_int);
                    break;
                case INT_TYPE:
                    return val;
                    break;
                case NO_TYPE:
                    return val;
                    break;
            }
            break;
        case DOUBLE_TYPE:
            switch (val.type)
            {
                case DOUBLE_TYPE:
                    return val;
                    break;
                case INT_TYPE:
                    retVal.type = DOUBLE_TYPE;
                    retVal.value = (double) val.value;
                    break;
                case NO_TYPE:
                    return val;
                    break;
            }
            break;
        case NO_TYPE:
            return val;
            break;
    }

    return retVal;

}
RET_VAL lookForSymbol(AST_NODE *symbolASTNode, char *id)
{
    SYMBOL_TABLE_NODE *temp;
    temp = NULL;

    RET_VAL val;
    int temp_int;
    double temp_double;

    //look for symbol
    while (symbolASTNode != NULL)
    {
        if (symbolASTNode->symbolTable != NULL)
        {
            temp = symbolASTNode->symbolTable;
        }

        if (symbolASTNode->argTable != NULL)
        {
            temp = symbolASTNode->argTable;
        }


        while (temp!= NULL)
        {
            // symbol is found
            if (strcmp(id, temp->id) == 0)
            {
                if (temp->symbolType == ARG_TYPE)
                {
                    val = temp->stack->value;
                    //val = typeCast(temp->ntype, val);

                    return val;
                }


                val = eval(temp->value);

                // if symbol if func, replace oplist with one node
                if (temp->value->type != NUM_NODE_TYPE)
                {
                    AST_NODE *node;
                    node = createNumberNode(val.value, val.type);
                    free(temp->value);

                    temp->value = node;
                    temp->value->parent = symbolASTNode;
                }

                //TASK 3 typecasting

                val = typeCast(temp->ntype, val);
                return val;
            }

            else
            {
                temp = temp->next;
            }
         }

        symbolASTNode = symbolASTNode->parent;
    }

    //didn't find variable. print warning and return nan
    warning("Variable %s undefined. NAN returned!", id);
    return NAN_RET_VAL;
}

SYMBOL_TABLE_NODE lookForCustomFunc(AST_NODE *node, char *id)
{
    SYMBOL_TABLE_NODE  *temp =  NULL;

    while (node != NULL)
    {
        temp = node->symbolTable;

        while (temp != NULL)
        {
            if (strcmp(id, temp->id) == 0 && temp->symbolType == LAMBDA_TYPE)
            {
                return *temp;
            }

            else
            {
                temp = temp->next;
            }
        }

        node = node->parent;
    }
}

void putOperandsOnStacks(SYMBOL_TABLE_NODE *node, STACK_NODE *linkedList)
{
    RET_VAL temp;

    while(node!= NULL)
    {
        /*
         * if there is an element on the stack, put new element on top and append old element
         */
        if (node->stack != NULL)
        {
            temp = node->stack->value;
            node->stack = &linkedList;
            node->stack->next = &temp;
        }

        // detach stuff
        node->stack = &linkedList->value;
        linkedList = linkedList->next;
        node->stack->next = NULL;

        node = node->next;
    }


}

RET_VAL evalCustomFunc(AST_NODE *funcNode)
{
    RET_VAL val;


    // look up function
    SYMBOL_TABLE_NODE customFuncNode = lookForCustomFunc(funcNode, funcNode->data.function.id);


    // eval all operands
    AST_NODE *oplist = funcNode->data.function.opList;

    STACK_NODE linkedList;
    linkedList.next = NULL;
    linkedList.value = eval(oplist);
    oplist = oplist->next;

    while (oplist != NULL)
    {
        STACK_NODE newElement;
        newElement.value = eval(oplist);
        linkedList.next = &newElement;
        oplist = oplist->next;
    }

    // put all operands on stacks
    putOperandsOnStacks(customFuncNode.value->argTable, &linkedList);

    // evaluate
    val = eval(customFuncNode.value);
    val = typeCast(customFuncNode.ntype, val);

    return val;
}


RET_VAL evalFuncNode(AST_NODE *node)
{
    if (!node)
    {
        yyerror("NULL ast node passed into evalFuncNode!");
        return NAN_RET_VAL; // unreachable but kills a clang-tidy warning
    }

    RET_VAL val;

    switch(node->data.function.func)
    {
        case NEG_FUNC:
            val = evalNeg(node->data.function.opList);
            break;

        case ABS_FUNC:
            val = evalAbs(node->data.function.opList);
            break;

        case ADD_FUNC:
            val = evalAdd(node->data.function.opList);
            break;

        case SUB_FUNC:
            val = evalSub(node->data.function.opList);
            break;

        case MULT_FUNC:
            val = evalMul(node->data.function.opList);
            break;

        case DIV_FUNC:
            val = evalDiv(node->data.function.opList);
            break;

        case REM_FUNC:
            val = evalRem(node->data.function.opList);
            break;

        case EXP_FUNC:
            val = evalExp(node->data.function.opList);
            break;

        case EXP2_FUNC:
            val = evalExp2(node->data.function.opList);
            break;

        case POW_FUNC:
            val = evalPow(node->data.function.opList);
            break;

        case LOG_FUNC:
            val = evalLog(node->data.function.opList);
            break;

        case SQRT_FUNC:
            val = evalSqrt(node->data.function.opList);
            break;

        case CBRT_FUNC:
            val = evalCbrt(node->data.function.opList);
            break;

        case HYPOT_FUNC:
            val = evalHypot(node->data.function.opList);
            break;

        case MAX_FUNC:
            val = evalMax(node->data.function.opList);
            break;

        case MIN_FUNC:
            val = evalMin(node->data.function.opList);
            break;

        case LESS_FUNC:
            val = evalLess(node->data.function.opList);
            break;

        case GREATER_FUNC:
            val = evalGreater(node->data.function.opList);
            break;

        case EQUAL_FUNC:
            val = evalEqual(node->data.function.opList);
            break;

        case PRINT_FUNC:
            val = evalPrint(node->data.function.opList);
            break;

        case RAND_FUNC:
            val = evalRand();
            break;

        case READ_FUNC:
            val = evalRead();
            break;

        case CUSTOM_FUNC:
            val = evalCustomFunc(node);
            break;
    }

    return val;
}

RET_VAL evalNumNode(AST_NODE *node)
{
    if (!node)
    {
        yyerror("NULL ast node passed into evalNumNode!");
        return NAN_RET_VAL;
    }

    RET_VAL val;

    val.type = node->data.number.type;
    val.value = node->data.number.value;

    return val;
}



RET_VAL evalConditionalNode(AST_NODE *node)
{
    RET_VAL val;
    RET_VAL retVal;

    val = eval(node->data.conditional.condition);

    if (val.value == 1)
    {
        retVal = eval(node->data.conditional.ifTrue);
    }
    else
    {
        retVal = eval(node->data.conditional.ifFalse);
    }

    return retVal;

}

RET_VAL eval(AST_NODE *node)
{
    if (!node)
    {
        yyerror("NULL ast node passed into eval!");
        return NAN_RET_VAL;
    }

    RET_VAL val;

    switch(node->type)
    {
        case NUM_NODE_TYPE:
            val = evalNumNode(node);
            break;

        case FUNC_NODE_TYPE:
            val = evalFuncNode(node);
            break;

        case SCOPE_NODE_TYPE:
            val = eval(node->data.scope.child);
            break;

        case SYM_NODE_TYPE:
            val = lookForSymbol(node, node->data.symbol.id);
            break;

        case CONDITIONAL_NODE_TYPE:
            val = evalConditionalNode(node);
            break;

        default:
            yyerror("node has wrong type");
    }

    return val;
}

// prints the type and value of a RET_VAL
void printRetVal(RET_VAL val)
{
    switch (val.type)
    {
        case INT_TYPE:
            printf("Integer : %.lf\n", val.value);
            break;
        case DOUBLE_TYPE:
            printf("Double : %lf\n", val.value);
            break;
        default:
            printf("No Type : %lf\n", val.value);
            break;
    }
}

void freeStack(STACK_NODE *node)
{
    if (!node)
    {
        return;
    }

    while(node)
    {
        free(node);
        node = node->next;

    }

}

void freeSymbolTableNode(SYMBOL_TABLE_NODE *node)
{
    if (!node)
    {
        return;
    }

    free(node->id);
    freeNode(node->value);
    freeStack(node->stack);
    freeSymbolTableNode(node->next);

    free(node);
    node = NULL;

}

void freeNode(AST_NODE *node)
{
    if (!node)
    {
        return;
    }

    // look through the AST_NODE struct, decide what
    // referenced data should have freeNode called on it

    switch(node->type)
    {
        case NUM_NODE_TYPE:
            break;

        case FUNC_NODE_TYPE:
            freeNode(node->data.function.opList);
            free(node->data.function.id);
            break;

        case SYM_NODE_TYPE:
            free(node->data.symbol.id);
            break;

        case SCOPE_NODE_TYPE:
            freeNode(node->data.scope.child);
            break;

        case CONDITIONAL_NODE_TYPE:
            freeNode(node->data.conditional.condition);
            freeNode(node->data.conditional.ifFalse);
            freeNode(node->data.conditional.ifTrue);
            break;
    }

    freeNode(node->next);

    free(node);
    node = NULL;
}