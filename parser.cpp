#include "parser.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <cctype>
#include <iostream>
#include <sstream>

extern llvm::LLVMContext TheContext;
extern llvm::IRBuilder<> Builder;
extern std::unique_ptr<llvm::Module> TheModule;

static std::string Input;
static size_t Pos = 0;

static char CurrentChar() {
    if (Pos >= Input.size()) return '\0';
    return Input[Pos];
}

static void NextChar() {
    if (Pos < Input.size()) Pos++;
}

static std::unique_ptr<ExprAST> ParseNumber() {
    std::string NumStr;
    while (isdigit(CurrentChar()) || CurrentChar() == '.') {
        NumStr += CurrentChar();
        NextChar();
    }
    double Val = std::stod(NumStr);
    return std::make_unique<NumberExprAST>(Val);
}

static std::unique_ptr<ExprAST> ParseParen() {
    NextChar();
    auto Expr = ParseExpression();
    NextChar();
    return Expr;
}

static std::unique_ptr<ExprAST> ParseIdentifier() {
    std::string IdStr;
    while (isalpha(CurrentChar())) {
        IdStr += CurrentChar();
        NextChar();
    }
    NextChar();
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurrentChar() != ')') {
        while (true) {
            if (auto Arg = ParseExpression()) {
                Args.push_back(std::move(Arg));
            }
            if (CurrentChar() == ')') break;
            NextChar();
        }
    }
    NextChar();
    return std::make_unique<CallExprAST>(IdStr, std::move(Args));
}

static std::unique_ptr<ExprAST> ParsePrimary() {
    if (isdigit(CurrentChar())) {
        return ParseNumber();
    } else if (CurrentChar() == '(') {
        return ParseParen();
    } else if (isalpha(CurrentChar())) {
        return ParseIdentifier();
    }
    return nullptr;
}

static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
    while (true) {
        char Op = CurrentChar();
        if (Op != '+' && Op != '-' && Op != '*' && Op != '/') {
            return LHS;
        }
        int TokPrec = (Op == '+' || Op == '-') ? 10 : 20;
        if (TokPrec < ExprPrec) {
            return LHS;
        }
        NextChar();
        auto RHS = ParsePrimary();
        char NextOp = CurrentChar();
        int NextPrec = (NextOp == '+' || NextOp == '-') ? 10 : 20;
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
        }
        LHS = std::make_unique<BinaryExprAST>(Op, std::move(LHS), std::move(RHS));
    }
}

static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<ExprAST> Parse(const std::string& input) {
    Input = input;
    Pos = 0;
    return ParseExpression();
}

llvm::Value* NumberExprAST::codegen() {
    return llvm::ConstantFP::get(TheContext, llvm::APFloat(Val));
}

llvm::Value* BinaryExprAST::codegen() {
    llvm::Value* L = LHS->codegen();
    llvm::Value* R = RHS->codegen();
    switch (Op) {
        case '+': return Builder.CreateFAdd(L, R, "addtmp");
        case '-': return Builder.CreateFSub(L, R, "subtmp");
        case '*': return Builder.CreateFMul(L, R, "multmp");
        case '/': return Builder.CreateFDiv(L, R, "divtmp");
        default: return nullptr;
    }
}

llvm::Value* CallExprAST::codegen() {
    if (Callee == "sqrt") {
        llvm::Value* Arg = Args[0]->codegen();
        llvm::Function* SqrtFn = TheModule->getFunction("sqrt");
        if (!SqrtFn) {
            llvm::FunctionType* FT = llvm::FunctionType::get(Builder.getDoubleTy(), {Builder.getDoubleTy()}, false);
            SqrtFn = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "sqrt", TheModule.get());
        }
        return Builder.CreateCall(SqrtFn, Arg, "sqrttmp");
    }
    return nullptr;
}