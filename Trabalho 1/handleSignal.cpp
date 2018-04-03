#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <unistd.h>

using namespace std;

void signalHandler(int signalIdentifier) {
    cout << "Signal received: " << signalIdentifier << endl;
    if (signalIdentifier == 1) {
        exit(0);
    }
}

int main(int argc, const char * argv[]) {
    signal(10, signalHandler);
    signal(12, signalHandler);
    signal(1, signalHandler);

    int waitingType = atoi(argv[1]);
    switch(waitingType){
        case 1:
            while(1) {
                cout << "Busy waiting process number " << getpid() << endl;
                sleep(1);
            }
            break;
        case 2:
        default:
            while(1) {
                cout << "Blocking waiting process number " << getpid() << endl;
                pause();
            }
            break;
    }

    return EXIT_SUCCESS;
}
