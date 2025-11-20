#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* concat(char* str1, char* str2);
char* itos(int x);
char* ctos(char c);
char* read_file(char* path);
void write_file(char* path, char* content);

// File: compiler.dav
// Author: David T.
// Description: Stage 1 compiler for the dav language.
// This file is compiled by the Stage 0 Python compiler.
// =============================================================
// Global Storage
// =============================================================
// --- Tokenizer Storage ---
char* token_types[50000];
int token_values[50000];
// Stores index into token_pool, or -1
int token_lines[50000];
int token_cols[50000];
char token_pool[500000];
// String pool for token values
int n_tokens = 0;
// Total number of tokens found
// --- Parser State ---
int parser_pos = 0;
// Current token index for the parser
char* current_fn_ret_type;
// Stores return type of fn being parsed
char* expr_type;
// Type of the last parsed expression, works like a forgetful stack
// --- Symbol Table Storage ---
// We store 'char*' pointers for names and types.
// The Stage 0 compiler will get the name strings from the token_pool.
// The type strings will be string literals (e.g., "int", "char*").
// Global Scope (self.env)
char* global_names[1000];
char* global_types[1000];
int n_globals = 0;
// Local Scope (self.variables)
char* local_names[1000];
char* local_types[1000];
int n_locals = 0;
// --- C Code Generation Buffer ---
char c_code_buffer[1000000];
// 1MB buffer for generated C
int c_code_pos = 0;
// Current position in the buffer
// --- Peek Buffer ---
char expr_peek_buffer[4096];
// Scratchpad for peeking code
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
int parse();
int global_decl();
int fn_decl();
int statement();
int let_stmt(int is_global);
int print_stmt();
int if_stmt();
int while_stmt();
int return_stmt();
int id_stmt();
int expr();
int logical();
int relational();
int additive();
int multiplicative();
int unary();
int atom();
char* peek();
int next();
int expect(char* kind);
int clear_local_symbols();
char* get_symbol_type(int is_global, char* name);
int add_symbol(int is_global, char* name, char* type);
int str_ends_with(char* s, char c);
char* op_to_c_op(char* tok_type);
int emit(char* s);
char* peek_code(char* level);
int c_include();
int c_prototype();
int c_helper();
int preset_global_functions();
// =============================================================
// Main Entry Point
// =============================================================
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("%s\n", "Usage: compiler <input_file.dav> <output_file.c>");
        return 1;
    }
    char* input_file = argv[1];
    char* output_file = argv[2];
    // 1. Read Input File
    char* code = read_file(input_file);
    if (code == 0) {
        // NULL check
        printf("%s\n", "Error: Could not read input file.");
        return 1;
    }
    // 2. Setup Code Generation

    c_include();
    c_prototype();
    preset_global_functions();
    // 3. Tokenize
    tokenize(code);
    // 4. Parse
    parse();
    // 5. Emit Helpers
    c_helper();
    // 6. Write Output File
    write_file(output_file, c_code_buffer);
    // boo("Done.");
    return 0;
}

// =============================================================
// Parser
// =============================================================
int parse() {
    // Main parser entry point.
    // Loops until EOF, parsing all global declarations.
    while (strcmp(peek(), "EOF") != 0) {
        global_decl();
    }
    return 0;
}

int global_decl() {
    // Dispatches to the correct parser function
    // based on the next token.
    char* tok = peek();
    if (strcmp(tok, "FN") == 0) {
        fn_decl();
    } else if (strcmp(tok, "LET") == 0) {
               let_stmt(1);
               // 1 for global
           } else {
               // Error handling
               int tok_line = token_lines[parser_pos];
               printf("%s\n", concat("Error: Unexpected global token on line ", itos(tok_line)));
               printf("%s\n", concat("Expected FN, LET, or COMMENT, but got: ", tok));
               // Consume the bad token to prevent infinite loop
               next();
               return -1;
           }
    return 0;
}

