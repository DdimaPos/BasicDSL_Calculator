#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include "include/cJSON.h" 
#include "include/cJSON.c" 
#include "include/lexer.h"
#include "include/parser.h"

void syntax_error(const char* expected, const Token found) {
    printf("Syntax error: Expected %s, ", expected);
    if (found.type == TOKEN_INT_LITERAL) {
        printf("found '%i' at line %d\n", found.value.i_val, found.line);
    } else if (found.type == TOKEN_DOUBLE_LITERAL) {
        printf("found '%f' at line %d\n", found.value.d_val, found.line);
    }
    else printf("found '%s' at line %d\n", found.lexeme, found.line);
    exit(1);
}
TokenType mapJsonToEnum(const char* jsonType){
    if (strcmp(jsonType, "TOKEN_INT_DECL") == 0) return TOKEN_INT_DECL;
    if (strcmp(jsonType, "TOKEN_DOUBLE_DECL") == 0) return TOKEN_DOUBLE_DECL;
    if (strcmp(jsonType, "TOKEN_INT_LITERAL") == 0) return TOKEN_INT_LITERAL;
    if (strcmp(jsonType, "TOKEN_DOUBLE_LITERAL") == 0) return TOKEN_DOUBLE_LITERAL;
    if (strcmp(jsonType, "TOKEN_IDENTIFIER") == 0) return TOKEN_IDENTIFIER;
    if (strcmp(jsonType, "TOKEN_PLUS") == 0) return TOKEN_PLUS;
    if (strcmp(jsonType, "TOKEN_MINUS") == 0) return TOKEN_MINUS;
    if (strcmp(jsonType, "TOKEN_MULTI") == 0) return TOKEN_MULTI;
    if (strcmp(jsonType, "TOKEN_DIVISION") == 0) return TOKEN_DIVISION;
    if (strcmp(jsonType, "TOKEN_DIV") == 0) return TOKEN_DIV;
    if (strcmp(jsonType, "TOKEN_MOD") == 0) return TOKEN_MOD;
    if (strcmp(jsonType, "TOKEN_ASSIGN") == 0) return TOKEN_ASSIGN;
    if (strcmp(jsonType, "TOKEN_PLUS_ASSIGN") == 0) return TOKEN_PLUS_ASSIGN;
    if (strcmp(jsonType, "TOKEN_MINUS_ASSIGN") == 0) return TOKEN_MINUS_ASSIGN;
    if (strcmp(jsonType, "TOKEN_MULTI_ASSIGN") == 0) return TOKEN_MULTI_ASSIGN;
    if (strcmp(jsonType, "TOKEN_DIVISION_ASSIGN") == 0) return TOKEN_DIVISION_ASSIGN;
    if (strcmp(jsonType, "TOKEN_DIV_ASSIGN") == 0) return TOKEN_DIV_ASSIGN;
    if (strcmp(jsonType, "TOKEN_LESS") == 0) return TOKEN_LESS;
    if (strcmp(jsonType, "TOKEN_GREATER") == 0) return TOKEN_GREATER;
    if (strcmp(jsonType, "TOKEN_EQUAL") == 0) return TOKEN_EQUAL;
    if (strcmp(jsonType, "TOKEN_OPEN_PAREN") == 0) return TOKEN_OPEN_PAREN;
    if (strcmp(jsonType, "TOKEN_CLOSE_PAREN") == 0) return TOKEN_CLOSE_PAREN;
    if (strcmp(jsonType, "TOKEN_OPEN_BRACE") == 0) return TOKEN_OPEN_BRACE;
    if (strcmp(jsonType, "TOKEN_CLOSE_BRACE") == 0) return TOKEN_CLOSE_BRACE;
    if (strcmp(jsonType, "TOKEN_PRINT") == 0) return TOKEN_PRINT;
    if (strcmp(jsonType, "TOKEN_INPUT") == 0) return TOKEN_INPUT;
    if (strcmp(jsonType, "TOKEN_CONDITION") == 0) return TOKEN_CONDITION;
    if (strcmp(jsonType, "TOKEN_THEN") == 0) return TOKEN_THEN;
    if (strcmp(jsonType, "TOKEN_ELSE") == 0) return TOKEN_ELSE;
    if (strcmp(jsonType, "TOKEN_EOF") == 0) return TOKEN_EOF;
    if (strcmp(jsonType, "TOKEN_ERROR") == 0) return TOKEN_ERROR;
    if (strcmp(jsonType, "TOKEN_NEW_LINE") == 0) return TOKEN_NEW_LINE;
    return TOKEN_ERROR;
}

