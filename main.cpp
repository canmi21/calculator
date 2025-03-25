#include "parser.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <iostream>
#include <memory>

llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);
std::unique_ptr<llvm::Module> TheModule;

int main() {
    std::string input;
    std::cout << "Enter an expression: ";
    std::getline(std::cin, input);

    auto AST = Parse(input);
    if (!AST) {
        std::cerr << "Failed to parse expression" << std::endl;
        return 1;
    }

    TheModule = std::make_unique<llvm::Module>("calculator", TheContext);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::FunctionType* FT = llvm::FunctionType::get(Builder.getDoubleTy(), false);
    llvm::Function* MainFunc = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule.get());

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(TheContext, "entry", MainFunc);
    Builder.SetInsertPoint(BB);

    llvm::Value* RetVal = AST->codegen();
    Builder.CreateRet(RetVal);

    llvm::verifyFunction(*MainFunc);
    TheModule->print(llvm::outs(), nullptr);

    std::string ErrStr;
    std::unique_ptr<llvm::ExecutionEngine> EE(
        llvm::EngineBuilder(std::move(TheModule)).setErrorStr(&ErrStr).create());
    if (!EE) {
        std::cerr << "Failed to create ExecutionEngine: " << ErrStr << std::endl;
        return 1;
    }

    llvm::Function* MainFuncPtr = EE->FindFunctionNamed("main");
    llvm::GenericValue Result = EE->runFunction(MainFuncPtr, {});
    std::cout << "Result: " << Result.DoubleVal << std::endl;

    return 0;
}