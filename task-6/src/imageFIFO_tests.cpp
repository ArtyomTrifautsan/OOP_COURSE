#include <chrono>

#include <gtest/gtest.h>

#include "imageFIFO.hpp"

using namespace ThreadCommunication;

const size_t BLOCK_SIZE = 1024 * 1024;
const size_t MAX_BLOCKS = 5;

TEST(ImageFIFOTest, BasicProductionConsumption)
{
    ImageFIFO fifo(1024, 2);
    void* block = fifo.getFree();
    std::memset(block, 0xAA, 1024);
    fifo.addReady(block);
    
    void* result = fifo.getReady();
    EXPECT_EQ(result, block);
    
    uint8_t* data = static_cast<uint8_t*>(result);
    EXPECT_EQ(data[0], 0xAA);
    
    fifo.addFree(result);
}

TEST(ImageFIFOTest, MultiProducerMultiConsumer) {
    const size_t numProducers = 3;
    const size_t numConsumers = 3;
    const int opsPerThread = 100;
    
    ImageFIFO fifo(1024, 5);
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    auto producer = [&]() {
        for(int i = 0; i < opsPerThread; ++i) {
            void* ptr = fifo.getFree();
            fifo.addReady(ptr);
            produced++;
        }
    };

    auto consumer = [&]() {
        for(int i = 0; i < opsPerThread; ++i) {
            void* ptr = fifo.getReady();
            fifo.addFree(ptr);
            consumed++;
        }
    };

    std::vector<std::thread> workers;
    for(int i = 0; i < numProducers; ++i) workers.emplace_back(producer);
    for(int i = 0; i < numConsumers; ++i) workers.emplace_back(consumer);

    for(auto& t : workers) t.join();

    EXPECT_EQ(produced, numProducers * opsPerThread);
    EXPECT_EQ(consumed, numConsumers * opsPerThread);
}

TEST(ImageFIFOTest, IntegrityCheck) {
    ImageFIFO fifo(1024, 2);
    void* block = fifo.getFree();
    
    // Портим данные за пределами блока (там, где канарейка 0xDEADBEEF)
    uint8_t* raw = static_cast<uint8_t*>(block);
    raw[1024] = 0x00; 
    
    EXPECT_THROW(fifo.addReady(block), std::runtime_error);
}

TEST(ImageFIFOTest, OverbookedThreads) {
    const size_t numBlocks = 2;
    const size_t numProducers = 10; // Больше, чем блоков
    const int opsPerThread = 10;
    
    ImageFIFO fifo(1024, numBlocks);
    std::atomic<int> completed{0};

    auto producer = [&]() {
        for(int i = 0; i < opsPerThread; ++i) {
            void* ptr = fifo.getFree();
            fifo.addReady(ptr);
        }
        completed++;
    };

    std::vector<std::thread> producers;
    for(int i = 0; i < numProducers; ++i) producers.emplace_back(producer);

    std::thread consumer([&]() {
        for(int i = 0; i < numProducers * opsPerThread; ++i) {
            void* ptr = fifo.getReady();
            fifo.addFree(ptr);
        }
    });

    for(auto& t : producers) t.join();
    consumer.join();

    EXPECT_EQ(completed, numProducers);
}

TEST(ImageFIFOTest, ProducerWaitingForConsumer) {
    const size_t numBlocks = 2;
    std::atomic<int> completed{0};
    ImageFIFO fifo(1024, numBlocks);
    
    std::thread producer([&]() {
        for(int i = 0; i < 5; ++i) {
            void* ptr = fifo.getFree(); 
            fifo.addReady(ptr);
        }
        ++completed;
    });

    std::thread consumer([&]() {
        for(int i = 0; i < 5; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            void* ptr = fifo.getReady();
            fifo.addFree(ptr);
        }
        ++completed;
    });

    producer.join();
    consumer.join();

    // здесь 2 - это один писатель и один читатель
    EXPECT_EQ(completed, 2);
}

TEST(ImageFIFOTest, ConsumerWaitingForProducer) {
    std::atomic<int> completed{0};
    ImageFIFO fifo(1024, 2);
    std::atomic<bool> producerStarted{false};

    std::thread consumer([&]() {
        void* ptr = fifo.getReady();
        fifo.addFree(ptr);
        ++completed;
    });

    std::thread producer([&]() {
        void* ptr = fifo.getFree();
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        fifo.addReady(ptr);
        ++completed;
    });

    consumer.join();
    producer.join();

    // здесь 2 - это один писатель и один читатель
    EXPECT_EQ(completed, 2);
}