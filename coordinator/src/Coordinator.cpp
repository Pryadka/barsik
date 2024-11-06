#include "Coordinator.h"
#include <algorithm>

using namespace std::chrono;

Coordinator::Coordinator()
{
    m_statusSetter.SetActiveStatus(true, false);

    for (auto &n : m_fisheyeDetections) {
        n = 11;
    }
}

Coordinator::~Coordinator() {

}

void Coordinator::NewFisheyeDetection(int n) {
    static const std::array<std::pair<int, int>, 10> anglesForSectors = {{
            { 0 ,  5},
            { 5,  5},
            { 5,  0},
            { 5, -5},
            { 0 , -5},
            { 0 ,  5},
            {-5,  5},
            {-5,  0},
            {-5, -5},
            { 0 , -5}
        }};
    if (m_fireInProgress) return;
    
    std::shift_right(m_fisheyeDetections.begin(), m_fisheyeDetections.end(), 1);
    m_fisheyeDetections[0] = n;

    int confidences[12] = {0};
    static const int confidenceTable[10] = {
        89, 55, 34, 21, 13, 8, 5, 3, 2, 1
    };


    for (int t = 0; t < 12; t++) {
        for (size_t i = 0; i < m_fisheyeDetections.size(); i++) {
            if (m_fisheyeDetections[i] == t) {
                confidences[t] += confidenceTable[i];
            }
        }
    }

    auto maxItemIt = std::max_element(std::begin(confidences), std::end(confidences));
    int filteredN = (*maxItemIt >= 144) ? std::distance(std::begin(confidences), maxItemIt) : 11;
    
    if (filteredN == 0) {
        m_statusSetter.SetActiveStatus(false, true);
        return;
    }
    if (n != 11) {
        m_turretController.SetHorizontalAngleDiff(anglesForSectors[n - 1].second);
        m_turretController.SetVerticalAngleDiff(anglesForSectors[n - 1].first);
    } else {
        m_turretController.MoveToBasePosition();
    }
}

void Coordinator::NewPreciseDetection(std::vector<PreciseDetection> &detections) {
    if (m_fireInProgress) return;

    if (detections.empty()) {
        m_statusSetter.SetActiveStatus(true, false);
        return;
    }

    // first step -> only one detection 
    const PreciseDetection &det = detections[0];
    int x_diff = det.x - 320;
    int y_diff = det.y - 320;

    int squared_distance_from_center = x_diff * x_diff + y_diff * y_diff;
    if (squared_distance_from_center < 200) {
        m_fireInProgress = true;
        std::thread waitForEndOfFireThread{[this](){
            std::this_thread::sleep_for(2000ms);
            m_fireInProgress = false;
        }};
        return;
    }

    float object_position_horizontal_ratio = (det.x - 320) / 640.;
    float object_position_vertical_ratio = (det.y - 320) / 640.;

    m_turretController.SetHorizontalAngleDiff(static_cast<int>(object_position_horizontal_ratio * 30.));
    m_turretController.SetVerticalAngleDiff(static_cast<int>(object_position_vertical_ratio * 30.));
}
