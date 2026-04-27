#include "llvm/MC/MCListNode.h"
template <class T>
class MCList
{
public:
    MCListNode<T>* firstNode;
    MCListNode<T>* lastNode;

    MCList():firstNode(nullptr),lastNode(nullptr){}

    void insertBack(T&data)
    {
        MCListNode<T>* newNode=new MCListNode<T>(data);
        if(!lastNode)
        {
            firstNode=lastNode=newNode;
        }
        else
        {
            lastNode->nextNode=newNode;
            newNode->prevNode=lastNode;
            lastNode=newNode;
        }

    }

};