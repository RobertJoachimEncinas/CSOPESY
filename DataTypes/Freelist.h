#include<cstdint>
#include<map>
#include<set>
#include"./MemoryChunk.h"

struct FirstFitComparator
{
    bool operator()(const MemoryFrame* x, const MemoryFrame* y) const
    {
        return x->startAddress < y->startAddress;
    }
};

struct BestFitComparator
{
    bool operator()(const MemoryFrame* x, const MemoryFrame* y) const
    {
        if(x->size == y->size) {
            return x->startAddress < y->startAddress;
        } else {
            return x->size < y->size;
        }
    }
};

class FreeList {
public:
    virtual MemoryFrame* pop(uint64_t size) { return nullptr; };
    virtual void remove(MemoryFrame* chunk) {};
    virtual void push(MemoryFrame* chunk) {};
    virtual void print() {};
};

class FirstFitFreeList: public FreeList {
private:
    std::set<MemoryFrame*, FirstFitComparator> chunks;
public:
    MemoryFrame* pop(uint64_t size) override {
        MemoryFrame* allocated = nullptr;

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

    void remove(MemoryFrame* chunk) override {
        chunks.erase(chunk);
    }

    void push(MemoryFrame* chunk) override {
        chunks.insert(chunk);
    }

    void print() {
        std::cout << "FREE LIST\n";
        for(const auto& chunk: chunks) {
            std::cout << chunk->toString() << "\n";
        }
    }
};