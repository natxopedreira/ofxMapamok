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
    
    virtual bool    loadSettings(string _fileConfig = "none");
    virtual bool    saveSettings(string _fileConfig = "none");
    
    virtual void    init(int _x = -1, int _y = -1, int _width = -1, int _height = -1);
 
    string          objName;
    bool            bEditMode;
	bool			dragging;
	bool			scaling;

protected:
    virtual void    _mousePressed(ofMouseEventArgs &e);
    virtual void    _mouseDragged(ofMouseEventArgs &e);
    virtual void    _mouseReleased(ofMouseEventArgs &e);
    virtual void    _keyPressed(ofKeyEventArgs &e);
    virtual void    _draw(ofEventArgs &e);
    
    string          saveSettingsFile;
};

#endif
