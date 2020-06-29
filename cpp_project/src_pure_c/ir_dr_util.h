#ifndef IR_DR_UTIL
#define IR_DR_UTIL

#include "ftd2xx.h"

bool prepare_IR_data_USER0(BYTE *bit_data_stored_in_byte, int &length);
bool prepare_IR_data_USER1(BYTE *bit_data_stored_in_byte, int &length);
bool prepare_USER1DR_data_VIR_CAPTURE(BYTE *bit_data_stored_in_byte, int &length, int user1_dr_length);
bool prepare_USER1DR_data_Command(
    BYTE *bit_data_stored_in_byte,
    int &length,
    int command,  // LSB is always shifted in first
    int vjtag_instance_ir_width,
    int vjtag_instance_addr,
    int user1_dr_length
);

#endif
