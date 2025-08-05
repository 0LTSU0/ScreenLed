#include "ScreenCapBase.h"
#include <iostream>

bool screenCaptureWorkerBase::loadConfigs(){
    std::cout << "screenCaptureWorkerBase::loadConfigs()" << std::endl;
    return false;
}

bool screenCaptureWorkerBase::createConfigFile(){
    return true;
}

bool screenCaptureWorkerBase::initScreenCaptureWorker(){
    loadConfigs();
    openUDPPort();
    return true;
}
