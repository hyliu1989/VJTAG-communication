#include <stdlib.h>
#include <stdio.h>
#include "ftd2xx.h"

// JTAG basic description
/* ref: Virtual JTAG (sld_virtual_jtag) Megafunction User Guide, Altera
― Test data in (TDI), used to shift data into the IR and DR shift register chains[captured at rising edge of TCK in shifting states]
― Test data out (TDO), used to shift data out of the IR and DR shift register chains [change at falling edge of TCK in shifting states]
― Test mode select (TMS), used as an input into the TAP controller
― TCK, used as the clock source for the JTAG circuitry
― TRST resets the TAP controller. This is an optional input pin defined by the 1149.1 standard.
  (The TRST pin is not present in the Cyclone device family.)
*/

// BitBanging description:
/* ref: http://sourceforge.net/apps/mediawiki/urjtag/index.php?title=Cable_Altera_USB-Blaster
For a byte called B to be transfer,
1. Remember bit 6 (0x40) in B as the "Read bit".
2. If bit 7 (0x80) is set, switch to Byte shift mode for the coming X bytes ( X := B & 0x3F ), and don't do anything else now.
3. Otherwise, set the JTAG signals as follows:
    1) TCK/DCLK high if bit 0 was set (0x01), otherwise low
    2) TMS/nCONFIG high if bit 1 was set (0x02), otherwise low
    3) nCE high if bit 2 was set (0x04), otherwise low
    4) nCS high if bit 3 was set (0x08), otherwise low
    5) TDI/ASDI/DATAO high if bit 4 was set (0x10), otherwise low
    6) Output Enable/LED active if bit 5 was set (0x20), otherwise low
4. If "Read bit" (0x40) was set, record the state of TDO(CONF_DONE) and DATAOUT/(nSTATUS) pins and
   put them as a byte( (DATAOUT<<1)|TDO) in the output FIFO _to_ the host.

    7        6        5        4        3        2        1        0
   Bit             output
  shift     read   enable     TDI      nCS      nCE      TMS      TCK
   mode
*/

#define BASE  0x0C
#define TCK   0x01
#define TMS   0x02
#define TDI   0x10
#define READ  0x40
#define SHIFT 0x80

// === The utility functions ============================================================================
static bool bStrEqualFirst(const char *s1, const char *s2, int imax);

// naming: if both DR and IR have a similar transition, I just omit the D and I in the function name.
static void SendBufUtil_BB_to_ByteShift( BYTE *buf, int &cnt, bool toRead, unsigned nbytes ); // the bit banging indicates changing to Byte shift mode
static void SendBufUtil_BB_Simple_TMS_0( BYTE *buf, int &cnt, bool toRead );
static void SendBufUtil_BB_Simple_TMS_1( BYTE *buf, int &cnt, bool toRead );
static void SendBufUtil_BBStateTrans_SR_to_SR  ( BYTE *buf, int &cnt, BYTE bit_to_shift_in, bool toRead ); // change state from [Shift_DR/IR] to [Shift_DR/IR], i.e. shift one bit
static void SendBufUtil_BBStateTrans_SR_to_EX1 ( BYTE *buf, int &cnt, BYTE bit_to_shift_in, bool toRead ); // change state from [Shift_DR/IR] to [Exit1_DR/IR]

