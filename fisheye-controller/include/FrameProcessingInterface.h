#ifndef __FRAME_PROCESSING_INTERFACE__
#define __FRAME_PROCESSING_INTERFACE__

#include <cstdint>

namespace camera_utility
{

    class FrameProcessingInterface
    {
    public:
        FrameProcessingInterface() = default;
        virtual ~FrameProcessingInterface() = default;

        virtual void ProcessFrame(uint8_t *address, uint32_t stride) = 0;
    };

} // namespace camera_utility

#endif