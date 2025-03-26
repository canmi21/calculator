#include "parser.h"
#include <iostream>
#include <ostream>
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
    if (!left) {
        std::cerr << "Error parsing term." << std::endl;
        return nullptr;
    }
    while (true) {
        skipWhitespace();
        char op = *iter;
        if (op != '+' && op != '-') {
            break;
        }
        getNextChar();
        llvm::Value* right = parseTerm();
        if (!right) {
            std::cerr << "Error parsing term." << std::endl;
            return nullptr;
        }
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
    if (!left) {
        std::cerr << "Error parsing factor." << std::endl;
        return nullptr;
    }
    while (true) {
        skipWhitespace();
        char op = *iter;
        if (op != '*' && op != '/') {
            break;
        }
        getNextChar();
        llvm::Value* right = parseFactor();
        if (!right) {
            std::cerr << "Error parsing factor." << std::endl;
            return nullptr;
        }
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
    } else if (std::string(iter, iter + 4) == "log(") {
        return parseLog();
    } else if (std::string(iter, iter + 4) == "sin(") {
        return parseSin();
    } else if (std::string(iter, iter + 4) == "cos(") {
        return parseCos();
    } else if (std::string(iter, iter + 4) == "tan(") {
        return parseTan();
    } else if (std::string(iter, iter + 5) == "asin(") {
        return parseAsin();
    } else if (std::string(iter, iter + 5) == "acos(") {
        return parseAcos();
    } else if (std::string(iter, iter + 5) == "atan(") {
        return parseAtan();
    } else if (std::string(iter, iter + 2) == "pi") {
        return parseConstant();
    } else if (std::string(iter, iter + 1) == "e") {
        return parseConstant();
    } else if (std::isalpha(*iter)) {
        return parseVariable();
    } else {
        std::cerr << "Invalid character: " << *iter << std::endl;
        return nullptr;
    }
}

llvm::Value* Parser::parsePower() {
    llvm::Value* left = parseFactor();
    if (!left) {
        std::cerr << "Error parsing factor." << std::endl;
        return nullptr;
    }
    while (true) {
        skipWhitespace();
        if (*iter == '^') {
            getNextChar();
            llvm::Value* right = parseFactor();
            if (!right) {
                std::cerr << "Error parsing factor." << std::endl;
                return nullptr;
            }
            left = builder.CreateCall(module->getFunction("pow"), {left, right}, "powtmp");
        } else {
            break;
        }
    }
    return left;
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
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
        return nullptr;
    }
    getNextChar();
    return expr;
}

llvm::Value* Parser::parseSqrt() {
    iter += 5;
    llvm::Value* expr = parseExpression();
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
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

llvm::Value* Parser::parseLog() {
    iter += 4;
    llvm::Value* expr = parseExpression();
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
        return nullptr;
    }
    getNextChar();
    llvm::Function* logFunc = module->getFunction("log");
    if (!logFunc) {
        llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
        logFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "log", module.get());
    }
    return builder.CreateCall(logFunc, expr, "logtmp");
}

llvm::Value* Parser::parseSin() {
    iter += 4;
    llvm::Value* expr = parseExpression();
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
        return nullptr;
    }
    getNextChar();
    llvm::Function* sinFunc = module->getFunction("sin");
    if (!sinFunc) {
        llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
        sinFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "sin", module.get());
    }
    return builder.CreateCall(sinFunc, expr, "sintmp");
}

llvm::Value* Parser::parseCos() {
    iter += 4;
    llvm::Value* expr = parseExpression();
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
        return nullptr;
    }
    getNextChar();
    llvm::Function* cosFunc = module->getFunction("cos");
    if (!cosFunc) {
        llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
        cosFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "cos", module.get());
    }
    return builder.CreateCall(cosFunc, expr, "costmp");
}

llvm::Value* Parser::parseTan() {
    iter += 4;
    llvm::Value* expr = parseExpression();
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
        return nullptr;
    }
    getNextChar();
    llvm::Function* tanFunc = module->getFunction("tan");
    if (!tanFunc) {
        llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
        tanFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "tan", module.get());
    }
    return builder.CreateCall(tanFunc, expr, "tantmp");
}

llvm::Value* Parser::parseAsin() {
    iter += 5;
    llvm::Value* expr = parseExpression();
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
        return nullptr;
    }
    getNextChar();
    llvm::Function* asinFunc = module->getFunction("asin");
    if (!asinFunc) {
        llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
        asinFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "asin", module.get());
    }
    return builder.CreateCall(asinFunc, expr, "asintmp");
}

llvm::Value* Parser::parseAcos() {
    iter += 5;
    llvm::Value* expr = parseExpression();
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
        return nullptr;
    }
    getNextChar();
    llvm::Function* acosFunc = module->getFunction("acos");
    if (!acosFunc) {
        llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
        acosFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "acos", module.get());
    }
    return builder.CreateCall(acosFunc, expr, "acostmp");
}

llvm::Value* Parser::parseAtan() {
    iter += 5;
    llvm::Value* expr = parseExpression();
    if (!expr) {
        std::cerr << "Error parsing expression." << std::endl;
        return nullptr;
    }
    skipWhitespace();
    if (*iter != ')') {
        std::cerr << "Expected ')'." << std::endl;
        return nullptr;
    }
    getNextChar();
    llvm::Function* atanFunc = module->getFunction("atan");
    if (!atanFunc) {
        llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
        atanFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "atan", module.get());
    }
    return builder.CreateCall(atanFunc, expr, "atantmp");
}

llvm::Value* Parser::parseConstant() {
    std::string constStr;
    while (iter != end && std::isalpha(*iter)) {
        constStr += *iter++;
    }
    if (constStr == "pi") {
        return llvm::ConstantFP::get(context, llvm::APFloat(M_PI));
    } else if (constStr == "e") {
        return llvm::ConstantFP::get(context, llvm::APFloat(M_E));
    } else {
        std::cerr << "Unknown constant: " << constStr << std::endl;
        return nullptr;
    }
}

llvm::Value* Parser::parseVariable() {
    std::string varName;
    while (iter != end && std::isalpha(*iter)) {
        varName += *iter++;
    }
    if (namedValues.find(varName) != namedValues.end()) {
        return namedValues[varName];
    } else {
        std::cerr << "Unknown variable: " << varName << std::endl;
        return nullptr;
    }
}