#include <stdio.h>
#include <string.h>

char* concat(char* str1, char* str2);
char* itos(int x);
char* ctos(char c);

// File: compiler.dav
// Author: David T.
// Description: Stage 1 compiler for the dav language.
// This file is compiled by the Stage 0 Python compiler.
// =============================================================
// Global Storage
// =============================================================
// --- Tokenizer Storage ---
char* token_types[1000];
int token_values[1000];
// Stores index into token_pool, or -1
int token_lines[1000];
int token_cols[1000];
char token_pool[50000];
// String pool for token values
int n_tokens = 0;
// Total number of tokens found
// --- Parser State ---
int parser_pos = 0;
// Current token index for the parser
// =============================================================
// Function Declarations
// =============================================================
// --- Lexer Helpers ---
int is_letter(char c);
int is_digit(char c);
int is_space(char c);
int is_ident_char(char c);
char* check_keywords(char* s);
int add_simple_token(int index, char* type, int line, int col);
int tokenize(char* source_code);
// --- Parser Helpers ---
char* peek();
int next();
int expect(char* kind);
char* parse();
char* global_decl();
char* fn_decl();
char* let_stmt();
char* comment_stmt();
void print_token() {
    char* buffer = "";
    int i = 0;
    int j = 0;
    int pool_pos = 0;
    while (j < n_tokens) {
        pool_pos = token_values[j];
        i = i + strlen(token_types[j]) + 1;
        buffer = concat(concat(buffer, token_types[j]), " ");
        if (pool_pos != -1) {
            while (token_pool[pool_pos] != '\0') {
                buffer[i] = token_pool[pool_pos];
                pool_pos = pool_pos + 1;
                i = i + 1;
            }
            buffer[i] = ' ';
            i = i + 1;
        }
        j = j + 1;
    }
    buffer[i] = '\0';
    printf("%s\n", buffer);
}

// =============================================================
// Main Entry Point
// =============================================================
int main() {
    // Test code with all global declaration types
    char* code = "// My Stage 1 Compiler\n\nbeg int global_var = 10;\n\nah int my_func();\n\nah int main() {\n    boo(global_var);\n}\n";
    // 1. Tokenize
    printf("%s\n", "--- Tokenizing ---");
    n_tokens = tokenize(code);
    printf("%s\n", "--- Tokenizing Complete ---");
    print_token();
    // 2. Parse
    printf("%s\n", "--- Parsing ---");
    char* c_code = parse();
    printf("%s\n", "--- Generated C Code ---");
    printf("%s\n", c_code);
    return 0;
}

// =============================================================
// Parser
// =============================================================
char* parse() {
    // Main parser entry point.
    // Loops until EOF, parsing all global declarations.
    char* code_buffer = "";
    // This will hold the entire generated C file
    while (strcmp(peek(), "EOF") != 0) {
        code_buffer = concat(code_buffer, global_decl());
    }
    return code_buffer;
}

char* global_decl() {
    // Dispatches to the correct parser function
    // based on the next token.
    char* t = peek();
    if (strcmp(t, "FN") == 0) {
        return fn_decl();
    } else if (strcmp(t, "LET") == 0) {
               return let_stmt();
           } else if (strcmp(t, "COMMENT") == 0) {
               return comment_stmt();
           } else {
               // Error handling
               int tok_line = token_lines[parser_pos];
               printf("%s\n", concat("Error: Unexpected global token on line ", itos(tok_line)));
               printf("%s\n", concat("Expected FN, LET, or COMMENT, but got: ", t));
               // Consume the bad token to prevent infinite loop
               next();
               return "";
               // Return empty string for this declaration
           }
}

char* fn_decl() {
    printf("%s\n", "Parsing function (stub)...");
    // Consume the function prototype/definition
    // ah int my_func(int a) { ... }
    expect("FN");
    if (strcmp(peek(), "TYPE") == 0) {
        next();
    }
    expect("ID");
    expect("LPAREN");
    while (strcmp(peek(), "RPAREN") != 0) {
        next();
        // Consume params
    }
    expect("RPAREN");
    if (strcmp(peek(), "LBRACE") == 0) {
        // Function Definition
        expect("LBRACE");
        while ((strcmp(peek(), "RBRACE") != 0)) {
            next();
            // Consume body
        }
        expect("RBRACE");
    } else {
        // Function Declaration (Prototype)
        expect("SEMICOL");
    }
    return "/* C code for function (stub) */\n";
}

