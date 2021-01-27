
#include "ImageProvider.h"

// constructor
ImageProvider::ImageProvider () {

     // get mushroom image
     baseImagePath = "https://mushroomobserver.nyc3.digitaloceanspaces.com/orig/";
     baseObservationPath = "https://mushroomobserver.org/api2/images?observation=";
}


vector<string> ImageProvider::fetchImages (string species) {
    string urlObservations = "https://mushroomobserver.org/api2/observations?children_of=" + species + "&format=json";
    ofHttpResponse httpObservations;

    httpObservations = ofLoadURL(urlObservations);
    
    string responseObservations = httpObservations.data;
    
    ofJson jsonObservations = ofJson::parse(responseObservations);
    
    ofLog () << jsonObservations;
    
    vector<string> imageUrls;
        
    // find 10 images
    for (int i = 0; i < jsonObservations["results"].size() && imageUrls.size() < 2; i++) {
        string observationID = ofToString(jsonObservations["results"][i]);
        
        string urlImages = baseObservationPath + observationID + "&format=json";
        ofHttpResponse httpImages;

        httpImages = ofLoadURL(urlImages);
        
        ofJson jsonImages = ofJson::parse(httpImages.data);
        
        ofLog () << "image results: " << jsonImages["results"];
        for (int a = 0; a < jsonImages["results"].size(); a++) {
            ofLog () << "add images" << ofToString(jsonImages["results"][a]);
            // create URL
            string urlToImage = baseImagePath + ofToString(jsonImages["results"][a]) + ".jpg";
            imageUrls.push_back(urlToImage);
        }
    }
    
    ofLog () << "found images! " << ofToString(imageUrls.size());
    return imageUrls;
}

ofImage * ImageProvider::fetchImage (string url) {
    ofHttpResponse http;
    http = ofLoadURL(url);
    ofImage * image;
    image = new ofImage();
    if (image->load(http.data)) {
        return image;
    }
    ofLogError() << "access denied to image";
    return NULL;
}
