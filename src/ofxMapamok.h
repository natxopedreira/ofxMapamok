//
//  ofxMapamok.h
//  mapping
//
//  Created by ignacio pedreira gonzalez on 03/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef mapping_ofxMapamok_h
#define mapping_ofxMapamok_h

#include "ofMain.h"

//  Native addons
//
#include "ofxAssimpModelLoader.h"

//  Non native addons
//
#include "ofxCv.h"
#include "ofxXmlSettings.h"

//  Other classes and methots
//
#include "LineArt.h"
#include "ofxProCamToolkit.h"

class ofxMapamok{
public:
    ofxMapamok();
    
    void update();
    void draw(ofTexture &texture);
    void drawRenderMode(ofTexture &texture);
    void drawSelectionMode(ofTexture &texture);
	void render(ofTexture &texture);
    void loadShader(string _shader);
	void drawLabeledPoint(int label, ofVec2f position, ofColor color, ofColor bg = ofColor::black, ofColor fg = ofColor::white);
	void updateRenderMode();
	

    void loadMesh(string _daeModel, int _textWidth, int _textHeight);
    void saveCalibration(string _xmlfile);
    
    void _mousePressed(int x, int y, int button);
    void _mouseReleased(int x, int y, int button);
    void _keyPressed(int key);
    
    //  Axis cam
    //
    void drawGrid(float scale, float ticks, bool labels, bool x, bool y, bool z);
    void drawGridPlane(float scale, float ticks, bool labels);
    
    //  Objects
    //
    ofxAssimpModelLoader model;
    ofEasyCam               cam;
	ofVboMesh               objectMesh;
	ofMesh                  imageMesh;
    int textWidth, textHeight, lineWidth, drawMode, selectionChoice, hoverChoice, selectionRadius, screenPointSize, aov, selectedPointSize;
    float slowLerpRate, fastLerpRate, scale;
    ofColor faceColor;
	
	vector<cv::Point3f>     objectPoints;
	vector<cv::Point2f>     imagePoints;
	vector<bool> referencePoints;
    
    cv::Mat rvec, tvec;
	ofMatrix4x4 modelMatrix;
	ofxCv::Intrinsics intrinsics;
	bool calibrationReady, selectionMode, useSmoothing, setupMode, selectedVert, dragging, arrowing, hoverSelected, showAxis, useShader;
	
    string  modelFile;
	Poco::Timestamp lastFragTimestamp, lastVertTimestamp;
	ofShader shader;
    
};
#endif
