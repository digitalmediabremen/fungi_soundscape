#include "ofApp.h"

#define MAX_GENERATIONS 8
#define SIDE 16

//--------------------------------------------------------------
void ofApp::setup(){
    
    //------------------- graphics --------------
    ofBackground(0);
    ofSetFrameRate(60);
    ofDisableAntiAliasing();
    brightColor = ofColor( 255, 255, 255);
    darkColor = ofColor( 35, 255, 35);
    ofSetWindowTitle( "read mushroom" );
    caHeight = SIDE*MAX_GENERATIONS;
    
    //----------------- ---------------------
    // Setting up sequencer
    engine.sequencer.setTempo(70.0f);
        
    // ----------- PATCHING -----------
    
    // loads reverb impulse response
    reverb.loadIR(ofToDataPath( "kingtubby-fl1.wav" ));
    
    zaps.setup(NUMSYNTHS);
    scopes.resize(NUMSYNTHS + 2);
    
    for ( int i=0; i<NUMSYNTHS; ++i ) {
        wolframSeq.out_trig(i) >> zaps.voices[i]; // patch the sequence outputs to the zaps
        zaps.voices[i] >> scopes[i] >> engine.blackhole();
    }
    
    
    zaps.fader.ch(0) >> engine.audio_out(0);
    zaps.fader.ch(1) >> engine.audio_out(1);   
    
    // patch the zaps to the reverb input 
    float revGain = -60.0f; // -65dB, this IRs are very loud
    zaps.fader.ch(0) * dB(revGain) >> reverb.ch(0);
    zaps.fader.ch(1) * dB(revGain) >> reverb.ch(1);
    
    // patch the reverb to an high pass filter and then to the engine
    // ( deactivated on rPi as the processor is too slow for IR convolution using FFT )
#ifndef __ARM_ARCH
    reverb.ch(0) >> revCut.ch(0) >> engine.audio_out(0);
    reverb.ch(1) >> revCut.ch(1) >> engine.audio_out(1);
    100.0f >> revCut.in_freq(); // 100hz, we cut the reverb muddy low end 
#endif

    // connect the zaps to the stereo delay
    zaps.fader.ch(0) >> dub.ch(0);
    zaps.fader.ch(1) >> dub.ch(1);
                        dub.ch(0) >> engine.audio_out(0);
                        dub.ch(1) >> engine.audio_out(1);
                        dub.ch(0) * dB(12.0f) >> scopes[NUMSYNTHS]   >> engine.blackhole();
                        dub.ch(1) * dB(12.0f) >> scopes[NUMSYNTHS+1] >> engine.blackhole();

     
    // ------------ GUI ------------
    gui.setup("", "config.xml", ofGetWidth()-220, 40);
    gui.setName( "read mush" );
    gui.add( wolframSeq.parameters );
    gui.add( zaps.parameters );
    gui.add( dub.parameters );
    mushroomType.set("Mushroom genus", "Agaricus");
    gui.add(mushroomType);
    
    // listen via class method
    mushroomType.addListener(this, &ofApp::onChangeMushroomGenus);

    
    //---------------------- audio setup -------------
    engine.listDevices();
    engine.setDeviceID(1); // <--- remember to set your index
    engine.setup( 44100, 512, 3);
    
    
    fillMatrix();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);

    if (contoursImage != NULL) {
            ofPushMatrix();
               ofTranslate(650, 30);
               //ofScale(0.4,0.4,0.4);
               currentImage.draw(0, 0, 100, 100);
           ofPopMatrix();
        ofPushMatrix();
            ofTranslate(350, 30);
            //ofScale(0.4,0.4,0.4);
            grayImage.draw(0, 0, 100, 100);
        ofPopMatrix();
        
        ofPushMatrix();
            ofTranslate(350,250);
            ofScale(0.2,0.2,0.2);
            contoursImage->draw(0,0, 500, 500);
        ofPopMatrix();
        
        if (contourFinder.nBlobs > 0) {
            ofPushMatrix();
            ofPushStyle();
                ofTranslate(650,250);
                ofScale(0.1,0.1,0.1);
            ofSetLineWidth(1);

            for( int i=0; i<(int)contourFinder.blobs.size(); i++ ) {
                ofNoFill();
                ofBeginShape();
                for( int j=0; j<contourFinder.blobs[i].nPts; j++ ) {
                    ofVertex( contourFinder.blobs[i].pts[j].x, contourFinder.blobs[i].pts[j].y );
                }
                ofEndShape();
            }
            ofPopStyle();
            ofPopMatrix();
        }

    }
    
    
    gui.draw();

    ofPushMatrix();
    ofTranslate( 0, 0 );
    wolframSeq.draw( SIDE, 120, brightColor, darkColor );
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


