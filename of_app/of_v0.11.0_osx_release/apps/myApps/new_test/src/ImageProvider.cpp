
#include "ImageProvider.h"

// constructor
ImageProvider::ImageProvider () {

     // get mushroom image
     baseImagePath = "https://mushroomobserver.nyc3.digitaloceanspaces.com/orig/";
     baseObservationPath = "https://mushroomobserver.org/api2/images?observation=";
     ofRegisterURLNotification(this);
    
    httpObservationsID = "observations";
    singleImageRequestID = "singleimage";
}


void ImageProvider::fetchImages (string species) {
    
    imageUrls = new vector<string>();

    string urlObservations = "https://mushroomobserver.org/api2/observations?children_of=" + species + "&format=json";
    
    ofLoadURLAsync(urlObservations, httpObservationsID);
}

void ImageProvider::urlResponse (ofHttpResponse &response) {
    ofLog () << "name" << response.request.name;
    ofLog () << "status" << response.status;

    if (response.request.name == httpObservationsID) {
        ofJson parsed = ofJson::parse(response.data);
        
        if (parsed.count("errors") > 0) {
            string error = ofToString(parsed["errors"][0]["details"]);
            failedEvent.notify(error);
            return;
        }
        requestImageURLs(parsed);
        return;
    }
    
    if (response.request.name == singleImageRequestID) {
        if (response.status != 200) {
            string error = ofToString(response.status);
            failedEvent.notify(error);
            return;
        }
        
        ofLog () << "completed single image";
        lastLoadedImage = new ofImage();
        lastLoadedImage->load(response.data);
        completedDownloadImage.notify();
        return;
    }
    
    ofJson json = ofJson::parse(response.data);
    pushImageURL(json);
}

void ImageProvider::requestImageURLs(ofJson jsonObservations) {
    requestCount = 0;
    // request image urls
    currentRequests = (jsonObservations["results"].size() >= MAX_REQUESTS ? MAX_REQUESTS : jsonObservations["results"].size());
    for (int i = 0; i < currentRequests; i++) {
        string observationID = ofToString(jsonObservations["results"][int(ofRandom(jsonObservations["results"].size() - 1))]);
    
        string urlImages = baseObservationPath + observationID + "&format=json";
        ofLoadURLAsync(urlImages);
    }
    
}

void ImageProvider::pushImageURL(ofJson response) {
    
    for (int a = 0; a < response["results"].size(); a++) {
        // create URL
        string urlToImage = baseImagePath + ofToString(response["results"][a]) + ".jpg";
        imageUrls->push_back(urlToImage);
    }
    requestCount++;
    
    if (requestCount == currentRequests) {
        if (imageUrls->size() > 0) {
            // clear requests
            ofRemoveAllURLRequests();
            // dispatch event
            ofLog() << "number of images: " << ofToString(imageUrls->size());
            completedGetImageURLs.notify(*imageUrls);
        } else {
            string error = "no image for this species";
            failedEvent.notify(error);
        }
    }
}

void ImageProvider::fetchImage (string url) {

    ofLog () << "fetch image";
     ofLoadURLAsync(url, singleImageRequestID);
}
