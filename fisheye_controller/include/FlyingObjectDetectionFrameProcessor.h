#ifndef __OPENCV_FRAME_PROCESSOR__
#define __OPENCV_FRAME_PROCESSOR__

#include "FrameProcessingInterface.h"
#include <list>
#include <thread>
#include <opencv2/opencv.hpp>

namespace camera_utility
{

    class OpenCVFrameProcessor : public FrameProcessingInterface
    {
    public:
        OpenCVFrameProcessor(uint32_t width, uint32_t height);
        ~OpenCVFrameProcessor() override;

        void ProcessFrame(uint8_t *address, uint32_t stride) override;

        bool HasFrame() const;
        const cv::Mat GetLatestFrame();

    private:
        cv::Mat m_frame;
        mutable std::mutex m_mtx;
        uint32_t m_width;
        uint32_t m_height;
    };

} // namespace camera_utility

#endif