
#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
#include "ofxOpenCv.h"

#define MWR 8


#define CA_STEPS 16

#define CA_WIDTH 64
#define CA_HEIGHT 64
#define MIN_TO_PLAY 0.7


class Automaton1D  {
    
public:

    Automaton1D();
        
    void random( float density, pdsp::Function & seq ) noexcept;
    void clear() noexcept;

    void advance() noexcept;
    
    void setRule( int rule ) noexcept;
    void fill( ofxCvGrayscaleImage img );

    float CA[CA_HEIGHT][CA_WIDTH];    

private:    
    int rule;
    int wrules [MWR];
    
};