static void SendBufUtil_BBStateTrans_IDL_to_IDL( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead ); } // change state from [Run_Test/Idle] to [Run_Test/Idle]
static void SendBufUtil_BBStateTrans_IDL_to_SDS( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Run_Test/Idle] to [Select_DR_Scan]
static void SendBufUtil_BBStateTrans_SDS_to_SIS( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Select_DR_Scan] to [Select_IR_Scan]
static void SendBufUtil_BBStateTrans_SS_to_CAP( BYTE *buf, int &cnt, bool toRead ){  SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead ); } // change state from [Select_DR/IR_Scan] to [Capture_DR/IR]
static void SendBufUtil_BBStateTrans_SIS_to_RST( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Select_IR_Scan] to [Test_Logic/Reset]
static void SendBufUtil_BBStateTrans_CAP_to_SR( BYTE *buf, int &cnt, bool toRead ){  SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead ); } // change state from [Capture_DR/IR] to [Shift_DR/IR]
static void SendBufUtil_BBStateTrans_CAP_to_EX1( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Capture_DR/IR] to [Exit1_DR/IR]
static void SendBufUtil_BBStateTrans_EX1_to_PAU( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead ); } // change state from [Exit1_DR/IR] to [Pause_DR/IR]
static void SendBufUtil_BBStateTrans_EX1_to_UPD( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Exit1_DR/IR] to [Update_DR/IR]
static void SendBufUtil_BBStateTrans_PAU_to_PAU( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead ); } // change state from [Pause_DR/IR] to [Pause_DR/IR]
static void SendBufUtil_BBStateTrans_PAU_to_EX2( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Pause_DR/IR] to [Exit2_DR/IR]
static void SendBufUtil_BBStateTrans_EX2_to_SR( BYTE *buf, int &cnt, bool toRead ){  SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead ); } // change state from [Exit2_DR/IR] to [Shift_DR/IR]
static void SendBufUtil_BBStateTrans_EX2_to_UPD( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Exit2_DR/IR] to [Update_DR/IR]
static void SendBufUtil_BBStateTrans_UPD_to_SDS( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Update_DR/IR] to [Select_DR_Scan]
static void SendBufUtil_BBStateTrans_UPD_to_IDL( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead ); } // change state from [Update_DR/IR] to [Run_Test/Idle]
static void SendBufUtil_BBStateTrans_RST_to_RST( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead ); } // change state from [Test_Logic/Reset] to [Test_Logic/Reset]
static void SendBufUtil_BBStateTrans_RST_to_IDL( BYTE *buf, int &cnt, bool toRead ){ SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead ); } // change state from [Test_Logic/Reset] to [Run_Test/Idle]


// === The operation =======================================================
static void SendBufOperation_BitBangBasic( BYTE *buf, int &cnt ); // The basic IO
static void SendBufOperation_ByteShiftBasic( BYTE *buf, int &cnt ); // The modified version of the above, VDR shift is using byte shift



int main()
{
	// define for device
	FT_STATUS   m_ftStatus;			//Status defined in D2XX to indicate operation result
	FT_HANDLE   m_ftHandle = NULL;			//Handle of FT2232H device port

	// define for open
    FT_STATUS	ftStatus;
    FT_HANDLE	ftHandleTemp;
    DWORD		numDevs, iSel, Flags, ID, Type, LocId;
    char		SerialNumber[16];
    char		Description[64];

	// define for write
	DWORD       dwCount=0;
	BYTE        sendBuf[65536];
	BYTE        readBuf[65536];
	int         cnt=0;
	bool        toRead = false;

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
//	SendBufOperation_ByteShiftBasic( sendBuf, cnt );


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

static void SendBufUtil_BB_to_ByteShift( BYTE *buf, int &cnt, bool toRead, unsigned nbytes ){
	BYTE base = (SHIFT) | (toRead? READ:0) | (nbytes & 0x3F);
	buf[cnt++] = base;
}

static void SendBufUtil_BB_Simple_TMS_0( BYTE *buf, int &cnt, bool toRead ){
	BYTE base  = (BASE) & (~TMS) & (~TCK);
	//buf[cnt++] = base | (toRead? READ:0);
	//buf[cnt++] = base | (toRead? READ:0) | TCK ;
	//buf[cnt++] = base | (toRead? READ:0);

	buf[cnt++] = base | (toRead? READ:0);
	buf[cnt++] = base | TCK ;
	buf[cnt++] = base ;
}

static void SendBufUtil_BB_Simple_TMS_1( BYTE *buf, int &cnt, bool toRead ){
	BYTE base  = (BASE | TMS) & (~TCK);
	//buf[cnt++] = base | (toRead? READ:0);
	//buf[cnt++] = base | (toRead? READ:0) | TCK ;
	//buf[cnt++] = base | (toRead? READ:0);

	buf[cnt++] = base | (toRead? READ:0);
	buf[cnt++] = base | TCK ;
	buf[cnt++] = base ;
}

// change state from [Shift_DR/IR] to [Shift_DR/IR], i.e. shift one bit
static void SendBufUtil_BBStateTrans_SR_to_SR( BYTE *buf, int &cnt, BYTE bit_to_shift_in, bool toRead) {
	SendBufUtil_BB_Simple_TMS_0( buf, cnt, toRead );
	if(bit_to_shift_in != 0){
		buf[cnt-3] = buf[cnt-3] | TDI;
		buf[cnt-2] = buf[cnt-2] | TDI;
		buf[cnt-1] = buf[cnt-1] | TDI;
	}
}

// change state from [Shift_DR/IR] to [Exit1_DR/IR]
static void SendBufUtil_BBStateTrans_SR_to_EX1( BYTE *buf, int &cnt, BYTE bit_to_shift_in, bool toRead ){
	SendBufUtil_BB_Simple_TMS_1( buf, cnt, toRead );
	if(bit_to_shift_in != 0){
		buf[cnt-3] = buf[cnt-3] | TDI;
		buf[cnt-2] = buf[cnt-2] | TDI;
		buf[cnt-1] = buf[cnt-1] | TDI;
	}
}




static void SendBufOperation_BitBangBasic( BYTE *sendBuf, int &cnt ){
	bool toRead = false;

	// go to reset by TMS high and clock it 5 times
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_IDL(sendBuf, cnt, toRead);

    // Hard state goes from IDL to shift IR
	SendBufUtil_BBStateTrans_IDL_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SDS_to_SIS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_IR (USER_1) 0x00E
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 0, toRead);
	// end shift IR

    // Hard state goes from Exit1 to Shift DR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_DR(ADDR + VIR)
	// also the VIRTUAL_CAPTURE command
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 0, toRead); // addr, 0: Hub
	// end shift DR

    // Hard state goes from Exit1 to Shift DR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_DR(ADDR + VIR)
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 1, toRead); // addr
	// end shift DR

    // Hard state goes from Exit1 to Shift IR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SDS_to_SIS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_IR (USER_0) 0x00C
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 0, toRead);
	// end shift IR

    // Hard state goes from Exit1 to Shift DR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_DR (VDR value)
	//   The data of this block would be read out in the application
	//   due to the toRead=true in the next second block.
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 1, toRead);
	// end shift DR

    // Hard state goes from Exit1 to Shift DR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_DR (VDR value)
	//   also read the (V)DR value, which is what we sent in last operation.
	toRead = true;
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);  // location 000
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);  // location 001
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);  // location 010
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);  // location 011
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);  // location 100
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);  // location 101
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);  // location 110
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 1, toRead); // location 111
	toRead = false;
	// end shift DR

	// Hard state goes from Exit1 to Idle
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_IDL(sendBuf, cnt, toRead);
}

