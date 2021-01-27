
#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

#define CA_WIDTH 64
#define CA_HEIGHT 64

class ImageProcessor  {

public:
    ImageProcessor();
    void draw();
    ofImage * pattern;

    ofxCvGrayscaleImage * processImage(ofImage * img);
    
   vector<ofxCvBlob> findContours(ofImage * img);

   ofxCvGrayscaleImage grayImage;//, grayBg, grayDiff;
   ofxCvContourFinder contourFinder;


   ofFbo contoursFbo;
    
   ofImage currentImage;
   ofxCvGrayscaleImage * processedImage;
private:
    void generateMatrix();

};
