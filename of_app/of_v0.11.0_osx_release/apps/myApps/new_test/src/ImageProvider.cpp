
#include "ImageProvider.h"

// constructor
ImageProvider::ImageProvider () {

 // get mushroom image
 baseImagePath = "https://mushroomobserver.nyc3.digitaloceanspaces.com/orig/";
}


void ImageProvider::getImages (string species) {
    string url = "https://mushroomobserver.org/api2/observations?children_of=" + species + "&format=json";
    http = ofLoadURL(url);
    
    string jsonResponse = http.data;
    ofLog() << jsonResponse;
    
    ofJson json = ofJson::parse( jsonResponse );
    
    vector<string> ids;
    
    ofLog () <<"RESULTADOS:" << json["results"];
}
