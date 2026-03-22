#include "ofApp.h"
using namespace ofx::MediaPipe;

//========================================================================
void ofApp::setup(){
    ofSetFrameRate(60);
    ofSetVerticalSync(false);
    ofEnableDepthTest();

    // Camera setup
    mGrabber.setup(1280, 720);
    if (camSettings.open("/dev/video0")) {
        camSettings.applyProfile();
        camSettings.close();
    }

    // Hand tracker setup
    handTracker = std::make_shared<HandTracker>();
    HandTracker::HandSettings hsettings;
    hsettings.runningMode = Tracker::MODE_LIVE_STREAM;
    hsettings.maxNum = 2;
    handTracker->setup(hsettings);
    
    // Controller setup
    handCam.setup(&cam, {0,0,0}, 750.f);
    handCam.smoothing      = 0.12f;
    handCam.clawMinDeg     = 35.f;
    handCam.clawMinFingers = 3;
    handCam.lockDuration   = 0.25f;
}

//========================================================================
void ofApp::update(){
    // Camera update
    mGrabber.update();
    if (mGrabber.isInitialized() && mGrabber.isFrameNew()) {
        mVideoPixels = mGrabber.getPixels();
        mVideoPixels.mirror(false, true);
        handTracker->process(mVideoPixels);
        mVideoTexture.loadData(mVideoPixels);
    }

    // Hand tracker update
    float dt = ofGetLastFrameTime();
    smoother.update(handTracker->getHands(), dt);
    handCam.update(handTracker, mGrabber.getWidth(), mGrabber.getHeight());
}

//========================================================================
void ofApp::draw(){
    ofBackground(0);

    // 3D
    cam.begin();
    ofEnableDepthTest();
    ofDrawGrid(20, 10, false, true, true, true);

    if (handCam.getMode() == HandCameraController::Mode::TWO_HANDS || 
        handCam.getMode() == HandCameraController::Mode::LOCKED) {
        ofPushStyle();
        ofNoFill();
        ofSetLineWidth(2);
        ofSetColor(255, 255, 255, 30);
        ofSetSphereResolution(16);
        ofDrawSphere(0, 0, 0, 250);
        ofPopStyle();
    }

    ofDisableDepthTest();
    cam.end();

    // Camera viewer
    float padding = 20.f;
    float panelW  = ofGetWidth() * 0.25f;
    float panelX  = ofGetWidth() - panelW - padding;

    if (mVideoTexture.isAllocated()) {
        float aspect = (float)mVideoTexture.getHeight() / (float)mVideoTexture.getWidth();
        float panelH = panelW * aspect;

        ofSetColor(255);
        mVideoTexture.draw(panelX, padding, panelW, panelH);
        ofPushMatrix(); 
        ofTranslate(panelX, padding);
        ofScale(panelW / mVideoTexture.getWidth(), panelH / mVideoTexture.getHeight());
        handTracker->draw();
        ofPopMatrix();
    }
}

//========================================================================
void ofApp::exit(){
    handTracker.reset();
    Tracker::PyShutdown();
}

//========================================================================
void ofApp::keyPressed(int key){
    switch(key) {
        case 'F': case 'f': ofToggleFullscreen();                break;
        case 'R': case 'r': handCam.setup(&cam, {0,0,0}, 500.f); break;
    }
}