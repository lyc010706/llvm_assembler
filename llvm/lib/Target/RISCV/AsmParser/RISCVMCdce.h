#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include <unordered_map>
#include <queue>
class RISCVMcdce
{
public:
    std::unordered_map<MCBasicBlock*,unsigned int> basicblockOutdegree;
    std::unordered_map<MCBasicBlock*,std::set<unsigned int>> useSets;
    std::unordered_map<MCBasicBlock*,std::set<unsigned int>> defSets;
    std::unordered_map<MCBasicBlock*,std::set<unsigned int>> liveInSets;
    std::unordered_map<MCBasicBlock*,std::set<unsigned int>> liveOutSets;

    std::unordered_map<unsigned int ,MCBasicBlock*> postorder;

    std::set<MCBasicBlock*>outBasicBlock;
    MCFunctionModule* MCFM;
    RISCVMcdce(MCFunctionModule* MCFM);
    void caculteLiveInOutsSets();
    void caculateUseDefSets();
    void dce();
    bool hasSideEffects(MCInst &mi,std::set<unsigned int>&livenessset);
    bool dealAssignment(MCInst &mi,std::set<unsigned int>&livenessset);

    void caculatePostorder(MCBasicBlock *curbb,unsigned int &num,std::set<MCBasicBlock*>&visited_set);
};