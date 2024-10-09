#include "Fisheye_EdgeTpu_Detector.h"

namespace barsik_cv {

Fisheye_EdgeTpu_Detector::Fisheye_EdgeTpu_Detector(
    size_t pcie_device_number,
    const std::string_view path_to_tflite_model) :
    EdgeTpu_DetectorBase(pcie_device_number, path_to_tflite_model)
{
}

int Fisheye_EdgeTpu_Detector::ExecuteInference(const uint8_t *data) {
    auto *input = m_interpreter->typed_input_tensor<uint8_t>(0);
    if (input == nullptr) {
	    std::cerr << "Failed to get input tensor" << std::endl;
	    return -1;
    }

    const size_t img_size = IMAGE_HEIGHT*IMAGE_WIDTH*IMAGE_CHANELS;
    for (size_t i = 0; i < img_size; i++) {
        input[i] = static_cast<int8_t>(data[i] * m_inputKoeficient) + m_inputOffset;
    }

    m_interpreter->Invoke();

    const auto *const output = m_interpreter->typed_output_tensor<uint8_t>(0);
    const uint8_t *const out = (const uint8_t*const)output;

    float highScore = 0.f;
    int res = -1;
    for (int i = 0; i < 12; i++) {
        float t = (out[i] - m_outputOffset) * m_outputScale;
        std::cout << "t=" << t << std::endl;
        if (t > highScore) {
            highScore = t;
            res = i;
        }
    }

    return res;
}

} // namespace barsik_cv