Node* parseDeclaration(cJSON* tokens, int *currentTokenIndex);//ready
Node* parseAssignment(cJSON* tokens, int *currentTokenIndex);//in process...
Node* parseExpression(cJSON* tokens, int *currentTokenIndex);
Node* parseTerm(cJSON* tokens, int *currentTokenIndex);
Node* parseFactor();
Node* parsePrimaryExpression(cJSON* tokens, int *currentTokenIndex);
Node* parseBinaryOperator(int precedence, cJSON* tokens, int *currentTokenIndex);
Node* parseLoop();
Node* parseCondition();
Node* parsePrint();
Token getCurrentToken(cJSON* tokens, int *currentTokenIndex){
    cJSON* token = cJSON_GetArrayItem(tokens, *currentTokenIndex);
    Token currentToken;
    const char* jsonType = cJSON_GetObjectItem(token, "type")->valuestring;
    currentToken.type = mapJsonToEnum(jsonType);
    currentToken.line = cJSON_GetObjectItem(token, "line")->valueint;
    if (cJSON_HasObjectItem(token, "value")) {
        cJSON* value = cJSON_GetObjectItem(token, "value");
        if (currentToken.type == TOKEN_INT_LITERAL) {
            currentToken.value.i_val = value->valueint;
        } else if (currentToken.type == TOKEN_DOUBLE_LITERAL) {
            currentToken.value.d_val = value->valuedouble;
        }
    } else if (cJSON_HasObjectItem(token, "lexeme")) {
        currentToken.lexeme = strdup(cJSON_GetObjectItem(token, "lexeme")->valuestring);
    }
    return currentToken;
}
Token getNextToken(cJSON* tokens,int *currentTokenIndex) {
    if (*currentTokenIndex < cJSON_GetArraySize(tokens)) {
        cJSON* token = cJSON_GetArrayItem(tokens, *currentTokenIndex);
        Token nextToken;
        const char* jsonType = cJSON_GetObjectItem(token, "type")->valuestring;
        nextToken.type = mapJsonToEnum(jsonType);
        nextToken.line = cJSON_GetObjectItem(token, "line")->valueint;

        if (cJSON_HasObjectItem(token, "value")) {
            cJSON* value = cJSON_GetObjectItem(token, "value");
            if (nextToken.type == TOKEN_INT_LITERAL) {
                nextToken.value.i_val = value->valueint;
            } else if (nextToken.type == TOKEN_DOUBLE_LITERAL) {
                nextToken.value.d_val = value->valuedouble;
            }
        } else if (cJSON_HasObjectItem(token, "lexeme")) {
            nextToken.lexeme = strdup(cJSON_GetObjectItem(token, "lexeme")->valuestring);
        }

        *currentTokenIndex=*currentTokenIndex+1;
        return nextToken;
    } else {
        // Return a special token to indicate the end of input
        Token endToken;
        //endToken.type = END_OF_INPUT;
        return endToken;
    }
}
int isOperator(TokenType type) {
    return (type == TOKEN_PLUS || type == TOKEN_MINUS || type == TOKEN_MULTI || type == TOKEN_DIVISION || type == TOKEN_DIV ||type == TOKEN_MOD);
}
//parsing funtions

int getOperatorPrecedence(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 1;
        case TOKEN_MULTI:
        case TOKEN_DIVISION:
            return 2;
        // Add more cases for other operators with appropriate precedence
        default:
            return 0; // Default precedence for non-operators
    }
}
Node* parsePrimaryExpression(cJSON* tokens, int *currentTokenIndex) {
    Token token = getCurrentToken(tokens, currentTokenIndex);
    if (token.type == TOKEN_OPEN_PAREN) {
        // Consume the left parenthesis
        getNextToken(tokens, currentTokenIndex);
        struct Node* subExpression = parseExpression(tokens, currentTokenIndex);

        // Check for the closing parenthesis
        token = getNextToken(tokens, currentTokenIndex);
        if (token.type != TOKEN_CLOSE_PAREN) {
            syntax_error("')'", token);
        }

        return subExpression;
    }else if(token.type==TOKEN_INT_LITERAL){
        Node* factorNode = (struct Node*)malloc(sizeof(struct Node));
        factorNode->type = LITERAL_NODE;
        factorNode->Value.intValue=token.value.i_val;
        getNextToken(tokens, currentTokenIndex);
        return factorNode;
    }else if (token.type == TOKEN_DOUBLE_LITERAL){
        Node* factorNode = (struct Node*)malloc(sizeof(struct Node));
        factorNode->type = LITERAL_NODE;
        factorNode->Value.doubleValue=token.value.d_val;
        getNextToken(tokens, currentTokenIndex);
        return factorNode;
    }else if (token.type==TOKEN_IDENTIFIER){
        Node* factorNode = (struct Node*)malloc(sizeof(struct Node));
        factorNode->type = VAR_NODE;
        factorNode->strValue = strdup(token.lexeme);
        getNextToken(tokens, currentTokenIndex);
        return factorNode;
    }
}
Node* parseBinaryOperator(int minPrecedence, cJSON* tokens, int *currentTokenIndex) {
    struct Node* left = parsePrimaryExpression(tokens, currentTokenIndex);
    while (getCurrentToken(tokens, currentTokenIndex).type!=TOKEN_NEW_LINE && getCurrentToken(tokens, currentTokenIndex).type!=TOKEN_CLOSE_PAREN) {
        Token token = getCurrentToken(tokens, currentTokenIndex);
        int precedence = getOperatorPrecedence(token.type);

        if (precedence < minPrecedence) {
            return left; // No operator with higher precedence
        }

        getNextToken(tokens, currentTokenIndex); // Consume the operator
        struct Node* right = parseBinaryOperator(precedence + 1, tokens, currentTokenIndex);

        // Create an operator node with left and right operands
        struct Node* operatorNode = (struct Node*)malloc(sizeof(struct Node));
        switch (token.type){
        case TOKEN_PLUS:operatorNode->type=PLUS_NODE; break;
        case TOKEN_MINUS:operatorNode->type=MINUS_NODE; break;
        case TOKEN_DIVISION:operatorNode->type=DIV_NODE; break;
        case TOKEN_MULTI:operatorNode->type=MULTIPLY_NODE; break;
        default:
            break;
        }
        operatorNode->left = left;
        operatorNode->right = right;

        left = operatorNode;
    }

    return left;
}
Node* parseExpression(cJSON* tokens, int *currentTokenIndex) {
    return parseBinaryOperator(0, tokens, currentTokenIndex); // Start with lowest precedence
}

