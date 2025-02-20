#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"


using namespace llvm;

namespace {
struct ConstantFoldingPass : public PassInfoMixin<ConstantFoldingPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
        for (BasicBlock &BB : F) {
            for (auto it = BB.begin(); it != BB.end(); ) {
                Instruction *I = &*it++;
                if (BinaryOperator *BO = dyn_cast<BinaryOperator>(I)) {
                    if (ConstantInt *LHS = dyn_cast<ConstantInt>(BO->getOperand(0))) {
                        if (ConstantInt *RHS = dyn_cast<ConstantInt>(BO->getOperand(1))) {
                            Constant *Result = ConstantExpr::get(BO->getOpcode(), LHS, RHS);
                            BO->replaceAllUsesWith(Result);
                            BO->eraseFromParent();
                        }
                    }
                }
            }
        }
        return PreservedAnalyses::all();
    }
};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "ConstantFoldingPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) { PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM, ...) {
                if (Name == "constant-folding") {
                    FPM.addPass(ConstantFoldingPass());
                    return true;
                }
                return false;
            }
        );}};
}