char* let_stmt() {
    printf("%s\n", "Parsing global variable (stub)...");
    // Consume the global variable
    // beg int x = 10;
    expect("LET");
    if (strcmp(peek(), "TYPE") == 0) {
        next();
    }
    expect("ID");
    if (strcmp(peek(), "LSQUARE") == 0) {
        expect("LSQUARE");
        if (strcmp(peek(), "NUMBER") == 0) {
            next();
        }
        expect("RSQUARE");
    }
    if (strcmp(peek(), "ASSIGN") == 0) {
        expect("ASSIGN");
        // Just consume one token for the value (e.g., NUMBER)
        next();
    }
    expect("SEMICOL");
    return "/* C code for global variable (stub) */\n";
}

char* comment_stmt() {
    next();
    return "";
}

// =============================================================
// Parser Helpers
// =============================================================
char* peek() {
    // Returns the type of the current token.
    // Automatically skips over any 'COMMENT' tokens.
    while (strcmp(token_types[parser_pos], "COMMENT") == 0) {
        parser_pos = parser_pos + 1;
    }
    return token_types[parser_pos];
}

int next() {
    // Consumes the current token and returns its index.
    // Make sure to call peek() first to skip comments.
    // Skips any comments
    peek();
    int current_pos = parser_pos;
    parser_pos = parser_pos + 1;
    return current_pos;
}

int expect(char* kind) {
    // Checks if the current token is of the expected 'kind'.
    // If yes, consumes it and returns its index.
    // If no, prints an error and returns -1.
    // Skips comments and gets type
    char* tok_type = peek();
    if (strcmp(tok_type, kind) == 0) {
        // Consume and return index
        return next();
    }
    // Handle error

    int tok_line = token_lines[parser_pos];
    printf("%s\n", concat("Error: Syntax Error on line ", itos(tok_line)));
    printf("%s\n", concat("Expected token: ", kind));
    printf("%s\n", concat("... but got token: ", tok_type));
    // In a real compiler, we'd exit here.
    return -1;
    // Indicate error
}

