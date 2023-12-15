#include "threaded.hpp"

#include <functional>
#include <vector>
#include <thread>

#include "LegacyEditor/LCE/FileListing/fileListing.hpp"


template<int threadCount, typename Function, typename... Args>
int run_parallel(Function func, Args... args) {
    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(std::bind(func, i, args...));
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    return 0;
}

template int run_parallel<4>(std::function<void(int)> func);

template int run_parallel<4>(
        void (*)(int, editor::FileListing&),
        std::reference_wrapper<editor::FileListing>
);