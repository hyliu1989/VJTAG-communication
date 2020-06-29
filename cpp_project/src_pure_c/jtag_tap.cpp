/*
This file implement the the big-bang operation for the JTAG tap controller state transitions.

BitBanging description:
BitBanging basically means taking a byte and mapping its bits to 8 (or less) pins of a chip or assigned with special
functions. In the current case, bits 7 (MSB) and 6 are functional.

For a byte called B to be transfer in bit banging mode, each of its bits has different meaning,
1. Bit 6 (0x40) in B  the "Read bit".
2. If bit 7 (0x80) is set, this byte does not make any change to the JTAG tap controller and simply mark the begining
   of the Byte shift mode, with the first byte in such mode being the next byte. This byte additionally defines the
   number of bytes in the Byte shift mode to be (B & 0x3F).
3. For other bits, they corresponds to the JTAG pin voltages as follows:
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

Ref [broken]: http://sourceforge.net/apps/mediawiki/urjtag/index.php?title=Cable_Altera_USB-Blaster (2012)
Ref: https://forum.sparkfun.com/viewtopic.php?t=20181&start=15
*/

#import "jtag_tap.h"

#define BASE  0x0C
#define TCK   0x01
#define TMS   0x02
#define TDI   0x10
#define READ  0x40
#define SHIFT 0x80

// Convenient byte constants for all combinations of TMS(M), TDI(D) and READ(R)
//                               | READ | TDI | TMS
static const BYTE RDM000 =  BASE                    ;
static const BYTE RDM001 =  BASE              | TMS ;
static const BYTE RDM010 =  BASE        | TDI       ;
//static const BYTE RDM011= BASE        | TDI | TMS ;
static const BYTE RDM100 =  BASE | READ             ;
//static const BYTE RDM101= BASE | READ       | TMS ;
static const BYTE RDM110 =  BASE | READ | TDI       ;
//static const BYTE RDM111= BASE | READ | TDI | TMS ;


static void append_TMS0_no_data( BYTE *buf, int &cnt)
{
    /*
    This function just clocks the JTAG tap controller with TMS==0 and without any data transaction.
    */
    buf[cnt++] = RDM000;
    buf[cnt++] = RDM000 | TCK ;
}

static void append_TMS1_no_data( BYTE *buf, int &cnt)
{
    /*
    This function just clocks the JTAG tap controller with TMS==1 and without any data transaction.
    */
    buf[cnt++] = RDM001;
    buf[cnt++] = RDM001 | TCK ;
}

// IDLE and Reset related
void atomic_state_trans_IDL_to_IDL( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Idle] to [Idle]
void atomic_state_trans_RST_to_RST( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Reset] to [Reset]
void atomic_state_trans_RST_to_IDL( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Reset] to [Idle]
void atomic_state_trans_SIS_to_RST( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Select_IR_Scan] to [Reset]

// Go to the DR scan and DR flow
void atomic_state_trans_IDL_to_SDS( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Idle] to [Select_DR_Scan]
void atomic_state_trans_SDS_to_CAP( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Select_DR_Scan] to [Capture_DR]
void atomic_state_trans_CAP_to_SDR( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Capture_DR] to [Shift_DR]
void atomic_state_trans_CAP_to_EX1( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Capture_DR/IR] to [Exit1_DR/IR]
void atomic_state_trans_EX1_to_PAU( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Exit1_DR/IR] to [Pause_DR/IR]
void atomic_state_trans_EX1_to_UPD( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Exit1_DR/IR] to [Update_DR/IR]
void atomic_state_trans_PAU_to_PAU( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Pause_DR/IR] to [Pause_DR/IR]
void atomic_state_trans_PAU_to_EX2( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Pause_DR/IR] to [Exit2_DR/IR]
void atomic_state_trans_EX2_to_SDR( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Exit2_DR] to [Shift_DR]
void atomic_state_trans_EX2_to_UPD( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Exit2_DR/IR] to [Update_DR/IR]
void atomic_state_trans_UPD_to_SDS( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Update_DR/IR] to [Select_DR_Scan]
void atomic_state_trans_UPD_to_IDL( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Update_DR/IR] to [Idle]