static void SendBufOperation_ByteShiftBasic( BYTE *sendBuf, int &cnt ){
	bool toRead = false;

	// go to reset by TMS high and clock it 5 times
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_RST(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_RST_to_IDL(sendBuf, cnt, toRead);

    // Hard state goes from IDL to shift IR
	SendBufUtil_BBStateTrans_IDL_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SDS_to_SIS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_IR (USER_1) 0x00E
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 0, toRead);
	// end shift IR

    // Hard state goes from Exit1 to Shift DR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_DR(ADDR + VIR)
	//   also the VIRTUAL_CAPTURE command
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 0, toRead); // addr, 0: Hub
	// end shift DR

    // Hard state goes from Exit1 to Shift DR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_DR(ADDR + VIR)
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 1, toRead); // addr
	// end shift DR

    // Hard state goes from Exit1 to Shift IR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SDS_to_SIS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_IR (USER_0) 0x00C
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 1, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_SR(sendBuf, cnt, 0, toRead);
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 0, toRead);
	// end shift IR

    // Hard state goes from Exit1 to Shift DR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_DR (VDR value)
	//   in Byte Shift mode, to send 1000_1101, we first send 0001_1010,
	//   then being back to Bit Banging mode, send the last bit with TMS
	//	 flag in order to make state transfer.
	SendBufUtil_BB_to_ByteShift( sendBuf, cnt, toRead, 1 );
	sendBuf[cnt++] = 0x1A;
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 1, toRead);
	// end shift DR

    // Hard state goes from Exit1 to Shift DR
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_SDS(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_SS_to_CAP(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_CAP_to_SR(sendBuf, cnt, toRead);

	// shift_DR (VDR value)
	//   (toRead = true) causes the reading of the (V)DR value, which is
	//   what we sent in the last operation.
	//   There is the same sending issue here.
	toRead = true;
	SendBufUtil_BB_to_ByteShift( sendBuf, cnt, toRead, 1 );
	toRead = false;
	sendBuf[cnt++] = 0x08;
	SendBufUtil_BBStateTrans_SR_to_EX1(sendBuf, cnt, 0, toRead);
	// end shift DR

	// Hard state goes from Exit1 to Idle
	SendBufUtil_BBStateTrans_EX1_to_UPD(sendBuf, cnt, toRead);
	SendBufUtil_BBStateTrans_UPD_to_IDL(sendBuf, cnt, toRead);
}


// reference:
/*
1. Virtual JTAG (sld_virtual_jtag) Megafunction User Guide, Altera
2. http://sourceforge.net/apps/mediawiki/urjtag/index.php?title=Cable_Altera_USB-Blaster
3. http://www.amontec.com/pub/amt_ann004.pdf
*/
