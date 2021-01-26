
#include "WolframSeq.h"

WolframSeq::WolframSeq(){

    parameters.setName( "wolfram seq" );
    parameters.add( this->rule.set( "rule", 60, 0, 150 ));
    parameters.add( this->steps.set( "steps", 16, 0, 16 ));
    parameters.add( this->activeOuts.set( "active outs", NUMSYNTHS, 0, NUMSYNTHS ));
    parameters.add( this->threshold.set( "threshold", 4, 0, 8 ));
    parameters.add( this->density.set( "seeds density", 0.5f, 0.0f, 1.0f ));
    parameters.add( this->regenerate.set( "regenerate", true ));

    ruleMem = -1;
    
    stepbars.resize( CA_WIDTH );
    
    seq.code = [&]() noexcept {
        
        if( seq.frame()%steps == 0 ){
            if( ruleMem!= rule ){
                setRule( rule );
                ruleMem = rule;
            }
            
            if(regenerate){
                ca.random( density, seq );
                regenerate = false;
            }else{
                ca.advance();
            }            

            for(int x=0; x < CA_WIDTH; ++x){
                int sum = 0;
                sum += ca.CA[CA_HEIGHT - 1][x];
                sum += ca.CA[CA_HEIGHT - 2][x];
                sum += ca.CA[CA_HEIGHT - 3][x];
                sum += ca.CA[CA_HEIGHT - 4][x];
                sum += ca.CA[CA_HEIGHT - 5][x];
                sum += ca.CA[CA_HEIGHT - 6][x];
                sum += ca.CA[CA_HEIGHT - 7][x];
                sum += ca.CA[CA_HEIGHT - 8][x];
                
                int sect = x / steps;
                float value = 0.0f;
                            
                if( sect <NUMSYNTHS ){
                    sum -= threshold;
                    int range = CA_HEIGHT - threshold;
           
                    if( sum>0 && range>0 ){
                        value = float( sum ) / float ( range );
                    }              
                }

                stepbars[x] = value;
            }             
        }
    
        int s = seq.frame() % steps;
        meter_step = s;
        
        for(int o=0; o < activeOuts; ++o) { 
            float outval = stepbars[ (o*steps) + s ];
            seq.send( o, outval );
        }
    };    
}

pdsp::SequencerGateOutput& WolframSeq::out_trig( int index ){
    return seq.out_trig( index );
}

int WolframSeq::currentStep() const {
    return meter_step;
}

float WolframSeq::getStep( int step, int out ) const{
    return stepbars[ step + out*steps ];
}

void WolframSeq::setImage( ofImage img ) {
    ca.fill(img);
}

void WolframSeq::draw( int ca_side, int bars_h, ofColor fg, ofColor bg ){
    ofPushMatrix();
    ofScale(0.3,0.3,0.3);
    ofSetColor( fg );
    
    for( int y=0; y<CA_HEIGHT; ++y ){
        for( int x=0; x<CA_WIDTH; ++x ){
            if( ca.CA[y][x] ){
                ofFill();
            }else{
                ofNoFill();
            }
            ofDrawRectangle( x*ca_side, y*ca_side, ca_side, ca_side );
        }
    }


    ofTranslate( 0, ca_side * (CA_HEIGHT+1));
    
    ofSetColor( bg );
    for( int x=0; x<CA_WIDTH; ++x ){
        if( x%steps == meter_step && x<activeOuts*steps ){
            ofSetColor( fg );
        }else{
            ofSetColor( bg );
        }
        ofFill();
        ofDrawRectangle( ca_side*x, bars_h, ca_side, -(stepbars[x]*bars_h) );

        ofNoFill();
        ofDrawRectangle( ca_side*x, 0, ca_side, bars_h );
    }
        
    ofTranslate( 0, bars_h );
    
    ofSetColor( fg );

    int sw = ca_side*steps;
    float soff = meter_percent() * sw;
    int sh = ca_side;
    for( int i=0; i<NUMSYNTHS; ++i ){
        int sx = sw*i;
        ofNoFill();
        ofDrawRectangle( sx, 0, sw, sh );
        ofDrawLine( sx + soff, 0, sx+soff, sh );
    }

    ofPopMatrix();
}

