#include "ofApp.h"
using namespace ofx::MediaPipe;

//========================================================================
void ofApp::setup(){
	ofSetVerticalSync(true);
	ofEnableDepthTest();
	ofSetCircleResolution(64);

	ofSetFrameRate(60);
    ofSetVerticalSync(false);
    mGrabber.setup(1280, 720);

    if (camSettings.open("/dev/video0")) {
        camSettings.applyProfile();
        camSettings.logCurrentValues();
        camSettings.close();
    }

    auto runMode = Tracker::MODE_LIVE_STREAM;
    handTracker = make_shared<HandTracker>();
    HandTracker::HandSettings hsettings;
    hsettings.runningMode = runMode;
    hsettings.maxNum = 2;
    handTracker->setup( hsettings );
}

//========================================================================
void ofApp::update(){

	mGrabber.update();
	if( mGrabber.isInitialized() && mGrabber.isFrameNew() ) {
		mVideoFps.newFrame();
		mVideoPixels = mGrabber.getPixels();
		mVideoPixels.mirror( false, true );
        handTracker->process(mVideoPixels);
		mVideoTexture.loadData(mVideoPixels);
	}

}

//========================================================================
void ofApp::draw(){

	ofBackground(0, 0, 0);
	cam.begin();
	ofSetConeResolution(20, 2);
	ofSetCylinderResolution(20, 2);
	ofEnableDepthTest();
	ofSetColor(ofColor::red);//RIGHT
	ofDrawCone(100, 0, 0, 50, 100);

	ofSetColor(ofColor::white);//LEFT
	ofDrawSphere(-100, 0, 0, 50);

	ofSetColor(ofColor::blue);//BOTTOM
	ofDrawBox(0, 100, 0, 100);

	ofSetColor(ofColor::cyan);//TOP
	ofDrawCylinder(0, -100, 0, 50, 100);

	ofSetColor(ofColor::yellow);//FRONT
	ofDrawBox(0, 0, 100, 100);

	ofSetColor(ofColor::magenta);//BACK
	ofDrawBox(0, 0, -100, 100);

	ofDrawGrid(20,10,false,true,true,true);
	ofDisableDepthTest();
	cam.end();
	drawInteractionArea();
	ofSetColor(255);

    float padding = 20.0f;
    float panelW  = ofGetWidth() * 0.25f;
    float panelX  = ofGetWidth() - panelW - padding;
    float currentY = padding;

	if( mVideoTexture.getWidth() > 0 && mVideoTexture.getHeight() > 0 ) {
        float videoAspect = (float)mVideoTexture.getHeight() / (float)mVideoTexture.getWidth();
        float panelH = panelW * videoAspect;        
        ofSetColor(255);
        mVideoTexture.draw(panelX, currentY, panelW, panelH);
        
        ofPushMatrix(); {
            ofTranslate(panelX, currentY);
            float scaleX = panelW / mVideoTexture.getWidth();
            float scaleY = panelH / mVideoTexture.getHeight();
            ofScale(scaleX, scaleY);
            handTracker->draw();

        } ofPopMatrix();

        currentY += panelH + padding; 
    }

}
//========================================================================
void ofApp::drawInteractionArea(){
	ofRectangle vp = ofGetCurrentViewport();
	ofPushStyle();
	ofSetLineWidth(3);
	ofSetColor(255, 255, 0);
	ofNoFill();
	ofPopStyle();
}

//========================================================================
void ofApp::exit(){
	handTracker.reset();
	Tracker::PyShutdown();
}

//========================================================================
void ofApp::keyPressed(int key){
	switch(key) {
		case 'F':
		case 'f':
			ofToggleFullscreen();
			break;
	}
}

//========================================================================
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}