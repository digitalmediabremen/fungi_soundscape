#include "ofApp.h"

#include <math.h>

//--------------------------------------------------------------
void ofApp::setup(){
    // ofSetDataPathRoot("./"); // if exporting application
    stringSynthNum = 2;
    //------------------- graphics --------------
    ofBackground(0);
    ofSetFrameRate(60);
    // ofDisableAntiAliasing();

    brightColor = ofColor( 255, 255, 255);
    darkColor = ofColor( 35, 255, 35);
    
    ofSetWindowTitle( "read mushroom" );
    
    map.load("map3.png");
    
    // NO_IMAGE
    // SUCCESS
    // NONE
    STATE = "NONE";
    //----------------- ---------------------
    // Setting up sequencer
    engine.sequencer.setTempo(MIN_BPM);
        
    // ----------- PATCHING -----------
    
    // loads reverb impulse response
    reverb.loadIR(ofToDataPath( "kingtubby-fl1.wav" ));
    
    // zaps.setup(NUMSYNTHS);
    synth.setup(NUMSYNTHS);
    //ksynth.setup(1);
    // drums.resize(NUMSYNTHS);

    scopes.resize(NUMSYNTHS + 2);
    
    for ( int i=0; i<NUMSYNTHS; ++i ) {
        //customSequencer.out_trig(i) >> zaps.voices[i]; // patch the sequence outputs to the zaps
        // zaps.voices[i] >> scopes[i] >> engine.blackhole();
        //if (i == stringSynthNum) { // strings in only one synth
            // customSequencer.out_trig(i)  >> ksynth.in_trig( 0 );
        //}
        // midiKeys.outs_pitch[i] >> ksynth.in_pitch( i );
        // customSequencer.out_trig(i) >> drums[i].in("trig");
        // drums[i] * dB(-2.0f) >> engine.audio_out(0);
        // drums[i] * dB(-2.0f) >> engine.audio_out(1);


        customSequencer.out_trig(i) >> synth.voices[i]; // patch the sequence outputs to the zaps
        synth.voices[i] >> scopes[i] >> engine.blackhole();
    }
    
    
    // ksynth.ch(0) >> chorus.ch(0) >> engine.audio_out(0);
    // ksynth.ch(1) >> chorus.ch(1) >> engine.audio_out(1);
    
    // zaps.fader.ch(0) >> engine.audio_out(0);
    // zaps.fader.ch(1) >> engine.audio_out(1);
    
    synth.ch(0) >> engine.audio_out(0);
    synth.ch(1) >> engine.audio_out(1);
    
    float reverbGain = -30.0f; // -65dB, this IRs are very loud // THIS MAKE S A CRAZY DIFFERENCE, use 0.0f for crazy
    // zaps.fader.ch(0) * dB(reverbGain) >> reverb.ch(0);
    // zaps.fader.ch(1) * dB(reverbGain) >> reverb.ch(1);
    
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
    // gui.add( zaps.parameters );
    gui.add( synth.ui );
    gui.add( dub.parameters );

    // gui.add( drums[0].parameters );
    //gui.add( chorus.parameters );
    
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
    ofAddListener(apiService.completedFetchCoordinates,this,&ofApp::onReceiveCoordinates);

    
    maxPitch = 75;
    
    DEBUG_MODE = true;
    isFullscreen = false;

        
    // description
    ofxParagraph* p = new ofxParagraph();
    p->setColor(ofColor(255, 255, 255, 255));
    p->drawBorder(ofColor::fromHex(0x777777));
    p->drawBorder(false);
    p->setAlignment(ofxParagraph::ALIGN_LEFT);
    p->setWidth(320);
    p->setIndent(0);
    p->setLeading(6);

    p->setFont("fonts/Roboto-Medium.ttf", 16);

    paragraph = p;//.push_back(p);
    
    // location
    ofxParagraph* p2 = new ofxParagraph();
    p2->setColor(ofColor(255, 255, 255, 255));
    p2->drawBorder(ofColor::fromHex(0x777777));
    p2->drawBorder(false);
    p2->setAlignment(ofxParagraph::ALIGN_LEFT);
    p2->setWidth(320);
    p2->setIndent(0);
    p->setLeading(6);

    p2->setFont("fonts/Roboto-Medium.ttf", 18);

    locationParagraph = p2;//.push_back(p);
    
    // name
    ofxParagraph* p3 = new ofxParagraph();
    p3->setColor(ofColor(255, 255, 255, 255));
    p3->drawBorder(ofColor::fromHex(0x777777));
    p3->drawBorder(false);
    p3->setAlignment(ofxParagraph::ALIGN_LEFT);
    p3->setWidth(320);
    p3->setIndent(0);
    p3->setLeading(10);
    p3->setFont("fonts/Roboto-Medium.ttf", 20);

    name = p3;//.push_back(p);
    
    // id
    ofxParagraph* p4 = new ofxParagraph();
    p4->setColor(ofColor(255, 255, 255, 255));
    p4->drawBorder(ofColor::fromHex(0x777777));
    p4->drawBorder(false);
    p4->setAlignment(ofxParagraph::ALIGN_LEFT);
    p4->setWidth(320);
    p4->setIndent(0);

    p4->setFont("fonts/Roboto-Medium.ttf", 18);

    idString = p4;//.push_back(p);
    
    // date
    ofxParagraph* p5 = new ofxParagraph();
    p5->setColor(ofColor(255, 255, 255, 255));
    p5->drawBorder(ofColor::fromHex(0x777777));
    p5->drawBorder(false);
    p5->setAlignment(ofxParagraph::ALIGN_LEFT);
    p5->setWidth(320);
    p5->setIndent(0);

    p5->setFont("fonts/Roboto-Medium.ttf", 18);
    
    dataParagraph = p5;
    
    // server
    ofx::HTTP::JSONRPCServerSettings settings;
    settings.setPort(8197);

    // Initialize the server.
    server.setup(settings);
    
    
    server.registerMethod("get-text",
                          "Returns the state to client.",
                          this,
                          &ofApp::sendCommand);
    
    server.registerMethod("set-text",
                            "Send location id to the client",
                            this,
                            &ofApp::onReceivedCommand);
    
    
    server.start();
    
    
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
            /*
            if (i == int(ofRandom(NUMSYNTHS))) { // strings in only one synth
                pitch >> drums[i].in("pitch");

            //    pitch >> ksynth.in_pitch(0 );
            }
            */

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
    
    int topMargin = 30;
    int leftMargin = 20;

    imageProcessor.draw();
    
    // map.draw(leftMargin, 100, 835/2.5, 439/2.5);

    ofPushMatrix();
    ofTranslate( ofGetWidth() - 420, topMargin - 8 );
    customSequencer.draw( 28, 120, brightColor, darkColor );
    ofPopMatrix();
    
    
    // draw the scopes
    ofPushMatrix();
        ofSetColor( brightColor );
    int boxWidth = 331;
    int boxHeight = 100;
    
    int space = 20;
    
    for(int y=0; y<2; ++y){
        for( int x=0; x<3; ++x){
            int i = x+y*3;
            string label;
            switch (i){
                case 0: case 1: case 2: case 3:
                    label = "zap " + ofToString(i);
                break;
                
                case 4: label = "delay L"; break;
                case 5: label = "delay R"; break;
            }
            int xpos = x*(boxWidth + space);
            int ypos = y*(boxHeight + space);
            scopes[i].draw(380 + xpos, ypos + (ofGetHeight() - 280), boxWidth, boxHeight);
        }
    }
    
    
    
    
    

    ofPopMatrix();
    
    if (currentFungus != NULL) {
        ofPushMatrix();
        ofSetColor(255, 255, 255, 255);
        int topStart = topMargin + 20;
        int spaceBetween = 25;
        name->setText(currentFungus->name);
        name->draw(leftMargin, topStart);
        //ofDrawBitmapString( currentFungus->name, leftMargin, topMargin);

        // ofDrawBitmapString( ofToString(currentFungus->id), leftMargin, topMargin + 20);
        idString->setText(ofToString(currentFungus->id));
        idString->draw(leftMargin, topStart + name->getHeight());
        //ofDrawBitmapString( currentFungus->location, leftMargin, 300);

        ofPopMatrix();
        /*
        if (currentFungus->latitude != 0.0) {
            ofPushStyle();
            ofSetColor(0,255,0);
            ofVec2f point = calculateMapPosition(currentFungus->latitude, currentFungus->longitude, 835/2.5, 439/2.5);
            
            // draw little map point
            ofDrawCircle((leftMargin - 8) + point.x, 125 + point.y, 8);
            ofPopStyle();
        }
        */
        locationParagraph->setText(currentFungus->location);
        locationParagraph->draw(leftMargin, spaceBetween + topStart + name->getHeight() + idString->getHeight());
        
        
        dataParagraph->setText(currentFungus->date);
        dataParagraph->draw(leftMargin, spaceBetween + topStart + name->getHeight() + idString->getHeight() + locationParagraph->getHeight());
        
        
        if (currentFungus->description != "null") {
            paragraph->setText(currentFungus->description);
            paragraph->draw(leftMargin, spaceBetween + spaceBetween + topStart + name->getHeight() + idString->getHeight() + dataParagraph->getHeight() + locationParagraph->getHeight());
        } else {
            paragraph->setText("No observation diary found.");
            paragraph->draw(leftMargin, spaceBetween + spaceBetween + topStart + name->getHeight() + idString->getHeight() + dataParagraph->getHeight() + locationParagraph->getHeight());
        }
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
        
        // if (currentFungus->hasLocation) {
        //    apiService.fetchCoordinates(currentFungus->id);
        // }
  }
}

