#ifndef __FISHEYE_EDGETPU_DETECTOR__
#define __FISHEYE_EDGETPU_DETECTOR__

#include <iostream>
#include <cstring>
#include <chrono>
#include <utility>

#include "EdgeTpu_DetectorBase.h"

#include <opencv2/opencv.hpp>

namespace barsik_cv {

class Fisheye_EdgeTpu_Detector: public EdgeTpu_DetectorBase {
public:
    Fisheye_EdgeTpu_Detector(size_t pcie_device_number, const std::string_view path_to_tflite_model);
    ~Fisheye_EdgeTpu_Detector(){}

    std::pair<int, float> ExecuteInference(const uint8_t *data);

private:
    static constexpr int IMAGE_HEIGHT = 1280;
    static constexpr int IMAGE_WIDTH = 1280;
    static constexpr int IMAGE_CHANELS = 3;
};

} // namespace barsik_cv

#endif // __FISHEYE_EDGETPU_DETECTOR__