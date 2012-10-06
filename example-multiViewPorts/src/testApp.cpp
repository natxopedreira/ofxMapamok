#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofBackground(0, 0, 0);
    
    textWidth = 640;
    textHeight = 480;
    
    //  Load two MapaMoks
    //
    mapamoko01.loadSettings("viewport01.xml");
    mapamoko01.loadMesh("model.dae", textWidth, textHeight);
    mapamoko01.drawMode = DRAW_FACES;
    
    mapamoko02.loadSettings("viewport02.xml");
    mapamoko02.loadMesh("model.dae", textWidth, textHeight);
    
    //  Init WebCam texture
    //
    video.initGrabber(textWidth,textHeight);
}

//--------------------------------------------------------------
void testApp::update(){
    video.update();
    
    mapamoko01.update();
    mapamoko02.update();
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
}

//--------------------------------------------------------------
void testApp::draw(){
    //
    //  Para editar el viewPort tiene q apretar 'E' as’ no se solapa con el resto
    //
        
    mapamoko01.draw( &canvas.getTextureReference() );
    mapamoko02.draw( );
            
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key == 'f'){
        
        //  Toggle Fullscreen
        //
        ofToggleFullscreen();
    } else if (key == 's'){
        
        //  Save All View Ports positions
        //
        mapamoko01.saveSettings();
        mapamoko02.saveSettings();
    } else if (key == 'S'){
        
        //  Save Calibrations
        //
        mapamoko01.saveCalibration();
    }
}
//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}