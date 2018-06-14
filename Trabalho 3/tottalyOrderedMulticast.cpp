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
vector <int> clientSockets;
vector<string> eventsWaitLine;
vector<int> eventsWaitLineOKReceived;

class Lock
{
    atomic_flag locked = ATOMIC_FLAG_INIT;
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
    atomic<LamportTime> time;
public:

    LamportTime getTime()
    {
        return time;
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
        time = max(time.load(), receivedTime);
        return time.fetch_add(1);
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

void createServerSocket(int portno)
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

void serverWrite(string processId, string message)
{
    cout << "Server " << processId << " is about to send to client: " << message << endl;
    if (write(serauxsocketfd, message.c_str(), 100) < 0) {
        cout << "Error writing to socket" << endl;
        exit(0);
    }
}

string serverRead(string processId)
{
    char charMessageReceived[100];
    if (read(serauxsocketfd, charMessageReceived, 100) < 0) {
        cout << "Error reading from socket" << endl;
        exit(0);
    }
    cout << "Server " << processId << " received from client: " << charMessageReceived << endl;
    return charMessageReceived;
}

void createClientSocket(string serverProcessIp, int serverPortno)
{
    struct sockaddr_in serverAddr;

    struct hostent *server;
    server = gethostbyname(serverProcessIp.c_str());

    //Open socket
    int clisockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clisockfd < 0) {
        cout << "Error opening socket" << endl;
        exit(0);
    }
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(serverPortno);

    //Connect socket
    if (connect(clisockfd,(struct sockaddr *) &serverAddr,sizeof(serverAddr)) < 0) {
        cout << "Error connecting socket" << endl;
        exit(0);
    }

    clientSockets.push_back(clisockfd);
}

void clientWrite(int clientSocket, string processId, string message)
{
    cout << "Client " << processId << " is about to send to server: " << message << endl;
    if (write(clientSocket, message.c_str(), 100) < 0) {
        cout << "Error writing to socket" << endl;
        exit(0);
    }
}

string clientRead(int clientSocket, string processId)
{
    char charMessageReceived[100];
    if (read(clientSocket, charMessageReceived, 100) < 0) {
        cout << "Error reading from socket" << endl;
        exit(0);
    }
    cout << "Client " << processId << " received from server: " << charMessageReceived << endl;
    return charMessageReceived;
}

void writeInSockets(string processId, string message)
{
    if (processId != "1") {
        serverWrite(processId, message);
    }

    int numberOfClientSockets = clientSockets.size();
    for (int i = 0; i < numberOfClientSockets; ++i)
    {
        clientWrite(clientSockets[i], processId, message);
    }
}

vector<string> readFromSockets(string processId)
{
    vector<string> messagesReceived;

    if (processId != "1") {
        string serverMessageReceived = serverRead(processId);
        messagesReceived.push_back(serverMessageReceived);
    }

    int numberOfClientSockets = clientSockets.size();
    for (int i = 0; i < numberOfClientSockets; ++i)
    {
        string clientMessageReceived = clientRead(clientSockets[i], processId);
        messagesReceived.push_back(clientMessageReceived);
    }

    return messagesReceived;
}

void closeSocket(int socket)
{
    close(socket);
}

int getFileNumberOfLines(string filePath)
{
    ifstream file(filePath);
    string line;

    int numberOfLines = 0;
    while (getline(file,line)) {
        numberOfLines++;
    }

    return numberOfLines;
}

vector<string> explode(string const & s, char delim)
{
    vector<string> result;
    istringstream iss(s);

    for (string token; getline(iss, token, delim); )
    {
        result.push_back(move(token));
    }

    return result;
}

vector<vector<string>> getProcessesArray(string processesFileName)
{
    int numberOfLines = getFileNumberOfLines(processesFileName);

    ifstream processesFile(processesFileName);
    string line;

    int i = 0;
    vector<vector<string>> processesArray(numberOfLines, vector<string> (3));
    for (unsigned int i = 0; i < numberOfLines; ++i)
    {
        getline(processesFile,line);
        vector<string> info = explode(line, ' ');

        processesArray[i][0] = info[0];
        processesArray[i][1] = info[1];
        processesArray[i][2] = info[2];
    }
    processesFile.close();
    return processesArray;
}

void coutStringVector(vector<string> path)
{
    int pathLenght = path.size();
    cout << "Print string vector:" << endl;
    for (int i = 0; i < pathLenght; ++i)
    {
        cout << path[i] << endl;
    }
}

void coutIntVector(vector<int> path)
{
    int pathLenght = path.size();
    cout << "Print int vector:" << endl;
    for (int i = 0; i < pathLenght; ++i)
    {
        cout << path[i] << endl;
    }
}

void addEventInWaitLine(string event)
{
    vector<string> eventInfo = explode(event, '_');
    string eventType = eventInfo[0];
    string eventProcessId = eventInfo[1];
    string eventLamportTime = eventInfo[2];
    float eventFloatLamportTime = stof(eventLamportTime + "." + eventProcessId);
    string eventMessage = eventInfo[3];

    if (eventType == "EV") {
        int waitLineLenght = eventsWaitLine.size();
        if (waitLineLenght == 0) {
            cout << "Will add " << event << " as first element because vector is empty." << endl;
            eventsWaitLine.push_back(event);
            eventsWaitLineOKReceived.push_back(1);
        } else if (waitLineLenght > 0) {
            cout << "Current vector size is: " << waitLineLenght << endl;
            bool wasAddded;
            for (int i = 0; i < waitLineLenght; ++i) {
                vector<string> vectorEventInfo = explode(eventsWaitLine[i], '_');
                string vectorEventType = vectorEventInfo[0];
                string vectorEventProcessId = vectorEventInfo[1];
                string vectorEventLamportTime = vectorEventInfo[2];
                float vectorEventFloatLamportTime = stof(vectorEventLamportTime + "." + vectorEventProcessId);
                string vectorEventMessage = vectorEventInfo[3];

                if (eventFloatLamportTime > vectorEventFloatLamportTime) {
                    auto match = find(begin(eventsWaitLine), end(eventsWaitLine), event);
                    if (end(eventsWaitLine) == match) {
                        cout << "About to add event " << event << " in wait line." << endl;
                        eventsWaitLine.insert(eventsWaitLine.begin() + i, event);
                        eventsWaitLineOKReceived.insert(eventsWaitLineOKReceived.begin() + i, 1);
                        wasAddded = true;
                    }
                }
            }
            if (!wasAddded) {
                cout << "Will add " << event << " as first element because lamport time is lower." << endl;
                auto match = find(begin(eventsWaitLine), end(eventsWaitLine), event);
                if (end(eventsWaitLine) == match) {
                    cout << "About to add event " << event << " in wait line." << endl;
                    eventsWaitLine.insert(eventsWaitLine.end(), event);
                    eventsWaitLineOKReceived.insert(eventsWaitLineOKReceived.end(), 1);
                }
            }
        }
    }
    coutStringVector(eventsWaitLine);
    coutIntVector(eventsWaitLineOKReceived);
}

void addOKInEvent(string event)
{
    vector<string> eventInfo = explode(event, '_');
    string eventType = eventInfo[0];
    string eventProcessId = eventInfo[1];
    string eventLamportTime = eventInfo[2];
    float eventFloatLamportTime = stof(eventLamportTime + "." + eventProcessId);
    string eventMessage = eventInfo[3];

    if (eventType == "OK") {
        int waitLineLenght = eventsWaitLine.size();
        for (int i = 0; i < waitLineLenght; ++i) {
            vector<string> vectorEventInfo = explode(eventsWaitLine[i], '_');
            string vectorEventType = eventInfo[0];
            string vectorEventProcessId = vectorEventInfo[1];
            string vectorEventLamportTime = vectorEventInfo[2];
            float vectorEventFloatLamportTime = stof(vectorEventLamportTime + "." + vectorEventProcessId);
            string vectorEventMessage = vectorEventInfo[3];

            if (eventMessage == vectorEventMessage) {
                int OKNumber = eventsWaitLineOKReceived[i] + 1;
                eventsWaitLineOKReceived[i] = OKNumber;
            }
        }
    }
    coutStringVector(eventsWaitLine);
    coutIntVector(eventsWaitLineOKReceived);
}

// Generate local events, add them to local wait line and send messages.
void localEventsManager(string processesFileName, string processId, int eventsPerSecond, int maxNumberOfEvents)
{
    int totalNumberOfEvents = 0;
    while(totalNumberOfEvents < maxNumberOfEvents) {
        for (int i = 0; i < eventsPerSecond; ++i)
        {
            string phraseSent = getRandomLineInFile("phrases.txt", 100) + processId;

            slock.aquire();
            cout << "Process " << processId <<  " local event: " << phraseSent << endl;
            lamport.sendEvent();
            cout << "Process " << processId <<  " lamport time after send local event: " << lamport.getTime() << endl;

            string messageSent = "EV_" + processId + "_" + to_string(lamport.getTime()) + "_" + phraseSent;
            addEventInWaitLine(messageSent);

            writeInSockets(processId, messageSent);
            slock.release();
        }
        sleep(1);
        totalNumberOfEvents += eventsPerSecond;
    }
}

//Receive messages, add external events to local wait line and send OK message.
void externalEventsManager(string processId)
{
    while (1) {
        vector<string> messagesReceived = readFromSockets(processId);

        int numberOfMessagesReceived = messagesReceived.size();
        for (int i = 0; i < numberOfMessagesReceived; ++i)
        {
            string messageReceived = messagesReceived[i];
            vector<string> messageReceivedSplited = explode(messageReceived, '_');
            string messageReceivedType = messageReceivedSplited[0];
            string processIdReceived = messageReceivedSplited[1];
            LamportTime LamportTimeReceived = stoi(messageReceivedSplited[2]);
            string phraseReceived = messageReceivedSplited[3];

            slock.aquire();
            lamport.receiveEvent(LamportTimeReceived);
            cout << "Lamport time after receive from process " << processIdReceived << " with lamport time = " << LamportTimeReceived << " is: " << to_string(lamport.getTime()) << endl;

            if (messageReceivedType == "EV") {
                addEventInWaitLine(messageReceived);
                string messageSent = "OK_" + processId + "_" + to_string(lamport.getTime()) + "_" + phraseReceived;
                writeInSockets(processId, messageSent);
                //Send confirmation to itself
                addOKInEvent(messageSent);
            }
            if (messageReceivedType == "OK") {
                addOKInEvent(messageReceived);
            }
            slock.release();
        }
    }
}

// Write phrase in log if OK messages from all processes were received and event is the first of local wait line.
void writePhraseInLog(string processId, int totalNumberOfprocesses)
{
    //Create log file
    string fileName = "process_" + processId + "_log.txt";
    ofstream logFile (fileName);
    logFile << "Process " << processId << " log file"<< endl;
    logFile << "" << endl;

    while(1) {
        slock.aquire();
        int waitLineLenght = eventsWaitLine.size();
        if (waitLineLenght != 0) {
            int lastElementIndex = waitLineLenght - 1;
            if (eventsWaitLineOKReceived[lastElementIndex] == totalNumberOfprocesses) {
                cout << "Number of OKs: " << eventsWaitLineOKReceived[lastElementIndex] << endl;
                //Write in log file
                logFile << eventsWaitLine[lastElementIndex] << endl;
                cout << "Phrase was written in log file: " << eventsWaitLine[lastElementIndex] << endl;
                eventsWaitLine.pop_back();
                eventsWaitLineOKReceived.pop_back();
            }
        }
        slock.release();
    }

    //Close log file
    logFile.close();
}

int main(int argc, char* argv[])
{
    string processesFileName = argv[1];
    string processId = argv[2];
    string processIp = argv[3];
    int portno = atoi(argv[4]);
    int lambda = atoi(argv[5]);
    int k = atoi(argv[6]);
    int totalNumberOfprocesses = getFileNumberOfLines(processesFileName);
    int totalNumberOfEventsPerProcess = int(k/totalNumberOfprocesses);

    cout << "--------" << endl;
    cout << "Running process number " << processId << " with IP "<< processIp << " receiving messages on port " << portno<< endl;

    if (processId != "1") {
        createServerSocket(portno);
    }

    vector<vector<string>> otherProcessesArray = getProcessesArray(processesFileName);
    for(int i = 0; i < totalNumberOfprocesses; ++i)
    {
        string otherProcessIp = otherProcessesArray[i][0];
        string otherProcessId = otherProcessesArray[i][1];
        int otherProcessPort = stoi(otherProcessesArray[i][2]);

        if (processId < otherProcessId) {
            createClientSocket(otherProcessIp, otherProcessPort);
        }
    }

    //Create threads
    thread firstThread(localEventsManager, processesFileName, processId, lambda, totalNumberOfEventsPerProcess);
    thread secondThread(externalEventsManager, processId);
    thread thirdThread(writePhraseInLog, processId, totalNumberOfprocesses);

    //Synchronize threads
    firstThread.join();
    secondThread.join();
    thirdThread.join();

    //Close sockets
    int numberOfClientSockets = clientSockets.size();
    for (int i = 0; i < numberOfClientSockets; ++i)
    {
        closeSocket(clientSockets[i]);
    }
    closeSocket(serauxsocketfd);
    closeSocket(sersockfd);

    return EXIT_SUCCESS;
}