#include "LandmarkSmoother.h"

using std::shared_ptr;

//========================================================================
float LandmarkSmoother::alpha(float cutoff, float dt) {
    float tau = 1.f / (2.f * static_cast<float>(M_PI) * cutoff);
    return 1.f / (1.f + tau / dt);
}

glm::vec3 LandmarkSmoother::filterVec3(Channel3D& ch, const glm::vec3& raw, float dt) const {
    if (!ch.init) {
        ch.xFilt  = raw;
        ch.dxFilt = {0.f, 0.f, 0.f};
        ch.init   = true;
        return raw;
    }

    glm::vec3 dx = (raw - ch.xFilt) / dt;
    float aDeriv = alpha(dCutoff, dt);
    ch.dxFilt    = aDeriv * dx + (1.f - aDeriv) * ch.dxFilt;
    float adaptiveCutoff = minCutoff + beta * glm::length(ch.dxFilt);
    float aMain  = alpha(adaptiveCutoff, dt);

    ch.xFilt = aMain * raw + (1.f - aMain) * ch.xFilt;
    return ch.xFilt;
}

//========================================================================
void LandmarkSmoother::update(std::vector<std::shared_ptr<ofx::MediaPipe::Hand>>& hands, float dt) {
    if (dt < 1e-6f) return;

    std::vector<unsigned int> activeIDs;
    activeIDs.reserve(hands.size());

    for (auto& hand : hands) {
        if (!hand) continue;
        
        unsigned int id = hand->ID;
        activeIDs.push_back(id);

        auto& state = mState[id];
        if (state.size() < hand->keypoints.size()) {
            state.resize(hand->keypoints.size());
        }

        for (size_t i = 0; i < hand->keypoints.size(); i++) {
            auto& kp = hand->keypoints[i];
            auto& st = state[i];

            kp.pos      = filterVec3(st[0], kp.pos, dt);
            kp.posWorld = filterVec3(st[1], kp.posWorld, dt);
        }
        
        hand->updateFromKeypoints();
    }

    // Clean lost hands
    for (auto it = mState.begin(); it != mState.end(); ) {
        if (std::find(activeIDs.begin(), activeIDs.end(), it->first) == activeIDs.end()) {
            it = mState.erase(it);
        } else {
            ++it;
        }
    }
}

void LandmarkSmoother::reset() {
    mState.clear();
}