module ledoutput(input clk,reset_n,
		 input cs,
		 input write,
		 input [7:0] DB,
		 output [7:0] LED);

	reg [7:0] value;
	
	always@(posedge clk,negedge reset_n)
	   if(~reset_n) value <=8'b00000000;
	   else if(cs&write) value <= DB;
	
	assign LED =value;
endmodule