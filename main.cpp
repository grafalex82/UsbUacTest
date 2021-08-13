#include "UacDevice.h"
#include "Utils.h"

#include <stdio.h>
#include <memory>


static const uint16_t IRIG_UD2_VID = 0x1963;
static const uint16_t IRIG_UD2_PID = 0x0033;

void printUsage()
{
    printf("Usage:\n");
    printf("    UAC2Test play <filename>\n");
    printf("    UAC2Test record <filename>\n");
    printf("    UAC2Test loopback\n");
}

void play(int argc, char ** argv)
{
    // TODO: Add parameter parsing, and particularly sample rate

    printf("Preparing the device...\n");
    UacDevice device(IRIG_UD2_VID, IRIG_UD2_PID);
    device.prepareAudioOutput();

    printf("Device ready. Setting audio parameters\n");
    device.setChannelVolume(UacDevice::Output, 20);
    device.setChannelSampleRate(UacDevice::Output, 48000);

    printf("    Current volume: %d\n", device.getChannelVolume(UacDevice::Output));
    printf("    Maximum volume: %d\n", device.getChannelMinVolume(UacDevice::Output));
    printf("    Minimum volume: %d\n", device.getChannelMaxVolume(UacDevice::Output));
    printf("    Mute: %d\n", device.getChannelMute(UacDevice::Output));
    printf("    Sample Rate: %d\n", device.getChannelSampleRate(UacDevice::Output));

    // Load file to play
    FILE * pcm = fopen(argv[2],"rb");
    check(pcm != NULL, "fopen() pcm file");
    fseek(pcm, 0, SEEK_END);
    long size = ftell(pcm);
    check(size != 0, "PCM file is empty");
    fseek(pcm, 0, SEEK_SET);
    check(ftell(pcm) == 0, "Incorrect file position");

    std::unique_ptr<unsigned char[]> pcmData(new unsigned char[size]);
    int readBytes = fread(pcmData.get(), 1, size, pcm);
    check(readBytes == size, "Cant read PCM data");
    fclose(pcm);

    device.playPCM(pcmData.get(), size);
}

void record(int argc, char ** argv)
{
    printf("Preparing the device...\n");
    UacDevice device(IRIG_UD2_VID, IRIG_UD2_PID);
    device.prepareAudioInput();

    printf("Device ready. Setting audio parameters\n");
    device.setChannelSampleRate(UacDevice::Input, 48000);
    printf("    Sample Rate: %d\n", device.getChannelSampleRate(UacDevice::Input));

    size_t size = 5 * 48000 * 3; // 5 seconds of 48kHz audio at 24 bit/sample
    std::unique_ptr<unsigned char[]> pcmData(new unsigned char[size]);
    device.recordPCM(pcmData.get(), size);
/*
    // Convert 24bit signed values to 16bit signed
    std::unique_ptr<unsigned char[]> pcmData2(new unsigned char[size/3*2]);
    for(size_t sample=0; sample < size /3; sample++)
    {
        uint8_t value[4];
        value[0] = pcmData[sample*3];
        value[1] = pcmData[sample*3 + 1];
        value[2] = pcmData[sample*3 + 2];
        value[3] = pcmData[sample*3 + 2] & 0x80 ? 0xff : 0x00;

        int32_t iValue = *(int32_t *)value;

        if (iValue >= 32787) iValue = 32767;
        if (iValue <= -32787) iValue = -32767;

        *(int16_t*)(pcmData2.get() + sample*2) = (int16_t)iValue;
    }
*/
    FILE * pcm = fopen(argv[2],"w+b");
    check(pcm != NULL, "fopen() pcm file");
//    fwrite(pcmData2.get(), 1, size/3*2, pcm);
    fwrite(pcmData.get(), 1, size, pcm);
    fclose(pcm);
}

void loopback(int argc, char ** argv)
{
    printf("Preparing the device...\n");
    UacDevice device(IRIG_UD2_VID, IRIG_UD2_PID);
    device.prepareAudioOutput();
    device.prepareAudioInput();

    device.setChannelSampleRate(UacDevice::Output, 44100);
    printf("    Output Sample Rate: %d\n", device.getChannelSampleRate(UacDevice::Output));

    device.setChannelSampleRate(UacDevice::Input, 44100);
    printf("    Input Sample Rate: %d\n", device.getChannelSampleRate(UacDevice::Input));

    device.loopback();
}

int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        printUsage();
        return 1;
    }

    if(!strcmp(argv[1], "play"))
        play(argc, argv);

    if(!strcmp(argv[1], "record"))
        record(argc, argv);

    if(!strcmp(argv[1], "loopback"))
        loopback(argc, argv);

    return 0;
}
