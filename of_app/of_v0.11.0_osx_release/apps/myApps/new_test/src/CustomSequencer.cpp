
#include "CustomSequencer.h"

CustomSequencer::CustomSequencer(){

    parameters.setName( "matrix" );
    parameters.add( this->steps.set( "steps", 16, 0, 16 ));
    parameters.add( this->activeOuts.set( "active outs", NUMSYNTHS, 0, NUMSYNTHS ));
    parameters.add( this->readHeight.set( "readHeight", 12, 1, 48 ));
    
    stepbars.resize( MATRIX_WIDTH );
    stepbarsFloat.resize( MATRIX_WIDTH );

    seq.code = [&]() noexcept {
        
        if( seq.frame()%steps == 0 ){
            stepsSinceChange++;
            ofLog() << stepsSinceChange;
            
            // advance matrix 1 bit height
            bm.step();
            bool isEmpty = true;
            for(int x=0; x < MATRIX_WIDTH; ++x){
                int sum = 0; // int play only contours, float plays all image;
                float floatSum = 0.0f;
                floatSum += bm.MATRIX[MATRIX_HEIGHT - 1][x];

                for (int a = 1; a < readHeight; a++) {
                    sum += bm.MATRIX[MATRIX_HEIGHT - a][x];
                }
 

                int sect = x / steps;
                float value = 0.0f;

                if( sect <NUMSYNTHS ){
                     int range = MATRIX_HEIGHT - readHeight;
                     if( sum>  0){
                         isEmpty = false;
                         value = float( sum ) / float ( range );
                     }
                }

                stepbars[x] = value;
                stepbarsFloat[x] = floatSum;
                            
            }
            
            if (isEmpty) {
                stepsSinceChange+=2;
                bm.step();
                bm.step();
            }
            
        }
    
        int s = seq.frame() % steps;
        meter_step = s;
        
        for(int o=0; o < activeOuts; ++o) { 
            float outval = stepbars[ (o*steps) + s ] * 2.0f; // hack force
            seq.send( o, outval );
        }
    };    
}


pdsp::SequencerGateOutput& CustomSequencer::out_trig( int index ){
    return seq.out_trig( index );
}

int CustomSequencer::currentStep() const {
    return meter_step;
}

float CustomSequencer::getStep( int step, int out ) const{
    return stepbars[ step + out*steps ];
}

float CustomSequencer::getStepFloat( int step, int out ) const{
    return stepbarsFloat[ step + out*steps ];
}


void CustomSequencer::setImage( ofxCvGrayscaleImage * img ) {
    if (!img) {
        return;
    }
    stepsSinceChange = 0;
    bm.fill(*img);
}

void CustomSequencer::draw( int side, int bars_h, ofColor fg, ofColor bg ){
    ofPushMatrix();
    ofScale(0.3,0.3,0.3);
    ofSetColor( fg);

    for( int y=0; y<MATRIX_HEIGHT; ++y ){
        for( int x=0; x<MATRIX_WIDTH; ++x ){
            if( bm.MATRIX[y][x] > MIN_TO_FILL ){
                ofSetColor( bm.MATRIX[y][x] * 255,bm.MATRIX[y][x] * 255,bm.MATRIX[y][x] * 255, bm.MATRIX[y][x] * 255);

                ofFill();
            }else{
                ofSetColor( bg);
                ofNoFill();
            }
            ofDrawRectangle( x*side, y*side, side, side );
        }
    }


    ofTranslate( 0, side * (MATRIX_HEIGHT+1));
    
    ofSetColor( bg );
    for( int x=0; x<MATRIX_WIDTH; ++x ){
        if( x%steps == meter_step && x<activeOuts*steps ){
            ofSetColor( fg );
        }else{
            ofSetColor( bg );
        }
        ofFill();
        ofDrawRectangle( side*x, bars_h, side, -(stepbars[x]*bars_h) );

        ofNoFill();
        ofDrawRectangle( side*x, bars_h, side, -((side*readHeight)+bars_h));
    }
        
    ofTranslate( 0, bars_h );
    
    ofSetColor( fg );

    int sw = side*steps;
    float soff = meter_percent() * sw;
    int sh = side;
    for( int i=0; i<NUMSYNTHS; ++i ){
        int sx = sw*i;
        ofNoFill();
        ofDrawRectangle( sx, 0, sw, sh );
        ofDrawLine( sx + soff, 0, sx+soff, sh );
    }

    ofPopMatrix();
}


float CustomSequencer::getMatrixFilledPercentage() {
    return bm.filledPercentage;
}
