#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ftd2xx.h"
#include "jtag_tap.h"
#include "device.h"
#include "ir_dr_util.h"


// === The main operation =======================================================
// The basic IO
static void SendBufOperation_BitBangBasic( BYTE *buf, int &cnt );
// The modified version of the above where VDR shift is using byte shift.
static void SendBufOperation_ByteShiftBasic( BYTE *buf, int &cnt );

// === Configuration copied from RTL report Blaster_Comm.map.rpt ================
const int VJTAG_INSTANCE_IR_WIDTH = 2;  // bits. The actual instruction register length for the VJTAG instance.
// The address value here already considers the bit shifting which reserves bits for the VIR command width. In the most
// common case, the VIR width is 4 which is the minimum required by VIR_CAPTURE command. Therefore addr 0x10 actually
// corresponds to 1 after removing the 4 least significant zeros.
const int VJTAG_INSTANCE_ADDR = 0x10;
const int USER1_DR_LENGTH = 5;


int main()
{
    //Handle of FT2232H device port
    FT_HANDLE m_ftHandle = open_jtag_device();  // open_jtag_device() defined in device.cpp

    // define for write
    DWORD       dwCount=0;
    BYTE        sendBuf[65536];
    BYTE        readBuf[65536];
    int         cnt=0;

    if(m_ftHandle == NULL){
        system("pause");
        return 1;
    }

    // User can switch between the BitBanging mode and ByteShift mode by changing this bool variable.
    bool byte_shift_mode = false;

    if(!byte_shift_mode){
        // BigBanging mode

        // Prepare buffer
        SendBufOperation_BitBangBasic(sendBuf, cnt);
        // Sending
        FT_Write(m_ftHandle,sendBuf,cnt,&dwCount);
        if (dwCount != (DWORD) cnt) {
            printf("Not all bytes was sent.\n");
        }
        // Reading
        FT_Read(m_ftHandle,readBuf,256,&dwCount);
        // Displaying
        printf("Print for Bit Banging:\n");
        for(int i = 0; i < dwCount; ++i)
            printf("%d%c",readBuf[i]&0x1,((i+1)%8==0)?'\n':'\t'); // for bit banging
        printf("\n");
    }
    else{
        // ByteShift mode

        // Prepare buffer
        SendBufOperation_ByteShiftBasic(sendBuf, cnt);
        // Sending
        FT_Write(m_ftHandle,sendBuf,cnt,&dwCount);
        if (dwCount != (DWORD) cnt) {
            printf("Not all bytes was sent.\n");
        }
        // Reading
        FT_Read(m_ftHandle,readBuf,256,&dwCount);
        // Displaying
        printf("Print for Byte Shift:\n");
        for(int i = 0; i < dwCount; ++i)
            printf("%X%c",readBuf[i],((i+1)%8==0)?'\n':'\t'); // for byte shift
        printf("\n");
    }

    // Close
    close_jtag_device(m_ftHandle);  // close_jtag_device() defined in device.cpp

    system("pause");
    return 0;
}


