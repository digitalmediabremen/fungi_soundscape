
#pragma once
// this class interacts with the mushroom API to fetch the URLS of species and photos

#include "ofMain.h"
#include "Fungus.h"

#define MAX_REQUESTS 5
#define IMAGE_SIZE 640
#define IMAGE_EXTENSION ".jpg"

class APIService  {

public:
    APIService();
    
    void fetchObservations(string species);
    ofEvent<void> completedFetchObservations;

    
    void fetchImage(string url);
    ofEvent<void> completedFetchImage;

    void urlResponse (ofHttpResponse &response);
    ofEvent<string> failedEvent;

    vector<string> * imageUrls;
    
    ofImage * lastLoadedImage;
    vector<Fungus*> fungiList;

private:
    string httpObservationsID;
    string singleImageRequestID;
    string baseObservationPath;
    string baseImagePath;
    string lastSpecies;
    void createFungi(ofJson jsonObservations);
};
