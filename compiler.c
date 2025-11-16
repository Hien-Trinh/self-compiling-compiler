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
char* expr_type;
// Type of the last parsed expression, works like a forgetful stack
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
char* parse();
char* global_decl();
char* fn_decl();
char* statement();
char* let_stmt(int is_global);
char* comment_stmt();
char* expr();
char* logical();
char* relational();
char* additive();
char* multiplicative();
char* unary();
char* atom();
char* peek();
int next();
int expect(char* kind);
int clear_local_symbols();
char* get_symbol_type(int is_global, char* name);
int add_symbol(int is_global, char* name, char* type);
int str_ends_with(char* s, char c);
char* op_to_c_op(char* tok_type);
// --- Symbol Table Storage ---
// We store 'char*' pointers for names and types.
// The Stage 0 compiler will get the name strings from the token_pool.
// The type strings will be string literals (e.g., "int", "char*").
// Global Scope (self.env)
char* global_names[100];
char* global_types[100];
int n_globals = 0;
// Local Scope (self.variables)
char* local_names[100];
char* local_types[100];
int n_locals = 0;
void print_token() {
    printf("%s\n", "--- Stored Tokens ---");
    int i = 0;
    int pool_pos = 0;
    while ((i < n_tokens)) {
        printf("%s\n", token_types[i]);
        pool_pos = token_values[i];
        if ((pool_pos != -1)) {
            // It has a value. Print it using pointer arithmetic.
            printf("%s\n", token_pool + pool_pos);
        }
        i = i + 1;
    }
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
    char* tok = peek();
    if (strcmp(tok, "FN") == 0) {
        return fn_decl();
    } else if (strcmp(tok, "LET") == 0) {
               return let_stmt(1);
           } else if (strcmp(tok, "COMMENT") == 0) {
               return comment_stmt();
           } else {
               // Error handling
               int tok_line = token_lines[parser_pos];
               printf("%s\n", concat("Error: Unexpected global token on line ", itos(tok_line)));
               printf("%s\n", concat("Expected FN, LET, or COMMENT, but got: ", tok));
               // Consume the bad token to prevent infinite loop
               next();
               return "";
               // Return empty string for this declaration
           }
}

char* fn_decl() {
    // Consume the function prototype/definition
    // ah int my_func(int a) { ... }
    expect("FN");
    // --- Get Type ---
    char* fn_type = "int";
    // Default type
    int fn_type_idx = -1;
    if (strcmp(peek(), "TYPE") == 0) {
        fn_type_idx = next();
        fn_type = token_pool + token_values[fn_type_idx];
    }
    // --- Get Name ---

    int fn_name_idx = expect("ID");
    char* fn_name = token_pool + token_values[fn_name_idx];
    // --- Add to global ---
    add_symbol(1, fn_name, fn_type);
    expect("LPAREN");
    while (strcmp(peek(), "RPAREN") != 0) {
        next();
        // Consume params
    }
    expect("RPAREN");
    // --- Inside the LBRACE branch ---
    if (strcmp(peek(), "LBRACE") == 0) {
        // Function Definition
        next();
        // --- Setup local scope ---
        clear_local_symbols();
        // TODO: Loop through params and add_symbol("local", ...)
        // ---
        char* body = "";
        while (strcmp(peek(), "RBRACE") != 0) {
            // TODO: This needs to call statement(), not next()
            next();
            // Stub
        }
        expect("RBRACE");
        return "/* C code for function (stub) */\n";
    } else {
        // ... (prototype logic) ...
    }
    return "/* C code for function (stub) */\n";
}

