//  Written by Samuel Payne, uses C++20 features

#include <iostream>
#include <fstream>
#include <filesystem>
#include <format>
#include <string>
#include <mutex>
//  The threadpool library I forked and altered, located on my github
#include "Threadpool.h"

static unsigned long genLines(unsigned long start, const unsigned long last, std::string& contents)
{
    //  flipping a boolean would be quicker for older computers but using C++20 kinda defeats that 
    //bool odd = false;

    while (start <= last)
    {
        /*if (odd)
        {
            contents += std::format("\telif num == {}:\n\t\tprint(\"odd\")\n", start);
            odd = false;
        }
        else
        {
            contents += std::format("\telif num == {}:\n\t\tprint(\"even\")\n", start);
            odd = true;
        }*/

        contents += start % 2 
            ? std::format("\telif num == {}:\n\t\tprint(\"odd\")\n", start) 
            : std::format("\telif num == {}:\n\t\tprint(\"even\")\n", start);

        start++;
    }

    //  return memory size in bits
    return contents.size() * sizeof(contents[0]);
}

int main(int argc, char* argv[])
{
    using namespace std;

    auto startTime = chrono::high_resolution_clock::now();

    unique_ptr<Threadpool> pool = make_unique<Threadpool>(6);

    //  each billion entries uses 37 gigabytes of disk space so be careful (5 billion = 185 gigabytes)
    unsigned long entries = pow(10,8);  // 100 million

    if (argc > 2)
    {
        string temp = string(argv[1]);

        for (auto& ch : temp)
        {
            if (!std::isdigit(ch)) return -1;
        }

        entries = std::stoul(temp);
    }

    const unsigned long bucketSize = 1000UL;
    const unsigned long numOfBuckets = ceil(entries / static_cast<long double>(bucketSize));
    vector<string> buckets(numOfBuckets, "");
    vector<unsigned long> bucketMemSize(numOfBuckets, 0UL);
    unsigned long long currMemSize = 0ULL;
    mutex mutx;
    const long double slowTasksDown = powl(2.0, 30.0);

    ofstream file;

    filesystem::path filepath = "H:\\modulusMeme\\meme.py";

    if (filesystem::exists(filepath))
    {
        cout << "File already exists, delete before making a new one." << endl;
        return 1;
    }
    
    file.open(filepath, ios::app);

    file << "def isEvenOrOdd(num: int):\n\tif num == 1:\n\t\tprint(\"odd\")\n";

    pool->enqueue([&buckets, &currMemSize, &bucketSize, &bucketMemSize, &entries]
        {
            currMemSize += genLines(2, min(entries, bucketSize), buckets[0]);
            bucketMemSize[0] = currMemSize;
            //cout << format("Current memory size: {}", currMemSize) << endl;
        });


    for (unsigned long index = 1; index < numOfBuckets; index++)
    {
        pool->enqueue([&buckets, index, &mutx, &currMemSize, &bucketSize, &bucketMemSize, &slowTasksDown, &entries]
            {
                bucketMemSize[index] = genLines(index * bucketSize + 1UL, min(entries, (index + 1UL) * bucketSize), buckets[index]);

                while (!mutx.try_lock())
                {
                    this_thread::sleep_for(chrono::nanoseconds(100LL));
                }

                currMemSize += bucketMemSize[index];

                mutx.unlock();

                /*  was an attempt to slow down the allocation of memory, with the
                 *  hope that clearing the string vector in the while loop below 
                 *  would decrease ram usage.
                 */
                if (currMemSize > slowTasksDown)
                {
                    this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned long long>(clamp(1.5f * (currMemSize / slowTasksDown), 2.0L, 2000.0L))));
                }
            });
    }

    unsigned long bucketIndex = 0;

    while (pool->numOfTasks() || pool->working() || bucketIndex < numOfBuckets)
    {
        while (bucketIndex < numOfBuckets && !buckets[bucketIndex].empty())
        {
            file << buckets[bucketIndex];

            buckets[bucketIndex].clear();

            while (!mutx.try_lock())
            {
                this_thread::sleep_for(chrono::nanoseconds(10));
            }

            currMemSize -= bucketMemSize[bucketIndex];

            mutx.unlock();

            bucketMemSize[bucketIndex] = 0UL;

            bucketIndex++;

            /*  was an attempt to flush the beginning file contents out of system memory but I don't 
             *  believe it's possible unless ios::ate was the correct parameter. So if you want to 
             *  generate a file with 5 billion entries, you'll need 185+ gigabytes of ram.
             */

            /*if (bucketIndex % 500 == 0)
            { 
                file.close();
                this_thread::sleep_for(chrono::nanoseconds(100));
                file.open(filepath, ios::app);

                while (!file.is_open())
                {
                    this_thread::sleep_for(chrono::nanoseconds(10));
                }
            }*/
        }

        cout << format("Num of Tasks left: {}, current bucketIndex: {}, max bucket Index: {}", pool->numOfTasks(), bucketIndex, numOfBuckets) << endl;
        this_thread::sleep_for(chrono::milliseconds(clamp(pool->numOfTasks(), 1, 1500)));
    }


    file << "\telse:\n\t\tprint(\"God there has to be an easier way.\")";
    file.close();

    const long long time = std::chrono::duration_cast<std::chrono::microseconds>(chrono::high_resolution_clock::now() - startTime).count();

    cout << format("Time taken to generate {} entries: {} seconds", entries, time / pow(10.0, 6)) << endl;

    return 0;
}