int fn_decl() {
    // Parses a function declaration or definition
    int fn_tok_idx = expect("FN");
    int line_num = token_lines[fn_tok_idx];
    // --- Get Type ---
    char* fn_type = "void";
    // Default type
    if (strcmp(peek(), "TYPE") == 0) {
        int fn_type_idx = next();
        fn_type = token_pool + token_values[fn_type_idx];
    }
    // --- Get Pointer ---

    if (strcmp(peek(), "MUL") == 0) {
        next();
        if (strcmp(fn_type, "int") == 0) {
            fn_type = "int*";
        } else if (strcmp(fn_type, "char") == 0) {
                 fn_type = "char*";
             } else if (strcmp(fn_type, "char*") == 0) {
                 fn_type = "char**";
             } else {
                 printf("%s\n", concat("Error: Cannot make array of type ", fn_type));
                 return -1;
             }
    }
    // --- Get Name ---

    int fn_name_idx = expect("ID");
    char* fn_name = token_pool + token_values[fn_name_idx];
    // --- Store for type-checking 'return' ---
    current_fn_ret_type = fn_type;
    add_symbol(1, fn_name, fn_type);
    expect("LPAREN");
    emit(fn_type);
    emit(" ");
    emit(fn_name);
    emit("(");
    // --- Parse parameters ---
    // Parallel arrays to store parameters
    char* param_types[20];
    char* param_names[20];
    int param_has_arrays_part[20];
    int n_params = 0;
    int i = 0;
    while (strcmp(peek(), "RPAREN") != 0) {
        if (n_params > 0) {
            expect("COMMA");
            emit(", ");
        }
        // Get param type (default int)

        char* param_type = "int";
        if (strcmp(peek(), "TYPE") == 0) {
            int param_type_idx = next();
            param_type = token_pool + token_values[param_type_idx];
        }
        // Get param pointer

        if (strcmp(peek(), "MUL") == 0) {
            next();
            if (strcmp(param_type, "int") == 0) {
                param_type = "int*";
            } else if (strcmp(param_type, "char") == 0) {
                     param_type = "char*";
                 } else if (strcmp(param_type, "char*") == 0) {
                     param_type = "char**";
                 } else {
                     printf("%s\n", concat("Error: Cannot make array of type ", param_type));
                     return -1;
                 }
        }
        // Get param name

        int param_name_idx = expect("ID");
        char* param_name = token_pool + token_values[param_name_idx];
        emit(param_type);
        emit(" ");
        emit(param_name);
        // Check for array param part
        int param_array_part = 0;
        if (strcmp(peek(), "LSQUARE") == 0) {
            next();
            param_array_part = 1;
            if (strcmp(peek(), "NUMBER") == 0) {
                int size_idx = next();
                char* size_str = token_pool + token_values[size_idx];
                emit("[");
                emit(size_str);
                emit("]");
            } else {
                emit("[]");
            }
            expect("RSQUARE");
        }
        // Store param

        param_types[n_params] = param_type;
        param_names[n_params] = param_name;
        param_has_arrays_part[n_params] = param_array_part;
        n_params = n_params + 1;
    }
    expect("RPAREN");
    emit(")");
    // --- Check for Prototype (;) or Definition ({) ---
    if (strcmp(peek(), "SEMICOL") == 0) {
        // Function Declaration (Prototype)
        next();
        emit(";\n");
        return 0;
    } else if (strcmp(peek(), "LBRACE") == 0) {
             // Function Definition
             next();
             emit(" {\n");
             // --- Setup local scope ---
             clear_local_symbols();
             i = 0;
             while (i < n_params) {
            char* var_type = param_types[i];
            if (param_has_arrays_part[i] == 1) {
                if (strcmp(var_type, "int") == 0) {
                    var_type = "int*";
                } else if (strcmp(var_type, "char") == 0) {
                         var_type = "char*";
                     } else if (strcmp(var_type, "char*") == 0) {
                         var_type = "char**";
                     } else {
                         printf("%s\n", concat("Error: Cannot make array of type ", var_type));
                     }
            }
            add_symbol(0, param_names[i], var_type);
            i = i + 1;
        }
             // --- Parse function body ---
             while (strcmp(peek(), "RBRACE") != 0 && strcmp(peek(), "EOF") != 0) {
            statement();
        }
             expect("RBRACE");
             emit("}\n");
             return 0;
         } else {
             printf("%s\n", concat("Error: Expected ';' or '{' after function signature, line ", itos(line_num)));
             return -1;
         }
}

int statement() {
    // Dispatches to the correct statement parser.
    char* tok = peek();
    if (strcmp(tok, "LET") == 0) {
        let_stmt(0);
        // 0 for local
    } else if (strcmp(tok, "PRINT") == 0) {
               print_stmt();
           } else if (strcmp(tok, "IF") == 0) {
               if_stmt();
           } else if (strcmp(tok, "WHILE") == 0) {
               while_stmt();
           } else if (strcmp(tok, "RETURN") == 0) {
               return_stmt();
           } else if (strcmp(tok, "ID") == 0) {
               id_stmt();
           } else {
               printf("%s\n", concat(concat(concat("Error: Unexpected statement: ", tok), " on line "), itos(token_lines[parser_pos])));
               next();
               // Consume bad token
               return -1;
           }
    return 0;
}

