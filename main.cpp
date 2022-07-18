#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

using Env = std::unordered_map<std::string, int>;

class Expr {
public:
    virtual int eval(Env& env) = 0;
};

class Num : public Expr {
public:
    explicit Num(int n) : val(n) {};

    int val;
    int eval(Env& env) override {
        return val;
    }
};

class Addition : public Expr {
public:
    Addition(
        std::unique_ptr<Expr>&& l,
        std::unique_ptr<Expr>&& r) :
        left(std::move(l)),
        right(std::move(r)) {}
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    int eval(Env& env) override {
        return left->eval(env) + right->eval(env);
    }
};

class Subtraction : public Expr {
public:
    Subtraction(
        std::unique_ptr<Expr>&& l,
        std::unique_ptr<Expr>&& r) :
        left(std::move(l)),
        right(std::move(r)) {}
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    int eval(Env& env) override {
        return left->eval(env) - right->eval(env);
    }
};

class Multiplication : public Expr {
public:
    Multiplication(
        std::unique_ptr<Expr>&& l,
        std::unique_ptr<Expr>&& r) :
        left(std::move(l)),
        right(std::move(r)) {}
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    int eval(Env& env) override {
        return left->eval(env) * right->eval(env);
    }
};

class Division : public Expr {
public:
    Division(
        std::unique_ptr<Expr>&& l,
        std::unique_ptr<Expr>&& r) :
        left(std::move(l)),
        right(std::move(r)) {}
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    int eval(Env& env) override {
        int l = left->eval(env);
        int r = right->eval(env);
        if (r == 0) throw std::runtime_error("Error: Cannot divide by 0");
        return l / r;
    }
};

class Assignment : public Expr {
public:
    Assignment(
        std::string name,
        std::unique_ptr<Expr>&& val) :
        name(name), val(std::move(val)) {}

    std::string name;
    std::unique_ptr<Expr> val;
    int eval(Env& env) override {
        int v = val->eval(env);
        env[name] = v;
        return v;
    }
};

class Variable : public Expr {
public:
    explicit Variable(std::string name) : name(name) {}

    std::string name;
    int eval(Env& env) override {
        return env.at(name);
    }
};

class Seq : public Expr {
public:
    explicit Seq(
        std::unique_ptr<Expr>&& l,
        std::unique_ptr<Expr>&& r) :
        left(std::move(l)),
        right(std::move(r)) {}
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    int eval(Env& env) override {
        left->eval(env);
        return right->eval(env);

    };
};


enum class Kind {
    Number = 0, // liczba
    LParen, // (
    RParen, // )
    Plus, // +
    Minus, // -
    Mult, // *
    Div, // /
    Assign, // =
    Name, // zmienna
    Semicolon, // ;

};

struct Token {
    Kind kind;
    std::string lexeme;
};

std::vector<Token> lex(const std::string& source) {
    std::vector<Token> tokens;
    std::size_t end = source.size();
    std::size_t curr = 0;
    while (curr < end) {
        if (std::isdigit(source[curr])) {
            std::string number; // 22
            while (curr < end && std::isdigit(source[curr])) {
                number += source[curr];
                curr += 1;
            }
            tokens.push_back(Token{
                Kind::Number, number });
            continue;
        }
        if (std::isalpha(source[curr])) {
            std::string var; // aa
            while (curr < end && std::isalpha(source[curr])) {
                var += source[curr];
                curr += 1;
            }
            tokens.push_back(Token{
                Kind::Name, var });
            continue;
        }
        if (std::isspace(source[curr])) {
            curr += 1;
            continue;
        }
        switch (source[curr]) {
        case '+': {
            tokens.push_back(Token{
                Kind::Plus, "+" });
            break;
        }
        case '(': {
            tokens.push_back(Token{
                Kind::LParen, "(" });
            break;
        }
        case ')': {
            tokens.push_back(Token{
                Kind::RParen, ")" });
            break;
        }
        case '*': {
            tokens.push_back(Token{
                Kind::Mult, "*" });
            break;
        }
        case '-': {
            tokens.push_back(Token{
                Kind::Minus, "-" });
            break;
        }
        case '/': {
            tokens.push_back(Token{
                Kind::Div, "/" });
            break;
        }
        case '=': {
            tokens.push_back(Token{
                Kind::Assign, "=" });
            break;
        }
        case ';': {
            tokens.push_back(Token{
                Kind::Semicolon, ";" });
            break;
        }
        default: {
            std::string msg = "Unknown character: ";
            msg += source[curr];
            throw std::runtime_error(msg);
        }
        }
        curr += 1;
    }
    return tokens;
}