char* let_stmt(int is_global) {
    int line_num = token_lines[parser_pos];
    expect("LET");
    // --- Get Type ---
    char* var_type = "int";
    // Default type
    int var_type_idx = -1;
    if (strcmp(peek(), "TYPE") == 0) {
        var_type_idx = next();
        var_type = token_pool + token_values[var_type_idx];
    }
    // --- Get Name ---

    int var_name_idx = expect("ID");
    char* var_name = token_pool + token_values[var_name_idx];
    // Check redefinition
    if ((is_global == 0 && strcmp(get_symbol_type(0, var_name), "") != 0) || (is_global == 1 && strcmp(get_symbol_type(1, var_name), "") != 0)) {
        printf("%s\n", "Error: Redefinition of variable");
        printf("%s\n", var_name);
        return "";
        // Error
    }
    // --- Parsing Cases ---

    if (strcmp(peek(), "ASSIGN") == 0) {
        // --- Case 1: Declaration with Assignment (e.g., beg x = 10) ---
        next();
        char* right_expr = expr();
        char* right_type = expr_type;
        if (strcmp(var_type, "int") == 0) {
            var_type = right_type;
            // Infer type
        } else if (strcmp(var_type, right_type) != 0) {
                   printf("%s\n", concat(concat(concat(concat(concat("Error: Incompatible type ", right_type), " to "), var_type), ", line "), itos(line_num)));
                   return "";
               }
        expect("SEMICOL");
        add_symbol(is_global, var_name, var_type);
        // C code: e.g., "int x = 5;"
        return concat(concat(concat(concat(concat(var_type, " "), var_name), " = "), right_expr), ";\n");
    } else if (strcmp(peek(), "LSQUARE") == 0) {
             // --- Case 2: Array Declaration (e.g., beg int arr[10]) ---
             next();
             if (strcmp(var_type, "int") == 0) {
            printf("%s\n", concat("Error: Array declaration must have an explicit type on line", itos(line_num)));
            return "";
        }
             int size_tok = expect("NUMBER");
             char* size = token_pool + token_values[size_tok];
             expect("RSQUARE");
             expect("SEMICOL");
             // Store array type as 'base_type*' (e.g., 'int*')
             char* array_type = concat(var_type, "*");
             add_symbol(is_global, var_name, array_type);
             // C code: e.g., "int arr[10];"
             return concat(concat(concat(concat(concat(var_type, " "), var_name), "["), size), "];\n");
         } else if (strcmp(peek(), "SEMICOL") == 0) {
             // --- Case 3: Declaration without Assignment (e.g., beg int x;) ---
             next();
             if (strcmp(var_type, "int") == 0) {
            printf("%s\n", concat("Error: Declaration without assignment must have explicit type on line", itos(line_num)));
            return "";
        }
             add_symbol(is_global, var_name, var_type);
             // C code: e.g., "int x;"
             return concat(concat(concat(var_type, " "), var_name), ";\n");
         } else {
             printf("%s\n", concat("Error: Expected '=', '[', or ';' after variable name on line", itos(line_num)));
             next();
             // Consume bad token
             return "";
         }
}

char* comment_stmt() {
    next();
    return "";
}

// =============================================================
// Expression Parsers
// =============================================================
char* expr() {
    // Main entry point for parsing an expression.
    // Returns C code string. Sets global 'expr_type'.
    return logical();
}

char* logical() {
    // Handles: expr (&& | ||) expr
    char* code = relational();
    char* left_type = expr_type;
    while (strcmp(peek(), "OR") == 0 || strcmp(peek(), "AND") == 0) {
        int op_idx = next();
        char* op = op_to_c_op(token_types[op_idx]);
        char* right_code = relational();
        char* right_type = expr_type;
        // Type check: logical ops must be on ints (or chars)
        if (strcmp(left_type, "int") != 0 || strcmp(right_type, "int") != 0) {
            printf("%s\n", concat("Error: Logical operators '&&' and '||' can only be used on integers, line ", itos(token_lines[op_idx])));
            return "";
        }
        code = concat(concat(concat(concat(code, " "), op), " "), right_code);
        expr_type = "int";
        // Result is always an int
        left_type = "int";
    }
    expr_type = left_type;
    // Set final type
    return code;
}

