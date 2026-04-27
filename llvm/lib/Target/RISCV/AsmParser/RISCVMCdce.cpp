#include "RISCVMCdce.h"
#include "MCTargetDesc/RISCVMCTargetDesc.h"
using namespace llvm;
RISCVMcdce::RISCVMcdce(MCFunctionModule* mcfm)
{
    MCFM=mcfm;
}

void RISCVMcdce::caculateUseDefSets()
{
    MCBasicBlock *firstbb=MCFM->firstBasicBlock;
    std::queue<MCBasicBlock*> dfsQueue;
    dfsQueue.push(firstbb);
    std::set<MCBasicBlock*>visited_set;
    visited_set.insert(firstbb);
    while(!dfsQueue.empty())
    {
        MCBasicBlock*curbb=dfsQueue.front();
        dfsQueue.pop();

        MCListNode<MCBasicBlock*> *cursuccessbb=curbb->succBlock->firstNode;
        //if(cursuccessbb==NULL) outBasicBlock.insert(cursuccessbb->instInfo);
        //int outdegree=0;
        while(cursuccessbb!=NULL)
        {
            if(!visited_set.count(cursuccessbb->instInfo))
            {
                dfsQueue.push(cursuccessbb->instInfo);
                visited_set.insert(cursuccessbb->instInfo);
            }
            //outdegree++;
            cursuccessbb=cursuccessbb->nextNode;
        }
        //basicblockOutdegree[curbb]=outdegree;
        //if(outdegree==0) outBasicBlock.insert(curbb);

        std::set<unsigned int>uset;
        std::set<unsigned int>dset;

        std::set<unsigned int>liset;
        std::set<unsigned int>loset;

        liveInSets[curbb]=liset;
        liveOutSets[curbb]=loset;
        useSets[curbb]=uset;
        defSets[curbb]=dset;
        MCList<MCInstInfo*> * instList=curbb->instList;
        MCListNode<MCInstInfo*>* currInstNode=instList->firstNode;
        while (currInstNode!=NULL)
        {
            MCInst temp=currInstNode->instInfo->inst;
            switch(temp.getOpcode())
            {
                 default:
                {
                    break;
                }
                case RISCV::XORI:
                case RISCV::SLLI:
                case RISCV::SRAI:
                case RISCV::SRLI:
                case RISCV::SLTI:
                case RISCV::SLTIU:
                case RISCV::ADDI:
                case RISCV::ANDI:
                case RISCV::ORI:
                {
                    unsigned int Reg1 = temp.getOperand(1).getReg();
                    unsigned int Reg0 = temp.getOperand(0).getReg();
                    if(!defSets[curbb].count(Reg1))
                    {
                        useSets[curbb].insert(Reg1);
                    }
                    defSets[curbb].insert(Reg0);
                    break;
                }
                case RISCV::SH:
                case RISCV::SW:
                case RISCV::SB:
                {
                    unsigned int Reg1 = temp.getOperand(1).getReg();
                    unsigned int Reg0 = temp.getOperand(0).getReg();
                    if(!defSets[curbb].count(Reg1))
                    {
                        useSets[curbb].insert(Reg1);
                    }
                    if(!defSets[curbb].count(Reg0))
                    {
                        useSets[curbb].insert(Reg0);
                    }
                    break;
                }
                case RISCV::LHU:
                case RISCV::LH:
                case RISCV::LBU:
                case RISCV::LB:
                case RISCV::LW:
                {
                    unsigned int Reg1 = temp.getOperand(1).getReg();
                    unsigned int Reg0 = temp.getOperand(0).getReg();
                    if(!defSets[curbb].count(Reg1))
                    {
                        useSets[curbb].insert(Reg1);
                    }
                    defSets[curbb].insert(Reg0);
                    break;
                }
                case RISCV::BEQ:
                case RISCV::BNE:
                case RISCV::BLT:
                case RISCV::BGE:
                case RISCV::BLTU:
                case RISCV::BGEU:
                {
                     unsigned int Reg1 = temp.getOperand(1).getReg();
                    unsigned int Reg0 = temp.getOperand(0).getReg();
                    if(!defSets[curbb].count(Reg1))
                    {
                        useSets[curbb].insert(Reg1);
                    }
                    if(!defSets[curbb].count(Reg0))
                    {
                        useSets[curbb].insert(Reg0);
                    }
                    break;
                }   
                case RISCV::JALR:
                {
                    unsigned int Reg1 = temp.getOperand(1).getReg();
                    unsigned int Reg0 = temp.getOperand(0).getReg();
                    if(!defSets[curbb].count(Reg1))
                    {
                        useSets[curbb].insert(Reg1);
                    }
                    if(Reg0!=41&&Reg0!=42)
                    {
                        defSets[curbb].insert(Reg0);
                    }
                    break;
                }
                case RISCV::JAL:
                {
                    unsigned int Reg0 = temp.getOperand(0).getReg();
                    if(Reg0!=41&&Reg0!=42)
                    {
                        defSets[curbb].insert(Reg0);
                    }
                    break;
                }
                case RISCV::ADD:
                case RISCV::MUL:
                case RISCV::MULH:
                case RISCV::MULHU:
                case RISCV::MULHSU:
                case RISCV::DIV:
                case RISCV::DIVU:
                case RISCV::REM:
                case RISCV::REMU:
                case RISCV::SLT:
                case RISCV::OR:
                case RISCV::AND:
                case RISCV::XOR:
                case RISCV::SLL:
                case RISCV::SRL:
                case RISCV::SUB:
                case RISCV::SRA:
                case RISCV::SLTU:
                {
                    unsigned int Reg1 = temp.getOperand(1).getReg();
                    unsigned int Reg2 = temp.getOperand(2).getReg();
                    unsigned int Reg0 = temp.getOperand(0).getReg();
                    if(!defSets[curbb].count(Reg1))
                    {
                        useSets[curbb].insert(Reg1);
                    }
                    if(!defSets[curbb].count(Reg2))
                    {
                        useSets[curbb].insert(Reg2);
                    }
                    defSets[curbb].insert(Reg0);
                    break;
                }
                case RISCV::AUIPC:
                case RISCV::LUI:
                {
                    unsigned int Reg0 = temp.getOperand(0).getReg();
                    defSets[curbb].insert(Reg0);
                    break;
                }
                //case RISCV::PseudoTAIL:
                case RISCV::PseudoCALL:
                {
                    for(int i=51;i<=58;i++)
                    {
                        if(!defSets[curbb].count(i))
                        {
                            useSets[curbb].insert(i);
                        }
                    }
                    break;
                }
                case RISCV::PseudoTAIL:
                {
                    for(int i=51;i<=58;i++)
                    {
                        if(!defSets[curbb].count(i))
                        {
                            useSets[curbb].insert(i);
                        }
                    }
                    if(!defSets[curbb].count(42))
                    {
                        useSets[curbb].insert(42);
                    }
                    break;
                }
                    
            }
            currInstNode=currInstNode->nextNode;

        }
        
    }
}


