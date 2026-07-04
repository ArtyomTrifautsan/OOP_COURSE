#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <random>
#include <chrono>


std::queue<int> buffer;
std::mutex mtx;
std::condition_variable cv;
bool finished = false;

// Поток-писатель
void producer() {
    // for (int i = 1; i <= 5; ++i) {
    //     {
    //         std::lock_guard<std::mutex> lock(mtx);
    //         buffer.push(i);
    //         std::cout << "Produced: " << i << std::endl;
    //     }
    //     cv.notify_one(); // Будим читателя
    // }

    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> num_dist(1, 100);
    std::uniform_int_distribution<> sleep_dist(100, 3000);

    while(true)
    {
        int value = num_dist(gen);
        int sleep_time = sleep_dist(gen);

        {
            std::lock_guard<std::mutex> lock(mtx);
            buffer.push(value);
            // std::cout << "Produced: " << value << std::endl;
            std::cout << "[Producer] Сгенерировал: " << value 
                      << " (ждал " << sleep_time << "ms)" << std::endl;
        }

        cv.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_one();    // Нужно ли? Проснется ли consumer сам?
    std::cout << "[Consumer] Завершил работу." << std::endl;
}

// Поток-читатель
void consumer() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sleep_dist(500, 5000);

    while (true) {
        {
            std::unique_lock<std::mutex> lock(mtx);

            // Ждем, пока буфер не станет непустым или пока работа не завершится
            cv.wait(lock, []{ return !buffer.empty() || finished; });
            
            while (!buffer.empty()) {
                int val = buffer.front();
                buffer.pop();
                std::cout << "[Consumer] Обработал: " << val << std::endl;
            }

            if (finished) break;
        }

        int processing_time = sleep_dist(gen);
        std::this_thread::sleep_for(std::chrono::milliseconds(processing_time));
    }

    std::cout << "[Consumer] Завершил работу." << std::endl;
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    
    t1.join();
    t2.join();
    return 0;
}