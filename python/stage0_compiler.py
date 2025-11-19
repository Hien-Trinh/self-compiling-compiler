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
    c_code = C_INCLUDE
    c_code += C_PROTOTYPE
    c_code += parser.parse()
    c_code += C_HELPERS
    open(dst, 'w').write(c_code)
    print(f"[ok] Generated {dst}")


C_INCLUDE = \
    "#include <stdio.h>\n" \
    "#include <stdlib.h>\n" \
    "#include <string.h>\n" \
    "\n"

C_PROTOTYPE = \
    "char* concat(char* str1, char* str2);\n" \
    "char* itos(int x);\n" \
    "char* ctos(char c);\n" \
    "char* read_file(char* path);\n" \
    "void write_file(char* path, char* content);\n" \
    "\n"

C_HELPERS = \
    "\n" \
    "char* concat(char* str1, char* str2) {\n" \
    "    static char buf[1024];\n" \
    "    snprintf(buf, sizeof(buf), \"%s%s\", str1, str2);\n" \
    "    return buf;\n" \
    "}\n" \
    "\n" \
    "char* itos(int x) {\n" \
    "    static char buf[32];\n" \
    "    snprintf(buf, sizeof(buf), \"%d\", x);\n" \
    "    return buf;\n" \
    "}\n" \
    "\n" \
    "char* ctos(char c) {\n" \
    "    static char buf[2];\n" \
    "    buf[0] = c;\n" \
    "    buf[1] = '\\0';\n" \
    "    return buf;\n" \
    "}\n" \
    "\n" \
    "char* read_file(char* path) {\n" \
    "    FILE* f = fopen(path, \"rb\");\n" \
    "    if (!f) return NULL;\n" \
    "    fseek(f, 0, SEEK_END);\n" \
    "    long len = ftell(f);\n" \
    "    fseek(f, 0, SEEK_SET);\n" \
    "    char* buf = malloc(len + 1);\n" \
    "    if (buf) {\n" \
    "        fread(buf, 1, len, f);\n" \
    "        buf[len] = '\\0';\n" \
    "    }\n" \
    "    fclose(f);\n" \
    "    return buf;\n" \
    "}\n" \
    "\n" \
    "void write_file(char* path, char* content) {\n" \
    "    FILE* f = fopen(path, \"w\");\n" \
    "    if (!f) return;\n" \
    "    fprintf(f, \"%s\", content);\n" \
    "    fclose(f);\n" \
    "}\n" \
    "\n"

if __name__ == '__main__':
    main()
