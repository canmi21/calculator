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
    if (!left) return nullptr;
    while (true) {
        skipWhitespace();
        char op = *iter;
        if (op != '+' && op != '-') break;
        getNextChar();
        llvm::Value* right = parseTerm();
        if (!right) return nullptr;
        if (op == '+') {
            left = builder.CreateFAdd(left, right, "addtmp");
        } else {
            left = builder.CreateFSub(left, right, "subtmp");
        }
    }
    return left;
}

llvm::Value* Parser::parseTerm() {
    llvm::Value* left = parsePower();
    if (!left) return nullptr;
    while (true) {
        skipWhitespace();
        char op = *iter;
        if (op != '*' && op != '/') break;
        getNextChar();
        llvm::Value* right = parsePower();
        if (!right) return nullptr;
        if (op == '*') {
            left = builder.CreateFMul(left, right, "multmp");
        } else {
            left = builder.CreateFDiv(left, right, "divtmp");
        }
    }
    return left;
}

llvm::Value* Parser::parsePower() {
    llvm::Value* left = parseFactor();
    if (!left) return nullptr;
    while (true) {
        skipWhitespace();
        if (*iter != '^') break;
        getNextChar();
        llvm::Value* right = parseFactor();
        if (!right) return nullptr;
        llvm::Function* powFunc = module->getFunction("pow");
        if (!powFunc) {
            llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy(), builder.getDoubleTy()}, false);
            powFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "pow", module.get());
        }
        left = builder.CreateCall(powFunc, {left, right}, "powtmp");
    }
    return left;
}

llvm::Value* Parser::parseFactor() {
    skipWhitespace();
    if (std::isdigit(*iter) || *iter == '.') {
        return parseNumber();
    } else if (*iter == '(') {
        return parseParenExpr();
    } else if (std::isalpha(*iter)) {
        std::string name;
        while (iter != end && std::isalpha(*iter)) {
            name += *iter++;
        }
        skipWhitespace();
        if (*iter == '(') {
            return parseFunction(name);
        } else if (name == "pi") {
            return llvm::ConstantFP::get(context, llvm::APFloat(M_PI));
        } else if (name == "e") {
            return llvm::ConstantFP::get(context, llvm::APFloat(M_E));
        } else {
            iter -= name.size();
            return parseVariable();
        }
    }
    return nullptr;
}

llvm::Value* Parser::parseFunction(const std::string& funcName) {
    getNextChar();
    llvm::Value* expr = parseExpression();
    if (!expr) return nullptr;
    skipWhitespace();
    if (*iter != ')') return nullptr;
    getNextChar();
    if (funcName == "sqrt") {
        llvm::Function* sqrtFunc = module->getFunction("sqrt");
        if (!sqrtFunc) {
            llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
            sqrtFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "sqrt", module.get());
        }
        return builder.CreateCall(sqrtFunc, expr, "sqrttmp");
    } else if (funcName == "log") {
        llvm::Function* logFunc = module->getFunction("log");
        if (!logFunc) {
            llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
            logFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "log", module.get());
        }
        return builder.CreateCall(logFunc, expr, "logtmp");
    } else if (funcName == "sin") {
        llvm::Function* sinFunc = module->getFunction("sin");
        if (!sinFunc) {
            llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
            sinFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "sin", module.get());
        }
        return builder.CreateCall(sinFunc, expr, "sintmp");
    } else if (funcName == "cos") {
        llvm::Function* cosFunc = module->getFunction("cos");
        if (!cosFunc) {
            llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
            cosFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "cos", module.get());
        }
        return builder.CreateCall(cosFunc, expr, "costmp");
    } else if (funcName == "atan") {
        llvm::Function* atanFunc = module->getFunction("atan");
        if (!atanFunc) {
            llvm::FunctionType* ft = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy()}, false);
            atanFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "atan", module.get());
        }
        return builder.CreateCall(atanFunc, expr, "atantmp");
    }
    return nullptr;
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
    if (!expr) return nullptr;
    skipWhitespace();
    if (*iter != ')') return nullptr;
    getNextChar();
    return expr;
}

llvm::Value* Parser::parseVariable() {
    std::string varName;
    while (iter != end && std::isalpha(*iter)) {
        varName += *iter++;
    }
    if (namedValues.find(varName) != namedValues.end()) {
        return namedValues[varName];
    }
    return nullptr;
}