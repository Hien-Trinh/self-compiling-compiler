import unittest
# Assuming the Parser class is in a file named parser.py
from python.parser.parser import Parser


class TestParser(unittest.TestCase):
    """
    Unit tests for the Parser class.
    Tokens are expected to be in the format: 
    (kind, value, line_number, optional_indentation_level)
    """

    def setUp(self):
        # A minimal set of tokens to represent the end of the file
        self.eof_token = ('EOF', '', 0, 0)

    # --- Utility Tests ---

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
            ('FN', 'fn', 1, 0), ('ID', 'main', 1, 0), ('LPAREN', '(', 1, 0),
            ('RPAREN', ')', 1, 0), ('LBRACE',
                                    '{', 1, 0), ('RBRACE', '}', 2, 0),
            self.eof_token
        ]
        parser = Parser(tokens)
        expected_code = (
            '#include <stdio.h>\n'
            '\n'
            'int main() {\n'
            '  return 0;\n'  # Note: 'main' function gets 'return 0;' automatically
            '}\n'
        )
        self.assertEqual(parser.parse(), expected_code)

    def test_parse_fn_with_params(self):
        tokens = [
            ('FN', 'fn', 1, 0), ('ID', 'add', 1, 0), ('LPAREN', '(', 1, 0),
            ('ID', 'a', 1, 0), ('COMMA', ',', 1, 0), ('ID', 'b', 1, 0),
            ('RPAREN', ')', 1, 0), ('LBRACE', '{', 1, 0),
            ('RETURN', 'return', 2, 4), ('ID', 'a', 2, 4), ('PLUS', '+', 2, 4),
            ('ID', 'b', 2, 4), ('SEMICOL', ';', 2, 4),
            ('RBRACE', '}', 3, 0),
            self.eof_token
        ]
        parser = Parser(tokens)
        expected_code = (
            '#include <stdio.h>\n'
            '\n'
            'int add(int a, int b) {\n'
            '  return (a + b);\n'
            '}\n'
        )
        self.assertEqual(parser.parse(), expected_code)

    # --- Statement Tests ---

    def test_let_statement_new_var(self):
        # Setup tokens for a single let statement inside a function
        tokens = [
            ('LET', 'let', 1, 4), ('ID', 'x', 1, 4), ('ASSIGN', '=', 1, 4),
            ('NUMBER', '5', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)  # End of function body
        ]
        parser = Parser(tokens)
        # Manually set up parser state as if inside fn_decl
        parser.variables = set()
        parser.expect_indent = 4  # This is not strictly used by parser, but good for context

        self.assertEqual(parser.statement(), 'int x = 5;')
        self.assertIn('x', parser.variables)

    def test_let_statement_reassignment(self):
        # Setup tokens for a single let statement inside a function
        tokens = [
            ('LET', 'let', 1, 4), ('ID', 'y', 1, 4), ('ASSIGN', '=', 1, 4),
            ('NUMBER', '10', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)  # End of function body
        ]
        parser = Parser(tokens)
        # 'y' is already defined (e.g., from parameters or a previous 'let')
        parser.variables = {'y'}

        self.assertEqual(parser.statement(), 'y = 10;')
        self.assertIn('y', parser.variables)

    def test_id_statement_assignment(self):
        tokens = [
            ('ID', 'myvar', 1, 4), ('ASSIGN', '=', 1, 4),
            ('NUMBER', '1', 1, 4), ('PLUS', '+', 1, 4), ('NUMBER', '2', 1, 4),
            ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)  # End of function body
        ]
        parser = Parser(tokens)
        parser.variables = {'myvar'}  # Must be defined to be assigned

        self.assertEqual(parser.statement(), 'myvar = (1 + 2);')

    def test_id_statement_fn_call(self):
        tokens = [
            ('ID', 'func', 1, 4), ('LPAREN', '(', 1, 4),
            ('NUMBER', '5', 1, 4), ('COMMA', ',', 1, 4),
            ('ID', 'z', 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)  # End of function body
        ]
        parser = Parser(tokens)
        parser.variables = set()  # Doesn't matter for function calls

        self.assertEqual(parser.statement(), 'func(5, z);')

    def test_print_statement(self):
        tokens = [
            ('PRINT', 'boo', 1, 4), ('LPAREN', '(', 1, 4),
            ('ID', 'val', 1, 4), ('MUL', '*', 1, 4), ('NUMBER', '2', 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)  # End of function body
        ]
        parser = Parser(tokens)
        parser.variables = set()

        self.assertEqual(parser.statement(), 'printf("%d\\n", (val * 2));')

    def test_return_statement(self):
        tokens = [
            ('RETURN', 'return', 1, 4),
            ('LPAREN', '(', 1, 4), ('NUMBER', '1', 1, 4),
            ('PLUS', '+', 1, 4), ('NUMBER', '1', 1, 4),
            ('RPAREN', ')', 1, 4), ('SEMICOL', ';', 1, 4),
            ('RBRACE', '}', 2, 0)  # End of function body
        ]
        parser = Parser(tokens)

        self.assertEqual(parser.statement(), 'return (1 + 1);')

    # --- Control Flow Tests ---

    def test_if_statement(self):
        tokens = [
            # if (x == 1) { print(x); }
            ('IF', 'if', 1, 4), ('ID', 'x', 1, 4), ('EQ',
                                                    '==', 1, 4), ('NUMBER', '1', 1, 4),
            ('LBRACE', '{', 1, 4),
            ('PRINT', 'print', 2, 6), ('LPAREN', '(', 2, 6), ('ID', 'x', 2, 6),
            ('RPAREN', ')', 2, 6), ('SEMICOL', ';', 2, 6),
            ('RBRACE', '}', 3, 4),
            ('RBRACE', '}', 4, 0)  # End of function body
        ]
        parser = Parser(tokens)
        parser.variables = {'x'}
        expected_code = (
            'if ((x == 1)) {\n'
            '      printf("%d\\n", x);\n'
            '}'
        )
        self.assertEqual(parser.statement(), expected_code)

    def test_if_else_statement(self):
        tokens = [
            # if (x > 0) { let y = 1; } else { y = 0; }
            ('IF', 'if', 1, 4), ('ID', 'x', 1, 4), ('GT',
                                                    '>', 1, 4), ('NUMBER', '0', 1, 4),
            ('LBRACE', '{', 1, 4),
            ('LET', 'let', 2, 6), ('ID', 'y', 2, 6), ('ASSIGN', '=',
                                                      2, 6), ('NUMBER', '1', 2, 6), ('SEMICOL', ';', 2, 6),
            ('RBRACE', '}', 3, 4),
            ('ELSE', 'else', 3, 4), ('LBRACE', '{', 3, 4),
            ('ID', 'y', 4, 6), ('ASSIGN', '=', 4,
                                6), ('NUMBER', '0', 4, 6), ('SEMICOL', ';', 4, 6),
            ('RBRACE', '}', 5, 4),
            ('RBRACE', '}', 6, 0)  # End of function body
        ]
        parser = Parser(tokens)
        parser.variables = {'x', 'y'}
        expected_code = (
            'if ((x > 0)) {\n'
            '      y = 1;\n'  # Since 'y' is in parser.variables
            '}'
            ' else {\n'
            '  y = 0;\n'
            '}'
        )
        self.assertEqual(parser.statement(), expected_code)

    def test_while_statement(self):
        tokens = [
            # while (i < 10) { i = i + 1; }
            ('WHILE', 'while', 1, 4), ('ID', 'i', 1,
                                       4), ('LT', '<', 1, 4), ('NUMBER', '10', 1, 4),
            ('LBRACE', '{', 1, 4),
            ('ID', 'i', 2, 6), ('ASSIGN', '=', 2, 6), ('ID', 'i', 2, 6), ('PLUS',
                                                                          '+', 2, 6), ('NUMBER', '1', 2, 6), ('SEMICOL', ';', 2, 6),
            ('RBRACE', '}', 3, 4),
            ('RBRACE', '}', 4, 0)  # End of function body
        ]
        parser = Parser(tokens)
        parser.variables = {'i'}
        expected_code = (
            'while ((i < 10)) {\n'
            '      i = (i + 1);\n'
            '}'
        )
        self.assertEqual(parser.statement(), expected_code)

    # --- Expression Tests ---

    def test_expression_arithmetic(self):
        tokens = [
            ('NUMBER', '1', 1, 0), ('PLUS', '+', 1, 0), ('NUMBER', '2', 1, 0),
            ('MUL', '*', 1, 0), ('NUMBER', '3', 1,
                                 0), ('SEMICOL', ';', 1, 0)  # Dummy end
        ]
        parser = Parser(tokens)
        # The parser respect typical operator precedence.
        # It parses as: (1 + (2 * 3))
        self.assertEqual(parser.expr(), '(1 + (2 * 3))')

    def test_expression_comparison(self):
        tokens = [
            ('ID', 'a', 1, 0), ('NE', '!=', 1, 0), ('NUMBER', '5', 1, 0),
            ('SEMICOL', ';', 1, 0)  # Dummy end
        ]
        parser = Parser(tokens)
        self.assertEqual(parser.expr(), '(a != 5)')

    def test_expression_parens(self):
        tokens = [
            ('LPAREN', '(', 1, 0), ('NUMBER', '1', 1, 0), ('PLUS', '+', 1, 0),
            ('NUMBER', '2', 1, 0), ('RPAREN', ')', 1, 0),
            ('MUL', '*', 1, 0), ('NUMBER', '3', 1,
                                 0), ('SEMICOL', ';', 1, 0)  # Dummy end
        ]
        parser = Parser(tokens)
        # Expression: (1 + 2) * 3
        # The inner expression `(1 + 2)` is fully resolved before being multiplied by 3
        self.assertEqual(parser.expr(), '((1 + 2) * 3)')

    def test_expression_fn_call(self):
        tokens = [
            # call(a, 10, b + 5)
            ('ID', 'call', 1, 0), ('LPAREN', '(', 1, 0),
            ('ID', 'a', 1, 0), ('COMMA', ',', 1, 0),
            ('NUMBER', '10', 1, 0), ('COMMA', ',', 1, 0),
            ('ID', 'b', 1, 0), ('PLUS', '+', 1, 0), ('NUMBER', '5', 1, 0),
            ('RPAREN', ')', 1, 0), ('SEMICOL', ';', 1, 0)  # Dummy end
        ]
        parser = Parser(tokens)
        # Factor handles the function call with arguments
        self.assertEqual(parser.factor(), 'call(a, 10, (b + 5))')

    # --- Error Handling Tests ---

    def test_invalid_statement(self):
        tokens = [('LPAREN', '(', 1, 4), ('RBRACE', '}', 2, 0)]
        parser = Parser(tokens)
        with self.assertRaises(SyntaxError) as cm:
            parser.statement()
        self.assertIn("Unexpected statement: LPAREN", str(cm.exception))

    def test_invalid_id_statement(self):
        # Tokens for 'myvar ;' which is not a valid statement
        tokens = [('ID', 'myvar', 1, 4), ('SEMICOL', ';', 1, 4)]
        parser = Parser(tokens)
        with self.assertRaises(SyntaxError) as cm:
            parser.statement()
        self.assertIn("Invalid statement start: myvar, line 1",
                      str(cm.exception))

    def test_expression_error_in_factor(self):
        # Tokens starting with a PLUS in an expression where it expects a term/factor
        tokens = [('PLUS', '+', 1, 0), self.eof_token]
        parser = Parser(tokens)
        with self.assertRaises(SyntaxError) as cm:
            parser.expr()
        self.assertIn(
            "Unexpected token in factor: ('PLUS', '+', 1, 0)", str(cm.exception))


if __name__ == '__main__':
    unittest.main()
