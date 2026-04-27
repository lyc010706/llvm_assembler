#include "RISCVMCconstant.h"
#include "MCTargetDesc/RISCVMCTargetDesc.h"
using namespace llvm;
RISCVMcconstant::RISCVMcconstant(MCFunctionModule* mcfm)
{
    MCFM=mcfm;
}
int RISCVMcconstant::mulh_32(int a, int b) {
    // 直接使用64位整数进行乘法
    int64_t product = (int64_t)a * (int64_t)b;
    
    // 右移32位获取高位部分
    return (int32_t)(product >> 32);
}
int RISCVMcconstant::mulhu_32(int a, int b) {
    // 直接使用64位整数进行乘法
    int64_t product = (int64_t)(static_cast<unsigned int>(a)) * (int64_t)(static_cast<unsigned int>(b));
    
    // 右移32位获取高位部分
    return (int32_t)(product >> 32);
}
int RISCVMcconstant::mulhsu_32(int a, int b) {
    // 直接使用64位整数进行乘法
    int64_t product = (int64_t)(static_cast<unsigned int>(a)) * (int64_t)(static_cast<unsigned int>(b));
    
    // 右移32位获取高位部分
    return (int32_t)(product >> 32);
}
bool RISCVMcconstant::IsBranch(int a, int b,unsigned u)
{
    int ans;
    switch(u)
    {
        default:
        {
            return false;
            break;
        }
        case RISCV::BEQ:
        {
            if(a==b) return true;
            return false;
        }
        case RISCV::BNE:
        {
            if(a!=b) return true;
            return false;
        }
        case RISCV::BLT:
        {
            if(a<b) return true;
            return false;
        }
        case RISCV::BGE:
        {
            if(a>b) return true;
            return false;
        }
        case RISCV::BLTU:
        {
            if((unsigned int)(a)<(unsigned int)(b)) return true;
            return false;
        }
        case RISCV::BGEU:
        {
            if((unsigned int)(a)>(unsigned int)(b)) return true;
            return false;
        }
    }
}
int RISCVMcconstant::caculCons(int a, int64_t b,unsigned u)
{
    int ans;
    switch(u)
    {
        default:
        {
            break;
        }
        case RISCV::XORI:
        {
            return a|b;
        }
        case RISCV::SLLI:
        {
            return a<<b;
        }
        case RISCV::SRAI:
        {
            return a>>b;
        }
        case RISCV::SRLI:
        {
            return static_cast<int>(static_cast<unsigned int>(a)>>b);
        }
        case RISCV::SLTI:
        {
            return a<((b<<52)>>52);
        }
        case RISCV::SLTIU:
        {
            return static_cast<int>(static_cast<unsigned int>(a)<b);
        }
        case RISCV::ADDI:
        {
            return a+b;
        }
        case RISCV::ANDI:
        {
            return a&b;
        }
        case RISCV::ORI:
        {
            return a|b;
        }
    }
}
int RISCVMcconstant::caculCons(int a, int b,unsigned u)
{
    int ans;
    switch(u)
    {
        case RISCV::ADD:
        {
            return a+b;
        }
        case RISCV::MUL:
        {
            return a*b;
        }
        case RISCV::MULH:
        {
            return mulh_32(a,b);
        }
        case RISCV::MULHU:
        {
            return mulhu_32(a,b);
        }
        case RISCV::MULHSU:
        {
            return mulhsu_32(a,b);
        }
        case RISCV::DIV:
        {
            return a/b;
        }
        case RISCV::DIVU:
        {
            return (static_cast<unsigned int>(a))/(static_cast<unsigned int>(b));
        }
        case RISCV::REM:
        {
            return a%b;
        }
        case RISCV::REMU:
        {
            return (static_cast<unsigned int>(a))%(static_cast<unsigned int>(b));
        }
        case RISCV::SLT:
        {
            return a<b;
        }
        case RISCV::OR:
        {
            return a|b;
        }
        case RISCV::AND:
        {
            return a&b;
        }
        case RISCV::XOR:
        {
            return a^b;
        }
        case RISCV::SLL:
        {
            return a<<b;
        }
        case RISCV::SRL:
        {
            return static_cast<int>(static_cast<unsigned int>(a)>>b);
        }
        case RISCV::SUB:
        {
            return a-b;
        }
        case RISCV::SRA:
        {
            return a>>b;
        }
        case RISCV::SLTU:
        {
            return static_cast<unsigned int>(a)<static_cast<unsigned int>(b);
        }
    }
}
void RISCVMcconstant::caculGenSets()
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
        std::unordered_map<unsigned int,ConstantState*>iset;
        std::unordered_map<unsigned int,ConstantState*>oset;
        inSets[curbb]=iset;
        outSets[curbb]=oset;

        std::unordered_map<unsigned int,ConstantState*> gSet;
        gSet[41]=new ConstantState(0);
        genSets[curbb]=gSet;
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
                    if(Reg1==Reg0)
                    {
                        if(genSets[curbb].count(Reg0)) genSets[curbb][Reg0]->state=1;
                        else{
                            genSets[curbb][Reg0]=new ConstantState();
                            genSets[curbb][Reg0]->state=1;
                        }
                        break;
                    }
                    __int64_t imm=temp.getOperand(2).getImm();
                    if(genSets[curbb].count(Reg1)&&genSets[curbb][Reg1]->state==0)
                    {
                        int val=caculCons(genSets[curbb][Reg1]->num,imm,temp.getOpcode());
                        genSets[curbb][Reg0]=new ConstantState(val);
                    }
                    else if(genSets[curbb].count(Reg1)&&genSets[curbb][Reg1]->state==1)
                    {
                        genSets[curbb][Reg0]=new ConstantState();
                        genSets[curbb][Reg0]->state=1;
                    }
                    else
                    {
                        genSets[curbb][Reg0]=new ConstantState();
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
                case RISCV::REMU:
                case RISCV::REM:
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
                    if(Reg1==Reg0||Reg2==Reg0)
                    {
                        if(genSets[curbb].count(Reg0)) genSets[curbb][Reg0]->state=1;
                        else{
                            genSets[curbb][Reg0]=new ConstantState();
                            genSets[curbb][Reg0]->state=1;
                        }
                        break;
                    }
                    if(genSets[curbb].count(Reg1)&&genSets[curbb][Reg1]->state==0&&genSets[curbb].count(Reg2)&&genSets[curbb][Reg1]->state==0)
                    {
                        int val=caculCons(genSets[curbb][Reg1]->num,genSets[curbb][Reg2]->num,temp.getOpcode());
                        genSets[curbb][Reg0]=new ConstantState(val);
                    }
                    else if((genSets[curbb].count(Reg1)&&genSets[curbb][Reg1]->state==1)||(genSets[curbb].count(Reg2)&&genSets[curbb][Reg2]->state==1))
                    {
                        genSets[curbb][Reg0]=new ConstantState();
                        genSets[curbb][Reg0]->state=1;
                    }
                    else
                    {
                        genSets[curbb][Reg0]=new ConstantState();
                    }
                    break;
               }
               
            }
        }
    }
}
void RISCVMcconstant::caculteInOutsSets()
{
    int ischanged=1;
    unsigned  ordernum=0;
    std::set<MCBasicBlock*>visited_set;
    caculatePostorder(MCFM->firstBasicBlock,ordernum,visited_set);
    while(ischanged)
    {
        ischanged=0;
        for(int i=postorder.size()-1;i>=0;i--)
        {
            MCBasicBlock*curbb=postorder[i];
            MCListNode<MCBasicBlock*> *curprevbb=curbb->preBlock->firstNode;
            std::unordered_map<unsigned int,ConstantState*>tempOset;
            std::unordered_map<unsigned int,ConstantState*>tempIset;
            // for (auto it = genSets[curbb].begin(); it != genSets[curbb].end(); ++it) {
            //     tempOset[it->first]=it->second;
            // }
            while(curprevbb!=NULL)
            {
                for (auto it = outSets[curprevbb->instInfo].begin(); it != outSets[curprevbb->instInfo].end(); ++it) 
                {
                    if(it->second->state==1||(inSets[curbb].count(it->first)&&inSets[curbb][it->first]->state==1))
                    {
                        if(!inSets[curbb].count(it->first)||(inSets[curbb].count(it->first)&&inSets[curbb][it->first]->state!=1)) ischanged=1;
                        inSets[curbb][it->first]=new ConstantState();
                        inSets[curbb][it->first]->state=1;
                    }
                    else if(it->second->state==0&&(inSets[curbb].count(it->first)&&inSets[curbb][it->first]->state==0&&inSets[curbb][it->first]->num==it->second->num))
                    {
                        inSets[curbb][it->first]=new ConstantState(it->second->num);
                    }
                    else if((it->second->state==0&&(inSets[curbb].count(it->first)&&inSets[curbb][it->first]->state==-1))||(it->second->state==0&&(!inSets[curbb].count(it->first))))
                    {
                        ischanged=1;
                        inSets[curbb][it->first]=new ConstantState(it->second->num);
                    }
                    else
                    {
                        if(!inSets[curbb].count(it->first)||(inSets[curbb].count(it->first)&&inSets[curbb][it->first]->state!=-1)) ischanged=1;
                        inSets[curbb][it->first]=new ConstantState();
                    }
                }
                curprevbb=curprevbb->nextNode;
            }
            //inSets[curbb]=tempIset;
            for (auto it = inSets[curbb].begin(); it != inSets[curbb].end(); ++it) 
            {
                if(it->second->state==1||(outSets[curbb].count(it->first)&&outSets[curbb][it->first]->state==1))
                {
                    if(!outSets[curbb].count(it->first)||(outSets[curbb].count(it->first)&&outSets[curbb][it->first]->state!=1)) ischanged=1;
                    outSets[curbb][it->first]=new ConstantState();
                    outSets[curbb][it->first]->state=1;
                }
                else if(it->second->state==0&&(outSets[curbb].count(it->first)&&outSets[curbb][it->first]->state==0&&outSets[curbb][it->first]->num==it->second->num))
                {
                    outSets[curbb][it->first]=new ConstantState(it->second->num);
                }
                else if((it->second->state==0&&(outSets[curbb].count(it->first)&&outSets[curbb][it->first]->state==-1))||(it->second->state==0&&(!outSets[curbb].count(it->first))))
                {
                    ischanged=1;
                    outSets[curbb][it->first]=new ConstantState(it->second->num);
                }
                else
                {
                    if(!outSets[curbb].count(it->first)||(outSets[curbb].count(it->first)&&outSets[curbb][it->first]->state!=-1)) ischanged=1;
                    outSets[curbb][it->first]=new ConstantState();
                }
            }


        }
    }
}

