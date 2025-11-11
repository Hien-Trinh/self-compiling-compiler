"""
File: test_lexer.py
Author: David T.
Description: testing for lexer
Reference: https://docs.python.org/3/library/re.html#writing-a-tokenizer
"""

import unittest
from python.lexer.custom_token import Token
from python.lexer.lexer import tokenize


class TokenizerTest(unittest.TestCase):

    def assertTokensEqual(self, actual, expected):
        """Helper to assert that two token lists are identical."""
        self.assertEqual(actual, expected)

    def test_empty_code(self):
        """Tests tokenizing an empty string."""
        expected = [Token('EOF', None, 1, 0)]
        self.assertTokensEqual(tokenize(""), expected)

    def test_basic_tokens_and_skip(self):
        """Tests simple IDs, assignment, and correct handling of whitespace."""
        code = "x = 10 ;"
        expected = [
            Token('ID', 'x', 1, 0),
            Token('ASSIGN', '=', 1, 2),
            Token('NUMBER', 10, 1, 4),
            Token('SEMICOL', ';', 1, 7),
            Token('EOF', None, 1, 8)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_keywords(self):
        """Tests that identifiers matching keywords are correctly re-classified."""
        code = "ah beg if else while return"
        expected = [
            Token('FN', 'ah', 1, 0),
            Token('LET', 'beg', 1, 3),
            Token('IF', 'if', 1, 7),
            Token('ELSE', 'else', 1, 10),
            Token('WHILE', 'while', 1, 15),
            Token('RETURN', 'return', 1, 21),
            Token('EOF', None, 1, 27)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_numbers_integers_and_floats(self):
        """Tests integer and float conversion (based on the adjusted regex)."""
        code = "123 45.6 78."
        expected = [
            Token('NUMBER', 123, 1, 0),
            Token('NUMBER', 45.6, 1, 4),
            Token('NUMBER', 78.0, 1, 9),
            Token('EOF', None, 1, 12)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_type_tokens(self):
        """Tests that type keywords are correctly tokenized."""
        code = "int char char* int*"
        expected = [
            Token('TYPE', 'int', 1, 0),
            Token('TYPE', 'char', 1, 4),
            Token('TYPE', 'char*', 1, 9),
            Token('TYPE', 'int*', 1, 15),
            Token('EOF', None, 1, 19)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_operators_and_symbols(self):
        """Tests all supported operators and grouping symbols."""
        code = "a + (b - c) * d / e == f != g < h > i"
        expected = [
            Token('ID', 'a', 1, 0),
            Token('PLUS', '+', 1, 2),
            Token('LPAREN', '(', 1, 4),
            Token('ID', 'b', 1, 5),
            Token('MINUS', '-', 1, 7),
            Token('ID', 'c', 1, 9),
            Token('RPAREN', ')', 1, 10),
            Token('MUL', '*', 1, 12),
            Token('ID', 'd', 1, 14),
            Token('DIV', '/', 1, 16),
            Token('ID', 'e', 1, 18),
            Token('EQ', '==', 1, 20),
            Token('ID', 'f', 1, 23),
            Token('NE', '!=', 1, 25),
            Token('ID', 'g', 1, 28),
            Token('LT', '<', 1, 30),
            Token('ID', 'h', 1, 32),
            Token('GT', '>', 1, 34),
            Token('ID', 'i', 1, 36),
            Token('EOF', None, 1, 37)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_new_operators_and_brackets(self):
        """Tests >=, <=, &&, ||, [], and comments."""
        code = "a >= b && c <= d || e[f] // comment"
        expected = [
            Token('ID', 'a', 1, 0),
            Token('GE', '>=', 1, 2),
            Token('ID', 'b', 1, 5),
            Token('AND', '&&', 1, 7),
            Token('ID', 'c', 1, 10),
            Token('LE', '<=', 1, 12),
            Token('ID', 'd', 1, 15),
            Token('OR', '||', 1, 17),
            Token('ID', 'e', 1, 20),
            Token('LSQUARE', '[', 1, 21),
            Token('ID', 'f', 1, 22),
            Token('RSQUARE', ']', 1, 23),
            Token('COMMENT', '// comment', 1, 25),
            Token('EOF', None, 1, 35)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_multiline_code_and_position(self):
        """Tests correct line and column tracking across multiple lines."""
        code = "beg x = 1\nif x == 1 {\nboo x}"
        expected = [
            Token('LET', 'beg', 1, 0),
            Token('ID', 'x', 1, 4),
            Token('ASSIGN', '=', 1, 6),
            Token('NUMBER', 1, 1, 8),

            Token('IF', 'if', 2, 0),  # New line, column 1
            Token('ID', 'x', 2, 3),
            Token('EQ', '==', 2, 5),
            Token('NUMBER', 1, 2, 8),
            Token('LBRACE', '{', 2, 10),

            Token('PRINT', 'boo', 3, 0),  # New line, column 1
            Token('ID', 'x', 3, 4),
            Token('RBRACE', '}', 3, 5),
            Token('EOF', None, 3, 6)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_string_literal(self):
        """Tests a simple string literal."""
        code = 'beg s = "hello world"'
        expected = [
            Token('LET', 'beg', 1, 0),
            Token('ID', 's', 1, 4),
            Token('ASSIGN', '=', 1, 6),
            Token('STRING', '"hello world"', 1, 8),
            Token('EOF', None, 1, 21)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_string_with_escapes(self):
        """Tests string literals with various escape sequences."""
        # Use a python raw string for the test code
        code = r'"\"\\\n\t"'
        expected = [
            # The token's value includes the quotes and the raw escapes
            Token('STRING', r'"\"\\\n\t"', 1, 0),
            Token('EOF', None, 1, 10)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_char_literal(self):
        """Tests a simple character literal."""
        code = "beg c = 'a'"
        expected = [
            Token('LET', 'beg', 1, 0),
            Token('ID', 'c', 1, 4),
            Token('ASSIGN', '=', 1, 6),
            Token('CHAR', "'a'", 1, 8),
            Token('EOF', None, 1, 11)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_char_with_escapes(self):
        """Tests char literals with various escape sequences."""
        code = r"'\'' '\\' '\n' '\t'"
        expected = [
            Token('CHAR', r"'\''", 1, 0),
            Token('CHAR', r"'\\'", 1, 5),
            Token('CHAR', r"'\n'", 1, 10),
            Token('CHAR', r"'\t'", 1, 15),
            Token('EOF', None, 1, 19)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_column_tracking_with_tabs(self):
        """Tests column tracking when mixing spaces and tabs (tabs are treated as a single space by SKIP)."""
        code = "a\t=\t1"
        expected = [
            Token('ID', 'a', 1, 0),
            Token('ASSIGN', '=', 1, 2),
            Token('NUMBER', 1, 1, 4),
            Token('EOF', None, 1, 5)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_mismatch_error(self):
        """Tests that an unexpected character raises the correct error."""
        code = "x = $ 10"
        with self.assertRaisesRegex(SyntaxError, r"Unexpected character '\$' on line 1"):
            tokenize(code)

    def test_mismatch_error_multiline(self):
        """Tests that an unexpected character on a later line reports the correct line number."""
        code = "x = 10\n y @ 20"
        with self.assertRaisesRegex(SyntaxError, r"Unexpected character '@' on line 2"):
            tokenize(code)

    def test_comment_token(self):
        """Tests that a comment is correctly tokenized."""
        code = "// this is a comment"
        expected = [
            # The value is the full text, including the '//'
            Token('COMMENT', '// this is a comment', 1, 0),
            Token('EOF', None, 1, 20)
        ]
        self.assertTokensEqual(tokenize(code), expected)

    def test_comment_with_code_and_newline(self):
        """Tests that comments and newlines are handled correctly."""
        code = "x = 1 // comment\ny = 2"
        expected = [
            Token('ID', 'x', 1, 0),
            Token('ASSIGN', '=', 1, 2),
            Token('NUMBER', 1, 1, 4),
            Token('COMMENT', '// comment', 1, 6),
            # The newline token is correctly skipped and not part of the comment
            Token('ID', 'y', 2, 0),
            Token('ASSIGN', '=', 2, 2),
            Token('NUMBER', 2, 2, 4),
            Token('EOF', None, 2, 5)
        ]
        # This test will fail if you have the line-counting bug in lexer.py
        # 'y' will incorrectly be on line 3
        self.assertTokensEqual(tokenize(code), expected)

    def test_comment_at_eof(self):
        """Tests a file that ends with a comment."""
        code = "x = 1 // eof"
        expected = [
            Token('ID', 'x', 1, 0),
            Token('ASSIGN', '=', 1, 2),
            Token('NUMBER', 1, 1, 4),
            Token('COMMENT', '// eof', 1, 6),
            Token('EOF', None, 1, 12)
        ]
        self.assertTokensEqual(tokenize(code), expected)


if __name__ == '__main__':
    unittest.main()
