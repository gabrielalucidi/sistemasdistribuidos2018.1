#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int getRandomNumber(int reference) {
    srand(time(NULL));
    int increment = rand() % 100 + 1;
    int randomNumber = reference + increment;

    return randomNumber;
}

int main(int argc, char *argv[])
{
    int sockfd, portno, numbersBeforeExit, intMessageSent, intLastMessage;
    struct sockaddr_in serverAddr;
    struct hostent *server;
    char charMessageSent[20];
    char charMessageReceived[20];
    char charLastMessage[20];

    server = gethostbyname(argv[1]);
    portno = atoi(argv[2]);
    numbersBeforeExit = atoi(argv[3]);

    //Open socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        cout << "Error opening socket" << endl;
        return EXIT_FAILURE;
    }
    if (server == NULL) {
        cout << "No such host" << endl;
        return EXIT_FAILURE;
    }
    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(portno);

    //Connect socket
    if (connect(sockfd,(struct sockaddr *) &serverAddr,sizeof(serverAddr)) < 0) {
        cout << "Error connecting socket" << endl;
        return EXIT_FAILURE;
    }

    intMessageSent = getRandomNumber(0);
    for(int i=0; i < numbersBeforeExit; i++) {
        //Write to socket
        int n = sprintf (charMessageSent, "%d", intMessageSent);
        cout << "Producer is about to send to consumer: " << charMessageSent << endl;
        if (write(sockfd, charMessageSent, 20) < 0) {
            cout << "Error writing to socket" << endl;
            return EXIT_FAILURE;
        }

        //Read from socket
        if (read(sockfd, charMessageReceived, 20) < 0) {
            cout << "Error reading from socket" << endl;
            return EXIT_FAILURE;
        }
        int intMessageReceived = atoi(charMessageReceived);
        cout << "Producer received from consumer: ";
        if (intMessageReceived) {
            cout << "Number is prime" << endl;
        } else {
            cout << "Number is not prime" << endl;
        }
        intMessageSent = getRandomNumber(intMessageSent);
    }

    intLastMessage = 0;
    int n = sprintf (charLastMessage, "%d", intLastMessage);
    write(sockfd, charLastMessage, 20);
    cout << "All products were sent" << endl;

    //Close socket
    close(sockfd);

    return EXIT_SUCCESS;
}