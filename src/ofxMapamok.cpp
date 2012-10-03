//
//  ofxMapamok.cpp
//  mapping
//
//  Created by ignacio pedreira gonzalez on 03/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "ofxMapamok.h"

using namespace ofxCv;
using namespace cv;

ofxMapamok::ofxMapamok(){
    lineWidth = 1;
    drawMode = 2;
    selectionRadius = 10;
    screenPointSize = 2;
    selectedPointSize = 3;
    aov = 80;
    
    slowLerpRate = .001;
    fastLerpRate = 1.;
    scale = 1.;
    
    selectedVert = false;
    hoverSelected = false;
    dragging = false;
    useSmoothing = false;
    arrowing = false;
    
    setupMode = true;
    selectionMode = true;
    showAxis = true;
    
    useShader = false;
    
}
void ofxMapamok::loadShader(string _shader){
   useShader = shader.load(_shader);
}
void ofxMapamok::update(){
	if(selectionMode) {
		cam.enableMouseInput();
	} else {
		updateRenderMode();
		cam.disableMouseInput();
	}
}
void ofxMapamok::updateRenderMode() {
	// generate camera matrix given aov guess
	//float aov = getf("aov");
	Size2i imageSize(ofGetWidth(), ofGetHeight());
	float f = imageSize.width * ofDegToRad(aov); // i think this is wrong, but it's optimized out anyway
	Point2f c = Point2f(imageSize) * (1. / 2);
	Mat1d cameraMatrix = (Mat1d(3, 3) <<
                          f, 0, c.x,
                          0, f, c.y,
                          0, 0, 1);
    
	// generate flags
	int flags =
    CV_CALIB_USE_INTRINSIC_GUESS |
    //cvCALIB_FIX_PRINCIPAL_POINT |
    CV_CALIB_FIX_ASPECT_RATIO |
    CV_CALIB_FIX_K1 |
    CV_CALIB_FIX_K2 |
    CV_CALIB_FIX_K3 |
    CV_CALIB_ZERO_TANGENT_DIST;
    
	
	vector<Mat> rvecs, tvecs;
	Mat distCoeffs;
	vector<vector<Point3f> > referenceObjectPoints(1);
	vector<vector<Point2f> > referenceImagePoints(1);
	int n = referencePoints.size();
	for(int i = 0; i < n; i++) {
		if(referencePoints[i]) {
			referenceObjectPoints[0].push_back(objectPoints[i]);
			referenceImagePoints[0].push_back(imagePoints[i]);
		}
	}
	const static int minPoints = 6;
	if(referenceObjectPoints[0].size() >= minPoints) {
		calibrateCamera(referenceObjectPoints, referenceImagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, flags);
		rvec = rvecs[0];
		tvec = tvecs[0];
		intrinsics.setup(cameraMatrix, imageSize);
		modelMatrix = makeMatrix(rvec, tvec);
		calibrationReady = true;
	} else {
		calibrationReady = false;
	}
}
void ofxMapamok::drawLabeledPoint(int label, ofVec2f position, ofColor color, ofColor bg, ofColor fg) {
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_DEPTH_TEST);
	ofVec2f tooltipOffset(5, -25);
	ofSetColor(color);
	float w = ofGetWidth();
	float h = ofGetHeight();
	ofSetLineWidth(lineWidth);
	ofLine(position - ofVec2f(w,0), position + ofVec2f(w,0));
	ofLine(position - ofVec2f(0,h), position + ofVec2f(0,h));
	ofCircle(position, selectedPointSize);
	drawHighlightString(ofToString(label), position + tooltipOffset, bg, fg);
	glPopAttrib();
}
void ofxMapamok::draw(ofTexture &texture){
    if(selectionMode) {
		drawSelectionMode(texture);
	} else {
		drawRenderMode(texture);
	}
}

void ofxMapamok::_mousePressed(int x, int y, int button){
	selectedVert = hoverSelected;
	selectionChoice = hoverChoice;
	if(selectedVert) {
        dragging = true;
	}
}

void ofxMapamok::_mouseReleased(int x, int y, int button){
    dragging = false;
}

