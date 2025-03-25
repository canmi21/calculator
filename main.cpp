#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/IRBuilder.h>

int main() {
    llvm::LLVMContext context;
    llvm::Module module("calculator", context);
    llvm::IRBuilder<> builder(context);

    llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getInt32Ty(), {builder.getInt32Ty(), builder.getInt32Ty()}, false);
    llvm::Function *addFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "add", module);

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", addFunc);
    builder.SetInsertPoint(entry);

    auto arg_it = addFunc->arg_begin();
    llvm::Value *x = *arg_it;
    ++arg_it;
    llvm::Value *y = *arg_it;

    llvm::Value *sum = builder.CreateAdd(x, y, "sum");
    builder.CreateRet(sum);

    module.print(llvm::outs(), nullptr);

    return 0;
}
