#include "llvm/MC/MCBasicBlock.h"
#include "llvm/ADT/StringMap.h"
#include <unordered_map>
#include <set>
class  MCFunctionModule
{
public:
    //specific number per func
    unsigned int number;
    MCBasicBlock* firstBasicBlock;
    MCList<MCInstInfo*> *instList;
    MCFunctionModule(int num);
    std::unordered_map<unsigned int,MCBasicBlock*> blockSymbolTabel;
    //std::unordered_map<StringRef,int> blockNum;
    llvm::StringMap<int> blockNum;

    bool islastJ;

    bool insertInstoBasicblock(MCInstInfo *MII,unsigned int basicblockNum);

    bool insertSym(MCInstInfo *MII);

    bool insertBasicBlock(unsigned int newbasicblockNum,unsigned int basicblockNum);

    bool insertfirstBasicBlock(unsigned int newbasicblockNum);

    void inserttempbb(unsigned int newbasicblockNum);

};