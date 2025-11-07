"""
File: token.py
Author: David T.
Description: token regex and reserved keywords
Reference: https://docs.python.org/3/library/re.html#writing-a-tokenizer
"""

from typing import NamedTuple


class Token(NamedTuple):
    type: str
    value: str | int | float | None
    line: int
    column: int


TOKEN_SPEC = [
    ('NUMBER',   r'\d+(\.\d*)?'),  # Integer or decimal number
    ('ID',       r'[A-Za-z_][A-Za-z0-9_]*'),
    ('EQ',        r'=='),
    ('NE',        r'!='),
    ('ASSIGN',    r'='),
    ('SEMICOL',   r';'),
    ('LPAREN',    r'\('),
    ('RPAREN',    r'\)'),
    ('LBRACE',    r'\{'),
    ('RBRACE',    r'\}'),
    ('PLUS',      r'\+'),
    ('MINUS',     r'-'),
    ('MUL',       r'\*'),
    ('DIV',       r'/'),
    ('LT',        r'<'),
    ('GT',        r'>'),
    ('COMMA',     r','),
    ('NEWLINE',   r'\n'),
    ('SKIP',      r'[ \t]+'),
    ('MISMATCH',  r'.')
]

TOKEN_REGEX = "|".join(
    f'(?P<{name}>{pattern})' for name, pattern in TOKEN_SPEC)

KEYWORDS = {
    'ah': 'FN',
    'beg': 'LET',
    'boo': 'PRINT',
    'if': 'IF',
    'else': 'ELSE',
    'while': 'WHILE',
    'return': 'RETURN'
}
