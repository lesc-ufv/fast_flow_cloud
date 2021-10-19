#ifndef FFCARGS_H
#define FFCARGS_H

#include <stdlib.h>

class FFC{
private:
    int argc;
    char **argv;
    static FFC * ffc_;
protected:
    FFC(){}

public:
    FFC(FFC &other) = delete;

    void operator=(const FFC &) = delete;

    static FFC * getInstance();

    void setArgs(int argc, char *argv[]);

    int getArgc();

    char ** getArgv();
};

#endif //FFCARGS_H
