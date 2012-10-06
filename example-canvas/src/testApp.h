#pragma once

#include "ofMain.h"

#include "ofxMapamok.h"
#include "ofxSmartViewPort.h"

class testApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    
    ofxMapaMok      mapamoko;
    
    ofVideoGrabber      video;
    ofxSmartViewPort    videoViewPort;
    
    ofFbo           canvas;
    
    int             textWidth;
    int             textHeight;
    
    bool            bCanvasMode;
};