int let_stmt(int is_global) {
    int line_num = token_lines[parser_pos];
    expect("LET");
    // --- Get Type ---
    char* var_type = "undefined";
    // Unspecified type
    if (strcmp(peek(), "TYPE") == 0) {
        int var_type_idx = next();
        var_type = token_pool + token_values[var_type_idx];
    }
    // --- Get Pointer ---

    if (strcmp(peek(), "MUL") == 0) {
        next();
        if (strcmp(var_type, "int") == 0) {
            var_type = "int*";
        } else if (strcmp(var_type, "char") == 0) {
                 var_type = "char*";
             } else {
                 printf("%s\n", concat("Error: Cannot make array of type ", var_type));
                 return -1;
             }
    }
    // --- Get Name ---

    int var_name_idx = expect("ID");
    char* var_name = token_pool + token_values[var_name_idx];
    // Check redefinition
    if ((is_global == 0 && strcmp(get_symbol_type(0, var_name), "") != 0) || (is_global == 1 && strcmp(get_symbol_type(1, var_name), "") != 0)) {
        printf("%s\n", concat(concat(concat("Error: Redefinition of variable ", var_name), ", line "), itos(line_num)));
        return -1;
        // Error
    }
    // --- Parsing Cases ---

    if (strcmp(peek(), "ASSIGN") == 0) {
        // --- Case 1: Declaration with Assignment (e.g., beg x = 10) ---
        next();
        emit(var_type);
        emit(" ");
        emit(var_name);
        emit(" = ");
        expr();
        // This emits the C code for the RHS
        emit(";\n");
        char* right_type = expr_type;
        // TODO: inference not working, come back to fix me please
        if (strcmp(var_type, "undefined") == 0) {
            var_type = right_type;
            // Infer type
        } else if (strcmp(var_type, right_type) != 0) {
                   printf("%s\n", concat(concat(concat(concat(concat("Error: Incompatible type ", right_type), " to "), var_type), ", line "), itos(line_num)));
                   return -1;
               }
        expect("SEMICOL");
        add_symbol(is_global, var_name, var_type);
        return 0;
    } else if (strcmp(peek(), "LSQUARE") == 0) {
             // --- Case 2: Array Declaration (e.g., beg int arr[10]) ---
             next();
             if (strcmp(var_type, "undefined") == 0) {
            printf("%s\n", concat("Error: Array declaration must have an explicit type on line", itos(line_num)));
            return -1;
        }
             int size_tok = expect("NUMBER");
             char* size = token_pool + token_values[size_tok];
             expect("RSQUARE");
             expect("SEMICOL");
             // Store array type as 'base_type*' (e.g., 'int*')
             char* array_type = "int*";
             // Default
             if (strcmp(var_type, "int") == 0) {
            array_type = "int*";
        } else if (strcmp(var_type, "char") == 0) {
                 array_type = "char*";
             } else if (strcmp(var_type, "char*") == 0) {
                 array_type = "char**";
             } else {
                 printf("%s\n", concat("Error: Cannot make array of type ", var_type));
                 return -1;
             }
             add_symbol(is_global, var_name, array_type);
             // C code: e.g., "int arr[10];"
             emit(var_type);
             emit(" ");
             emit(var_name);
             emit("[");
             emit(size);
             emit("];\n");
             return 0;
         } else if (strcmp(peek(), "SEMICOL") == 0) {
             // --- Case 3: Declaration without Assignment (e.g., beg int x;) ---
             next();
             if (strcmp(var_type, "undefined") == 0) {
            printf("%s\n", concat("Error: Declaration without assignment must have explicit type on line", itos(line_num)));
            return -1;
        }
             add_symbol(is_global, var_name, var_type);
             emit(var_type);
             emit(" ");
             emit(var_name);
             emit(";\n");
             return 0;
         } else {
             printf("%s\n", concat("Error: Expected '=', '[', or ';' after variable name on line", itos(line_num)));
             next();
             // Consume bad token
             return -1;
         }
}

int print_stmt() {
    int line_num = token_lines[parser_pos];
    expect("PRINT");
    expect("LPAREN");
    // Peek the code for the expression to determine its type
    // Level "expr" calls the top-level expr() parser
    char* expr_code = peek_code("expr");
    char* type = expr_type;
    // peek_code sets this global
    if (strcmp(type, "int") == 0) {
        emit("printf(\"%d\\n\", ");
    } else if (strcmp(type, "char") == 0) {
               emit("printf(\"%c\\n\", ");
           } else if (strcmp(type, "char*") == 0) {
               emit("printf(\"%s\\n\", ");
           } else {
               printf("%s\n", concat(concat(concat("Error: Unprintable type '", type), "' on line "), itos(line_num)));
               return -1;
           }
    // Now emit the code we peeked
    emit(expr_code);
    emit(");\n");
    expect("RPAREN");
    expect("SEMICOL");
    return 0;
}

