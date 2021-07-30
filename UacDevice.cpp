#include "UacDevice.h"
#include "Utils.h"

#include <libusb.h>

static const uint8_t AUDIO_CONTROL_INTERFACE = 0; // TODO: Should be taken from the descriptor
static const uint8_t AUDIO_OUTPUT_INTERFACE = 1; // TODO: Should be taken from the descriptor
static const uint8_t AUDIO_INPUT_INTERFACE = 2; // TODO: Should be taken from the descriptor

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

}

int UacDevice::getOutputVolume()
{
    int16_t res = 0;

    device.controlReq(LIBUSB_ENDPOINT_IN |LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
        AUDIO_REQ_GET_CUR, // Request type
        AUDIO_CONTROL_SELECTOR_VOLUME << 8, // target control 
        AUDIO_OUT_CTRL_UNIT << 8 | AUDIO_CONTROL_INTERFACE,
        sizeof(int16_t),
        (unsigned char*)&res
        );

    return res;
}

int UacDevice::getOutputMinVolume()
{
    int16_t res = 0;

    device.controlReq(LIBUSB_ENDPOINT_IN |LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
        AUDIO_REQ_GET_MIN, // Request type
        AUDIO_CONTROL_SELECTOR_VOLUME << 8, // target control 
        AUDIO_OUT_CTRL_UNIT << 8 | AUDIO_CONTROL_INTERFACE,
        sizeof(int16_t),
        (unsigned char*)&res
        );

    return res;
}

int UacDevice::getOutputMaxVolume()
{
    int16_t res = 0;

    device.controlReq(LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
        AUDIO_REQ_GET_MAX, // Request type
        AUDIO_CONTROL_SELECTOR_VOLUME << 8, // target control 
        AUDIO_OUT_CTRL_UNIT << 8 | AUDIO_CONTROL_INTERFACE,
        sizeof(int16_t),
        (unsigned char*)&res
        );

    return res;
}

int UacDevice::getOutputMute()
{
    uint8_t res = 0;

    device.controlReq(LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
        AUDIO_REQ_GET_CUR, // Request type
        AUDIO_CONTROL_SELECTOR_MUTE << 8, // target control 
        AUDIO_OUT_CTRL_UNIT << 8 | AUDIO_CONTROL_INTERFACE,
        sizeof(res),
        (unsigned char*)&res
        );

    return res;
}

void UacDevice::setOutputSampleRate()
{

}

int UacDevice::getOutputSampleRate()
{
    uint32_t res = 0;

    device.controlReq(LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_ENDPOINT,
        AUDIO_REQ_GET_CUR, // Request type
        1 << 8, // ????????
        1, // ??????????
        3, // yes 3, not 4
        (unsigned char*)&res
        );

    return res;
}
