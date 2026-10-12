#include "drone_coordinate_provider/drone_coordinate_provider_node.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>

using namespace std::chrono_literals;

DroneCoordinateProviderNode::DroneCoordinateProviderNode()
: Node("drone_coordinate_provider")
{
  drone_state_pub_ = create_publisher<barsik_interfaces::msg::DroneState>("drone/state", 10);
  self_coordinates_pub_ = create_publisher<sensor_msgs::msg::NavSatFix>("self/coordinates", 10);

  constexpr char port[] = "/dev/ttyAMA0";

  // Open serial port
  serial_ = open(port, O_RDWR | O_NOCTTY | O_SYNC);

  if (serial_ < 0) {
    RCLCPP_ERROR(get_logger(), "Error opening serial port");
    throw std::runtime_error("SERIAL failed");
  }

  // Get current serial configuration
  struct termios tty{};

  if (tcgetattr(serial_, &tty) != 0) {
    RCLCPP_ERROR(get_logger(), "Error getting serial settings");
    close(serial_);
    throw std::runtime_error("SERIAL tty failed");
  }

  // Baud rate
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  // 8 data bits
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;

  // No parity
  tty.c_cflag &= ~PARENB;

  // 1 stop bit
  tty.c_cflag &= ~CSTOPB;

  // No hardware flow control
  tty.c_cflag &= ~CRTSCTS;

  // Enable receiver, ignore modem control lines
  tty.c_cflag |= CREAD | CLOCAL;

  // Raw input
  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  // Raw output
  tty.c_oflag &= ~OPOST;

  // No special input processing
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(INLCR | ICRNL);

  // Read timeout
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 10; // 1 second

  // Apply settings
  if (tcsetattr(serial_, TCSANOW, &tty) != 0) {
    RCLCPP_ERROR(get_logger(), "Error applying serial settings");
    close(serial_);
    throw std::runtime_error("SERIAL tty failed");
  }

  timer_ = create_wall_timer(1ms, std::bind(&DroneCoordinateProviderNode::read_coordinates, this));

  RCLCPP_INFO(get_logger(), "Drone coordinate provider node started");
}

void DroneCoordinateProviderNode::read_coordinates()
{
  static uint8_t buffer[1024];

  int bytesRead = read(serial_, buffer, sizeof(buffer) - 1);

  if (bytesRead > 0) {
    // std::cout << "Received: " << bytesRead << '\n';
	  // for (int i  = 0; i < bytesRead; i++) {
 	  //   static constexpr char digs[] = "0123456789abcdef";
		//   std::cout << digs[0xf & (buffer[i] >> 4)] << digs[buffer[i] & 0xf];
	  // }
		
    uint8_t *buffer_start = buffer;
	  while (bytesRead >= 10) {
   		constexpr uint32_t M_BEGIN{0xffffffff}, M_END{0xc5c5c5c5};
   		uint32_t data_size = 5;
		  if (std::memcmp(buffer_start, reinterpret_cast<const uint8_t *>(&M_BEGIN), sizeof(uint32_t)) == 0) {
		    RemoteDataCode data_code = static_cast<RemoteDataCode>(buffer_start[4]);
        DataFrame *data_frame = nullptr;
        GPS_Frame *gps_frame = nullptr;
        switch(data_code) {
          case RemoteDataCode::ERROR:
            RCLCPP_ERROR(get_logger(), "Error. Code = %d", buffer_start[5]);
            data_size += 1;
            break;
          case RemoteDataCode::DRONE:
            RCLCPP_DEBUG(get_logger(), "Drone coordinates");
            data_frame = reinterpret_cast<DataFrame*>(buffer_start + 5);
            data_size += sizeof(DataFrame);
            break;
          case RemoteDataCode::SELF:
            RCLCPP_DEBUG(get_logger(), "Self coordinates");
            gps_frame = reinterpret_cast<GPS_Frame *>(buffer_start + 5);
            data_size += sizeof(GPS_Frame);
            break;
          default: std::cout <<  "Not defined operation" << std::endl;
        }
        bytesRead -= data_size + sizeof(uint32_t);
        if (bytesRead < 0 || 
            std::memcmp(buffer_start + data_size, reinterpret_cast<const uint8_t *>(&M_END), sizeof(uint32_t)) != 0) {
				  RCLCPP_ERROR(get_logger(), "Wrong End of frame");
				  break;
		    }
        if (data_frame != nullptr) {
          publish_drone_state(data_frame);
        }
        if (gps_frame != nullptr) {
          publish_self_gps(gps_frame);
        }
		    buffer_start += data_size + sizeof(uint32_t);
		  } else {
        std::cout << "Wrong BEGIN" << std::endl;
        break;
      }
    }
	}
}

void DroneCoordinateProviderNode::publish_drone_state(DataFrame const *frame) const {
  barsik_interfaces::msg::DroneState msg;

  msg.latitude = frame->Latitude;
  msg.longitude = frame->Longitude;
  msg.altitude = frame->Altitude;
  msg.relative_altitude = frame->Relative_altitude;
  msg.vx = frame->VX;
  msg.vy = frame->VY;
  msg.vz = frame->VZ;
  msg.heading = frame->Heading;
  msg.satellited_visible = frame->Satellited_visible;
  msg.yaw = frame->Yaw;
  drone_state_pub_->publish(msg);
}

void DroneCoordinateProviderNode::publish_self_gps(GPS_Frame const *frame) const {
  sensor_msgs::msg::NavSatFix msg;
  msg.header.stamp = this->get_clock()->now();
  msg.header.frame_id = "gps_link";

        // 2. Set the Status (Fix type)
  msg.status.status = sensor_msgs::msg::NavSatStatus::STATUS_FIX; // Basic unaugmented fix
  msg.status.service = sensor_msgs::msg::NavSatStatus::SERVICE_GPS;

        // 3. Set Coordinates (Coordinates around Böblingen, Germany)
  msg.latitude = frame->lat;
  msg.longitude = frame->lon;
  msg.altitude = frame->alt; // Meters above ellipsoid

  // 4. Set Position Covariance (Diagonal matrix for uncertainty)
  // Order is East, North, Up (ENU)
  msg.position_covariance[0] = 0.5; // East variance
  msg.position_covariance[4] = 0.5; // North variance
  msg.position_covariance[8] = 1.2; // Up/Vertical variance
  
  // Define how the covariance is filled
  msg.position_covariance_type = sensor_msgs::msg::NavSatFix::COVARIANCE_TYPE_DIAGONAL_KNOWN;

  self_coordinates_pub_->publish(msg);
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DroneCoordinateProviderNode>());
  rclcpp::shutdown();
  return 0;
}