int id_stmt() {
    // Handles statements beginning with an identifier:
    // 1. x = 10;        (Assignment)
    // 2. my_func(10);   (Function Call)
    // 3. arr[0] = 5;    (Array Assignment)
    int tok_idx = next();
    int line_num = token_lines[tok_idx];
    char* var_name = token_pool + token_values[tok_idx];
    // Get variable from local/global scope
    char* var_type = get_symbol_type(0, var_name);
    // Declare this here since this compiler can't handle
    // sub-function (if/while body) scoped declarations
    char* right_type;
    if (strcmp(var_type, "") == 0) {
        printf("%s\n", concat(concat(concat("Error: Undeclared identifier '", var_name), "' on line "), itos(line_num)));
        return -1;
    }
    // --- Case 1: Variable Assignment ---

    if (strcmp(peek(), "ASSIGN") == 0) {
        next();
        emit(var_name);
        emit(" = ");
        expr();
        // Emits RHS
        emit(";\n");
        // Type check
        right_type = expr_type;
        if (strcmp(var_type, right_type) != 0) {
            printf("%s\n", concat(concat(concat(concat(concat("Error: Incompatible ", right_type), " to "), var_type), " conversion on line "), itos(line_num)));
            return -1;
        }
        expect("SEMICOL");
        return 0;
    }
    // --- Case 2: Function Call ---
    else if (strcmp(peek(), "LPAREN") == 0) {
             next();
             // TODO: Check if var_type is a function type
             // For now, we assume if it's not an assignment, it's a function call.
             emit(var_name);
             emit("(");
             int arg_count = 0;
             while (strcmp(peek(), "RPAREN") != 0) {
            if (arg_count > 0) {
                expect("COMMA");
                emit(", ");
            }
            expr();
            // Emits argument
            arg_count = arg_count + 1;
        }
             expect("RPAREN");
             expect("SEMICOL");
             emit(");\n");
             return 0;
         }
         // --- Case 3: Array Assignment ---
         else if (strcmp(peek(), "LSQUARE") == 0) {
             next();
             // Check if var_type is a pointer
             if (str_ends_with(var_type, '*') == 0) {
            printf("%s\n", concat(concat(concat("Error: Variable '", var_name), "' is not an array and cannot be indexed, line "), itos(line_num)));
            return -1;
        }
             emit(var_name);
             emit("[");
             expr();
             // Emits index
             emit("] = ");
             if (strcmp(expr_type, "int") != 0) {
            printf("%s\n", concat(concat(concat("Error: Array index must be an integer, got ", expr_type), ", line "), itos(line_num)));
            return -1;
        }
             expect("RSQUARE");
             expect("ASSIGN");
             expr();
             // Emits RHS
             emit(";\n");
             // Type check
             right_type = expr_type;
             char* base_type = "int";
             // Default to int
             if (strcmp(var_type, "int*") == 0) {
            base_type = "int";
        } else if (strcmp(var_type, "char*") == 0) {
                 base_type = "char";
             } else if (strcmp(var_type, "char**") == 0) {
                 base_type = "char*";
             }
             if (strcmp(base_type, right_type) != 0) {
            printf("%s\n", concat(concat(concat(concat(concat("Error: Incompatible types: cannot assign ", right_type), " to array element of type "), base_type), ", line "), itos(line_num)));
            return -1;
        }
             expect("SEMICOL");
             return 0;
         }
         // --- Case 4: Error ---
         else {
             printf("%s\n", concat(concat(concat("Error: Invalid statement start. Expected '=', '(', or '[' after ID '", var_name), "', line "), itos(line_num)));
             return -1;
         }
}

int if_stmt() {
    expect("IF");
    emit("if (");
    expr();
    // Emit condition
    emit(") {\n");
    expect("LBRACE");
    while (strcmp(peek(), "RBRACE") != 0 && strcmp(peek(), "EOF") != 0) {
        statement();
    }
    expect("RBRACE");
    emit("}\n");
    // Handle else
    if (strcmp(peek(), "ELSE") == 0) {
        next();
        emit("else ");
        // Case 1: else-if
        if (strcmp(peek(), "IF") == 0) {
            if_stmt();
        }
        // Case 2: else
        else if (strcmp(peek(), "LBRACE") == 0) {
                 next();
                 emit("{\n");
                 while (strcmp(peek(), "RBRACE") != 0 && strcmp(peek(), "EOF") != 0) {
                statement();
            }
                 expect("RBRACE");
                 emit("}\n");
             }
             // Case 3: Error
             else {
                 int tok_line = token_lines[parser_pos];
                 printf("%s\n", concat("Error: Expected 'if' or '{' after 'else', line ", itos(tok_line)));
                 return -1;
             }
    }
    return 0;
}

