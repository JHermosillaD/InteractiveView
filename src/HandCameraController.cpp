#include "HandCameraController.h"

using namespace ofx::MediaPipe;
using std::shared_ptr;
using std::string;

//========================================================================
static glm::quat swingTwist(glm::quat q, glm::vec3 axis) {
    glm::vec3 r = { q.x, q.y, q.z };
    glm::vec3 p = glm::dot(r, axis) * axis;
    glm::quat twist = glm::quat(q.w, p.x, p.y, p.z);
    float len = glm::length(glm::vec4(twist.x, twist.y, twist.z, twist.w));
    if (len < 1e-6f) return glm::quat(1.f, 0.f, 0.f, 0.f);
    return twist / len;
}

//========================================================================
// Setup
void HandCameraController::setup(ofEasyCam* cam, glm::vec3 target, float distance) {
    mCam = cam;
    mTarget = target;
    mDistance = mSmoothedDist = distance;
    mCameraQuat = glm::angleAxis(glm::radians(-20.f), glm::vec3(1.f, 0.f, 0.f));
    mSmoothedQuat = mCameraQuat;
    mCam->disableMouseInput();
    applyToCamera();
}

//========================================================================
// Helpers
bool HandCameraController::isClaw(shared_ptr<Hand>& h) const {
    float minRad = glm::radians(clawMinDeg);
    int count = 0;
    for (int fi = Hand::FINGER_INDEX; fi <= Hand::FINGER_PINKY; fi++) {
        auto f = h->getFinger(static_cast<Hand::FingerType>(fi));
        if (f && f->closedAmountRad > minRad) ++count;
    }
    return count >= clawMinFingers;
}

glm::quat HandCameraController::handQuat(shared_ptr<Hand>& h) const {
    auto& kp = h->keypoints;
    if (kp.size() < 21) return glm::quat(1.f, 0.f, 0.f, 0.f);

    glm::vec3 wrist  = kp[Hand::WRIST].posWorld;
    glm::vec3 palmR  = kp[Hand::INDEX_FINGER_MCP].posWorld;
    glm::vec3 midMCP = kp[Hand::MIDDLE_FINGER_MCP].posWorld;
    glm::vec3 palmL  = kp[Hand::PINKY_MCP].posWorld;
    
    glm::vec3 normal = glm::normalize(glm::cross(palmL - palmR, palmR - wrist));
    if (h->handed == Hand::RIGHT) normal *= -1.f;
    
    glm::vec3 up   = glm::normalize(midMCP - wrist);
    glm::vec3 side = glm::normalize(glm::cross(up, normal));
    up             = glm::normalize(glm::cross(normal, side));

    if (glm::any(glm::isnan(normal)) || glm::any(glm::isnan(up)) || glm::any(glm::isnan(side))) {
        return glm::quat(1.f, 0.f, 0.f, 0.f);
    }

    return glm::normalize(glm::quat_cast(glm::mat3(side, up, normal)));
}

float HandCameraController::spreadNorm(shared_ptr<Hand>& a, shared_ptr<Hand>& b, int vW, int vH) const {
    glm::vec2 pa = { a->palmPos.x / float(vW), a->palmPos.y / float(vH) };
    glm::vec2 pb = { b->palmPos.x / float(vW), b->palmPos.y / float(vH) };
    return glm::distance(pa, pb);
}

//========================================================================
// Update
void HandCameraController::update(shared_ptr<Tracker>& tracker, int videoW, int videoH) {
    if (!mCam) return;

    // Identify hands
    shared_ptr<Hand> leftHand, rightHand;
    for (auto& h : tracker->getHands()) {
        if (h->handed == Hand::RIGHT) rightHand = h;
        else                          leftHand  = h;
    }

    // Sanity check
    bool bothClaw = (leftHand && isClaw(leftHand)) && (rightHand && isClaw(rightHand));

    // State Machine
    if (bothClaw) {
        if (mMode != Mode::TWO_HANDS) {
            mRefQuatL        = handQuat(leftHand);
            mRefQuatR        = handQuat(rightHand);
            mCamQuatAtRefTwo = mCameraQuat;
            mRefSpread       = spreadNorm(leftHand, rightHand, videoW, videoH);
            mDistAtRef       = mDistance;
            mMode            = Mode::TWO_HANDS;
        }
    } else {
        if (mMode == Mode::TWO_HANDS) {
            mMode      = Mode::LOCKED;
            mLockTimer = lockDuration;
        } else if (mMode == Mode::LOCKED) {
            mLockTimer -= ofGetLastFrameTime();
            if (mLockTimer <= 0.f) mMode = Mode::IDLE;
        }
    }

    // Controll camera
    if (mMode == Mode::TWO_HANDS) { 
        glm::quat qDeltaL = handQuat(leftHand)  * glm::inverse(mRefQuatL);
        glm::quat qDeltaR = handQuat(rightHand) * glm::inverse(mRefQuatR);
        glm::quat yaw   = swingTwist(qDeltaL, glm::vec3(0.f, 1.f, 0.f));
        glm::quat pitch = swingTwist(qDeltaR, mCamQuatAtRefTwo * glm::vec3(1.f, 0.f, 0.f));
        mCameraQuat = glm::normalize(yaw * pitch * mCamQuatAtRefTwo);

        float curSpread = spreadNorm(leftHand, rightHand, videoW, videoH);
        if (curSpread > 1e-4f && mRefSpread > 1e-4f) {
            mDistance = glm::clamp(mDistAtRef * (mRefSpread / curSpread), minDistance, maxDistance);
        }
    }

    // Smoothing
    float t       = 1.f - smoothing;
    mSmoothedQuat = glm::slerp(mSmoothedQuat, mCameraQuat, t);
    mSmoothedDist += t * (mDistance - mSmoothedDist);
    applyToCamera();
}

//========================================================================
// Camera Matrix
void HandCameraController::applyToCamera() {
    if (!mCam) return;
    
    glm::vec3 offset = mSmoothedQuat * glm::vec3(0.f, 0.f, 1.f) * mSmoothedDist;
    mCam->setPosition(mTarget + offset);
    mCam->lookAt(mTarget, mSmoothedQuat * glm::vec3(0.f, 1.f, 0.f));
}