void RISCVMcdce::caculteLiveInOutsSets()
{
    int ischanged=1;
    unsigned  ordernum=0;
    std::set<MCBasicBlock*>visited_set;
    caculatePostorder(MCFM->firstBasicBlock,ordernum,visited_set);

    for (auto it = outBasicBlock.begin(); it != outBasicBlock.end(); ++it) 
    {
        std::set<unsigned int> savedres={49,50,59,60,61,62,63,64,65,66,67,68,51,52,43};//s0-s11  a0 a1 sp ra
        liveOutSets[*it]=savedres;
    }

    while(ischanged)
    {
        ischanged=0;
        std::queue<MCBasicBlock*> dfsQueue;
        // for (auto it = outBasicBlock.begin(); it != outBasicBlock.end(); ++it) 
        // {
        //     dfsQueue.push(*it);
        //     //std::set<unsigned int> savedres={49,50,59,60,61,62,63,64,65,66,67,68};//s1-s11
        //     //liveOutSets[*it]=savedres;
        // }

        //while(!dfsQueue.empty())
        for(int i=0;i<postorder.size();i++)
        {
            //MCBasicBlock*curbb=dfsQueue.front();
            MCBasicBlock*curbb=postorder[i];
            //dfsQueue.pop();


            //MCListNode<MCBasicBlock*> *curprevbb=curbb->preBlock->firstNode;
            // while(curprevbb!=NULL)
            // {
            //   basicblockOutdegree[curprevbb->instInfo]--;
            //   if(basicblockOutdegree[curprevbb->instInfo]==0)
            //   {
            //     dfsQueue.push(curprevbb->instInfo);
            //   }
            //   curprevbb=curprevbb->prevNode;
            // }


            MCListNode<MCBasicBlock*> *cursuccessbb=curbb->succBlock->firstNode;
            while(cursuccessbb!=NULL)
            {
               //liveOutSets[curbb].insert(liveInSets[cursuccessbb->instInfo].begin(),liveInSets[cursuccessbb->instInfo].end());

               for (unsigned  num : liveInSets[cursuccessbb->instInfo]) {
                    auto result =liveOutSets[curbb].insert(num);
                    if (result.second) {
                        ischanged=1;
                    }
                }
                cursuccessbb=cursuccessbb->nextNode;
            }

            std::set<unsigned int> differset;
            std::set_difference(
                liveOutSets[curbb].begin(), liveOutSets[curbb].end(),
                defSets[curbb].begin(), defSets[curbb].end(),std::inserter(differset,differset.begin()));
            
            //liveInSets.insert(differset.begin(),differset.end());
            for (unsigned  num : differset) {
                auto result =liveInSets[curbb].insert(num);
                if (result.second) {
                    ischanged=1;
                }
            }

            for (unsigned  num : useSets[curbb]) {
                auto result =liveInSets[curbb].insert(num);
                if (result.second) {
                    ischanged=1;
                }
            }
            
        }


    }

}

