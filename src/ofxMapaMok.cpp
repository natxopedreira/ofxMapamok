//
//  ofxMapaMok.cpp
//  mapping
//
//  Created by ignacio pedreira gonzalez on 03/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#include "ofxMapaMok.h"

ofxMapaMok::ofxMapaMok(){
    
    //  Public Default Properties
    //
    setupMode   = SETUP_SELECT;
    refMode     = REFERENCE_AXIS;
    drawMode    = DRAW_OCCLUDED_WIREFRAME;
    
    faceColor.set(80, 155);
    
    scale = 1.;
    lineWidth = 1;
    selectionRadius = 10;
    screenPointSize = 2;
    selectedPointSize = 3;
    slowLerpRate = .001;
    fastLerpRate = 1.;
    
    
    //  Private Defaul Properties ( the user don't need to have acces to them )
    //
    aov = 80;   // No se que es esto. Vos?
    selectedVert = false;
    hoverSelected = false;
    dragging = false;
    arrowing = false;
    shader = NULL;
    useLights = true;
    //  ViewPort default setup
    //
    init(0,0,ofGetWidth(),ofGetHeight());
    objName = "mapamok";
    bEditMode = false;
}

//  ------------------------------------------ MAIN LOOP

void ofxMapaMok::update(){
    
	if( setupMode == SETUP_SELECT ) {
	
        cam.enableMouseInput();
	
    } else {
		
        //  Generate camera matrix given aov guess
        //
        cv::Size2i imageSize(ofGetWidth(), ofGetHeight());
        float f = imageSize.width * ofDegToRad(aov); // i think this is wrong, but it's optimized out anyway
        cv::Point2f c = cv::Point2f(imageSize) * (1. / 2);
        cv::Mat1d cameraMatrix = (cv::Mat1d(3, 3) <<
                                  f, 0, c.x,
                                  0, f, c.y,
                                  0, 0, 1);
        
        //  Generate flags
        //
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

void ofxMapaMok::draw(ofTexture *_texture){
    
    if (_texture != NULL)
        if ((_texture->getWidth() != textWidth ) ||
            (_texture->getHeight() != textHeight) )
            ofLog(OF_LOG_WARNING, "The applied texture have a diferent size of the one spected");
    
    if( setupMode == SETUP_SELECT ) {
    
		//  Init easyCam
        //
        cam.begin( (ofRectangle)*this );
        
        //  Reference
        //
        if(refMode == REFERENCE_AXIS)
            ofDrawAxis(100);
        else if (refMode == REFERENCE_GRID)
            ofDrawGrid(100);
        
        //  Scale
        //
        ofScale(scale, scale, scale);
        
        //  Render
        //
        render(_texture);
        
        //  Update dots positions
        //
        if( setupMode ) {
            imageMesh = getProjectedMesh(objectMesh);
        }
        
        cam.end();
        
        //  On any tipe of setup mode
        //
        if( setupMode ) {
            ofPushStyle();
            
            //  Draw all points cyan small
            //
            ofSetColor( ofxCv::cyanPrint );
            for(int i=0; i< imageMesh.getVertices().size(); i++){
                if( inside(ofVec2f(imageMesh.getVertex(i).x,imageMesh.getVertex(i).y))){
                    ofCircle(imageMesh.getVertex(i).x, imageMesh.getVertex(i).y, 2);
                }
            }
            
            //  Draw all reference points cyan
            //
            int n = referencePoints.size();
            for(int i = 0; i < n; i++) {
                if(referencePoints[i]) {
                    drawLabeledPoint(i, imageMesh.getVertex(i), ofxCv::cyanPrint );
                }
            }
            
            //  Check to see if anything is selected
            //  Draw hover point magenta
            //
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
            
            //  Draw selected point yellow
            //
            if( selectedVert ) {
                int choice = selectionChoice;
                ofVec2f selected = imageMesh.getVertex(choice);
                drawLabeledPoint(choice, selected, ofxCv::yellowPrint, ofColor::white, ofColor::black);
            }
            
            ofPopStyle();
        }
        
	} else {
		
        if ( calibrationReady ) {
            glPushMatrix();
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glMatrixMode(GL_MODELVIEW);
            
            
            intrinsics.loadProjectionMatrix(10, 2000);
            ofxCv::applyMatrix(modelMatrix);
            render(_texture);
            if(setupMode) {
                imageMesh = getProjectedMesh(objectMesh);
            }
            
            
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            
        } else {
            
            ofDrawBitmapString("CALIBRATION NOT READY", x+width*0.5-80, y+height*0.5 );
            ofDrawBitmapString("(you need to set more dots)", x+width*0.5-100, y+height*0.5 + 15);
            
        }
        
        if( setupMode ) {
            ofPushStyle();
            
            // draw all reference points cyan
            //
            int n = referencePoints.size();
            for(int i = 0; i < n; i++) {
                if(referencePoints[i]) {
					if(i == selectionChoice){
						drawLabeledPoint(i, ofxCv::toOf(imagePoints[i]), ofxCv::yellowPrint, ofColor::white, ofColor::black);
					}else{
						drawLabeledPoint(i, ofxCv::toOf(imagePoints[i]), ofxCv::cyanPrint);
					}
                }
            }
            
            // move points that need to be dragged
            // draw selected yellow
            //
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
            
            ofPopStyle();
        }
        
	}
    
    if( setupMode != SETUP_NONE ) {
        if (inside(ofGetMouseX(), ofGetMouseY())){
            ofPushStyle();
            ofSetColor(255,100);
            ofNoFill();
            ofRect( (ofRectangle)*this );
            if ( setupMode == SETUP_CALIBRATE)
                ofDrawBitmapString("CALIBRATE the DOT", x+width*0.5-80,y+15);
            else if ( setupMode == SETUP_SELECT ){
                if (selectedVert)
                    ofDrawBitmapString("PRESS SPACE to CALIBRATE", x+width*0.5-100,y+15);
                else
                    ofDrawBitmapString("CLICK ONE DOT", x+width*0.5-60,y+15);
            }
            
            ofPopStyle();
        }
    }
}

void ofxMapaMok::drawLabeledPoint(int label, ofVec2f position, ofColor color, ofColor bg, ofColor fg) {
    
    if(!inside(position)) return;
    
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
    
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

void ofxMapaMok::render(ofTexture *_texture){
    
    ofPushStyle();
	ofSetLineWidth(lineWidth);
	if(useLights) {
		light.enable();
		ofEnableLighting();
		glShadeModel(GL_SMOOTH);
		glEnable(GL_NORMALIZE);
	}
	ofSetColor(255);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_DEPTH_TEST);
    if(shader != NULL) {
		shader->begin();
		shader->setUniform1f("elapsedTime", ofGetElapsedTimef());
		shader->end();
	}
    
	switch(drawMode) {
		case DRAW_FACES:
            
			if(shader != NULL)
                shader->begin();
            
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
            
            if (_texture != NULL)
                _texture->bind();
            
			objectMesh.drawFaces();
            
            if (_texture != NULL)
                _texture->unbind();
            
			if( shader != NULL )
                shader->end();
            
			break;
            
		case DRAW_FULL_WIREFRAME:
			
            if(shader != NULL)
                shader->begin();
            
			objectMesh.drawWireframe();
			
            if( shader != NULL )
                shader->end();
			break;
            
		case DRAW_OUTLINE_WIREFRAME:
            LineArt::draw(objectMesh, true, faceColor, shader);
			break;
            
		case DRAW_OCCLUDED_WIREFRAME:
            LineArt::draw(objectMesh, false, faceColor, shader);
			break;
	}
	glPopAttrib();
    if(useLights) {
		ofDisableLighting();
	}
	ofPopStyle();
}

// ------------------------------------------------------- EVENTS

void ofxMapaMok::_mousePressed(ofMouseEventArgs &e){
    selectedVert = hoverSelected;
	selectionChoice = hoverChoice;
	if(selectedVert) {
        dragging = true;
	}
}

void ofxMapaMok::_mouseReleased(ofMouseEventArgs &e){
    dragging = false;
    
    if (bEditMode){
        init(x, y, width, height);
    }
}

void ofxMapaMok::_keyPressed(ofKeyEventArgs &e){
    if (inside(ofGetMouseX(), ofGetMouseY())){
    
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
        
        //  Delete Selected
        //
        if(e.key == OF_KEY_BACKSPACE) {
            if(selectedVert) {
                selectedVert = false;
                int choice = selectionChoice;
                referencePoints[choice] = false;
                imagePoints[choice] = cv::Point2f();
            }
        }
        
        //  De-Select
        //
        if(e.key == '\n') {
            selectedVert = false;
        }
        
        //  Toggle SETUP Mode
        //
        if(e.key == ' ') {
            if (setupMode == SETUP_SELECT){
                setupMode = SETUP_CALIBRATE;
            }else if (setupMode == SETUP_CALIBRATE){
                    setupMode = SETUP_SELECT;
            }
            /*
            if (setupMode == SETUP_NONE){
                setupMode = SETUP_SELECT;
            } else if (setupMode == SETUP_SELECT){
                if (selectedVert)
                    setupMode = SETUP_CALIBRATE;
            } else if (setupMode == SETUP_CALIBRATE){
                if (calibrationReady)
                    setupMode = SETUP_NONE;
                else
                    setupMode = SETUP_SELECT;
            }*/
        }
        
        //  Toggle Drawing Mode
        //
        if(e.key == '\t') {
            if (drawMode == DRAW_FACES){
                drawMode = DRAW_FULL_WIREFRAME;
            } else if (drawMode == DRAW_FULL_WIREFRAME){
                drawMode = DRAW_OUTLINE_WIREFRAME;
            } else if (drawMode == DRAW_OUTLINE_WIREFRAME){
                drawMode = DRAW_OCCLUDED_WIREFRAME;
            } else if (drawMode == DRAW_OCCLUDED_WIREFRAME){
                drawMode = DRAW_FACES;
            }
        }
    }
    
    //  ViewPort Edit Mode
    //
    if(e.key == 'E'){
        bEditMode = !bEditMode;
    }
}


// --------------------------------------------------- LOAD & SAVE
bool ofxMapaMok::loadMesh(string _daeModel, int _textWidth, int _textHeight){
    bool fileLoaded = false;
    
    //  Cargamos el modelo
    //
    ofxAssimpModelLoader model;
	if (model.loadModel(_daeModel)){
        
        textWidth = _textWidth;
        textHeight = _textHeight;
        
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
            
            if ( (x > 1) || (y > 1) || (x < -1) || (y < -1))
                ofLog(OF_LOG_WARNING,"TexCoord " + ofToString(i) + " it's out of normalized values");
            
            objectMesh.getTexCoords()[i] = ofVec2f( x*textWidth, y*textHeight);
        }
        
        //  Creamos y asignamos los valores a los vectores contienen los puntos del modelo y los projectados
        //
        objectPoints.resize(n);
        imagePoints.resize(n);
        referencePoints.resize(n, false);
        for(int i = 0; i < n; i++) {
            objectPoints[i] = ofxCv::toCv(objectMesh.getVertex(i));
        }
        
        loadCalibration(modelFile);
    }
}

bool ofxMapaMok::loadCalibration(string _xmlfile) {
    
    //  Vemos si ya posee una calibración previamente realizada guardada dentro del dae
    //
    ofxXmlSettings XML;
    if (XML.loadFile(_xmlfile)){
        
        if (XML.tagExists("MAPAMOK")){
            ofLog(OF_LOG_NOTICE,"Loading MapaMok calibration found at " + modelFile );
            XML.pushTag("MAPAMOK");
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

bool ofxMapaMok::saveCalibration(string _xmlfile) {
    bool fileSaved = false;
    
    //  Si no se le pasa un .xml guarda la calibración dentro del .dae
    //
    if (_xmlfile == "none")
        _xmlfile = modelFile;
    
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

void ofxMapaMok::saveCameraMatrix(string _file){
    posCamara = cam.getGlobalTransformMatrix();
    
    ofFile outFile;
    outFile.open(_file, ofFile::WriteOnly, true);
    outFile.write((char*) posCamara.getPtr(), sizeof(float) * 16);
    outFile.close();
}

void ofxMapaMok::loadCameraMatrix(string _file){
    ofFile inFile;
    inFile.open(_file, ofFile::ReadOnly, true);
    inFile.read((char*) posCamara.getPtr(), sizeof(float) * 16);
    inFile.close();
    cam.setTransformMatrix(posCamara);
}