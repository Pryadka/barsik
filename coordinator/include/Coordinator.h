#ifndef __COORDINATOR_H__
#define __COORDINATOR_H__

#include <array>
#include <chrono>

#include "DataProviderStatusPublisher.h"
#include "TurretPositionController.h"

class Coordinator {
public:
    Coordinator();
    ~Coordinator();

    struct PrecisionDetection {
        int x;
        int y;
        int size;
        int focusFactor;
    };

    void NewFisheyeDetection(int n);
    void NewPreciseDetection(std::vector<PrecisionDetection> &detections);

private:
    DataProviderStatusPublisher m_statusSetter;
    TurretPositionController m_turretController;

    std::array<int, 10> m_fisheyeDetections;
    bool m_fireInProgress = false;
};

#endif // __COORDINATOR_H__