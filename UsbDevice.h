#ifndef _USB_DEVICE_H
#define _USB_DEVICE_H

#include <stdint.h>

struct libusb_device_handle;

class UsbDevice
{
    libusb_device_handle * hdev;    

public:
    UsbDevice(uint16_t vid, uint16_t pid);
    ~UsbDevice();

    void openInterface(uint8_t interface);
    void setAltsetting(uint8_t interface, uint8_t altsetting);

    void closeInterface(uint8_t interface);
};

#endif //_USB_DEVICE_H