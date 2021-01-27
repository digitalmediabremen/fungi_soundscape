
#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
#include "flags.h"
#include "BitMatrix.h"
#include "ofxOpenCv.h"

#define NUMSYNTHS 4

class CustomSequencer : public pdsp::Sequence {

public:
    CustomSequencer();

    ofParameterGroup parameters;
    
    ofParameter<int>    readHeight;

    ofParameter<int>     activeOuts;
    ofParameter<int>     steps;

    void draw( int side, int bars_h, ofColor fg, ofColor bg );
    int currentStep() const;
    float getStep( int step, int out ) const;
    float getStepFloat( int step, int out ) const;

    pdsp::SequencerGateOutput& out_trig( int index );
    
    vector<float>   stepbars;
    vector<float>   stepbarsFloat;

    void setImage(ofxCvGrayscaleImage * img);
    pdsp::Function seq;

    
    int stepsSinceChange;
private:
    BitMatrix     bm;
    
    std::atomic<int> meter_step;

};
