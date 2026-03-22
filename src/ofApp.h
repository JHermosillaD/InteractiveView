#pragma once
#include "ofMain.h"
#include "ofxMediaPipeHandTracker.h"
#include "V4L2CameraSettings.h"
#include "HandCameraController.h"
#include "LandmarkSmoother.h"

class ofApp : public ofBaseApp {
    public:
        void setup() override;
        void update() override;
        void draw() override;
        void exit() override;
        void keyPressed(int key) override;

        // Hand tracker
        ofEasyCam            cam;
        HandCameraController handCam;
        LandmarkSmoother     smoother;

        // Camera settings
        std::shared_ptr<ofx::MediaPipe::HandTracker> handTracker;
        V4L2CameraSettings camSettings;
        ofVideoGrabber     mGrabber;
        ofPixels           mVideoPixels;
        ofTexture          mVideoTexture;
};