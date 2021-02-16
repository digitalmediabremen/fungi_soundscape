
#include "Fungus.h"
Fungus::Fungus () {
}
// constructor
void Fungus::setup (string _name, string _description, int _id, int _numberViews, string _location, string _imageURL, float _confidence) {
    
    description = _description;
    ofLog() << description;
    name = _name;
    id = _id;
    numberViews = _numberViews;
    location = _location;
    imageURL = _imageURL;
    confidence = _confidence;

}

