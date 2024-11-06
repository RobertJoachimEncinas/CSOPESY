#include<cstdint>
#include<map>
#include<set>
#include"./MemoryChunk.h"

struct FirstFitComparator
{
    bool operator()(const MemoryChunk* x, const MemoryChunk* y) const
    {
        return x->startAddress < y->startAddress;
    }
};

struct BestFitComparator
{
    bool operator()(const MemoryChunk* x, const MemoryChunk* y) const
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
    virtual MemoryChunk* pop(uint64_t size) { return nullptr; };
    virtual void remove(MemoryChunk* chunk) {};
    virtual void push(MemoryChunk* chunk) {};
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

    void remove(MemoryChunk* chunk) override {
        chunks.erase(chunk);
    }

    void push(MemoryChunk* chunk) override {
        chunks.insert(chunk);
    }

    void print() {
        std::cout << "FREE LIST\n";
        for(const auto& chunk: chunks) {
            std::cout << chunk->toString() << "\n";
        }
    }
};

class BestFitFreeList: public FreeList {
private:
    std::set<MemoryChunk*, BestFitComparator> chunks;
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

    void remove(MemoryChunk* chunk) override {
        chunks.erase(chunk);
    }

    void push(MemoryChunk* chunk) override {
        chunks.insert(chunk);
    }

    void print() {
        std::cout << "FREE LIST\n";
        for(const auto& chunk: chunks) {
            std::cout << chunk->toString() << "\n";
        }
    }
};