void RISCVMcdce::dce()
{
    int isChanged=1;
    while(isChanged)
    {
        caculateUseDefSets();
        caculteLiveInOutsSets();
        MCBasicBlock *firstbb=MCFM->firstBasicBlock;
        std::queue<MCBasicBlock*> dfsQueue;
        dfsQueue.push(firstbb);
        std::set<MCBasicBlock*>visited_set;
        visited_set.insert(firstbb);
        isChanged=0;
        while(!dfsQueue.empty())
        {

            MCBasicBlock*curbb=dfsQueue.front();
            dfsQueue.pop();
            MCListNode<MCBasicBlock*> *cursuccessbb=curbb->succBlock->firstNode;
            while(cursuccessbb!=NULL)
            {
                if(!visited_set.count(cursuccessbb->instInfo))
                {
                    dfsQueue.push(cursuccessbb->instInfo);
                    visited_set.insert(cursuccessbb->instInfo);
                }
                cursuccessbb=cursuccessbb->nextNode;
            }

            MCList<MCInstInfo*> * instList=curbb->instList;

            MCListNode<MCInstInfo*>* currInstNode=instList->lastNode;


            std::set<unsigned int>livenessset;
            livenessset=liveOutSets[curbb];

            while (currInstNode!=NULL)
            {
                if(hasSideEffects(currInstNode->instInfo->inst,livenessset)){currInstNode=currInstNode->prevNode;}
                else
                {
                    if(!dealAssignment(currInstNode->instInfo->inst,livenessset))
                    {
                        currInstNode->instInfo->isRemove=1;
                        isChanged=1;
                        if(instList->lastNode==instList->firstNode) //基本快只有一句的情况
                        {
                            free(currInstNode);
                            instList->firstNode=NULL;
                            instList->lastNode=NULL;
                            currInstNode=NULL;
                            continue;

                        }
                        if(currInstNode==instList->lastNode) 
                        {
                            instList->lastNode=currInstNode->prevNode;
                            currInstNode->prevNode->nextNode=nullptr;
                            MCListNode<MCInstInfo*>* deletenode=currInstNode;
                            currInstNode=currInstNode->prevNode;
                            free(deletenode);
                        }
                        else if(currInstNode==instList->firstNode)
                        {
                            instList->firstNode=currInstNode->nextNode;
                            currInstNode->nextNode->prevNode=nullptr;
                            free(currInstNode);
                            currInstNode=NULL;
                        }
                        else
                        {
                            currInstNode->prevNode->nextNode=currInstNode->nextNode;
                            currInstNode->nextNode->prevNode=currInstNode->prevNode;
                            MCListNode<MCInstInfo*>* deletenode=currInstNode;
                            currInstNode=currInstNode->prevNode;
                            free(deletenode);
                        }
                    }
                    else
                    {
                        currInstNode=currInstNode->prevNode;
                    }
                }
                //currInstNode=currInstNode->nextNode;
            }
        }
    }


    MCList<MCInstInfo*>* tempList=MCFM->instList;
    MCListNode<MCInstInfo*>* tempnode=tempList->firstNode;
    //while(tempnode!=MCFM->instList->lastNode)
    while(tempnode!=NULL)
    {
      if(tempnode->instInfo->isRemove==1)
      {
        if(tempnode==tempList->firstNode)
        {
            tempnode->nextNode->prevNode=nullptr;
            tempList->firstNode=tempnode->nextNode;;
            MCListNode<MCInstInfo*>* deletenode=tempnode;
            tempnode=tempnode->nextNode;
            free(deletenode->instInfo);
            free(deletenode);
             continue;
        }
        tempnode->prevNode->nextNode=tempnode->nextNode;
        tempnode->nextNode->prevNode=tempnode->prevNode;
        MCListNode<MCInstInfo*>* deletenode=tempnode;
        tempnode=tempnode->nextNode;
        free(deletenode->instInfo);
        free(deletenode);
        continue;
      }
      tempnode=tempnode->nextNode;
    }

}

