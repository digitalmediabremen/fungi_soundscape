
#pragma once
// this class interacts with the mushroom API to fetch the URLS of species and photos

#include "ofMain.h"

class ImageProvider  {

public:
    ImageProvider();
    
    vector<string> fetchImages(string species);
    ofImage * fetchImage(string url);
private:
    string baseImagePath;
    string baseObservationPath;
};
