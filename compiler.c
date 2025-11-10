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

// We will also need helpers for keywords, but we can add those later.
// e.g., ah int check_keywords(char* s) { ... }
// =============================================================
// Tokenizer
//
// This is the main lexer logic, ported from python/lexer/lexer.py
// =============================================================
int tokenize(char* source_code) {
    int pos = 0;
    int line_num = 1;
    int line_start = 0;
    printf("%s\n", "Starting tokenizer...");
    // Debug print
    // We will need to get the length of the source code.
    // Let's assume a helper function 'str_len(char* s)' exists.
    // We will need to add it to the C helpers.
    // beg int length = str_len(source_code);
    // For now, let's just loop until we see a null terminator '\0'
    while (source_code[pos] != '\0') {
        char c = source_code[pos];
        // Calculate column
        int col = pos - line_start;
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
                 // We found the start of a number
                 // We need to consume all digits
                 // (We'll add this logic next)
                 printf("%s\n", "Found NUMBER");
                 pos = pos + 1;
                 // Stub: just consume one char
             }
             // --- 3. Check for Identifiers & Keywords ---
             else if (is_letter(c)) {
                 // We found the start of an identifier
                 // We need to consume all ident_chars
                 // (We'll add this logic next)
                 printf("%s\n", "Found ID");
                 pos = pos + 1;
                 // Stub: just consume one char
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
                 printf("%s\n", "Found LBRACE");
                 pos = pos + 1;
             } else if (c == ']') {
                 printf("%s\n", "Found RBRACE");
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
             }
             // (add cases for != >= <= && ||)
             // --- 6. Handle Strings and Chars ---
             else if (c == '"') {
                 // Found start of a string
                 // (We'll add this logic next)
                 printf("%s\n", "Found STRING");
                 pos = pos + 1;
                 // Stub
             } else if (c == '\'') {
                 // Found start of a char
                 // (We'll add this logic next)
                 printf("%s\n", "Found CHAR");
                 pos = pos + 1;
                 // Stub
             }
             // --- 7. Handle Errors ---
             else {
                 printf("%s\n", "Error: Unexpected character!");
                 // We need a way to print the char: boo(ctos(c));
                 return 1;
                 // Exit with error
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