void ofApp::onFailedToReceiveImagesURL(string & error) {
    ofLogError() << "failed to fetch, response: " << error;
    STATE = "NO_IMAGE";
}

void ofApp::onCompletedImageDownload () {
    ofImage * fungusImage;
    fungusImage = apiService.lastLoadedImage;
    if (fungusImage) {
        customSequencer.setImage(imageProcessor.processImage(fungusImage));
    }
    
    customizeSequencer();
    STATE = "SUCCESS";
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

ofVec2f ofApp::calculateMapPosition(float lat, float lng, float width, float height) {
    
    
    // get x
    float x = (lng + 180) * (width / 360);
    // convert from degrees to radians
    float latRad = lat * M_PI / 180;
    // get y value
    float mercN = log(tan((M_PI / 4) + (latRad / 2)));
    float y = (height / 2) - (width * mercN / (2 * M_PI));

    ofVec2f v1; // v1.x is 0, v1.y is 0
    v1.set( x, y ); // now v1.x is 10, v1.y is 50
    return v1;
}


void ofApp::onReceiveCoordinates() {
    ofVec2f coordinates = apiService.lastCoordinates;
    currentFungus->latitude = coordinates.y;
    currentFungus->longitude = coordinates.x;
}


void ofApp::onReceivedCommand(ofx::JSONRPC::MethodArgs& args)
{
    // Set the user text.
    ofLog() << "ID RECEIVED: " << args.params.dump(4);
    string id = args.params.dump(4);
    apiService.fetchObservationsByLocationID(id);

}

void ofApp::sendCommand(ofx::JSONRPC::MethodArgs& args)
{
    ofLog () << "get text called";
    // send
    args.result = STATE;

}
