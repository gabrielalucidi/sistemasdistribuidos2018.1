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
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
using namespace std;


typedef unsigned int LamportTime;
bool seeded = false;
int sersockfd, serauxsocketfd;
int clisockfd;

class Lock
{
    std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
    void aquire()
    {
        while(locked.test_and_set()){}
    }
    void release()
    {
        locked.clear();
    }
};

Lock slock;

class Lamport
{
    std::atomic<LamportTime> time;
public:

    LamportTime getTime()
    {
        return time;
    }
    //Handle local event.
    //Increment local Lamport time and return the new value.
    LamportTime localEvent()
    {
        return time.fetch_add(1);
    }

    //Handle send event.
    //Increment local Lamport time and return the new value.
    LamportTime sendEvent()
    {
        return time.fetch_add(1);
    }

    //Handle receive event.
    //Receive sender's Lamport time and set the local Lamport time to the maximum between received and local time plus 1.
    LamportTime receiveEvent(LamportTime receivedTime)
    {
        return max(time.load(), receivedTime) +1;
    }
};

Lamport lamport;


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

void createServerSockets(int portno)
{
    socklen_t clientSize;
    struct sockaddr_in serverAddr, clientAddr;

    //Open socket
    sersockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sersockfd < 0) {
        cout << "Error opening socket" << endl;
        exit(0);
    }
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portno);

    //Bind socket
    if (::bind(sersockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Error binding socket" << endl;
        exit(0);
    }

    //Listen to socket
    listen(sersockfd, 5);
    clientSize = sizeof(clientAddr);

    //Accept socket
    serauxsocketfd = accept(sersockfd, (struct sockaddr *) &clientAddr, &clientSize);
    if (serauxsocketfd < 0) {
        cout << "Error accepting socket" << endl;
        exit(0);
    }
}

void createClientSockets(string processIp, int portno)
{
    struct sockaddr_in serverAddr;

    struct hostent *server;
    server = gethostbyname(processIp.c_str());

    //Open socket
    clisockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clisockfd < 0) {
        cout << "Error opening socket" << endl;
        exit(0);
    }
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(portno);

    //Connect socket
    if (connect(clisockfd,(struct sockaddr *) &serverAddr,sizeof(serverAddr)) < 0) {
        cout << "Error connecting socket" << endl;
        exit(0);
    }
}

void closeSocket(int socket)
{
    close(socket);
}

// Generate local events, add them to local time and send messages.
void localEventsManager(int processId, string processIp, int portno, int eventsPerSecond, int maxNumberOfEvents)
{
    char charMessageReceived[2];
    string line = getRandomLineInFile("phrases.txt", 100);
    createClientSockets(processIp, portno);
    //write in socket
    cout << "Client is about to send to server: " << line << endl;
    if (write(clisockfd, line.c_str(), 100) < 0) {
        cout << "Error writing to socket" << endl;
        exit(0);
    }
    slock.aquire();
    lamport.localEvent();
    cout << lamport.getTime() << endl;
    slock.release();
    //Read from socket
    if (read(clisockfd, charMessageReceived, 2) < 0) {
        cout << "Error reading from socket" << endl;
        exit(0);
    }
    cout << "Client received from server: " << charMessageReceived << endl;
/*    int totalNumberOfEvents = 0;
    while(totalNumberOfEvents < maxNumberOfEvents) {
        for (int i = 0; i < eventsPerSecond; ++i)
        {
            string line = getRandomLineInFile("phrases.txt", 100);
            slock.aquire();
            lamport.localEvent();
            cout << lamport.getTime() << endl;
            slock.release();
        }
        sleep(1);
        totalNumberOfEvents += eventsPerSecond;
    }*/
    closeSocket(clisockfd);
}

//Receive messages, add external events to local time and execute a proccess if messages from all processes were received and event is the first of local line.
void externalEventsManager(int processId, int portno)
{
    char charMessageReceived[100];

    createServerSockets(portno);

    while (1) {
        //Read from socket
        if (read(serauxsocketfd, charMessageReceived, 100) < 0) {
            cout << "Error reading from socket" << endl;
            exit(0);
        }
        cout << "Server received from client: " << charMessageReceived << endl;

        //Write to socket
        string stringMessageSent = "OK";
        cout << "Server is about to send to client: " << stringMessageSent << endl;
        if (write(serauxsocketfd, stringMessageSent.c_str(), 2) < 0) {
            cout << "Error writing to socket" << endl;
            exit(0);
        }
    }

    closeSocket(serauxsocketfd);
    closeSocket(sersockfd);
}


int main(int argc, char* argv[])
{
    int processId = atoi(argv[1]);
    string processIp = argv[2];
    int portno = 8084;
    int lambda = atoi(argv[3]);
    int k = atoi(argv[4]);

    cout << "--------" endl;
    cout << "Running process number " << processId << endl;

    //Create threads
    thread firstThread(localEventsManager, processId, processIp, portno, lambda, k);
    thread secondThread(externalEventsManager, processId, portno);

    //synchronize threads
    firstThread.join();
    secondThread.join();

    return EXIT_SUCCESS;
}