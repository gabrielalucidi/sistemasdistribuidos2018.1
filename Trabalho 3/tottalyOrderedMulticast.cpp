#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <fstream>

typedef unsigned int LamportTime;
std::atomic<LamportTime> time_;
bool seeded = false;

//Handle local event.
//Increment local Lamport time and return the new value.
LamportTime localEvent()
{
    return time_.fetch_add(1);
}

//Handle send event.
//Increment local Lamport time and return the new value.
LamportTime sendEvent()
{
    return time_.fetch_add(1);
}

//Handle receive event.
//Receive sender's Lamport time and set the local Lamport time to the maximum between received and local time plus 1.
LamportTime receiveEvent(LamportTime receivedTime)
{
    return std::max(time_.load(), receivedTime) +1;
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

//Gets random line of a file
std::string getRandomLineInFile(std::string filePath, int numberOfLines)
{
    std::ifstream file(filePath);
    int randomLineNumber = getRandomNumber(1, numberOfLines);

    std::string line;
    for (int i = 0; i < randomLineNumber; ++i)
    {
        std::getline(file,line);
    }
    file.close();
    return line;
}

// Generate local events, add them to local line and send messages.
void localEventsManager()
{
}

//Receive messages, add external events to local line and execute a proccess if messages from all processes were received and event is the first of local line.
void externalEventsManager()
{
}

int main(int argc, char* argv[])
{
    //testing phrase generator
    std::string line = getRandomLineInFile("phrases.txt", 100);
    std::cout << line << std::endl;

    //Create threads
    std::thread firstThread(localEventsManager);
    std::thread secondThread(externalEventsManager);

    //synchronize threads
    firstThread.join();
    secondThread.join();

    return EXIT_SUCCESS;
}