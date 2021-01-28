
#include "ImageProvider.h"

#define IMAGE_SIZE 640
#define IMAGE_EXTENSION ".jpg"
// constructor
ImageProvider::ImageProvider () {

     // get mushroom image
     baseImagePath = "https://mushroomobserver.org/images/" + ofToString(IMAGE_SIZE) + "/";
     baseObservationPath = "https://mushroomobserver.org/api2/observations?children_of=";
     ofRegisterURLNotification(this);
    
    httpObservationsID = "observations";
    singleImageRequestID = "singleimage";
    lastSpecies = "";
}


void ImageProvider::fetchImageURLs (string species) {
    
    if (species == lastSpecies) {
        completedGetImageURLs.notify(*imageUrls);
        return;
    }
    
    lastSpecies = species;
    imageUrls = new vector<string>();

    string urlObservations = baseObservationPath + species + "&format=json&detail=low";
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
        storeImageURLs(parsed);
        return;
    }
    
    if (response.request.name == singleImageRequestID) {
        if (response.status != 200) {
            string error = ofToString(response.status);
            failedEvent.notify(error);
            return;
        }
        
        ofLog () << "completed download image";
        lastLoadedImage = new ofImage();
        lastLoadedImage->load(response.data);
        completedDownloadImage.notify();
        return;
    }
}

void ImageProvider::storeImageURLs(ofJson jsonObservations) {
    int image_count = 0;
    // store image urls
    for (int i = 0; i < jsonObservations["results"].size(); i++) {
        ofLog () << ofToString(jsonObservations["results"][i]);
        if (jsonObservations["results"][i].count("primary_image_id") == 0) continue;
        image_count++;
        string image_url = baseImagePath + ofToString(jsonObservations["results"][i]["primary_image_id"]) + IMAGE_EXTENSION;
        imageUrls->push_back(image_url);
    }
    
    if (image_count > 0) {
        ofRemoveAllURLRequests();
        // dispatch event
        ofLog() << "number of images: " << ofToString(imageUrls->size());
        completedGetImageURLs.notify(*imageUrls);
    } else {
        string error = "no image for this species";
        failedEvent.notify(error);
    }
}

void ImageProvider::fetchImage (string url) {

     ofLog () << "fetch image";
     ofLoadURLAsync(url, singleImageRequestID);
}