char* relational() {
    // Handles: expr (== | != | < | > | <= | >=) expr
    char* code = additive();
    char* left_type = expr_type;
    while (strcmp(peek(), "EQ") == 0 || strcmp(peek(), "NE") == 0 || strcmp(peek(), "LT") == 0 || strcmp(peek(), "GT") == 0 || strcmp(peek(), "LE") == 0 || strcmp(peek(), "GE") == 0) {
        int op_idx = next();
        char* op = op_to_c_op(token_types[op_idx]);
        int line = token_lines[op_idx];
        char* right_code = additive();
        char* right_type = expr_type;
        // Type check
        if (strcmp(left_type, "char*") == 0 && strcmp(right_type, "char*") == 0) {
            if (strcmp(op, "==") == 0) {
                code = concat(concat(concat(concat("strcmp(", code), ", "), right_code), ") == 0");
            } else if (strcmp(op, "!=") == 0) {
                       code = concat(concat(concat(concat("strcmp(", code), ", "), right_code), ") != 0");
                   } else {
                       printf("%s\n", concat(concat(concat("Error: Operator '", op), "' not allowed on strings, line "), itos(line)));
                       return "";
                   }
        } else if (strcmp(left_type, "char*") == 0 || strcmp(right_type, "char*") == 0) {
                   printf("%s\n", concat(concat(concat("Error: Operator '", op), "' not allowed between string and non-string, line "), itos(line)));
                   return "";
               } else {
                   // int/char comparison
                   code = concat(concat(concat(concat(concat(concat("(", code), " "), op), " "), right_code), ")");
               }
        expr_type = "int";
        // Result is always an int
        left_type = "int";
    }
    expr_type = left_type;
    // Set final type
    return code;
}

char* additive() {
    // Handles: expr (+ | -) expr
    // This also handles pointer arithmetic.
    char* code = multiplicative();
    char* left_type = expr_type;
    while (strcmp(peek(), "PLUS") == 0 || strcmp(peek(), "MINUS") == 0) {
        int op_idx = next();
        char* op = op_to_c_op(token_types[op_idx]);
        int line = token_lines[op_idx];
        char* right_code = multiplicative();
        char* right_type = expr_type;
        // Case 1: int + int
        if (strcmp(left_type, "int") == 0 && strcmp(right_type, "int") == 0) {
            expr_type = "int";
            code = concat(concat(concat(concat(code, " "), op), " "), right_code);
        }
        // Case 2: Pointer Arithmetic
        else if (str_ends_with(left_type, '*') && strcmp(right_type, "int") == 0) {
                 expr_type = left_type;
                 // e.g., int* + int = int*
                 code = concat(concat(concat(concat(code, " "), op), " "), right_code);
             } else if (strcmp(left_type, "int") == 0 && str_ends_with(right_type, '*')) {
                 if ((strcmp(op, "+") == 0)) {
                expr_type = right_type;
                // int + int* = int*
                code = concat(concat(concat(concat(code, " "), op), " "), right_code);
            } else {
                printf("%s\n", concat("Error: Cannot subtract a pointer from an integer, line ", itos(line)));
                return "";
            }
             }
             // Case 3: String Concat (char* + char*)
             else if (strcmp(left_type, "char*") == 0 && strcmp(right_type, "char*") == 0 && strcmp(op, "+") == 0) {
                 expr_type = "char*";
                 code = concat(concat(concat(concat("concat(", code), ", "), right_code), ")");
             }
             // Case 4: Error
             else {
                 printf("%s\n", concat(concat(concat(concat(concat(concat(concat("Error: Operator '", op), "' not allowed between '"), left_type), "' and '"), right_type), "', line "), itos(line)));
                 return "";
             }
        left_type = expr_type;
    }
    expr_type = left_type;
    return code;
}

