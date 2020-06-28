#include <stdlib.h>
#include <stdio.h>
#include "ftd2xx.h"
#include "jtag_tap.h"


// === The utility functions ============================================================================
static bool bStrEqualFirst(const char *s1, const char *s2, int imax);



// === The main operation =======================================================
static void SendBufOperation_BitBangBasic( BYTE *buf, int &cnt ); // The basic IO
static void SendBufOperation_ByteShiftBasic( BYTE *buf, int &cnt ); // The modified version of the above, VDR shift is using byte shift



int main()
{
    // define for device
    FT_STATUS   m_ftStatus;         //Status defined in D2XX to indicate operation result
    FT_HANDLE   m_ftHandle = NULL;          //Handle of FT2232H device port

    // define for open
    FT_STATUS   ftStatus;
    FT_HANDLE   ftHandleTemp;
    DWORD       numDevs, iSel, Flags, ID, Type, LocId;
    char        SerialNumber[16];
    char        Description[64];

    // define for write
    DWORD       dwCount=0;
    BYTE        sendBuf[65536];
    BYTE        readBuf[65536];
    int         cnt=0;
    bool        to_read = false;

    // auto-selecting a device
    if (FT_CreateDeviceInfoList(&numDevs) == FT_OK) {
        for (iSel = 0; iSel < numDevs; iSel++) {
            ftStatus = FT_GetDeviceInfoDetail(iSel, &Flags, &Type, &ID, &LocId, SerialNumber, Description, &ftHandleTemp);
            if (ftStatus == FT_OK) {
                //printf("Dev=%i %s\n",iSel,Description);
                if(bStrEqualFirst(Description,"USB-Blaster",11))
                    break;
            }
        }
        if(iSel == numDevs){ // not found a USB-Blaster device
            printf("The USB-Blaster device is not found\n");
            goto BAD;
        }
    }
    else{
        printf("Listing device error!\n");
        goto BAD;
    }

    // open
    if (iSel >= 0) {
        if (FT_Open(iSel,&m_ftHandle) == FT_OK) {
            FT_SetBitMode(m_ftHandle,0,0x40);
            FT_SetTimeouts(m_ftHandle,5,0);
            FT_Purge(m_ftHandle, FT_PURGE_RX | FT_PURGE_TX);
            printf("Open successfully\n");
        }
    }
    else{
        printf("Open fail\n");
        goto BAD;
    }


    // Buffer-to-send manipulation
    // (Users could switch between these two lines)
    // NOTE: in byte shift mode, the value of read buffer should not be masked by 0x01.
    SendBufOperation_BitBangBasic( sendBuf, cnt );
//  SendBufOperation_ByteShiftBasic( sendBuf, cnt );


    //sending part
    FT_SetBaudRate(m_ftHandle,FT_BAUD_460800);
    FT_SetTimeouts(m_ftHandle,50,0);
    FT_SetLatencyTimer(m_ftHandle,2);
    FT_Write(m_ftHandle,sendBuf,cnt,&dwCount);
    if (dwCount != (DWORD) cnt) {
        printf("Not all bytes was sent.\n");
    }

    //reading part
    FT_Read(m_ftHandle,readBuf,256,&dwCount);
    printf("Print for Bit Banging(ignore it if using Byte Shift):\n");
    for(int i = 0; i < dwCount; ++i)
        printf("%d%c",readBuf[i]&0x1,((i+1)%8==0)?'\n':'\t'); // for bit banging
    printf("\n");
    printf("Print for Byte Shift(ignore it if using Bit Banging):\n");
    for(int i = 0; i < dwCount; ++i)
        printf("%X%c",readBuf[i]/*&0x1*/,((i+1)%8==0)?'\n':'\t'); // for byte shift
    printf("\n");

    // close
    FT_Close(m_ftHandle);

    //TODO:speed choosing
    system("pause");
    return 0;
BAD:
    system("pause");
    return 1;
}

static bool bStrEqualFirst(const char *s1, const char *s2, int imax){
    for(int i = 0; i < imax; ++i){
        if (s1[i] != s2[i])
            return false;
    }
    return true;
}


