#include "ir_dr_util.h"



bool prepare_IR_data_USER0(BYTE *bit_data_stored_in_byte, int &length)
{
    /*
    Fill the array, bit_data_stored_in_byte, with the USER0 (0x00C, 10 bits) instruction.
    */
    bit_data_stored_in_byte[0] = 0;
    bit_data_stored_in_byte[1] = 0;
    bit_data_stored_in_byte[2] = 1;
    bit_data_stored_in_byte[3] = 1;
    bit_data_stored_in_byte[4] = 0;
    bit_data_stored_in_byte[5] = 0;
    bit_data_stored_in_byte[6] = 0;
    bit_data_stored_in_byte[7] = 0;
    bit_data_stored_in_byte[8] = 0;
    bit_data_stored_in_byte[9] = 0;
    length = 10;
    return true;
}

bool prepare_IR_data_USER1(BYTE *bit_data_stored_in_byte, int &length)
{
    /*
    Fill the array, bit_data_stored_in_byte, with the USER1 (0x00E, 10 bits) instruction.
    */
    bit_data_stored_in_byte[0] = 0;
    bit_data_stored_in_byte[1] = 1;
    bit_data_stored_in_byte[2] = 1;
    bit_data_stored_in_byte[3] = 1;
    bit_data_stored_in_byte[4] = 0;
    bit_data_stored_in_byte[5] = 0;
    bit_data_stored_in_byte[6] = 0;
    bit_data_stored_in_byte[7] = 0;
    bit_data_stored_in_byte[8] = 0;
    bit_data_stored_in_byte[9] = 0;
    length = 10;
    return true;
}

bool prepare_USER1DR_data_VIR_CAPTURE(BYTE *bit_data_stored_in_byte, int &length, int user1_dr_length)
{
    bit_data_stored_in_byte[0] = 1;  // VIRTUAL_CAPTURE
    bit_data_stored_in_byte[1] = 1;  // VIRTUAL_CAPTURE
    bit_data_stored_in_byte[2] = 0;  // VIRTUAL_CAPTURE
    bit_data_stored_in_byte[3] = 1;  // VIRTUAL_CAPTURE
    for(int i = 4; i < user1_dr_length; ++i)
        bit_data_stored_in_byte[i] = 0;  // VJTAG device addr, 0 for the Hub
    length = user1_dr_length;
    return true;
}


bool prepare_USER1DR_data_Command(
    BYTE *bit_data_stored_in_byte,
    int &length,
    int command,
    int vjtag_instance_ir_width,
    int vjtag_instance_addr,
    int user1_dr_length
){
    int vir_length = (vjtag_instance_ir_width > 4)? vjtag_instance_ir_width : 4;  // 4, minimum required by VIR_CAPTURE

    // Fill the user defined command
    for(int i = 0; i < vjtag_instance_ir_width; ++i){
        bit_data_stored_in_byte[i] = (command>>i) & 0b1;
    }

    // Pad 0's if needed the ir width is shorter than the length required by VIR_CAPTURE
    for(int i = vjtag_instance_ir_width; i < vir_length; ++i)
        bit_data_stored_in_byte[i] = 0;

    // Specify the address bits
    for(int i = vir_length; i < user1_dr_length; ++i)
        bit_data_stored_in_byte[i] = (vjtag_instance_addr>>i) & 0b1;  // VJTAG device addr, 1 for the VJTAG instance

    length = user1_dr_length;
    return true;
}
