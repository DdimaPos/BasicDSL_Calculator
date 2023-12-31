#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "include/parser.h"
#include "include/interpreter.h"

SymbolTableEntry *symbolTable = NULL;

void freeSymbolTable(SymbolTableEntry *table){
    while (table != NULL)
    {
        SymbolTableEntry *temp = table;
        table = table->next;
        free(temp->variable);
        free(temp);
    }
    
}
// Check if number is an integer or a double
int isWholeNumber(double value) {
    return value == floor(value);
}
SymbolTableEntry *findOrAddSymbolTableEntry(const char* variable, int addIfNotFound, NodeType type){
    SymbolTableEntry *entry = symbolTable;
    while (entry) {
        if (strcmp(entry->variable, variable) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    if (addIfNotFound) {
        SymbolTableEntry *newEntry = malloc(sizeof(SymbolTableEntry));
        if (!newEntry) {
            fprintf(stderr, "Memory allocation error\n");
            exit(EXIT_FAILURE);
        }
        newEntry->variable = strdup(variable);
        if(type == LITERAL_INT_NODE){
            newEntry->value.i_val = 0;
            newEntry->type = "int";
        }else if(type == LITERAL_DOUBLE_NODE){
            newEntry->value.d_val = 0;
            newEntry->type = "double";
        }
        newEntry->next = symbolTable;
        symbolTable = newEntry;
        return newEntry;
    }
    return NULL;
}

double interpret(Node *ast){
    SymbolTableEntry *entry;
    double leftVal, rightVal;
    if(!ast){
        // printf("NO node\n");
        return 0;
    }
    // printf("%s\n", NodeTypeToString(ast->type));
    switch (ast->type)
    {
    case ROOT_NODE:
        // printf("Enter root\n");
        return interpret(ast->left);
    case DEC_INT_NODE:
    case DEC_DOUBLE_NODE:
        // printf("declared successful\n");
        entry = findOrAddSymbolTableEntry(ast->left->strValue, 0, ast->type);
        if(entry){
            printf("ERROR on line %i, Variable %s already declared\n",ast->line, ast->left->strValue);
            exit(EXIT_FAILURE);
        }else{
                if(ast->type == DEC_DOUBLE_NODE){
                    entry = findOrAddSymbolTableEntry(ast->left->strValue, 1, LITERAL_DOUBLE_NODE);
                }else{
                    entry = findOrAddSymbolTableEntry(ast->left->strValue, 1, LITERAL_INT_NODE);
                }
                interpret(ast->right);
                return 0;
        }
        break;
    case LITERAL_INT_NODE:
        return ast->Value.intValue;
    case LITERAL_DOUBLE_NODE:
        return ast->Value.doubleValue;
    case ASS_NODE:
        // printf("Start assignment\n");
        entry = findOrAddSymbolTableEntry(ast->strValue, 0, ast->type);
        if (!entry) {
            printf("ERROR: undeclared variable %s on line %i\n", ast->strValue, ast->line);
            exit(EXIT_FAILURE);
        }
        double leftVal = interpret(ast->left);
        if (strcmp(entry->type, "int") == 0) {
            if (isWholeNumber(leftVal)) {
                entry->value.i_val = (int)leftVal;
            } else {
                printf("Type mismatch on line %i: Cannot assign a non-integer value to an integer variable\n", ast->line);
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(entry->type, "double") == 0) {
            entry->value.d_val = leftVal;
        } else {
            printf("Unknown variable type on line %i\n", ast->line);
            exit(EXIT_FAILURE);
        }
        interpret(ast->right);
        break;
    case VAR_NODE:
        entry = findOrAddSymbolTableEntry(ast->strValue, 0, ast->type);
        if(entry){
            if(entry->type == "int"){
                return entry->value.i_val;
            }else if(entry->type == "double"){
                return entry->value.d_val;
            }
        }else{
            printf("ERROR: undeclared variable on line %i\n", ast->line);
            exit(EXIT_FAILURE);
        }
        break;
    case PLUS_NODE:
        // printf("ADD\n");
        return interpret(ast->left) + interpret(ast->right);
    case MINUS_NODE:
        // printf("MIN\n");
        return interpret(ast->left) - interpret(ast->right);
    case MULTIPLY_NODE:
        // printf("MULTI\n");
        return interpret(ast->left) * interpret(ast->right);
    case DIV_NODE:
        // printf("DIV\n");
        return interpret(ast->left) / interpret(ast->right);
    case GREATER_NODE:
        // printf("GREATER\n");
        return interpret(ast->left) > interpret(ast->right);
    case LESS_NODE:
        return interpret(ast->left) < interpret(ast->right);
    case EQUAL_NODE:
        return interpret(ast->left) == interpret(ast->right);
    case BLOCK_NODE:
        interpret(ast->left);
        break;
    case IF_ROOT_NODE:
        // printf("IF\n");
        if((int)interpret(ast->left->left)){
            interpret(ast->left->right->left->left);
        }else{
            interpret(ast->left->right->right->left);
        }
        interpret(ast->right);
        break;
    case WHILE_ROOT_NODE:
        // printf("WHILE\n");
        while((int)interpret(ast->left->left)){
            interpret(ast->left->right);
        }
        interpret(ast->right);
        break;
    case PRINT_NODE:
        leftVal= interpret(ast->left);
        if(isWholeNumber(leftVal)){
            printf("%i\n", (int)leftVal);   
        }else{
            printf("%f\n", leftVal);
        }
        interpret(ast->right);
        break;
    default:
        perror("Unknown Node type");
        exit(EXIT_FAILURE);
        break;
    }
}


int interpreter(Node *ast) {
    interpret(ast);
    freeSymbolTable(symbolTable);
    return 0;
}
