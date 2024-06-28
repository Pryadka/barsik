#ifndef __OBJECT_DETECTION_FRAME_PROCESSOR__
#define __OBJECT_DETECTION_FRAME_PROCESSOR__

#include "FrameProcessingInterface.h"
#include <list>
#include <thread>
#include <opencv2/opencv.hpp>

namespace camera_utility
{

class FlyingObjectDetectionFrameProcessor : public FrameProcessingInterface {
public:
    FlyingObjectDetectionFrameProcessor();
    ~FlyingObjectDetectionFrameProcessor() override;

    void ProcessFrame(uint8_t *address, uint32_t stride) override;

    bool HasFrame() const;
    const cv::Mat GetLatestFrame();

    void SetFrameSize(uint32_t width, uint32_t height) override;

private:
    cv::Mat m_frame;
    mutable std::mutex m_mtx;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

} // namespace camera_utility

#endif // __OBJECT_DETECTION_FRAME_PROCESSOR__