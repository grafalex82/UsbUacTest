#include "UacDevice.h"
#include "Utils.h"

#include <libusb.h>
#include <algorithm>

static const uint8_t AUDIO_CONTROL_INTERFACE = 0; // TODO: Should be taken from the descriptor
static const uint8_t AUDIO_OUTPUT_INTERFACE = 1; // TODO: Should be taken from the descriptor
static const uint8_t AUDIO_INPUT_INTERFACE = 2; // TODO: Should be taken from the descriptor

static const uint8_t AUDIO_OUTPUT_STREAMING_EP = 1; // TODO: Should be taken from the descriptor

static const uint8_t SAMPLE_SIZE_16BIT_ALTSETTING = 1; // TODO: Should be taken from the descriptor
static const uint8_t SAMPLE_SIZE_24BIT_ALTSETTING = 2; // TODO: Should be taken from the descriptor

static const uint8_t AUDIO_REQ_GET_CUR = 0x81U;
static const uint8_t AUDIO_REQ_GET_MIN = 0x82U;
static const uint8_t AUDIO_REQ_GET_MAX = 0x83U;
static const uint8_t AUDIO_REQ_SET_CUR = 0x01U;
static const uint8_t AUDIO_REQ_SET_MIN = 0x02U;
static const uint8_t AUDIO_REQ_SET_MAX = 0x03U;

static const uint8_t AUDIO_OUT_CTRL_UNIT = 0x02U; // TODO: Should be taken from the descriptor

static const uint8_t AUDIO_CONTROL_SELECTOR_MUTE = 0x01U;
static const uint8_t AUDIO_CONTROL_SELECTOR_VOLUME = 0x02U;

static const uint8_t SAMPLING_FREQ_CONTROL = 0x01U;


UacDevice::UacDevice(uint16_t vid, uint16_t pid)
    : device(vid, pid)
{
}

UacDevice::~UacDevice()
{
    device.closeInterface(AUDIO_CONTROL_INTERFACE);
    device.closeInterface(AUDIO_OUTPUT_INTERFACE);
}

void UacDevice::prepareAudioOutput()
{
    // Open the USB interface
    device.openInterface(AUDIO_CONTROL_INTERFACE);
    device.openInterface(AUDIO_OUTPUT_INTERFACE);

    // Select interface configuration with 16bit sample size
    device.setAltsetting(AUDIO_OUTPUT_INTERFACE, SAMPLE_SIZE_16BIT_ALTSETTING);
}

void UacDevice::setOutputVolume(int volume)
{
    setControlValue(AUDIO_REQ_SET_CUR, AUDIO_CONTROL_SELECTOR_VOLUME, volume, sizeof(int16_t));
}

int UacDevice::getOutputVolume()
{
    return (int16_t)getControlValue(AUDIO_REQ_GET_CUR, AUDIO_CONTROL_SELECTOR_VOLUME, sizeof(int16_t));
}

int UacDevice::getOutputMinVolume()
{
    return (int16_t)getControlValue(AUDIO_REQ_GET_MIN, AUDIO_CONTROL_SELECTOR_VOLUME, sizeof(int16_t));
}

int UacDevice::getOutputMaxVolume()
{
    return (int16_t)getControlValue(AUDIO_REQ_GET_MAX, AUDIO_CONTROL_SELECTOR_VOLUME, sizeof(int16_t));
}

int UacDevice::getOutputMute()
{
    return getControlValue(AUDIO_REQ_GET_CUR, AUDIO_CONTROL_SELECTOR_MUTE, sizeof(uint8_t));
}

int UacDevice::getControlValue(uint8_t reqType, uint8_t control, uint16_t valueSize)
{
    int32_t res = 0;

    device.controlReq(LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
        reqType, 
        control << 8, // target control 
        AUDIO_OUT_CTRL_UNIT << 8 | AUDIO_CONTROL_INTERFACE,
        std::min(valueSize, (uint16_t)sizeof(res)),
        (unsigned char*)&res
        );

    return res;
}

void UacDevice::setControlValue(uint8_t reqType, uint8_t control, int value, uint16_t valueSize)
{
    device.controlReq(LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
        reqType, 
        control << 8, // target control 
        AUDIO_OUT_CTRL_UNIT << 8 | AUDIO_CONTROL_INTERFACE,
        std::min(valueSize, (uint16_t)sizeof(int)),
        (unsigned char*)&value
        );
}

void UacDevice::setOutputSampleRate(int rate)
{
    device.controlReq(LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT,
        AUDIO_REQ_SET_CUR, // Request type
        SAMPLING_FREQ_CONTROL << 8, // selector type
        AUDIO_OUTPUT_STREAMING_EP, // endpoint number
        3, // yes 3, not 4
        (unsigned char*)&rate
        );
}

int UacDevice::getOutputSampleRate()
{
    uint32_t res = 0;

    device.controlReq(LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT,
        AUDIO_REQ_GET_CUR, // Request type
        SAMPLING_FREQ_CONTROL << 8, // selector type
        AUDIO_OUTPUT_STREAMING_EP, // endpoint number
        3, // yes 3, not 4
        (unsigned char*)&res
        );

    return res;
}
