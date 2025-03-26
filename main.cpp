#include "parser.h"
#include <iostream>
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    Parser parser;
    std::string input;
    std::cout << "Enter expression: ";
    std::getline(std::cin, input);

    llvm::FunctionType* ft = llvm::FunctionType::get(parser.getBuilder().getDoubleTy(), false);
    llvm::Function* func = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", parser.getModule().get());
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(parser.getContext(), "entry", func);
    parser.getBuilder().SetInsertPoint(bb);

    llvm::Value* result = parser.parse(input);
    if (!result) {
        std::cerr << "Error parsing expression." << std::endl;
        return 1;
    }

    parser.getBuilder().CreateRet(result);
    parser.getModule()->print(llvm::errs(), nullptr);

    std::string error;
    llvm::ExecutionEngine* ee = llvm::EngineBuilder(std::move(parser.getModule())).setErrorStr(&error).create();
    if (!ee) {
        std::cerr << "Error creating execution engine: " << error << std::endl;
        return 1;
    }

    llvm::GenericValue gv = ee->runFunction(func, {});
    std::cout << "Result: " << gv.DoubleVal << std::endl;

    return 0;
}