#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

int isPrime(int number){
    int i;
    for (i=2; i < number; i++){
        if (number % i == 0){
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int sockfd, auxsocketfd, portno, bindStatus, intMessageSent;
    socklen_t clientSize;
    struct sockaddr_in serverAddr, clientAddr;
    char charMessageSent[20];
    char charMessageReceived[20];

    portno = atoi(argv[1]);

    //Open socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        cout << "Error opening socket" << endl;
        return EXIT_FAILURE;
    }
    bzero((char *) &serverAddr, sizeof(serverAddr));
    portno = atoi(argv[1]);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portno);

    //Bind socket
    if (bind(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Error binding socket" << endl;
        return EXIT_FAILURE;
    }

    //Listen to socket
    listen(sockfd, 5);
    clientSize = sizeof(clientAddr);

    //Accept socket
    auxsocketfd = accept(sockfd, (struct sockaddr *) &clientAddr, &clientSize);
    if (auxsocketfd < 0) {
        cout << "Error accepting socket" << endl;
        return EXIT_FAILURE;
    }
    while (1) {
        //Read from socket
        if (read(auxsocketfd, charMessageReceived, 20) < 0) {
            cout << "Error reading from socket" << endl;
            return EXIT_FAILURE;
        }
        int intMessageReceived = atoi(charMessageReceived);
        cout << "Consumer received from producer: " << charMessageReceived << endl;
        if (intMessageReceived == 0) {
            break;
        }

        //Write to socket
        intMessageSent = isPrime(intMessageReceived);
        int n = sprintf (charMessageSent, "%d", intMessageSent);
        cout << "Consumer is about to send to producer: " << charMessageSent << endl;
        if (write(auxsocketfd, charMessageSent, 20) < 0) {
            cout << "Error writing to socket" << endl;
            return EXIT_FAILURE;
        }
    }
    cout << "All products received" << endl;

    //Close socket
    close(auxsocketfd);
    close(sockfd);

    return EXIT_SUCCESS;
}