#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    ofEnableSmoothing();
    ofEnableAlphaBlending();
    ofBackground(0, 0, 0);
    
    textWidth = 800;
    textHeight = 600;
    
    
    //  Load two MapaMoks
    //
    mapamoko.loadSettings("mapamok.xml");
    mapamoko.loadMesh("model.dae", textWidth, textHeight);
    mapamoko.drawMode = DRAW_FACES;
    
    //  Setup the canvas
    //
    canvas.allocate(textWidth, textHeight);
    canvas.begin();
    ofClear(0);
    canvas.end();
    
    //  Init WebCam texture
    //
    video.initGrabber(640,480);
    videoViewPort.loadSettings("video.xml");
    
    bCanvasMode = false;
}

//--------------------------------------------------------------
void testApp::update(){
    video.update();
    
    //  Render the canvas
    //
    canvas.begin();
    ofClear(0);
    video.draw(videoViewPort);
    canvas.end();
    
    if (!bCanvasMode){
        mapamoko.update();
    }
    
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
}

//--------------------------------------------------------------
void testApp::draw(){
    
    if (bCanvasMode){
        ofSetColor(255);
        canvas.draw(0, 0);
        ofDrawBitmapString("CANVAS MODE ( press 'e' to drag )", 15,15);
        
    } else {
        
        //
        //  Para editar el viewPort tiene q apretar 'E' as’ no se solapa con el resto
        //
        
        mapamoko.draw( &canvas.getTextureReference() );
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key == 'f'){
        
        //  Toggle Fullscreen
        //
        ofToggleFullscreen();
    } else if(key == 'c'){
        
        //  Canvas Mode to edit what is proyected over the surface
        //
        bCanvasMode = !bCanvasMode;
    } else if (key == 's'){
        
        //  Save All View Ports positions
        //
        videoViewPort.saveSettings();
        mapamoko.saveSettings();
    } else if (key == 'S'){
        
        //  Save Calibrations
        //
        mapamoko.saveCalibration();
    }
}
//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}