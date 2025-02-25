#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include <unordered_map>

using namespace llvm;

#define DEBUG_TYPE "lvn"

namespace {

/// **Expression Struct** (Handles Commutativity & Hashing Properly)
struct Expression {
    unsigned Opcode;
    Value *Op1, *Op2;

    bool operator==(const Expression &E) const {
        return Opcode == E.Opcode &&
               ((Op1 == E.Op1 && Op2 == E.Op2) || (Op1 == E.Op2 && Op2 == E.Op1));
    }

    bool operator<(const Expression &E) const {
        return std::tie(Opcode, std::min(Op1, Op2), std::max(Op1, Op2)) <
               std::tie(E.Opcode, std::min(E.Op1, E.Op2), std::max(E.Op1, E.Op2));
    }
};

/// **Hash Function for Expressions**
struct HashExpression {
    size_t operator()(const Expression &E) const {
        return hash_combine(E.Opcode, E.Op1, E.Op2);
    }
};

/// **Local Value Numbering Pass**
class LVNPass : public PassInfoMixin<LVNPass> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
        errs() << "Running LVN Pass on function: " << F.getName() << "\n";

        for (BasicBlock &BB : F) {
            errs() << "Processing Basic Block: " << BB.getName() << "\n";

            std::unordered_map<Value*, int> NameTable;
            std::unordered_map<Expression, int, HashExpression> ValueTable;
            std::unordered_map<int, Value*> NumberToValue;
            int nextValueNumber = 1;

            for (auto it = BB.begin(); it != BB.end();) {
                Instruction &I = *it++;

                if (auto *BO = dyn_cast<BinaryOperator>(&I)) {
                    Value *Op1 = BO->getOperand(0);
                    Value *Op2 = BO->getOperand(1);

                    if (!NameTable.count(Op1)) NameTable[Op1] = nextValueNumber++;
                    if (!NameTable.count(Op2)) NameTable[Op2] = nextValueNumber++;

                    Expression Expr = {BO->getOpcode(), Op1, Op2};

                    if (ValueTable.count(Expr)) {
                        Value *Existing = NumberToValue[ValueTable[Expr]];
                        errs() << "Replacing " << *BO << " with " << *Existing << "\n";
                        BO->replaceAllUsesWith(Existing);
                        BO->eraseFromParent();
                    } else {
                        int VN = nextValueNumber++;
                        ValueTable[Expr] = VN;
                        NumberToValue[VN] = BO;
                        NameTable[BO] = VN;
                    }
                }
            }
        }
        return PreservedAnalyses::none();
    }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "LVNPass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "lvn") {
                            FPM.addPass(LVNPass());
                            return true;
                        }
                        return false;
                    });
            }};
}
