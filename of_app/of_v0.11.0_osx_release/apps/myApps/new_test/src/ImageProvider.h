
#pragma once
// this class interacts with the mushroom API to fetch the URLS of species and photos

#include "ofMain.h"
#include "ofxThreadedImageLoader.h"

#define MAX_REQUESTS 2

class ImageProvider  {

public:
    ImageProvider();
    
    void fetchImages(string species);
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

    string baseImagePath;
    string baseObservationPath;
        
    void requestImageURLs(ofJson jsonObservations);
    void pushImageURL(ofJson jsonImages);
        
    int currentRequests;
    int requestCount;
    
    ofxThreadedImageLoader imageLoader;

};
