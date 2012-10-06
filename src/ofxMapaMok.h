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

enum SetupMode{
    SETUP_NONE,
    SETUP_SELECT,
    SETUP_CALIBRATE
};

enum DrawMode{
    DRAW_FACES,
    DRAW_FULL_WIREFRAME,
    DRAW_OUTLINE_WIREFRAME,
    DRAW_OCCLUDED_WIREFRAME
};

enum RefMode{
    REFERENCE_NONE,
    REFERENCE_AXIS,
    REFERENCE_GRID
};

class ofxMapaMok : public ofxSmartViewPort {
public:
    ofxMapaMok();
    
    bool loadMesh(string _daeModel, int _textWidth, int _textHeight);
    
    bool loadCalibration(string _xmlfile );
    bool saveCalibration(string _xmlfile = "none"); // IF NONE will save it inside the _daeModel
    
    void loadCameraMatrix(string _file = "camera.mat");
    void saveCameraMatrix(string _file = "camera.mat");
    
    void linkShader(ofShader *_shader){ shader = _shader; }
    
    void update();
    
    void draw(ofTexture *_texture = NULL);
	
    //  Properties
    //
    RefMode     refMode;
    DrawMode    drawMode;
    SetupMode   setupMode;
    
    ofColor     faceColor;
    
    float       slowLerpRate,
                fastLerpRate,
                scale;
    
    int         textWidth,
                textHeight,
                lineWidth,
                selectionRadius,
                screenPointSize,
                selectedPointSize;
	
private:
    void    _mousePressed(ofMouseEventArgs &e);
    void    _mouseReleased(ofMouseEventArgs &e);
    void    _keyPressed(ofKeyEventArgs &e);
    
    void    drawLabeledPoint(int label, ofVec2f position, ofColor color, ofColor bg = ofColor::black, ofColor fg = ofColor::white);
    void    render(ofTexture *_texture = NULL);
    
    ofEasyCam           cam;
	ofVboMesh           objectMesh;
	ofMesh              imageMesh;
    
    ofShader            *shader;
    
    Poco::Timestamp     lastFragTimestamp,
    lastVertTimestamp;
    
    cv::Mat             rvec,tvec;
	ofMatrix4x4         modelMatrix;
    ofMatrix4x4         posCamara;
	
    ofxCv::Intrinsics   intrinsics;
    
    vector<cv::Point3f> objectPoints;
	vector<cv::Point2f> imagePoints;
	vector<bool>        referencePoints;
    
    string  modelFile;
    
    int     selectionChoice,
            hoverChoice,
            aov;
    
    bool    calibrationReady,
            selectedVert,
            dragging,
            arrowing,
            hoverSelected;
};
#endif
