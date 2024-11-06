#include<cstdint>
#include<memory>
#include<iostream>
#include<mutex>
#include<condition_variable>
#include"../DataTypes/Freelist.h"

enum AllocatorType {
    FIRSTFIT = 1,
    BESTFIT = 0
};

class MemoryInterface {
    private:
        uint64_t startAddress;
        uint64_t endAddress;
        uint64_t memorySize;
        MemoryChunk* memoryStart;
        FreeList* freeList;
        std::mutex mtx;
        std::condition_variable cv;

    public:
        ~MemoryInterface() {
            delete freeList;
            MemoryChunk *next, *temp;
            temp = memoryStart;
            do {
                next = temp->next;
                delete temp;
                temp = next;
            } while(temp != nullptr);
        }

        MemoryInterface(uint64_t memorySize, AllocatorType allocator)  {
            this->memorySize = memorySize;
            this->startAddress = 0;
            this->endAddress = memorySize;
            this->memoryStart = new MemoryChunk(memorySize, 0, nullptr, nullptr);
            
            if(allocator == FIRSTFIT) {
                this->freeList = new FirstFitFreeList();
            } else {
                this->freeList = new BestFitFreeList();
            }

            this->freeList->push(memoryStart);
        }

        MemoryChunk* alloc(uint64_t size) {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryChunk* allocated = freeList->pop(size);

            if(allocated == nullptr) {
                return allocated;
            }

            if(allocated->startAddress == 0) {
                this->memoryStart = allocated;
            }

            lock.unlock();
            return allocated;
        }

        void free(MemoryChunk* chunk) {
            std::unique_lock<std::mutex> lock(mtx);
            MemoryChunk* previousChunk = chunk->prev;
            MemoryChunk* nextChunk = chunk->next;

            chunk->isInUse = false;

            if(previousChunk != nullptr) {
                if(!previousChunk->isInUse) {
                    //Remove the previous chunk from the freelist
                    freeList->remove(previousChunk);

                    //Merge previousChunk into the chunk being freed
                    chunk->prev = previousChunk->prev;
                    chunk->size += previousChunk->size;
                    chunk->startAddress = previousChunk->startAddress;

                    //Set previousChunk's previous to point to the new merged chunk
                    if(previousChunk->prev != nullptr) {
                        previousChunk->prev->next = chunk;
                    }

                    //Reset head of chunk linked list
                    if(chunk->startAddress == 0) {
                        this->memoryStart = chunk;
                    }

                    //Free the memory of the previous chunk
                    delete previousChunk;
                }
            }

            if(nextChunk != nullptr) {
                if(!nextChunk->isInUse) {
                    //Remove the next chunk from the freelist
                    freeList->remove(nextChunk);

                    //Merge nextChunk into the chunk being freed
                    chunk->next = nextChunk->next;
                    chunk->size += nextChunk->size;
                    chunk->endAddress = nextChunk->endAddress;

                    //Set nextChunk's next to point to the new merged chunk
                    if(nextChunk->next != nullptr) {
                        nextChunk->next->prev = chunk;
                    }

                    //Free the memory of the next chunk
                    delete nextChunk;
                }
            }

            //Add the freed and coalesced chunk into the freelist
            freeList->push(chunk);
            lock.unlock();
        }

        void printFreeList() {
            this->freeList->print();
        }
};