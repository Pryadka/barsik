#ifndef __EDGETPU_DETECTOR__
#define __EDGETPU_DETECTOR__

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/kernels/register.h>
#include <edgetpu.h>
#include <edgetpu_c.h>

namespace barsik_cv {

class EdgeTpu_DetectorBase {
public:
    EdgeTpu_DetectorBase(size_t pcie_device_number, const std::string_view path_to_tflite_model);
    ~EdgeTpu_DetectorBase();

    enum class Status: uint8_t {
        Ready = 0,
        Busy,
        Failed,
    };

    Status GetStatus() const {return m_status;}

protected:
    std::unique_ptr<tflite::Interpreter> m_interpreter;
    TfLiteDelegate* m_delegate;
    std::unique_ptr<tflite::FlatBufferModel> m_model;
    float m_inputKoeficient;
    int m_inputOffset;
    float m_outputScale;
    int m_outputOffset;

private:
    Status m_status;
};

} // namespace barsik_cv

#endif // __FISHEYE_EDGETPU_DETECTOR__