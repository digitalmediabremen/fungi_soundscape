
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
    
    void fetchObservationsOfSpecies(string species);
    void fetchObservationsByLocation(string location);
    void fetchObservationsByLocationID(string locationID);
    void fetchObservationByID(string id);

    ofEvent<void> completedFetchObservations;

    void fetchImage(string url);
    ofEvent<void> completedFetchImage;

    void urlResponse (ofHttpResponse &response);
    ofEvent<string> failedEvent;

    vector<string> * imageUrls;
    
    ofImage * lastLoadedImage;
    vector<Fungus*> fungiList;
    
    void fetchCoordinates (int id);
    ofEvent<void> completedFetchCoordinates;
    ofVec2f lastCoordinates;

private:
    string httpObservationsID;
    string httpCoordinatesID;

    string singleImageRequestID;
    string baseObservationPath;
    string baseImagePath;
    string lastSpecies;
    string lastId;

    string lastLocation;
    void createFungi(ofJson jsonObservations);
};
