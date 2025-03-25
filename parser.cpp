#include "parser.h"

Parser::Parser(const std::string &expression) : expression(expression), pos(0) {}

std::unique_ptr<ASTNode> Parser::parse() {
    return parseExpression();
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseTerm();
    while (match('+') || match('-')) {
        char op = expression[pos - 1];
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();
    while (match('*') || match('/')) {
        char op = expression[pos - 1];
        auto right = parseFactor();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    if (match('(')) {
        auto expr = parseExpression();
        expect(')');
        return expr;
    } else if (isdigit(peek())) {
        return parseNumber();
    } else if (match('s')) {
        expect('q'); expect('r'); expect('t'); expect('(');
        auto expr = parseExpression();
        expect(')');
        return std::make_unique<UnaryOpNode>('s', std::move(expr));
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseNumber() {
    size_t start = pos;
    while (isdigit(peek()) || peek() == '.') advance();
    return std::make_unique<NumberNode>(std::stod(expression.substr(start, pos - start)));
}

char Parser::peek() {
    return pos < expression.size() ? expression[pos] : '\0';
}

char Parser::advance() {
    return expression[pos++];
}

bool Parser::match(char expected) {
    if (peek() == expected) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(char expected) {
    if (!match(expected)) {
        throw std::runtime_error(std::string("Expected '") + expected + "'");
    }
}