void RISCVMcconstant::constant()
{
    caculGenSets();
    caculteInOutsSets();
    MCBasicBlock *firstbb=MCFM->firstBasicBlock;
    std::queue<MCBasicBlock*> dfsQueue;
    dfsQueue.push(firstbb);
    std::set<MCBasicBlock*>visited_set;
    visited_set.insert(firstbb);
    //isChanged=0;
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

        MCListNode<MCInstInfo*>* currInstNode=instList->firstNode;
        std::unordered_map<unsigned int,ConstantState*> conVal;
        conVal=inSets[curbb];
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
                    if(conVal.count(Reg1)&&conVal[Reg1]->state==0)
                    {
                        __int64_t imm=temp.getOperand(2).getImm();
                        int val=caculCons(genSets[curbb][Reg1]->num,imm,temp.getOpcode());

                        conVal[Reg0]->state=1;
                        conVal[Reg0]->num=val;
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
                case RISCV::REMU:
                case RISCV::REM:
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
                    if(conVal.count(Reg1)&&conVal[Reg1]->state==0&&conVal.count(Reg2)&&conVal[Reg2]->state==0)
                    {
                        int val=caculCons(genSets[curbb][Reg1]->num,genSets[curbb][Reg2]->num,temp.getOpcode());
                        conVal[Reg0]->state=1;
                        conVal[Reg0]->num=val;
                    }
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
                    bool isb=true;
                    if(conVal.count(Reg1)&&conVal[Reg1]->state==0&&conVal.count(Reg0)&&conVal[Reg0]->state==0)
                    {
                        isb=IsBranch(conVal[Reg0]->num,conVal[Reg1]->num,temp.getOpcode());
                    }
                    if(isb==false)
                    {
                        currInstNode->instInfo->isRemove=1;
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
                }


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

void RISCVMcconstant::caculatePostorder(MCBasicBlock *curbb,unsigned int &num,std::set<MCBasicBlock*>&visited_set)
{
   visited_set.insert(curbb);
    MCListNode<MCBasicBlock*> *cursuccessvbb=curbb->succBlock->firstNode;
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