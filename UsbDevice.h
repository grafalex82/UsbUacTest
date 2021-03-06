#ifndef _USB_DEVICE_H
#define _USB_DEVICE_H

#include <stdint.h>
#include <vector>

namespace std
{
    class thread;
};

struct libusb_device_handle;
struct libusb_transfer;

class UsbDevice
{
    libusb_device_handle * hdev;

    std::vector<libusb_transfer *>  availableXfers;
    std::vector<libusb_transfer *>  availableOutXfers;
    std::vector<uint8_t *> buffers;

    uint8_t inEp;
    uint8_t outEp;
    uint16_t inPacketSize;
    uint16_t outPacketSize;
    std::thread * loopbackThread;

public:
    UsbDevice(uint16_t vid, uint16_t pid);
    ~UsbDevice();

    void openInterface(uint8_t interface);
    void setAltsetting(uint8_t interface, uint8_t altsetting);

    void controlReq(uint8_t requestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data);

    void getControlAttr(bool recepient, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data);
    void setControlAttr(bool recepient, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data);

    void sendIsoData(uint8_t ep, unsigned char * data, size_t size, uint16_t packetSize);
    void receiveIsoData(uint8_t ep, unsigned char * data, size_t size, uint16_t packetSize);

    void loopback(uint8_t inEp, uint16_t inPacketSize, uint8_t outEp, uint16_t outPacketSize);

    void closeInterface(uint8_t interface);


protected:
    virtual void loopbackEventLoop();

    static void transferCompleteCB(libusb_transfer * xfer);
    virtual void handleTransferCompleteCB(libusb_transfer * xfer);

    static void loopbackPacketReceiveCB(libusb_transfer * xfer);
    virtual void handleLoopbackPacketReceive(libusb_transfer * xfer);
    static void loopbackPacketSendCB(libusb_transfer * xfer);
    virtual void handleLoopbackPacketSend(libusb_transfer * xfer);
};

#endif //_USB_DEVICE_H
