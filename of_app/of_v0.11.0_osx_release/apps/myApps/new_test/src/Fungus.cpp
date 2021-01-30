
#include "Fungus.h"
Fungus::Fungus () {
}
// constructor
void Fungus::setup (string _name, string _description, int _id, int _numberViews, string _location, string _imageURL, float _confidence) {
    name = _name;
    description = _description;
    id = _id;
    numberViews = _numberViews;
    location = _location;
    imageURL = _imageURL;
    confidence = _confidence;

}

