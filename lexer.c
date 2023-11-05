#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "include\lexer.h"

#define INITIAL_BUFFER_SIZE 64

char* read_lexeme(FILE* source) {
    int buffer_size = INITIAL_BUFFER_SIZE;
    int lexeme_length = 0;
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    
    char ch;
    while ((ch = fgetc(source)) != EOF && (isalnum(ch) || ch == '.' || ch == '_')) {
        // Resize the buffer if necessary
        if (lexeme_length + 1 >= buffer_size) {
            buffer_size *= 2;  // Double the buffer size
            char* new_buffer = realloc(buffer, buffer_size);
            if (!new_buffer) {
                free(buffer);
                fprintf(stderr, "Memory reallocation failed.\n");
                exit(EXIT_FAILURE);
            }
            buffer = new_buffer;
        }
        
        buffer[lexeme_length++] = ch;
    }
    
    if (ch != EOF) {
        ungetc(ch, source);  // Put back the last character read if not EOF
    }
    
    buffer[lexeme_length] = '\0'; // Null-terminate the string
    return buffer;
}

Token get_next_token(FILE *source, int line){
    char ch;
    Token token;
    // skip all whitespaces
    do{
        ch = fgetc(source);
        // Check if it a new line
        if(ch == '\n'){ 
            line ++;
            token.type = TOKEN_NEW_LINE;
            token.line = line;
            token.lexeme = "";
            return token;
        }
    }while(isspace(ch));
    // check if end of file
    if (feof(source)) {
        token.type = TOKEN_EOF;
        return token;
    }

    switch (ch)
    {
    case '#':
        // Check if it,s a declaration
        ch = fgetc(source);
         if (ch == 'i') {
            token.type = TOKEN_INT_DECL;
            token.lexeme = "#i";
        } else if (ch == 'd') {
            token.type = TOKEN_DOUBLE_DECL;
            token.lexeme = "#d";
        } else {
            token.type = TOKEN_ERROR; //return an error if it's not a declaration
        }
        break;
    case '+':
        // Check for +=
        ch = fgetc(source);
        if (ch == '=') {
            token.type = TOKEN_PLUS_ASSIGN;
            token.lexeme = "+=";
        } else {
            ungetc(ch, source); // Put back the character that is not part of this token
            token.type = TOKEN_PLUS;
            token.lexeme = "+";
        }
        break;
    case '-':
        // Check for -=
        ch = fgetc(source);
        if (ch == '=') {
            token.type = TOKEN_MINUS_ASSIGN;
            token.lexeme = "-=";
        } else {
            ungetc(ch, source); // Put back the character that is not part of this token
            token.type = TOKEN_MINUS;
            token.lexeme = "-";
        }
        break;
    case '*':
        // Check for *=
        ch = fgetc(source);
        if (ch == '=') {
            token.type = TOKEN_MULTI_ASSIGN;
            token.lexeme = "*=";
        } else {
            ungetc(ch, source); // Put back the character that is not part of this token
            token.type = TOKEN_MULTI;
            token.lexeme = "*";
        }
        break;
    case '/':
        // Check for /=
        ch = fgetc(source);
        if (ch == '=') {
            token.type = TOKEN_DIVISION_ASSIGN;
            token.lexeme = "/=";
        } else if (ch == '/'){
            ch = fgetc(source);
            if (ch == '=') {
                token.type = TOKEN_DIV_ASSIGN;
                token.lexeme = "//=";
            } else {
                ungetc(ch, source); // Put back the character that is not part of this token
                token.type = TOKEN_DIV;
                token.lexeme = "//";
            }
           
        }else{
            ungetc(ch, source); // Put back the character that is not part of this token
            token.type = TOKEN_DIVISION;
            token.lexeme = "/";
        }
        break;
    default:
        if (isdigit(ch) || ch == '.'){
            ungetc(ch, source);
            char *buffer = read_lexeme(source); 
            if (fscanf(source, "%255[0-9.]", buffer) == 1) {
                if (strchr(buffer, '.') != NULL) {
                    token.type = TOKEN_DOUBLE_LITERAL;
                    token.value.d_val = atof(buffer);
                } else {
                    token.type = TOKEN_INT_LITERAL;
                    token.value.i_val = atoi(buffer);
                }
                token.lexeme = strdup(buffer); // Duplicate the buffer to token.lexeme
                free(buffer); // Make sure to free this later
            } else {
                // Error handling for malformed number
                token.type = TOKEN_ERROR;
            }

        }else if (isalpha(ch)) {
            ungetc(ch, source);
            char *lexeme = read_lexeme(source); // Assume this function reads an identifier/keyword

            // Check if the lexeme is a keyword
            if (strcmp(lexeme, "print") == 0) {
                token.type = TOKEN_PRINT;
            } else if (strcmp(lexeme, "input") == 0) {
                token.type = TOKEN_INPUT;
            } else if (strcmp(lexeme, "then") == 0) {
                token.type = TOKEN_THEN;
            } else if (strcmp(lexeme, "else") == 0) {
                token.type = TOKEN_ELSE;
            } else {
                token.type = TOKEN_IDENTIFIER; // If it's not a keyword, it's an identifier
            }
            token.lexeme = strdup(lexeme); // Make sure to free this later
            free(lexeme);
        } else {
            // Handle single character tokens
            switch (ch) {
                case '=':
                    ch = fgetc(source);
                    if (ch == '=') {
                        token.type = TOKEN_EQUAL;
                        token.lexeme = "==";
                    }else{
                        ungetc(ch, source);
                        token.type = TOKEN_ASSIGN;
                        token.lexeme = "="; 
                    }
                    break;
                case '>':
                    token.type = TOKEN_GREATER;
                    token.lexeme = ">"; 
                    break;
                case '<':
                    token.type = TOKEN_LESS;
                    token.lexeme = "<"; 
                    break;
                case '(':
                    token.type = TOKEN_OPEN_PAREN;
                    token.lexeme = "(";
                    break;
                case ')':
                    token.type = TOKEN_CLOSE_PAREN;
                    token.lexeme = ")";
                    break;
                case '{':
                    token.type = TOKEN_OPEN_BRACE;
                    token.lexeme = "{";
                    break;
                case '}':
                    token.type = TOKEN_CLOSE_BRACE;
                    token.lexeme = "}";
                    break;
                case '?':
                    token.type = TOKEN_THEN;
                    token.lexeme = "?";
                    break;
                case ':':
                    token.type = TOKEN_ELSE;
                    token.lexeme = ":";
                    break;
                default:
                    token.type = TOKEN_ERROR;
                    break;
            }
        }
        break;
    }
    token.line = line;
    return token;
}

