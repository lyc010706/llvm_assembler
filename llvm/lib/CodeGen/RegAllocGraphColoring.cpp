#include "RegAllocBase.h"
#include "llvm/CodeGen/LiveInterval.h"
#include "llvm/CodeGen/LiveRangeEdit.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/VirtRegMap.h"
#include "llvm/CodeGen/Spiller.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/MachineBlockFrequencyInfo.h"
#include "llvm/CodeGen/CalcSpillWeights.h"
#include "llvm/Pass.h"
#include "llvm/InitializePasses.h"
#include "llvm/CodeGen/LiveStacks.h"
#include "llvm/CodeGen/MachineDominators.h"
using namespace llvm;
namespace llvm {
    FunctionPass* createGraphColourRegisterAllocator();
}

namespace  {
class InterferenceGraph {
public:
  // 邻接表: VReg -> 与之冲突的VReg集合
  DenseMap<Register, SmallSet<Register, 8>> AdjacencyList;

  DenseMap<Register, SmallSet<Register, 8>> wholeAdjacencyList;
  
  // 寄存器度数缓存
  DenseMap<Register, unsigned> DegreeCache;
  
  DenseMap<Register, unsigned> ComponentNum;

  
  // 需要重新计算度数的标志
  bool DegreesDirty = true;
public:
  InterferenceGraph() = default;
  
  void addNode(Register VReg1)
  {
    if (AdjacencyList.count(VReg1))
      return;

    SmallSet<Register, 8> ss;
    AdjacencyList[VReg1]=ss;

    wholeAdjacencyList[VReg1]=ss;
    
  }
  // 添加冲突边
  void addInterference(Register VReg1, Register VReg2) {
    if (VReg1 == VReg2) return;
    
    // 检查边是否已存在
    if (AdjacencyList[VReg1].count(VReg2))
      return;
    
    AdjacencyList[VReg1].insert(VReg2);
    AdjacencyList[VReg2].insert(VReg1);

    wholeAdjacencyList[VReg1].insert(VReg2);
    wholeAdjacencyList[VReg2].insert(VReg1);
    DegreesDirty = true;
  }

   // 检查是否有冲突
  bool interferes(Register VReg1, Register VReg2) const {
    auto It = AdjacencyList.find(VReg1);
    if (It == AdjacencyList.end())
      return false;
    
    return It->second.count(VReg2);
  }


  // 获取与指定虚拟寄存器冲突的所有寄存器
  SmallVector<unsigned, 8> getInterferingVRegs(Register VReg) const {
    SmallVector<unsigned, 8> Result;
    auto It = wholeAdjacencyList.find(VReg);
    
    if (It != wholeAdjacencyList.end()) {
      for (unsigned Neighbor : It->second) {
        Result.push_back(Neighbor);
      }
    }
    
    return Result;
  }


   // 获取节点的度数
  unsigned getDegree(Register VReg) {
    if (DegreesDirty) {
      updateDegrees();
    }
    
    auto It = DegreeCache.find(VReg);
    return It != DegreeCache.end() ? It->second : 0;
  }
  
  // 更新所有节点的度数
  void updateDegrees() {
    DegreeCache.clear();
    for (auto &Entry : AdjacencyList) {
      DegreeCache[Entry.first] = Entry.second.size();
    }
    DegreesDirty = false;
  }


  // 移除节点
  void removeNode(Register VReg) {
    auto It = AdjacencyList.find(VReg);
    if (It == AdjacencyList.end())
      return;
    
    // 从所有邻居的邻接表中移除该节点
    for (unsigned Neighbor : It->second) {
      auto NeighborIt = AdjacencyList.find(Neighbor);
      if (NeighborIt != AdjacencyList.end()) {
        NeighborIt->second.erase(VReg);
      }
    }
    
    // 移除该节点
    AdjacencyList.erase(It);
    DegreesDirty = true;
  }

  // 获取所有节点
  SmallVector<Register, 32> getNodes() const {
    SmallVector<Register, 32> Result;
    for (auto &Entry : AdjacencyList) {
      Result.push_back(Entry.first);
    }
    return Result;
  }
  
