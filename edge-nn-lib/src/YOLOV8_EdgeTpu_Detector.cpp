#include "YOLOV8_EdgeTpu_Detector.h"



YOLOV8_EdgeTpu_Detector::YOLOV8_EdgeTpu_Detector(size_t pcie_device_number,
                                                 const std::string_view path_to_tflite_model,
                                                 float threshold) :
    m_threshold{threshold},
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
    auto in_scale = in_quant_params.scale;
    m_inputOffset =  in_quant_params.zero_point;
    m_inputKoeficient = (1. / 255.) / in_scale;

    TfLiteQuantizationParams quant_params = m_interpreter->output_tensor(0)->params;

    m_outputScale = quant_params.scale;
    m_outputOffset =  quant_params.zero_point;

    std::cout << "Output scale=" << m_outputScale << ", outputOffset=" << m_outputOffset << std::endl;

    m_status = Status::Ready;
}

std::vector<YOLOV8_EdgeTpu_Detector::Detection> YOLOV8_EdgeTpu_Detector::ExecuteInference(const uint8_t *data) {
    auto *input = m_interpreter->typed_input_tensor<int8_t>(0);
    if (input == nullptr) {
	std::cerr << "Failed to get input tensor" << std::endl;
	return {};
    }

    const size_t img_size = 640*640*3;
    for (size_t i = 0; i < img_size; i++) {
        input[i] = static_cast<int8_t>(data[i] * m_inputKoeficient) + m_inputOffset;
    }

    m_interpreter->Invoke();

    const auto *const output = m_interpreter->typed_output_tensor<int8_t>(0);
    typedef int8_t OutputArray[1][84][8400];
    OutputArray* out = (OutputArray*)output;

    std::vector<Detection> res;

    for (size_t i = 4; i < 84; i++) {
        std::vector<cv::Rect> rects;
        const int class_num = i - 4;
	for (size_t j = 0; j < 8400; j++) {
            float p = ((*out)[0][i][j] - m_outputOffset) * m_outputScale;
            if (p > m_threshold) {
                float x = ((*out)[0][0][j] - m_outputOffset) * m_outputScale * 640;
                float y = ((*out)[0][1][j] - m_outputOffset) * m_outputScale * 640;
                float w = ((*out)[0][2][j] - m_outputOffset) * m_outputScale * 640;
                float h = ((*out)[0][3][j] - m_outputOffset) * m_outputScale * 640;
                int x1 = std::max(0, static_cast<int>(x - w/2));
                int y1 = std::max(0, static_cast<int>(y - h/2));
                rects.emplace_back(cv::Rect{x1, y1, static_cast<int>(w), static_cast<int>(h)});
            }
        }
        std::cout << "Class " << class_num << " has " << rects.size() << " bounding boxes" << std::endl;
        if (rects.size() > 0) {
            for (const auto &r : rects) {
                 std::cout << "  x=" << r.x << ",y=" << r.y << ",w=" << r.width << ",h=" << r.height << std::endl;
            }
        }
        auto pred = NonMaximumSupression(rects);
        if (pred.size() > 0) {
            std::cout << "Class " << class_num << " has " << pred.size() << " objects" << std::endl;
            int i = 1;
            for (const auto &obj:pred) {
                std::cout << "   " << i++ << ". with x=" << obj.x << ",y=" << obj.y << ",width=" << obj.width << ",height=" << obj.height << std::endl;
               	Detection det;
		det.object_class = class_num;
                det.center = (obj.br() + obj.tl()) / 2.;
                det.size = obj.size();
		res.emplace_back(det);
            }
        }
    }

    return res;
}

std::vector<cv::Rect> YOLOV8_EdgeTpu_Detector::NonMaximumSupression(const std::vector<cv::Rect>& srcRects) {
    std::vector<cv::Rect> resRects;

    const size_t size = srcRects.size();
    if (size > 0) {

        // Sort the bounding boxes by the bottom - right y - coordinate of the bounding box
        std::multimap<int, size_t> idxs;
        for (size_t i = 0; i < size; ++i) {
            idxs.emplace(srcRects[i].br().y, i);
        }

        // keep looping while some indexes still remain in the indexes list
        while (idxs.size() > 0) {
            // grab the last rectangle
            auto lastElem = --std::end(idxs);
            const cv::Rect& rect1 = srcRects[lastElem->second];

            int neigborsCount = 0;
            idxs.erase(lastElem);

            for (auto pos = std::begin(idxs); pos != std::end(idxs); ) {
                // grab the current rectangle
                const cv::Rect& rect2 = srcRects[pos->second];

                float intArea = static_cast<float>((rect1 & rect2).area());
                float unionArea = rect1.area() + rect2.area() - intArea;
                float overlap = intArea / unionArea;

                // if there is sufficient overlap, suppress the current bounding box
                if (overlap > m_threshold) {
                    pos = idxs.erase(pos);
                    ++neigborsCount;
                } else {
                    ++pos;
                }
            }
            if (neigborsCount >= m_neighbors)
                resRects.push_back(rect1);
        }
    }

    return resRects;
}
