#ifndef _UAC_DEVICE_H_
#define _UAC_DEVICE_H_

#include "UsbDevice.h"

class UacDevice
{
    UsbDevice device;

public:
    UacDevice(uint16_t vid, uint16_t pid);
    ~UacDevice();

    void prepareAudioOutput();

    void setOutputVolume(int volume);
    int getOutputVolume();
    int getOutputMinVolume();
    int getOutputMaxVolume();

    int getOutputMute();

    void setOutputSampleRate();
    int getOutputSampleRate();
};


#endif // _UAC_DEVICE_H_