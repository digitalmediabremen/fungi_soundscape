
#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
#include "ofxOpenCv.h"


#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 64
#define MIN_TO_FILL 0.7


class BitMatrix  {
    
public:

    BitMatrix();
        
    void clear() noexcept;
    
    void step();
    
    void fill( ofxCvGrayscaleImage img );

    float MATRIX[MATRIX_HEIGHT][MATRIX_WIDTH];


};