static void SendBufOperation_BitBangBasic( BYTE *sendBuf, int &cnt ){
    bool to_read = false;

    // go to reset by TMS high and clock it 5 times
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_IDL(sendBuf, cnt);

    // Hard state goes from IDL to shift IR
    atomic_state_trans_IDL_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_SIS(sendBuf, cnt);
    atomic_state_trans_SIS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SIR(sendBuf, cnt);

    // shift_IR (USER_1) 0x00E
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 0, to_read);
    // end shift IR

    // Hard state goes from Exit1 to Shift DR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);

    // shift_DR(ADDR + VIR)
    // also the VIRTUAL_CAPTURE command
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 0, to_read); // addr, 0: Hub
    // end shift DR

    // Hard state goes from Exit1 to Shift DR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);

    // shift_DR(ADDR + VIR)
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 1, to_read); // addr
    // end shift DR

    // Hard state goes from Exit1 to Shift IR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_SIS(sendBuf, cnt);
    atomic_state_trans_SIS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SIR(sendBuf, cnt);

    // shift_IR (USER_0) 0x00C
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 0, to_read);
    // end shift IR

    // Hard state goes from Exit1 to Shift DR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);

    // shift_DR (VDR value)
    //   The data of this block would be read out in the application
    //   due to the to_read=true in the next second block.
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 1, to_read);
    // end shift DR

    // Hard state goes from Exit1 to Shift DR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);

    // shift_DR (VDR value)
    //   also read the (V)DR value, which is what we sent in last operation.
    to_read = true;
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);  // location 000
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);  // location 001
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);  // location 010
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);  // location 011
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);  // location 100
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);  // location 101
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);  // location 110
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 1, to_read); // location 111
    to_read = false;
    // end shift DR

    // Hard state goes from Exit1 to Idle
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_IDL(sendBuf, cnt);
}

static void SendBufOperation_ByteShiftBasic( BYTE *sendBuf, int &cnt ){
    bool to_read = false;

    // go to reset by TMS high and clock it 5 times
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_RST(sendBuf, cnt);
    atomic_state_trans_RST_to_IDL(sendBuf, cnt);

    // Hard state goes from IDL to shift IR
    atomic_state_trans_IDL_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_SIS(sendBuf, cnt);
    atomic_state_trans_SIS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SIR(sendBuf, cnt);

    // shift_IR (USER_1) 0x00E
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 0, to_read);
    // end shift IR

    // Hard state goes from Exit1 to Shift DR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);

    // shift_DR(ADDR + VIR)
    //   also the VIRTUAL_CAPTURE command
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 0, to_read); // addr, 0: Hub
    // end shift DR

    // Hard state goes from Exit1 to Shift DR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);

    // shift_DR(ADDR + VIR)
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 1, to_read); // addr
    // end shift DR

    // Hard state goes from Exit1 to Shift IR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_SIS(sendBuf, cnt);
    atomic_state_trans_SIS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SIR(sendBuf, cnt);

    // shift_IR (USER_0) 0x00C
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 1, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_SR(sendBuf, cnt, 0, to_read);
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 0, to_read);
    // end shift IR

    // Hard state goes from Exit1 to Shift DR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);

    // shift_DR (VDR value)
    //   in Byte Shift mode, to send 1000_1101, we first send 0001_1010,
    //   then being back to Bit Banging mode, send the last bit with TMS
    //   flag in order to make state transfer.
    intiate_ByteShift( sendBuf, cnt, to_read, 1 );
    sendBuf[cnt++] = 0x1A;
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 1, to_read);
    // end shift DR

    // Hard state goes from Exit1 to Shift DR
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_SDS(sendBuf, cnt);
    atomic_state_trans_SDS_to_CAP(sendBuf, cnt);
    atomic_state_trans_CAP_to_SDR(sendBuf, cnt);

    // shift_DR (VDR value)
    //   (to_read = true) causes the reading of the (V)DR value, which is
    //   what we sent in the last operation.
    //   There is the same sending issue here.
    to_read = true;
    intiate_ByteShift( sendBuf, cnt, to_read, 1 );
    to_read = false;
    sendBuf[cnt++] = 0x08;
    atomic_state_trans_SR_to_EX1(sendBuf, cnt, 0, to_read);
    // end shift DR

    // Hard state goes from Exit1 to Idle
    atomic_state_trans_EX1_to_UPD(sendBuf, cnt);
    atomic_state_trans_UPD_to_IDL(sendBuf, cnt);
}


// reference:
/*
1. Virtual JTAG (sld_virtual_jtag) Megafunction User Guide, Altera
2. http://sourceforge.net/apps/mediawiki/urjtag/index.php?title=Cable_Altera_USB-Blaster
3. http://www.amontec.com/pub/amt_ann004.pdf
*/
