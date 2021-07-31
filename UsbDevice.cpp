#include "UsbDevice.h"
#include "Utils.h"

#include <libusb.h>
#include <stdio.h>


UsbDevice::UsbDevice(uint16_t vid, uint16_t pid)
{
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