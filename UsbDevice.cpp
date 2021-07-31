#include "UsbDevice.h"
#include "Utils.h"

#include <libusb.h>
#include <stdio.h>


UsbDevice::UsbDevice(uint16_t vid, uint16_t pid)
{
    transferInProgress = false;

    int ret;

    // Init the library (TODO: perhaps this should move to a global context)
    ret = libusb_init(NULL);
    check(ret, "libusb_init()");

    // Now open the device with specified VID/PID
    hdev = libusb_open_device_with_vid_pid(NULL, vid, pid);
    check(hdev != NULL, "open_device_with_vid_pid");
}

UsbDevice::~UsbDevice()
{
    libusb_close(hdev);
	libusb_exit(NULL);
}

void UsbDevice::openInterface(uint8_t interface)
{
    // Detach kernel driver if needed (so that we can operate instead of the driver)
    int ret = libusb_kernel_driver_active(hdev, interface);
    check(ret, "libusb_kernel_driver_active()");
    if(ret == 1)
    {
        ret = libusb_detach_kernel_driver(hdev, interface);
        check(ret, "libusb_detach_kernel_driver()");
    }

    // Now claim the interface
    ret = libusb_claim_interface(hdev, interface);
    check(ret, "libusb_claim_interface()");
}

void UsbDevice::closeInterface(uint8_t interface)
{
    int ret = libusb_release_interface(hdev, interface);
    check(ret, "libusb_release_interface()");
}

void UsbDevice::setAltsetting(uint8_t interface, uint8_t altsetting)
{
    int ret = libusb_set_interface_alt_setting(hdev, interface, altsetting);
    check(ret, "libusb_set_interface_alt_setting()");
}

void UsbDevice::controlReq(uint8_t requestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data)
{
    int ret = libusb_control_transfer(hdev, requestType, bRequest, wValue, wIndex, data, wLength, 1000);
    check(ret, "libusb_control_transfer()");
}

void UsbDevice::getControlAttr(bool recepient, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data)
{
    uint8_t receiver = recepient ? LIBUSB_RECIPIENT_ENDPOINT : LIBUSB_RECIPIENT_INTERFACE;
    uint8_t bmRequestType = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | receiver;
    controlReq(bmRequestType, bRequest, wValue, wIndex, wLength, data);
}

void UsbDevice::setControlAttr(bool recepient, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data)
{
    uint8_t receiver = recepient ? LIBUSB_RECIPIENT_ENDPOINT : LIBUSB_RECIPIENT_INTERFACE;
    uint8_t bmRequestType = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | receiver;
    controlReq(bmRequestType, bRequest, wValue, wIndex, wLength, data);
}

void UsbDevice::transferCompleteCB(struct libusb_transfer * xfer)
{
    UsbDevice * device = static_cast<UsbDevice*>(xfer->user_data);
    device->handleTransferCompleteCB(xfer);
}

void UsbDevice::handleTransferCompleteCB(libusb_transfer * xfer)
{
//    printf("Transfer complete\n");
    transferInProgress = false;
}

void UsbDevice::transferIsoData(uint8_t ep, unsigned char * data, uint16_t numPackets, uint16_t packetSize)
{

    libusb_transfer * xfer = libusb_alloc_transfer(numPackets);
    libusb_fill_iso_transfer(xfer, hdev, ep, data, packetSize * numPackets, numPackets, transferCompleteCB, this, 1000);
    libusb_set_iso_packet_lengths(xfer, packetSize);

    transferInProgress = true;
    libusb_submit_transfer(xfer);

    while(transferInProgress)
    {
	int ret = libusb_handle_events(NULL);
	check(ret, "libusb_handle_events()");
    }

    libusb_free_transfer(xfer);
}
