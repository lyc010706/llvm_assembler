#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include <unordered_map>
#include <queue>
class ConstantState
{
public:
    int state;//-1为不确定值，0为常量，1为非常量
    int num;
    ConstantState()
    {
        state=-1;
    }
    ConstantState(int val)
    {
        state=0;
        num=val;
    }
};
class RISCVMcconstant
{
public:
    std::unordered_map<MCBasicBlock*,std::unordered_map<unsigned int,ConstantState*>> genSets;
    std::unordered_map<MCBasicBlock*,std::unordered_map<unsigned int,ConstantState*>> inSets;
    std::unordered_map<MCBasicBlock*,std::unordered_map<unsigned int,ConstantState*>> outSets;
    MCFunctionModule* MCFM;
    int caculCons(int a, int b,unsigned u);
    int caculCons(int a, int64_t b,unsigned u);
    int mulh_32(int a, int b);
    int mulhu_32(int a, int b);
    int mulhsu_32(int a, int b);
    RISCVMcconstant(MCFunctionModule* mcfm);
    void caculGenSets();
    void caculteInOutsSets();
    void caculatePostorder(MCBasicBlock *curbb,unsigned int &num,std::set<MCBasicBlock*>&visited_set);
    std::unordered_map<unsigned int ,MCBasicBlock*> postorder;

    void constant();
    bool IsBranch(int a, int b,unsigned u);
};
