#include "application.h"


int main(int argc, char* argv[]) {
    Application app = {0};

    if (!appInit(&app)) {
        return 1;
    }
    appRun(&app);
    appDestroy(&app);
    return 0;
}