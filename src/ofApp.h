#pragma once
#include "ofMain.h"
#include "ofxMediaPipeHandTracker.h"
#include "V4L2CameraSettings.h"

class ofApp : public ofBaseApp {
	public:
		void setup();
		void update();
		void draw();
		void exit() override;

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void drawInteractionArea();
		bool bHelpText;
		ofEasyCam cam;

	    V4L2CameraSettings camSettings;
		ofVideoGrabber mGrabber;
		ofPixels mVideoPixels;
		ofTexture mVideoTexture;
		ofFpsCounter mVideoFps;
		std::shared_ptr<ofx::MediaPipe::HandTracker> handTracker;

};