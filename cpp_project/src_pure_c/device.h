#ifndef JTAG_DEVICE_H
#define JTAG_DEVICE_H

#include "ftd2xx.h"

FT_HANDLE open_jtag_device();
void close_jtag_device(FT_HANDLE ftHandle);

#endif // JTAG_DEVICE_H
