#ifndef _USB_DEVICE_H
#define _USB_DEVICE_H

#include <stdint.h>

struct libusb_device_handle;
struct libusb_transfer;

class UsbDevice
{
    libusb_device_handle * hdev;
    bool transferInProgress;

public:
    UsbDevice(uint16_t vid, uint16_t pid);
    ~UsbDevice();

    void openInterface(uint8_t interface);
    void setAltsetting(uint8_t interface, uint8_t altsetting);

    void controlReq(uint8_t requestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data);

    void getControlAttr(bool recepient, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data);
    void setControlAttr(bool recepient, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, unsigned char * data);

    void transferIsoData(uint8_t ep, unsigned char * data, uint16_t numPackets, uint16_t packetSize);

    void closeInterface(uint8_t interface);


protected:
    static void transferCompleteCB(libusb_transfer * xfer);
    virtual void handleTransferCompleteCB(libusb_transfer * xfer);
};

#endif //_USB_DEVICE_H