bool past_end(int curr, const std::vector<Token> tokens) {
    return curr >= tokens.size();
}

std::unique_ptr<Expr> parse_expr(int&, const std::vector<Token>&);

std::unique_ptr<Expr> parse_term(
    int& curr, const std::vector<Token>& tokens)
{
    if (past_end(curr, tokens)) {
        throw std::runtime_error("Expected number or parenthesis");
    }
    const Token& tok = tokens[curr];
    curr += 1;
    switch (tok.kind) {
    case Kind::Number: {
        int n = std::stoi(tok.lexeme);
        std::unique_ptr<Expr> number =
            std::make_unique<Num>(n);
        return number;
    }
    case Kind::LParen: {
        auto expr = parse_expr(curr, tokens);
        if (past_end(curr, tokens)) {
            throw std::runtime_error("Unclosed parenthesis");
        }
        const Token& tok = tokens[curr];
        if (tok.kind != Kind::RParen) {
            throw std::runtime_error("Unclosed parenthesis");
        }
        curr += 1;
        return expr;
    }
    case Kind::Name: {
        std::string n = tok.lexeme;
        std::unique_ptr<Expr> variable =
            std::make_unique<Variable>(n);
        return variable;
    }
    };
    throw std::runtime_error("Expected number or parenthesis");
}


// term * term
std::unique_ptr<Expr> parse_multiplication(
    int& curr, const std::vector<Token>& tokens)
{
    auto left = parse_term(curr, tokens);
    if (past_end(curr, tokens)) {
        return left;
    }
    Token curr_tok = tokens[curr];
    while (curr_tok.kind == Kind::Mult || curr_tok.kind == Kind::Div) {
        curr += 1;
        auto right = parse_term(curr, tokens);
        if (curr_tok.kind == Kind::Mult) {
            left = std::make_unique<Multiplication>(
                std::move(left),
                std::move(right));
        }
        else if (curr_tok.kind == Kind::Div) {
            left = std::make_unique<Division>(
                std::move(left),
                std::move(right));
        }
        if (past_end(curr, tokens)) {
            return left;
        }
        curr_tok = tokens[curr];
    }
    return left;
}

// multiplication + multiplication
std::unique_ptr<Expr> parse_addition(
    int& curr, const std::vector<Token>& tokens)
{
    auto left = parse_multiplication(curr, tokens);
    if (past_end(curr, tokens)) {
        return left;
    }
    Token curr_tok = tokens[curr];
    while (curr_tok.kind == Kind::Plus || curr_tok.kind == Kind::Minus) {
        curr += 1;
        auto right = parse_multiplication(curr, tokens);
        if (curr_tok.kind == Kind::Plus) {
            left = std::make_unique<Addition>(
                std::move(left),
                std::move(right));
        }
        else if (curr_tok.kind == Kind::Minus) {
            left = std::make_unique<Subtraction>(
                std::move(left),
                std::move(right));
        }
        
        if (past_end(curr, tokens)) {
            return left;
        }
        curr_tok = tokens[curr];
    }
    return left;
}

