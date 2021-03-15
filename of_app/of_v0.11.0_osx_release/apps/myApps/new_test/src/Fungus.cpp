
#include "Fungus.h"
Fungus::Fungus () {
}
// constructor
void Fungus::setup (string _name, string _description, int _id, int _numberViews, string _location, string _imageURL, float _confidence) {
    
    description = cleanString(_description);
    ofLog() << description;
    name = _name;
    id = _id;
    numberViews = _numberViews;
    location = _location;
    imageURL = _imageURL;
    confidence = _confidence;

}

string Fungus::cleanString (string str) {
    
    string cleaned = str;
    
    while (cleaned.find("<") != std::string::npos)
    {
        auto startpos = cleaned.find("<");
        auto endpos = cleaned.find(">") + 1;

        if (endpos != std::string::npos)
        {
            cleaned.erase(startpos, endpos - startpos);
        }
    }
    
    while (cleaned.find("\n") != std::string::npos)
    {
        auto startpos = cleaned.find("\n");
        auto endpos = cleaned.find("\n") + 1;

        if (endpos != std::string::npos)
        {
            cleaned.erase(startpos, endpos - startpos);
        }
    }
    ofLog() << cleaned;

    return cleaned;

}
