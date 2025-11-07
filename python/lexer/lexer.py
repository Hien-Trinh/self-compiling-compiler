"""
File: lexer.py
Author: David T.
Description: lexical analysis of program to generate list of tokens
Reference: https://docs.python.org/3/library/re.html#writing-a-tokenizer
"""

import re
from .custom_token import Token, TOKEN_REGEX, KEYWORDS


def tokenize(code):
    """
    Converts source code string into a list of Token namedtuples.
    Tracks line and column numbers for each token.
    """
    tokens = []
    line_num = 1
    line_start = 0

    # Initialize 'column' to 0 for the EOF token if the input code is empty
    column = 0

    for mo in re.finditer(TOKEN_REGEX, code):
        kind = mo.lastgroup
        value = mo.group()
        # Calculate column based on match start position relative to the line's start
        column = mo.start() - line_start

        if kind == 'NUMBER':
            # Converts to float if decimal is present, otherwise integer
            value = float(value) if '.' in value else int(value)
        elif kind == 'ID' and value in KEYWORDS:
            kind = KEYWORDS[value]
        elif kind == 'NEWLINE':
            # Update line tracking variables and skip adding token
            line_start = mo.end()
            line_num += 1
            continue
        elif kind == 'SKIP':
            # Skip whitespace and tabs
            continue
        elif kind == 'MISMATCH':
            # Handles unexpected characters
            raise SyntaxError(
                f'Unexpected character {value!r} on line {line_num}')

        # Append the successfully matched token
        tokens.append(Token(kind, value, line_num, column))

    # Determine the column for the EOF token
    eof_column = mo.end() - line_start if 'mo' in locals() else 0

    # Append the mandatory End-Of-File token
    tokens.append(Token('EOF', None, line_num, eof_column))

    return tokens
