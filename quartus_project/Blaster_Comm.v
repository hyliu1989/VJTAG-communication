module Blaster_Comm(
    //////////// CLOCK //////////
    input                           CLOCK_50,
    //////////// LED //////////
    output [7:0]                    LED,
    //////////// KEY //////////
    input  [1:0]                    KEY,
    //////////// SW //////////
    input  [3:0]                    SW,
    //////////// SDRAM //////////
    output [12:0]                   DRAM_ADDR,
    output [1:0]                    DRAM_BA,
    output                          DRAM_CAS_N,
    output                          DRAM_CKE,
    output                          DRAM_CLK,
    output                          DRAM_CS_N,
    inout  [15:0]                   DRAM_DQ,
    output [1:0]                    DRAM_DQM,
    output                          DRAM_RAS_N,
    output                          DRAM_WE_N,
    //////////// EPCS //////////
    //output                          EPCS_ASDO,
    //input                           EPCS_DATA0,
    //output                          EPCS_DCLK,
    //output                          EPCS_NCSO,

    //////////// Accelerometer and EEPROM //////////
    output                          G_SENSOR_CS_N,
    input                           G_SENSOR_INT,
    output                          I2C_SCLK,
    inout                           I2C_SDAT,

    //////////// ADC //////////
    output                          ADC_CS_N,
    output                          ADC_SADDR,
    output                          ADC_SCLK,
    input                           ADC_SDAT,

    //////////// 2x13 GPIO Header //////////
    inout  [12:0]                   GPIO_2,
    input  [2:0]                    GPIO_2_IN,

    //////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
    inout  [33:0]                   GPIO_0_D,
    input  [1:0]                    GPIO_0_IN,

    //////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
    inout  [33:0]                   GPIO_1_D,
    input  [1:0]                    GPIO_1_IN
);

//=======================================================
//  Test Code
//=======================================================
// The signals you need to watch are not commented
assign GPIO_0_D[0] = vjtag_tck;
assign GPIO_0_D[1] = vjtag_tdi;
//assign GPIO_0_D[2] = st_idle;
//assign GPIO_0_D[3] = st_sdrs;
//assign GPIO_0_D[4] = st_sirs;
//assign GPIO_0_D[5] = st_capt_i|st_capt_d;
assign GPIO_0_D[6] = st_shift_i|st_shift_d;
//assign GPIO_0_D[7] = st_exit1_i|st_exit1_d;
//assign GPIO_0_D[8] = st_pause_i|st_pause_d;
//assign GPIO_0_D[9] = st_exit2_i|st_exit2_d;
//assign GPIO_0_D[10] = st_upda_i|st_upda_d;
assign GPIO_0_D[11] = tms;
assign GPIO_0_D[12] = vjtag_irin;
//assign GPIO_0_D[13] = vjtag_sdr;
//assign GPIO_0_D[14] = vjtag_udr;
assign GPIO_0_D[15] = vjtag_tdo;

//=======================================================
//  REG/WIRE declarations
//=======================================================
reg          rst_n;
wire [7:0]   list_of_tf;

wire         vjtag_tck;
wire         vjtag_tdi;
wire [1:0]   vjtag_irin;
wire         vjtag_cdr;
wire         vjtag_cir;
wire         vjtag_e1dr;
wire         vjtag_e2dr;
wire         vjtag_pdr;
wire         vjtag_sdr;
wire         vjtag_udr;
wire         vjtag_uir;
wire         vjtag_tdo;
wire         st_idle;
wire         st_sdrs;
wire         st_sirs;
wire         st_capt_i,st_capt_d;
wire         st_shift_i,st_shift_d;
wire         st_exit1_i,st_exit1_d;
wire         st_pause_i,st_pause_d;
wire         st_exit2_i,st_exit2_d;
wire         st_upda_i,st_upda_d;
wire         tms;


//=======================================================
//  Structural coding
//=======================================================
assign LED[7:0] = list_of_tf;

vjtag vjtag0(
	.ir_out(),
	.tdo(vjtag_tdo),
	.ir_in(vjtag_irin),
	.tck(vjtag_tck),
	.tdi(vjtag_tdi),
	.virtual_state_cdr(vjtag_cdr),
	.virtual_state_cir(vjtag_cir),
	.virtual_state_e1dr(vjtag_e1dr),
	.virtual_state_e2dr(vjtag_e2dr),
	.virtual_state_pdr(vjtag_pdr),
	.virtual_state_sdr(vjtag_sdr),
	.virtual_state_udr(vjtag_udr),
	.virtual_state_uir(vjtag_uir),
	
	.jtag_state_cdr(st_capt_d),
	.jtag_state_cir(st_capt_i),
	.jtag_state_e1dr(st_exit1_d),
	.jtag_state_e1ir(st_exit1_i),
	.jtag_state_e2dr(st_exit2_d),
	.jtag_state_e2ir(st_exit2_i),
	.jtag_state_pdr(st_pause_d),
	.jtag_state_pir(st_pause_i),
	.jtag_state_rti(st_idle),
	.jtag_state_sdr(st_shift_d),
	.jtag_state_sdrs(st_sdrs),
	.jtag_state_sir(st_shift_i),
	.jtag_state_sirs(st_sirs),
	.jtag_state_tlr(),
	.jtag_state_udr(st_upda_d),
	.jtag_state_uir(st_upda_i),
	.tms(tms)
);

vJTAG_interface jtag0(
	.tck(vjtag_tck),
	.tdi(vjtag_tdi),
	.aclr(),
	.ir_in(vjtag_irin),
	.v_cdr(vjtag_cdr),
	.v_sdr(vjtag_sdr),
	.v_udr(vjtag_udr),
	.data_sent_to_pc({SW,SW}),
	
	.data_from_pc(list_of_tf),
	.tdo(vjtag_tdo)
);

endmodule
