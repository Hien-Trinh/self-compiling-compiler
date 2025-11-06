'''
Stage-0 compiler for dav
'''

import sys

src = open(sys.argv[1]).read()

c = src.replace("fn main()", "int main()").replace(
    "print(", "printf(\"%d\\n\", ")
c = "#include <stdio.h>\nint main() {\n" + \
    c.split("{", 1)[1].rsplit("}", 1)[0] + "\n}\n"

open(sys.argv[2], "w").write(c)