int while_stmt() {
    expect("WHILE");
    emit("while (");
    expr();
    emit(") {\n");
    expect("LBRACE");
    while (strcmp(peek(), "RBRACE") != 0 && strcmp(peek(), "EOF") != 0) {
        statement();
    }
    expect("RBRACE");
    emit("}\n");
    return 0;
}

int return_stmt() {
    int line_num = token_lines[parser_pos];
    expect("RETURN");
    emit("return ");
    expr();
    // Emit expression
    emit(";\n");
    char* ret_type = expr_type;
    expect("SEMICOL");
    if (strcmp(current_fn_ret_type, ret_type) != 0) {
        printf("%s\n", concat(concat(concat(concat(concat("Error: Incompatible ", ret_type), " to "), current_fn_ret_type), " conversion on line "), itos(line_num)));
        return -1;
    }
    return 0;
}

// =============================================================
// Expression Parsers
// =============================================================
int expr() {
    // Main entry point for parsing an expression.
    // Emits C code. Sets global 'expr_type'.
    return logical();
}

int logical() {
    // Handles: expr (&& | ||) expr
    relational();
    // Emits left side
    char* left_type = expr_type;
    while (strcmp(peek(), "OR") == 0 || strcmp(peek(), "AND") == 0) {
        int op_idx = next();
        char* op = op_to_c_op(token_types[op_idx]);
        emit(" ");
        emit(op);
        emit(" ");
        relational();
        // Emits right side
        char* right_type = expr_type;
        // Type check: logical ops must be on ints (or chars)
        if (strcmp(left_type, "int") != 0 || strcmp(right_type, "int") != 0) {
            printf("%s\n", concat("Error: Logical operators '&&' and '||' can only be used on integers, line ", itos(token_lines[op_idx])));
            return -1;
        }
        expr_type = "int";
        // Result is always an int
        left_type = "int";
    }
    expr_type = left_type;
    // Set final type
    return 0;
}

int relational() {
    // Handles: expr (== | != | < | > | <= | >=) expr
    // 1. Peek LHS (using additive parser)
    char* left_ptr = peek_code("additive");
    char* left_type = expr_type;
    // Need local copy because peek_code buffer will be overwritten
    char left_buf[2048];
    int i = 0;
    while (left_ptr[i] != '\0') {
        left_buf[i] = left_ptr[i];
        i = i + 1;
    }
    left_buf[i] = '\0';
    if (strcmp(peek(), "EQ") == 0 || strcmp(peek(), "NE") == 0 || strcmp(peek(), "LT") == 0 || strcmp(peek(), "GT") == 0 || strcmp(peek(), "LE") == 0 || strcmp(peek(), "GE") == 0) {
        while (strcmp(peek(), "EQ") == 0 || strcmp(peek(), "NE") == 0 || strcmp(peek(), "LT") == 0 || strcmp(peek(), "GT") == 0 || strcmp(peek(), "LE") == 0 || strcmp(peek(), "GE") == 0) {
            int op_idx = next();
            char* op = op_to_c_op(token_types[op_idx]);
            int line = token_lines[op_idx];
            // 2. Peek RHS
            char* right_code = peek_code("additive");
            char* right_type = expr_type;
            // 3. Generate Code
            if (strcmp(left_type, "char*") == 0 && strcmp(right_type, "char*") == 0) {
                if (strcmp(op, "==") == 0) {
                    emit("strcmp(");
                    emit(left_buf);
                    emit(", ");
                    emit(right_code);
                    emit(") == 0");
                } else if (strcmp(op, "!=") == 0) {
                           emit("strcmp(");
                           emit(left_buf);
                           emit(", ");
                           emit(right_code);
                           emit(") != 0");
                       } else {
                           printf("%s\n", concat(concat(concat("Error: Operator '", op), "' not allowed on strings, line "), itos(line)));
                           return -1;
                       }
            } else if ((strcmp(left_type, "char*") == 0 && strcmp(right_type, "int") == 0) || (strcmp(left_type, "int") == 0 && strcmp(right_type, "char*") == 0)) {
                       if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) {
                    emit(left_buf);
                    emit(" ");
                    emit(op);
                    emit(" ");
                    emit(right_code);
                } else {
                    printf("%s\n", concat(concat(concat("Error: Operator '", op), "' not allowed on strings, line "), itos(line)));
                    return -1;
                }
                   } else if (strcmp(left_type, "char*") == 0 || strcmp(right_type, "char*") == 0) {
                       printf("%s\n", concat("Error: Comparison between string and non-string, line ", itos(line)));
                       return -1;
                   } else {
                       // Standard int/char
                       emit(left_buf);
                       emit(" ");
                       emit(op);
                       emit(" ");
                       emit(right_code);
                   }
            expr_type = "int";
            left_type = "int";
        }
    } else {
        // No operators, just emit LHS
        emit(left_buf);
        expr_type = left_type;
        // Restore type
    }
    return 0;
}

