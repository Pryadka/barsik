#include "YOLOV8_EdgeTpu_Detector.h"

namespace barsik_cv {

YOLOV8_EdgeTpu_Detector::YOLOV8_EdgeTpu_Detector(size_t pcie_device_number,
                                                 const std::string_view path_to_tflite_model,
                                                 float threshold) :
    EdgeTpu_DetectorBase(pcie_device_number, path_to_tflite_model),
    m_threshold{threshold}
{
    m_inputKoeficient = (1. / 255.) / m_inputKoeficient;
    std::cout << "m_inputKoeficient=" << m_inputKoeficient << ", offset=" << m_inputOffset << std::endl;
}

std::vector<YOLOV8_EdgeTpu_Detector::Detection> YOLOV8_EdgeTpu_Detector::ExecuteInference(const uint8_t *data) {
    auto *input = m_interpreter->typed_input_tensor<int8_t>(0);
    if (input == nullptr) {
	    std::cerr << "Failed to get input tensor" << std::endl;
	    return {};
    }

    const size_t img_size = IMAGE_HEIGHT*IMAGE_WIDTH*IMAGE_CHANELS;
    for (size_t i = 0; i < img_size; i++) {
        input[i] = static_cast<int8_t>(data[i] * m_inputKoeficient) + m_inputOffset;
    }

    m_interpreter->Invoke();

    const auto *const output = m_interpreter->typed_output_tensor<int8_t>(0);
    typedef int8_t OutputArray[84][8400];
    OutputArray* out = (OutputArray*)output;

    std::vector<Detection> res;
    const float image_scale = m_outputScale * 640;

    for (size_t i = 4; i < 84; i++) {
        std::vector<cv::Rect> rects;
        const int class_num = i - 4;
	    for (size_t j = 0; j < 8400; j++) {
            float p = (static_cast<int>((*out)[i][j]) - m_outputOffset) * m_outputScale;
            if (p > m_threshold) {
                float x = (static_cast<int>((*out)[0][j]) - m_outputOffset) * image_scale;
                float y = (static_cast<int>((*out)[1][j]) - m_outputOffset) * image_scale;
                float w = (static_cast<int>((*out)[2][j]) - m_outputOffset) * image_scale;
                float h = (static_cast<int>((*out)[3][j]) - m_outputOffset) * image_scale;
                int x1 = std::max(0, static_cast<int>(x - w/2));
                int y1 = std::max(0, static_cast<int>(y - h/2));
                rects.emplace_back(cv::Rect{x1, y1, static_cast<int>(w), static_cast<int>(h)});
            }
        }
        // std::cout << "Class " << class_num << " has " << rects.size() << " bounding boxes" << std::endl;
        // if (rects.size() > 0) {
        //     for (const auto &r : rects) {
        //          std::cout << "  x=" << r.x << ",y=" << r.y << ",w=" << r.width << ",h=" << r.height << std::endl;
        //     }
        // }
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

} // namespace barsik_cv