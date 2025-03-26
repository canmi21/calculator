#include "parser.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/ExecutionSession.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeContext.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <iostream>
#include <memory>
#include <string>

int main() {
    llvm::LLVMContext TheContext;
    llvm::IRBuilder<> Builder(TheContext);
    auto TheModule = std::make_unique<llvm::Module>("calculator", TheContext);

    std::string input;
    std::cout << "Enter an expression: ";
    std::getline(std::cin, input);

    auto AST = Parse(input);
    if (!AST) {
        std::cerr << "Failed to parse expression" << std::endl;
        return 1;
    }

    llvm::FunctionType* FT = llvm::FunctionType::get(Builder.getDoubleTy(), false);
    llvm::Function* MainFunc = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", TheModule.get());

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(TheContext, "entry", MainFunc);
    Builder.SetInsertPoint(BB);

    llvm::Value* RetVal = AST->codegen();
    Builder.CreateRet(RetVal);

    llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> JIT = llvm::orc::LLJITBuilder().create();
    if (!JIT) {
        std::cerr << "Failed to create JIT: " << toString(JIT.takeError()) << std::endl;
        return 1;
    }

    auto& JITInstance = *JIT.get();
    if (auto Err = JITInstance->addIRModule(llvm::orc::ThreadSafeModule(std::move(TheModule), std::make_shared<llvm::orc::ThreadSafeContext>(TheContext)))) {
        std::cerr << "Error adding module to JIT: " << toString(std::move(Err)) << std::endl;
        return 1;
    }

    auto MainSymbol = JITInstance->lookup("main");
    if (!MainSymbol) {
        std::cerr << "Error finding 'main' function" << std::endl;
        return 1;
    }

    typedef double (*MainFuncType)();
    MainFuncType MainFuncPtr = reinterpret_cast<MainFuncType>(MainSymbol->getAddress());
    std::cout << "Result: " << MainFuncPtr() << std::endl;

    return 0;
}
