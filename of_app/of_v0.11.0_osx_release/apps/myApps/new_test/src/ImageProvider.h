
#pragma once
// this class interacts with the mushroom API to fetch the URLS of species and photos

#include "ofMain.h"

#define MAX_REQUESTS 5

class ImageProvider  {

public:
    ImageProvider();
    
    void fetchImageURLs(string species);
    void fetchImage(string url);
    void urlResponse (ofHttpResponse &response);
    ofEvent<void> completedDownloadImage;
    ofEvent<vector<string>> completedGetImageURLs;
    ofEvent<string> failedEvent;

    vector<string> * imageUrls;
    
    ofImage * lastLoadedImage;

private:
    string httpObservationsID;
    string singleImageRequestID;
    string baseObservationPath;
    string baseImagePath;
    string lastSpecies;
    void storeImageURLs(ofJson jsonObservations);
};
