#include <gtest/gtest.h>

#include "imageFIFO.hpp"

using namespace ThreadCommunication;

const size_t BLOCK_SIZE = 1024 * 1024;
const size_t MAX_BLOCKS = 5;

void producer_thread(ImageFIFO& fifo, int totalFrames) {
    for (int i = 0; i < totalFrames; ++i) {
        void* ptr = fifo.getFree();
        
        uint8_t* data = static_cast<uint8_t*>(ptr);
        std::memset(data, i % 256, BLOCK_SIZE);
        
        std::cout << "[Producer] Считал кадр " << i << " в буфер " << ptr << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        

        fifo.addReady(ptr);
    }
}

void consumer_thread(ImageFIFO& fifo, int totalFrames) {
    for (int i = 0; i < totalFrames; ++i) {
        void* ptr = fifo.getReady();
        
        uint8_t* data = static_cast<uint8_t*>(ptr);
        
        bool ok = true;
        for(size_t j = 0; j < BLOCK_SIZE; ++j) {
            if (data[j] != i % 256) { ok = false; break; }
        }
        
        std::cout << "[Consumer] Записал кадр " << i 
                  << (ok ? " (OK)" : " (CORRUPTED!) ") << " из " << ptr << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        
        fifo.addFree(ptr);
    }
}

TEST(CommonUsage, Simple)
{
    ImageFIFO fifo(BLOCK_SIZE, MAX_BLOCKS);
    int framesToProcess = 10;

    std::cout << "--- Запуск многопоточного теста ---" << std::endl;

    std::thread t1(producer_thread, std::ref(fifo), framesToProcess);
    std::thread t2(consumer_thread, std::ref(fifo), framesToProcess);

    t1.join();
    t2.join();

    std::cout << "--- Тест успешно завершен ---" << std::endl;
}
