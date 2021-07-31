#include "UsbDevice.h"
#include "Utils.h"

#include <libusb.h>
#include <stdio.h>

static const size_t NUM_TRANSFERS = 10;
static const uint8_t NUM_PACKETS = 2;

UsbDevice::UsbDevice(uint16_t vid, uint16_t pid)
{
    // Init the library (TODO: perhaps this should move to a global context)
    int ret = libusb_init(NULL);
    check(ret, "libusb_init()");

    // Now open the device with specified VID/PID
    hdev = libusb_open_device_with_vid_pid(NULL, vid, pid);
    check(hdev != NULL, "open_device_with_vid_pid");

    // Allocate needed number of transfer objects
    for(size_t i=0; i<NUM_TRANSFERS; i++)
    {
        libusb_transfer * xfer = libusb_alloc_transfer(NUM_PACKETS);
        availableXfers.push_back(xfer);
    }

}

UsbDevice::~UsbDevice()
{
    for(libusb_transfer * xfer : availableXfers)
        libusb_free_transfer(xfer);

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
    fflush(stdout);
    availableXfers.push_back(xfer);
}

void UsbDevice::transferIsoData(uint8_t ep, unsigned char * data, size_t size, uint16_t packetSize)
{
    size_t totalPackets = size / packetSize;
    size_t bytesToGo = size;

    while(bytesToGo > 0)
    {
        // Feed as many packets as possible
        while(availableXfers.size() > 0)
        {
            size_t chunkSize = std::min((size_t)packetSize * NUM_PACKETS, bytesToGo);

            libusb_transfer * xfer = availableXfers.back();
            availableXfers.pop_back();
            libusb_fill_iso_transfer(xfer, hdev, ep, data, chunkSize, NUM_PACKETS, transferCompleteCB, this, 1000);
            libusb_set_iso_packet_lengths(xfer, packetSize);
            libusb_submit_transfer(xfer);

            data += chunkSize;
            bytesToGo -= chunkSize;
        }

        int ret = libusb_handle_events(NULL);
        check(ret, "libusb_handle_events()");
    }

    // Wait for remaining packets to be sent
    while(availableXfers.size() != NUM_TRANSFERS)
    {
        int ret = libusb_handle_events(NULL);
        check(ret, "libusb_handle_events()");
    }
}
