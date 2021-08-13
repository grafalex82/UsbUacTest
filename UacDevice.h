#ifndef _UAC_DEVICE_H_
#define _UAC_DEVICE_H_

#include "UsbDevice.h"

#include <string.h> // for size_t

class UacDevice
{
    UsbDevice device;

public:
    enum Channel
    {
        Output,
        Input
    };

    UacDevice(uint16_t vid, uint16_t pid);
    ~UacDevice();

    void prepareAudioOutput();
    void prepareAudioInput();

    void setChannelVolume(Channel channel, int volume);
    int getChannelVolume(Channel channel);
    int getChannelMinVolume(Channel channel);
    int getChannelMaxVolume(Channel channel);

    int getChannelMute(Channel channel);

    void setChannelSampleRate(Channel channel, int rate);
    int getChannelSampleRate(Channel channel);

    void playPCM(unsigned char * data, size_t size);
    void recordPCM(unsigned char * data, size_t size);
    void loopback();

private:
    int getControlValue(uint8_t reqType, Channel channel, uint8_t control, uint16_t valueSize);
    void setControlValue(uint8_t reqType, Channel channel, uint8_t control, int value, uint16_t valueSize);

    uint8_t getEpForChannel(Channel channel);
    uint8_t getInterfaceForChannel(Channel channel);
    uint8_t getCtrlUnitForChannel(Channel channel);
};


#endif // _UAC_DEVICE_H_
