
#pragma once

#include "ofMain.h"

class ImageProvider  {

public:
    ImageProvider();
    
    string baseImagePath;
    
    
    void getImages(string species);
private:
    ofHttpResponse http;
};
