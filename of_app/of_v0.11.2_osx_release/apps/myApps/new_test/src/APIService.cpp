
#include "APIService.h"

// constructor
APIService::APIService () {

     baseImagePath = "https://mushroomobserver.org/images/" + ofToString(IMAGE_SIZE) + "/";
     baseObservationPath = "https://mushroomobserver.org/api2/observations";
    
     ofRegisterURLNotification(this);
    
    httpObservationsID = "observations";
    singleImageRequestID = "image";
    httpCoordinatesID = "coordinates";
    lastSpecies = "";
}


void APIService::fetchObservationsOfSpecies (string species) {
    
    ofStringReplace(species, " ", "%20");
    
    if (species == lastSpecies) {
        completedFetchObservations.notify();
        return;
    }
    
    lastSpecies = species;
    lastLocation = "";
    imageUrls = new vector<string>();
    ofLog() << "species:" << species;
    string urlObservations = baseObservationPath + "?api_key=t53d3qson4iv8gal8fiove4vqgwudkd2&children_of=" + species + "&format=json&detail=low";
    ofLog () << urlObservations;
    ofLoadURLAsync(urlObservations, httpObservationsID);
}

void APIService::fetchObservationByID (string id) {
    id.erase(std::remove(id.begin(), id.end(), '\"'),id.end());

    //https://mushroomobserver.org/api2/observations?id=541&format=json&detail=high
    if (lastId == id) {
        completedFetchObservations.notify();
        return;
    }
    lastId = id;
    
    lastSpecies = "";
    lastLocation = "";
    imageUrls = new vector<string>();
    string urlObservations = baseObservationPath + "?api_key=t53d3qson4iv8gal8fiove4vqgwudkd2&id=" + id + "&format=json&detail=low";
    //&has_notes=1
    ofLog () << urlObservations;

    ofLoadURLAsync(urlObservations, httpObservationsID);
}

void APIService::fetchObservationsByLocation (string location) {
    
    ofStringReplace(location, " ", "%20");
    
    if (location == lastLocation) {
        completedFetchObservations.notify();
        return;
    }
    
    lastLocation = location;
    lastSpecies = "";
    imageUrls = new vector<string>();
    ofLog() << "location:" << location;
    string urlObservations = baseObservationPath + "?region=" + location + "&api_key=t53d3qson4iv8gal8fiove4vqgwudkd2&format=json&detail=low";
    ofLog () << urlObservations;

    ofLoadURLAsync(urlObservations, httpObservationsID);
}

void APIService::fetchObservationsByLocationID (string id) {
    
    id.erase(std::remove(id.begin(), id.end(), '\"'),id.end());

    if (id == lastLocation) {
        completedFetchObservations.notify();
        return;
    }
    
    lastLocation = id;
    lastSpecies = "";
    imageUrls = new vector<string>();
    ofLog() << "locationID:" << id;
    string urlObservations = baseObservationPath + "?location=" + id + "&api_key=t53d3qson4iv8gal8fiove4vqgwudkd2&format=json&detail=low";
    ofLog () << urlObservations;

    ofLoadURLAsync(urlObservations, httpObservationsID);
}

void APIService::urlResponse (ofHttpResponse &response) {
    ofLog () << "name" << response.request.name;
    ofLog () << "status" << response.status;
    
    if (response.status != 200) {
        string error = ofToString(response.status);
        ofLog() << ofToString(response.error);
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
    
    if (response.request.name == httpCoordinatesID) {

        ofJson parsed = ofJson::parse(response.data);
        if (parsed["results"].size() > 0) {
            ofLog () <<parsed["results"];
            lastCoordinates.x = parsed["results"][0]["location"]["longitude_east"];
            lastCoordinates.y = parsed["results"][0]["location"]["latitude_north"];
            completedFetchCoordinates.notify();
        }
    }

}

void APIService::createFungi(ofJson jsonObservations) {
    bool hasAnyImage = false;
    fungiList.clear();
    // store image urls
    for (int i = 0; i < jsonObservations["results"].size(); i++) {
        if (jsonObservations["results"][i].count("primary_image_id") == 0) continue; // no image
        bool gps_hidden = (jsonObservations["results"][i]["gps_hidden"] == true); // no gps data

        hasAnyImage = true;
        
        Fungus * f;
        string image_url = baseImagePath + ofToString(jsonObservations["results"][i]["primary_image_id"]) + IMAGE_EXTENSION;

        f = new Fungus();
        ofLog () <<  ofToString(jsonObservations["results"][i]);

        string name = jsonObservations["results"][i]["consensus_name"];
        string description = ofToString(jsonObservations["results"][i]["notes"]);
        int id = (int)jsonObservations["results"][i]["id"];
        int views = (int)jsonObservations["results"][i]["number_of_views"];
        string location = ofToString(jsonObservations["results"][i]["location_name"]);
        bool hasLocation = !gps_hidden;
        string date = jsonObservations["results"][i]["created_at"];
        /*
        bool hasLatitude = !gps_hidden && jsonObservations["results"][i].count("latitude") > 0;
        bool hasLongitude = !gps_hidden && jsonObservations["results"][i].count("longitude") > 0;

        float lat = hasLatitude ? ofToFloat(jsonObservations["results"][i]["latitude"]) : 0.0f;
        float lng = hasLongitude ? ofToFloat(jsonObservations["results"][i]["longitude"]) : 0.0f;
         */

        bool hasConfidence = jsonObservations["results"][i].count("confidence") > 0;
        
        float confidence = hasConfidence ? float(jsonObservations["results"][i]["confidence"]) : 1.0f;

        f->setup(name, description, id, views , location, image_url, confidence, hasLocation, date);


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

void APIService::fetchCoordinates (int id) {

    // https://mushroomobserver.org/api2/observations?id=541&format=json&detail=high

    string urlObservations = baseObservationPath + "?api_key=t53d3qson4iv8gal8fiove4vqgwudkd2&id=" + ofToString(id) + "&format=json&detail=high";
    ofLoadURLAsync(urlObservations, httpCoordinatesID);

}



