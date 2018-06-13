#include <iostream>
#include <vector>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <utility>
using namespace std;

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

vector<vector<string> > getProcessesArray(string processesFileName)
{
    int numberOfLines = getFileNumberOfLines(processesFileName);

    ifstream processesFile(processesFileName);
    string line;

    int i = 0;
    vector<vector<string> > processesArray(numberOfLines, vector<string> (3));
    for (int i = 0; i < numberOfLines; ++i)
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

int main(int argc, char* argv[])
{
    string processesFileName = argv[1];
    string localIpNumber = argv[2];
    string lambda = argv[3];
    string k = argv[4];

    //Transform what's in .txt into an array
    vector<vector<string> > processesArray = getProcessesArray(processesFileName);

    //Generate all processes
    for(int i = 0; i < sizeof(processesArray); ++i)
    {
        string processIp = processesArray[i][0];
        string processId = processesArray[i][1];
        string processPort = processesArray[i][2];

        if (processIp == localIpNumber) {
            string cmd("./tottalyOrderedMulticast ");
            cmd += processesFileName + " " + processId + " " + processIp + " " + processPort + " " + lambda + " " + k;
            cout << "About to run command: " << cmd << endl;
            system(cmd.c_str());
        }
    }

    return EXIT_SUCCESS;
}