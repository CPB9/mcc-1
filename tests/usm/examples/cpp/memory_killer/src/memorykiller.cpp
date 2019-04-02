#include <iostream>
#include <chrono>
#include <thread>
#include <array>

int main(int argc, char *argv[])
{
    std::cout << "Starting to steal memory..." << std::endl;

    while(true)
    {
        const std::size_t n = 1024*1024;
        std::array<char, n>* a = new std::array<char, n>();
        a->at(n-1) = 0xFF;
        std::chrono::milliseconds tickTimeout(10);
        std::this_thread::sleep_for(tickTimeout);
    }

    return 0;
}
