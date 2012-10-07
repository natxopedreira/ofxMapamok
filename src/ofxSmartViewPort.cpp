//
//  ofxSmartViewPort.cpp
//  TUIOpainter
//
//  Created by Patricio Gonzalez Vivo on 10/5/12.
//
//

#include "ofxSmartViewPort.h"

ofxSmartViewPort::ofxSmartViewPort(){
    objName = "none";
    
    bEditMode = false;
	dragging  = false;
	scaling   = false;
    saveSettingsFile = "viewPort.xml";
    
    //  Eventos de Mouse
    //
    ofAddListener(ofEvents().mousePressed, this, &ofxSmartViewPort::_mousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &ofxSmartViewPort::_mouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &ofxSmartViewPort::_mouseReleased);
    ofAddListener(ofEvents().keyPressed, this, &ofxSmartViewPort::_keyPressed);
    ofAddListener(ofEvents().draw, this, &ofxSmartViewPort::_draw);
}

bool ofxSmartViewPort::loadSettings(string _fileConfig){
    bool fileFound = false;
    
    if (_fileConfig == "none")
    _fileConfig = saveSettingsFile;
    
    ofxXmlSettings XML;
    if (XML.loadFile(_fileConfig)){
        fileFound = true;
        saveSettingsFile = _fileConfig;
        
        x = XML.getValue(objName+":area:x", 0);
        y = XML.getValue(objName+":area:y", 0);
        width = XML.getValue(objName+":area:width", ofGetScreenWidth());
        height = XML.getValue(objName+":area:height", ofGetScreenHeight());
    }
    
    this->init(x, y, width, height);
    
    return fileFound;
}

bool ofxSmartViewPort::saveSettings(string _fileConfig){
    
    bool fileFound = false;
    
    if (_fileConfig == "none")
        _fileConfig = saveSettingsFile;
    
    ofxXmlSettings XML;
    if (XML.loadFile(_fileConfig)){
        
        if ( XML.pushTag(objName) ){

            XML.setValue("area:x", x);
            XML.setValue("area:y", y);
            XML.setValue("area:width", width);
            XML.setValue("area:height", height);
            
            XML.popTag();
            
            fileFound = XML.saveFile();
        }
    }
    
    return fileFound;
}

void ofxSmartViewPort::init(int _x, int _y, int _width, int _height){
    
    if (_x == -1)
        _x = this->x;
    
    if (_y == -1)
        _y = this->y;
    
    if (_width == -1)
        _width = this->width;
    
    if (_height == -1)
        _height = this->height;
    
    //  Setea las propiedades del rectangulo que contiene Graffiti
    //
    ofRectangle::set(_x, _y, _width, _height);
    
}

void ofxSmartViewPort::_draw(ofEventArgs &e){
    //  Draggable area-boxes for Edit modes
    //
	
	
	
    if ( bEditMode ){
		float rate = .6;
		if(dragging){
			x = ofLerp(x, ofGetAppPtr()->mouseX, rate);
			y = ofLerp(y, ofGetAppPtr()->mouseY, rate);
		}
		if(scaling){
			width = ofLerp(width, ofGetAppPtr()->mouseX - x, rate);
			height = ofLerp(height, ofGetAppPtr()->mouseY - y, rate);
			//width += ofGetAppPtr()->mouseX - x - width;
			//height += e.y - y - height;
		}
		ofPushStyle();
        ofPushMatrix();
        ofTranslate(x, y);
        
        //  Graffiti Area
        //
        ofNoFill();
        ofSetColor(245, 58, 135,200);
        ofRect(0,0,width,height);
        ofFill();
        ofSetColor(245, 58, 135,150);
        ofRect(-7,-7,14,14);
        ofRect(width-7,height-7,14,14);
        ofSetColor(245, 58, 135,255);
        ofDrawBitmapString(objName + " Area",15,15);
		ofDrawBitmapString("x:"+ofToString((int)x)+" y:"+ofToString((int)y),15,30);
        ofDrawBitmapString("width:"+ofToString((int)width)+" height:"+ofToString((int)height),15,45);
        ofPopMatrix();
        ofPopStyle();
    }
}

//----------------------------------------------------------- Mouse
void ofxSmartViewPort::_mousePressed(ofMouseEventArgs &e){
    
	if(bEditMode){
		ofPoint mouse = ofPoint(e.x,e.y);
		
		ofPoint A = ofPoint(x,y);
        ofPoint B = ofPoint(x+width,y+height);
		
		
		if ( A.distance(mouse) < 20 ){
			dragging = true;
		}
		if ( B.distance(mouse) < 20 ){
			scaling = true;
		}
		
	}
	
};

void ofxSmartViewPort::_mouseDragged(ofMouseEventArgs &e){
}

void ofxSmartViewPort::_mouseReleased(ofMouseEventArgs &e){
    if (bEditMode){
		if(scaling) scaling = false;
		if(dragging) dragging = false;
        init(x, y, width, height);
    }
}

void ofxSmartViewPort::_keyPressed(ofKeyEventArgs &e){
    if(e.key == 'e'){
        bEditMode = !bEditMode;
    }
}