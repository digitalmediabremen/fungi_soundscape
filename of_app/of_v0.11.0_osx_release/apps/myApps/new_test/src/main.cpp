#include "ofMain.h"
#include "ofApp.h"
// #define NANOVG_GL3_IMPLEMENTATION 1

//========================================================================
int main( ){
	ofSetupOpenGL( 1280, 960, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