int additive() {
    // Handles: expr (+ | -) expr
    // This also handles pointer arithmetic.
    // 1. Peek LHS (using multiplicative parser)
    char* left_ptr = peek_code("multiplicative");
    char* left_type = expr_type;
    // Need local copy because peek_code buffer will be overwritten
    char left_buf[2048];
    int i = 0;
    while ((left_ptr[i] != '\0')) {
        left_buf[i] = left_ptr[i];
        i = i + 1;
    }
    left_buf[i] = '\0';
    if (strcmp(peek(), "PLUS") == 0 || strcmp(peek(), "MINUS") == 0) {
        while (strcmp(peek(), "PLUS") == 0 || strcmp(peek(), "MINUS") == 0) {
            int op_idx = next();
            char* op = op_to_c_op(token_types[op_idx]);
            int line = token_lines[op_idx];
            // 2. Peek RHS
            char* right_code = peek_code("multiplicative");
            char* right_type = expr_type;
            // 3. Generate Code
            // Case 1: int + int
            if (strcmp(left_type, "int") == 0 && strcmp(right_type, "int") == 0) {
                emit(left_buf);
                emit(" ");
                emit(op);
                emit(" ");
                emit(right_code);
                expr_type = "int";
            }
            // Case 2: Pointer Arithmetic
            else if (str_ends_with(left_type, '*') && strcmp(right_type, "int") == 0) {
                     emit(left_buf);
                     emit(" ");
                     emit(op);
                     emit(" ");
                     emit(right_code);
                     expr_type = left_type;
                     // e.g., int* + int = int*
                 } else if (strcmp(left_type, "int") == 0 && str_ends_with(right_type, '*')) {
                     if (strcmp(op, "+") == 0) {
                    emit(left_buf);
                    emit(op);
                    emit(right_code);
                    expr_type = right_type;
                    // int + int* = int*
                } else {
                    printf("%s\n", concat("Error: Cannot subtract a pointer from an integer, line ", itos(line)));
                    return -1;
                }
                 }
                 // Case 3: String Concat (char* + char*)
                 else if (strcmp(left_type, "char*") == 0 && strcmp(right_type, "char*") == 0 && strcmp(op, "+") == 0) {
                     emit("concat(");
                     emit(left_buf);
                     emit(", ");
                     emit(right_code);
                     emit(")");
                     expr_type = "char*";
                 }
                 // Case 4: Error
                 else {
                     printf("%s\n", concat(concat(concat(concat(concat(concat(concat("Error: Operator '", op), "' not allowed between '"), left_type), "' and '"), right_type), "', line "), itos(line)));
                     return -1;
                 }
            left_type = expr_type;
        }
    } else {
        // No operators, just emit LHS
        emit(left_buf);
        expr_type = left_type;
        // Restore type
    }
    expr_type = left_type;
    return 0;
}

int multiplicative() {
    // Handles: expr (* | /) expr
    unary();
    char* left_type = expr_type;
    while (strcmp(peek(), "MUL") == 0 || strcmp(peek(), "DIV") == 0) {
        int op_idx = next();
        char* op = op_to_c_op(token_types[op_idx]);
        emit(" ");
        emit(op);
        emit(" ");
        unary();
        char* right_type = expr_type;
        if (strcmp(left_type, "int") != 0 || strcmp(right_type, "int") != 0) {
            printf("%s\n", concat("Error: Operators '*' and '/' can only be used on integers, line ", itos(token_lines[op_idx])));
            return -1;
        }
        expr_type = "int";
        left_type = "int";
    }
    expr_type = left_type;
    return 0;
}

int unary() {
    // Handles: -expr
    if (strcmp(peek(), "MINUS") == 0) {
        int op_idx = next();
        emit("-");
        unary();
        // Recursive call
        if (strcmp(expr_type, "int") != 0) {
            printf("%s\n", concat("Error: Unary '-' operator can only be applied to integers, line ", itos(token_lines[op_idx])));
            return -1;
        }
        expr_type = "int";
        return 0;
    }
    return atom();
}

