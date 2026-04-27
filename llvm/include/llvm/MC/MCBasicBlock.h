#include "llvm/MC/MCList.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstInfo.h"
using namespace llvm;
class MCBasicBlock
{
public:
    MCList<MCInstInfo*> *instList;
    unsigned int number;
    MCList<MCBasicBlock*> *preBlock;
    MCList<MCBasicBlock*> *succBlock;
    MCBasicBlock(unsigned int num);
    bool addMCInstruction(MCInstInfo *MII);
    
    bool addSuccessor(MCBasicBlock* succ);

    void addPredecessor(MCBasicBlock* pred);
};