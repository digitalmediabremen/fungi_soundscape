
#include "ofApp.h"
#define WIDTH 320
#define HEIGHT 240

//--------------------------------------------------------------
void ofApp::setup(){
    
    
    ofSetWindowTitle("data to waveform");
        
    pallete.push_back(0x264653);
    pallete.push_back(0x2a9d8f);
    pallete.push_back(0xe9c46a);
    pallete.push_back(0xf4a261);
    pallete.push_back(0xe76f51);

    
    col = 160;            // col for getting pixels to wave
    mode = 0;
    
    // image
    imagePattern.allocate(WIDTH, HEIGHT, GL_RGB);

    waveplot.allocate(HEIGHT*2 + 4, 170);
    
    //patching-------------------------------
    keyboard.setPolyMode( 8 );

    int voicesNum = keyboard.getVoicesNumber();
    
    synth.datatable.setup( HEIGHT, HEIGHT );
    //synth.datatable.smoothing(0.5f);

    synth.setup( voicesNum );
    
    setupSequencer();

    
    for(int i=0; i<voicesNum; ++i){
        keyboard.out_trig(i)  >> synth.voices[i].in("trig");
        keyboard.out_pitch(i) >> synth.voices[i].in("pitch");
        
        seq.out_trig("kick") >> synth.voices[i].in("kick");
        
        seq.out_trig("gate") >> synth.voices[i].in("trig3");
        seq.out_value("pitch") >> synth.voices[i].in("pitch3");
    }
    
    // patch synth to the engine
    synth.ch(0) >> engine.audio_out(0);
    synth.ch(1) >> engine.audio_out(1);

    // graphic setup---------------------------
    ofSetVerticalSync(true);
    ofDisableAntiAliasing();
    ofBackground(0);
    ofSetColor(ofColor(0,100,100));
    ofNoFill();
    ofSetLineWidth(1.0f);
    ofSetFrameRate(24);

    // GUI -----------------------------------
    gui.setup("", "settings.xml", 10, 0);
    gui.add( synth.ui );
    gui.add( smooth.set("wave smoothing", 0.0f, 0.0f, 0.95f) );
    smooth.addListener(this, &ofApp::smoothCall );
    smooth.set(0.3f);
    
    // audio / midi setup----------------------------
#ifdef USE_MIDI_KEYBOARD
    midiIn.listPorts();
    midiIn.openPort(0);
    engine.addMidiController( keyboard, midiIn );
#endif
    engine.listDevices();
    engine.setDeviceID(1); // my sound output index
    engine.setup( 44100, 512, 3);
    
    generatePattern();

}

//--------------------------------------------------------------
void ofApp::update(){
    
    // generating the image
    ofPixels pixels;
    imagePattern.readToPixels(pixels);
        
    //convert data to a waveform in real time

    switch( mode ){
        case 0: // converting pixels to waveform samples
            synth.datatable.begin();
            for(int n=0; n<HEIGHT; ++n){
                float sample = ofMap(pixels.getData()[col*3 + channel + n*3*WIDTH], 0, 255, -0.5f, 0.5f);
                synth.datatable.data(n, sample);
            }
            synth.datatable.end(false);
        break; // remember, raw waveform could have DC offsets, we have filtered them in the synth using an hpf
        
        case 1: // converting pixels to partials for additive synthesis
            synth.datatable.begin();
            for(int n=0; n<HEIGHT; ++n){
                float partial = ofMap(pixels.getData()[col*3 + channel + n*3*WIDTH], 0, 255, 0.0f, 1.0f);
                synth.datatable.data(n, partial);
            }
            synth.datatable.end(true);
        break;
    }
    
    // ----------------- PLOTTING THE WAVEFORM ---------------------
    waveplot.begin();
    ofClear(0, 0, 0, 0);
    
    ofSetColor(255);
    ofDrawRectangle(1, 1, waveplot.getWidth()-2, waveplot.getHeight()-2);
    ofTranslate(2, 2);
    switch( mode ){
        case 0: // plot the raw waveforms
            ofBeginShape();
            for(int n=0; n<HEIGHT; ++n){
                float y = ofMap(pixels.getData()[col*3 + channel + n*3*WIDTH], 0, 255, HEIGHT, 0);
                ofVertex( n*2, y );
            }
            ofEndShape();
        break;
        
        case 1: // plot the partials
            for(int n=0; n<HEIGHT; ++n){
                float partial = ofMap(pixels.getData()[col*3 + channel + n*3*WIDTH], 0, 255, 0.0f, 1.0f);
                int h = waveplot.getHeight() * partial;
                int y = waveplot.getHeight() - h;
                ofDrawLine(n*2, y, n*2, HEIGHT );
            }
        break;
    }
    waveplot.end();
}