std::unique_ptr<Expr> parse_assign(
    int& curr, const std::vector<Token>& tokens) {

    if (past_end(curr, tokens)) {
        return parse_addition(curr, tokens);
    }
    if (past_end(curr+1, tokens)) {
        return parse_addition(curr, tokens);
    }
    Token curr_tok = tokens[curr];

    if (curr_tok.kind == Kind::Name && tokens[curr + 1].kind == Kind::Assign) {
        curr += 2;
        auto right = parse_addition(curr, tokens);
        return std::make_unique<Assignment>(
                curr_tok.lexeme,
                std::move(right));
    }
    return parse_addition(curr, tokens);
}


std::unique_ptr<Expr> parse_seq(
    int& curr, const std::vector<Token>& tokens) {
    auto left = parse_assign(curr, tokens);

    if (past_end(curr, tokens)) {
        return left;
    }
    Token curr_tok = tokens[curr];

    while (curr_tok.kind == Kind::Semicolon) {
        curr += 1;
        auto right = parse_assign(curr, tokens);
        left = std::make_unique<Seq>(
            std::move(left),
            std::move(right));

        if (past_end(curr, tokens)) {
            return left;
        }
        curr_tok = tokens[curr];
    }
    return left;
}

std::unique_ptr<Expr> parse_expr(
    int& curr, const std::vector<Token>& tokens) {
    return parse_seq(curr, tokens);
}

std::unique_ptr<Expr> parse(const std::vector<Token>& tokens) {
    int curr = 0;
    return parse_expr(curr, tokens);
}

int main() {
    std::cout << "ENG" << std::endl;
    std::cout << "Quick manual below: " << std::endl;
    std::cout << "Addition or Subtraction: In order to add or subtract values use '+' for addition or '-' for subtraction e.g. '1 + 2'." << std::endl;
    std::cout << "Multiplication or Divistion: In order to multipy or divide values use '*' for multiplication or '/' for division e.g. '1 + 2 / 1'." << std::endl;
    std::cout << "Parenthesis: Use '(' to start expression in parenthesis and ')' to end it e.g. '(1+3) / 2'" << std::endl;
    std::cout << "Variable: In order to define a variable assign a value to a name of the variable by using '='. Then stop the assiging by typing ';'." << std::endl;
    std::cout << "Now it is possible to write expression with defined variable e.g. 'x=4; (x+5) * 2'" << std::endl;
    std::cout << "Quiting the program: You can write any number of expressions or type 'end' in order for the program to stop running." << std::endl;
    std::cout << "PL" << std::endl;
    std::cout << "Szybka instrukcja ponizej: " << std::endl;
    std::cout << "Dodawanie lub Odejnowanie: Aby dodac lub odjac wartosci uzyj '+' do dodawania lub '-' do odejmowania np. '1 + 2'." << std::endl;
    std::cout << "Mnozenie lub Dzielenie: Aby pomnozyc lub podzielic wartosci uzyj '*' do mnozenia lub '/' do dzielenia np. '1 + 2 / 1'." << std::endl;
    std::cout << "Nawiasy: Uzyj '(' aby zaczac wyrazenie w nawiasie i ')' by je zakonczyc np. '(1+3) / 2'" << std::endl;
    std::cout << "Zmienne: W celu zdefiniowania zmiennej przypisz wartosc do nazwy zmiennej uzywajac '='. Nastepnie aby zakonczyc definiowanie napisz ';'." << std::endl;
    std::cout << "Teraz mozliwe jest napisanie wyrazenia z uzyciem zdefiniowanej zmiennej np. 'x=4; (x+5) * 2'" << std::endl;
    std::cout << "Zakończenie działania programu: Mozesz napisac dowolna liczbe wyrazen do obliczenia lub napisać 'end' w celu zatrzymania programu." << std::endl << std::endl;
    Env env;
    while (true) {
        std::string source;
        std::cout << "Your expression:  / Twoje wyrazenie: ";
        std::getline(std::cin, source);
        if (source == "end") return 0;
        try {
            auto tokens = lex(source);
            auto expr = parse(tokens);
            std::cout << std::endl << "Result: / Wynik: " << expr->eval(env) << std::endl;
        }
        catch (const std::exception& e) {
            std::cout<<e.what() <<std::endl;
        }
        
    }

    return 0;
}
