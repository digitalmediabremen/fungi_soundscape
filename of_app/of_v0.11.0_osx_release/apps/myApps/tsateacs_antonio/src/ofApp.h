#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxPDSP.h"
#include "ofxGui.h"
#include "synth.h"


//#define USE_MIDI_KEYBOARD

class ofApp : public ofBaseApp{

    public:
        void setup();
        void update();
        void draw();
        
        void keyPressed(int key);
        void keyReleased(int key);
        void mouseMoved(int x, int y );
        void mouseDragged(int x, int y, int button);
        void mousePressed(int x, int y, int button);
        void mouseReleased(int x, int y, int button);
        void mouseEntered(int x, int y);
        void mouseExited(int x, int y);
        void windowResized(int w, int h);
        void dragEvent(ofDragInfo dragInfo);
        void gotMessage(ofMessage msg);
        void generatePattern();
        pdsp::Engine   engine;
    

        pdsp::Function          seq;
    void setupSequencer();
        PolySynth                   synth;

#ifdef USE_MIDI_KEYBOARD
        pdsp::midi::Keys        keyboard;
        pdsp::midi::Input       midiIn;
#else
        pdsp::ComputerKeyboard  keyboard;
#endif

        ofxPanel                    gui;

        vector<int> pallete;
        
        int col;
        int channel;
        
        int mode;
        
        ofParameter<float> smooth;
        void smoothCall( float & value );
        
        ofFbo waveplot;
    
        ofFbo imagePattern;
    
        
};
