#include "ofApp.h"


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
    engine.sequencer.setTempo(MIN_BPM);
        
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
    mushroomType.set("Mushroom genus search", "");
    locationSearch.set("Location Search", "Germany");
    mushroomId.set("Search by ID", "");
    gui.add(mushroomType);
    gui.add(locationSearch);
    gui.add(mushroomId);
    gui.add( customSequencer.parameters );
    //gui.add( zaps.parameters );
    gui.add( synth.ui );
    gui.add( dub.parameters );

    
    // listen via class method
    mushroomType.addListener(this, &ofApp::onChangeMushroomGenus);
    locationSearch.addListener(this, &ofApp::onChangeLocationSearch);
    mushroomId.addListener(this, &ofApp::onChangeMushroomID);

    
    // get default
    int outputDeviceID = -1;

    for (int i=0; i<soundstream.getDeviceList().size(); ++i) {

            if (soundstream.getDeviceList()[i].isDefaultOutput) {
                  outputDeviceID = i;
                  break;
            }
     }
    ofLog () << "default audio output:" << outputDeviceID;
    
    //---------------------- audio setup -------------
    engine.listDevices();
    engine.setDeviceID(outputDeviceID);
    engine.setup( 44100, 2048, 3);
    
        
    ofAddListener(apiService.completedFetchObservations,this,&ofApp::onReceiveObservations);
    ofAddListener(apiService.completedFetchImage,this,&ofApp::onCompletedImageDownload);
    ofAddListener(apiService.failedEvent,this,&ofApp::onFailedToReceiveImagesURL);

    
    maxPitch = 75;
    
    DEBUG_MODE = true;
    isFullscreen = false;

        
    // texts
    ofxParagraph* p = new ofxParagraph();
    p->setColor(ofColor(255, 255, 255, 255));
    p->drawBorder(ofColor::fromHex(0x777777));
    p->drawBorder(true);
    p->setAlignment(ofxParagraph::ALIGN_LEFT);
    p->setWidth(280);

    p->setFont("fonts/Roboto-Medium.ttf", 9);

    paragraph = p;//.push_back(p);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    int curStep = customSequencer.currentStep();

    for ( int i=0; i<NUMSYNTHS; ++i ) {
        
        // increase base pitch by X
        float value = customSequencer.getStepFloat(curStep, i);
        if (value) {
            float pitch = (value) * (float)maxPitch;
            
            if (pitch < ABSOLUTE_MIN_PITCH) {
                // ofLog () << "min pitch!" << pitch;
                pitch = ofRandom(ABSOLUTE_MIN_PITCH, ABSOLUTE_MIN_PITCH + 5.0f);
            }
            
            
            if (pitch >= maxPitch - 1.0f) { // basically filled bit (1)
                // ofLog () << "max pitch!" << pitch;

                // chose max pitch from scale, because it's the contour
                float difference = ABSOLUTE_MAX_PITCH - maxPitch;
                float akebono[] = { 72.0f - difference, 74.0f - difference, 75.0f - difference, 79.0f - difference, 80.0f - difference, 84.0f - difference, 86.0f - difference, 87.0f - difference }; //
                
                pitch = akebono[int(ofRandom(8))];
            } else {
                // ofLog () << "normal pitch!" << pitch;
            }
             
            
             
            //zaps.voices[i].pitchControl.set(pitch);
            pitch >> synth.voices[i].in("pitch");
        }
    }
    
    if (customSequencer.stepsSinceChange >= MATRIX_HEIGHT - 10) { // start to fetch next shrooms.
        customSequencer.stepsSinceChange = - 100;
        apiService.fetchObservationsByLocation(locationSearch.get());
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    int topMargin = 100;
    
    imageProcessor.draw();
    
    
    ofPushMatrix();
    ofTranslate( 380, topMargin );
    customSequencer.draw( 28, 120, brightColor, darkColor );
    ofPopMatrix();
    
    
    // draw the scopes
    ofPushMatrix();
        ofSetColor( brightColor );
    int boxWidth = 330;
    int boxHeight = 80;
        for(int y=0; y<6; ++y){
            //for( int x=0; x<colums; ++x){
                int i = y;
                /*
                string label;
                switch (i){
                    case 0: case 1: case 2: case 3:
                        label = "synth " + ofToString(i);
                    break;
                    
                    case 4: label = "delay L"; break;
                    case 5: label = "delay R"; break;
                }
                 */
                int xpos = ofGetWidth() - (boxWidth + 20);
                int ypos = y * 110;
                // ofDrawBitmapString( "", xpos, ypos);
                scopes[i].draw( xpos, ypos + topMargin, boxWidth, boxHeight);
            //}
        }
    ofPopMatrix();
    
    if (currentFungus != NULL) {
        ofPushMatrix();
        ofSetColor(255, 255, 255, 255);
        ofDrawBitmapString( ofToString(currentFungus->id), 50, topMargin - 20);

        ofDrawBitmapString( currentFungus->name, 50, topMargin);
        ofDrawBitmapString( currentFungus->location, 50, 50);

        ofPopMatrix();
        paragraph->setText(currentFungus->description);
        paragraph->draw(50, 200);
    }
    
    if (DEBUG_MODE) {
        gui.draw();
    }
}