int atom() {
    // Handles: literals, variables, (expr), fn_call(), arr[idx]
    // This is the first function to set the global 'expr_type'.
    int tok_idx = next();
    char* tok_type = token_types[tok_idx];
    int tok_val_idx = token_values[tok_idx];
    int tok_line = token_lines[tok_idx];
    // Case 1: Literals
    if (strcmp(tok_type, "NUMBER") == 0) {
        expr_type = "int";
        emit(token_pool + tok_val_idx);
    } else if (strcmp(tok_type, "CHAR") == 0) {
             expr_type = "char";
             emit(token_pool + tok_val_idx);
         } else if (strcmp(tok_type, "STRING") == 0) {
             expr_type = "char*";
             emit("\"");
             emit(token_pool + tok_val_idx);
             emit("\"");
         }
         // Case 2: Parenthesized Expression
         else if (strcmp(tok_type, "LPAREN") == 0) {
             emit("(");
             expr();
             emit(")");
             expect("RPAREN");
         }
         // Case 3: Identifier (var, array index, function call)
         else if (strcmp(tok_type, "ID") == 0) {
             char* var_name = token_pool + tok_val_idx;
             // Look for symbol in local, then global scope
             char* sym_type = get_symbol_type(0, var_name);
             if (strcmp(sym_type, "") == 0) {
            printf("%s\n", concat(concat(concat("Error: Undeclared identifier '", var_name), "' on line "), itos(tok_line)));
            return -1;
        }
        // Sub-case 3a: Function Call - ID()

             if (strcmp(peek(), "LPAREN") == 0) {
            next();
            emit(var_name);
            emit("(");
            int arg_count = 0;
            while (strcmp(peek(), "RPAREN") != 0) {
                if (arg_count > 0) {
                    expect("COMMA");
                    emit(", ");
                }
                expr();
                arg_count = arg_count + 1;
            }
            expr_type = sym_type;
            // Type is the function's return type
            expect("RPAREN");
            emit(")");
        }
        // Sub-case 3b: Array Access - ID[]
        else if (strcmp(peek(), "LSQUARE") == 0) {
                 if (str_ends_with(sym_type, '*') == 0) {
                printf("%s\n", concat(concat(concat("Error: Variable '", var_name), "' is not an array and cannot be indexed, line "), itos(tok_line)));
                return -1;
            }
                 next();
                 emit(var_name);
                 emit("[");
                 expr();
                 if (strcmp(expr_type, "int") != 0) {
                printf("%s\n", concat("Error: Array index must be an integer, line ", itos(tok_line)));
                return -1;
            }
                 expect("RSQUARE");
                 emit("]");
                 // Set type to the base type (e.g., "int*" -> "int")
                 // TODO: We need a string function for this.
                 // For now, we assume simple types.
                 if (strcmp(sym_type, "int*") == 0) {
                expr_type = "int";
            } else if (strcmp(sym_type, "char*") == 0) {
                     expr_type = "char";
                 } else if (strcmp(sym_type, "char**") == 0) {
                     expr_type = "char*";
                 } else {
                     expr_type = "int";
                 }
                 // Default assumption
             }
             // Sub-case 3c: Simple Variable
             else {
                 expr_type = sym_type;
                 emit(var_name);
             }
         }
         // Case 4: Error
         else {
             printf("%s\n", concat(concat(concat("Error: Unexpected token in expression: ", tok_type), " on line "), itos(tok_line)));
             return -1;
         }
    return 0;
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

int emit(char* s) {
    // Appends a string 's' to the global c_code_buffer.
    int i = 0;
    int len = strlen(s);
    // --- Bounds check ---
    if (c_code_pos + len >= 1000000) {
        printf("%s\n", "CRITICAL ERROR: C code output buffer overflow! Increase c_code_buffer size.");
        return -1;
        // This will likely cascade errors, but it prints the warning.
    }
    while (i < len) {
        c_code_buffer[c_code_pos] = s[i];
        c_code_pos = c_code_pos + 1;
        i = i + 1;
    }
    c_code_buffer[c_code_pos] = '\0';
    // Keep buffer null-terminated
    return 0;
}

char* peek_code(char* level) {
    int start_pos = c_code_pos;
    if ((strcmp(level, "expr") == 0)) {
        expr();
    } else if ((strcmp(level, "logical") == 0)) {
             logical();
         } else if ((strcmp(level, "relational") == 0)) {
             relational();
         } else if ((strcmp(level, "additive") == 0)) {
             additive();
         } else if ((strcmp(level, "multiplicative") == 0)) {
             multiplicative();
         } else if ((strcmp(level, "unary") == 0)) {
             unary();
         } else if ((strcmp(level, "atom") == 0)) {
             atom();
         } else {
             printf("%s\n", concat("Error: Unknown peek level: ", level));
             return "";
         }
    int end_pos = c_code_pos;
    int len = end_pos - start_pos;
    if (len >= 4096) {
        printf("%s\n", "Error: Expression too complex to peek (max 4096 chars).");
        return "";
    }
    int i = 0;
    while (i < len) {
        expr_peek_buffer[i] = c_code_buffer[start_pos + i];
        i = i + 1;
    }
    expr_peek_buffer[i] = '\0';
    // Rewind
    c_code_pos = start_pos;
    return expr_peek_buffer;
}

int c_include() {
    // Emit C include
    emit("#include <stdio.h>\n");
    emit("#include <stdlib.h>\n");
    emit("#include <string.h>\n\n");
    return 0;
}

int c_prototype() {
    // Emit C prototype
    emit("char* concat(char* str1, char* str2);\n");
    emit("char* itos(int x);\n");
    emit("char* ctos(char c);\n\n");
    emit("char* read_file(char* path);\n");
    emit("void write_file(char* path, char* content);\n");
    return 0;
}

int c_helper() {
    // Emit C helper
    emit("\nchar* concat(char* str1, char* str2) {\n");
    emit("static char buf[1024];\n");
    emit("snprintf(buf, sizeof(buf), \"%s%s\", str1, str2);\n");
    emit("return buf;\n}\n\n");
    emit("char* itos(int x) {\n");
    emit("static char buf[32];\n");
    emit("snprintf(buf, sizeof(buf), \"%d\", x);\n");
    emit("return buf;\n}\n\n");
    emit("char* ctos(char c) {\n");
    emit("static char buf[2];\n");
    emit("buf[0] = c;\n");
    emit("buf[1] = '\\0';\n");
    emit("return buf;\n}\n\n");
    emit("char* read_file(char* path) {\n");
    emit("FILE* f = fopen(path, \"rb\");\n");
    emit("if (!f) return NULL;\n");
    emit("fseek(f, 0, SEEK_END);\n");
    emit("long len = ftell(f);\n");
    emit("fseek(f, 0, SEEK_SET);\n");
    emit("char* buf = malloc(len + 1);\n");
    emit("fread(buf, 1, len, f);\n");
    emit("buf[len] = '\\0';\n");
    emit("fclose(f);\n");
    emit("return buf;\n}\n\n");
    emit("void write_file(char* path, char* content) {\n");
    emit("FILE* f = fopen(path, \"w\");\n");
    emit("if (!f) return;\n");
    emit("fprintf(f, \"%s\", content);\n");
    emit("fclose(f);\n}\n");
    return 0;
}

int preset_global_functions() {
    // Preset global scope with util functions
    add_symbol(1, "concat", "char*");
    add_symbol(1, "ctos", "char*");
    add_symbol(1, "itos", "char*");
    add_symbol(1, "strlen", "int");
    add_symbol(1, "strcmp", "int");
    add_symbol(1, "read_file", "char*");
    add_symbol(1, "write_file", "void");
    return 0;
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
        // --- Bounds check ---
        if (token_count >= 50000) {
            printf("%s\n", "CRITICAL ERROR: Too many tokens! Increase token array sizes.");
            return 0;
        }
        if (pool_pos >= 499000) {
            // Leave some safety margin
            printf("%s\n", "CRITICAL ERROR: String pool overflow! Increase token_pool size.");
            return 0;
        }
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
                // TODO: Maybe add comments
                // add_simple_token(token_count, "COMMENT", line_num, col);
                // token_count = token_count + 1; pos = pos + 2;
                // Loop to skip till after newline or EOL
                while (source_code[pos] != '\n' && source_code[pos] != '\n') {
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
                    buffer[i] = '\\';
                    i = i + 1;
                    pos = pos + 1;
                    c = source_code[pos];
                    if (c == 'n') {
                        buffer[i] = 'n';
                    } else if (c == 't') {
                               buffer[i] = 't';
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
                 while (j <= i) {
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
                 printf("%s\n", concat("Error: Unexpected character!", ctos(c)));
                 return 1;
             }
    }
    // Add EOF Token
    add_simple_token(token_count, "EOF", line_num, col);
    token_count = token_count + 1;
    n_tokens = token_count;
    return 0;
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
    return c >= '0' && c <= '9';
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

int add_simple_token(int index, char* type, int line, int col) {
    // Helper to add a simple token (without a value) to the token arrays.
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

char* read_file(char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    if (buf) {
        fread(buf, 1, len, f);
        buf[len] = '\0';
    }
    fclose(f);
    return buf;
}

void write_file(char* path, char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "%s", content);
    fclose(f);
}

