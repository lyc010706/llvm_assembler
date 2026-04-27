#include "llvm/MC/MCFunctionModule.h"

MCFunctionModule::MCFunctionModule(int num)
{
    number=num;
    islastJ=0;
    instList=new MCList<MCInstInfo*>();
    firstBasicBlock=NULL;
}

bool MCFunctionModule::insertInstoBasicblock(MCInstInfo *MII,unsigned int basicblockNum)
{
    MCBasicBlock *bb=blockSymbolTabel[basicblockNum];
    bb->addMCInstruction(MII);

    instList->insertBack(MII);
}


bool MCFunctionModule::insertSym(MCInstInfo *MII)
{
    instList->insertBack(MII);
}


bool MCFunctionModule::insertBasicBlock(unsigned int newbasicblockNum,unsigned int basicblockNum)
{  
    if(!blockSymbolTabel.count(newbasicblockNum))
    {
        MCBasicBlock *newbb=new MCBasicBlock(newbasicblockNum);
        blockSymbolTabel[newbasicblockNum]=newbb;

        MCBasicBlock *bb=blockSymbolTabel[basicblockNum];
        bb->addSuccessor(newbb);
        newbb->addPredecessor(bb);
    }
    else
    {
        MCBasicBlock *newbb=blockSymbolTabel[newbasicblockNum];
        MCBasicBlock *bb=blockSymbolTabel[basicblockNum];
        bb->addSuccessor(newbb);
        newbb->addPredecessor(bb);
    }
}


bool  MCFunctionModule::insertfirstBasicBlock(unsigned int newbasicblockNum)
{
    firstBasicBlock=new MCBasicBlock(newbasicblockNum);
    blockSymbolTabel[newbasicblockNum]=firstBasicBlock;
}


void MCFunctionModule::inserttempbb(unsigned int newbasicblockNum)
{
    MCBasicBlock *newbb=new MCBasicBlock(newbasicblockNum);
    blockSymbolTabel[newbasicblockNum]=newbb;
}
