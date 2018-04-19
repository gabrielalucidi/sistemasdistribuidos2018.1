#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

using namespace std;

int numberOfProducts = 10000;
int sharedMemorySize = 32;
vector<int> sharedMemory(sharedMemorySize);
vector<int> consumerLocalMemory(numberOfProducts);

int numberOfProductsProduced = 0;
int numberOfProductsConsumed = 0;

//Some variable needed by the semaphore
mutex semaphoreMutex;
condition_variable semaphoreFull;
condition_variable semaphoreEmpty;

bool seeded = false;

//Checks if number is prime
int isPrime(int number)
{
    int i;
    for (i=2; i < number; i++) {
        if (number % i == 0) {
            return 0;
        }
    }
    return 1;
}

//Gets random number between min and max
int getRandomNumber(int min, int max)
{
    if (!seeded) {
        srand(time(NULL)); //Seeds only once
        seeded = true;
    }
    return min + rand() % (( max + 1 ) - min);
}

int getFirstFreePosition(vector<int> memory)
{
    for (int i = 0; i < memory.size(); ++i)
    {
        if (memory[i] == 0) {
            return i;
        }
    }
    return -1;
}

int getFirstFullPosition(vector<int> memory)
{
    for (int i = 0; i < memory.size(); ++i)
    {
        if (memory[i] != 0) {
            return i;
        }
    }
    return -1;
}

bool isTotallyFree(vector<int> memory)
{
    if (getFirstFullPosition(memory) == -1) {
        return true;
    }
    return false;
}

bool isTotallyFull(vector<int> memory)
{

    if (getFirstFreePosition(memory) == -1) {
        return true;
    }
    return false;
}

void producer()
{
    while(numberOfProductsProduced <= numberOfProducts) {
        int producerProduct = getRandomNumber(1, 10000000);
        unique_lock<mutex> lock(semaphoreMutex);
        //When thread is ready, check if predicate is true;
        //If not, go to the list of waiting threads and will be ready again when notify_all() or notify_one() is called to empty semaphores or when the time expires.
        //When predicate is true, locks mutex, run critical section, wakes up full semaphores and unlocks mutex.
        if(semaphoreEmpty.wait_for(lock, chrono::milliseconds(200), [] {return !isTotallyFull(sharedMemory);})){
            //checks again because condition might not be true anymore
            if (numberOfProductsProduced <= numberOfProducts) {
                int sharedMemoryFreePosition = getFirstFreePosition(sharedMemory);
                sharedMemory[sharedMemoryFreePosition] = producerProduct;
                numberOfProductsProduced += 1;
                semaphoreFull.notify_all();
            }
        }
    }
}

void consumer()
{
    while (numberOfProductsConsumed <= numberOfProducts) {
        int consumerProduct;
        unique_lock<mutex> lock(semaphoreMutex);
        //When thread is ready, check if predicate is true;
        //If not, go to the list of waiting threads and will be ready again when notify_all() or notify_one() is called to full semaphores or when the time expires.
        //When predicate is true, locks mutex, run critical section, wakes up empty semaphores and unlocks mutex.
        if(semaphoreFull.wait_for(lock, chrono::milliseconds(200), [] {return !isTotallyFree(sharedMemory);})){
            //checks again because condition might not be true anymore
            if (numberOfProductsConsumed <= numberOfProducts) {
                int sharedMemoryFullPosition = getFirstFullPosition(sharedMemory);
                int consumerLocalMemoryFreePosition = getFirstFreePosition(consumerLocalMemory);
                consumerProduct = sharedMemory[sharedMemoryFullPosition];
                consumerLocalMemory[consumerLocalMemoryFreePosition] = consumerProduct;
                sharedMemory[sharedMemoryFullPosition] = 0;
                numberOfProductsConsumed += 1;
                semaphoreEmpty.notify_all();
            }
        }
        int isPrimeNumber = isPrime(consumerProduct);
    }
}

int main(int argc, char* argv[])
{
    int Nc, Np;
    Np = atoi(argv[1]);
    Nc = atoi(argv[2]);

    int numberOfProducerThreads = Np;
    int numberOfConsumerThreads = Nc;
    int totalNumberOfThreads = numberOfProducerThreads + numberOfConsumerThreads;

    thread allThreads[totalNumberOfThreads];

    //Sets all sharedMemory N elements to zero because it should be totally emtpy in the begining.
    for (int i = 0; i < sharedMemory.size(); ++i)
    {
        sharedMemory[i] = 0;
    }

    //Register producer threads
    for (int i = 0; i < numberOfProducerThreads; ++i)
    {
        allThreads[i] = thread(producer);
    }

    //Register consumer threads
    for (int i = numberOfProducerThreads; i < totalNumberOfThreads; ++i)
    {
        allThreads[i] = thread(consumer);
    }

    //Starts counting total time (threads join and execute)
    chrono::time_point<std::chrono::system_clock> start, end;
    start = chrono::system_clock::now();

    //Join all threads to main thread
    for (int i = 0; i < totalNumberOfThreads; ++i)
    {
        allThreads[i].join();
    }

    //Ends counting total time (threads join and execute)
    end = chrono::system_clock::now();
    long totalTime = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();
    cout << totalTime << endl;

    return EXIT_SUCCESS;
}