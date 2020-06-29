/*
This file handles the device operation.
*/
#include <stdio.h>
#include "ftd2xx.h"
#include "jtag_tap.h"

static bool b_str_equal_first(const char *s1, const char *s2, int imax){
    for(int i = 0; i < imax; ++i){
        if (s1[i] != s2[i])
            return false;
    }
    return true;
}


FT_HANDLE open_jtag_device()
{
    // define for device
    FT_HANDLE   ftHandle = NULL;          //Handle of FT2232H device port

    // define for open
    FT_STATUS   ftStatus;
    FT_HANDLE   ftHandleTemp;
    DWORD       numDevs, iSel, Flags, ID, Type, LocId;
    char        SerialNumber[16];
    char        Description[64];

    // auto-selecting a device
    if (FT_CreateDeviceInfoList(&numDevs) == FT_OK) {
        for (iSel = 0; iSel < numDevs; iSel++) {
            ftStatus = FT_GetDeviceInfoDetail(iSel, &Flags, &Type, &ID, &LocId, SerialNumber, Description, &ftHandleTemp);
            if (ftStatus == FT_OK) {
                //printf("Dev=%i %s\n",iSel,Description);
                if(b_str_equal_first(Description,"USB-Blaster",11))
                    break;
            }
        }
        if(iSel == numDevs){ // not found a USB-Blaster device
            printf("The USB-Blaster device is not found.\n");
            return NULL;
        }
    }
    else{
        printf("Listing device error!\n");
        return NULL;
    }

    // Open
    if (iSel >= 0) {
        if (FT_Open(iSel,&ftHandle) == FT_OK) {
            FT_SetBitMode(ftHandle,0,0x40);
            FT_SetTimeouts(ftHandle,5,0);
            FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
            printf("Open successfully\n");
        }
    }
    else{
        printf("Open fail\n");
        return NULL;
    }

    // Set the timing configuration
    FT_SetBaudRate(ftHandle,FT_BAUD_460800);
    FT_SetTimeouts(ftHandle,50,0);
    FT_SetLatencyTimer(ftHandle,2);

    // Return the device pointer
    return ftHandle;
}


void close_jtag_device(FT_HANDLE ftHandle)
{
    FT_Close(ftHandle);
}

// // define for write
//     DWORD       dwCount=0;
//     BYTE        sendBuf[65536];
//     BYTE        readBuf[65536];
//     int         cnt=0;
//     bool        to_read = false;
