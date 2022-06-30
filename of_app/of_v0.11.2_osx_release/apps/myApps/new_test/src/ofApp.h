#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
#include "CustomSequencer.h"
#include "ofxGui.h"
//#include "Zaps.h"
#include "Chorus.h"
#include "SinePercussion.h"
#include "KarplusStrong.h"

#include "StereoDelay.h"
#include "ofxOpenCv.h"
#include "APIService.h"
#include "ImageProcessor.h"
#include "PolySynth.h"
#include "Fungus.h"
#include "ofxParagraph.h"
#include "ofxCsv.h"

#define MAX_BPM 100
#define MIN_BPM 60
#define ABSOLUTE_MAX_PITCH 100
#define ABSOLUTE_MIN_PITCH 40

#include "ofxJSONRPC.h"


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
        ofxCsv csv;

        // pdsp modules
        pdsp::Engine   engine; 
             
        CustomSequencer customSequencer;
        vector<string> locationsIds;

        // vector<np::synth::SinePercussion> drums;

        // np::effect::Chorus chorus;

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
        ofParameter<string> locationSearch;
        ofParameter<string> mushroomId;

        void onChangeMushroomGenus(string&);
        void onChangeLocationSearch(string&);
        void onChangeMushroomID(string&);
        ImageProcessor imageProcessor;
        APIService apiService;
        
        void onReceiveObservations();
        void onReceiveCoordinates();
        void onFailedToReceiveImagesURL(string & error);
        void onCompletedImageDownload();
    
        Fungus * currentFungus;
    
        float calculateTempo(float confidence);
        int calculateReadHeight(float percentage);
    
        ofSoundStream soundstream;
        
        bool isFullscreen;
        int calculateMaxPitch(float percentage);
            int maxPitch;
        
        void customizeSequencer();
        
        int stringSynthNum;
        
        ofxParagraph* paragraph;
        ofxParagraph* locationParagraph;

        ofxParagraph* name;
        ofxParagraph* idString;
        ofxParagraph* dataParagraph;
        ofxParagraph* latLngParagraph;

        ofImage map;
        
        ofVec2f calculateMapPosition(float lat, float lng, float width, float height);
        
        
        // Registered methods.
        void sendCommand(ofx::JSONRPC::MethodArgs& args);

        void onReceivedCommand(ofx::JSONRPC::MethodArgs& args);
        /// \brief The server that handles the JSONRPC requests.
        ofx::HTTP::JSONRPCServer server;
    
        string STATE;
};



