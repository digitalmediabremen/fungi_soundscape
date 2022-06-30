
#pragma once
// main Fungi object

#include "ofMain.h"


class Fungus  {

public:
    Fungus();
    void setup(string name, string description, int id, int numberViews, string location, string imageURL, float confidence, bool hasLocation, string date, int locationID);
    float confidence;
    int id;
    int numberViews;
    string description;
    string name;
    string location;
    string imageURL;
    float latitude;
    float longitude;
    string cleanString(string str);
    bool hasLocation;
    string date;
    int locationID;
private:
    
};
