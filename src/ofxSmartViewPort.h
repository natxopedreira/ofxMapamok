//
//  ofxMapaRect.h
//  TUIOpainter
//
//  Created by Patricio Gonzalez Vivo on 10/5/12.
//
//

#ifndef OFXSMARTVIEWPORT
#define OFXSMARTVIEWPORT

#include "ofMain.h"

#include "ofxXmlSettings.h"

class ofxSmartViewPort : public ofRectangle {
public:
    ofxSmartViewPort();
    
    virtual bool    loadSettings(string _fileConfig = "config.xml");
    virtual bool    saveSettings(string _fileConfig = "config.xml");
    
    virtual void    init(int _x = -1, int _y = -1, int _width = -1, int _height = -1);
 
    bool            bEditMode;

protected:
    virtual void    _mousePressed(ofMouseEventArgs &e) = 0;
    virtual void    _mouseDragged(ofMouseEventArgs &e);
    virtual void    _mouseReleased(ofMouseEventArgs &e);
    virtual void    _keyPressed(ofKeyEventArgs &e) = 0;
    virtual void    _draw(ofEventArgs &e);
    
    string  objName;
};

#endif
