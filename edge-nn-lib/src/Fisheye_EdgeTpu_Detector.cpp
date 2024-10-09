#include "Fisheye_EdgeTpu_Detector.h"

namespace barsik_cv {

Fisheye_EdgeTpu_Detector::Fisheye_EdgeTpu_Detector(size_t pcie_device_number,
                                                 const std::string_view path_to_tflite_model) :
    m_status{Status::Failed}
{
    const auto& available_tpus =
        edgetpu::EdgeTpuManager::GetSingleton()->EnumerateEdgeTpu();
    std::cout << "Available tpus=" << available_tpus.size() << std::endl;

    auto edgetpuContext = edgetpu::EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!edgetpuContext) {
        std::cerr << "No coral found!" << std::endl;
        return;
    }

    size_t num_devices;
    std::unique_ptr<edgetpu_device, decltype(&edgetpu_free_devices)> devices(
        edgetpu_list_devices(&num_devices), &edgetpu_free_devices);

    if (num_devices == 0) {
        std::cerr << "No connected TPU found" << std::endl;
        return;
    }
    for (size_t i = 0; i < num_devices; ++i) {
        const auto& device = devices.get()[i];
        std::cout << i << " type=" << device.type << ",path=" << device.path << std::endl;
    }

    if (num_devices < pcie_device_number) {
        std::cerr << "Can't find device " << pcie_device_number << std::endl;
    }

    std::cout << "Num devices=" << num_devices << std::endl;
    const auto& device = devices.get()[pcie_device_number];

    tflite::ops::builtin::BuiltinOpResolver resolver;
    resolver.AddCustom(edgetpu::kCustomOp, edgetpu::RegisterCustomOp());

    m_model = tflite::FlatBufferModel::BuildFromFile(path_to_tflite_model.data());

    if (m_model == nullptr) {
        std::cerr << "Model not found!" << std::endl;
        return;
    }

    std::cout << "Model is ready" << std::endl;

    if (tflite::InterpreterBuilder(*m_model, resolver)(&m_interpreter) != kTfLiteOk) {
        std::cerr << "Failed to build interpreter!" << std::endl;
        return;
    }

    std::cout << "Interpreter is ready.Type=" << device.type << ",path=" << device.path << std::endl;

    m_delegate =  edgetpu_create_delegate(device.type, device.path, nullptr, 0);
    if (m_delegate == nullptr) {
        std::cerr << "Failed to create delegate" << std::endl;
        return;
    }
    m_interpreter->ModifyGraphWithDelegate(m_delegate);

    auto res = m_interpreter->AllocateTensors();
    if (res != kTfLiteOk) {
        std::cerr << "Failed accocate tensors" << std::endl;
        return;
    }

    std::cout << "Tensors allocated" << std::endl;

    // Invoke the interpreter once to load the model onto the coral
    res = m_interpreter->Invoke();
    if (res != kTfLiteOk) {
        std::cerr << "Failed to upload model to TPU" << std::endl;
        return;
    }

    std::cout << "Model uploaded, tensors_size=" << m_interpreter->tensors_size()  << std::endl;

    std::cout << "Input tensor bytes=" << m_interpreter->tensor(0)->bytes << std::endl;
    std::cout << "Output tensor bytes=" << m_interpreter->output_tensor(0)->bytes << std::endl;
    const auto *output_tensor_dims = m_interpreter->output_tensor(0)->dims;
    std::cout << "output tensor size=" << output_tensor_dims->size << std::endl;
    for (int i = 0; i < output_tensor_dims->size; i++) {
        std::cout << "     [" << i << "] " << output_tensor_dims->data[i] << std::endl;
    }

    TfLiteQuantizationParams in_quant_params = m_interpreter->input_tensor(0)->params;
    m_inputOffset =  in_quant_params.zero_point;
    m_inputKoeficient = in_quant_params.scale;
    std::cout << "Input scale=" << m_inputKoeficient << ", InputOffset=" << m_inputOffset << std::endl;

    TfLiteQuantizationParams quant_params = m_interpreter->output_tensor(0)->params;

    m_outputScale = quant_params.scale;
    m_outputOffset =  quant_params.zero_point;

    std::cout << "Output scale=" << m_outputScale << ", outputOffset=" << m_outputOffset << std::endl;

    m_status = Status::Ready;
}

int Fisheye_EdgeTpu_Detector::ExecuteInference(const uint8_t *data) {
    auto *input = m_interpreter->typed_input_tensor<uint8_t>(0);
    if (input == nullptr) {
	std::cerr << "Failed to get input tensor" << std::endl;
	return -1;
    }

    const size_t img_size = 1280*1280*3;
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