"""
File: compiler.py
Author: David T.
Description: stage0-compiler for the language dav to c
"""

import sys
from .lexer.custom_token import *

src = open(sys.argv[1]).read()

c = src.replace("fn main()", "int main()").replace(
    "print(", "printf(\"%d\\n\", ")
c = "#include <stdio.h>\nint main() {\n" + \
    c.split("{", 1)[1].rsplit("}", 1)[0] + "\n}\n"

open(sys.argv[2], "w").write(c)