// for printing token info in terminal
const char* getTokenTypeName(TokenType type) {
     switch (type) {
        case TOKEN_INT_DECL: return "TOKEN_INT_DECL";
        case TOKEN_DOUBLE_DECL: return "TOKEN_DOUBLE_DECL";
        case TOKEN_INT_LITERAL: return "TOKEN_INT_LITERAL";
        case TOKEN_DOUBLE_LITERAL: return "TOKEN_DOUBLE_LITERAL";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_MULTI: return "TOKEN_MULTI";
        case TOKEN_DIVISION: return "TOKEN_DIVISION";
        case TOKEN_DIV: return "TOKEN_DIV";
        case TOKEN_MOD: return "TOKEN_MOD";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_PLUS_ASSIGN: return "TOKEN_PLUS_ASSIGN";
        case TOKEN_MINUS_ASSIGN: return "TOKEN_MINUS_ASSIGN";
        case TOKEN_MULTI_ASSIGN: return "TOKEN_MULTI_ASSIGN";
        case TOKEN_DIVISION_ASSIGN: return "TOKEN_DIVISION_ASSIGN";
        case TOKEN_DIV_ASSIGN: return "TOKEN_DIV_ASSIGN";
        case TOKEN_LESS: return "TOKEN_LESS";
        case TOKEN_GREATER: return "TOKEN_GREATER";
        case TOKEN_EQUAL: return "TOKEN_EQUAL";
        case TOKEN_OPEN_PAREN: return "TOKEN_OPEN_PAREN";
        case TOKEN_CLOSE_PAREN: return "TOKEN_CLOSE_PAREN";
        case TOKEN_OPEN_BRACE: return "TOKEN_OPEN_BRACE";
        case TOKEN_CLOSE_BRACE: return "TOKEN_CLOSE_BRACE";
        case TOKEN_PRINT: return "TOKEN_PRINT";
        case TOKEN_INPUT: return "TOKEN_INPUT";
        case TOKEN_CONDITION: return "TOKEN_CONDITION";
        case TOKEN_THEN: return "TOKEN_THEN";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_ERROR: return "TOKEN_ERROR";
        case TOKEN_NEW_LINE: return "TOKEN_NEW_LINE";
        default: return "Unknown TokenType";
    }
}

void token_to_json(FILE* file, Token token) {
    const char* tokenType = getTokenTypeName(token.type);
    char json[1024]; // Assuming a single token won't exceed 1024 characters
    if(token.type == TOKEN_EOF || token.type == TOKEN_ERROR){
        sprintf(json, "{\"type\":\"%s\", \"lexeme\":\"%s\", \"line\":%d}\n", tokenType, token.lexeme, token.line);
        fputs(json, file);
        return;
    }
    // Generate the JSON string based on the type of the token
    switch (token.type) {
        case TOKEN_INT_LITERAL:
            sprintf(json, "{\"type\":\"%s\", \"value\":%d, \"line\":%d},\n", tokenType, token.value.i_val, token.line);
            break;
        case TOKEN_DOUBLE_LITERAL:
            sprintf(json, "{\"type\":\"%s\", \"value\":%f, \"line\":%d},\n", tokenType, token.value.d_val, token.line);
            break;
        default:
            sprintf(json, "{\"type\":\"%s\", \"lexeme\":\"%s\", \"line\":%d},\n", tokenType, token.lexeme, token.line);
            break;
    }

    // Write the JSON string to the file
    fputs(json, file);
}


int main(){
    FILE *source = fopen("example/example.capatichi", "r");
    FILE *jsonFile = fopen("tokens/tokens.json", "w");
    if(!source){
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    Token token;
    token.line = 1;
    fputs("[", jsonFile);
    do {
        token = get_next_token(source, token.line);
        token_to_json(jsonFile, token);
        printf("%s %s %i\n", getTokenTypeName(token.type), token.lexeme, token.line);
        // Process the token, e.g., print it or store it for the next phase
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
    fputs("]", jsonFile);
    fclose(jsonFile); // Close the JSON file
    fclose(source); // Close the source file
    return 0;
}