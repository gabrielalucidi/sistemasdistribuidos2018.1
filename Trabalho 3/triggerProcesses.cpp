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

int main(int argc, char* argv[])
{
    string processesFileName = argv[1];
    string localIpNumber = argv[2];
    string lambda = argv[3];
    string k = argv[4];

    //Transform what's in .txt into an array
    vector<vector<string>> processesArray = getProcessesArray(processesFileName);

    //Generate all processes
    for(int i = 0; i < sizeof(processesArray); ++i)
    {
        string processIp = processesArray[i][0];
        string processId = processesArray[i][1];
        if (processIp == localIpNumber) {
            string cmd("./tottalyOrderedMulticast ");
            cmd += processId + " " + processIp + " " + lambda + " " + k;
            cout << "About to run command: " << cmd << endl;
            system(cmd.c_str());
        }
    }

    return EXIT_SUCCESS;
}