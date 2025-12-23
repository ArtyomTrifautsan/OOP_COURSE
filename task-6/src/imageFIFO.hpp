#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <stdexcept>


namespace ThreadCommunication {

    class ImageFIFO
    {
    private:
        size_t blockSize;
        size_t maxBlocks;
        uint8_t* buffer;
        
        std::queue<void*> freeBlocks;
        std::queue<void*> readyBlocks;
        
        std::mutex mtx;
        std::condition_variable cvFree;
        std::condition_variable cvReady;

        const uint32_t ENDFLAG = 0xDEADBEEF;
        size_t actualBlockSize;

    public:
        ImageFIFO(size_t blockSize, size_t maxBlocks) 
            : blockSize(blockSize), maxBlocks(maxBlocks)
        {
            actualBlockSize = blockSize + sizeof(ENDFLAG);
            buffer = new uint8_t[actualBlockSize * maxBlocks];

            for (size_t i = 0; i < maxBlocks; ++i)
            {
                uint8_t* ptr = buffer + (i * actualBlockSize);
                memcpy(ptr + blockSize, &ENDFLAG, sizeof(ENDFLAG));
                freeBlocks.push(ptr);
            }
        }

        ~ImageFIFO()
        {
            delete[] buffer;
        }

        void checkIntegrity(void* data)
        {
            uint32_t currentCanary;
            memcpy(&currentCanary, (uint8_t*)data + blockSize, sizeof(ENDFLAG));
            if (currentCanary != ENDFLAG) {
                throw std::runtime_error("Buffer overflow detected! Memory corruption.");
            }
        }

        void* getFree()
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvFree.wait(lock, [this] { return !freeBlocks.empty(); });
            
            void* ptr = freeBlocks.front();
            freeBlocks.pop();
            return ptr;
        }

        void addReady(void* data)
        {
            checkIntegrity(data);
            {
                std::lock_guard<std::mutex> lock(mtx);
                readyBlocks.push(data);
            }
            cvReady.notify_one();
        }

        void* getReady()
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvReady.wait(lock, [this] { return !readyBlocks.empty(); });
            
            void* ptr = readyBlocks.front();
            readyBlocks.pop();
            return ptr;
        }

        void addFree(void* data)
        {
            checkIntegrity(data);
            {
                std::lock_guard<std::mutex> lock(mtx);
                freeBlocks.push(data);
            }
            cvFree.notify_one();
        }
    };

}