#ifndef __FRAME_PROCESSING_INTERFACE__
#define __FRAME_PROCESSING_INTERFACE__

#include <cstdint>

namespace camera_utility
{

class FrameProcessingInterface {
public:
    FrameProcessingInterface() = default;
    virtual ~FrameProcessingInterface() = default;

    virtual void ProcessFrame(uint8_t *address, uint32_t stride) = 0;
    virtual void SetFrameSize(uint32_t width, uint32_t height) = 0;
};

} // namespace camera_utility

#endif  // __FRAME_PROCESSING_INTERFACE__