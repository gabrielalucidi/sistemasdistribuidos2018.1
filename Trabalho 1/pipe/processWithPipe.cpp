#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>

using namespace std;

int getRandomNumber(int reference) {
    srand(time(NULL));
    int increment = rand() % 100 + 1;
    int randomNumber = reference + increment;

    return randomNumber;
}

bool isPrime(int number){
    int i;
    for (i=2; i < number; i++){
        if (number % i == 0){
            return false;
        }
    }
    return true;
}

int main(int argc, const char * argv[]) {
    int fd[2];
    if (pipe(fd) == -1) {
        cout << "Error creating pipe" << endl;
        return EXIT_FAILURE;
    }

    int forkStatus = fork();
    if (forkStatus == -1) {
        cout << "Error forking" << endl;
        return EXIT_FAILURE;
    }

    int numberOfProducts = atoi(argv[1]);
    int lowestNumber = 0;
    if (forkStatus > 0) {
        // Producer. This is the parent process. Close other end first.
        close(fd[0]);
        for (int i = 0; i < numberOfProducts; i++) {
            int intMessage = getRandomNumber(lowestNumber);
            lowestNumber = intMessage;
            char charMessage[20];
            int n = sprintf(charMessage, "%d", intMessage);
            cout << "Producer is about to send: " << charMessage << endl;
            write(fd[1], charMessage, 20);
            sleep(1);
        }

        cout << "All products were sent" << endl;
        write(fd[1], "0", 20);
        exit(0);
    } else {
        // Consumer. This is the child process. Close other end first.
        while (1) {
            close(fd[1]);
            char charMessage[20];
            read(fd[0], charMessage, 20);
            cout << "Consumer received: " << charMessage << endl;
            int intMessage = atoi(charMessage);
            if (intMessage == 0) {
                break;
            }
            int prime = isPrime(intMessage);
            if (prime) {
                cout << "Number " << charMessage << " is prime"<< endl;
            } else {
                cout << "Number " << charMessage << " is not prime"<< endl;
            }
        }
        close(fd[0]);
        cout << "All products received"<< endl;
        exit(0);
    }

    return EXIT_SUCCESS;
}