// =============================================================
// Tokenizer
//
// This is the main lexer logic, ported from python/lexer/lexer.py
// =============================================================
int tokenize(char* source_code) {
    int pos = 0;
    int line_num = 1;
    int line_start = 0;
    char buffer[100];
    int i = 0;
    int token_count = 0;
    int pool_pos = 0;
    char c;
    int col;
    int j;
    char token_val;
    int token_start_col;
    while (source_code[pos] != '\0') {
        c = source_code[pos];
        col = pos - line_start;
        i = 0;
        // --- 1. Skip Whitespace ---
        if (is_space(c)) {
            if (c == '\n') {
                line_num = line_num + 1;
                line_start = pos + 1;
            }
            pos = pos + 1;
        }
        // --- 2. Check for Numbers ---
        else if (is_digit(c)) {
                 token_start_col = col;
                 while (is_digit(c)) {
                buffer[i] = c;
                i = i + 1;
                pos = pos + 1;
                c = source_code[pos];
            }
                 if (c == '.') {
                buffer[i] = c;
                i = i + 1;
                pos = pos + 1;
                c = source_code[pos];
                while (is_digit(c)) {
                    buffer[i] = c;
                    i = i + 1;
                    pos = pos + 1;
                    c = source_code[pos];
                }
            }
                 buffer[i] = '\0';
                 token_types[token_count] = "NUMBER";
                 token_lines[token_count] = line_num;
                 token_cols[token_count] = token_start_col;
                 // Copy buffer to string pool
                 token_values[token_count] = pool_pos;
                 j = 0;
                 // <= to include the '\0'
                 while (j <= i) {
                token_pool[pool_pos] = buffer[j];
                pool_pos = pool_pos + 1;
                j = j + 1;
            }
                 token_count = token_count + 1;
             }
             // --- 3. Check for Identifiers & Keywords ---
             else if (is_letter(c)) {
                 token_start_col = col;
                 while (is_ident_char(c)) {
                buffer[i] = c;
                i = i + 1;
                pos = pos + 1;
                c = source_code[pos];
            }
                 buffer[i] = '\0';
                 char* tok_type = check_keywords(buffer);
                 token_types[token_count] = tok_type;
                 token_lines[token_count] = line_num;
                 token_cols[token_count] = token_start_col;
                 // Copy buffer to string pool
                 if (strcmp(tok_type, "ID") != 0 && strcmp(tok_type, "TYPE") != 0) {
                token_values[token_count] = -1;
            } else {
                token_values[token_count] = pool_pos;
                j = 0;
                while (j <= i) {
                    token_pool[pool_pos] = buffer[j];
                    pool_pos = pool_pos + 1;
                    j = j + 1;
                }
            }
                 token_count = token_count + 1;
             }
             // --- 4. Check for Multi-Char Tokens ---
             else if (c == '=') {
                 if (source_code[pos + 1] == '=') {
                add_simple_token(token_count, "EQ", line_num, col);
                token_count = token_count + 1;
                pos = pos + 2;
            } else {
                add_simple_token(token_count, "ASSIGN", line_num, col);
                token_count = token_count + 1;
                pos = pos + 1;
            }
             } else if (c == '!' && source_code[pos + 1] == '=') {
                 add_simple_token(token_count, "NE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 2;
             } else if (c == '>') {
                 if (source_code[pos + 1] == '=') {
                add_simple_token(token_count, "GE", line_num, col);
                token_count = token_count + 1;
                pos = pos + 2;
            } else {
                add_simple_token(token_count, "GT", line_num, col);
                token_count = token_count + 1;
                pos = pos + 1;
            }
             } else if (c == '<') {
                 if (source_code[pos + 1] == '=') {
                add_simple_token(token_count, "LE", line_num, col);
                token_count = token_count + 1;
                pos = pos + 2;
            } else {
                add_simple_token(token_count, "LT", line_num, col);
                token_count = token_count + 1;
                pos = pos + 1;
            }
             } else if (c == '&' && source_code[pos + 1] == '&') {
                 add_simple_token(token_count, "AND", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 2;
             } else if (c == '|' && source_code[pos + 1] == '|') {
                 add_simple_token(token_count, "OR", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 2;
             } else if (c == '/') {
                 if (source_code[pos + 1] == '/') {
                add_simple_token(token_count, "COMMENT", line_num, col);
                token_count = token_count + 1;
                pos = pos + 2;
                // Loop to skip till after newline or EOL
                while (source_code[pos] != '\n') {
                    pos = pos + 1;
                }
            } else {
                add_simple_token(token_count, "DIV", line_num, col);
                token_count = token_count + 1;
                pos = pos + 1;
            }
             }
             // --- 5. Check for Single-Char Tokens ---
             else if (c == '(') {
                 add_simple_token(token_count, "LPAREN", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == ')') {
                 add_simple_token(token_count, "RPAREN", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '{') {
                 add_simple_token(token_count, "LBRACE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '}') {
                 add_simple_token(token_count, "RBRACE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '[') {
                 add_simple_token(token_count, "LSQUARE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == ']') {
                 add_simple_token(token_count, "RSQUARE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '+') {
                 add_simple_token(token_count, "PLUS", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '-') {
                 add_simple_token(token_count, "MINUS", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '*') {
                 add_simple_token(token_count, "MUL", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == ';') {
                 add_simple_token(token_count, "SEMICOL", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == ',') {
                 add_simple_token(token_count, "COMMA", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             }
             // --- 6. Handle Strings and Chars ---
             else if (c == '"') {
                 token_start_col = col;
                 pos = pos + 1;
                 c = source_code[pos];
                 while (c != '"' && c != '\0') {
                if (c == '\\') {
                    pos = pos + 1;
                    c = source_code[pos];
                    if (c == 'n') {
                        buffer[i] = '\n';
                    } else if (c == 't') {
                               buffer[i] = '\t';
                           } else if (c == '"') {
                               buffer[i] = '"';
                           } else if (c == '\\') {
                               buffer[i] = '\\';
                           } else {
                               buffer[i] = c;
                           }
                } else {
                    buffer[i] = c;
                }
                i = i + 1;
                pos = pos + 1;
                c = source_code[pos];
            }
                 // Check unclosed string
                 if (c == '\0') {
                printf("%s\n", "Error: Unclosed string literal!");
                return 1;
            }
                 pos = pos + 1;
                 buffer[i] = '\0';
                 // Add token
                 token_types[token_count] = "STRING";
                 token_lines[token_count] = line_num;
                 token_cols[token_count] = token_start_col;
                 token_values[token_count] = pool_pos;
                 j = 0;
                 while ((j <= i)) {
                token_pool[pool_pos] = buffer[j];
                pool_pos = pool_pos + 1;
                j = j + 1;
            }
                 token_count = token_count + 1;
             } else if (c == '\'') {
                 token_start_col = col;
                 pos = pos + 1;
                 c = source_code[pos];
                 token_val = c;
                 if (c == '\\') {
                pos = pos + 1;
                c = source_code[pos];
                if (c == 'n') {
                    token_val = '\n';
                } else if (c == 't') {
                           token_val = '\t';
                       } else if (c == '\'') {
                           token_val = '\'';
                       } else if (c == '\\') {
                           token_val = '\\';
                       } else {
                           token_val = c;
                       }
            }
                 pos = pos + 1;
                 c = source_code[pos];
                 if (c != '\'') {
                printf("%s\n", "Error: Unclosed or invalid char literal!");
                return 1;
            }
                 pos = pos + 1;
                 // Add token
                 buffer[0] = token_val;
                 buffer[1] = '\0';
                 token_types[token_count] = "CHAR";
                 token_lines[token_count] = line_num;
                 token_cols[token_count] = token_start_col;
                 token_values[token_count] = pool_pos;
                 token_pool[pool_pos] = buffer[0];
                 token_pool[pool_pos + 1] = buffer[1];
                 pool_pos = pool_pos + 2;
                 token_count = token_count + 1;
             }
             // --- 7. Handle Errors ---
             else {
                 printf("%s\n", "Error: Unexpected character!");
                 printf("%s\n", ctos(c));
                 return 1;
             }
    }
    // Add EOF Token
    add_simple_token(token_count, "EOF", line_num, col);
    token_count = token_count + 1;
    return token_count;
}

// =============================================================
// Lexer Helpers
//
// We port the logic from the Python lexer.
// =============================================================
int is_letter(char c) {
    // Checks if a character is a letter or underscore.
    // Corresponds to: [A-Za-z_]
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

int is_digit(char c) {
    // Checks if a character is a 0-9 digit.
    // Corresponds to: \d
    return (c >= '0' && c <= '9');
}

int is_space(char c) {
    // Checks for whitespace characters to skip.
    // Corresponds to: [ \t\n]
    return (c == ' ') || (c == '\t') || (c == '\n');
}

int is_ident_char(char c) {
    // Checks if a char can be part of an identifier *after* the first char.
    // Corresponds to: [A-Za-z0-9_]
    return is_letter(c) || is_digit(c);
}

char* check_keywords(char* s) {
    // Checks if a string 's' is a keyword.
    // If it is, return the keyword's Token Type.
    // Otherwise, return "ID".
    if (strcmp(s, "ah") == 0) {
        return "FN";
    } else if (strcmp(s, "beg") == 0) {
               return "LET";
           } else if (strcmp(s, "boo") == 0) {
               return "PRINT";
           } else if (strcmp(s, "if") == 0) {
               return "IF";
           } else if (strcmp(s, "else") == 0) {
               return "ELSE";
           } else if (strcmp(s, "while") == 0) {
               return "WHILE";
           } else if (strcmp(s, "return") == 0) {
               return "RETURN";
           } else if (strcmp(s, "int*") == 0 || strcmp(s, "char*") == 0 || strcmp(s, "int") == 0 || strcmp(s, "char") == 0 || strcmp(s, "void") == 0) {
               return "TYPE";
           }
           // Default case: not a keyword

    return "ID";
}

// Helper to add a simple token (without a value) to the token arrays.
int add_simple_token(int index, char* type, int line, int col) {
    token_types[index] = type;
    token_values[index] = -1;
    // -1 means no value
    token_lines[index] = line;
    token_cols[index] = col;
    return 0;
}

char* concat(char* str1, char* str2) {
    static char buf[1024];
    snprintf(buf, sizeof(buf), "%s%s", str1, str2);
    return buf;
}

char* itos(int x) {
    static char buf[32];
    snprintf(buf, sizeof(buf), "%d", x);
    return buf;
}

char* ctos(char c) {
    static char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return buf;
}

