#include <iostream>
#include <signal.h>
#include <cstdlib>

using namespace std;

int main(int argc, const char * argv[]) {
    int processIdentifier = atoi(argv[1]);
    int signalIdentifier = atoi(argv[2]);

    int success = kill(processIdentifier, signalIdentifier);
    if (success == -1) {
        cout << "Can't send signal " << argv[2]<<" because process " << argv[1] << " doesn't exist" << endl;
        return EXIT_FAILURE;
    }

    cout << "Signal " << argv[2]<<" was sent to process " << argv[1] << endl;
    return EXIT_SUCCESS;
}
