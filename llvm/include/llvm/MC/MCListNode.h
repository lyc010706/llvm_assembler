template <class T>
class MCListNode
{
public:
    T instInfo;
    MCListNode* nextNode;
    MCListNode* prevNode;
    MCListNode(T&d):instInfo(d),prevNode(nullptr),nextNode(nullptr){}
};