  // 清空图
  void clear() {
    AdjacencyList.clear();
    DegreeCache.clear();
    DegreesDirty = true;
  }

   bool empty() const {
    return AdjacencyList.empty();
  }
  void getConnectedComponents()  {
    SmallVector<SmallVector<Register, 32>, 4> Components;
    DenseSet<Register> Visited;
    unsigned int num=0;
    for (auto &Entry : AdjacencyList) {
      Register StartNode = Entry.first;
      
      // 如果节点已被访问，跳过
      if (Visited.count(StartNode))
        continue;
      
      // 使用BFS遍历连通分量
      num++;
      SmallVector<Register, 32> Component;
      std::queue<Register> Queue;
      Queue.push(StartNode);
      Visited.insert(StartNode);
      while (!Queue.empty()) {
        Register Current = Queue.front();
        Queue.pop();
        Component.push_back(Current);
        ComponentNum[Current]=num;
        auto It = AdjacencyList.find(Current);
        if (It != AdjacencyList.end()) {
          for (Register Neighbor : It->second) {
            if (!Visited.count(Neighbor)) {
              Visited.insert(Neighbor);
              Queue.push(Neighbor);
            }
          }
        }
      }
      
    }
  }

  int getComponentNum(Register VReg1)
  {
    return ComponentNum[VReg1];
  }
};





class GraphColoringRegAlloc : public MachineFunctionPass,
                              //public RegAllocBase,
                              private LiveRangeEdit::Delegate
{
private:
  // 冲突图实例
  InterferenceGraph IG;
  
  // 简化阶段使用的栈
  SmallVector<Register, 32> SimplifyStack;
  
  // 溢出候选列表
  SmallVector<Register, 16> SpillCandidates;
  
  // 当前处理的机器函数
  MachineFunction *MF= nullptr;

   // 可用的物理寄存器数量
  unsigned NumPhysRegs;

  // 虚拟寄存器到物理寄存器的映射
  DenseMap<Register, MCPhysReg> VRegToPhysReg;
  
  // 物理寄存器到虚拟寄存器的映射（反向映射）
  DenseMap<MCPhysReg, Register> PhysRegToVReg;

  std::unique_ptr<Spiller> SpillerInstance;


  VirtRegMap *VRM = nullptr;
  LiveIntervals *LIS = nullptr;
  MachineRegisterInfo *MRI = nullptr;
  std::unique_ptr<VirtRegAuxInfo> VRAI;
  MachineBlockFrequencyInfo *MBFI = nullptr;
  MachineLoopInfo *Loops = nullptr;


  DenseMap<unsigned, DenseSet<Register>> ComponentPhyReg;
  void init(VirtRegMap &vrm, LiveIntervals &lis)
  {
    MRI = &vrm.getRegInfo();
    VRM = &vrm;
    LIS = &lis;
  }
  // 构建冲突图
  void buildInterferenceGraph() {
    IG.clear();
    
    // 获取LiveIntervals分析
    //LiveIntervals *LIS = &getAnalysis<LiveIntervals>();
    //MachineRegisterInfo *MRI = &MF->getRegInfo();
    
    SmallVector<Register, 32> VRegs;
    for (unsigned I = 0, E = MRI->getNumVirtRegs(); I != E; ++I) {
      Register Reg = Register::index2VirtReg(I);
      if (MRI->reg_nodbg_empty(Reg))
        continue;
      VRegs.push_back(Reg);
    }
    
    // 检查每对虚拟寄存器之间的冲突
    for (size_t i = 0; i < VRegs.size(); ++i) {
      Register VReg1 = VRegs[i];
      LiveInterval &LI1 = LIS->getInterval(VReg1);
      
      for (size_t j = i + 1; j < VRegs.size(); ++j) {
        Register VReg2 = VRegs[j];
        LiveInterval &LI2 = LIS->getInterval(VReg2);
        
        // 如果两个活跃区间重叠，则添加冲突边
        if (LI1.overlaps(LI2)) {
          IG.addInterference(VReg1, VReg2);
        }
      }
      IG.addNode(VReg1);
    }

  }


   void simplify() {
    IG.updateDegrees();
    SimplifyStack.clear();
    
    // 创建一个工作列表，包含所有节点
    SmallVector<Register, 32> Worklist = IG.getNodes();
    bool Changed;
    while(!IG.empty())
    {
        do {
        Changed = false;
        
        // 查找度数小于K的节点
        for (auto It = Worklist.begin(); It != Worklist.end(); ) {
            Register VReg = *It;
            unsigned Degree = IG.getDegree(VReg);
            
            if (Degree < NumPhysRegs) 
            {
                // 将节点压入栈并从图中移除
                SimplifyStack.push_back(VReg);
                IG.removeNode(VReg);
                It = Worklist.erase(It);
                Changed = true;
            } else 
            {
                ++It;
            }
        }
        } while (Changed);

        if(IG.empty()) break;
        auto It = Worklist.begin();
        Register VReg = *It;
        SimplifyStack.push_back(VReg);
        IG.removeNode(VReg);
        It = Worklist.erase(It);
    }
   }


   // 选择阶段：为节点分配寄存器
  void select() {
    const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
    
    // 从栈顶开始处理节点（逆序）
    while (!SimplifyStack.empty()) {
      Register VReg = SimplifyStack.pop_back_val();
      
      // 获取可用的物理寄存器
      BitVector AvailableRegs = getAvailableRegs(VReg);
      
      // 排除已被邻居使用的寄存器
      for (Register Neighbor : IG.getInterferingVRegs(VReg)) {
        auto It = VRegToPhysReg.find(Neighbor);
        if (It != VRegToPhysReg.end()) {
          AvailableRegs.reset(It->second);
        }
      }
      
      // 如果有可用的寄存器，则分配
      if (AvailableRegs.any()) {
        int PhysReg = AvailableRegs.find_first();
        VRegToPhysReg[VReg] = PhysReg;
        PhysRegToVReg[PhysReg] = VReg;
      } else {
        // 没有可用寄存器，添加到溢出列表
        SpillCandidates.push_back(VReg);
      }
      
      // 将节点添加回图中
      // 注意：在实际实现中，需要重新构建冲突关系
      // 这里简化处理，只添加节点而不添加边
    //   for (Register Neighbor : IG.getInterferingVRegs(VReg)) {
    //     IG.addInterference(VReg, Neighbor);
    //   }
    }    
  }

   // 获取可用于虚拟寄存器的物理寄存器
  BitVector getAvailableRegs(Register VReg) 
  {
    MachineRegisterInfo *MRI = &MF->getRegInfo();
    const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
    const TargetRegisterClass *RC = MRI->getRegClass(VReg);
    
    //const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
    NumPhysRegs=TRI->getNumRegs();

    BitVector AvailableRegs(NumPhysRegs);
    AvailableRegs.set();
    
    BitVector ReservedRegs = TRI->getReservedRegs(*MF);
    // 排除保留寄存器
    for (auto Reg : ReservedRegs.set_bits()) {
         AvailableRegs.reset(Reg);
     }
    
    // 只保留目标寄存器类中的寄存器
    BitVector RegClassRegs(NumPhysRegs);
    for (MCPhysReg Reg : *RC) {
      RegClassRegs.set(Reg);
    }
    AvailableRegs &= RegClassRegs;
    
    return AvailableRegs;
  }


  // 处理溢出
  void handleSpills() 
  {
    //LiveIntervals *LIS = &getAnalysis<LiveIntervals>();
    //MachineRegisterInfo *MRI = &MF->getRegInfo();
    //VirtRegMap *VRM = &getAnalysis<VirtRegMap>();
    const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();
    
    // 获取栈帧信息
    MachineFrameInfo &MFI = MF->getFrameInfo();
    
    // 对每个溢出候选进行处理
    for (Register VReg : SpillCandidates) {
        //LLVM_DEBUG(dbgs() << "Processing spill candidate: " << VReg << "\n");
        
        // 获取寄存器类别
        const TargetRegisterClass *RC = MRI->getRegClass(VReg);
        
        // 分配堆栈槽
        int FrameIndex = MFI.CreateSpillStackObject(
            TRI->getSpillSize(*RC), TRI->getSpillAlign(*RC));
        
        // 记录溢出位置
        VRM->assignVirt2StackSlot(VReg, FrameIndex);
        
        // 创建新寄存器列表（用于接收溢出过程中创建的新寄存器）
        SmallVector<Register, 4> NewRegs;
        
        // 创建LiveRangeEdit对象
        LiveRangeEdit LRE(&LIS->getInterval(VReg), NewRegs, *MF, *LIS, VRM, this);
        
        // 创建并配置溢出器
        //InlineSpiller Spiller(*MF, *LIS, *VRM);
        
        // 执行溢出操作
        spiller().spill(LRE);
        
       
        // LLVM_DEBUG(dbgs() << "Successfully spilled VReg: " << VReg 
        //                   << " to stack slot: " << FrameIndex << "\n");
    }
    
    // 清空溢出候选列表，因为已经处理完毕
    SpillCandidates.clear();
  }

  void RAColour()
  {
    for(;;)
    {
        buildInterferenceGraph();
        simplify();
        select();
        if(SpillCandidates.empty()) break;
        handleSpills();
    }
    return;
  }
public:
    static char ID; // Pass identification
    GraphColoringRegAlloc():
    MachineFunctionPass(ID) {    
      
    }
    Spiller &spiller()  { return *SpillerInstance; }
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesCFG();
      AU.addRequired<MachineBlockFrequencyInfo>();
      AU.addPreserved<MachineBlockFrequencyInfo>();
      AU.addRequired<MachineLoopInfo>();
      AU.addPreserved<MachineLoopInfo>();
      AU.addRequired<LiveIntervals>();
      AU.addPreserved<LiveIntervals>();

      AU.addRequired<VirtRegMap>();
      AU.addPreserved<VirtRegMap>();

      AU.addRequired<LiveStacks>();
      AU.addPreserved<LiveStacks>();

      AU.addRequired<MachineDominatorTree>();
      AU.addPreserved<MachineDominatorTree>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }
    bool runOnMachineFunction(MachineFunction &mf) override
    {
        MF = &mf;

        init(getAnalysis<VirtRegMap>(),getAnalysis<LiveIntervals>());

        Loops = &getAnalysis<MachineLoopInfo>();
        MBFI = &getAnalysis<MachineBlockFrequencyInfo>();
        VRAI = std::make_unique<VirtRegAuxInfo>(*MF, *LIS, *VRM, *Loops, *MBFI);
        SpillerInstance.reset(createInlineSpiller(*this, *MF, *VRM, *VRAI));

        RAColour();
        
        for (auto &Entry : VRegToPhysReg) {
            Register VReg = Entry.first;
            MCPhysReg PhysReg = Entry.second;
            
            // 替换虚拟寄存器为物理寄存器
            // for (MachineOperand &MO : MRI->reg_operands(VReg)) {
            //     MO.setReg(PhysReg);
            // }
             VRM->assignVirt2Phys(VReg, PhysReg);
        }
    }
};
char GraphColoringRegAlloc::ID = 0;


}


INITIALIZE_PASS_BEGIN(GraphColoringRegAlloc, "regallocgraph", "Fast Register Allocator", false,
                false)
INITIALIZE_PASS_DEPENDENCY(LiveStacks)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_DEPENDENCY(VirtRegMap)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_END(GraphColoringRegAlloc, "regallocgraph", "Fast Register Allocator", false,
                false)
FunctionPass* llvm::createGraphColourRegisterAllocator() {
  return new GraphColoringRegAlloc();
}
static RegisterRegAlloc graphcolourRegAlloc("graphcolour", "graph colour register allocator",
                                       createGraphColourRegisterAllocator);
