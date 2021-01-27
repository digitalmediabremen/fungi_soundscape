
#pragma once

#include "ofMain.h"

class ImageProvider  {

public:
    ImageProvider();
    
    string baseImagePath;
    string baseObservationPath;
    
    vector<string> fetchImages(string species);
    ofImage * fetchImage(string url);

private:
};
