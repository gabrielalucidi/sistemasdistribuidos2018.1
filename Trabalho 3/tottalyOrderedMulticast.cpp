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
#include <arpa/inet.h>
using namespace std;


typedef unsigned int LamportTime;
bool seeded = false;

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

// Generate local events, add them to local time and send messages.
void localEventsManager(string processId, int eventsPerSecond, int maxNumberOfEvents)
{
    int totalNumberOfEvents = 0;
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
    }
}

//Receive messages, add external events to local time and execute a proccess if messages from all processes were received and event is the first of local line.
void externalEventsManager(string processId)
{
}

int getFileNumberOfLines(std::string filePath)
{
    std::ifstream file(filePath);
    std::string line;

    int numberOfLines = 0;
    while (std::getline(file,line)) {
        numberOfLines++;
    }

    return numberOfLines;
}

vector<string> explodeFileLine(const string& s, const char& c)
{
    string buff;
    vector<string> v;

    for(auto n:s) {
        if(n != c) {
            buff+=n;
        } else if(n == c && buff != "") {
            v.push_back(buff);
            buff = "";
        }
    }
    if(buff != "") {
        v.push_back(buff);
    }

    return v;
}

vector<vector<string>> getProcessesArray(string processesFileName)
{
    int numberOfLines = getFileNumberOfLines(processesFileName);

    ifstream processesFile(processesFileName);
    string line;

    int i = 0;
    vector<vector<string>> processesArray(numberOfLines, vector<string> (2));
    for (unsigned int i = 0; i < numberOfLines; ++i)
    {
        getline(processesFile,line);
        vector<string> info = explodeFileLine(line, ' ');

        processesArray[i][0] = info[0];
        processesArray[i][1] = info[1];
    }
    processesFile.close();
    return processesArray;
}

void createClientSockets()
{
    int sockfd, auxsocketfd, portno;
    socklen_t clientSize;
    struct sockaddr_in serverAddr, clientAddr;

    portno = 3000;

    //Open socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        cout << "Error opening socket" << endl;
        exit(0);
    }
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portno);

    //Bind socket
    if (::bind(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Error binding socket" << endl;
        exit(0);
    }

    //Listen to socket
    listen(sockfd, 5);
    clientSize = sizeof(clientAddr);

    //Accept socket
    auxsocketfd = accept(sockfd, (struct sockaddr *) &clientAddr, &clientSize);
    if (auxsocketfd < 0) {
        cout << "Error accepting socket" << endl;
        exit(0);
    }
}

void createServerSockets(vector<vector<string>> &processesArray, string localIdNumber)
{
    int sockfd;
    struct sockaddr_in serverAddr;

    for (int i = 0; i < sizeof(processesArray); ++i)
    {
        string processIp = processesArray[i][0];
        string processId = processesArray[i][1];

        const char *ipstr = processIp.c_str();
        struct in_addr *ip;
        struct hostent *server;

        if (!inet_aton(ipstr, ip)) {
            cout << "Can't parse IP address" << endl;
            exit(0);
        }

        if ((server = gethostbyaddr((const void *)&ip, sizeof ip, AF_INET)) == NULL) {
            cout << "No such host" << endl;
            exit(0);
        }

        if (localIdNumber != processId) {
            int portno = 3000;

            //Open socket
            sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sockfd < 0) {
                cout << "Error opening socket" << endl;
                exit(0);
            }
            bzero((char *) &serverAddr, sizeof(serverAddr));
            serverAddr.sin_family = AF_INET;
            bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
            serverAddr.sin_port = htons(portno);

            //Connect socket
            if (connect(sockfd,(struct sockaddr *) &serverAddr,sizeof(serverAddr)) < 0) {
                cout << "Error connecting socket" << endl;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    string processesFileName = argv[1];
    string localIpNumber = argv[2];
    int lambda = atoi(argv[3]);
    int k = atoi(argv[4]);

    //Transform what's in .txt into array
    vector<vector<string>> processesArray = getProcessesArray(processesFileName);

    //Generate all processes
    for(int i = 0; i < sizeof(processesArray); ++i)
    {
        string processIp = processesArray[i][0];
        string processId = processesArray[i][1];
        if (processIp == localIpNumber) {
            int forkStatus = fork();
            if (forkStatus == -1) {
                cout << "Error forking" << endl;
                return EXIT_FAILURE;
            }
            if (forkStatus == 0) { //Child process
                //Create sockets - REVIEW
                createServerSockets(processesArray, processId);
                createClientSockets();

                //Create threads
                thread firstThread(localEventsManager, processId, lambda, k);
                thread secondThread(externalEventsManager, processId);

                //synchronize threads
                firstThread.join();
                secondThread.join();
                exit(0);
            }
        }
    }

    return EXIT_SUCCESS;
}