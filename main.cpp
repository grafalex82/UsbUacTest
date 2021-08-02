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

    std::unique_ptr<unsigned char[]> pcmData2(new unsigned char[size/3*2]);
    for(size_t sample=0; sample < size /3; sample++)
    {
        pcmData2[sample*2] = pcmData[sample*3];
        pcmData2[sample*2+1] = pcmData[sample*3+1];
    }

    FILE * pcm = fopen(argv[2],"w+b");
    check(pcm != NULL, "fopen() pcm file");
    fwrite(pcmData2.get(), 1, size/3*2, pcm);
    fclose(pcm);
}

int main(int argc, char ** argv)
{
    if(argc < 3)
    {
        printUsage();
        return 1;
    }

    if(!strcmp(argv[1], "play"))
        play(argc, argv);

    if(!strcmp(argv[1], "record"))
        record(argc, argv);

	return 0;
}



#if 0
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
 
#include "libusb.h"
 
//Device related information, related to your device
#define VID 0x1963
#define PID 0x0033
 #define EP_ISO_IN 0x82 //Endpoint address
 #define IFACE_NUM 2 //usb "interface" number The bNumInterfaces value in the Configuration Descriptor: indicates the number of interfaces in the configuration, and each interface in the configuration has its own interface number bInterfaceNumber,
 
 
 #define NUM_TRANSFERS 16 //This can be changed, if you like
 #define PACKET_SIZE 144 //The maximum transmission supported by lsusb is 200
 #define NUM_PACKETS 10 //This can also be changed if you like
 
 //After the data transfer is completed, the transfer task will call this callback function. We take the data here, and continue to add new transfer tasks, read it cyclically
static FILE* fout=NULL;
static int bFisrt = 1;
static void cb_xfr(struct libusb_transfer *xfr)
{
	if(bFisrt)
	{
		bFisrt = 0;
		fout = fopen("data.pcm","w+");
		if(fout == NULL)
		{
			printf("canok:: erro to openfile[%d%s] \n",__LINE__,__FUNCTION__);
		}
	}
 
	int  i =0;
	if(fout)
	 {//Retrieve data to file
		for(i=0;i<xfr->num_iso_packets;i++)
		{
			struct libusb_iso_packet_descriptor *pack = &xfr->iso_packet_desc[i];
			if(pack->status != LIBUSB_TRANSFER_COMPLETED)
			{
				printf("canok:: erro transfer status %d %s [%d%s]\n",pack->status,libusb_error_name(pack->status),__LINE__,__FUNCTION__);
				break;
			}
 
			const uint8_t *data = libusb_get_iso_packet_buffer_simple(xfr, i);
			printf("get out data %d [%d%s]\n",pack->actual_length,__LINE__,__FUNCTION__);
			fwrite(data,1,pack->actual_length,fout);
		}
 
	}	
 
	 //Re-submit the transfer task
	if (libusb_submit_transfer(xfr) < 0) {
		printf("error re-submitting !!!!!!!exit ----------[%d%s]\n",__LINE__,__FUNCTION__);
		exit(1);
	}
	else
	{
		printf("re-submint ok !\n");
	} 
}

int main()
{
 
	 //-------------------Start transfer
	uint8_t buf[PACKET_SIZE*NUM_PACKETS];
	struct libusb_transfer* xfr[NUM_TRANSFERS];
	 //There is a comment in the demo, which means that multiple transfers can be submitted at one time, so that when there is data on the bus being transferred, there can be some transfers that have been completed from the USB bus.
	 //transfer is calling callback, which can improve the usb bus utilization, so NUM_TRANSFER transfers are submitted here
	 //Each transfer transfers NUM_PACKETS packets, and the size of each packet is PACKET_SIZE. It can be understood that a transfer is a transfer task
	int i =0;
	for(i=0; i<NUM_TRANSFERS; i++)
	{
		xfr[i] = libusb_alloc_transfer(NUM_PACKETS);
		if(!xfr[i])
		{
			printf("can:: alloc transfer err [%d%s]o\n",__LINE__,__FUNCTION__);
			return -1;
		}
                                 //Fill transfer, such as transfer callback, cb_xfr, this transfer function will be called after the transfer is completed, we can be in the callback
		 //Take the data away. And resubmit the transfer task, so that iterates repeatedly.
		libusb_fill_iso_transfer(xfr[i],devh,EP_ISO_IN,buf,PACKET_SIZE*NUM_PACKETS,NUM_PACKETS,cb_xfr,NULL,1000);
		libusb_set_iso_packet_lengths(xfr[i], PACKET_SIZE);
 
		 //Formally submit the task
		ret = libusb_submit_transfer(xfr[i]);
		if(ret ==0)
		{
			printf("canok:: transfer submint ok ! start capture[%d%s]\n",__LINE__,__FUNCTION__);
		}
		else
		{
			printf("canok:: transfer submint erro %d %s [%d%s]\n",ret,libusb_error_name(ret),__LINE__,__FUNCTION__);
		}
	}
 
	 //------------------Main loop, driving event
	while(1)
	{
		ret = libusb_handle_events(NULL);
		if(ret != LIBUSB_SUCCESS)
		{
			printf("can:: handle event erro ,exit! [%d%s]\n",__LINE__,__FUNCTION__);
			break;
		}
	}
 
}

#endif //0
