
#pragma once
// this class interacts with the mushroom API to fetch the URLS of species and photos

#include "ofMain.h"

#define MAX_REQUESTS 2

class ImageProvider  {

public:
    ImageProvider();
    
    void fetchImages(string species);
    ofImage * fetchImage(string url);
    void urlResponse (ofHttpResponse &response);
    ofEvent<vector<string>> completedEvent;
    ofEvent<string> failedEvent;

    vector<string> * imageUrls;

private:
    string httpObservationsID;

    string baseImagePath;
    string baseObservationPath;
        
    void requestImageURLs(ofJson jsonObservations);
    void pushImageURL(ofJson jsonImages);
        
    int currentRequests;
    int requestCount;

};
