
#include "APIService.h"

// constructor
APIService::APIService () {

     baseImagePath = "https://mushroomobserver.org/images/" + ofToString(IMAGE_SIZE) + "/";
     baseObservationPath = "https://mushroomobserver.org/api2/observations?children_of=";
     ofRegisterURLNotification(this);
    
    httpObservationsID = "observations";
    singleImageRequestID = "image";
    lastSpecies = "";
}


void APIService::fetchObservations (string species) {
    
    ofStringReplace(species, " ", "%20");
    
    if (species == lastSpecies) {
        completedFetchObservations.notify();
        return;
    }
    
    lastSpecies = species;
    imageUrls = new vector<string>();
    ofLog() << "species:" << species;
    string urlObservations = baseObservationPath + species + "&format=json&detail=low";
    ofLoadURLAsync(urlObservations, httpObservationsID);
}

void APIService::urlResponse (ofHttpResponse &response) {
    ofLog () << "name" << response.request.name;
    ofLog () << "status" << response.status;
    
    if (response.status != 200) {
        string error = ofToString(response.status);
        failedEvent.notify(error);
        return;
    }

    if (response.request.name == httpObservationsID) {
        ofJson parsed = ofJson::parse(response.data);
        
        if (parsed.count("errors") > 0) {
            string error = ofToString(parsed["errors"][0]["details"]);
            failedEvent.notify(error);
            return;
        }
        createFungi(parsed);
        return;
    }
    
    if (response.request.name == singleImageRequestID) {
        
        ofLog () << "completed download image";
        lastLoadedImage = new ofImage();
        lastLoadedImage->load(response.data);
        completedFetchImage.notify();
        return;
    }
}

void APIService::createFungi(ofJson jsonObservations) {
    
    bool hasAnyImage = false;
    fungiList.clear();
    // store image urls
    for (int i = 0; i < jsonObservations["results"].size(); i++) {
        if (jsonObservations["results"][i].count("primary_image_id") == 0) continue; // no image
        hasAnyImage = true;
        
        Fungus * f;
        string image_url = baseImagePath + ofToString(jsonObservations["results"][i]["primary_image_id"]) + IMAGE_EXTENSION;

        f = new Fungus();
        
        string name = jsonObservations["results"][i]["consensus_name"];
        string description = ofToString(jsonObservations["results"][i]["notes"]);
        int id = (int)jsonObservations["results"][i]["id"];
        int views = (int)jsonObservations["results"][i]["number_of_views"];
        string location = ofToString(jsonObservations["results"][i]["location_name"]);
        
        bool hasConfidence = jsonObservations["results"][i].count("confidence") > 0;
        
        float confidence = hasConfidence ? float(jsonObservations["results"][i]["confidence"]) : 1.0f;

        ofLog() << "confidence: "<< ofToString(confidence);
                                     
        f->setup(name, description, id, views , location, image_url, confidence);
        
        fungiList.push_back(f);
                // image_count++;
        // string image_url = baseImagePath + ofToString(jsonObservations["results"][i]["primary_image_id"]) + IMAGE_EXTENSION;
        // imageUrls->push_back(image_url);
    }

    ofRemoveAllURLRequests();
    
    if (hasAnyImage) {
        // dispatch event
        completedFetchObservations.notify();
    } else {
        string error = "no image for this species";
        failedEvent.notify(error);
    }
}

void APIService::fetchImage (string url) {

     ofLog () << "fetch image";
     ofLoadURLAsync(url, singleImageRequestID);
}
