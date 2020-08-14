#pragma once

#include <chrono>
#include <iostream>

auto tic = []() { return std::chrono::system_clock::now(); };

auto toc = [](std::chrono::system_clock::time_point start) {
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "cost"
              << double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den
              << "s" << std::endl;
};

std::chrono::system_clock::time_point t1 = std::chrono::system_clock::now();
std::chrono::system_clock::time_point t2 = std::chrono::system_clock::now();
std::chrono::system_clock::time_point t3 = std::chrono::system_clock::now();