//--------------------------------------------------------------
void ofApp::draw(){

    // image generator
    
    imagePattern.draw(0,0);

    // draw GUI
    gui.draw();
    
    ofPushMatrix();
    // draw some keyboard keys and infos
    switch(channel){
        case 0: ofSetColor(255, 0, 0); break;
        case 1: ofSetColor(0, 255, 0); break;
        case 2: ofSetColor(0, 0, 255); break;
        default: break;
    }
    
    string info = "press spacebar to change color channel\n";
    info += "datatable mode (press m to change): ";
    switch(mode){
        case 0: info+="raw waveform\n"; break;
        case 1: info+="additive synthesis\n"; break;
        default: break;
    }
    info += "click on the webcam image to change aquired pixel line";
    
    ofTranslate( 0, HEIGHT );
    ofDrawBitmapString(info, 0, 10);

    ofTranslate( 0, 250 );
    waveplot.draw(0, 0);
    ofPopMatrix();

#ifndef USE_MIDI_KEYBOARD
    keyboard.draw( 10, 350, 200, 110);
#endif

    ofSetColor(255);
    ofDrawLine(  col, 0, col, HEIGHT );
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
#ifndef USE_MIDI_KEYBOARD
    // sends key messages to ofxPDSPComputerKeyboard
    keyboard.keyPressed( key );
#endif
    switch(key){
        case ' ':
            channel++;
            if(channel==3) channel = 0;
        break;
        case 'm': case 'M':
            mode++;
            if(mode==2) mode = 0;
        break;
    }
}

void ofApp::generatePattern() {
    
    imagePattern.begin();
    ofClear(0,0,0, 0);
    imagePattern.end();

    /*
        imagePattern.begin();

        ofSetColor(24,50,24, 255);
        ofFill();
        ofDrawRectangle(0,120,WIDTH,HEIGHT/2);
        ofSetColor(120,10,254, 255);
        ofDrawRectangle(10,50,WIDTH/3,HEIGHT/5);
        imagePattern.end();

     */
    imagePattern.begin();

    
    for (int y = -HEIGHT/2; y < HEIGHT; y += HEIGHT / 7) {
       // int color = pallete[ofRandom(pallete.size())];

        ofColor c = ofColor(ofRandom(0,255), ofRandom(0, 255), ofRandom(0,255));     ofSetColor(c);
        ofFill();
        ofPath p = ofPath();
        p.setStrokeColor(c);
        p.setFilled(true);
        p.setStrokeWidth(100);

          for (int x = -100; x <= WIDTH+100; x+=3) {
            int yy = y + ofMap(ofNoise(y, x / 2, ofGetFrameNum() * 0000.1 / 5), 0, 0.5, -HEIGHT / 10, HEIGHT / 10);
            p.lineTo(x, yy);
          }
          //p.lineTo(WIDTH+200, HEIGHT);
          //p.lineTo(0-200, HEIGHT);
        p.draw();
    }
    imagePattern.end();
}


void ofApp::setupSequencer () {
    engine.sequencer.setTempo( 108 );
    
    // pdsp::Function can be used to make generative sequencers
    
    // this assignable function is executed each 16th
    seq.code = [&]() noexcept {
        // synth -------
        if( seq.chance( 0.5f ) ){
            seq.send("gate", 1.0f ); // sends note on to "gate" out
        }else{
            seq.send("gate", 0.0f ); // note off
        }
       
        static float akebono[] = { 72.0f, 74.0f, 75.0f, 79.0f, 80.0f, 84.0f, 86.0f, 87.0f }; // akebono scale
        float p = akebono[ seq.dice( 8 ) ]; // random note from scale
     
        seq.send( "pitch", p - 20.0f );
        
        // kick -------
        const float x = 1.0f;
        const float o = 0.0f;
        static float ks[] = { x,x,x,o, o,o,o,o, x,o,o,o, o,o,o,o };
        
        // frame returns the playback in bars multiplied for timing
        // as int, use modulo operations for getting the step
        seq.send( "kick", ks[seq.frame()%16] );
    };
    

    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == OF_KEY_RETURN) {
        engine.sequencer.play();
        generatePattern();
    }
#ifndef USE_MIDI_KEYBOARD
    // sends key messages to ofxPDSPComputerKeyboard
    keyboard.keyReleased( key );
#endif

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    if( x< WIDTH && y<HEIGHT){
        col = x;
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if( x< WIDTH && y<HEIGHT){
        col = x;
    }
}

//--------------------------------------------------------------
void ofApp::smoothCall( float & value ) {
    synth.datatable.smoothing( value  );
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){}
//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){}
//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){}
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){}
