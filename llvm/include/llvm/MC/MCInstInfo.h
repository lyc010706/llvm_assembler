#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCSymbol.h"
using namespace llvm;
class MCInstInfo
{
public:
    MCInst inst;
    SMLoc IDLoc;
    int isRemove;

    int isSymbol;
    MCSymbol *Sym;
    MCInstInfo(MCInst Inst,SMLoc Loc)
    {
        inst=Inst;
        isRemove=0;
        isSymbol=0;
        Sym=nullptr;
    }

    MCInstInfo(MCSymbol *sym,SMLoc Loc)
    {
        Sym=sym;
        isRemove=0;
        isSymbol=1;
    }
};