
#include "ImageProvider.h"

// constructor
ImageProvider::ImageProvider () {

     // get mushroom image
     baseImagePath = "https://mushroomobserver.nyc3.digitaloceanspaces.com/orig/";
     baseObservationPath = "https://mushroomobserver.org/api2/images?observation=";
     ofRegisterURLNotification(this);
    
    httpObservationsID = "observations";
}


void ImageProvider::fetchImages (string species) {
    
    imageUrls = new vector<string>();

    string urlObservations = "https://mushroomobserver.org/api2/observations?children_of=" + species + "&format=json";
    
    ofLoadURLAsync(urlObservations, httpObservationsID);
}

void ImageProvider::urlResponse (ofHttpResponse &response) {
    if (response.request.name == httpObservationsID) {
        ofJson parsed = ofJson::parse(response.data);
        
        if (parsed.count("errors") > 0) {
            string error = ofToString(parsed["errors"][0]["details"]);
            failedEvent.notify(error);
            return;
        }
        requestImageURLs(parsed);
    } else {
        ofJson json = ofJson::parse(response.data);
        pushImageURL(json);
    }
}

void ImageProvider::requestImageURLs(ofJson jsonObservations) {
    requestCount = 0;
    // request image urls
    currentRequests = (jsonObservations["results"].size() >= MAX_REQUESTS ? MAX_REQUESTS : jsonObservations["results"].size());
    for (int i = 0; i < currentRequests; i++) {
        string observationID = ofToString(jsonObservations["results"][i]);
    
        string urlImages = baseObservationPath + observationID + "&format=json";
        ofLoadURLAsync(urlImages);
    }
    
}

void ImageProvider::pushImageURL(ofJson response) {
    requestCount++;
    for (int a = 0; a < response["results"].size(); a++) {
        // create URL
        string urlToImage = baseImagePath + ofToString(response["results"][a]) + ".jpg";
        imageUrls->push_back(urlToImage);
    }
    if (requestCount == currentRequests - 1) {
        if (imageUrls->size() > 0) {
            // dispatch event
            completedEvent.notify(*imageUrls);
        } else {
            string error = "no image for this species";
            failedEvent.notify(error);
        }
    }
    
}

ofImage * ImageProvider::fetchImage (string url) {
    ofLog() << "url:" << url;
    ofHttpResponse http;
    http = ofLoadURL(url);
    ofLog () << "response" << ofToString(http.status);
    ofImage * image;
    image = new ofImage();
    if (image->load(http.data)) {
        return image;
    }
    ofLogError() << "access denied to image";
    return NULL;
}
