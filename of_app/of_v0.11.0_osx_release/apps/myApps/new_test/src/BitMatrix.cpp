
#include "BitMatrix.h"
#include "ofxPDSP.h"

BitMatrix::BitMatrix(){
    clear();
}

void BitMatrix::fill( ofxCvGrayscaleImage img) {
    ofLog() << "filll";
    for( int x=0; x<MATRIX_WIDTH; ++x ){
        for( int y=0; y<MATRIX_HEIGHT; ++y ){
            if (img.getPixels().getColor(x,y).getLightness() > MIN_TO_FILL) {
                float fill = img.getPixels().getColor(x,y).getLightness() / 255.0;
                ofLog() << "filling..: " << ofToString(fill);
                MATRIX[y][x] = fill;
            } else {
                MATRIX[y][x] = 0;
            }
        }
    }
    ofLog() << "foi";

}

void BitMatrix::clear() noexcept{
    for( int y=0; y<MATRIX_HEIGHT; ++y ){
        for( int x=0; x<MATRIX_WIDTH; ++x ){
            MATRIX[y][x] = 0;
        }        
    }    
}


