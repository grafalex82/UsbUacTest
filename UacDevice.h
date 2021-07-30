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

    void setOutputSampleRate(int rate);
    int getOutputSampleRate();

private:
    int getControlValue(uint8_t reqType, uint8_t control, uint16_t valueSize);
    void setControlValue(uint8_t reqType, uint8_t control, int value, uint16_t valueSize);
};


#endif // _UAC_DEVICE_H_