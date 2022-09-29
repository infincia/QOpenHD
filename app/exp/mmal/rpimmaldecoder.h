#ifndef RPIMMALDECODER_H
#define RPIMMALDECODER_H

#include "bcm_host.h"
#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/vcos/vcos.h"

#include <cstdint>

struct CONTEXT_T {
    VCOS_SEMAPHORE_T in_semaphore;
    VCOS_SEMAPHORE_T out_semaphore;
    MMAL_QUEUE_T *queue;
};


class RPIMMALDecoder
{
public:
    RPIMMALDecoder();

    static RPIMMALDecoder& instance();

    void initialize(const uint8_t* config_data,const int config_data_size,int width,int height,int fps);

    // Called every time we got a frame from the decoder
    void on_new_frame(MMAL_BUFFER_HEADER_T* buffer);

private:
    struct CONTEXT_T m_context;

    MMAL_STATUS_T m_status = MMAL_EINVAL;
    MMAL_COMPONENT_T *m_decoder = 0;

    MMAL_POOL_T *m_pool_in = 0;
    MMAL_POOL_T *m_pool_out = 0;

private:
    bool initialized_mmal=false;
};

#endif // RPIMMALDECODER_H
