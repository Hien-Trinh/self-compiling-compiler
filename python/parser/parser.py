"""
File: parser.py
Author: David T.
Description: parser takes in a list of tokens and output c code
"""


class Parser:
    """
    A parser that works...
    """

    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
        self.fn_name = ""
        self.variables = {}
        self.env = {}

    # Returns kind of the next token
    def peek(self):
        return self.tokens[self.pos][0]

    # Returns current token, moves pointer forward
    def next(self):
        tok = self.tokens[self.pos]
        self.pos += 1
        return tok

    # Check kind if expected, returns current token, moves pointer forward
    def expect(self, kind):
        tok = self.next()
        if tok[0] != kind:
            raise SyntaxError(f'Expected {kind}, got {tok}, line {tok[2]}')
        return tok

    # Essential C, parse till EOF
    def parse(self):
        code = []
        while self.peek() != 'EOF':
            code.append(self.fn_decl())
        return '\n'.join(code)

    # ==============================================================
    # Functions

    def fn_decl(self):
        """
        ah int name(int integer1, integer2, char* str1) {
            body;
        }
        """
        if self.peek() == 'COMMENT':
            return self.comment_stmt()

        indent = self.expect('FN')[3]

        ret_type = 'int'  # Set default return type to int if no type given
        if self.peek() == 'TYPE':
            ret_type = self.expect('TYPE')[1]
        self.fn_name = self.expect('ID')[1]

        # Set global environment function
        self.env[self.fn_name] = ret_type

        self.expect('LPAREN')
        params = []
        while self.peek() != 'RPAREN':
            # Set default param type to int if no type given
            ptype = 'int'
            if self.peek() == 'TYPE':
                ptype = self.expect('TYPE')[1]
            params.append((ptype, self.expect('ID')[1]))
            if self.peek() == 'COMMA':
                self.next()

        self.expect('RPAREN')
        self.expect('LBRACE')
        self.variables = {p: t for t, p in params}
        body = []
        while self.peek() not in ('RBRACE', 'EOF'):
            body.append(self.statement())
        self.expect('RBRACE')
        param_list = ', '.join(f'{ptype} {pname}' for ptype, pname in params)
        result = f'{ret_type} {self.fn_name}({param_list}) {{\n'
        result += '\n'.join(' ' * indent + '    ' + s for s in body)
        result += '\n}\n'
        return result

    # ==============================================================
    # Statements

    def statement(self):
        t = self.peek()
        if t == 'LET':
            return self.let_stmt()
        elif t == 'PRINT':
            return self.print_stmt()
        elif t == 'IF':
            return self.if_stmt()
        elif t == 'WHILE':
            return self.while_stmt()
        elif t == 'RETURN':
            return self.return_stmt()
        elif t == 'ID':
            return self.id_stmt()
        elif t == 'COMMENT':
            return self.comment_stmt()
        else:
            raise SyntaxError(f'Unexpected statement: {t}')

    def let_stmt(self):
        line_num = self.expect('LET')[2]
        # Set variable type to undefined as default
        var_type = 'undefined'
        if self.peek() == 'TYPE':
            var_type = self.expect('TYPE')[1]

        var_name = self.expect('ID')[1]
        if self.peek() == 'ASSIGN':
            # Variable assignment
            # e.g., beg x = 10; beg char* y = "howdie"
            self.next()

            # Dynamically set type to expression type
            rhs_type, rhs_expr = self.expr()
            if var_type == 'undefined':
                var_type = rhs_type  # Infer type
            elif var_type != rhs_type:
                raise TypeError(
                    f'Incompatible {rhs_type} to {var_type} conversion, line {line_num}')

            self.expect('SEMICOL')
            if var_name in self.variables:
                if self.variables[var_name] == var_type:
                    return f'{var_name} = {rhs_expr};'
                else:
                    raise TypeError(
                        f'Redefinition of \'{var_name}\' with a different type: \'{var_type}\' vs \'{self.variables[var_name]}\', line {line_num}')
            else:
                self.variables[var_name] = var_type
                return f'{var_type} {var_name} = {rhs_expr};'

        elif self.peek() == 'LSQUARE':
            # Array declaration
            # e.g., beg int x[10];
            self.next()
            if var_type == 'undefined':
                raise SyntaxError(
                    f'Array declaration must have an explicit type, line {line_num}')

            size = self.expect('NUMBER')[1]
            if not isinstance(size, int) or size < 1:
                raise SyntaxError(
                    f'Array size must be a positive integer, got {size}, line {line_num}')

            self.expect('RSQUARE')
            self.expect('SEMICOL')

            if var_name in self.variables:
                raise TypeError(
                    f'Redefinition of \'{var_name}\', line {line_num}')

            # Store array type as 'base_type*' (e.g., 'int*')
            array_type = var_type + '*'
            self.variables[var_name] = array_type

            # C code for array declaration
            return f'{var_type} {var_name}[{size}];'

        else:
            raise SyntaxError(
                f'Expected \'=\' or \'[\' after identifier in let statement, line {line_num}')

    def print_stmt(self):
        line_num = self.expect('PRINT')[2]
        self.expect('LPAREN')
        message_type, message_expr = self.expr()
        self.expect('RPAREN')
        self.expect('SEMICOL')
        if message_type == 'int':
            return f'printf("%d\\n", {message_expr});'
        elif message_type == 'char':
            return f'printf("%c\\n", {message_expr});'
        elif message_type == 'char*':
            return f'printf("%s\\n", {message_expr});'
        else:
            raise TypeError(
                f'Unprintable expression of type: {message_type}, line {line_num}')

    def id_stmt(self):
        expect = self.expect('ID')
        name = expect[1]
        line_num = expect[2]

        if self.peek() == 'ASSIGN':
            # Variable assignment
            self.next()
            # Check for assignment: MUST be in local variables
            if name not in self.variables:
                raise SyntaxError(
                    f'Undeclared identifier, {name}, line {line_num}')

            rhs_type, rhs_expr = self.expr()
            if self.variables[name] != rhs_type:
                raise TypeError(
                    f'Incompatible {rhs_type} to {self.variables[name]} conversion, line {line_num}')

            self.expect('SEMICOL')
            return f'{name} = {rhs_expr};'

        elif self.peek() == 'LPAREN':
            # Function call
            self.next()
            # Check for function call: MUST be in global environment
            if name not in self.env:
                raise ReferenceError(
                    f'Call to undeclared function \'{name}\', line {line_num}')

            args = []
            if self.peek() != 'RPAREN':
                args.append(str(self.expr()[1]))
                while self.peek() == 'COMMA':
                    self.next()
                    args.append(str(self.expr()[1]))

            self.expect('RPAREN')
            self.expect('SEMICOL')
            return f'{name}({", ".join(args)});'

        elif self.peek() == 'LSQUARE':
            # Array assignment
            self.next()

            if name not in self.variables:
                raise ReferenceError(
                    f'Use of undeclared identifier \'{name}\', line {line_num}')

            # Check type. Must be a pointer type, e.g., 'int*'
            var_type = self.variables[name]
            if not var_type[-1] == '*':
                raise TypeError(
                    f'Variable \'{name}\' is not an array and cannot be indexed, line {line_num}')

            # Get index expression
            index_type, index_expr = self.expr()
            if index_type != 'int':
                raise TypeError(
                    f'Array index must be an integer, got {index_type}, line {line_num}')

            self.expect('RSQUARE')
            self.expect('ASSIGN')

            rhs_type, rhs_expr = self.expr()
            # Type check the assignment
            base_type = var_type[:-1]
            if base_type != rhs_type:
                raise TypeError(
                    f'Incompatible types: cannot assign {rhs_type} to array element of type {base_type}, line {line_num}')

            self.expect('SEMICOL')
            return f'{name}[{index_expr}] = {rhs_expr};'

        else:
            raise SyntaxError(
                f'Invalid statement start: {name}, line {line_num}')

    def if_stmt(self):
        else_indent = ' '
        indent = self.expect('IF')[3]
        cond = self.expr()[1]
        self.expect('LBRACE')
        then_body = []
        while self.peek() not in ('RBRACE', 'EOF'):
            then_body.append(self.statement())
        self.expect('RBRACE')
        code = f'if ({cond}) {{\n'
        code += '\n'.join(' ' * indent + '    ' + s for s in then_body)
        code += f'\n{' ' * indent}}}'

        # Consume any comments that appear between '}' and 'else'
        # and append them to the generated code.
        if self.peek() == 'COMMENT':
            code += '\n'
            else_indent = ' ' * indent
        while self.peek() == 'COMMENT':
            code += f'{else_indent}{self.comment_stmt()}\n'

        if self.peek() == 'ELSE':
            self.next()
            self.expect('LBRACE')
            else_body = []
            while self.peek() not in ('RBRACE', 'EOF'):
                else_body.append(self.statement())
            self.expect('RBRACE')
            code += f'{else_indent}else {{\n'
            code += '\n'.join(' ' * indent + '    ' + s for s in else_body)
            code += f'\n{' ' * indent}}}'
        return code

    def while_stmt(self):
        indent = self.expect('WHILE')[3]
        cond = self.expr()[1]
        self.expect('LBRACE')
        body = []
        while self.peek() not in ('RBRACE', 'EOF'):
            body.append(self.statement())
        self.expect('RBRACE')
        code = f'while ({cond}) {{\n'
        code += '\n'.join(' ' * indent + '    ' + s for s in body)
        code += f'\n{' ' * indent}}}'
        return code

    def comment_stmt(self):
        return f'{self.next()[1]}'

    def return_stmt(self):
        line_num = self.expect('RETURN')[2]
        rhs_type, rhs_expr = self.expr()
        ret_type = self.env[self.fn_name]
        if rhs_type != ret_type:
            raise TypeError(
                f'Incompatible {rhs_type} to {ret_type} conversion, line {line_num}')
        self.expect('SEMICOL')
        return f'return {rhs_expr};'

    # ==============================================================
    # Expressions
    # returns type, value
    def expr(self):
        return self.logical()

    def logical(self):
        res_type, result = self.relational()
        while self.peek() in ('OR', 'AND'):
            op = self.next()[1]
            rhs = self.relational()[1]
            res_type, result = 'int', f'({result[1]} {op} {rhs})'
        return res_type, result

    def relational(self):
        res_type, result = self.additive()
        while self.peek() in ('EQ', 'NE', 'LT', 'GT', 'LE', 'GE'):
            op = self.next()[1]
            rhs_type, rhs = self.additive()
            if res_type == 'char*' and rhs_type == 'char*':
                if op == "==":
                    result = 'int', f'(strcmp({result}, {rhs}) == 0)'
                elif op == '!=':
                    result = 'int', f'(strcmp({result}, {rhs}) != 0)'
            elif res_type == 'char*' or rhs_type == 'char*':
                raise TypeError(
                    f'Operation \'{op}\' not allowed between \'{res_type}\' and \'{rhs_type}\'')
            else:
                res_type, result = 'int', f'({result} {op} {rhs})'
        return res_type, result

    def additive(self):
        res_type, result = self.multiplicative()
        while self.peek() in ('PLUS', 'MINUS'):
            op = self.next()[1]
            rhs_type, rhs = self.multiplicative()
            if res_type == 'char*' or rhs_type == 'char*':
                if op == "+":
                    res_type, result = 'char*', f'({self.string_plus((res_type, result), (rhs_type, rhs))})'
                else:
                    raise TypeError(
                        f'Operation \'{op}\' not allowed between \'{res_type}\' and \'{rhs_type}\'')
            else:
                res_type, result = 'int', f'({result} {op} {rhs})'
        return res_type, result

    def multiplicative(self):
        res_type, result = self.atom()
        while self.peek() in ('MUL', 'DIV'):
            op = self.next()[1]
            rhs_type, rhs = self.atom()
            if res_type == 'char*' or rhs_type == 'char*':
                raise TypeError(
                    f'Operation \'{op}\' not allowed between \'{res_type}\' and \'{rhs_type}\'')
            res_type, result = 'int', f'({result} {op} {rhs})'
        return res_type, result

    def atom(self):
        tok = self.next()
        var_name = tok[1]
        line_num = tok[2]
        if tok[0] == 'NUMBER':
            return 'int', var_name
        elif tok[0] == 'CHAR':
            return 'char', var_name
        elif tok[0] == 'STRING':
            return 'char*', var_name
        elif tok[0] == 'ID':
            if self.peek() == 'LPAREN':
                # Function call
                if var_name not in self.env:
                    raise ReferenceError(
                        f'Call to undeclared function \'{var_name}\', line {line_num}')
                self.next()

                args = []
                if self.peek() != 'RPAREN':
                    args.append(str(self.expr()[1]))
                    while self.peek() == 'COMMA':
                        self.next()
                        args.append(str(self.expr()[1]))
                self.expect('RPAREN')
                return self.env[var_name], f'{var_name}({", ".join(args)})'

            elif self.peek() == 'LSQUARE':
                # Array access
                if var_name not in self.variables:
                    raise ReferenceError(
                        f'Use of undeclared identifier \'{var_name}\', line {line_num}')
                self.next()

                # Check type. Must be a pointer type, e.g., 'int*'
                var_type = self.variables[var_name]
                if not var_type.endswith('*'):
                    raise TypeError(
                        f'Variable \'{var_name}\' is not an array and cannot be indexed, line {line_num}')

                # Get index expression
                index_type, index_expr = self.expr()
                if index_type != 'int':
                    raise TypeError(
                        f'Array index must be an integer, got {index_type}, line {line_num}')

                self.expect('RSQUARE')
                c_code = f'{var_name}[{index_expr}]'
                base_type = var_type[:-1]
                return base_type, c_code

            else:
                # Just a variable
                if var_name not in self.variables:
                    raise ReferenceError(
                        f'Use of undeclared identifier \'{var_name}\', line {line_num}')
                return self.variables[var_name], var_name

        elif tok[0] == 'LPAREN':
            expr_type, expr = self.expr()
            self.expect('RPAREN')
            return expr_type, expr
        else:
            raise SyntaxError(
                f'Unexpected token in atom: {tok}, line {tok[2]}')

    # ==============================================================
    # Utils

    def string_plus(self, a, b):
        str_a = self.to_string(a)
        str_b = self.to_string(b)
        output = f'concat({str_a}, {str_b})'
        return output

    def to_string(self, var):
        expr_type, expr = var
        if expr_type == 'int':
            return f'itos({expr})'
        elif expr_type == 'char':
            return f'ctos({expr})'
        elif expr_type == 'char*':
            return expr
