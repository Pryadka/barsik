#ifndef __YOLOV8_EDGETPU_DETECTOR__
#define __YOLOV8_EDGETPU_DETECTOR__

#include <cstring>
#include <map>
#include <chrono>

#include "EdgeTpu_DetectorBase.h"

#include <opencv2/opencv.hpp>

namespace barsik_cv {

class YOLOV8_EdgeTpu_Detector: public EdgeTpu_DetectorBase {
public:
    using VectorType = cv::Point2i;
    YOLOV8_EdgeTpu_Detector(size_t pcie_device_number, const std::string_view path_to_tflite_model, float threshold = 0.5f);
    ~YOLOV8_EdgeTpu_Detector(){}

    struct Detection {
        int object_class = -1;
        VectorType center;
        VectorType size;
    };

    std::vector<Detection> ExecuteInference(const uint8_t *data); //rgb 3 bytes per pixel
    void SetThreshold(float threshold) {m_threshold = threshold;}

private:
    float m_threshold;
    int m_neighbors = 0;

    std::vector<cv::Rect> NonMaximumSupression(const std::vector<cv::Rect>& srcRects);

    static constexpr int IMAGE_HEIGHT = 640;
    static constexpr int IMAGE_WIDTH = 640;
    static constexpr int IMAGE_CHANELS = 3;
};

} // namespace barsik_cv

#endif // __YOLOV8_EDGETPU_DETECTOR__