Node* parseDeclaration(cJSON* tokens, int *currentTokenIndex) {
     Node* declarationNode = ( Node*)malloc(sizeof( Node));
    declarationNode->type = DEC_NODE;

    Token token = getNextToken(tokens, currentTokenIndex);
    if (token.type == TOKEN_INT_DECL || token.type == TOKEN_DOUBLE_DECL) {
        declarationNode->strValue = strdup(token.lexeme);
    } else {
        syntax_error("data type", token);
    }
    token = getNextToken(tokens, currentTokenIndex);
    if (token.type == TOKEN_IDENTIFIER) {
        declarationNode->left = ( Node*)malloc(sizeof( Node));
        declarationNode->left->type = TOKEN_IDENTIFIER;
        declarationNode->left->strValue = strdup(token.lexeme);
    } else {
        syntax_error("identifier", token);
    }
    // Expect an assignment or end of line declaration
    token = getNextToken(tokens, currentTokenIndex);
    switch (token.type){
    case TOKEN_ASSIGN:
        *currentTokenIndex=*currentTokenIndex-2;
        parseAssignment(tokens, currentTokenIndex);break; //need to add something to connect declaration identifier with assignment identifier
    case TOKEN_NEW_LINE:break;
    default:syntax_error("new line", token);break;
    }
    return declarationNode;
}
Node* parseAssignment(cJSON* tokens, int *currentTokenIndex) {
    Token token = getNextToken(tokens, currentTokenIndex);
     Node* assignmentNode = ( Node*)malloc(sizeof( Node));
    assignmentNode->type = ASS_NODE;

    if (token.type == TOKEN_IDENTIFIER) {
        assignmentNode->strValue = strdup(token.lexeme);
    } else {
        syntax_error("identifier", token);
    }

    token = getNextToken(tokens, currentTokenIndex);
    if (token.type != TOKEN_ASSIGN) {
        syntax_error("'='", token);
    }

    // Parse the right-hand side (expression or value)
    assignmentNode->left = parseExpression(tokens, currentTokenIndex); 
    // Expect nline
    token = getNextToken(tokens, currentTokenIndex);
    if (token.type != TOKEN_NEW_LINE) {
        syntax_error("new line", token);
    }

    return assignmentNode;
}


int main() {
    cJSON* root;
    cJSON* tokens;
    int currentTokenIndex=0;
    FILE* file = fopen("./tokens/tokens.json", "r");
    if (file == NULL) {
        printf("Error opening input file.\n");
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* jsonBuffer = (char*)malloc(fileSize + 1);
    fread(jsonBuffer, 1, fileSize, file);
    fclose(file);
    jsonBuffer[fileSize] = '\0';
    
    root = cJSON_Parse(jsonBuffer);
    if (!root) {
        fprintf(stderr, "JSON is fucked\n");
        return 1;
    }
    int i=0;
    tokens = cJSON_GetObjectItem(root, "tokens");
    while (currentTokenIndex < cJSON_GetArraySize(tokens))
    {
        switch(getCurrentToken(tokens, &currentTokenIndex).type){
            case TOKEN_INT_DECL: 
            case TOKEN_DOUBLE_DECL:
                parseDeclaration(tokens, &currentTokenIndex);
                break;
        
            case TOKEN_ERROR:
                printf("Invalid syntax: line %i", getCurrentToken(tokens, &currentTokenIndex).line);
                break;
            default:
            printf("uknown sequence of tokens");
            exit(1);
        }
    }
    cJSON_Delete(root);
    free(jsonBuffer);
    return 0;
}