char* multiplicative() {
    // Handles: expr (* | /) expr
    char* code = unary();
    char* left_type = expr_type;
    while (strcmp(peek(), "MUL") == 0 || strcmp(peek(), "DIV") == 0) {
        int op_idx = next();
        char* op = op_to_c_op(token_types[op_idx]);
        char* right_code = unary();
        char* right_type = expr_type;
        if (strcmp(left_type, "int") != 0 || strcmp(right_type, "int") != 0) {
            printf("%s\n", concat("Error: Operators '*' and '/' can only be used on integers, line ", itos(token_lines[op_idx])));
            return "";
        }
        code = concat(concat(concat(concat(code, " "), op), " "), right_code);
        expr_type = "int";
        left_type = "int";
    }
    expr_type = left_type;
    return code;
}

char* unary() {
    // Handles: -expr
    if (strcmp(peek(), "MINUS") == 0) {
        int op_idx = next();
        // Consume '-'
        char* code = unary();
        // Recursive call
        if (strcmp(expr_type, "int") != 0) {
            printf("%s\n", concat("Error: Unary '-' operator can only be applied to integers, line ", itos(token_lines[op_idx])));
            return "";
        }
        expr_type = "int";
        return concat("-", code);
    }
    return atom();
}

char* atom() {
    // Handles: literals, variables, (expr), fn_call(), arr[idx]
    // This is the first function to set the global 'expr_type'.
    int tok_idx = next();
    char* tok_type = token_types[tok_idx];
    int tok_val_idx = token_values[tok_idx];
    int tok_line = token_lines[tok_idx];
    // Case 1: Literals
    if (strcmp(tok_type, "NUMBER") == 0) {
        expr_type = "int";
        return token_pool + tok_val_idx;
    } else if (strcmp(tok_type, "CHAR") == 0) {
             expr_type = "char";
             return token_pool + tok_val_idx;
         } else if (strcmp(tok_type, "STRING") == 0) {
             expr_type = "char*";
             return token_pool + tok_val_idx;
         }
         // Case 2: Parenthesized Expression
         else if (strcmp(tok_type, "LPAREN") == 0) {
             char* code = expr();
             // expr_type is already set by the call above
             expect("RPAREN");
             return concat(concat("(", code), ")");
         }
         // Case 3: Identifier (var, array index, function call)
         else if (strcmp(tok_type, "ID") == 0) {
             char* var_name = token_pool + tok_val_idx;
             // Look for symbol in local, then global scope
             char* sym_type = get_symbol_type(0, var_name);
             if (strcmp(sym_type, "") == 0) {
            printf("%s\n", concat(concat(concat("Error: Undeclared identifier '", var_name), "' on line "), itos(tok_line)));
            return "";
        }
        // Sub-case 3a: Function Call - ID()

             if (strcmp(peek(), "LPAREN") == 0) {
            next();
            expr_type = sym_type;
            // Type is the function's return type
            char* c_code = concat(var_name, "(");
            int arg_count = 0;
            while (strcmp(peek(), "RPAREN") != 0) {
                if (arg_count > 0) {
                    expect("COMMA");
                    c_code = concat(c_code, ", ");
                }
                c_code = concat(c_code, expr());
                arg_count = arg_count + 1;
            }
            expect("RPAREN");
            return concat(c_code, ")");
        }
        // Sub-case 3b: Array Access - ID[]
        else if (strcmp(peek(), "LSQUARE") == 0) {
                 if (str_ends_with(sym_type, '*') == 0) {
                printf("%s\n", concat(concat(concat("Error: Variable '", var_name), "' is not an array and cannot be indexed, line "), itos(tok_line)));
                return "";
            }
                 next();
                 char* idx_code = expr();
                 if (strcmp(expr_type, "int") != 0) {
                printf("%s\n", concat("Error: Array index must be an integer, line ", itos(tok_line)));
                return "";
            }
                 expect("RSQUARE");
                 // Set type to the base type (e.g., "int*" -> "int")
                 // We need a string function for this.
                 // For now, we assume simple types.
                 if (strcmp(sym_type, "int*") == 0) {
                expr_type = "int";
            } else if (strcmp(sym_type, "char*") == 0) {
                     expr_type = "char";
                 } else {
                     expr_type = "int";
                 }
                 // Default assumption
                 return concat(concat(concat(var_name, "["), idx_code), "]");
             }
             // Sub-case 3c: Simple Variable
             else {
                 expr_type = sym_type;
                 return var_name;
             }
         }
         // Case 4: Error
         else {
             printf("%s\n", concat(concat(concat("Error: Unexpected token in expression: ", tok_type), " on line "), itos(tok_line)));
             return "";
         }
}

