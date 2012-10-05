//
//  ofxMapamok.cpp
//  mapping
//
//  Created by ignacio pedreira gonzalez on 03/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "ofxMapamok.h"

//using namespace ofxCv;
//using namespace cv;

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
    
    objName = "mapamok";
    bEditMode = false;
    
    init(0,0,800,600);
}

//  ------------------------------------------ MAIN LOOP

void ofxMapamok::update(){
	if(selectionMode) {
		cam.enableMouseInput();
	} else {
		
        // generate camera matrix given aov guess
        // float aov = getf("aov");
        cv::Size2i imageSize(ofGetWidth(), ofGetHeight());
        float f = imageSize.width * ofDegToRad(aov); // i think this is wrong, but it's optimized out anyway
        cv::Point2f c = cv::Point2f(imageSize) * (1. / 2);
        cv::Mat1d cameraMatrix = (cv::Mat1d(3, 3) <<
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
        
        
        vector<cv::Mat> rvecs, tvecs;
        cv::Mat distCoeffs;
        vector<vector<cv::Point3f> > referenceObjectPoints(1);
        vector<vector<cv::Point2f> > referenceImagePoints(1);
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
            modelMatrix = ofxCv::makeMatrix(rvec, tvec);
            calibrationReady = true;
        } else {
            calibrationReady = false;
        }
        
		cam.disableMouseInput();
	}
}

// ------------------------------------------- RENDER

void ofxMapamok::draw(ofTexture &texture){
    if(selectionMode) {
		drawSelectionMode(texture);
	} else {
		drawRenderMode(texture);
	}
}

void ofxMapamok::drawLabeledPoint(int label, ofVec2f position, ofColor color, ofColor bg, ofColor fg) {
    
    if(!inside(position)) return;
    
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_DEPTH_TEST);
	ofVec2f tooltipOffset(5, -25);
	ofSetColor(color);
	float w = 40;
	float h = 40;
    
	ofSetLineWidth(lineWidth);
	ofLine(position - ofVec2f(w,0), position + ofVec2f(w,0));
	ofLine(position - ofVec2f(0,h), position + ofVec2f(0,h));
	ofCircle(position, selectedPointSize);
    ofxCv::drawHighlightString(ofToString(label), position + tooltipOffset, bg, fg);
	glPopAttrib();
}

