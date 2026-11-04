module memory(input clk,reset_n,
	      input cs_n,
	      input read,
	      input write,
	      input [7:0] Addr,
	      input [7:0] DBin,
	      output [7:0] DBout
		);
	
	reg [7:0] RAM[255:0];
	
	assign DBout = (~cs_n&read) ? RAM[Addr] : 8'b00000000;
	
	always@(posedge clk,negedge reset_n)
		if(~reset_n) 
			begin
//测试用例22：
//目标：验证“PUSH_AC”, “POP_AC”指令的状态设计和输出信号是否能够完成指令的功能
//测试输入：无
//预期结果：观察输出设备，即8位LED指示灯，其结果应该等于C3
RAM[8'H00] <= 8'H13; //MOV_AC  C3
RAM[8'H01] <= 8'HC3;
RAM[8'H02] <= 8'H02; //PUSH_AC
RAM[8'H03] <= 8'H13; //MOV_AC  00
RAM[8'H04] <= 8'H00;
RAM[8'H05] <= 8'H03; //POP_AC
RAM[8'H06] <= 8'H12; //OUT 00
RAM[8'H07] <= 8'H00;
RAM[8'H08] <= 8'H10; //JMP 00
RAM[8'H09] <= 8'H00;

//测试用例23：
//目标：验证“CALL addr”, “RET”指令的状态设计和输出信号是否能够完成指令的功能
//测试输入：在输入设备上，拨动8位开关Switch
//预期结果：观察输出设备，即8位LED指示灯，其结果应该等于开关输入
RAM[8'H00] <= 8'H04; //CALL 06  
RAM[8'H01] <= 8'H06;
RAM[8'H02] <= 8'H12; //OUT 00
RAM[8'H03] <= 8'H00;
RAM[8'H04] <= 8'H10; //JMP 00
RAM[8'H05] <= 8'H00;
RAM[8'H06] <= 8'H11; //IN  01
RAM[8'H07] <= 8'H01; 
RAM[8'H08] <= 8'H05; //RET











			end
		else
			if(~cs_n&write) RAM[Addr] <= DBin;
endmodule