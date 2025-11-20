#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* concat(char* str1, char* str2);
char* itos(int x);
char* ctos(char c);

char* read_file(char* path);
void write_file(char* path, char* content);
char* token_types[50000];
int token_values[50000];
int token_lines[50000];
int token_cols[50000];
char token_pool[500000];
int n_tokens = 0;
int parser_pos = 0;
char* current_fn_ret_type;
char* expr_type;
char* global_names[1000];
char* global_types[1000];
int n_globals = 0;
char* local_names[1000];
char* local_types[1000];
int n_locals = 0;
char c_code_buffer[1000000];
int c_code_pos = 0;
char expr_peek_buffer[4096];
int is_letter(char c);
int is_digit(char c);
int is_space(char c);
int is_ident_char(char c);
char* check_keywords(char* s);
int add_simple_token(int index, char* type, int line, int col);
int tokenize(char* source_code);
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
void print_global_symbols() {
int i = 0;
while (i < n_globals) {
printf("%s\n", concat(global_names[i], " ")concat(global_names[i], global_types[i]));
i = i + 1;
}
}
void print_local_symbols() {
int i = 0;
while (i < n_locals) {
printf("%s\n", concat(local_names[i], " ")concat(local_names[i], local_types[i]));
i = i + 1;
}
}
int main(int argc, char* argv[]) {
if (argc != 3) {
printf("%s\n", "Usage: compiler <input_file.dav> <output_file.c>");
return 1;
}
char* input_file = argv[1];
char* output_file = argv[2];
char* code = read_file(input_file);
if (code == 0) {
printf("%s\n", "Error: Could not read input file.");
return 1;
}
c_include();
c_prototype();
preset_global_functions();
tokenize(code);
parse();
c_helper();
write_file(output_file, c_code_buffer);
return 0;
}
int parse() {
while (strcmp(peek(), "EOF") != 0) {
global_decl();
}
return 0;
}
int global_decl() {
char* tok = peek();
if (strcmp(tok, "FN") == 0) {
fn_decl();
}
else if (strcmp(tok, "LET") == 0) {
let_stmt(1);
}
else {
int tok_line = token_lines[parser_pos];
printf("%s\n", concat("Error: Unexpected global token on line ", itos(tok_line)));
printf("%s\n", concat("Expected FN, LET, or COMMENT, but got: ", tok));
next();
return -1;
}
return 0;
}
int fn_decl() {
int fn_tok_idx = expect("FN");
int line_num = token_lines[fn_tok_idx];
char* fn_type = "void";
if (strcmp(peek(), "TYPE") == 0) {
int fn_type_idx = next();
fn_type = token_pool + token_values[fn_type_idx];
}
if (strcmp(peek(), "MUL") == 0) {
next();
if (strcmp(fn_type, "int") == 0) {
fn_type = "int*";
}
else if (strcmp(fn_type, "char") == 0) {
fn_type = "char*";
}
else if (strcmp(fn_type, "char*") == 0) {
fn_type = "char**";
}
else {
printf("%s\n", concat("Error: Cannot make array of type ", fn_type));
return -1;
}
}
int fn_name_idx = expect("ID");
char* fn_name = token_pool + token_values[fn_name_idx];
current_fn_ret_type = fn_type;
add_symbol(1, fn_name, fn_type);
expect("LPAREN");
emit(fn_type);
emit(" ");
emit(fn_name);
emit("(");
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
char* param_type = "int";
if (strcmp(peek(), "TYPE") == 0) {
int param_type_idx = next();
param_type = token_pool + token_values[param_type_idx];
}
if (strcmp(peek(), "MUL") == 0) {
next();
if (strcmp(param_type, "int") == 0) {
param_type = "int*";
}
else if (strcmp(param_type, "char") == 0) {
param_type = "char*";
}
else if (strcmp(param_type, "char*") == 0) {
param_type = "char**";
}
else {
printf("%s\n", concat("Error: Cannot make array of type ", param_type));
return -1;
}
}
int param_name_idx = expect("ID");
char* param_name = token_pool + token_values[param_name_idx];
emit(param_type);
emit(" ");
emit(param_name);
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
}
else {
emit("[]");
}
expect("RSQUARE");
}
param_types[n_params] = param_type;
param_names[n_params] = param_name;
param_has_arrays_part[n_params] = param_array_part;
n_params = n_params + 1;
}
expect("RPAREN");
emit(")");
if (strcmp(peek(), "SEMICOL") == 0) {
next();
emit(";
");
return 0;
}
else if (strcmp(peek(), "LBRACE") == 0) {
next();
emit(" {
");
clear_local_symbols();
i = 0;
while (i < n_params) {
char* var_type = param_types[i];
if (param_has_arrays_part[i] == 1) {
if (strcmp(var_type, "int") == 0) {
var_type = "int*";
}
else if (strcmp(var_type, "char") == 0) {
var_type = "char*";
}
else if (strcmp(var_type, "char*") == 0) {
var_type = "char**";
}
else {
printf("%s\n", concat("Error: Cannot make array of type ", var_type));
}
}
add_symbol(0, param_names[i], var_type);
i = i + 1;
}
while (strcmp(peek(), "RBRACE") != 0 && strcmp(peek(), "EOF") != 0) {
statement();
}
expect("RBRACE");
emit("}
");
return 0;
}
else {
printf("%s\n", concat("Error: Expected ';' or '{' after function signature, line ", itos(line_num)));
return -1;
}
}
int statement() {
char* tok = peek();
if (strcmp(tok, "LET") == 0) {
let_stmt(0);
}
else if (strcmp(tok, "PRINT") == 0) {
print_stmt();
}
else if (strcmp(tok, "IF") == 0) {
if_stmt();
}
else if (strcmp(tok, "WHILE") == 0) {
while_stmt();
}
else if (strcmp(tok, "RETURN") == 0) {
return_stmt();
}
else if (strcmp(tok, "ID") == 0) {
id_stmt();
}
else {
printf("%s\n", concat("Error: Unexpected statement: ", tok)concat("Error: Unexpected statement: ", " on line ")concat("Error: Unexpected statement: ", itos(token_lines[parser_pos])));
next();
return -1;
}
return 0;
}
int let_stmt(int is_global) {
int line_num = token_lines[parser_pos];
expect("LET");
char* var_type = "undefined";
if (strcmp(peek(), "TYPE") == 0) {
int var_type_idx = next();
var_type = token_pool + token_values[var_type_idx];
}
if (strcmp(peek(), "MUL") == 0) {
next();
if (strcmp(var_type, "int") == 0) {
var_type = "int*";
}
else if (strcmp(var_type, "char") == 0) {
var_type = "char*";
}
else {
printf("%s\n", concat("Error: Cannot make array of type ", var_type));
return -1;
}
}
int var_name_idx = expect("ID");
char* var_name = token_pool + token_values[var_name_idx];
if ((is_global == 0 && strcmp(get_symbol_type(0, var_name), "") != 0) || (is_global == 1 && strcmp(get_symbol_type(1, var_name), "") != 0)) {
printf("%s\n", concat("Error: Redefinition of variable ", var_name)concat("Error: Redefinition of variable ", ", line ")concat("Error: Redefinition of variable ", itos(line_num)));
return -1;
}
if (strcmp(peek(), "ASSIGN") == 0) {
next();
emit(var_type);
emit(" ");
emit(var_name);
emit(" = ");
expr();
emit(";
");
char* right_type = expr_type;
if (strcmp(var_type, "undefined") == 0) {
var_type = right_type;
}
else if (strcmp(var_type, right_type) != 0) {
printf("%s\n", concat("Error: Incompatible type ", right_type)concat("Error: Incompatible type ", " to ")concat("Error: Incompatible type ", var_type)concat("Error: Incompatible type ", ", line ")concat("Error: Incompatible type ", itos(line_num)));
return -1;
}
expect("SEMICOL");
add_symbol(is_global, var_name, var_type);
return 0;
}
else if (strcmp(peek(), "LSQUARE") == 0) {
next();
if (strcmp(var_type, "undefined") == 0) {
printf("%s\n", concat("Error: Array declaration must have an explicit type on line", itos(line_num)));
return -1;
}
int size_tok = expect("NUMBER");
char* size = token_pool + token_values[size_tok];
expect("RSQUARE");
expect("SEMICOL");
char* array_type = "int*";
if (strcmp(var_type, "int") == 0) {
array_type = "int*";
}
else if (strcmp(var_type, "char") == 0) {
array_type = "char*";
}
else if (strcmp(var_type, "char*") == 0) {
array_type = "char**";
}
else {
printf("%s\n", concat("Error: Cannot make array of type ", var_type));
return -1;
}
add_symbol(is_global, var_name, array_type);
emit(var_type);
emit(" ");
emit(var_name);
emit("[");
emit(size);
emit("];
");
return 0;
}
else if (strcmp(peek(), "SEMICOL") == 0) {
next();
if (strcmp(var_type, "undefined") == 0) {
printf("%s\n", concat("Error: Declaration without assignment must have explicit type on line", itos(line_num)));
return -1;
}
add_symbol(is_global, var_name, var_type);
emit(var_type);
emit(" ");
emit(var_name);
emit(";
");
return 0;
}
else {
printf("%s\n", concat("Error: Expected '=', '[', or ';' after variable name on line", itos(line_num)));
next();
return -1;
}
}
int print_stmt() {
int line_num = token_lines[parser_pos];
expect("PRINT");
expect("LPAREN");
char* expr_code = peek_code("expr");
char* type = expr_type;
if (strcmp(type, "int") == 0) {
emit("printf("%d\n", ");
}
else if (strcmp(type, "char") == 0) {
emit("printf("%c\n", ");
}
else if (strcmp(type, "char*") == 0) {
emit("printf("%s\n", ");
}
else {
printf("%s\n", concat("Error: Unprintable type '", type)concat("Error: Unprintable type '", "' on line ")concat("Error: Unprintable type '", itos(line_num)));
return -1;
}
emit(expr_code);
emit(");
");
expect("RPAREN");
expect("SEMICOL");
return 0;
}
int id_stmt() {
int tok_idx = next();
int line_num = token_lines[tok_idx];
char* var_name = token_pool + token_values[tok_idx];
char* var_type = get_symbol_type(0, var_name);
char* right_type;
if (strcmp(var_type, "") == 0) {
printf("%s\n", concat("Error: Undeclared identifier '", var_name)concat("Error: Undeclared identifier '", "' on line ")concat("Error: Undeclared identifier '", itos(line_num)));
return -1;
}
if (strcmp(peek(), "ASSIGN") == 0) {
next();
emit(var_name);
emit(" = ");
expr();
emit(";
");
right_type = expr_type;
if (strcmp(var_type, right_type) != 0) {
printf("%s\n", concat("Error: Incompatible ", right_type)concat("Error: Incompatible ", " to ")concat("Error: Incompatible ", var_type)concat("Error: Incompatible ", " conversion on line ")concat("Error: Incompatible ", itos(line_num)));
return -1;
}
expect("SEMICOL");
return 0;
}
else if (strcmp(peek(), "LPAREN") == 0) {
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
expect("RPAREN");
expect("SEMICOL");
emit(");
");
return 0;
}
else if (strcmp(peek(), "LSQUARE") == 0) {
next();
if (str_ends_with(var_type, *) == 0) {
printf("%s\n", concat("Error: Variable '", var_name)concat("Error: Variable '", "' is not an array and cannot be indexed, line ")concat("Error: Variable '", itos(line_num)));
return -1;
}
emit(var_name);
emit("[");
expr();
emit("] = ");
if (strcmp(expr_type, "int") != 0) {
printf("%s\n", concat("Error: Array index must be an integer, got ", expr_type)concat("Error: Array index must be an integer, got ", ", line ")concat("Error: Array index must be an integer, got ", itos(line_num)));
return -1;
}
expect("RSQUARE");
expect("ASSIGN");
expr();
emit(";
");
right_type = expr_type;
char* base_type = "int";
if (strcmp(var_type, "int*") == 0) {
base_type = "int";
}
else if (strcmp(var_type, "char*") == 0) {
base_type = "char";
}
else if (strcmp(var_type, "char**") == 0) {
base_type = "char*";
}
if (strcmp(base_type, right_type) != 0) {
printf("%s\n", concat("Error: Incompatible types: cannot assign ", right_type)concat("Error: Incompatible types: cannot assign ", " to array element of type ")concat("Error: Incompatible types: cannot assign ", base_type)concat("Error: Incompatible types: cannot assign ", ", line ")concat("Error: Incompatible types: cannot assign ", itos(line_num)));
return -1;
}
expect("SEMICOL");
return 0;
}
else {
printf("%s\n", concat("Error: Invalid statement start. Expected '=', '(', or '[' after ID '", var_name)concat("Error: Invalid statement start. Expected '=', '(', or '[' after ID '", "', line ")concat("Error: Invalid statement start. Expected '=', '(', or '[' after ID '", itos(line_num)));
return -1;
}
}
int if_stmt() {
expect("IF");
emit("if (");
expr();
emit(") {
");
expect("LBRACE");
while (strcmp(peek(), "RBRACE") != 0 && strcmp(peek(), "EOF") != 0) {
statement();
}
expect("RBRACE");
emit("}
");
if (strcmp(peek(), "ELSE") == 0) {
next();
emit("else ");
if (strcmp(peek(), "IF") == 0) {
if_stmt();
}
else if (strcmp(peek(), "LBRACE") == 0) {
next();
emit("{
");
while (strcmp(peek(), "RBRACE") != 0 && strcmp(peek(), "EOF") != 0) {
statement();
}
expect("RBRACE");
emit("}
");
}
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
emit(") {
");
expect("LBRACE");
while (strcmp(peek(), "RBRACE") != 0 && strcmp(peek(), "EOF") != 0) {
statement();
}
expect("RBRACE");
emit("}
");
return 0;
}
int return_stmt() {
int line_num = token_lines[parser_pos];
expect("RETURN");
emit("return ");
expr();
emit(";
");
char* ret_type = expr_type;
expect("SEMICOL");
if (strcmp(current_fn_ret_type, ret_type) != 0) {
printf("%s\n", concat("Error: Incompatible ", ret_type)concat("Error: Incompatible ", " to ")concat("Error: Incompatible ", current_fn_ret_type)concat("Error: Incompatible ", " conversion on line ")concat("Error: Incompatible ", itos(line_num)));
return -1;
}
return 0;
}
int expr() {
return logical();
}
int logical() {
relational();
char* left_type = expr_type;
while (strcmp(peek(), "OR") == 0 || strcmp(peek(), "AND") == 0) {
int op_idx = next();
char* op = op_to_c_op(token_types[op_idx]);
emit(" ");
emit(op);
emit(" ");
relational();
char* right_type = expr_type;
if (strcmp(left_type, "int") != 0 || strcmp(right_type, "int") != 0) {
printf("%s\n", concat("Error: Logical operators '&&' and '||' can only be used on integers, line ", itos(token_lines[op_idx])));
return -1;
}
expr_type = "int";
left_type = "int";
}
expr_type = left_type;
return 0;
}
int relational() {
char* left_ptr = peek_code("additive");
char* left_type = expr_type;
char left_buf[2048];
int i = 0;
while (left_ptr[i] != 0) {
left_buf[i] = left_ptr[i];
i = i + 1;
}
left_buf[i] = 0;
if (strcmp(peek(), "EQ") == 0 || strcmp(peek(), "NE") == 0 || strcmp(peek(), "LT") == 0 || strcmp(peek(), "GT") == 0 || strcmp(peek(), "LE") == 0 || strcmp(peek(), "GE") == 0) {
while (strcmp(peek(), "EQ") == 0 || strcmp(peek(), "NE") == 0 || strcmp(peek(), "LT") == 0 || strcmp(peek(), "GT") == 0 || strcmp(peek(), "LE") == 0 || strcmp(peek(), "GE") == 0) {
int op_idx = next();
char* op = op_to_c_op(token_types[op_idx]);
int line = token_lines[op_idx];
char* right_code = peek_code("additive");
char* right_type = expr_type;
if (strcmp(left_type, "char*") == 0 && strcmp(right_type, "char*") == 0) {
if (strcmp(op, "==") == 0) {
emit("strcmp(");
emit(left_buf);
emit(", ");
emit(right_code);
emit(") == 0");
}
else if (strcmp(op, "!=") == 0) {
emit("strcmp(");
emit(left_buf);
emit(", ");
emit(right_code);
emit(") != 0");
}
else {
printf("%s\n", concat("Error: Operator '", op)concat("Error: Operator '", "' not allowed on strings, line ")concat("Error: Operator '", itos(line)));
return -1;
}
}
else if ((strcmp(left_type, "char*") == 0 && strcmp(right_type, "int") == 0) || (strcmp(left_type, "int") == 0 && strcmp(right_type, "char*") == 0)) {
if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) {
emit(left_buf);
emit(" ");
emit(op);
emit(" ");
emit(right_code);
}
else {
printf("%s\n", concat("Error: Operator '", op)concat("Error: Operator '", "' not allowed on strings, line ")concat("Error: Operator '", itos(line)));
return -1;
}
}
else if (strcmp(left_type, "char*") == 0 || strcmp(right_type, "char*") == 0) {
printf("%s\n", concat("Error: Comparison between string and non-string, line ", itos(line)));
return -1;
}
else {
emit(left_buf);
emit(" ");
emit(op);
emit(" ");
emit(right_code);
}
expr_type = "int";
left_type = "int";
}
}
else {
emit(left_buf);
expr_type = left_type;
}
return 0;
}
int additive() {
char* left_ptr = peek_code("multiplicative");
char* left_type = expr_type;
char left_buf[2048];
int i = 0;
while ((left_ptr[i] != 0)) {
left_buf[i] = left_ptr[i];
i = i + 1;
}
left_buf[i] = 0;
if (strcmp(peek(), "PLUS") == 0 || strcmp(peek(), "MINUS") == 0) {
while (strcmp(peek(), "PLUS") == 0 || strcmp(peek(), "MINUS") == 0) {
int op_idx = next();
char* op = op_to_c_op(token_types[op_idx]);
int line = token_lines[op_idx];
char* right_code = peek_code("multiplicative");
char* right_type = expr_type;
if (strcmp(left_type, "int") == 0 && strcmp(right_type, "int") == 0) {
emit(left_buf);
emit(" ");
emit(op);
emit(" ");
emit(right_code);
expr_type = "int";
}
else if (str_ends_with(left_type, *) && strcmp(right_type, "int") == 0) {
emit(left_buf);
emit(" ");
emit(op);
emit(" ");
emit(right_code);
expr_type = left_type;
}
else if (strcmp(left_type, "int") == 0 && str_ends_with(right_type, *)) {
if (strcmp(op, "+") == 0) {
emit(left_buf);
emit(op);
emit(right_code);
expr_type = right_type;
}
else {
printf("%s\n", concat("Error: Cannot subtract a pointer from an integer, line ", itos(line)));
return -1;
}
}
else if (strcmp(left_type, "char*") == 0 && strcmp(right_type, "char*") == 0 && strcmp(op, "+") == 0) {
emit("concat(");
emit(left_buf);
emit(", ");
emit(right_code);
emit(")");
expr_type = "char*";
}
else {
printf("%s\n", concat("Error: Operator '", op)concat("Error: Operator '", "' not allowed between '")concat("Error: Operator '", left_type)concat("Error: Operator '", "' and '")concat("Error: Operator '", right_type)concat("Error: Operator '", "', line ")concat("Error: Operator '", itos(line)));
return -1;
}
left_type = expr_type;
}
}
else {
emit(left_buf);
expr_type = left_type;
}
expr_type = left_type;
return 0;
}
int multiplicative() {
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
if (strcmp(peek(), "MINUS") == 0) {
int op_idx = next();
emit("-");
unary();
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
int tok_idx = next();
char* tok_type = token_types[tok_idx];
int tok_val_idx = token_values[tok_idx];
int tok_line = token_lines[tok_idx];
if (strcmp(tok_type, "NUMBER") == 0) {
expr_type = "int";
emit(token_pool + tok_val_idx);
}
else if (strcmp(tok_type, "CHAR") == 0) {
expr_type = "char";
emit(token_pool + tok_val_idx);
}
else if (strcmp(tok_type, "STRING") == 0) {
expr_type = "char*";
emit(""");
emit(token_pool + tok_val_idx);
emit(""");
}
else if (strcmp(tok_type, "LPAREN") == 0) {
emit("(");
expr();
emit(")");
expect("RPAREN");
}
else if (strcmp(tok_type, "ID") == 0) {
char* var_name = token_pool + tok_val_idx;
char* sym_type = get_symbol_type(0, var_name);
if (strcmp(sym_type, "") == 0) {
printf("%s\n", concat("Error: Undeclared identifier '", var_name)concat("Error: Undeclared identifier '", "' on line ")concat("Error: Undeclared identifier '", itos(tok_line)));
return -1;
}
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
expect("RPAREN");
emit(")");
}
else if (strcmp(peek(), "LSQUARE") == 0) {
if (str_ends_with(sym_type, *) == 0) {
printf("%s\n", concat("Error: Variable '", var_name)concat("Error: Variable '", "' is not an array and cannot be indexed, line ")concat("Error: Variable '", itos(tok_line)));
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
if (strcmp(sym_type, "int*") == 0) {
expr_type = "int";
}
else if (strcmp(sym_type, "char*") == 0) {
expr_type = "char";
}
else if (strcmp(sym_type, "char**") == 0) {
expr_type = "char*";
}
else {
expr_type = "int";
}
}
else {
expr_type = sym_type;
emit(var_name);
}
}
else {
printf("%s\n", concat("Error: Unexpected token in expression: ", tok_type)concat("Error: Unexpected token in expression: ", " on line ")concat("Error: Unexpected token in expression: ", itos(tok_line)));
return -1;
}
return 0;
}
char* peek() {
return token_types[parser_pos];
}
int next() {
int current_pos = parser_pos;
parser_pos = parser_pos + 1;
return current_pos;
}
int expect(char* kind) {
char* tok_type = peek();
if (strcmp(tok_type, kind) == 0) {
return next();
}
int tok_line = token_lines[parser_pos];
printf("%s\n", concat("Error: Syntax Error on line ", itos(tok_line)));
printf("%s\n", concat("Expected token: ", kind));
printf("%s\n", concat("... but got token: ", tok_type));
return -1;
}
int clear_local_symbols() {
n_locals = 0;
return 0;
}
char* get_symbol_type(int is_global, char* name) {
int i = 0;
if (is_global == 0) {
while (i < n_locals) {
if (strcmp(local_names[i], name) == 0) {
return local_types[i];
}
i = i + 1;
}
}
else {
while (i < n_globals) {
if (strcmp(global_names[i], name) == 0) {
return global_types[i];
}
i = i + 1;
}
}
if (is_global == 0) {
return get_symbol_type(1, name);
}
return "";
}
int add_symbol(int is_global, char* name, char* type) {
if (is_global == 0) {
local_names[n_locals] = name;
local_types[n_locals] = type;
n_locals = n_locals + 1;
}
else {
global_names[n_globals] = name;
global_types[n_globals] = type;
n_globals = n_globals + 1;
}
return 0;
}
int str_ends_with(char* s, char c) {
int len = strlen(s);
if (len == 0) {
return 0;
}
if (s[len - 1] == c) {
return 1;
}
return 0;
}
char* op_to_c_op(char* tok_type) {
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
return "";
}
int emit(char* s) {
int i = 0;
int len = strlen(s);
if (c_code_pos + len >= 1000000) {
printf("%s\n", "CRITICAL ERROR: C code output buffer overflow! Increase c_code_buffer size.");
return -1;
}
while (i < len) {
c_code_buffer[c_code_pos] = s[i];
c_code_pos = c_code_pos + 1;
i = i + 1;
}
c_code_buffer[c_code_pos] = 0;
return 0;
}
char* peek_code(char* level) {
int start_pos = c_code_pos;
if ((strcmp(level, "expr") == 0)) {
expr();
}
else if ((strcmp(level, "logical") == 0)) {
logical();
}
else if ((strcmp(level, "relational") == 0)) {
relational();
}
else if ((strcmp(level, "additive") == 0)) {
additive();
}
else if ((strcmp(level, "multiplicative") == 0)) {
multiplicative();
}
else if ((strcmp(level, "unary") == 0)) {
unary();
}
else if ((strcmp(level, "atom") == 0)) {
atom();
}
else {
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
expr_peek_buffer[i] = 0;
c_code_pos = start_pos;
return expr_peek_buffer;
}
int c_include() {
emit("#include <stdio.h>
");
emit("#include <stdlib.h>
");
emit("#include <string.h>

");
return 0;
}
int c_prototype() {
emit("char* concat(char* str1, char* str2);
");
emit("char* itos(int x);
");
emit("char* ctos(char c);

");
emit("char* read_file(char* path);
");
emit("void write_file(char* path, char* content);
");
return 0;
}
int c_helper() {
emit("
char* concat(char* str1, char* str2) {
");
emit("static char buf[1024];
");
emit("snprintf(buf, sizeof(buf), "%s%s", str1, str2);
");
emit("return buf;
}

");
emit("char* itos(int x) {
");
emit("static char buf[32];
");
emit("snprintf(buf, sizeof(buf), "%d", x);
");
emit("return buf;
}

");
emit("char* ctos(char c) {
");
emit("static char buf[2];
");
emit("buf[0] = c;
");
emit("buf[1] = '\0';
");
emit("return buf;
}

");
emit("char* read_file(char* path) {
");
emit("FILE* f = fopen(path, "rb");
");
emit("if (!f) return NULL;
");
emit("fseek(f, 0, SEEK_END);
");
emit("long len = ftell(f);
");
emit("fseek(f, 0, SEEK_SET);
");
emit("char* buf = malloc(len + 1);
");
emit("fread(buf, 1, len, f);
");
emit("buf[len] = '\0';
");
emit("fclose(f);
");
emit("return buf;
}

");
emit("void write_file(char* path, char* content) {
");
emit("FILE* f = fopen(path, "w");
");
emit("if (!f) return;
");
emit("fprintf(f, "%s", content);
");
emit("fclose(f);
}
");
return 0;
}
int preset_global_functions() {
add_symbol(1, "concat", "char*");
add_symbol(1, "ctos", "char*");
add_symbol(1, "itos", "char*");
add_symbol(1, "strlen", "int");
add_symbol(1, "strcmp", "int");
add_symbol(1, "read_file", "char*");
add_symbol(1, "write_file", "void");
return 0;
}
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
while (source_code[pos] != 0) {
c = source_code[pos];
col = pos - line_start;
i = 0;
if (token_count >= 50000) {
printf("%s\n", "CRITICAL ERROR: Too many tokens! Increase token array sizes.");
return 0;
}
if (pool_pos >= 499000) {
printf("%s\n", "CRITICAL ERROR: String pool overflow! Increase token_pool size.");
return 0;
}
if (is_space(c)) {
if (c == 
) {
line_num = line_num + 1;
line_start = pos + 1;
}
pos = pos + 1;
}
else if (is_digit(c)) {
token_start_col = col;
while (is_digit(c)) {
buffer[i] = c;
i = i + 1;
pos = pos + 1;
c = source_code[pos];
}
if (c == .) {
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
buffer[i] = 0;
token_types[token_count] = "NUMBER";
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
}
else if (is_letter(c)) {
token_start_col = col;
while (is_ident_char(c)) {
buffer[i] = c;
i = i + 1;
pos = pos + 1;
c = source_code[pos];
}
buffer[i] = 0;
char* tok_type = check_keywords(buffer);
token_types[token_count] = tok_type;
token_lines[token_count] = line_num;
token_cols[token_count] = token_start_col;
if (strcmp(tok_type, "ID") != 0 && strcmp(tok_type, "TYPE") != 0) {
token_values[token_count] = -1;
}
else {
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
else if (c == =) {
if (source_code[pos + 1] == =) {
add_simple_token(token_count, "EQ", line_num, col);
token_count = token_count + 1;
pos = pos + 2;
}
else {
add_simple_token(token_count, "ASSIGN", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
}
else if (c == ! && source_code[pos + 1] == =) {
add_simple_token(token_count, "NE", line_num, col);
token_count = token_count + 1;
pos = pos + 2;
}
else if (c == >) {
if (source_code[pos + 1] == =) {
add_simple_token(token_count, "GE", line_num, col);
token_count = token_count + 1;
pos = pos + 2;
}
else {
add_simple_token(token_count, "GT", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
}
else if (c == <) {
if (source_code[pos + 1] == =) {
add_simple_token(token_count, "LE", line_num, col);
token_count = token_count + 1;
pos = pos + 2;
}
else {
add_simple_token(token_count, "LT", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
}
else if (c == & && source_code[pos + 1] == &) {
add_simple_token(token_count, "AND", line_num, col);
token_count = token_count + 1;
pos = pos + 2;
}
else if (c == | && source_code[pos + 1] == |) {
add_simple_token(token_count, "OR", line_num, col);
token_count = token_count + 1;
pos = pos + 2;
}
else if (c == /) {
if (source_code[pos + 1] == /) {
while (source_code[pos] != 
 && source_code[pos] != 
) {
pos = pos + 1;
}
}
else {
add_simple_token(token_count, "DIV", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
}
else if (c == () {
add_simple_token(token_count, "LPAREN", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == )) {
add_simple_token(token_count, "RPAREN", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == {) {
add_simple_token(token_count, "LBRACE", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == }) {
add_simple_token(token_count, "RBRACE", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == [) {
add_simple_token(token_count, "LSQUARE", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == ]) {
add_simple_token(token_count, "RSQUARE", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == +) {
add_simple_token(token_count, "PLUS", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == -) {
add_simple_token(token_count, "MINUS", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == *) {
add_simple_token(token_count, "MUL", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == ;) {
add_simple_token(token_count, "SEMICOL", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == ,) {
add_simple_token(token_count, "COMMA", line_num, col);
token_count = token_count + 1;
pos = pos + 1;
}
else if (c == ") {
token_start_col = col;
pos = pos + 1;
c = source_code[pos];
while (c != " && c != 0) {
if (c == \) {
pos = pos + 1;
c = source_code[pos];
if (c == n) {
buffer[i] = 
;
}
else if (c == t) {
buffer[i] = 	;
}
else if (c == ") {
buffer[i] = ";
}
else if (c == \) {
buffer[i] = \;
}
else {
buffer[i] = c;
}
}
else {
buffer[i] = c;
}
i = i + 1;
pos = pos + 1;
c = source_code[pos];
}
if (c == 0) {
printf("%s\n", "Error: Unclosed string literal!");
return 1;
}
pos = pos + 1;
buffer[i] = 0;
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
}
else if (c == ') {
token_start_col = col;
pos = pos + 1;
c = source_code[pos];
token_val = c;
if (c == \) {
pos = pos + 1;
c = source_code[pos];
if (c == n) {
token_val = 
;
}
else if (c == t) {
token_val = 	;
}
else if (c == ') {
token_val = ';
}
else if (c == \) {
token_val = \;
}
else {
token_val = c;
}
}
pos = pos + 1;
c = source_code[pos];
if (c != ') {
printf("%s\n", "Error: Unclosed or invalid char literal!");
return 1;
}
pos = pos + 1;
buffer[0] = token_val;
buffer[1] = 0;
token_types[token_count] = "CHAR";
token_lines[token_count] = line_num;
token_cols[token_count] = token_start_col;
token_values[token_count] = pool_pos;
token_pool[pool_pos] = buffer[0];
token_pool[pool_pos + 1] = buffer[1];
pool_pos = pool_pos + 2;
token_count = token_count + 1;
}
else {
printf("%s\n", concat("Error: Unexpected character!", ctos(c)));
return 1;
}
}
add_simple_token(token_count, "EOF", line_num, col);
token_count = token_count + 1;
n_tokens = token_count;
return 0;
}
int is_letter(char c) {
return (c >= a && c <= z) || (c >= A && c <= Z) || (c == _);
}
int is_digit(char c) {
return c >= 0 && c <= 9;
}
int is_space(char c) {
return (c ==  ) || (c == 	) || (c == 
);
}
int is_ident_char(char c) {
return is_letter(c) || is_digit(c);
}
char* check_keywords(char* s) {
if (strcmp(s, "ah") == 0) {
return "FN";
}
else if (strcmp(s, "beg") == 0) {
return "LET";
}
else if (strcmp(s, "boo") == 0) {
return "PRINT";
}
else if (strcmp(s, "if") == 0) {
return "IF";
}
else if (strcmp(s, "else") == 0) {
return "ELSE";
}
else if (strcmp(s, "while") == 0) {
return "WHILE";
}
else if (strcmp(s, "return") == 0) {
return "RETURN";
}
else if (strcmp(s, "int*") == 0 || strcmp(s, "char*") == 0 || strcmp(s, "int") == 0 || strcmp(s, "char") == 0 || strcmp(s, "void") == 0) {
return "TYPE";
}
return "ID";
}
int add_simple_token(int index, char* type, int line, int col) {
token_types[index] = type;
token_values[index] = -1;
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
fread(buf, 1, len, f);
buf[len] = '\0';
fclose(f);
return buf;
}

void write_file(char* path, char* content) {
FILE* f = fopen(path, "w");
if (!f) return;
fprintf(f, "%s", content);
fclose(f);
}
