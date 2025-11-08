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

    def peek(self):
        return self.tokens[self.pos][0]

    def next(self):
        tok = self.tokens[self.pos]
        self.pos += 1
        return tok

    def expect(self, kind):
        tok = self.next()
        if tok[0] != kind:
            raise SyntaxError(f'Expected {kind}, got {tok}, line {tok[2]}')
        return tok

    def parse(self):
        code = [
            '#include <stdio.h>',
            '#include <string.h>',
            ''
        ]
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

        self.next()  # Must be RPAREN
        self.expect('LBRACE')
        self.variables = {p: t for t, p in params}
        self.variables[self.fn_name] = ret_type
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
        else:
            raise SyntaxError(f'Unexpected statement: {t}')

    def let_stmt(self):
        line_num = self.expect('LET')[2]
        # Set variable type to undefined as default
        var_type = 'undefined'
        if self.peek() == 'TYPE':
            var_type = self.expect('TYPE')[1]
        var_name = self.expect('ID')[1]
        self.expect('ASSIGN')

        # Dynamically set type to expression type
        expr_type, expr = self.expr()
        if var_type != 'undefined' and var_type != expr_type:
            raise TypeError(
                f'Incompatible {expr_type} to {var_type} conversion, line {line_num}')
        var_type = expr_type

        self.expect('SEMICOL')
        if var_name in self.variables:
            if var_type == self.variables[var_name]:
                return f'{var_name} = {expr};'
            else:
                raise TypeError(
                    f'Redefinition of \'{var_name}\' with a different type: \'{var_type}\' vs \'{self.variables[var_name]}\', line {line_num}')
        else:
            self.variables[var_name] = var_type
            return f'{var_type} {var_name} = {expr};'

    def print_stmt(self):
        line_num = self.expect('PRINT')[2]
        self.expect('LPAREN')
        expr_type, expr = self.expr()
        self.expect('RPAREN')
        self.expect('SEMICOL')
        if expr_type == 'int':
            return f'printf("%d\\n", {expr});'
        elif expr_type == 'char':
            return f'printf("%c\\n", {expr});'
        elif expr_type == 'char*':
            return f'printf("%s\\n", {expr});'
        else:
            raise TypeError(
                f'Unprintable expression of type: {expr_type}, line {line_num}')

    def id_stmt(self):
        expect = self.expect('ID')
        name = expect[1]
        line_num = expect[2]

        # Check if ID declared
        if name not in self.variables:
            raise SyntaxError(
                f'Undeclared identifier, {name}, line {line_num}')

        if self.peek() == 'ASSIGN':
            self.next()
            expr_type, expr = self.expr()
            self.expect('SEMICOL')
            return f'{name} = {expr};'
        elif self.peek() == 'LPAREN':
            self.next()
            args = []
            if self.peek() != 'RPAREN':
                while True:
                    args.append(self.expr()[1])
                    if self.peek() == 'COMMA':
                        self.next()
                        continue
                    break
            self.expect('RPAREN')
            self.expect('SEMICOL')
            return f'{name}({", ".join(args)});'
        else:
            raise SyntaxError(
                f'Invalid statement start: {name}, line {line_num}')

    def if_stmt(self):
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
        if self.peek() == 'ELSE':
            self.next()
            self.expect('LBRACE')
            else_body = []
            while self.peek() not in ('RBRACE', 'EOF'):
                else_body.append(self.statement())
            self.expect('RBRACE')
            code += ' else {\n'
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

    def return_stmt(self):
        line_num = self.expect('RETURN')[2]
        expr_type, expr = self.expr()
        ret_type = self.env[self.fn_name]
        if expr_type != ret_type:
            raise TypeError(
                f'Incompatible {expr_type} to {ret_type} conversion, line {line_num}')
        self.expect('SEMICOL')
        return f'return {expr};'

    # ==============================================================
    # Expressions
    # returns type, value
    def expr(self):
        return self.logical()

    def logical(self):
        result = self.relational()
        while self.peek() in ('OR', 'AND'):
            op = self.next()[1]
            rhs = self.relational()[1]
            result = ('int', f'({result[1]} {op} {rhs})')
        return result

    def relational(self):
        result = self.additive()
        while self.peek() in ('EQ', 'NE', 'LT', 'GT', 'LE', 'GE'):
            op = self.next()[1]
            rhs = self.additive()[1]
            result = ('int', f'({result[1]} {op} {rhs})')
        return result

    def additive(self):
        result = self.multiplicative()
        while self.peek() in ('PLUS', 'MINUS'):
            op = self.next()[1]
            rhs = self.multiplicative()[1]
            result = ('int', f'({result[1]} {op} {rhs})')
        return result

    def multiplicative(self):
        result = self.factor()
        while self.peek() in ('MUL', 'DIV') and result[0]:
            op = self.next()[1]
            rhs = self.factor()[1]
            result = ('int', f'({result[1]} {op} {rhs})')
        return result

    def factor(self):
        tok = self.next()
        line_num = tok[2]
        if tok[0] == 'NUMBER':
            return ('int', tok[1])
        elif tok[0] == 'CHAR':
            return ('char', tok[1])
        elif tok[0] == 'STRING':
            return ('char*', tok[1])
        elif tok[0] == 'ID':
            if self.peek() == 'LPAREN':
                if tok[1] not in self.env:
                    raise ReferenceError(
                        f'Call to undeclared function \'{tok[1]}\', line {line_num}')
                self.next()
                args = []
                if self.peek() != 'RPAREN':
                    args.append(self.expr()[1])
                    while self.peek() == 'COMMA':
                        self.next()
                        args.append(self.expr()[1])
                self.expect('RPAREN')
                return (self.env[tok[1]], f'{tok[1]}({", ".join(args)})')
            else:
                if tok[1] not in self.variables:
                    raise ReferenceError(
                        f'Use of undeclared identifier \'{tok[1]}\', line {line_num}')
                return (self.variables[tok[1]], tok[1])
        elif tok[0] == 'LPAREN':
            expr_type, expr = self.expr()
            self.expect('RPAREN')
            return (expr_type, expr)
        else:
            raise SyntaxError(
                f'Unexpected token in factor: {tok}, line {tok[2]}')
