#include "llvm/IR/Function.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include <unordered_map>
#include <tuple>

using namespace llvm;

// Custom hash function for std::tuple<unsigned int, Value*, Value*>
struct TupleHash {
    std::size_t operator()(const std::tuple<unsigned int, Value*, Value*>& key) const {
        auto [opcode, op1, op2] = key;
        std::size_t h1 = std::hash<unsigned int>{}(opcode);
        std::size_t h2 = std::hash<Value*>{}(op1);
        std::size_t h3 = std::hash<Value*>{}(op2);
        return h1 ^ (h2 << 1) ^ (h3 << 2);  // Combine hash values
    }
};

namespace {
struct LVNPass : public PassInfoMixin<LVNPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
        std::unordered_map<std::tuple<unsigned int, Value*, Value*>, Value*, TupleHash> valueTable;

        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (I.isBinaryOp()) {
                    unsigned int opcode = I.getOpcode();
                    Value* op1 = I.getOperand(0);
                    Value* op2 = I.getOperand(1);

                    std::tuple<unsigned int, Value*, Value*> key = {opcode, op1, op2};

                    if (valueTable.count(key)) {
                        // Replace the instruction with the previously computed value
                        I.replaceAllUsesWith(valueTable[key]);
                        errs() << "LVN: Replaced " << I << " with existing value.\n";
                        I.eraseFromParent();
                    } else {
                        // Store this instruction as the canonical computation
                        valueTable[key] = &I;
                    }
                }
            }
        }
        return PreservedAnalyses::all();
    }
};
} // namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "LVNPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "lvn-pass") {
                        FPM.addPass(LVNPass());
                        return true;
                    }
                    return false;
                });
        }
    };
}
