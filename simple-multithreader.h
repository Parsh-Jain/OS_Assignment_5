#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <pthread.h>
#include <vector>
#include <functional>
#include <iostream>
#include <chrono>
#include <string>
#include <cerrno> 
#include <algorithm>


// Struct for arguments for 1D parallel_for
struct ThreadArgs1D {
    int low, high;
    std::function<void(int)> lambda;
};

// Struct for arguments for 2D parallel_for
struct ThreadArgs2D {
    int low1, high1, low2, high2;
    std::function<void(int, int)> lambda;
};

// function for 1D parallel_for
void* thread_1D(void* args) {
    auto* threadArgs = static_cast<ThreadArgs1D*>(args);
    for (int i = threadArgs->low; i < threadArgs->high; ++i) {
        threadArgs->lambda(i);
    }
    return nullptr;
}

// function for 2D parallel_for
void* thread_2D(void* args) {
    auto* threadArgs = static_cast<ThreadArgs2D*>(args);
    for (int i = threadArgs->low1; i < threadArgs->high1; ++i) {
        for (int j = threadArgs->low2; j < threadArgs->high2; ++j) {
            threadArgs->lambda(i, j);
        }
    }
    return nullptr;
}

// 1D parallel_for 
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
    auto startTime = std::chrono::high_resolution_clock::now();

    if (numThreads <= 0 || low >= high) {
        std::cerr << "Error: Invalid input parameters for parallel_for (1D).\n";
        return;
    }

    int range = high - low;
    int chunkSize = (range + numThreads - 1) / numThreads; //Improved workload balance

    std::vector<pthread_t> threads(numThreads);
    std::vector<ThreadArgs1D> threadArgs(numThreads);

    for (int t = 0; t < numThreads; ++t) {
        threadArgs[t] = {low + t * chunkSize, std::min(low + (t + 1) * chunkSize, high), lambda};
        int ret = pthread_create(&threads[t], nullptr, thread_1D, &threadArgs[t]);
        if (ret != 0) {
            std::cerr << "Error creating thread " << t << ": " << strerror(ret) << "\n";
            //Clean up already created threads before exiting
            for (int i = 0; i < t; ++i) {
                pthread_join(threads[i], nullptr);
            }
            return;
        }
    }

    for (int t = 0; t < numThreads; ++t) {
        int ret = pthread_join(threads[t], nullptr);
        if (ret != 0) {
            std::cerr << "Error joining thread " << t << ": " << strerror(ret) << "\n";
            return;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> execTime = endTime - startTime;
    std::cout << "Execution Time (1D): " << execTime.count() << " seconds\n";
}

// 2D parallel_for
void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads) {
    auto startTime = std::chrono::high_resolution_clock::now();

    if (numThreads <= 0 || low1 >= high1 || low2 >= high2) {
        std::cerr << "Error: Invalid input parameters for parallel_for (2D).\n";
        return;
    }

    int range1 = high1 - low1;
    int range2 = high2 - low2;
    int chunkSize1 = (range1 + numThreads -1) / numThreads;
    int chunkSize2 = range2; //Simplified for demonstration - could be further divided.

    std::vector<pthread_t> threads(numThreads);
    std::vector<ThreadArgs2D> threadArgs(numThreads);

    for (int t = 0; t < numThreads; ++t) {
        threadArgs[t] = {low1 + t * chunkSize1, std::min(low1 + (t + 1) * chunkSize1, high1), low2, high2, lambda};
        int ret = pthread_create(&threads[t], nullptr, thread_2D, &threadArgs[t]);
        if (ret != 0) {
            std::cerr << "Error creating thread " << t << ": " << strerror(ret) << "\n";
             //Clean up already created threads before exiting
            for (int i = 0; i < t; ++i) {
                pthread_join(threads[i], nullptr);
            }
            return;
        }
    }

    for (int t = 0; t < numThreads; ++t) {
        int ret = pthread_join(threads[t], nullptr);
        if (ret != 0) {
            std::cerr << "Error joining thread " << t << ": " << strerror(ret) << "\n";
            return;
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> execTime = endTime - startTime;
    std::cout << "Execution Time (2D): " << execTime.count() << " seconds\n";
}

#endif // SIMPLE_MULTITHREADER_H