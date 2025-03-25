#include <iostream>
#include <string>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include "parser.h"

int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::LLVMContext context;
    auto jit = cantFail(llvm::orc::LLJITBuilder().create());
    auto module = std::make_unique<llvm::Module>("calculator", context);
    llvm::IRBuilder<> builder(context);

    auto exprAST = parseExpression();
    llvm::Function *func = exprAST->codegen(context, module.get(), builder);
    module->print(llvm::outs(), nullptr);

    cantFail(jit->addIRModule(llvm::orc::ThreadSafeModule(std::move(module), std::make_unique<llvm::LLVMContext>())));
    auto sym = cantFail(jit->lookup("compute"));
    auto compute = (double (*)())sym.getAddress();
    std::cout << compute() << std::endl;
    return 0;
}
