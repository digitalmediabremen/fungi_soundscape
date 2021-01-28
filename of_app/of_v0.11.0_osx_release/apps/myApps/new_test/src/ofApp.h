#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
#include "CustomSequencer.h"
#include "ofxGui.h"
#include "Zaps.h"
#include "StereoDelay.h"
#include "ofxOpenCv.h"
#include "ImageProvider.h"
#include "ImageProcessor.h"
#include "PolySynth.h"

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

        void switchRule();
    
        bool DEBUG_MODE = false;

        // pdsp modules
        pdsp::Engine   engine; 
             
        CustomSequencer customSequencer;
        
        //Zaps zaps;
    
        PolySynth synth;

        pdsp::IRVerb reverb;
        
        pdsp::ParameterGain dubSwitch;

        StereoDelay dub;

        pdsp::Sequence     masterplan;
        pdsp::LowCut       revCut;

        ofxPanel gui;
        
        int caHeight;

        ofColor brightColor;
        ofColor darkColor;
        
        std::vector<pdsp::Scope> scopes;
    
        
        ofParameter<string> mushroomType;
        void onChangeMushroomGenus(string&);
    
        ImageProcessor imageProcessor;
        ImageProvider imageProvider;
        
        void onReceivedImageUrls(vector<string> & images);
        void onFailedToReceiveImagesURL(string & error);
        void onCompletedImageDownload();

};