// Go to the IR scan and IR flow. The IR flow and DR flow are identical and the following functions are aliases to make
// the meaning of the script clear.
void atomic_state_trans_SDS_to_SIS( BYTE *buf, int &cnt){ append_TMS1_no_data( buf, cnt); }  // [Select_DR_Scan] to [Select_IR_Scan]
void atomic_state_trans_SIS_to_CAP( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Select_IR_Scan] to [Capture_IR]
void atomic_state_trans_CAP_to_SIR( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Capture_IR] to [Shift_IR]
void atomic_state_trans_EX2_to_SIR( BYTE *buf, int &cnt){ append_TMS0_no_data( buf, cnt); }  // [Exit2_IR] to [Shift_IR]


//[Shift_DR/IR] to [Shift_DR/IR], i.e. shift one bit
void atomic_state_trans_SR_to_SR( BYTE *buf, int &cnt, BYTE bit_to_shift_in, bool to_read) {
    if(bit_to_shift_in == 0){
        if(!to_read){
            buf[cnt++] = RDM000;
            buf[cnt++] = RDM000 | TCK ;
        }
        else{
            buf[cnt++] = RDM100;
            buf[cnt++] = RDM000 | TCK ;
        }
    }
    else{
        if(!to_read){
            buf[cnt++] = RDM010;
            buf[cnt++] = RDM010 | TCK ;
        }
        else{
            buf[cnt++] = RDM110;
            buf[cnt++] = RDM010 | TCK ;
        }
    }
}

//[Shift_DR/IR] to [Exit1_DR/IR]
void atomic_state_trans_SR_to_EX1( BYTE *buf, int &cnt, BYTE bit_to_shift_in, bool to_read){
    atomic_state_trans_SR_to_SR(buf, cnt, bit_to_shift_in, to_read);
    buf[cnt-2] = buf[cnt-2] | TMS;
    buf[cnt-1] = buf[cnt-1] | TMS;
}


// Common functions
void common_functions_ANY_to_RST_to_IDL(BYTE *buf, int &cnt)
{
    // go to reset by TMS high and clock it 5 times
    for(int i = 0; i < 5; ++i){
        atomic_state_trans_RST_to_RST(buf, cnt);
    }

    atomic_state_trans_RST_to_IDL(buf, cnt);
}

static void common_functions_shift_data(BYTE *buf, int &cnt, BYTE bits[], int length, bool to_read, bool is_ir_shift)
{
    /*
    The state transition to shift_IR and that to shift_DR are identical except one step. This function merges the two
    and provide a handle `is_ir_shift` to distinguish the two shifts.

    The following pairs of functions have the same functionality:
    - atomic_state_trans_SIS_to_CAP / atomic_state_trans_SDS_to_CAP
    - atomic_state_trans_CAP_to_SIR / atomic_state_trans_CAP_to_SDR
    */

    // Go from IDL to shift_IR (or shift_DR)
    atomic_state_trans_IDL_to_SDS(buf, cnt);
    if(is_ir_shift){
        atomic_state_trans_SDS_to_SIS(buf, cnt);
    }
    atomic_state_trans_SIS_to_CAP(buf, cnt);

    // Go to the shift IR state only if the length of bits is nonzero
    if(length > 0){
        atomic_state_trans_CAP_to_SIR(buf, cnt);
        for(int i = 0; i < length-1; ++i)
            atomic_state_trans_SR_to_SR(buf, cnt, bits[i], to_read);
        atomic_state_trans_SR_to_EX1(buf, cnt, bits[length-1], to_read);
    }
    else{
        atomic_state_trans_CAP_to_EX1(buf, cnt);
    }

    // Go back to IDL state
    atomic_state_trans_EX1_to_UPD(buf, cnt);
    atomic_state_trans_UPD_to_IDL(buf, cnt);
}

void common_functions_IDL_to_SIR_to_IDL(BYTE *buf, int &cnt, BYTE bits[], int length, bool to_read)
{
    return common_functions_shift_data(buf, cnt, bits, length, to_read, true);
}

void common_functions_IDL_to_SDR_to_IDL(BYTE *buf, int &cnt, BYTE bits[], int length, bool to_read)
{
    return common_functions_shift_data(buf, cnt, bits, length, to_read, false);
}


// This bit banging indicates changing to Byte shift mode
void intiate_ByteShift(BYTE *buf, int &cnt, bool to_read, unsigned nbytes){
    BYTE base = (SHIFT) | (to_read? READ:0) | (nbytes & 0x3F);
    buf[cnt++] = base;
}