//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    if (key == ' ') {
        DEBUG_MODE = !DEBUG_MODE;
    }
    
    if (key == 'F') {
        ofSetFullscreen(!isFullscreen);
        isFullscreen = !isFullscreen;
    }
    
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
    apiService.fetchObservationsOfSpecies(mushroomType.get());
}

void ofApp::onChangeLocationSearch(string&) {
    ofLog () << "changed location: " << locationSearch.get();
    apiService.fetchObservationsByLocation(locationSearch.get());
}

void ofApp::onChangeMushroomID(string&) {
    if (mushroomId.get() == "") {
        return;
    }
    ofLog() << "changed to id" << mushroomId.get();
    apiService.fetchObservationByID(mushroomId.get());

}

void ofApp::onReceiveObservations() {
    
    if (apiService.fungiList.size() > 0) {
        currentFungus = apiService.fungiList[(int)ofRandom(apiService.fungiList.size()-1)];
        string imageURL = currentFungus->imageURL;
        
        ofLog () << "FUNGUS ID: "<<ofToString(currentFungus->id);
        // ofImage * downloadedFungus;
        //imageURL = "https://mushroomobserver.org/images/640/928.jpg"; // example of good fallback when no contours (mush is darker, around is brighter)
        //imageURL = "https://mushroomobserver.org/images/640/125451.jpg"; // no contours
        //imageURL = "https://mushroomobserver.org/images/640/4828.jpg"; // good shading debug for low frequencies
        
        // interesting
        //imageURL = "https://mushroomobserver.org/images/640/74243.jpg";
        
        ofLog () << imageURL;

        apiService.fetchImage(imageURL);
  }
}

void ofApp::onFailedToReceiveImagesURL(string & error) {
    ofLogError() << "failed to fetch, response: " << error;
}

void ofApp::onCompletedImageDownload () {
    ofImage * fungusImage;
    fungusImage = apiService.lastLoadedImage;
    if (fungusImage) {
        customSequencer.setImage(imageProcessor.processImage(fungusImage));
    }
    
    customizeSequencer();
}

float ofApp::calculateTempo(float confidence) { // more confidence in identification of fungus, the faster it plays
    return ofMap(confidence, 0.0f, 3.0f, MIN_BPM, MAX_BPM);
}

int ofApp::calculateReadHeight(float percentage) { // more filled matrix needs to be read in smaller parts at the time - vice-versa
    return ofMap(percentage, 0.0f, 0.5f, 50, 10, true);
}

int ofApp::calculateMaxPitch(float percentage) {
    return ofMap(percentage, 0.0f, 0.7f, ABSOLUTE_MAX_PITCH, ABSOLUTE_MIN_PITCH, true); // bigger the fungus, more low values, vice-verse
}

void ofApp::customizeSequencer() {
        // set tempo
       float tempo = calculateTempo(currentFungus->confidence);
       engine.sequencer.setTempo(tempo);
        ofLog () << "TEMPO: " << tempo;
        float matrixFilledPercentage = customSequencer.getMatrixFilledPercentage(); // may correspond to size of fungus
       int readHeight = calculateReadHeight(matrixFilledPercentage);
       customSequencer.readHeight.set(readHeight);
    
    maxPitch = calculateMaxPitch(matrixFilledPercentage);
    ofLog () << "max pitch" << maxPitch;
    
    
    //for ( int i=0; i<NUMSYNTHS; ++i ) {
        // wave index is completely random...
        synth.table_ctrl.set((float)ofRandom(0.0f, 10.0f));

    //}
    
    ofLog() << currentFungus->description;
}

