#include "UacDevice.h"

#include <stdio.h>

static const uint16_t IRIG_UD2_VID = 0x1963;
static const uint16_t IRIG_UD2_PID = 0x0033;


int main(int argc, char ** argv)
{
	printf("Preparing the device...\n");
	UacDevice device(IRIG_UD2_VID, IRIG_UD2_PID);
	device.prepareAudioOutput();

	printf("Device ready\n");

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
		fout = fopen("./output_data.pcm","w+");
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
 
	 //-------------------The same interface can have multiple interface descriptors, identified by bAlternateSetting.
	 //BAlternateSetting value in Interface Descriptor
	libusb_set_interface_alt_setting(devh, IFACE_NUM, 1);
 
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