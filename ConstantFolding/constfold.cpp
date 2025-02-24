#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct ConstantFoldingPass : public PassInfoMixin<ConstantFoldingPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
        errs() << "Running Constant Folding on Function: " << F.getName() << "\n";

        bool Changed = false;

        for (BasicBlock &BB : F) {
            for (auto it = BB.begin(); it != BB.end(); ) {
                Instruction *I = &*it++;

                if (auto *BO = dyn_cast<BinaryOperator>(I)) {
                    if (auto *LHS = dyn_cast<ConstantInt>(BO->getOperand(0))) {
                        if (auto *RHS = dyn_cast<ConstantInt>(BO->getOperand(1))) {
                            
                            if (Constant *Folded = ConstantExpr::get(BO->getOpcode(), LHS, RHS)) {
                                errs() << "Folding: " << *BO << " -> " << *Folded << "\n";
                                BO->replaceAllUsesWith(Folded);
                                BO->eraseFromParent();
                                Changed = true;
                            }
                        }
                    }
                }
            }
        }

        return (Changed ? PreservedAnalyses::none() : PreservedAnalyses::all());
    }
};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "ConstantFoldingPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ...) {
                    if (Name == "constant-folding") {
                        FPM.addPass(ConstantFoldingPass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}
