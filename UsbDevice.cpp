#include "UsbDevice.h"
#include "Utils.h"

#include <libusb.h>
#include <stdio.h>

static const size_t NUM_TRANSFERS = 2;
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

        libusb_transfer * xfer2 = libusb_alloc_transfer(NUM_PACKETS);
        availableOutXfers.push_back(xfer2);

        // TODO: gather in and out transfers, along with associated buffer in a single structure
        uint8_t * buf = new uint8_t[1024*NUM_PACKETS];  // Size of the buffer shall correspond input and output packet sizes, multiplied by NUM_TRANSFERS
        buffers.push_back(buf);
        // we need double number of buffers to handle simultaneous input and output
        buf = new uint8_t[1024*NUM_PACKETS];  // Size of the buffer shall correspond input and output packet sizes, multiplied by NUM_TRANSFERS
        buffers.push_back(buf);
    }
}

UsbDevice::~UsbDevice()
{
    for(libusb_transfer * xfer : availableXfers)
        libusb_free_transfer(xfer);

    for(libusb_transfer * xfer : availableOutXfers)
        libusb_free_transfer(xfer);

    for(uint8_t * buf : buffers)
        delete [] buf;

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
    availableXfers.push_back(xfer);
}

void UsbDevice::sendIsoData(uint8_t ep, unsigned char * data, size_t size, uint16_t packetSize)
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

void UsbDevice::receiveIsoData(uint8_t ep, unsigned char * data, size_t size, uint16_t packetSize)
{
    size_t totalPackets = size / packetSize;
    size_t bytesToGo = size;

    while(bytesToGo > 0)
    {
        // Schedule as many packet transfers as possible
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

void UsbDevice::loopback(uint8_t inEp, uint16_t inPacketSize, uint8_t outEp, uint16_t outPacketSize)
{
    this->outEp = outEp;
    this->outPacketSize = outPacketSize;

    while(true)
    {
        // Schedule as many packet transfers as possible
        while(availableXfers.size() > 0)
        {
            size_t chunkSize = inPacketSize * NUM_PACKETS;

            libusb_transfer * xfer = availableXfers.back();
            availableXfers.pop_back();

            uint8_t * buf = buffers.back();
            buffers.pop_back();

            libusb_fill_iso_transfer(xfer, hdev, inEp, buf, chunkSize, NUM_PACKETS, loopbackPacketReceiveCB, this, 1000);
            libusb_set_iso_packet_lengths(xfer, inPacketSize);
            libusb_submit_transfer(xfer);
        }

        int ret = libusb_handle_events(NULL);
        check(ret, "libusb_handle_events()");
    }
}

void UsbDevice::loopbackPacketReceiveCB(libusb_transfer * xfer)
{
    UsbDevice * device = static_cast<UsbDevice*>(xfer->user_data);
    device->handleLoopbackPacketReceive(xfer);
}

void UsbDevice::handleLoopbackPacketReceive(libusb_transfer * xfer)
{
    // Skip this transfer if there is no output transfers available
    if(buffers.size() == 0 || availableOutXfers.size() == 0)
    {
        // return input transfer and its buffer to the pool
        buffers.push_back(xfer->buffer);
        availableXfers.push_back(xfer);
        return;
    }

    // Convert 24bit mono to 16bit stereo
    uint8_t * inputBuf = xfer->buffer;
    uint8_t * outputBuf = buffers.back();
    buffers.pop_back();

    for(int i=0; i<xfer->length/3; i++)
    {
        int16_t v = *(int16_t *)(inputBuf + i*3 + 1);
        *(int16_t *)(outputBuf + i*4) = v;
        *(int16_t *)(outputBuf + i*4 + 2) = v;
    }


    // return input transfer and its buffer to the pool
    buffers.push_back(inputBuf);
    availableXfers.push_back(xfer);

    // Schedule output transfer
    libusb_transfer * outXfer = availableOutXfers.back();
    availableOutXfers.pop_back();

    size_t chunkSize = outPacketSize * NUM_PACKETS;
    libusb_fill_iso_transfer(outXfer, hdev, outEp, outputBuf, chunkSize, NUM_PACKETS, loopbackPacketSendCB, this, 1000);
    libusb_set_iso_packet_lengths(outXfer, outPacketSize);
    libusb_submit_transfer(outXfer);
}

void UsbDevice::loopbackPacketSendCB(libusb_transfer * xfer)
{
    UsbDevice * device = static_cast<UsbDevice*>(xfer->user_data);
    device->handleLoopbackPacketSend(xfer);
}

void UsbDevice::handleLoopbackPacketSend(libusb_transfer * xfer)
{
    //return output transfer and its buffer to the pool
    buffers.push_back(xfer->buffer);
    availableOutXfers.push_back(xfer);
}
