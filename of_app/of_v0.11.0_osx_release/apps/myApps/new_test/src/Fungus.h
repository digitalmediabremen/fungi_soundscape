
#pragma once
// main Fungi object

#include "ofMain.h"


class Fungus  {

public:
    Fungus();
    void setup(string name, string description, int id, int numberViews, string location, string imageURL, float confidence);
    float confidence;
    int id;
    int numberViews;
    string description;
    string name;
    string location;
    string imageURL;
private:

};
