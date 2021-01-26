
#pragma once

#include "ofMain.h"

class ImageProvider  {

public:
    ImageProvider();
    
    string baseImagePath;
    
    
    vector<string> fetchImages(string species);
private:
};
