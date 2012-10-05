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

// Classes
//
#include "LineArt.h"
#include "ofxProCamToolkit.h"
#include "ofxSmartViewPort.h"

//  Native addons
//
#include "ofxAssimpModelLoader.h"

//  Non native addons
//
#include "ofxCv.h"
#include "ofxXmlSettings.h"

class ofxMapamok : public ofxSmartViewPort {
public:
    ofxMapamok();
    
    bool loadMesh(string _daeModel, int _textWidth, int _textHeight);
    bool saveCalibration(string _xmlfile = "none");
    
    void loadCameraMatrix();
    void saveCameraMatrix();
    
    void loadShader(string _shader);
    
    void update();
    
    void draw(ofTexture &texture);
    void drawRenderMode(ofTexture &texture);
    void drawSelectionMode(ofTexture &texture);
	void drawLabeledPoint(int label, ofVec2f position, ofColor color, ofColor bg = ofColor::black, ofColor fg = ofColor::white);
	
    //  Objects
    //
    ofEasyCam           cam;
	ofVboMesh           objectMesh;
	ofMesh              imageMesh;
    
    ofShader            shader;
    
    string              modelFile;
    
    Poco::Timestamp     lastFragTimestamp,
                        lastVertTimestamp;
    
    cv::Mat             rvec,tvec;
	ofMatrix4x4         modelMatrix;
    ofMatrix4x4         posCamara;
	
    ofxCv::Intrinsics   intrinsics;
    
    vector<cv::Point3f> objectPoints;
	vector<cv::Point2f> imagePoints;
	vector<bool>        referencePoints;
    
    ofColor             faceColor;
    
    float               slowLerpRate,
                        fastLerpRate,
                        scale;
    
    int                 textWidth,
                        textHeight,
                        lineWidth,
                        drawMode,
                        selectionChoice,
                        hoverChoice,
                        selectionRadius,
                        screenPointSize,
                        aov,
                        selectedPointSize;
	
	bool                calibrationReady,
                        selectionMode,
                        useSmoothing,
                        setupMode,
                        selectedVert,
                        dragging,
                        arrowing,
                        hoverSelected,
                        showAxis,
                        useShader;
    
private:
    void    _mousePressed(ofMouseEventArgs &e);
    void    _mouseReleased(ofMouseEventArgs &e);
    void    _keyPressed(ofKeyEventArgs &e);
    
    void    render(ofTexture &texture);
};
#endif
