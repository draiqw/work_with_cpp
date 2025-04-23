#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <utility>

struct Token {
    bool is_number;
    int value;
    char op;    

    Token(int v) : is_number(true), value(v), op(0) {}
    Token(char c) : is_number(false), value(0), op(c) {}
};

std::vector<Token> tokenize(const std::string& s) {
    std::vector<Token> tokens;
    size_t i = 0, n = s.size();

    while (i < n) {
        char c = s[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            ++i;
        }
        else if (c == '+' || c == '-' || c == '(' || c == ')') {
            tokens.emplace_back(c);
            ++i;
        }
        else if (std::isdigit(static_cast<unsigned char>(c))) {
            int num = 0;
            while (i < n && std::isdigit(static_cast<unsigned char>(s[i]))) {
                num = num * 10 + (s[i] - '0');
                ++i;
            }
            tokens.emplace_back(num);
        }
        else {
            ++i;
        }
    }
    return tokens;
}

std::pair<int, size_t> eval_expr(const std::vector<Token>& tokens, size_t pos = 0) {
    int result = 0;
    int sign = 1;
    bool prev_was_value = false;
    size_t i = pos;

    while (i < tokens.size()) {
        const Token& tk = tokens[i++];
        
        if (tk.is_number) {
            result += sign * tk.value;
            prev_was_value = true;
        }
        else if (tk.op == '(') {
            auto [val, new_pos] = eval_expr(tokens, i);
            result += sign * val;
            prev_was_value = true;
            i = new_pos;
        }
        else if (tk.op == ')') {
            return { result, i };
        }
        else {
            if (tk.op == '+') {
                sign = 1;
            }
            else { 
                if (prev_was_value) {
                    sign = -1;       
                }
                else {
                    sign = -sign;       
                }
            }
            prev_was_value = false;
        }
    }

    return { result, i };
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string expr;
    std::getline(std::cin, expr);

    auto tokens = tokenize(expr);
    auto [result, _] = eval_expr(tokens, 0);

    std::cout << result << "\n";
    return 0;
}