bool ofApp::processImage(ofImage * img) {
    if (!img || !img->isAllocated())
    {
        ofLog() << "img null";
        return false;
    }
    // image to bits
     ofxCvColorImage colorImg;
    
    int width = img->getWidth();
    int height = img->getHeight();
    
    colorImg.allocate(width, height);
    grayImage.allocate(width,height);
    
    colorImg.setFromPixels(img->getPixels());
    grayImage = colorImg; // convert our color image to a grayscale image
    grayImage.brightnessContrast(1,1.5);
    grayImage.blurGaussian(5);
    grayImage.adaptiveThreshold(10);
    //grayImage.threshold(30);

    //grayImage.threshold(30);
    //grayImage.contrastStretch();
    grayImage.dilate();
    grayImage.dilate();
    grayImage.erode();
    grayImage.erode();

    grayImage.invert();

    int totalArea = grayImage.width*grayImage.height;
   int minArea = totalArea * 0.02;
   int maxArea = totalArea * 0.80;
   int nConsidered = 5;

    contourFinder.findContours(grayImage, minArea, maxArea, nConsidered, false, false);
    
    generatePattern(grayImage);
    return true;
}

void ofApp::generatePattern(ofxCvGrayscaleImage grayImage) {
    int dataWidth = contourFinder.getWidth();
    int dataHeight = contourFinder.getHeight();
    
    int w = CA_WIDTH;
    int h = CA_HEIGHT;

    int wMultiplier = dataWidth/w;
    int hMultiplier = dataHeight/h;

    
    ofPixels pix;
    contoursImage = new ofxCvGrayscaleImage();

    if (contourFinder.nBlobs > 0) {
        contoursFbo.allocate(dataWidth, dataHeight, GL_RED);
            
        contoursFbo.begin();
        ofPushMatrix();
        ofPushStyle();
            ofClear(0);
            ofSetColor(255);
            ofSetLineWidth(700);
             for( int i=0; i<(int)contourFinder.blobs.size(); i++ ) {
               ofNoFill();
               ofBeginShape();
               for( int j=0; j<contourFinder.blobs[i].nPts; j++ ) {
                   ofVertex( contourFinder.blobs[i].pts[j].x, contourFinder.blobs[i].pts[j].y );
               }
               ofEndShape();
           }
        ofPopStyle();
        ofPopMatrix();
        contoursFbo.end();
        
        pix.allocate(dataWidth, dataHeight, OF_IMAGE_GRAYSCALE); /// <-- manually set this.
        contoursFbo.readToPixels(pix);

        ofxCvGrayscaleImage temp;
        temp.setFromPixels(pix);
        contoursImage->allocate(w, h);
    
        //colorImg.setFromPixels(pix);
        contoursImage->scaleIntoMe(temp);
        contoursImage->blur(1);
        contoursImage->contrastStretch();
        
        } else {
            contoursImage->allocate(w, h);
            contoursImage->scaleIntoMe(grayImage, CV_INTER_CUBIC);
        }
        

       // contoursImage.contrastStretch();
         
        ofLog() << "num channels" << ofToString(pix.getNumChannels());
        /*
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                ofColor thisPixColor;
                if (contourFinder.nBlobs > 0) {
                    thisPixColor = contoursImage.getPixels().getColor(i*wMultiplier, j*hMultiplier);
                } else {
                    thisPixColor = grayImage.getPixels().getColor(i*wMultiplier,j*hMultiplier);
                }

                pattern->setColor(i % w, j % h, thisPixColor);
            }
        }
        pattern->update();
     */
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
    fillMatrix();
}

void ofApp::fillMatrix() {
    vector<string> imageUrls = imageProvider.fetchImages(mushroomType.get());
       if (imageUrls.size() > 0) {
           ofImage * downloadedFungus;
           ofLog () << imageUrls[0];
           downloadedFungus = imageProvider.fetchImage(imageUrls[0]);
           int tries = 0;
           while (!downloadedFungus && tries < imageUrls.size() - 1) {
               ofLog () << "failed to get image, try next one";
               tries++;
               downloadedFungus = imageProvider.fetchImage(imageUrls[tries]);
           }
           if (downloadedFungus) {
               processImage(downloadedFungus);
               wolframSeq.setImage(contoursImage);
               currentImage = *downloadedFungus;
           }
       }
}

