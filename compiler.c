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

// =============================================================
// Tokenizer
//
// This is the main lexer logic, ported from python/lexer/lexer.py
// =============================================================
int tokenize(char* source_code) {
    int pos = 0;
    int line_num = 1;
    int line_start = 0;
    // Buffer for string/num/id values
    char buffer[100];
    // Buffer index
    int i = 0;
    printf("%s\n", "Starting tokenizer...");
    // Debug print
    // Loop until we see a null terminator '\0'
    while (source_code[pos] != '\0') {
        char c = source_code[pos];
        int col = pos - line_start;
        // Reset buffer index
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
                 // Consume the integer
                 while (is_digit(c)) {
                buffer[i] = c;
                i = i + 1;
                pos = pos + 1;
                c = source_code[pos];
            }
                 // Check for a decimal part
                 if (c == '.') {
                buffer[i] = c;
                i = i + 1;
                pos = pos + 1;
                c = source_code[pos];
                // Consume the fractional part
                while (is_digit(c)) {
                    buffer[i] = c;
                    i = i + 1;
                    pos = pos + 1;
                    c = source_code[pos];
                }
            }
            // Null-terminate the string in the buffer

                 buffer[i] = '\0';
                 printf("%s\n", concat("Found NUMBER:", buffer));
             }
             // --- 3. Check for Identifiers & Keywords ---
             else if (is_letter(c)) {
                 // Consume letter and digit
                 while (is_ident_char(c)) {
                buffer[i] = c;
                i = i + 1;
                pos = pos + 1;
                c = source_code[pos];
            }
                 // Null-terminate the string in the buffer
                 buffer[i] = '\0';
                 // Check if the identifier is a keyword
                 char* token_type = check_keywords(buffer);
                 printf("%s\n", token_type);
                 printf("%s\n", buffer);
             }
             // --- 4. Check for Single-Char Tokens ---
             else if (c == '(') {
                 printf("%s\n", "Found LPAREN");
                 pos = pos + 1;
             } else if (c == ')') {
                 printf("%s\n", "Found RPAREN");
                 pos = pos + 1;
             } else if (c == '{') {
                 printf("%s\n", "Found LBRACE");
                 pos = pos + 1;
             } else if (c == '}') {
                 printf("%s\n", "Found RBRACE");
                 pos = pos + 1;
             } else if (c == '[') {
                 printf("%s\n", "Found LSQUARE");
                 pos = pos + 1;
             } else if (c == ']') {
                 printf("%s\n", "Found RSQUARE");
                 pos = pos + 1;
             } else if (c == '+') {
                 printf("%s\n", "Found PLUS");
                 pos = pos + 1;
             } else if (c == '-') {
                 printf("%s\n", "Found MINUS");
                 pos = pos + 1;
             } else if (c == '*') {
                 printf("%s\n", "Found MUL");
                 pos = pos + 1;
             } else if (c == '/') {
                 printf("%s\n", "Found DIV");
                 pos = pos + 1;
             } else if (c == ';') {
                 printf("%s\n", "Found SEMICOL");
                 pos = pos + 1;
             } else if (c == ',') {
                 printf("%s\n", "Found COMMA");
                 pos = pos + 1;
             }
             // --- 5. Check for Multi-Char Tokens ---
             else if (c == '=') {
                 if (source_code[pos + 1] == '=') {
                printf("%s\n", "Found EQ");
                pos = pos + 2;
            } else {
                printf("%s\n", "Found ASSIGN");
                pos = pos + 1;
            }
             } else if (c == '!' && source_code[pos + 1] == '=') {
                 printf("%s\n", "Found NE");
                 pos = pos + 2;
             } else if (c == '>') {
                 if (source_code[pos + 1] == '=') {
                printf("%s\n", "Found GE");
                pos = pos + 2;
            } else {
                printf("%s\n", "Found GT");
                pos = pos + 1;
            }
             } else if (c == '<') {
                 if (source_code[pos + 1] == '=') {
                printf("%s\n", "Found LE");
                pos = pos + 2;
            } else {
                printf("%s\n", "Found LT");
                pos = pos + 1;
            }
             } else if (c == '&' && source_code[pos + 1] == '&') {
                 printf("%s\n", "Found AND");
                 pos = pos + 2;
             } else if (c == '|' && source_code[pos + 1] == '|') {
                 printf("%s\n", "Found OR");
                 pos = pos + 2;
             }
             // --- 6. Handle Strings and Chars ---
             else if (c == '"') {
                 // Skip over left double-quote
                 pos = pos + 1;
                 c = source_code[pos];
                 // Between double-quotes
                 while (c != '"' && c != '\0') {
                if ((c == '\\')) {
                    // Escape characters, consume '\'
                    pos = pos + 1;
                    c = source_code[pos];
                    if ((c == 'n')) {
                        buffer[i] = '\n';
                    } else if ((c == 't')) {
                               buffer[i] = '\t';
                           } else if ((c == '"')) {
                               buffer[i] = '"';
                           } else if ((c == '\\')) {
                               buffer[i] = '\\';
                           } else {
                               // Unknown escape, just add the char
                               buffer[i] = c;
                           }
                } else {
                    // Regular character
                    buffer[i] = c;
                }
                i = i + 1;
                pos = pos + 1;
                c = source_code[pos];
            }
                 // Check unclosed string
                 if ((c == '\0')) {
                printf("%s\n", "Error: Unclosed string literal!");
                return 1;
            }
            // Skip over right double-quote

                 pos = pos + 1;
                 // Null-terminate the string in the buffer
                 buffer[i] = '\0';
                 printf("%s\n", "Found STRING");
                 printf("%s\n", buffer);
             } else if (c == '\'') {
                 // Skip over left quote
                 pos = pos + 1;
                 c = source_code[pos];
                 // Store the char
                 char token_val = c;
                 if ((c == '\\')) {
                // Escape characters, consume '\'
                pos = pos + 1;
                c = source_code[pos];
                if ((c == 'n')) {
                    token_val = '\n';
                } else if ((c == 't')) {
                           token_val = '\t';
                       } else if ((c == '\'')) {
                           token_val = '\'';
                       } else if ((c == '\\')) {
                           token_val = '\\';
                       } else {
                           token_val = c;
                       }
            }
            // Move to the *next* char, which should be the closing quote

                 pos = pos + 1;
                 c = source_code[pos];
                 // Check for closing quote
                 if ((c != '\'')) {
                printf("%s\n", "Error: Unclosed or invalid char literal!");
                return 1;
            }
            // Skip over right quote

                 pos = pos + 1;
                 // "Add" the token
                 printf("%s\n", "Found CHAR");
                 // Store char in buffer to print as a 1-char string
                 buffer[0] = token_val;
                 buffer[1] = '\0';
                 printf("%s\n", buffer);
             }
             // --- 7. Handle Errors ---
             else {
                 printf("%s\n", "Error: Unexpected character!");
                 // Converts char 'c' to string
                 printf("%s\n", ctos(c));
                 // Exit with error
                 return 1;
             }
    }
    printf("%s\n", "Tokenizing complete.");
    return 0;
}

// =============================================================
// Main Entry Point
// =============================================================
int main() {
    // This is a stub to test the lexer helpers
    char x = '0';
    char y = 'a';
    char z = '_';
    char w = ' ';
    printf("%d\n", is_digit(x));
    // Should print 1 (true)
    printf("%d\n", is_letter(y));
    // Should print 1 (true)
    printf("%d\n", is_letter(z));
    // Should print 1 (true)
    printf("%d\n", is_space(w));
    // Should print 1 (true)
    printf("%d\n", is_letter('5'));
    // Should print 0 (false)
    // A simple test of the tokenizer
    tokenize("ah main() { beg x = 5; }");
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

