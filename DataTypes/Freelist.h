#include<cstdint>
#include<map>
#include<set>
#include"./Memory.h"

struct FirstFitComparator
{
    bool operator()(const MemoryChunk* x, const MemoryChunk* y) const
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

class FirstFitPagingFreeList: public FreeList {
private:
    std::queue<MemoryFrame*> frames;
    uint64_t frameSize;

public:
    ~FirstFitPagingFreeList() {}

    FirstFitPagingFreeList(uint64_t frameSize) {
        this->frameSize = frameSize;
    }

    MemoryFrame* pop(uint64_t size) override {
        if(frames.size() > 0) {
            MemoryFrame* allocated = frames.front();
            frames.pop();
            return allocated;
        }

        return nullptr;
    }

    void remove(AllocatedMemory* chunk) override {
        MemoryFrame* allocated = frames.front();
        frames.pop();
    }

    void push(AllocatedMemory* chunk) override {
        frames.push((MemoryFrame*) chunk);
    }

    uint64_t getAvailableMemory() {
        return frames.size() * frameSize;
    }
};