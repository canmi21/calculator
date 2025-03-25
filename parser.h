#include "parser.h"
#include <cctype>
#include <stdexcept>

Parser::Parser(llvm::LLVMContext &ctx, llvm::Module &mod) 
    : context(ctx), module(mod), builder(ctx), index(0) {}

llvm::Value *Parser::parseExpression(const std::string &expr) {
    input = expr;
    index = 0;
    return parseTerm();
}

llvm::Value *Parser::parsePrimary() {
    while (index < input.size() && isspace(input[index])) ++index;
    if (isdigit(input[index]) || input[index] == '.') {
        size_t start = index;
        while (index < input.size() && (isdigit(input[index]) || input[index] == '.')) ++index;
        double value = std::stod(input.substr(start, index - start));
        return llvm::ConstantFP::get(builder.getDoubleTy(), value);
    }
    throw std::runtime_error("Unexpected token");
}

llvm::Value *Parser::parseFactor() {
    llvm::Value *lhs = parsePrimary();
    while (index < input.size() && (input[index] == '*' || input[index] == '/')) {
        char op = input[index++];
        llvm::Value *rhs = parsePrimary();
        if (op == '*') lhs = builder.CreateFMul(lhs, rhs);
        else lhs = builder.CreateFDiv(lhs, rhs);
    }
    return lhs;
}

llvm::Value *Parser::parseTerm() {
    llvm::Value *lhs = parseFactor();
    while (index < input.size() && (input[index] == '+' || input[index] == '-')) {
        char op = input[index++];
        llvm::Value *rhs = parseFactor();
        if (op == '+') lhs = builder.CreateFAdd(lhs, rhs);
        else lhs = builder.CreateFSub(lhs, rhs);
    }
    return lhs;
}
