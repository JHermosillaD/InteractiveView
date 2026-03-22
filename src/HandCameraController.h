#pragma once
#include "ofMain.h"
#include "ofEasyCam.h"
#include "ofxMediaPipeHandTracker.h"

// - Claw on both hands: Engage (≥3 non-thumb fingers)
// - Left Wrist:         Controls pan (Yaw / Y-Axis)
// - Right Wrist:        Controls Tilt (Pitch / X-Axis)
// - Spread:             Controls Zoom

//========================================================================
class HandCameraController {
public:
    enum class Mode { IDLE, LOCKED, TWO_HANDS };
    void setup(ofEasyCam* cam, glm::vec3  target = {0.f, 0.f, 0.f}, float distance = 500.f);
    void update(std::shared_ptr<ofx::MediaPipe::HandTracker>& tracker, int videoW, int videoH);

    // Parameters
    float smoothing      = 0.12f;
    float clawMinDeg     = 35.f;
    int   clawMinFingers = 3;
    float lockDuration   = 0.25f;
    float minDistance    = 50.f;
    float maxDistance    = 3000.f;

    // Functions
    Mode getMode() const { return mMode; }
    float getDistance() const { return mDistance; }

private:
    using Hand    = ofx::MediaPipe::Hand;
    using Tracker = ofx::MediaPipe::HandTracker;

    bool isClaw (std::shared_ptr<Hand>& h) const;
    glm::quat handQuat (std::shared_ptr<Hand>& h) const;
    float spreadNorm(std::shared_ptr<Hand>& a, std::shared_ptr<Hand>& b, int vW, int vH) const;
    void applyToCamera();

    // Camera state
    ofEasyCam* mCam    = nullptr;
    glm::vec3  mTarget = {0.f, 0.f, 0.f};
    glm::quat mCameraQuat;        
    glm::quat mSmoothedQuat;      
    float mDistance     = 500.f;
    float mSmoothedDist = 500.f;

    // State machine
    Mode  mMode      = Mode::IDLE;
    float mLockTimer = 0.f;

    // Snapshots taken on mode entry
    glm::quat mRefQuatL;          
    glm::quat mRefQuatR;          
    glm::quat mCamQuatAtRefTwo;   
    float     mRefSpread = 0.f;   
    float     mDistAtRef = 0.f;   
};