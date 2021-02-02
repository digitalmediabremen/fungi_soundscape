
#include "BitMatrix.h"
#include "ofxPDSP.h"

BitMatrix::BitMatrix(){
    clear();
}

void BitMatrix::step () {
    for( int y=MATRIX_HEIGHT-1; y>0; --y ){
        for( int x=0; x<MATRIX_WIDTH; ++x){
            MATRIX[y][x] = MATRIX[y-1][x];
        }
    }

}

void BitMatrix::fill( ofxCvGrayscaleImage img) {
    filledPercentage = 0.0f;
    for( int x=0; x<MATRIX_WIDTH; ++x ){
        for( int y=0; y<MATRIX_HEIGHT; ++y ){
            float pixelForce = img.getPixels().getColor(x,y).getLightness() / 255.0f;
            if (pixelForce > MIN_TO_FILL) {
                MATRIX[y][x] = pixelForce;
                filledPercentage+=pixelForce;
            } else {
                MATRIX[y][x] = 0;
            }
        }
    }
    filledPercentage = filledPercentage / float(MATRIX_HEIGHT * MATRIX_WIDTH);
    ofLog() << "filled percentage: " << filledPercentage;
}

void BitMatrix::clear() noexcept{
    for( int y=0; y<MATRIX_HEIGHT; ++y ){
        for( int x=0; x<MATRIX_WIDTH; ++x ){
            MATRIX[y][x] = 0;
        }        
    }
    filledPercentage = 0.0f;
}


