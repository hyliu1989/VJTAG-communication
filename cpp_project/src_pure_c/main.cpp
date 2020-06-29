#include <stdlib.h>
#include <stdio.h>
#include "ftd2xx.h"
#include "jtag_tap.h"
#include "device.h"


// === The main operation =======================================================
static void SendBufOperation_BitBangBasic( BYTE *buf, int &cnt ); // The basic IO
static void SendBufOperation_ByteShiftBasic( BYTE *buf, int &cnt ); // The modified version of the above, VDR shift is using byte shift



int main()
{
    //Handle of FT2232H device port
    FT_HANDLE m_ftHandle = open_jtag_device();  // open_jtag_device() defined in device.cpp

    // define for write
    DWORD       dwCount=0;
    BYTE        sendBuf[65536];
    BYTE        readBuf[65536];
    int         cnt=0;

    if(m_ftHandle == NULL)
        goto BAD;

    // Buffer-to-send manipulation
    // (Users could switch between these two lines)
    // NOTE: in byte shift mode, the value of read buffer should not be masked by 0x01.
    SendBufOperation_BitBangBasic( sendBuf, cnt );
//  SendBufOperation_ByteShiftBasic( sendBuf, cnt );


    // Sending
    FT_Write(m_ftHandle,sendBuf,cnt,&dwCount);
    if (dwCount != (DWORD) cnt) {
        printf("Not all bytes was sent.\n");
    }

    // Reading
    FT_Read(m_ftHandle,readBuf,256,&dwCount);
    printf("Print for Bit Banging(ignore it if using Byte Shift):\n");
    for(int i = 0; i < dwCount; ++i)
        printf("%d%c",readBuf[i]&0x1,((i+1)%8==0)?'\n':'\t'); // for bit banging
    printf("\n");
    printf("Print for Byte Shift(ignore it if using Bit Banging):\n");
    for(int i = 0; i < dwCount; ++i)
        printf("%X%c",readBuf[i]/*&0x1*/,((i+1)%8==0)?'\n':'\t'); // for byte shift
    printf("\n");

    // Close
    close_jtag_device(m_ftHandle);  // close_jtag_device() defined in device.cpp

    //TODO: speed choosing
    system("pause");
    return 0;
BAD:
    system("pause");
    return 1;
}


static void SendBufOperation_BitBangBasic( BYTE *sendBuf, int &cnt ){
    bool to_read = false;
    BYTE data[256];

    // Sync the JTAG tap controller state to IDL
    common_functions_ANY_to_RST_to_IDL(sendBuf, cnt);

    // Send USER_1 instruction (0x00E) to the instruction register (IR)
    data[0] = 0;
    data[1] = 1;
    data[2] = 1;
    data[3] = 1;
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
    data[7] = 0;
    data[8] = 0;
    data[9] = 0;
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, 10, to_read);

    // Send the virtual ir: VIRTUAL_CAPTURE
    // Virtual ir as its name suggests is not an instruction of jtag. It is accomplished through jtag data registers so
    // we go to shift_dr state to send the data. The length of the VIR is 5.
    data[0] = 1;  // VIRTUAL_CAPTURE
    data[1] = 1;  // VIRTUAL_CAPTURE
    data[2] = 0;  // VIRTUAL_CAPTURE
    data[3] = 1;  // VIRTUAL_CAPTURE
    data[4] = 0;  // VJTAG device addr, 0 for the Hub
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 5, to_read);

    // Send the virtual ir: Actual instruction we want the VJTAG node to get
    data[0] = 1;  // Instruction
    data[1] = 0;  // padded 0 between the length of ir of the VJTAG instance and VIRTUAL_CAPTURE requirement
    data[2] = 0;  // padded 0 between the length of ir of the VJTAG instance and VIRTUAL_CAPTURE requirement
    data[3] = 0;  // padded 0 between the length of ir of the VJTAG instance and VIRTUAL_CAPTURE requirement
    data[4] = 1;  // VJTAG device addr, 1 for the VJTAG instance
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 5, to_read);

    // Send USER_0 instruction (0x00C) to the instruction register (IR)
    data[0] = 0;
    data[1] = 0;
    data[2] = 1;
    data[3] = 1;
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
    data[7] = 0;
    data[8] = 0;
    data[9] = 0;
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, 10, to_read);

    // Send the data we want the VJTAG device to get
    //   The data in this block will be read out in after the following block is excuted. This reading is enabled by
    //   to_read==true in the followint block.
    data[0] = 1;
    data[1] = 0;
    data[2] = 1;
    data[3] = 1;
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
    data[7] = 1;
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 8, to_read);

    // Send the data we want the VJTAG device to get
    //   The data in this block will be presented on the DE0-Nano LED.
    data[0] = 1;
    data[1] = 1;
    data[2] = 1;
    data[3] = 0;
    data[4] = 0;
    data[5] = 1;
    data[6] = 0;
    data[7] = 1;
    to_read = true;
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 8, to_read);
    to_read = false;
}

