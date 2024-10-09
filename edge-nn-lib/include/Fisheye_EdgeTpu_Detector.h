#include <iostream>
#include <cstring>
#include <map>
#include <chrono>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/kernels/register.h>
#include <edgetpu.h>
#include <edgetpu_c.h>

#include <opencv2/opencv.hpp>

namespace barsik_cv {

class Fisheye_EdgeTpu_Detector {
public:
    Fisheye_EdgeTpu_Detector(size_t pcie_device_number, const std::string_view path_to_tflite_model);
    ~Fisheye_EdgeTpu_Detector(){}

    enum class Status: uint8_t {
        Ready = 0,
        Busy,
        Failed,
    };

    Status GetStatus() const {return m_status;}
    int ExecuteInference(const uint8_t *data);

private:
    Status m_status;
    std::unique_ptr<tflite::Interpreter> m_interpreter;
    TfLiteDelegate* m_delegate;
    std::unique_ptr<tflite::FlatBufferModel> m_model;
    float m_inputKoeficient;
    int m_inputOffset;
    float m_outputScale;
    int m_outputOffset;
};

} // namespace barsik_cv