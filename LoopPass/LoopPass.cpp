#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;

namespace {
    struct LoopAnalysisPass : public PassInfoMixin<LoopAnalysisPass> {
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            auto &LI = FAM.getResult<LoopAnalysis>(F);
            for (Loop *L : LI) {
                errs() << "Loop detected with header: " << L->getHeader()->getName() << "\n";

                // Print all basic blocks in the loop
                for (BasicBlock *BB : L->blocks()) {
                    errs() << "  Basic Block: " << BB->getName() << "\n";
                }
            }

            return PreservedAnalyses::all();
        }
    };
}

// Register the pass
llvm::PassPluginLibraryInfo getLoopAnalysisPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "LoopAnalysisPass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "loop-analysis") {
                            FPM.addPass(LoopAnalysisPass());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getLoopAnalysisPassPluginInfo();
}

