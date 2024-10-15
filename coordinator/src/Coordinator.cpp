#include "Coordinator.h"
#include <algorithm>

using namespace std::chrono ;

Coordinator::Coordinator() {
    m_statusSetter.SetActiveStatus(true, false);
}

Coordinator::~Coordinator() {

}

void Coordinator::NewFisheyeDetection(int n) {
    if (m_fireInProgress) return;

    std::shift_right(m_fisheyeDetections.begin(), m_fisheyeDetections.end(), 1);
    m_fisheyeDetections[0] = n;
    if (n == 0) {
        m_statusSetter.SetActiveStatus(false, true);
        return;
    }
    if (n != 11) {
        static const std::array<std::pair<int, int>, 10> anglesForSectors = {{
            {0, -72},
            {90, -36},
            {90, 0},
            {90, 36},
            {0, 72},
            {0, -72},
            {-90, -36},
            {-90, 0},
            {-90, 36},
            {0, 72}
        }};
        m_turretController.SetHorizontalAngleDiff(anglesForSectors[n-1].second);
        m_turretController.SetVerticalAngleDiff(anglesForSectors[n-1].first);
    }
}

void Coordinator::NewPreciseDetection(std::vector<PrecisionDetection> &detections) {
    if (m_fireInProgress) return;

    if (detections.empty()) {
        m_statusSetter.SetActiveStatus(true, false);
        return;
    }

    // first step -> only one detection 
    const PrecisionDetection &det = detections[0];
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
