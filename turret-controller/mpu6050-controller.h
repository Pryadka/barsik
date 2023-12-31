#ifndef __MPU_6050__
#define __MPU_6050__

#include "../eigen/Eigen/Dense"

namespace imu
{

class MPU_6050_Controller {
public:
    MPU_6050_Controller(unsigned int address = 0x68);
    ~MPU_6050_Controller();
    MPU_6050_Controller(const MPU_6050_Controller&) = delete;

    Eigen::Vector3d GetAccelData();
    Eigen::Vector3d GetGyroData();
    float GetTemp();
                                               
private :
    int m_i2c_handle;
};
    
} // namespace imu


#endif // __MPU_6050__