void ofxMapamok::drawSelectionMode(ofTexture &texture) {
	ofSetColor(255);
	cam.begin( (ofRectangle)*this );
    
    if(showAxis){
        ofDrawGrid(100);
    }
	ofScale(scale, scale, scale);
	
	render(texture);
    
	if(setupMode) {
		imageMesh = getProjectedMesh(objectMesh);
	}
	cam.end();
    
    ofPushStyle();
    ofSetColor(255,255,255);
    ofNoFill();
    ofRect( (ofRectangle)*this );
    ofPopStyle();
	
	if(setupMode) {
		// draw all points cyan small
		//glPointSize(screenPointSize);
		//glEnable(GL_POINT_SMOOTH);
        
        //  Hago esto para saber si el vertex esta dentro del viewport
        //
		ofSetColor( ofxCv::cyanPrint );
        ofEnableSmoothing();
        for(int i=0; i< imageMesh.getVertices().size(); i++){
            if( inside(ofVec2f(imageMesh.getVertex(i).x,imageMesh.getVertex(i).y))){
                ofCircle(imageMesh.getVertex(i).x, imageMesh.getVertex(i).y, 2);
            }
        }
        ofDisableSmoothing();
		//imageMesh.drawVertices();
        
		// draw all reference points cyan
		int n = referencePoints.size();
		for(int i = 0; i < n; i++) {
			if(referencePoints[i]) {
				drawLabeledPoint(i, imageMesh.getVertex(i), ofxCv::cyanPrint );
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
			drawLabeledPoint(choice, selected, ofxCv::magentaPrint);
		} else {
            hoverSelected = false;
		}
		
		// draw selected point yellow
		if(selectedVert) {
			int choice = selectionChoice;
			ofVec2f selected = imageMesh.getVertex(choice);
			drawLabeledPoint(choice, selected, ofxCv::yellowPrint, ofColor::white, ofColor::black);
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
		ofxCv::applyMatrix(modelMatrix);
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
				drawLabeledPoint(i, ofxCv::toOf(imagePoints[i]), ofxCv::cyanPrint);
			}
		}
		
		// move points that need to be dragged
		// draw selected yellow
		int choice = selectionChoice;
		if(selectedVert) {
			referencePoints[choice] = true;
            cv::Point2f& cur = imagePoints[choice];
			if(cur == cv::Point2f()) {
				if(calibrationReady) {
					cur = ofxCv::toCv(ofVec2f(imageMesh.getVertex(choice)));
				} else {
					cur = cv::Point2f(ofGetAppPtr()->mouseX, ofGetAppPtr()->mouseY);
				}
			}
		}
		if(dragging) {
            cv::Point2f& cur = imagePoints[choice];
			float rate = ofGetMousePressed(0) ? slowLerpRate : fastLerpRate;
			cur = cv::Point2f(ofLerp(cur.x, ofGetAppPtr()->mouseX, rate), ofLerp(cur.y, ofGetAppPtr()->mouseY, rate));
			drawLabeledPoint(choice, ofxCv::toOf(cur), ofxCv::yellowPrint, ofColor::white, ofColor::black);
			ofSetColor(ofColor::black);
			ofRect( ofxCv::toOf(cur), 1, 1);
		} else if(arrowing) {
            cv::Point2f& cur = imagePoints[choice];
			drawLabeledPoint(choice, ofxCv::toOf(cur), ofxCv::yellowPrint, ofColor::white, ofColor::black);
			ofSetColor(ofColor::black);
			ofRect( ofxCv::toOf(cur), 1, 1);
        } else {
			// check to see if anything is selected
			// draw hover magenta
			float distance;
			ofVec2f selected = ofxCv::toOf(getClosestPoint(imagePoints, ofGetAppPtr()->mouseX, ofGetAppPtr()->mouseY, &choice, &distance));
			if(!ofGetMousePressed() && referencePoints[choice] && distance < selectionRadius) {
				hoverChoice = choice;
                hoverSelected = true;
				drawLabeledPoint(choice, selected, ofxCv::magentaPrint);
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

// ------------------------------------------------------- EVENTS

void ofxMapamok::_mousePressed(ofMouseEventArgs &e){
    selectedVert = hoverSelected;
	selectionChoice = hoverChoice;
	if(selectedVert) {
        dragging = true;
	}
}

void ofxMapamok::_mouseReleased(ofMouseEventArgs &e){
    dragging = false;
    
    if (bEditMode){
        init(x, y, width, height);
    }
}

void ofxMapamok::_keyPressed(ofKeyEventArgs &e){
    if(e.key == OF_KEY_LEFT || e.key == OF_KEY_UP || e.key == OF_KEY_RIGHT|| e.key == OF_KEY_DOWN){
		int choice = selectionChoice;
        arrowing = true;
		if(choice > 0){
            cv::Point2f& cur = imagePoints[choice];
			switch(e.key) {
				case OF_KEY_LEFT: cur.x -= 1; break;
				case OF_KEY_RIGHT: cur.x += 1; break;
				case OF_KEY_UP: cur.y -= 1; break;
				case OF_KEY_DOWN: cur.y += 1; break;
			}
		}
	} else {
		arrowing = false;
	}
	if(e.key == OF_KEY_BACKSPACE) { // delete selected
		if(selectedVert) {
			selectedVert = false;
			int choice = selectionChoice;
			referencePoints[choice] = false;
			imagePoints[choice] = cv::Point2f();
		}
	}
	if(e.key == '\n') { // deselect
		selectedVert = false;
	}
	if(e.key == ' ') { // toggle render/select mode
        selectionMode = !selectionMode;
	}
    if(e.key == 'f'){
        ofToggleFullscreen();
    }
    if(e.key == 'e'){
        bEditMode = !bEditMode;
    }
}


// --------------------------------------------------- LOAD & SAVE

void ofxMapamok::loadShader(string _shader){
    useShader = shader.load(_shader);
}

bool ofxMapamok::loadMesh(string _daeModel, int _textWidth, int _textHeight){
    bool fileLoaded = false;
    
    //  Cargamos el modelo
    //
    ofxAssimpModelLoader model;
	if (model.loadModel(_daeModel)){
        
        fileLoaded = true;
        
        //  Guardamos el nombre para después poder guardar los puntos calibrados como un XML
        //
        modelFile = _daeModel;
        
        objectMesh = model.getMesh(0);
        
        //  Checkeamos si la cantidad de vértices coincide con las coordenadas de textura
        //
        int n = objectMesh.getNumVertices();
        if ( n != objectMesh.getNumTexCoords() ){
            cout << "ERROR: not same amount of texCoords for all vertices" << endl;
        }
        
        //  Ajustamos los coordenadas que estan normalizadas por el tamaño de la textura
        //
        for(int i = 0; i < n; i++){
            float x = objectMesh.getTexCoords()[i].x;
            float y = objectMesh.getTexCoords()[i].y;
            objectMesh.getTexCoords()[i] = ofVec2f( x*_textWidth, y*_textHeight);
        }
        
        //  Creamos y asignamos los valores a los vectores contienen los puntos del modelo y los projectados
        //
        objectPoints.resize(n);
        imagePoints.resize(n);
        referencePoints.resize(n, false);
        for(int i = 0; i < n; i++) {
            objectPoints[i] = ofxCv::toCv(objectMesh.getVertex(i));
        }
        
        //  Vemos si ya posee una calibración previamente realizada guardada dentro del dae
        //
        ofxXmlSettings XML;
        if (XML.loadFile(modelFile)){
            
            if (XML.pushTag("MAPAMOK")){
                int total = XML.getNumTags("point");
                for (int i = 0; i < total; i++) {
                    XML.pushTag("point",i);
                    if ( XML.getValue("calib", 1) ){
                        cv::Point2f& cur = imagePoints[i];
                        referencePoints[i] = true;
                        cur.x = XML.getValue("x", 0.0f);
                        cur.y = XML.getValue("y", 0.0f);
                    }
                    XML.popTag();
                }
            }
            
            XML.popTag();
        }
    }
}

bool ofxMapamok::saveCalibration(string _xmlfile) {
    bool fileSaved = false;
    
    //  Si no se le pasa un .xml guarda la calibración dentro del .dae
    //
    if (_xmlfile == "none")
        _xmlfile == modelFile;
    
    //  Guardamos a nuestro estilo la calibración
    //
    ofxXmlSettings XML;
    if (XML.loadFile(_xmlfile)){
        
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
        
        fileSaved = XML.saveFile(modelFile);
    }
    
    return fileSaved;
}

void ofxMapamok::saveCameraMatrix(){
    posCamara = cam.getGlobalTransformMatrix();
    
    ofFile outFile;
    outFile.open("posCamara.mat", ofFile::WriteOnly, true);
    outFile.write((char*) posCamara.getPtr(), sizeof(float) * 16);
    outFile.close();
}

void ofxMapamok::loadCameraMatrix(){
    ofFile inFile;
    inFile.open("posCamara.mat", ofFile::ReadOnly, true);
    inFile.read((char*) posCamara.getPtr(), sizeof(float) * 16);
    inFile.close();
    cam.setTransformMatrix(posCamara);
}