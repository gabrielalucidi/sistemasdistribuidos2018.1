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

void produce()
{
    //Thread keeps waiting until mutex and empty are unlocked.
    //When mutex is unlocked, lock mutex, but keep waiting until empty is unlocked.
    //When empty is unlocked, lock empty, but keep waiting until predicate is true.
    //If predicate is true in less than 200s, run critical section, unlock mutex, unlock full.
    //If predicate is not true in 200s, unlock mutex, unlock empty.
    unique_lock<mutex> lock(semaphoreMutex);
    if(semaphoreEmpty.wait_for(lock, chrono::milliseconds(200), [] {return !isTotallyFull(sharedMemory);})){
        int product = getRandomNumber(1, 10000000);
        int sharedMemoryFreePosition = getFirstFreePosition(sharedMemory);
        sharedMemory[sharedMemoryFreePosition] = product;
        cout << "Consumer sent product: " << sharedMemory[sharedMemoryFreePosition] << endl;
        numberOfProductsProduced += 1;
        semaphoreFull.notify_all();
    }
}

void consume()
{
    //Thread keeps waiting until mutex and full are unlocked.
    //When mutex is unlocked, lock mutex, but keep waiting until full is unlocked.
    //When full is unlocked, lock full, but keep waiting until predicate is true.
    //If predicate is true in less than 200s, run critical section, unlock mutex, unlock empty.
    //If predicate is not true in 200s, unlock mutex, unlock full.
    unique_lock<mutex> lock(semaphoreMutex);
    if(semaphoreFull.wait_for(lock, chrono::milliseconds(200), [] {return !isTotallyFree(sharedMemory);})){
        int sharedMemoryFullPosition = getFirstFullPosition(sharedMemory);
        cout << "Consumer received product: " << sharedMemory[sharedMemoryFullPosition] << endl;
        int isPrimeNumber = isPrime(sharedMemory[sharedMemoryFullPosition]);
        if (isPrimeNumber) {
            cout << "Number "<< sharedMemory[sharedMemoryFullPosition] << " is prime"<< endl;
        } else {
            cout << "Number "<< sharedMemory[sharedMemoryFullPosition] << " is not prime"<< endl;
        }
        int consumerLocalMemoryFreePosition = getFirstFreePosition(consumerLocalMemory);
        consumerLocalMemory[consumerLocalMemoryFreePosition] = sharedMemory[sharedMemoryFullPosition];
        sharedMemory[sharedMemoryFullPosition] = 0;
        numberOfProductsConsumed += 1;
        semaphoreEmpty.notify_all();
    }
}

// Only produce if number of produced products is < 10000
void producer()
{
    while (numberOfProductsProduced < numberOfProducts) {
        produce();
    }
}

// Only consume if number of consumed products is < 10000
void consumer()
{
    while (numberOfProductsConsumed < numberOfProducts) {
        consume();
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

    //Starts counting time
    chrono::time_point<std::chrono::system_clock> start, end;
    start = chrono::system_clock::now();

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

    //Join all threads to main thread
    for (int i = 0; i < totalNumberOfThreads; ++i)
    {
        allThreads[i].join();
    }

    //Ends counting time
    end = chrono::system_clock::now();
    long totalTime = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();
    cout << "Total time is: " << totalTime << endl;

    return EXIT_SUCCESS;
}