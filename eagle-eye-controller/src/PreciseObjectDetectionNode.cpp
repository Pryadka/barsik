#include "PreciseObjectDetectionNode.h"


PreciseObjectDetectionNode::PreciseObjectDetectionNode() :
    rclcpp::Node{"flying_object_detector"}
  , m_cameraController{libcamera::formats::RGB888, 1280, 1280}
  , m_objectDetectionPublisher{create_publisher<barsik_interfaces::msg::PreciseDetections>("object_detection", 10)}
{

    if (m_cameraController.GetStatus() != camera_utility::PreciseCameraController::Status::READY) {
        RCLCPP_FATAL(rclcpp::get_logger("rclcpp"), "Precise camera is not reday");
        return;
    }
    m_cameraController.SetViewPort(camera_utility::PreciseCameraController::k_matrixWidth/2, camera_utility::PreciseCameraController::k_matrixHeight/2, 640*10, 640*10);
    m_cameraController.Start(true);

    m_frameProcessor = std::make_shared<camera_utility::PreciseObjectDetectionFrameProcessor>(m_objectDetectionPublisher);

    m_cameraController.AddProcessor(m_frameProcessor);

    auto now = std::chrono::system_clock::now();
    auto seconds_since_epoch =
        std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    m_dataPath = fs::path("/home/sasa-r/src/barsik/") / "images_y" / std::to_string(seconds_since_epoch);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Creating data folder: %s", m_dataPath.string().c_str());
    bool pathCreationResult = fs::create_directory(m_dataPath);
    if (!pathCreationResult) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to create data folder");
    }

    auto activation_callback =
        [this](barsik_interfaces::msg::ActivateDataProvider::UniquePtr msg) -> void {
            RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Fish eye camera active %s", msg->fish_eye_camera_active ? "Yes": "No");
            m_cameraController.SetActive(msg->fish_eye_camera_active);
            if (msg->fish_eye_camera_active == false) {
                if (m_frameProcessor->HasFrame()) {
                    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Saving image %lu", m_imageNumber);
                    SaveImage(m_frameProcessor->GetLatestFrame());
                }
            }
        };
            
    m_activation_subscription =
        create_subscription<barsik_interfaces::msg::ActivateDataProvider>("video_provider_activation", 10, activation_callback);

    auto save_current_frame_callback =
        [this](barsik_interfaces::msg::DebugSaveCurrentImage::UniquePtr msg) -> void {
            if (msg->precise == true) {
                if (m_frameProcessor->HasFrame()) {
                    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Saving image %lu", m_imageNumber);
                    SaveImage(m_frameProcessor->GetLatestFrame());
                }
            }
        };

    m_save_current_frame_subscription =
        create_subscription<barsik_interfaces::msg::DebugSaveCurrentImage>("video_save_frame", 10, save_current_frame_callback);
}

PreciseObjectDetectionNode::~PreciseObjectDetectionNode() {
}

void PreciseObjectDetectionNode::SaveImage(const cv::Mat &image) {
	auto dataPath = m_dataPath / (std::to_string(m_imageNumber++) + ".jpg");
	const std::string fileName = dataPath.string();

	cv::imwrite(fileName, image);
}