// =============================================================
// Parser Helpers
// =============================================================
char* peek() {
    // Returns the type of the current token.
    return token_types[parser_pos];
}

int next() {
    // Consumes the current token and returns its index.
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
// Symbol Table Helpers
// =============================================================
int clear_local_symbols() {
    // Clears the local (function-level) symbol table.
    // Called when entering a new function.
    n_locals = 0;
    return 0;
}

char* get_symbol_type(int is_global, char* name) {
    // Searches for a variable 'name' in the given 'scope'.
    // Returns its type (e.g., "int", "char*") if found.
    // Returns "" (empty string) if not found.
    int i = 0;
    if (is_global == 0) {
        while (i < n_locals) {
            if (strcmp(local_names[i], name) == 0) {
                return local_types[i];
            }
            i = i + 1;
        }
    } else {
        // "global"
        while (i < n_globals) {
            if (strcmp(global_names[i], name) == 0) {
                return global_types[i];
            }
            i = i + 1;
        }
    }
    // Not found, check outer scope (if local)
    if (is_global == 0) {
        return get_symbol_type(1, name);
    }
    return "";
    // Not found anywhere
}

int add_symbol(int is_global, char* name, char* type) {
    // Adds a new variable to the symbol table.
    // Returns 0 on success.
    // NOTE: This function assumes you have already checked for redefinition.
    if (is_global == 0) {
        local_names[n_locals] = name;
        local_types[n_locals] = type;
        n_locals = n_locals + 1;
    } else {
        global_names[n_globals] = name;
        global_types[n_globals] = type;
        n_globals = n_globals + 1;
    }
    return 0;
}

// =============================================================
// Parser Utils
// =============================================================
int str_ends_with(char* s, char c) {
    // Checks if string 's' ends with character 'c'.
    // Returns 1 (true) or 0 (false).
    int len = strlen(s);
    if (len == 0) {
        return 0;
        // Empty string
    }
    if (s[len - 1] == c) {
        return 1;
    }
    return 0;
}

char* op_to_c_op(char* tok_type) {
    // Translates a token type (e.g., "PLUS") to its C operator (e.g., "+").
    if (strcmp(tok_type, "PLUS") == 0) {
        return "+";
    }
    if (strcmp(tok_type, "MINUS") == 0) {
        return "-";
    }
    if (strcmp(tok_type, "MUL") == 0) {
        return "*";
    }
    if (strcmp(tok_type, "DIV") == 0) {
        return "/";
    }
    if (strcmp(tok_type, "EQ") == 0) {
        return "==";
    }
    if (strcmp(tok_type, "NE") == 0) {
        return "!=";
    }
    if (strcmp(tok_type, "LT") == 0) {
        return "<";
    }
    if (strcmp(tok_type, "GT") == 0) {
        return ">";
    }
    if (strcmp(tok_type, "LE") == 0) {
        return "<=";
    }
    if (strcmp(tok_type, "GE") == 0) {
        return ">=";
    }
    if (strcmp(tok_type, "AND") == 0) {
        return "&&";
    }
    if (strcmp(tok_type, "OR") == 0) {
        return "||";
    }
    // Should never happen, but good to have a default.

    return "";
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

