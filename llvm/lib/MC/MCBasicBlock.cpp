#include "llvm/MC/MCBasicBlock.h"

MCBasicBlock::MCBasicBlock(unsigned int num)
{
    number=num;
    instList=new MCList<MCInstInfo*>();
    preBlock=new MCList<MCBasicBlock*>();
    succBlock=new MCList<MCBasicBlock*>();

}
bool MCBasicBlock::addMCInstruction(MCInstInfo *MII)
{
    instList->insertBack(MII);
    return 1;
}
    
bool MCBasicBlock::addSuccessor(MCBasicBlock* succ)
{
    succBlock->insertBack(succ);
    return 1;
}

void MCBasicBlock::addPredecessor(MCBasicBlock* pred)
{
    preBlock->insertBack(pred);
}