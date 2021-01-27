
#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
// this class processes a image to fit the 64x64 bit matrix with it's contours

#define CA_WIDTH 64
#define CA_HEIGHT 64

class ImageProcessor  {

public:
    ImageProcessor();
    void draw();
    ofxCvGrayscaleImage * processImage(ofImage * img);
    ofxCvGrayscaleImage * processedImage;
    vector<ofxCvBlob> findContours(ofImage * img);
private:
    ofxCvGrayscaleImage grayImage;//, grayBg, grayDiff;
    ofxCvContourFinder contourFinder;
    ofFbo contoursFbo;
    ofImage currentImage;
    void generateMatrix();
};
