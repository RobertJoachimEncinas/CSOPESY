#include<cstdint>
#include<map>
#include<set>
#include"./Memory.h"

struct FirstFitComparator
{
    bool operator()(const AllocatedMemory* x, const AllocatedMemory* y) const
    {
        return x->startAddress < y->startAddress;
    }
};

class FreeList {
public:
    virtual AllocatedMemory* pop(uint64_t size) { return nullptr; };
    virtual void remove(AllocatedMemory* chunk) {};
    virtual void push(AllocatedMemory* chunk) {};
    virtual void print() {};
};

class FirstFitFreeList: public FreeList {
private:
    std::set<MemoryChunk*, FirstFitComparator> chunks;
public:
    MemoryChunk* pop(uint64_t size) override {
        MemoryChunk* allocated = nullptr;

        for(const auto& chunk: chunks) {
            if(chunk->size == size) {
                allocated = chunk;
                allocated->isInUse = true;
                chunks.erase(chunk); //Remove the chunk from the freelist
                break;
            } else if(chunk->size > size) {
                //The original chunk has been resized by getPartition so no need to remove
                allocated = chunk->getPartition(size); 
                break;
            }
        }

        return allocated;
    }

    void remove(AllocatedMemory* chunk) override {
        chunks.erase((MemoryChunk*) chunk);
    }

    void push(AllocatedMemory* chunk) override {
        chunks.insert((MemoryChunk*) chunk);
    }
};