bool RISCVMcdce::hasSideEffects(MCInst &mi,std::set<unsigned int>&livenessset)
{
    switch(mi.getOpcode())
    {
        default:
        {
            return false;
        }
        case RISCV::SH:
        case RISCV::SW:
        case RISCV::SB:
        {
           livenessset.insert(mi.getOperand(0).getReg());
           livenessset.insert(mi.getOperand(1).getReg());
           return true;
        }
        case RISCV::JALR:
        {
            livenessset.insert(mi.getOperand(1).getReg());
            unsigned int Reg0=mi.getOperand(0).getReg();
            if(Reg0!=41&&Reg0!=42)
            {
                livenessset.erase(Reg0);
            }
            return true;
        }
        case RISCV::JAL:
        {
            unsigned int Reg0=mi.getOperand(0).getReg();
            if(Reg0!=41&&Reg0!=42)
            {
                livenessset.erase(Reg0);
            }
            return true;
        }
        case RISCV::BEQ:
        case RISCV::BNE:
        case RISCV::BGE:
        case RISCV::BLTU:
        case RISCV::BGEU:
        case RISCV::BLT:
        {
            livenessset.insert(mi.getOperand(0).getReg());
            livenessset.insert(mi.getOperand(1).getReg());
            return true;
        }
        //case RISCV::PseudoTAIL:
        case RISCV::PseudoCALL:
        {
            for(int i=51;i<=58;i++)
            {
                livenessset.insert(i);
            }
            return true;
        }
        case RISCV::PseudoTAIL:
        {
             for(int i=51;i<=58;i++)
            {
                livenessset.insert(i);
            }
            livenessset.insert(42);
            return true;
        }
        

    }
}


bool RISCVMcdce::dealAssignment(MCInst &mi,std::set<unsigned int>&livenessset)
{
     switch(mi.getOpcode())
    {
        default:
        {
            return false;
        }
        case RISCV::XORI:
        case RISCV::SLLI:
        case RISCV::SRAI:
        case RISCV::SRLI:
        case RISCV::SLTI:
        case RISCV::SLTIU:
        case RISCV::ADDI:
        case RISCV::ANDI:
        case RISCV::ORI:
        {
            if(livenessset.count(mi.getOperand(0).getReg()))
            {
                livenessset.erase(mi.getOperand(0).getReg());
                livenessset.insert(mi.getOperand(1).getReg());
                return true;
            }
           return false;
        }
        case RISCV::LHU:
        case RISCV::LH:
        case RISCV::LBU:
        case RISCV::LB:
        case RISCV::LW:
        {
            if(livenessset.count(mi.getOperand(0).getReg()))
            {
                livenessset.erase(mi.getOperand(0).getReg());
                livenessset.insert(mi.getOperand(1).getReg());
                return true;
            }
           return false;
        }
        case RISCV::AUIPC:
        case RISCV::LUI:
        {
            if(livenessset.count(mi.getOperand(0).getReg()))
            {
                livenessset.erase(mi.getOperand(0).getReg());
                //livenessset.insert(mi.getOperand(1).getReg());
                return true;
            }
           return false;
        }
        case RISCV::ADD:
        case RISCV::MUL:
        case RISCV::MULH:
        case RISCV::MULHU:
        case RISCV::MULHSU:
        case RISCV::DIV:
        case RISCV::DIVU:
        case RISCV::REM:
        case RISCV::REMU:
        case RISCV::SLT:
        case RISCV::OR:
        case RISCV::AND:
        case RISCV::XOR:
        case RISCV::SLL:
        case RISCV::SRL:
        case RISCV::SUB:
        case RISCV::SRA:
        case RISCV::SLTU:
        {
            if(livenessset.count(mi.getOperand(0).getReg()))
            {
                livenessset.erase(mi.getOperand(0).getReg());
                livenessset.insert(mi.getOperand(1).getReg());
                livenessset.insert(mi.getOperand(2).getReg());
                return true;
            }
           return false;
        }
        

    }
}


void RISCVMcdce::caculatePostorder(MCBasicBlock *curbb,unsigned int &num,std::set<MCBasicBlock*>&visited_set)
{
    visited_set.insert(curbb);
    MCListNode<MCBasicBlock*> *cursuccessvbb=curbb->succBlock->firstNode;
    if(cursuccessvbb==NULL) outBasicBlock.insert(curbb);
    while(cursuccessvbb!=NULL)
    {
        if(!visited_set.count(cursuccessvbb->instInfo))
        {
            caculatePostorder(cursuccessvbb->instInfo,num,visited_set);
        }
        cursuccessvbb=cursuccessvbb->nextNode;
    }
    postorder[num]=curbb;
    num++;
           
}   