static void SendBufOperation_BitBangBasic( BYTE *sendBuf, int &cnt ){
    bool to_read = false;
    BYTE data[256];
    int data_length;


    // Sync the JTAG tap controller state to IDL
    common_functions_ANY_to_RST_to_IDL(sendBuf, cnt);


    // Send USER_1 instruction (0x00E) to the instruction register (IR)
    prepare_IR_data_USER1(data, data_length);
    assert(data_length == 10);
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, data_length, to_read);


    // Send the virtual instruction: VIRTUAL_CAPTURE (through the USER1 DR which is already specified above)
    // All virtual instructions are sent through USER1 DR and they are not JTAG instructions. Therefore we go to
    // shift_dr state to send the data.
    // For the usual case that USER1_DR_LENGTH == 5, the 4 least significant bits in the USER1 DR are for VIRTUAL_CAPTURE
    // and the MSB is for the address of the Hub (usual cases have only 1 bit for addresses).
    //     data[0] = 1;  // VIRTUAL_CAPTURE
    //     data[1] = 1;  // VIRTUAL_CAPTURE
    //     data[2] = 0;  // VIRTUAL_CAPTURE
    //     data[3] = 1;  // VIRTUAL_CAPTURE
    //     data[4] = 0;  // VJTAG device addr, 0 for the Hub
    prepare_USER1DR_data_VIR_CAPTURE(data, data_length, USER1_DR_LENGTH);
    assert(data_length == USER1_DR_LENGTH);
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, data_length, to_read);


    // Send the virtual instruction: Actual instruction we want the VJTAG node to get (through the USER1 DR)
    // The USER1 DR is split into three parts: actual instruction bits, padded 0's, and VJTAG address. For a usual case
    // that has 2 bits of the VJTAG instruction and 1 bit for addressing the instance, we have
    //     data[0] = 1;  // Instruction (if we want to send 0b01 as the instruction)
    //     data[1] = 0;  // Instruction (if we want to send 0b01 as the instruction)
    //     data[2] = 0;  // Padded 0 between the ir width of the VJTAG instance and VIR_CAPTURE required length (4 bits)
    //     data[3] = 0;  // Padded 0 between the ir width of the VJTAG instance and VIR_CAPTURE required length (4 bits)
    //     data[4] = 1;  // VJTAG device addr, 1 for the VJTAG instance
    int command = 0b01;
    prepare_USER1DR_data_Command(data, data_length, command, VJTAG_INSTANCE_IR_WIDTH, VJTAG_INSTANCE_ADDR, USER1_DR_LENGTH);
    assert(data_length == USER1_DR_LENGTH);
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, data_length, to_read);


    // Send USER_0 instruction (0x00C) to the instruction register (IR)
    prepare_IR_data_USER0(data, data_length);
    assert(data_length == 10);
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, data_length, to_read);


    // Send the data we want the VJTAG device to get
    //   The data in this block will be read out after the following block is excuted. This reading is enabled by
    //   to_read==true in the following block.
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
    data[4] = 1;
    data[5] = 1;
    data[6] = 0;
    data[7] = 1;
    to_read = true;
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 8, to_read);
    to_read = false;


    // Test another command (0b10) that reads the switch values
    // [IR update] Go to USER1 again in order to update the virtual instruction register (VIR)
    prepare_IR_data_USER1(data, data_length);
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, data_length, to_read);
    // VIR_CAPTURE is needed because we change the VIR
    prepare_USER1DR_data_VIR_CAPTURE(data, data_length, USER1_DR_LENGTH);
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, data_length, to_read);
    // Send the actual command (0b10)
    command = 0b10;
    prepare_USER1DR_data_Command(data, data_length, command, VJTAG_INSTANCE_IR_WIDTH, VJTAG_INSTANCE_ADDR, USER1_DR_LENGTH);
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, data_length, to_read);
    // [IR update] Go to USER0
    prepare_IR_data_USER0(data, data_length);
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, data_length, to_read);
    // Clock out the TDO to see what we read
    to_read = true;
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 8, to_read);  // the content of `data` does not matter.
    to_read = false;
}

static void SendBufOperation_ByteShiftBasic( BYTE *sendBuf, int &cnt ){
    bool to_read = false;
    BYTE data[256];
    int data_length;

    // Sync the JTAG tap controller state to IDL
    common_functions_ANY_to_RST_to_IDL(sendBuf, cnt);

    // Send USER_1 instruction (0x00E) to the instruction register (IR)
    prepare_IR_data_USER1(data, data_length);
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, data_length, to_read);

    // Send the virtual instruction: VIRTUAL_CAPTURE
    // (see SendBufOperation_BitBangBasic for details)
    prepare_USER1DR_data_VIR_CAPTURE(data, data_length, USER1_DR_LENGTH);
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, data_length, to_read);

    // Send the virtual instruction: actual instruction we want the VJTAG node to get
    // (see SendBufOperation_BitBangBasic for details)
    int command = 0b01;
    prepare_USER1DR_data_Command(data, data_length, command, VJTAG_INSTANCE_IR_WIDTH, VJTAG_INSTANCE_ADDR, USER1_DR_LENGTH);
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, data_length, to_read);

    // Send USER_0 instruction (0x00C) to the instruction register (IR)
    prepare_IR_data_USER0(data, data_length);
    common_functions_IDL_to_SIR_to_IDL(sendBuf, cnt, data, data_length, to_read);

    // Send the data we want the VJTAG device to get
    //   The data in this block will be read out after the following block is excuted. This reading is enabled by
    //   to_read==true in the following block.
    data[0] = 1;
    data[1] = 0;
    data[2] = 1;
    data[3] = 1;
    data[4] = 0;
    data[5] = 0;
    data[6] = 0;
    data[7] = 1;
    common_functions_IDL_to_SDR_to_IDL(sendBuf, cnt, data, 8, to_read);

    // shift_DR (VDR value) in Byte Shift mode
    //   Since this mode uses bytes as the unit, in order to to send 1010_1101 (8 bits), we first send 0101_1010 without
    //   the MSB (it is a 1 in this case) using the byte shift mode. This is equivalent to sending one byte. And then
    //   we go back to the Bit Banging mode and send the last bit, the MSB of 1000_1101, with TMS flag on. The last Bit
    //   Banging operation will also shift out LSB (0) of 0001_1010.
    int num_bytes = 1;
    atomic_state_trans_IDL_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);
    to_read = true;
    initiate_ByteShift( sendBuf, cnt, to_read, num_bytes );
    to_read = false;
    sendBuf[cnt++] = 0x5A;
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 1, to_read);
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_IDL(sendBuf, cnt);

    // TODO: add the readout for command 0b10.
}


// reference:
/*
1. Virtual JTAG (sld_virtual_jtag) Megafunction User Guide, Altera
2. http://sourceforge.net/apps/mediawiki/urjtag/index.php?title=Cable_Altera_USB-Blaster
3. http://www.amontec.com/pub/amt_ann004.pdf
*/