void ofxMapamok::_keyPressed(int key) {
	if(key == OF_KEY_LEFT || key == OF_KEY_UP || key == OF_KEY_RIGHT|| key == OF_KEY_DOWN){
		int choice = selectionChoice;
        arrowing = true;
		if(choice > 0){
			Point2f& cur = imagePoints[choice];
			switch(key) {
				case OF_KEY_LEFT: cur.x -= 1; break;
				case OF_KEY_RIGHT: cur.x += 1; break;
				case OF_KEY_UP: cur.y -= 1; break;
				case OF_KEY_DOWN: cur.y += 1; break;
			}
		}
	} else {
		arrowing = false;
	}
	if(key == OF_KEY_BACKSPACE) { // delete selected
		if(selectedVert) {
			selectedVert = false;
			int choice = selectionChoice;
			referencePoints[choice] = false;
			imagePoints[choice] = Point2f();
		}
	}
	if(key == '\n') { // deselect
		selectedVert = false;
	}
	if(key == ' ') { // toggle render/select mode
        selectionMode = !selectionMode;
	}
    if(key == 'f'){
        ofToggleFullscreen();
    }
    
}

void ofxMapamok::drawSelectionMode(ofTexture &texture) {
	ofSetColor(255);
	cam.begin();
    
    if(showAxis){
        ofDrawGrid(100);
    }
	ofScale(scale, scale, scale);
	
	render(texture);
    
	if(setupMode) {
		imageMesh = getProjectedMesh(objectMesh);
	}
	cam.end();
	
	if(setupMode) {
		// draw all points cyan small
		glPointSize(screenPointSize);
		glEnable(GL_POINT_SMOOTH);
		ofSetColor(cyanPrint);
		imageMesh.drawVertices();
        
		// draw all reference points cyan
		int n = referencePoints.size();
		for(int i = 0; i < n; i++) {
			if(referencePoints[i]) {
				drawLabeledPoint(i, imageMesh.getVertex(i), cyanPrint);
			}
		}
		
		// check to see if anything is selected
		// draw hover point magenta
		int choice;
		float distance;
		ofVec3f selected = getClosestPointOnMesh(imageMesh, ofGetAppPtr()->mouseX, ofGetAppPtr()->mouseY, &choice, &distance);
		if(!ofGetMousePressed() && distance < selectionRadius) {
			hoverChoice = choice;
			hoverSelected = true;
			drawLabeledPoint(choice, selected, magentaPrint);
		} else {
            hoverSelected = false;
		}
		
		// draw selected point yellow
		if(selectedVert) {
			int choice = selectionChoice;
			ofVec2f selected = imageMesh.getVertex(choice);
			drawLabeledPoint(choice, selected, yellowPrint, ofColor::white, ofColor::black);
		}
	}
}
void ofxMapamok::drawRenderMode(ofTexture &texture) {
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	if(calibrationReady) {
		intrinsics.loadProjectionMatrix(10, 2000);
		applyMatrix(modelMatrix);
		render(texture);
		if(setupMode) {
			imageMesh = getProjectedMesh(objectMesh);
		}
	}
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	if(setupMode) {
		// draw all reference points cyan
		int n = referencePoints.size();
		for(int i = 0; i < n; i++) {
			if(referencePoints[i]) {
				drawLabeledPoint(i, toOf(imagePoints[i]), cyanPrint);
			}
		}
		
		// move points that need to be dragged
		// draw selected yellow
		int choice = selectionChoice;
		if(selectedVert) {
			referencePoints[choice] = true;
			Point2f& cur = imagePoints[choice];
			if(cur == Point2f()) {
				if(calibrationReady) {
					cur = toCv(ofVec2f(imageMesh.getVertex(choice)));
				} else {
					cur = Point2f(ofGetAppPtr()->mouseX, ofGetAppPtr()->mouseY);
				}
			}
		}
		if(dragging) {
			Point2f& cur = imagePoints[choice];
			float rate = ofGetMousePressed(0) ? slowLerpRate : fastLerpRate;
			cur = Point2f(ofLerp(cur.x, ofGetAppPtr()->mouseX, rate), ofLerp(cur.y, ofGetAppPtr()->mouseY, rate));
			drawLabeledPoint(choice, toOf(cur), yellowPrint, ofColor::white, ofColor::black);
			ofSetColor(ofColor::black);
			ofRect(toOf(cur), 1, 1);
		} else if(arrowing) {
			Point2f& cur = imagePoints[choice];
			drawLabeledPoint(choice, toOf(cur), yellowPrint, ofColor::white, ofColor::black);
			ofSetColor(ofColor::black);
			ofRect(toOf(cur), 1, 1);
        } else {
			// check to see if anything is selected
			// draw hover magenta
			float distance;
			ofVec2f selected = toOf(getClosestPoint(imagePoints, ofGetAppPtr()->mouseX, ofGetAppPtr()->mouseY, &choice, &distance));
			if(!ofGetMousePressed() && referencePoints[choice] && distance < selectionRadius) {
				hoverChoice = choice;
                hoverSelected = true;
				drawLabeledPoint(choice, selected, magentaPrint);
			} else {
				hoverSelected = false;
			}
		}
	}
}
void ofxMapamok::render(ofTexture &texture){

    ofPushStyle();
	ofSetLineWidth(lineWidth);
	if(useSmoothing) {
		ofEnableSmoothing();
	} else {
		ofDisableSmoothing();
	}
	
	ofSetColor(255);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
    if(useShader) {
		shader.begin();
		shader.setUniform1f("elapsedTime", ofGetElapsedTimef());
        shader.setUniformTexture("tex0", texture, 0);
		shader.end();
	}

    
	switch(drawMode) {
		case 0: // faces
			//if(useShader) shader.begin();
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
            texture.bind();
			objectMesh.drawFaces();
            texture.unbind();
			//if(useShader) shader.end();
			break;
		case 1: // fullWireframe
			if(useShader) shader.begin();
			objectMesh.drawWireframe();
			if(useShader) shader.end();
			break;
		case 2: // outlineWireframe
            LineArt::draw(objectMesh, true, faceColor, useShader ? &shader : NULL);
			break;
		case 3: // occludedWireframe
            LineArt::draw(objectMesh, false, faceColor, useShader ? &shader : NULL);
			break;
	}
	glPopAttrib();
	ofPopStyle();
}
void ofxMapamok::loadMesh(string _daeModel, int _textWidth, int _textHeight){
    /// cargamos el modelo
    
    modelFile = _daeModel;
	model.loadModel(modelFile);
	objectMesh = model.getMesh(0);
	int n = objectMesh.getNumVertices();
    if ( n != objectMesh.getNumTexCoords() ){
        cout << "ERROR: not same amount of texCoords for all vertices" << endl;
    }
    
    for(int i = 0; i < n; i++){
        float x = objectMesh.getTexCoords()[i].x;
        float y = objectMesh.getTexCoords()[i].y;
        objectMesh.getTexCoords()[i] = ofVec2f( x*_textWidth, y*_textHeight);
    }
    
	objectPoints.resize(n);
	imagePoints.resize(n);
	referencePoints.resize(n, false);
	for(int i = 0; i < n; i++) {
		objectPoints[i] = toCv(objectMesh.getVertex(i));
	}
    
    /// intenta buscar la calibracion en el dae
    ofxXmlSettings XML;
    if (XML.loadFile(modelFile)){
        if (XML.pushTag("MAPAMOK")){
            
            int total = XML.getNumTags("point");
            for (int i = 0; i < total; i++) {
                XML.pushTag("point",i);
                if ( XML.getValue("calib", 1) ){
                    Point2f& cur = imagePoints[i];
                    referencePoints[i] = true;
                    cur.x = XML.getValue("x", 0.0f);
                    cur.y = XML.getValue("y", 0.0f);
                    // cout << "Point " << i << " loaded at " << cur << endl;
                }
                XML.popTag();
            }
        }
        
        XML.popTag();
    }
}
void ofxMapamok::saveCalibration(string _folder) {
	/*
     entiendo que esto ya no es necesario porque no lo estamos usando
    //  Create a folder to store the calibration files
    //
	ofDirectory dir(_folder);
	dir.create();
	
	FileStorage fs(ofToDataPath(_folder + "calibration-advanced.yml"), FileStorage::WRITE);
    
	Mat cameraMatrix = intrinsics.getCameraMatrix();
	fs << "cameraMatrix" << cameraMatrix;
	double focalLength = intrinsics.getFocalLength();
	fs << "focalLength" << focalLength;
	Point2d fov = intrinsics.getFov();
	fs << "fov" << fov;
	Point2d principalPoint = intrinsics.getPrincipalPoint();
	fs << "principalPoint" << principalPoint;
	cv::Size imageSize = intrinsics.getImageSize();
	fs << "imageSize" << imageSize;
	fs << "translationVector" << tvec;
	fs << "rotationVector" << rvec;
	Mat rotationMatrix;
	Rodrigues(rvec, rotationMatrix);
	fs << "rotationMatrix" << rotationMatrix;
	double rotationAngleRadians = norm(rvec, NORM_L2);
	double rotationAngleDegrees = ofRadToDeg(rotationAngleRadians);
	Mat rotationAxis = rvec / rotationAngleRadians;
	fs << "rotationAngleRadians" << rotationAngleRadians;
	fs << "rotationAngleDegrees" << rotationAngleDegrees;
	fs << "rotationAxis" << rotationAxis;
	
	ofVec3f axis(rotationAxis.at<double>(0), rotationAxis.at<double>(1), rotationAxis.at<double>(2));
	ofVec3f euler = ofQuaternion(rotationAngleDegrees, axis).getEuler();
	Mat eulerMat = (Mat_<double>(3,1) << euler.x, euler.y, euler.z);
	fs << "euler" << eulerMat;
	ofFile basic("calibration-basic.txt", ofFile::WriteOnly);
	ofVec3f position( tvec.at<double>(1), tvec.at<double>(2));
	basic << "position (in world units):" << endl;
	basic << "\tx: " << ofToString(tvec.at<double>(0), 2) << endl;
	basic << "\ty: " << ofToString(tvec.at<double>(1), 2) << endl;
	basic << "\tz: " << ofToString(tvec.at<double>(2), 2) << endl;
	basic << "axis-angle rotation (in degrees):" << endl;
	basic << "\taxis x: " << ofToString(axis.x, 2) << endl;
	basic << "\taxis y: " << ofToString(axis.y, 2) << endl;
	basic << "\taxis z: " << ofToString(axis.z, 2) << endl;
	basic << "\tangle: " << ofToString(rotationAngleDegrees, 2) << endl;
	basic << "euler rotation (in degrees):" << endl;
	basic << "\tx: " << ofToString(euler.x, 2) << endl;
	basic << "\ty: " << ofToString(euler.y, 2) << endl;
	basic << "\tz: " << ofToString(euler.z, 2) << endl;
	basic << "fov (in degrees):" << endl;
	basic << "\thorizontal: " << ofToString(fov.x, 2) << endl;
	basic << "\tvertical: " << ofToString(fov.y, 2) << endl;
	basic << "principal point (in screen units):" << endl;
	basic << "\tx: " << ofToString(principalPoint.x, 2) << endl;
	basic << "\ty: " << ofToString(principalPoint.y, 2) << endl;
	basic << "image size (in pixels):" << endl;
	basic << "\tx: " << ofToString(principalPoint.x, 2) << endl;
	basic << "\ty: " << ofToString(principalPoint.y, 2) << endl;
	
	saveMat(Mat(objectPoints), _folder + "objectPoints.yml");
	saveMat(Mat(imagePoints), _folder + "imagePoints.yml");
    */
    ofxXmlSettings XML;
    
    if (XML.loadFile(modelFile)){
        
        if (!XML.tagExists("MAPAMOK")){
            XML.addTag("MAPAMOK");
        }
        
        XML.pushTag("MAPAMOK");
        
        //  How much there are
        //
        int totalPoints = XML.getNumTags("point");
        
        //  Create the need ones
        //
        for(int i = totalPoints; i < referencePoints.size(); i++){
            XML.addTag("point");
        }
        
        //  Fill everything with data
        //
        for(int i = 0; i < referencePoints.size(); i++){
            XML.pushTag("point",i);
            
            XML.setValue("id", i);
            XML.setValue("calib", referencePoints[i]);
            XML.setValue("x", imagePoints[i].x );
            XML.setValue("y", imagePoints[i].y );
            XML.popTag();
        }
        
        XML.popTag();
        
        XML.saveFile(modelFile);
    }
    
    
}
