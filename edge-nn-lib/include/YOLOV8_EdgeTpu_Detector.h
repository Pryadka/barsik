#ifndef __YOLOV8_EDGE_TPU__
#define __YOLOV8_EDGE_TPU__

#include <cstring>
#include <map>
#include <chrono>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/kernels/register.h>
#include <edgetpu.h>
#include <edgetpu_c.h>

#include <opencv2/opencv.hpp>

class YOLOV8_EdgeTpu_Detector {
public:
    using VectorType = cv::Point2i;
    YOLOV8_EdgeTpu_Detector(size_t pcie_device_number, const std::string_view path_to_tflite_model, float threshold = 0.5f);
    ~YOLOV8_EdgeTpu_Detector(){}

    struct Detection {
        int object_class = -1;
        VectorType center;
        VectorType size;
    };

    enum class Status: uint8_t {
        Ready = 0,
        Busy,
        Failed,
    };

    Status GetStatus() const {return m_status;}
    std::vector<Detection> ExecuteInference(const uint8_t *data); //rgb 3 bytes per pixel
    void SetThreshold(float threshold) {m_threshold = threshold;}

private:
    float m_threshold;
    int m_neighbors = 0;
    Status m_status;
    std::unique_ptr<tflite::Interpreter> m_interpreter;
    TfLiteDelegate* m_delegate;
    std::unique_ptr<tflite::FlatBufferModel> m_model;
    float m_inputKoeficient;
    int m_inputOffset;
    float m_outputScale;
    int m_outputOffset;

    std::vector<cv::Rect> NonMaximumSupression(const std::vector<cv::Rect>& srcRects);
};

#endif // __YOLOV8_EDGE_TPU__
