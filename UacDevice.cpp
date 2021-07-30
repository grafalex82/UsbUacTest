#include "UacDevice.h"
#include "Utils.h"

static const uint8_t AUDIO_CONTROL_INTERFACE = 0; // TODO: Should be taken from the descriptor
static const uint8_t AUDIO_OUTPUT_INTERFACE = 1; // TODO: Should be taken from the descriptor
static const uint8_t AUDIO_INPUT_INTERFACE = 2; // TODO: Should be taken from the descriptor

static const uint8_t SAMPLE_SIZE_16BIT_ALTSETTING = 1; // TODO: Should be taken from the descriptor
static const uint8_t SAMPLE_SIZE_24BIT_ALTSETTING = 2; // TODO: Should be taken from the descriptor


UacDevice::UacDevice(uint16_t vid, uint16_t pid)
    : device(vid, pid)
{
}

void UacDevice::prepareAudioOutput()
{
    // Open the USB interface
    device.openInterface(AUDIO_OUTPUT_INTERFACE);

    // Select interface configuration with 16bit sample size
    device.setAltsetting(AUDIO_OUTPUT_INTERFACE, SAMPLE_SIZE_16BIT_ALTSETTING);
}
