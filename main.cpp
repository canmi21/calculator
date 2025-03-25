#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicsX86.h>

int main() {
    llvm::LLVMContext context;
    llvm::Module module("calculator", context);
    llvm::IRBuilder<> builder(context);

    llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getDoubleTy(), {builder.getDoubleTy(), builder.getDoubleTy()}, false);
    llvm::Function *calcFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "calculate", module);

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", calcFunc);
    builder.SetInsertPoint(entry);
    
    auto arg_it = calcFunc->arg_begin();
    llvm::Value *x = &(*arg_it);
    ++arg_it;
    llvm::Value *y = &(*arg_it);
    
    llvm::Value *add = builder.CreateFAdd(x, y, "add");
    llvm::Value *sub = builder.CreateFSub(x, y, "sub");
    llvm::Value *mul = builder.CreateFMul(x, y, "mul");
    llvm::Value *div = builder.CreateFDiv(x, y, "div");
    llvm::Function *sqrtFunc = llvm::Intrinsic::getDeclaration(&module, llvm::Intrinsic::sqrt, {builder.getDoubleTy()});
    llvm::Value *sqrt_x = builder.CreateCall(sqrtFunc, x, "sqrt_x");
    llvm::Value *sqrt_y = builder.CreateCall(sqrtFunc, y, "sqrt_y");
    
    builder.CreateRet(add);
    module.print(llvm::outs(), nullptr);
    
    return 0;
}
