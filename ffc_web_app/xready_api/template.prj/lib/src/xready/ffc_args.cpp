#include <xready/ffc_args.h>

FFC* FFC::ffc_= nullptr;;

FFC *FFC::getInstance(){
    if(FFC::ffc_ == nullptr){
        FFC::ffc_ = new FFC();
    }
    return FFC::ffc_;
}

void FFC::setArgs(int argc, char *argv[]){
    FFC::argc = argc;
    for(int i=0;i < argc;++i){
        FFC::argv[i] = argv[i];
    }
}

int FFC::getArgc(){
    return FFC::argc;
}

char ** FFC::getArgv(){
    return FFC::argv;
}
