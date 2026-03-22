#pragma once
#include "ofMain.h"
#include "ofxMediaPipeHandTracker.h"
#include <unordered_map>
#include <vector>
#include <array>

// 3D Vectorized One Euro Filter for hand tracking landmarks

//========================================================================
class LandmarkSmoother {
public:
    float minCutoff = 1.5f;
    float beta      = 0.08f;
    float dCutoff   = 1.0f;

    void update(std::vector<std::shared_ptr<ofx::MediaPipe::Hand>>& hands, float dt);
    void reset();

private:
    struct Channel3D {
        glm::vec3 xFilt  = {0.f, 0.f, 0.f};
        glm::vec3 dxFilt = {0.f, 0.f, 0.f};
        bool      init   = false;
    };

    glm::vec3 filterVec3(Channel3D& ch, const glm::vec3& raw, float dt) const;
    static float alpha(float cutoff, float dt);

    using LandmarkState = std::array<Channel3D, 2>; 
    using HandState     = std::vector<LandmarkState>;
    
    std::unordered_map<unsigned int, HandState> mState;
};