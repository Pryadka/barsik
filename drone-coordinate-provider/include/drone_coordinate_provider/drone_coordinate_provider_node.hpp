#ifndef DRONE_COORDINATE_PROVIDER__DRONE_COORDINATE_PROVIDER_NODE_HPP_
#define DRONE_COORDINATE_PROVIDER__DRONE_COORDINATE_PROVIDER_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/nav_sat_fix.hpp>
#include "barsik_interfaces/msg/drone_state.hpp"


class DroneCoordinateProviderNode : public rclcpp::Node
{
public:
  DroneCoordinateProviderNode();

private:
  struct [[gnu::packed]] DataFrame {
  	int32_t Latitude;
  	int32_t Longitude;
  	int32_t Altitude;
  	int32_t Relative_altitude;
  	int16_t VX;
  	int16_t VY;
  	int16_t VZ;
  	uint16_t Heading;
  	uint16_t Satellited_visible;
  	uint16_t Yaw;
  };
  
  struct [[gnu::packed]] GPS_Frame {
  	double lat;
  	double lon;
  	double alt;
  };

  enum class RemoteDataCode: uint8_t {
    ERROR = 0,
    DRONE = 1,
    SELF = 2,
  };

  void read_coordinates();
  void publish_drone_state(DataFrame const *) const;
  void publish_self_gps(GPS_Frame const *) const;

  rclcpp::Publisher<barsik_interfaces::msg::DroneState>::SharedPtr drone_state_pub_;
  rclcpp::Publisher<sensor_msgs::msg::NavSatFix>::SharedPtr self_coordinates_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  int serial_;
};

#endif  // DRONE_COORDINATE_PROVIDER__DRONE_COORDINATE_PROVIDER_NODE_HPP_
