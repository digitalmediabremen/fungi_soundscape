#include "ofMain.h"
#include "ofApp.h"
// #define NANOVG_GL3_IMPLEMENTATION 1

//========================================================================
int main( ){
    ofGLFWWindowSettings glWindowSettings;
    // Uncomment the following line to use the programmable gl renderer.
    //glWindowSettings.setGLVersion(3, 2);

    glWindowSettings.setSize(1920,1200);
    glWindowSettings.windowMode = OF_FULLSCREEN;// OF_WINDOW



    //try changing some of the following. The values shown are the defaults
    glWindowSettings.visible = true;
    glWindowSettings.iconified = false;
    glWindowSettings.decorated = true;
    glWindowSettings.resizable = true;
    glWindowSettings.monitor = 1;
    glWindowSettings.multiMonitorFullScreen = false;


        ofCreateWindow(glWindowSettings);
        
        
        ofRunApp( new ofApp());


}
