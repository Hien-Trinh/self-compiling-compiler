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
        self.variables = set()

    def peek(self): return self.tokens[self.pos][0]

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
        ah rettype name(int integer1, integer2, string str1) {
            body;
        }
        """
        indent = self.expect('FN')[3]

        # Set default return type to int if no type given
        rettype = 'int'
        if self.peek() == 'TYPE':
            rettype = self.expect('TYPE')[1]

        name = self.expect('ID')[1]
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
        self.variables = set(params)
        body = []
        while self.peek() not in ('RBRACE', 'EOF'):
            body.append(self.statement())
        self.expect('RBRACE')
        param_list = ', '.join(f'{ptype} {pname}' for ptype, pname in params)
        result = f'{rettype} {name}({param_list}) {{\n'
        result += '\n'.join(' ' * indent + '    ' + s for s in body)
        if name == 'main':
            result += f'{'\n' * (len(body) > 0)}    return 0;'
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
        self.expect('LET')
        name = self.expect('ID')[1]
        self.expect('ASSIGN')
        expr = self.expr()
        self.expect('SEMICOL')
        if name in self.variables:
            return f'{name} = {expr};'
        else:
            self.variables.add(name)
            return f'int {name} = {expr};'

    def print_stmt(self):
        self.expect('PRINT')
        self.expect('LPAREN')
        expr = self.expr()
        self.expect('RPAREN')
        self.expect('SEMICOL')
        return f'printf("%d\\n", {expr});'

    def id_stmt(self):
        expect = self.expect('ID')
        name = expect[1]
        line_num = expect[2]
        if self.peek() == 'ASSIGN':
            self.next()
            expr = self.expr()
            self.expect('SEMICOL')
            return f'{name} = {expr};'
        elif self.peek() == 'LPAREN':
            self.next()
            args = []
            if self.peek() != 'RPAREN':
                while True:
                    args.append(self.expr())
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
        cond = self.expr()
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
        cond = self.expr()
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
        self.expect('RETURN')
        expr = self.expr()
        self.expect('SEMICOL')
        return f'return {expr};'

    # ==============================================================
    # Expressions
    def expr(self):
        return self.logical()

    def logical(self):
        result = self.relational()
        while self.peek() in ('OR', 'AND'):
            op = self.next()[1]
            rhs = self.relational()
            result = f'({result} {op} {rhs})'
        return result

    def relational(self):
        result = self.additive()
        while self.peek() in ('EQ', 'NE', 'LT', 'GT', 'LE', 'GE'):
            op = self.next()[1]
            rhs = self.additive()
            result = f'({result} {op} {rhs})'
        return result

    def additive(self):
        result = self.multiplicative()
        while self.peek() in ('PLUS', 'MINUS'):
            op = self.next()[1]
            rhs = self.multiplicative()
            result = f'({result} {op} {rhs})'
        return result

    def multiplicative(self):
        result = self.factor()
        while self.peek() in ('MUL', 'DIV'):
            op = self.next()[1]
            rhs = self.factor()
            result = f'({result} {op} {rhs})'
        return result

    def factor(self):
        tok = self.next()
        if tok[0] == 'NUMBER':
            return tok[1]
        elif tok[0] == 'CHAR':
            return tok[1]
        elif tok[0] == 'ID':
            if self.peek() == 'LPAREN':
                self.next()
                args = []
                if self.peek() != 'RPAREN':
                    args.append(self.expr())
                    while self.peek() == 'COMMA':
                        self.next()
                        args.append(self.expr())
                self.expect('RPAREN')
                return f'{tok[1]}({", ".join(args)})'
            else:
                return tok[1]
        elif tok[0] == 'LPAREN':
            expr = self.expr()
            self.expect('RPAREN')
            return expr
        else:
            raise SyntaxError(
                f'Unexpected token in factor: {tok}, line {tok[2]}')
