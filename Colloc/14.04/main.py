def tokenize(s):
    tokens = []
    i, n = 0, len(s)
    while i < n:
        c = s[i]
        if c.isspace():
            i += 1
        elif c in '+-()':
            tokens.append(c)
            i += 1
        elif c.isdigit():
            num = 0
            while i < n and s[i].isdigit():
                num = num * 10 + int(s[i])
                i += 1
            tokens.append(num)
    return tokens

def parse_term(tokens, i):
    if tokens[i] == '+':
        return parse_term(tokens, i+1)
    if tokens[i] == '-':
        val, j = parse_term(tokens, i+1)
        return -val, j
    if tokens[i] == '(':
        val, j = parse_expr(tokens, i+1)
        return val, j+1
    return tokens[i], i+1

def parse_expr(tokens, i=0):
    val, i = parse_term(tokens, i)
    while i < len(tokens) and tokens[i] in ('+', '-'):
        op = tokens[i]
        rhs, i = parse_term(tokens, i+1)
        if op == '+':
            val += rhs
        else:
            val -= rhs
    return val, i

expr = input().strip()
tokens = tokenize(expr)
result, pos = parse_expr(tokens)
print(result)
