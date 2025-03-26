#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <map>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

class Parser {
public:
    Parser();
    llvm::Value* parse(const std::string& input);

    llvm::IRBuilder<>& getBuilder() { return builder; }
    llvm::LLVMContext& getContext() { return context; }
    std::unique_ptr<llvm::Module>& getModule() { return module; }

private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;
    std::map<std::string, llvm::Value*> namedValues;

    llvm::Value* parseExpression();
    llvm::Value* parseTerm();
    llvm::Value* parseFactor();
    llvm::Value* parseNumber();
    llvm::Value* parseParenExpr();
    llvm::Value* parseSqrt();

    std::string::const_iterator iter;
    std::string::const_iterator end;

    char getNextChar();
    void skipWhitespace();
};

#endif // PARSER_H
