/*

Napisz równoległą wersję algorytmu std::accumulate, który będzie współbieżnie sumował fragmenty kontenera. Wyniki powinny zostać również zapisane w kontenerze.

- on empty returns init
- calculate number of threads - hardware_threads = hardware_concurrency() != 0 ? hardware_concurrency() : 2
- divide on blocks according to the number of threads
- create vector of results
- run every thread in parallel
- put results in vector element passed by reference (threads cannot return a value)
- join threads
- accumulate results from the result vector
- test on 1M elements and compare with standard std::accumulate
- compare with std::par execution policy in std::accumulate from C++17 ;)
- templatize algorithm on Iterator (depends on container) and type T (usually int, double)

*/

#include <iostream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <thread>
#include <mutex>
#include <iomanip>
#include <numeric>
//#include <execution>


std::mutex mtx;

template <typename InputIterator, typename T>
T accumulate_parallel(InputIterator first, InputIterator last, T init)
{

    // calculate size of container
    int cont_size = last-first;

    // calculate number of threads
    const int hardware_threads = std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 2;

    // calculate block size
    int block_size = cont_size/hardware_threads;

    // divide on blocks according to the number of threads
    std::vector<std::thread> vec_thread;
    vec_thread.reserve(hardware_threads);

    // put results in vector element passed by reference (threads cannot return a value)
    std::vector<int> vec_result(hardware_threads,0);


    // threads
    for(int i = 0; i < hardware_threads; i++)
    {
        // first element of accumulation in each block
        auto temp_first = first + (block_size*i);

        // last element of accumulation in each block
        auto temp_last = first + block_size*(i+1);


        if( i == hardware_threads-1 )
        {
            temp_last = last;  // if in last block;
        }


        vec_thread.emplace_back(
            [=](int& res){
                        res = std::accumulate(temp_first,temp_last,0);
//                        mtx.lock();
//                        std::cout << "Block :" << i << " res :" << res << std::endl;
//                        mtx.unlock();
                        },
            std::ref(vec_result.at(i)));
    }

    // join thread
    for(auto && element : vec_thread)
        element.join();

    for(auto element : vec_result)
    {
        init += element;
    }

    return init;
}

int main()
{
    using namespace std::literals::chrono_literals;

    long long int res;

    std::vector<int> vec(1000000,2);

    // test of accumulate_parallel
    auto start_parallel = std::chrono::high_resolution_clock::now();    // start
    res = accumulate_parallel(vec.begin(), vec.end(), 0);
    auto end_parallel = std::chrono::high_resolution_clock::now();      // end

    std::chrono::duration<float> duration_parallel = end_parallel - start_parallel;
    std::cout << std::setprecision(10) << duration_parallel.count() << std::endl;

/*
    // test of accumulate with std::execution::par
    auto start = std::chrono::high_resolution_clock::now();
    res = std::accumulate(std::execution::par, vec.begin(), vec.end(), 0);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<float> duration = end - start;
    std::cout << std::setprecision(10) << duration.count() << std::endl;
*/

    std::cout << res << std::endl;

    return 0;
}