import unittest
# Assuming the Parser class is in a file named parser.py
from python.parser.parser import Parser


class TestParser(unittest.TestCase):
    """
    Unit tests for the updated Parser class with type-checking and strings.
    Tokens are expected to be in the format:
    (kind, value, line_number, optional_indentation_level)
    """

    def setUp(self):
        # A minimal set of tokens to represent the end of the file
        self.eof_token = ('EOF', '', 0, 0)

    # --- Utility Tests (Unchanged) ---

    def test_expect_success(self):
        tokens = [('ID', 'my_var', 1, 0), self.eof_token]
        parser = Parser(tokens)

        # Test expect
        tok = parser.expect('ID')
        self.assertEqual(tok, ('ID', 'my_var', 1, 0))
        self.assertEqual(parser.pos, 1)

    def test_expect_failure(self):
        tokens = [('NUMBER', '10', 1, 0), self.eof_token]
        parser = Parser(tokens)

        # Test expect raising SyntaxError
        with self.assertRaises(SyntaxError) as cm:
            parser.expect('ID')
        self.assertIn("Expected ID, got ('NUMBER', '10', 1, 0)",
                      str(cm.exception))

    def test_peek_next(self):
        tokens = [('ID', 'var1', 1, 0), ('PLUS', '+', 1, 0), self.eof_token]
        parser = Parser(tokens)

        self.assertEqual(parser.peek(), 'ID')
        self.assertEqual(parser.next(), ('ID', 'var1', 1, 0))
        self.assertEqual(parser.peek(), 'PLUS')
        self.assertEqual(parser.next(), ('PLUS', '+', 1, 0))
        self.assertEqual(parser.peek(), 'EOF')

    # --- Full Parse (fn_decl and parse) Tests ---

    def test_parse_empty_fn(self):
        tokens = [
            ('FN', 'ah', 1, 0), ('ID', 'main', 1, 0), ('LPAREN', '(', 1, 0),
            ('RPAREN', ')', 1, 0), ('LBRACE',
                                    '{', 1, 0), ('RBRACE', '}', 2, 0),
            self.eof_token
        ]
        parser = Parser(tokens)
        # Note: The new parser does not auto-add 'return 0;'
        expected_code = (
            'int main() {\n'
            '\n'
            '}\n'
        )
        self.assertEqual(parser.parse(), expected_code)

    def test_parse_fn_with_default_params(self):
        tokens = [
            ('FN', 'ah', 1, 0), ('ID', 'add', 1, 0), ('LPAREN', '(', 1, 0),
            ('ID', 'a', 1, 0), ('COMMA', ',', 1, 0), ('ID', 'b', 1, 0),
            ('RPAREN', ')', 1, 0), ('LBRACE', '{', 1, 0),
            ('RETURN', 'return', 2, 4), ('ID', 'a', 2, 4), ('PLUS', '+', 2, 4),
            ('ID', 'b', 2, 4), ('SEMICOL', ';', 2, 4),
            ('RBRACE', '}', 3, 0),
            self.eof_token
        ]
        parser = Parser(tokens)
        expected_code = (
            'int add(int a, int b) {\n'
            '    return a + b;\n'
            '}\n'
        )
        self.assertEqual(parser.parse(), expected_code)

    def test_parse_fn_with_typed_params(self):
        tokens = [
            ('FN', 'ah', 1, 0), ('TYPE', 'char*', 1,
                                 0), ('ID', 'greet', 1, 0), ('LPAREN', '(', 1, 0),
            ('TYPE', 'char*', 1, 0), ('ID', 'name', 1, 0),
            ('RPAREN', ')', 1, 0), ('LBRACE', '{', 1, 0),
            ('RETURN', 'return', 2, 4), ('STRING',
                                         '"Hello"', 2, 4), ('PLUS', '+', 2, 4),
            ('ID', 'name', 2, 4), ('SEMICOL', ';', 2, 4),
            ('RBRACE', '}', 3, 0),
            self.eof_token
        ]
        parser = Parser(tokens)
        expected_code = (
            'char* greet(char* name) {\n'
            '    return concat("Hello", name);\n'
            '}\n'
        )
        self.assertEqual(parser.parse(), expected_code)

    def test_parse_fn_with_array_param(self):
        tokens = [
            ('FN', 'ah', 1, 0), ('ID', 'get_first',
                                 1, 0), ('LPAREN', '(', 1, 0),
            ('TYPE', 'int*', 1, 0), ('ID', 'arr', 1, 0),
            ('RPAREN', ')', 1, 0), ('LBRACE', '{', 1, 0),
            ('RETURN', 'return', 2, 4), ('ID',
                                         'arr', 2, 4), ('LSQUARE', '[', 2, 4),
            ('NUMBER', 0, 2, 4), ('RSQUARE', ']', 2, 4), ('SEMICOL', ';', 2, 4),
            ('RBRACE', '}', 3, 0),
            self.eof_token
        ]
        parser = Parser(tokens)
        expected_code = (
            'int get_first(int* arr) {\n'
            '    return arr[0];\n'
            '}\n'
        )
        self.assertEqual(parser.parse(), expected_code)

    # --- Statement Tests (Updated) ---

    def test_let_statement_new_var(self):
        tokens = [
            ('LET', 'beg', 1, 4), ('ID', 'x', 1, 4), ('ASSIGN', '=', 1, 4),
            ('NUMBER', 5, 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {}  # Must be a dict now
        parser.env = {}

        self.assertEqual(parser.statement(), 'int x = 5;')
        self.assertIn('x', parser.variables)
        self.assertEqual(parser.variables['x'], 'int')

    def test_let_statement_reassignment(self):
        tokens = [
            ('LET', 'beg', 1, 4), ('ID', 'y', 1, 4), ('ASSIGN', '=', 1, 4),
            ('NUMBER', 10, 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        # 'y' is already defined
        parser.variables = {'y': 'int'}
        parser.env = {}

        self.assertEqual(parser.statement(), 'y = 10;')
        self.assertIn('y', parser.variables)

    def test_id_statement_assignment(self):
        tokens = [
            ('ID', 'myvar', 1, 4), ('ASSIGN', '=', 1, 4),
            ('NUMBER', 1, 1, 4), ('PLUS', '+', 1, 4), ('NUMBER', 2, 1, 4),
            ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {'myvar': 'int'}  # Must be defined
        parser.env = {}

        self.assertEqual(parser.statement(), 'myvar = 1 + 2;')

    def test_id_statement_fn_call(self):
        tokens = [
            ('ID', 'func', 1, 4), ('LPAREN', '(', 1, 4),
            ('NUMBER', 5, 1, 4), ('COMMA', ',', 1, 4),
            ('ID', 'z', 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        # z is a local variable, func is in the global env
        parser.variables = {'z': 'int'}
        parser.env = {'func': 'int'}

        self.assertEqual(parser.statement(), 'func(5, z);')

    def test_print_statement_int(self):
        tokens = [
            ('PRINT', 'boo', 1, 4), ('LPAREN', '(', 1, 4),
            ('ID', 'val', 1, 4), ('MUL', '*', 1, 4), ('NUMBER', 2, 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {'val': 'int'}
        parser.env = {}

        self.assertEqual(parser.statement(), 'printf("%d\\n", val * 2);')

    def test_return_statement_int(self):
        tokens = [
            ('RETURN', 'return', 1, 4),
            ('LPAREN', '(', 1, 4), ('NUMBER', 1, 1, 4),
            ('PLUS', '+', 1, 4), ('NUMBER', 1, 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.fn_name = "my_func"
        parser.env = {"my_func": "int"}  # Setup return type for function

        self.assertEqual(parser.statement(), 'return (1 + 1);')

    # --- Control Flow Tests (Updated) ---

    def test_if_statement(self):
        tokens = [
            ('IF', 'if', 1, 4), ('ID', 'x', 1, 4), ('EQ',
                                                    '==', 1, 4), ('NUMBER', 1, 1, 4),
            ('LBRACE', '{', 1, 4),
            ('PRINT', 'boo', 2, 6), ('LPAREN', '(', 2, 6), ('ID', 'x', 2, 6),
            ('RPAREN', ')', 2, 6), ('SEMICOL', ';', 2, 6),
            ('RBRACE', '}', 3, 4),
            ('RBRACE', '}', 4, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {'x': 'int'}
        expected_code = (
            'if (x == 1) {\n'
            '        printf("%d\\n", x);\n'
            '    }'
        )
        self.assertEqual(parser.statement(), expected_code)

    def test_if_else_statement(self):
        tokens = [
            ('IF', 'if', 1, 4), ('ID', 'x', 1, 4), ('GT',
                                                    '>', 1, 4), ('NUMBER', 0, 1, 4),
            ('LBRACE', '{', 1, 4),
            ('LET', 'beg', 2, 6), ('ID', 'y', 2, 6), ('ASSIGN', '=',
                                                      2, 6), ('NUMBER', 1, 2, 6), ('SEMICOL', ';', 2, 6),
            ('RBRACE', '}', 3, 4),
            ('ELSE', 'else', 3, 4), ('LBRACE', '{', 3, 4),
            ('ID', 'y', 4, 6), ('ASSIGN', '=', 4,
                                6), ('NUMBER', 0, 4, 6), ('SEMICOL', ';', 4, 6),
            ('RBRACE', '}', 5, 4),
            ('RBRACE', '}', 6, 0)
        ]
        parser = Parser(tokens)
        # y is pre-defined
        parser.variables = {'x': 'int', 'y': 'int'}
        expected_code = (
            'if (x > 0) {\n'
            '        y = 1;\n'  # 'y' is already in variables, so it's a reassignment
            '    } else {\n'
            '        y = 0;\n'
            '    }'
        )
        self.assertEqual(parser.statement(), expected_code)

    def test_while_statement(self):
        tokens = [
            ('WHILE', 'while', 1, 4), ('ID', 'i', 1,
                                       4), ('LT', '<', 1, 4), ('NUMBER', 10, 1, 4),
            ('LBRACE', '{', 1, 4),
            ('ID', 'i', 2, 6), ('ASSIGN', '=', 2, 6), ('ID', 'i', 2, 6), ('PLUS',
                                                                          '+', 2, 6), ('NUMBER', 1, 2, 6), ('SEMICOL', ';', 2, 6),
            ('RBRACE', '}', 3, 4),
            ('RBRACE', '}', 4, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {'i': 'int'}
        expected_code = (
            'while (i < 10) {\n'
            '        i = i + 1;\n'
            '    }'
        )
        self.assertEqual(parser.statement(), expected_code)

    # --- Expression Tests (Updated) ---

    def test_expression_arithmetic(self):
        tokens = [
            ('NUMBER', 1, 1, 0), ('PLUS', '+', 1, 0), ('NUMBER', 2, 1, 0),
            ('MUL', '*', 1, 0), ('NUMBER', 3, 1,
                                 0), ('SEMICOL', ';', 1, 0)
        ]
        parser = Parser(tokens)
        # expr() now returns (type, value)
        self.assertEqual(parser.expr(), ('int', '1 + 2 * 3'))

    def test_expression_comparison(self):
        tokens = [
            ('ID', 'a', 1, 0), ('NE', '!=', 1, 0), ('NUMBER', 5, 1, 0),
            ('SEMICOL', ';', 1, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {'a': 'int'}
        self.assertEqual(parser.expr(), ('int', 'a != 5'))

    def test_expression_parens(self):
        tokens = [
            ('LPAREN', '(', 1, 0), ('NUMBER', 1, 1, 0), ('PLUS', '+', 1, 0),
            ('NUMBER', 2, 1, 0), ('RPAREN', ')', 1, 0),
            ('MUL', '*', 1, 0), ('NUMBER', 3, 1,
                                 0), ('SEMICOL', ';', 1, 0)
        ]
        parser = Parser(tokens)
        self.assertEqual(parser.expr(), ('int', '(1 + 2) * 3'))

    def test_expression_fn_call_atom(self):
        tokens = [
            ('ID', 'call', 1, 0), ('LPAREN', '(', 1, 0),
            ('ID', 'a', 1, 0), ('COMMA', ',', 1, 0),
            ('NUMBER', 10, 1, 0), ('COMMA', ',', 1, 0),
            ('ID', 'b', 1, 0), ('PLUS', '+', 1, 0), ('NUMBER', 5, 1, 0),
            ('RPAREN', ')', 1, 0), ('SEMICOL', ';', 1, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {'a': 'int', 'b': 'int'}
        parser.env = {'call': 'int'}
        # Test atom() (formerly factor())
        self.assertEqual(parser.atom(), ('int', 'call(a, 10, b + 5)'))

    # --- NEW: String and Type Tests ---

    def test_let_string_var(self):
        tokens = [
            ('LET', 'beg', 1, 4), ('TYPE', 'char*', 1,
                                   4), ('ID', 's', 1, 4), ('ASSIGN', '=', 1, 4),
            ('STRING', '"hello"', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {}
        parser.env = {}
        self.assertEqual(parser.statement(), 'char* s = "hello";')
        self.assertEqual(parser.variables['s'], 'char*')

    def test_let_string_var_implicit(self):
        tokens = [
            ('LET', 'beg', 1, 4), ('ID', 's', 1, 4), ('ASSIGN', '=', 1, 4),
            ('STRING', '"hello"', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {}
        parser.env = {}
        # Type is inferred from the expression
        self.assertEqual(parser.statement(), 'char* s = "hello";')
        self.assertEqual(parser.variables['s'], 'char*')

    def test_print_string(self):
        tokens = [
            ('PRINT', 'boo', 1, 4), ('LPAREN', '(', 1, 4),
            ('STRING', '"Hello World"', 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {}
        self.assertEqual(parser.statement(), 'printf("%s\\n", "Hello World");')

    def test_print_char(self):
        tokens = [
            ('PRINT', 'boo', 1, 4), ('LPAREN', '(', 1, 4),
            ('CHAR', "'c'", 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {}
        self.assertEqual(parser.statement(), 'printf("%c\\n", \'c\');')

    def test_string_concat_expr(self):
        tokens = [
            ('ID', 's1', 1, 0), ('PLUS', '+', 1, 0), ('STRING', '" world"', 1, 0),
            ('SEMICOL', ';', 1, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {'s1': 'char*'}
        self.assertEqual(parser.expr(), ('char*', 'concat(s1, " world")'))

    def test_pointer_arithmetic_expr(self):
        """Tests that pointer + int arithmetic IS allowed."""
        tokens = [
            ('ID', 'my_array', 1, 0), ('PLUS', '+', 1, 0), ('NUMBER', 5, 1, 0),
            ('SEMICOL', ';', 1, 0)
        ]
        parser = Parser(tokens)
        # my_array is 'int*' (an int array)
        parser.variables = {'my_array': 'int*'}

        # This should correctly generate C pointer math
        self.assertEqual(parser.expr(), ('int*', '(my_array + 5)'))

    def test_let_array_declaration(self):
        tokens = [
            ('LET', 'beg', 1, 4), ('TYPE', 'int', 1, 4), ('ID', 'my_arr', 1, 4),
            ('LSQUARE', '[', 1, 4), ('NUMBER',
                                     10, 1, 4), ('RSQUARE', ']', 1, 4),
            ('SEMICOL', ';', 1, 4), ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {}
        self.assertEqual(parser.statement(), 'int my_arr[10];')
        # Check that the type is stored as a pointer
        self.assertEqual(parser.variables['my_arr'], 'int*')

    def test_id_stmt_array_assignment(self):
        tokens = [
            ('ID', 'my_arr', 1, 4), ('LSQUARE',
                                     '[', 1, 4), ('NUMBER', 0, 1, 4),
            ('RSQUARE', ']', 1, 4), ('ASSIGN', '=', 1, 4), ('NUMBER', 5, 1, 4),
            ('SEMICOL', ';', 1, 4), ('RBRACE', '}', 2, 0)
        ]
        parser = Parser(tokens)
        parser.variables = {'my_arr': 'int*'}  # Array must be pre-defined
        self.assertEqual(parser.statement(), 'my_arr[0] = 5;')

    def test_atom_array_access(self):
        tokens = [
            ('ID', 'my_arr', 1, 4), ('LSQUARE',
                                     '[', 1, 4), ('NUMBER', 0, 1, 4),
            ('RSQUARE', ']', 1, 4), ('SEMICOL', ';', 1, 4)
        ]
        parser = Parser(tokens)
        parser.variables = {'my_arr': 'int*'}
        # Test the atom() (formerly factor()) part
        # It should return the base type ('int') and the C code
        self.assertEqual(parser.atom(), ('int', 'my_arr[0]'))

    def test_expr_array_access(self):
        tokens = [
            ('ID', 'my_arr', 1, 4), ('LSQUARE',
                                     '[', 1, 4), ('NUMBER', 0, 1, 4),
            ('RSQUARE', ']', 1, 4), ('PLUS', '+', 1, 4), ('NUMBER', 1, 1, 4),
            ('SEMICOL', ';', 1, 4)
        ]
        parser = Parser(tokens)
        parser.variables = {'my_arr': 'int*'}
        self.assertEqual(parser.expr(), ('int', 'my_arr[0] + 1'))

    def test_error_let_array_no_type(self):
        tokens = [
            ('LET', 'beg', 1, 4), ('ID', 'my_arr',
                                   1, 4), ('LSQUARE', '[', 1, 4),
            ('NUMBER', 10, 1, 4), ('RSQUARE', ']', 1, 4), ('SEMICOL', ';', 1, 4),
        ]
        parser = Parser(tokens)
        parser.variables = {}
        with self.assertRaises(SyntaxError) as cm:
            parser.statement()
        self.assertIn("must have an explicit type", str(cm.exception))

    def test_error_let_array_invalid_size(self):
        tokens = [
            ('LET', 'beg', 1, 4), ('TYPE', 'int', 1, 4), ('ID', 'my_arr', 1, 4),
            ('LSQUARE', '[', 1, 4), ('NUMBER',
                                     0, 1, 4), ('RSQUARE', ']', 1, 4),
            ('SEMICOL', ';', 1, 4),
        ]
        parser = Parser(tokens)
        parser.variables = {}
        with self.assertRaises(SyntaxError) as cm:
            parser.statement()
        self.assertIn("must be a positive integer", str(cm.exception))

    def test_error_id_stmt_array_type_mismatch(self):
        tokens = [
            ('ID', 'my_arr', 1, 4), ('LSQUARE',
                                     '[', 1, 4), ('NUMBER', 0, 1, 4),
            ('RSQUARE', ']', 1, 4), ('ASSIGN', '=',
                                     1, 4), ('STRING', '"hello"', 1, 4),
            ('SEMICOL', ';', 1, 4),
        ]
        parser = Parser(tokens)
        parser.variables = {'my_arr': 'int*'}
        with self.assertRaises(TypeError) as cm:
            parser.statement()
        self.assertIn(
            "cannot assign char* to array element of type int", str(cm.exception))

    def test_error_id_stmt_array_bad_index(self):
        tokens = [
            ('ID', 'my_arr', 1, 4), ('LSQUARE',
                                     '[', 1, 4), ('STRING', '"a"', 1, 4),
            ('RSQUARE', ']', 1, 4), ('ASSIGN', '=', 1, 4), ('NUMBER', 5, 1, 4),
            ('SEMICOL', ';', 1, 4),
        ]
        parser = Parser(tokens)
        parser.variables = {'my_arr': 'int*'}
        with self.assertRaises(TypeError) as cm:
            parser.statement()
        self.assertIn("Array index must be an integer", str(cm.exception))

    def test_error_id_stmt_indexing_non_array(self):
        tokens = [
            ('ID', 'my_var', 1, 4), ('LSQUARE',
                                     '[', 1, 4), ('NUMBER', 0, 1, 4),
            ('RSQUARE', ']', 1, 4), ('ASSIGN', '=', 1, 4), ('NUMBER', 5, 1, 4),
            ('SEMICOL', ';', 1, 4),
        ]
        parser = Parser(tokens)
        parser.variables = {'my_var': 'int'}  # my_var is an int, not int*
        with self.assertRaises(TypeError) as cm:
            parser.statement()
        self.assertIn("is not an array and cannot be indexed",
                      str(cm.exception))

    def test_error_atom_indexing_non_array(self):
        tokens = [
            ('ID', 'my_var', 1, 4), ('LSQUARE',
                                     '[', 1, 4), ('NUMBER', 0, 1, 4),
            ('RSQUARE', ']', 1, 4), ('SEMICOL', ';', 1, 4)
        ]
        parser = Parser(tokens)
        parser.variables = {'my_var': 'int'}  # my_var is an int, not int*
        with self.assertRaises(TypeError) as cm:
            parser.atom()
        self.assertIn("is not an array and cannot be indexed",
                      str(cm.exception))

    # --- Error Handling Tests (Updated and New) ---

    def test_invalid_statement(self):
        tokens = [('LPAREN', '(', 1, 4), ('RBRACE', '}', 2, 0)]
        parser = Parser(tokens)
        with self.assertRaises(SyntaxError) as cm:
            parser.statement()
        self.assertIn("Unexpected statement: LPAREN", str(cm.exception))

    def test_invalid_id_statement(self):
        tokens = [('ID', 'myvar', 1, 4), ('SEMICOL', ';', 1, 4)]
        parser = Parser(tokens)
        parser.variables = {'myvar': 'int'}  # Needs to be declared
        with self.assertRaises(SyntaxError) as cm:
            parser.statement()
        self.assertIn("Invalid statement start: myvar, line 1",
                      str(cm.exception))

    def test_expression_error_in_atom(self):
        tokens = [('PLUS', '+', 1, 0), self.eof_token]
        parser = Parser(tokens)
        with self.assertRaises(SyntaxError) as cm:
            parser.expr()  # This will fail down in atom()
        self.assertIn(
            "Unexpected token in atom: ('PLUS', '+', 1, 0)", str(cm.exception))

    def test_type_mismatch_let(self):
        tokens = [
            ('LET', 'beg', 1, 4), ('TYPE', 'int', 1,
                                   4), ('ID', 'x', 1, 4), ('ASSIGN', '=', 1, 4),
            ('STRING', '"hello"', 1, 4), ('SEMICOL', ';', 1, 4),
        ]
        parser = Parser(tokens)
        parser.variables = {}
        with self.assertRaises(TypeError) as cm:
            parser.statement()
        self.assertIn("Incompatible char* to int conversion",
                      str(cm.exception))

    def test_type_mismatch_return(self):
        tokens = [
            ('RETURN', 'return', 1, 4),
            ('STRING', '"hello"', 1, 4),
            ('SEMICOL', ';', 1, 4),
        ]
        parser = Parser(tokens)
        parser.fn_name = "my_func"
        # Function returns int, but we return string
        parser.env = {"my_func": "int"}
        with self.assertRaises(TypeError) as cm:
            parser.statement()
        self.assertIn("Incompatible char* to int conversion",
                      str(cm.exception))

    def test_undeclared_var_error(self):
        tokens = [
            ('ID', 'x', 1, 4), ('ASSIGN', '=', 1, 4), ('NUMBER', 10, 1, 4),
            ('SEMICOL', ';', 1, 4)
        ]
        parser = Parser(tokens)
        parser.variables = {}  # 'x' is not in variables
        with self.assertRaises(SyntaxError) as cm:
            parser.statement()
        self.assertIn("Undeclared identifier, x", str(cm.exception))

    def test_undeclared_fn_call_error(self):
        tokens = [
            ('ID', 'no_such_func', 1, 4), ('LPAREN', '(', 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4)
        ]
        parser = Parser(tokens)
        parser.variables = {}  # 'no_such_func' not in variables
        parser.env = {}  # 'no_such_func' not in env
        with self.assertRaises(ReferenceError) as cm:
            parser.statement()
        # This fails the 'id_stmt' check first
        self.assertIn(
            "Call to undeclared function 'no_such_func', line 1", str(cm.exception))

    def test_undeclared_fn_factor_error(self):
        tokens = [
            ('PRINT', 'boo', 1, 4), ('LPAREN', '(', 1, 4),
            ('ID', 'no_such_func', 1, 4), ('LPAREN', '(', 1, 4),
            ('RPAREN', ')', 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4)
        ]
        parser = Parser(tokens)
        parser.variables = {}
        parser.env = {}
        with self.assertRaises(ReferenceError) as cm:
            parser.statement()
        # This fails the 'atom' check
        self.assertIn(
            "Call to undeclared function 'no_such_func'", str(cm.exception))

    # --- NEW: Comment Tests ---

    def test_statement_comment(self):
        """Tests that a comment is correctly parsed as a statement."""
        tokens = [
            ('COMMENT', '// my comment', 1, 4),
            ('RBRACE', '}', 2, 0)  # End of function body
        ]
        parser = Parser(tokens)
        self.assertEqual(parser.statement(), '// my comment')

    def test_parse_comment_before_fn(self):
        """Tests parsing a file with a comment before a function."""
        tokens = [
            ('COMMENT', '// file header', 1, 0),
            ('FN', 'ah', 2, 0), ('ID', 'main', 2, 0), ('LPAREN', '(', 2, 0),
            ('RPAREN', ')', 2, 0), ('LBRACE', '{', 2, 0),
            ('RBRACE', '}', 3, 0),
            self.eof_token
        ]
        parser = Parser(tokens)
        expected_code = (
            '// file header\n'
            'int main() {\n'
            '\n'
            '}\n'
        )
        self.assertEqual(parser.parse(), expected_code)

    def test_parse_full_file_with_comments(self):
        """Tests a full parse with comments inside and outside a fn."""
        tokens = [
            ('COMMENT', '// start', 1, 0),
            ('FN', 'ah', 2, 0), ('ID', 'main', 2, 0), ('LPAREN', '(', 2, 0),
            ('RPAREN', ')', 2, 0), ('LBRACE', '{', 2, 0),
            ('COMMENT', '// do work', 3, 4),
            ('LET', 'beg', 4, 4), ('ID', 'x', 4, 4), ('ASSIGN', '=', 4, 4),
            ('NUMBER', 5, 4, 4), ('SEMICOL', ';', 4, 4),
            ('RBRACE', '}', 5, 0),
            self.eof_token
        ]
        parser = Parser(tokens)
        expected_code = (
            '// start\n'
            'int main() {\n'
            '    // do work\n'
            '    int x = 5;\n'
            '}\n'
        )
        self.assertEqual(parser.parse(), expected_code)

    def test_if_else_with_comment_before_else(self):
        """Tests that a comment between '}' and 'else' is parsed correctly."""
        tokens = [
            ('IF', 'if', 1, 4), ('ID', 'x', 1,
                                 4), ('GT', '>', 1, 4), ('NUMBER', 0, 1, 4),
            ('LBRACE', '{', 1, 4),
            ('ID', 'x', 2, 6), ('ASSIGN', '=', 2,
                                6), ('NUMBER', 1, 2, 6), ('SEMICOL', ';', 2, 6),
            ('RBRACE', '}', 3, 4),
            ('COMMENT', '// this comment should be preserved', 4, 4),
            ('ELSE', 'else', 5, 4), ('LBRACE', '{', 5, 4),
            ('ID', 'x', 6, 6), ('ASSIGN', '=', 6,
                                6), ('NUMBER', 0, 6, 6), ('SEMICOL', ';', 6, 6),
            ('RBRACE', '}', 7, 4),
            ('RBRACE', '}', 8, 0)  # Dummy end of function
        ]
        parser = Parser(tokens)
        parser.variables = {'x': 'int'}

        expected_code = (
            'if (x > 0) {\n'
            '        x = 1;\n'
            '    }\n'
            '    // this comment should be preserved\n'
            '    else {\n'
            '        x = 0;\n'
            '    }'
        )
        self.assertEqual(parser.statement(), expected_code)

    def test_if_else_if_else_statement(self):
        """Tests a full 'if / else if / else' chain."""
        tokens = [
            # if (x == 1) { x = 10; }
            ('IF', 'if', 1, 4), ('ID', 'x', 1,
                                 4), ('EQ', '==', 1, 4), ('NUMBER', 1, 1, 4),
            ('LBRACE', '{', 1, 4),
            ('ID', 'x', 2, 6), ('ASSIGN', '=', 2,
                                6), ('NUMBER', 10, 2, 6), ('SEMICOL', ';', 2, 6),
            ('RBRACE', '}', 3, 4),
            # else if (x == 2) { x = 20; }
            ('ELSE', 'else', 3, 4), ('IF', 'if', 3, 4), ('ID',
                                                         'x', 3, 4), ('EQ', '==', 3, 4), ('NUMBER', 2, 3, 4),
            ('LBRACE', '{', 3, 4),
            ('ID', 'x', 4, 6), ('ASSIGN', '=', 4,
                                6), ('NUMBER', 20, 4, 6), ('SEMICOL', ';', 4, 6),
            ('RBRACE', '}', 5, 4),
            # else { x = 30; }
            ('ELSE', 'else', 5, 4), ('LBRACE', '{', 5, 4),
            ('ID', 'x', 6, 6), ('ASSIGN', '=', 6,
                                6), ('NUMBER', 30, 6, 6), ('SEMICOL', ';', 6, 6),
            ('RBRACE', '}', 7, 4),
            ('RBRACE', '}', 8, 0)  # Dummy end of function
        ]
        parser = Parser(tokens)
        parser.variables = {'x': 'int'}

        expected_code = (
            'if (x == 1) {\n'
            '        x = 10;\n'
            '    } else if (x == 2) {\n'
            '        x = 20;\n'
            '    } else {\n'
            '        x = 30;\n'
            '    }'
        )
        self.assertEqual(parser.statement(), expected_code)


if __name__ == '__main__':
    unittest.main()
