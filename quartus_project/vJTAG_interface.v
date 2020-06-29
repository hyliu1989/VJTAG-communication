module vJTAG_interface (
	input tck, tdi, aclr, v_cdr, v_sdr, v_udr,
	input [1:0] ir_in,
	input [7:0] data_sent_to_pc,
	
	output reg [7:0] data_from_pc,
	output reg tdo
);
// v_cdr is virtual capture dr, which indicates the timing for loading the data that will be shift out
// to the JTAG chain for PC to read. We will use this 

reg DR0_bypass_reg; // Safeguard in case bad IR is sent through JTAG 
reg [7:0] DR1; // Date, time and revision DR.  We could make separate Data Registers for each one, but
reg [7:0] DR2;


// Bypass mode
wire select_DR0 = (ir_in==0) | (ir_in==3); // Default to 0, which is the bypass register
// Example input mode
wire select_DR1 = (ir_in==1); // Data Register 1 will collect the new LED settings from JTAG chain.
// Example output mode
wire select_DR2 = (ir_in==2); // Data Register 2 will be output to the JTAG chain for PC the read.

always @ (posedge tck or posedge aclr) begin
	if (aclr) begin
		DR0_bypass_reg <= 1'b0;
		DR1 <= 8'b00000000;
		DR2 <= 8'b00000000;
	end
	else begin
		// Update the Bypass Register always. Whether to use it in TDO or not is determined elsewhere.
		DR0_bypass_reg <= tdi;

		if(select_DR1) begin
			if(v_sdr) begin
				// VJAG is in Shift DR state & ir_in has been set to choose DR1
				DR1 <= {tdi, DR1[7:1]}; // Shifting in (and out) the data
			end
		end

		if(select_DR2) begin
			if(v_cdr) begin
				DR2 <= data_sent_to_pc;
			end
			else if(v_sdr) begin
				DR2 <= {tdi, DR2[7:1]};
			end
		end
	end
end


// Maintain the TDO Continuity
always @ (*) begin
	if (select_DR1)
		tdo <= DR1[0];
	else if (select_DR2)
		tdo <= DR2[0];
	else 
		tdo <= DR0_bypass_reg;	
end


// The v_udr signal will assert when the data has been transmitted and it's time to update the DR. We copy
// DR1 to the output LED register. Note that directly use the output register in the shifting operation
// will cause unwanted behavior as data is shifted through it
always @(negedge v_udr) begin
	data_from_pc <= DR1;
end


endmodule

