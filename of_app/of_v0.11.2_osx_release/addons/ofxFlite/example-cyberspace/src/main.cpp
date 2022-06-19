#include "ofMain.h"
#include "ofApp.h"

#include "flite.h" 
extern "C"
{ // c declares
	cst_voice* register_cmu_us_kal(); // you can change this to select a different voice

}




//========================================================================
int main( ){
	ofSetupOpenGL(1024,600,OF_WINDOW);
	ofRunApp(new ofApp());


	cst_voice *v;
	flite_init();
	v=register_cmu_us_kal();
	flite_text_to_speech("hello world, how are you?",v,"play");
	return 0;
}
