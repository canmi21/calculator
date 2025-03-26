#include "parser.h"
#include <cctype>
#include <cmath>

Parser::Parser() : builder(context), iter(), end() {
    module = std::make_unique<llvm::Module>("calc", context);
}

char Parser::getNextChar() {
    if (iter != end) {
        return *iter++;
    }
    return '\0';
}

void Parser::skipWhitespace() {
    while (iter != end && std::isspace(*iter)) {
        ++iter;
    }
}

llvm::Value* Parser::parse(const std::string& input) {
    iter = input.begin();
    end = input.end();
    skipWhitespace();
    return parseExpression();
}

llvm::Value* Parser::parseExpression() {
    llvm::Value* left = parseTerm();
    while (true) {
        skipWhitespace();
        char op = *iter;
        if (op != '+' && op != '-') {
            break;
        }
        getNextChar();
        llvm::Value* right = parseTerm();
        if (op == '+') {
            left = builder.CreateFAdd(left, right, "addtmp");
        } else {
            left = builder.CreateFSub(left, right, "subtmp");
        }
    }
    return left;
}

llvm::Value* Parser::parseTerm() {
    llvm::Value* left = parseFactor();
    while (true) {
        skipWhitespace();
        char op = *iter;
        if (op != '*' && op != '/') {
            break;
        }
        getNextChar();
        llvm::Value* right = parseFactor();
        if (op == '*') {
            left = builder.CreateFMul(left, right, "multmp");
        } else {
            left = builder.CreateFDiv(left, right, "divtmp");
        }
    }
    return left;
}

llvm::Value* Parser::parseFactor() {
    skipWhitespace();
    if (std::isdigit(*iter) || *iter == '.') {
        return parseNumber();
    } else if (*iter == '(') {
        return parseParenExpr();
    } else if (std::string(iter, iter + 5) == "sqrt(") {
        return parseSqrt();
    } else {
        return nullptr;
    }
}

llvm::Value* Parser::parseNumber() {
    std::string numStr;
    while (iter != end && (std::isdigit(*iter) || *iter == '.')) {
        numStr += *iter++;
    }
    double val = std::stod(numStr);
    return llvm::ConstantFP::get(context, llvm::APFloat(val));
}

llvm::Value* Parser::parseParenExpr() {
    getNextChar();
    llvm::Value* expr = parseExpression();
    skipWhitespace();
    if (*iter != ')') {
        return nullptr;
    }
    getNextChar();
    return expr;
}

llvm::Value* Parser::parseSqrt() {
    iter += 5;
    llvm::Value* expr = parseExpression();
    skipWhitespace();
    if (*iter != ')') {
        return nullptr;
    }
    getNextChar();
    llvm::Function* sqrtFunc = module->getFunction("sqrt");
    if (!sqrtFunc) {
        llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
        sqrtFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "sqrt", module.get());
    }
    return builder.CreateCall(sqrtFunc, expr, "sqrttmp");
}
