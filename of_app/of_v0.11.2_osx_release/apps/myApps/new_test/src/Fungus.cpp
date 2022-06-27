
#include "Fungus.h"
Fungus::Fungus () {
}
#include <iostream>
#include <iomanip>
#include <ctime>
// constructor
void Fungus::setup (string _name, string _description, int _id, int _numberViews, string _location, string _imageURL, float _confidence, bool _hasLocation, string _date) {
    
    description = cleanString(_description);
    ofLog() << description;
    name = _name;
    id = _id;
    numberViews = _numberViews;
    _location.erase(std::remove(_location.begin(), _location.end(), '\"'),_location.end());
    location = _location;
    imageURL = _imageURL;
    confidence = _confidence;
    latitude = 0.0;
    longitude = 0.0;
    hasLocation = _hasLocation;
    struct tm tm;

    strptime(_date.c_str(), "%Y-%m-%d T%H:%M:%S", &tm);

    char buffer[80];

    std::strftime(buffer,sizeof(buffer),"%d-%m-%Y at %H:%M:%S",&tm);
    std::string str(buffer);

    date = str;
    
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
