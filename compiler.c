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
         }
         // Default case: not a keyword

    return "ID";
}

// Helper to add a simple token (without a value) to the token arrays.
int add_simple_token(char* token_types[1000], int token_values[1000], int token_lines[1000], int token_cols[1000], int index, char* type, int line, int col) {
    token_types[index] = type;
    token_values[index] = -1;
    // -1 means no value
    token_lines[index] = line;
    token_cols[index] = col;
    return 0;
}

// =============================================================
// Tokenizer
//
// This is the main lexer logic, ported from python/lexer/lexer.py
// =============================================================
int tokenize(char* source_code, char* token_types[1000], int token_values[1000], int token_lines[1000], int token_cols[1000], char token_pool[50000]) {
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
                 token_types[token_count] = check_keywords(buffer);
                 token_lines[token_count] = line_num;
                 token_cols[token_count] = token_start_col;
                 // Copy buffer to string pool
                 token_values[token_count] = pool_pos;
                 j = 0;
                 while (j <= i) {
                token_pool[pool_pos] = buffer[j];
                pool_pos = pool_pos + 1;
                j = j + 1;
            }
                 token_count = token_count + 1;
             }
             // --- 4. Check for Single-Char Tokens ---
             else if (c == '(') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "LPAREN", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == ')') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "RPAREN", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '{') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "LBRACE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '}') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "RBRACE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '[') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "LSQUARE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == ']') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "RSQUARE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '+') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "PLUS", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '-') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "MINUS", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '*') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "MUL", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == '/') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "DIV", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == ';') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "SEMICOL", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             } else if (c == ',') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "COMMA", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 1;
             }
             // --- 5. Check for Multi-Char Tokens ---
             else if (c == '=') {
                 if (source_code[pos + 1] == '=') {
                add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "EQ", line_num, col);
                token_count = token_count + 1;
                pos = pos + 2;
            } else {
                add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "ASSIGN", line_num, col);
                token_count = token_count + 1;
                pos = pos + 1;
            }
             } else if (c == '!' && source_code[pos + 1] == '=') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "NE", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 2;
             } else if (c == '>') {
                 if (source_code[pos + 1] == '=') {
                add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "GE", line_num, col);
                token_count = token_count + 1;
                pos = pos + 2;
            } else {
                add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "GT", line_num, col);
                token_count = token_count + 1;
                pos = pos + 1;
            }
             } else if (c == '<') {
                 if (source_code[pos + 1] == '=') {
                add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "LE", line_num, col);
                token_count = token_count + 1;
                pos = pos + 2;
            } else {
                add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "LT", line_num, col);
                token_count = token_count + 1;
                pos = pos + 1;
            }
             } else if (c == '&' && source_code[pos + 1] == '&') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "AND", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 2;
             } else if (c == '|' && source_code[pos + 1] == '|') {
                 add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "OR", line_num, col);
                 token_count = token_count + 1;
                 pos = pos + 2;
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
    add_simple_token(token_types, token_values, token_lines, token_cols, token_count, "EOF", line_num, col);
    token_count = token_count + 1;
    return token_count;
}

// =============================================================
// Main Entry Point
// =============================================================
int main() {
    // --- NEW: Declare token storage arrays ---
    // These arrays will be populated by the tokenizer
    // Max 1000 tokens
    char* token_types[1000];
    // token_values stores an *index* into the token_pool
    // -1 means no value (for single-char tokens)
    int token_values[1000];
    int token_lines[1000];
    int token_cols[1000];
    // A "string pool" or "arena" for all token values (strings, numbers)
    // 50k chars should be enough for this compiler
    char token_pool[50000];
    // A simple test of the tokenizer
    char* code = "ah main() { beg x = 123.45; }";
    int n_tokens = tokenize(code, token_types, token_values, token_lines, token_cols, token_pool);
    // Print the tokens we stored
    printf("%s\n", "--- Stored Tokens ---");
    int i = 0;
    char* type;
    int val_idx;
    while (i < n_tokens) {
        type = token_types[i];
        val_idx = token_values[i];
        printf("%s\n", type);
        if (val_idx != -1) {
            printf("%s\n", (token_pool + val_idx));
        }
        i = i + 1;
    }
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

