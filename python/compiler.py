"""
File: compiler.py
Author: David T.
Description: stage0-compiler for the language dav to c
"""

import sys
from lexer.lexer import tokenize
from parser.parser import Parser


def main():
    if len(sys.argv) != 3:
        print("Usage: python3 python/compiler.py compiler.dav compiler.c")
        return
    src, dst = sys.argv[1], sys.argv[2]
    code = open(src).read()
    tokens = tokenize(code)
    parser = Parser(tokens)
    c_code = parser.parse()
    open(dst, 'w').write(c_code)
    print(f"[ok] Generated {dst}")


if __name__ == '__main__':
    main()