static void SendBufOperation_ByteShiftBasic( BYTE *sendBuf, int &cnt ){
    bool to_read = false;
    BYTE data[256];

    // Sync the JTAG tap controller state to IDL
    common_functions_ANY_to_RST_to_IDL(sendBuf, cnt);

    // Send USER_1 instruction (0x00E) to the instruction register (IR)
    data[0] = 0;
    data[1] = 1;
    data[2] = 1;
    data[3] = 1;
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
    data[7] = 0;
    data[8] = 0;
    data[9] = 0;
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, 10, to_read);

    // Send the virtual ir: VIRTUAL_CAPTURE
    // Virtual ir as its name suggests is not an instruction of jtag. It is accomplished through jtag data registers so
    // we go to shift_dr state to send the data. The length of the VIR is 5.
    data[0] = 1;  // VIRTUAL_CAPTURE
    data[1] = 1;  // VIRTUAL_CAPTURE
    data[2] = 0;  // VIRTUAL_CAPTURE
    data[3] = 1;  // VIRTUAL_CAPTURE
    data[4] = 0;  // VJTAG device addr, 0 for the Hub
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 5, to_read);

    // Send the virtual ir: actual instruction we want the VJTAG node to get
    data[0] = 1;  // Instruction
    data[1] = 0;  // padded 0 between the length of ir of the VJTAG instance and VIRTUAL_CAPTURE requirement
    data[2] = 0;  // padded 0 between the length of ir of the VJTAG instance and VIRTUAL_CAPTURE requirement
    data[3] = 0;  // padded 0 between the length of ir of the VJTAG instance and VIRTUAL_CAPTURE requirement
    data[4] = 1;  // VJTAG device addr, 1 for the VJTAG instance
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 5, to_read);

    // Send USER_0 instruction (0x00C) to the instruction register (IR)
    data[0] = 0;
    data[1] = 0;
    data[2] = 1;
    data[3] = 1;
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
    data[7] = 0;
    data[8] = 0;
    data[9] = 0;
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, 10, to_read);

    // shift_DR (VDR value) in Byte Shift mode
    //   Since this mode uses bytes as the unit, in order to to send 1000_1101 (8 bits), we first send 0001_1010 without
    //   the MSB (it is a 1 in this case) using the byte shift mode. This is equivalent to sending one byte. And then
    //   we go back to the Bit Banging mode and send the last bit, the MSB of 1000_1101, with TMS flag on. The last Bit
    //   Banging operation will also shift out LSB (0) of 0001_1010.
    int num_bytes = 1;
    atomic_state_trans_IDL_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);
    intiate_ByteShift( sendBuf, cnt, to_read, num_bytes );
    sendBuf[cnt++] = 0x1A;
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 1, to_read);
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_IDL(sendBuf, cnt);
}


// reference:
/*
1. Virtual JTAG (sld_virtual_jtag) Megafunction User Guide, Altera
2. http://sourceforge.net/apps/mediawiki/urjtag/index.php?title=Cable_Altera_USB-Blaster
3. http://www.amontec.com/pub/amt_ann004.pdf
*/
