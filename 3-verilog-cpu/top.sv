`default_nettype none
// Empty top module

module top (
  // I/O ports
  input  logic hz100, reset,
  input  logic [29:0] pb,
  output logic [7:0] Regs15, Regs14, Regs13, Regs12, Regs11, Regs10, Regs9, Regs8,
  output logic [7:0] Regs7, Regs6, Regs5, Regs4, Regs3, Regs2, Regs1, Regs0,
  output logic [7:0] left, right, lefthigh, righthigh, leftlow, rightlow,
         ss7, ss6, ss5, ss4, ss3, ss2, ss1, ss0,
  output logic red, green, blue,

  // Ports from/to UART
  output logic [7:0] txdata,
  input  logic [7:0] rxdata,
  output logic txclk, rxclk,
  input  logic txready, rxready
);

  // Your code goes here...
  
  wire myreset = reset | pb[0];
  assign red = myreset;
  
  wire [7:0] ledoutput;
  wire [7:0] pcReg;
  wire [28:0] controlSigs;
  
	computer computer1(
		.clock( pb[9] ?  pb[29] : hz100 ),
		.reset_n(~myreset),
		.switches(pb[8:1]),
		.leds(ledoutput),
		.irReg(Regs0),
		.pswReg(Regs1[3:0]),
		.crReg(Regs2),
		.brReg(Regs3),
		.acReg(Regs4),
		.spReg(Regs5),
		.pcReg(Regs6),
		.mdrReg(Regs7),
		.marReg(Regs8),
    .controlSignals(controlSigs)
  //     .irReg(irReg),		
  //     .crReg(crReg),
		// .brReg(brReg),
		// .acReg(acReg),
		// .spReg(spReg),
		// .pcReg(pcReg)
		// .mdrReg(mdrReg),
		// .marReg(marReg),
	);

    // sevenSegDisplay inst0(data[3:0],SD0);
    // sevenSegDisplay inst1(data[7:4],SD1);
    
    assign right = ledoutput;
    assign Regs9 = ledoutput;
    ssdec ssled0(ledoutput[3:0], 1'b1, ss0);
    ssdec ssled1(ledoutput[7:4], 1'b1, ss1);

    ssdec ssled6(pcReg[3:0], 1'b1, ss6);
    ssdec ssled7(pcReg[7:4], 1'b1, ss7);
    
    assign lefthigh = controlSigs[7:0];
    assign righthigh = controlSigs[15:8];
    assign leftlow = controlSigs[23:16];
    assign rightlow[0] = controlSigs[24];
endmodule

// Add more modules down here...
module ssdec(in, en, out);
  input logic [3:0] in;
  input logic en;
  output logic [7:0] out;

  logic [15:0] pos; // ypos is the positively-asserted version of the output
  assign pos = {in[3] & in[2] & in[1] & in[0],
  in[3] & in[2] & in[1] & ~in[0],
  in[3] & in[2] & ~in[1] &  in[0], 
  in[3] & in[2] & ~in[1] & ~in[0],
  in[3] & ~in[2] &  in[1] & in[0],
  in[3] & ~in[2] &  in[1] & ~in[0],
  in[3] & ~in[2] & ~in[1] & in[0],
  in[3] & ~in[2] & ~in[1] & ~in[0],
  ~in[3] &  in[2] & in[1] & in[0],
  ~in[3] &  in[2] & in[1] & ~in[0],
  ~in[3] &  in[2] & ~in[1] &  in[0],
  ~in[3] &  in[2] & ~in[1] & ~in[0],
  ~in[3] & ~in[2] & in[1] & in[0],
  ~in[3] & ~in[2] & in[1] & ~in[0],
  ~in[3] & ~in[2] & ~in[1] & in[0],
  ~in[3] & ~in[2] & ~in[1] & ~in[0] };

  assign out[0] = en & ~(pos[1] | pos[4] | pos[11] | pos[13]);
  assign out[1] = en & ~(pos[5] | pos[6] | pos[11] | pos[12] | pos[14] | pos[15]);
  assign out[2] = en & ~(pos[2] | pos[12]| pos[14] | pos[15]);
  assign out[3] = en & ~(pos[1] | pos[4] | pos[7] | pos[9] | pos[10] | pos[15]);
  assign out[4] = en & ~(pos[1] | pos[3] | pos[5] | pos[7] | pos[9] | pos[4]);
  assign out[5] = en & ~(pos[1] | pos[2] | pos[3] | pos[7] | pos[13]);
  assign out[6] = en & ~(pos[0] | pos[1] | pos[7] | pos[12]);
  assign out[7] = 1'b0;

endmodule