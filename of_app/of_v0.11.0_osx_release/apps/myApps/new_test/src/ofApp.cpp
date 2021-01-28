#include "ofApp.h"

#define SIDE 16

#define MAX_PITCH 87

//--------------------------------------------------------------
void ofApp::setup(){
    
    //------------------- graphics --------------
    ofBackground(0);
    ofSetFrameRate(60);
    ofDisableAntiAliasing();

    brightColor = ofColor( 255, 255, 255);
    darkColor = ofColor( 35, 255, 35);
    
    ofSetWindowTitle( "read mushroom" );
    
    //----------------- ---------------------
    // Setting up sequencer
    engine.sequencer.setTempo(100.0f);
        
    // ----------- PATCHING -----------
    
    // loads reverb impulse response
    reverb.loadIR(ofToDataPath( "kingtubby-fl1.wav" ));
    
    //zaps.setup(NUMSYNTHS);
    synth.setup(NUMSYNTHS);
    
    scopes.resize(NUMSYNTHS + 2);
    
    for ( int i=0; i<NUMSYNTHS; ++i ) {
        // customSequencer.out_trig(i) >> zaps.voices[i]; // patch the sequence outputs to the zaps
        //zaps.voices[i] >> scopes[i] >> engine.blackhole();
        customSequencer.out_trig(i) >> synth.voices[i]; // patch the sequence outputs to the zaps
        synth.voices[i] >> scopes[i] >> engine.blackhole();

    }
    
    
    // zaps.fader.ch(0) >> engine.audio_out(0);
    // zaps.fader.ch(1) >> engine.audio_out(1);
    
    synth.ch(0) >> engine.audio_out(0);
    synth.ch(1) >> engine.audio_out(1);
    
    float reverbGain = -30.0f; // -65dB, this IRs are very loud // THIS MAKE S A CRAZY DIFFERENCE, use 0.0f for crazy
    //zaps.fader.ch(0) * dB(revGain) >> reverb.ch(0);
    //zaps.fader.ch(1) * dB(revGain) >> reverb.ch(1);
    
    synth.ch(0) * dB(reverbGain) >> reverb.ch(0);
    synth.ch(1) * dB(reverbGain) >> reverb.ch(1);
    
    // patch the reverb to an high pass filter and then to the engine
    // ( deactivated on rPi as the processor is too slow for IR convolution using FFT )
#ifndef __ARM_ARCH
    reverb.ch(0) >> revCut.ch(0) >> engine.audio_out(0);
    reverb.ch(1) >> revCut.ch(1) >> engine.audio_out(1);
    100.0f >> revCut.in_freq(); // 100hz, we cut the reverb muddy low end 
#endif

    // connect the zaps to the stereo delay
    // zaps.fader.ch(0) >> dub.ch(0);
    // zaps.fader.ch(1) >> dub.ch(1);
    synth.ch(0) >> dub.ch(0);
    synth.ch(1) >> dub.ch(1);
                        dub.ch(0) >> engine.audio_out(0);
                        dub.ch(1) >> engine.audio_out(1);
                        dub.ch(0) * dB(12.0f) >> scopes[NUMSYNTHS]   >> engine.blackhole();
                        dub.ch(1) * dB(12.0f) >> scopes[NUMSYNTHS+1] >> engine.blackhole();

     
    // ------------ GUI ------------
    gui.setup("", "config.xml", ofGetWidth()-220, 40);
    gui.setName( "read mush" );
    mushroomType.set("Mushroom genus", "Agaricus");
    gui.add(mushroomType);
    gui.add( customSequencer.parameters );
    //gui.add( zaps.parameters );
    gui.add( synth.ui );
    gui.add( dub.parameters );

    
    // listen via class method
    mushroomType.addListener(this, &ofApp::onChangeMushroomGenus);

    
    //---------------------- audio setup -------------
    engine.listDevices();
    engine.setDeviceID(1); // <--- remember to set your index
    engine.setup( 44100, 512, 3);
    
        
    ofAddListener(imageProvider.completedGetImageURLs,this,&ofApp::onReceivedImageUrls);
    ofAddListener(imageProvider.completedDownloadImage,this,&ofApp::onCompletedImageDownload);
    ofAddListener(imageProvider.failedEvent,this,&ofApp::onFailedToReceiveImagesURL);

    imageProvider.fetchImages(mushroomType.get());
}

//--------------------------------------------------------------
void ofApp::update(){
    int curStep = customSequencer.currentStep();

    for ( int i=0; i<NUMSYNTHS; ++i ) {
        float value = customSequencer.getStepFloat(curStep, i);
        if (value) {
            float pitch = (value) * (float)MAX_PITCH; // max pitch 80
            /*
            if (pitch >= 75.0f) {
                // chose max pitch from scale, because it's the contour
                float akebono[] = { 72.0f, 74.0f, 75.0f, 79.0f, 80.0f, 84.0f, 86.0f, 87.0f }; //
                
                pitch = akebono[int(ofRandom(8))] - 20;
            }
            */
             
            //zaps.voices[i].pitchControl.set(pitch);
            pitch >> synth.voices[i].in("pitch");
        }
    }
    
    if (customSequencer.stepsSinceChange >= MATRIX_HEIGHT - 20) { // start to fetch next shrooms.
        customSequencer.stepsSinceChange = -100;
        imageProvider.fetchImages("Amanita");
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    imageProcessor.draw();
    
    gui.draw();

    ofPushMatrix();
    ofTranslate( 0, 0 );
    customSequencer.draw( SIDE, 120, brightColor, darkColor );
    ofPopMatrix();
    
    // draw the scopes
    ofPushMatrix();
        ofTranslate( 20, 370 );
        ofSetColor( brightColor );
        
        for(int y=0; y<2; ++y){
            for( int x=0; x<3; ++x){
                int i = x+y*3;
                string label;
                switch (i){
                    case 0: case 1: case 2: case 3:
                        label = "synth " + ofToString(i);
                    break;
                    
                    case 4: label = "delay L"; break;
                    case 5: label = "delay R"; break;
                }
                int xpos = x*347;
                int ypos = y*120;
                ofDrawBitmapString( label, xpos, ypos);
                scopes[i].draw( xpos, ypos + 10, 330, 80);
            }
        }
    ofPopMatrix();

}



//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

void ofApp::onChangeMushroomGenus(string& ){
    ofLog () << "changed: " << mushroomType.get();
    imageProvider.fetchImages(mushroomType.get());
}

void ofApp::onReceivedImageUrls(vector<string> & images) {
    if (images.size() > 0) {
      string imageURL = images[(int)ofRandom(images.size()-1)];
     // ofImage * downloadedFungus;
      ofLog () << images[0];
      imageProvider.fetchImage(imageURL);
     /*
      int tries = 0;
      while (!downloadedFungus && tries < images.size() - 1) {
          ofLog () << "failed to get image, try next one";
          tries++;
          imageURL = images[tries];
          downloadedFungus = imageProvider.fetchImage(imageURL);
      }
      if (downloadedFungus) {
          customSequencer.setImage(imageProcessor.processImage(downloadedFungus));
      }
     */
  }
}

void ofApp::onFailedToReceiveImagesURL(string & error) {
    ofLogError() << "failed to fetch, response: " << error;
    imageProvider.fetchImages("Cortinarius");
}

void ofApp::onCompletedImageDownload () {
    ofImage * downloadedFungus;
    downloadedFungus = imageProvider.lastLoadedImage;
    if (downloadedFungus) {
        customSequencer.setImage(imageProcessor.processImage(downloadedFungus));
    }
}
