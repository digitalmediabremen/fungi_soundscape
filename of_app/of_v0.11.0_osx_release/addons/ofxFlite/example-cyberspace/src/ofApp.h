#pragma once

#include "ofMain.h"

#include "ofxFlite~.h"



class ofApp : public ofBaseApp{

	public:

		void setup(){

			int sr = 44100;
			int bs = 512;
			float speed = 0.5f;
			float vol = 0.7;

			flite.setup(sr,bs,"..Hello There. Try. Try Pressing the L key, to Load the File onto Network Rom.",speed,vol);

			sound.listDevices();
			sound.setDeviceID(6);
			sound.setup(this, 2, 0, sr, bs, 4);
			ofSetFrameRate(30);
			ofSetWindowTitle("ofxFlite~ cyberspace");

		}

		void update(){

			if(ofGetMousePressed(0)){
				float s = (float)ofGetMouseX() / (ofGetWidth()/2);
				flite.setSpeakSpeed(s);
			}

		}


		// bool togglerec=false;
		// int nvoice = 0;

		void keyPressed(int key){
			if(key=='1'){
				flite.setText("you stupid programmer, that does not work.");
			}
			if(key=='2'){
				flite.setText("1 + 1 + 10000 + 7.34 + 16, that does work");
			}
			if(key=='3'){
				flite.setText("sunshine");
			}
			if(key=='4'){
				flite.setText("mach speed");
			}

			if(key=='l'){
				flite.loadFile("cyberspace.txt");
			}



		}


		void draw(){

			ofSetColor(255);

			ofSetColor( 100, 200, 72);
			flite.draw(100, 400, 25);

			if(flite.isThreadRunning()){
				ofSetColor(0,255,0);
			}else {
				ofSetColor(255,0,0);
			}
			ofRect(50,10,25,25);

		}

		void audioOut(float * outpt, int bufferSize, int nChannels){
			float * audiosys = flite.processBuffer();

			for (int i = 0; i < bufferSize; i++){
				 outpt[i*nChannels    ] = audiosys[i] ;
				 outpt[i*nChannels + 1] =  audiosys[i] ;
			 }

		}



		ofxFlite flite;
		ofSoundStream	sound;

};
