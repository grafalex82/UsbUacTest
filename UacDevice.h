#ifndef _UAC_DEVICE_H_
#define _UAC_DEVICE_H_

#include "UsbDevice.h"

class UacDevice
{
    UsbDevice device;

public:
    UacDevice(uint16_t vid, uint16_t pid);

    void prepareAudioOutput();
};


#endif // _UAC_DEVICE_H_