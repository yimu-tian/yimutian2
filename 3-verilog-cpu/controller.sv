module controller(	input         	clk, reset_n,//时钟和复位信号
			    input	[3:0]		PSWwire,
				input 	[7:0]		IRwire,
				output	reg 		MARLD, //以下是控制器输出的控制信号
				output	reg 		MDRLD,
				output	reg 		MDRbus,
				output	reg 		ACLD,
				output	reg 		ACbus,
				output	reg [3:0]	ALUsel,
				output	reg 		BRLD,
				output	reg 		BRbus,
				output	reg 		CRLD,
				output	reg 		CRbus,
				output	reg 		SPLD,
				output	reg 		SPINC,
				output	reg 		SPDEC,
				output	reg 		SPbus,
				output	reg 		PCLD,
				output	reg 		PCbus,
				output	reg 		PCinc,
				output	reg 		IRLD,
				output	reg 		PSWLD,
				output	reg 		write,
				output  reg     read,
				output	reg		IO_M); //如上是模块定义
	parameter Reset =7'd0;		
	parameter Fetch1 = 7'd1; //取指令的第1个状态Fetch1的编码
	parameter Fetch2 = 7'd2; //取指令的第2个状态Fetch2的编码
	parameter Fetch3 = 7'd3; //取指令的第3个状态Fetch3的编码
  parameter LD1 = 7'd4; //LD指令的状态编码
	parameter LD2 = 7'd5; 
	parameter LD3 = 7'd6; 
	parameter LD4 = 7'd7; 
	parameter LD5 = 7'd8; 
	parameter ST1 = 7'd9; //ST指令的状态编码
	parameter ST2=7'd10;
	parameter ST3=7'd11;
	parameter ST4=7'd12;
	parameter ST5=7'd13;
	parameter PUSH_AC1=7'd14;
	parameter PUSH_AC2=7'd15;
	parameter PUSH_AC3=7'd16;
	parameter POP_AC1=7'd17;
	parameter POP_AC2=7'd18;
	parameter POP_AC3=7'd19;
	parameter CALL1=7'd20;
	parameter CALL2=7'd21;
	parameter CALL3=7'd22;
	parameter CALL4=7'd23;
	parameter CALL5=7'd24;
	parameter CALL6=7'd25;
	parameter RET1=7'd26;
	parameter RET2=7'd27;
	parameter RET3=7'd28;
	parameter ADD_AC1=7'd29;
	parameter ADD_AC2=7'd30;
	parameter ADD_AC3=7'd31;
	parameter ADD_AC4=7'd32;
	parameter SUB_AC1=7'd33;
	parameter SUB_AC2=7'd34;
	parameter SUB_AC3=7'd35;
	parameter SUB_AC4=7'd36;
	parameter AND_AC1=7'd37;
	parameter AND_AC2=7'd38;
	parameter AND_AC3=7'd39;
	parameter AND_AC4=7'd40;
	parameter OR_AC1=7'd41;
	parameter OR_AC2=7'd42;
	parameter OR_AC3=7'd43;
	parameter OR_AC4=7'd44;
	parameter XOR_AC1=7'd45;
	parameter XOR_AC2=7'd46;
	parameter XOR_AC3=7'd47;
	parameter XOR_AC4=7'd48;
	parameter	NOT_AC1	=7'd49;
	parameter	JC1		=7'd50;
	parameter	JC2		=7'd51;
	parameter	JC3		=7'd52;
	parameter	JZ1		=7'd53;
	parameter	JZ2		=7'd54;
	parameter	JZ3		=7'd55;
	parameter	JO1		=7'd56;
	parameter	JO2		=7'd57;
	parameter	JO3		=7'd58;
	parameter	JS1		=7'd59;
	parameter	JS2		=7'd60;
	parameter	JS3		=7'd61;
	parameter	JMP1	=7'd62;
	parameter	JMP2	=7'd63;
	parameter	IN1		=7'd64;
	parameter IN2=7'd65;
	parameter IN3 = 7'd66;
	parameter IN4 = 7'd67;
	parameter OUT1 = 7'd68;
	parameter OUT2 = 7'd69;
	parameter OUT3 = 7'd70;
	parameter OUT4 = 7'd71;
	parameter MOV_AC1 = 7'd72;
	parameter MOV_AC2 = 7'd73;
	parameter MOV_AC3 = 7'd74;
	parameter MOV_SP1 = 7'd75;
	parameter MOV_SP2 = 7'd76;
	parameter MOV_SP3 = 7'd77;
	parameter MOV_BR1 = 7'd78;
	parameter MOV_BR2 = 7'd79;
	parameter MOV_BR3 = 7'd80;
	parameter MOV_CR1=7'd81;
	parameter MOV_CR2=7'd82;
	parameter MOV_CR3=7'd83;
	parameter MOV_SP2AC1=7'd84;
	parameter MOV_BR2AC1=7'd85;
	parameter MOV_CR2AC1=7'd86;
	parameter MOV_AC2SP1=7'd87;
	parameter MOV_AC2BR1=7'd88;
	parameter MOV_AC2CR1=7'd89;
	parameter LDI1=7'd90;
	parameter LDI2=7'd91;
	parameter STI1=7'd92;
	parameter STI2=7'd93;
	parameter INC_AC1=7'd94;
	parameter DEC_AC1=7'd95;




	parameter IR_LD = 8'h00; //LD 指令的编码00H
	parameter IR_ST = 8'h01; //ST 指令的编码01H
	parameter IR_PUSH_AC = 8'h02; //PUSH_AC 指令的编码02H
	parameter IR_POP_AC = 8'h03;
	parameter IR_CALL = 8'h04;
	parameter IR_RET = 8'h05;
	parameter IR_ADD_AC = 8'h06;
	parameter IR_SUB_AC = 8'h07;
	parameter IR_AND_AC = 8'h08;
	parameter IR_OR_AC = 8'h09;
	parameter IR_XOR_AC = 8'h0A;
	parameter IR_NOT_AC = 8'h0B;
	parameter IR_JC = 8'h0C;
	parameter IR_JZ = 8'h0D;
	parameter IR_JO = 8'h0E;
	parameter IR_JS = 8'h0F;
	parameter IR_JMP = 8'h10;
	parameter IR_IN = 8'h11;
	parameter IR_OUT = 8'h12;
	parameter IR_MOV_AC = 8'h13;
	parameter IR_MOV_SP = 8'h14;
	parameter IR_MOV_BR = 8'h15;
	parameter IR_MOV_CR = 8'h16;
	parameter IR_MOV_SP2AC = 8'h17;
	parameter IR_MOV_BR2AC = 8'h18;
	parameter IR_MOV_CR2AC = 8'h19;
	parameter IR_MOV_AC2SP = 8'h1A;
	parameter IR_MOV_AC2BR = 8'h1B;
	parameter IR_MOV_AC2CR = 8'h1C;
	parameter IR_LDI = 8'h1D;
	parameter IR_STI = 8'h1E;
	parameter IR_INC_AC = 8'h1F;
	parameter IR_DEC_AC = 8'h20;
// 	parameter NOP = 8'h21;
// 	parameter HALT = 8'h22;
// 	parameter CALL_BR = 8'h23;
// 	parameter JMP_BR = 8'h24;
// 	parameter JC_BR = 8'h25;
// 	parameter JZ_BR = 8'h26;
// 	parameter JO_BR = 8'h27;
// 	parameter JS_BR= 8'h28;
// 	parameter CMP_AC2CR = 8'h29;
// 	parameter TEST_AC2CR = 8'h2A;
// 	parameter SHL_ACC = 8'h2B;
// 	parameter SHR_ACC = 8'h2C;
// 	parameter SAR_ACC = 8'h2D;
			
			
			
	reg [6:0] state,nextstate;

	always @(posedge clk,negedge reset_n)
	if (~reset_n) state <= Reset;
	else state <= nextstate;
				
	always @(*)
	case (state)
		Reset,
		Fetch1, 
		Fetch2, 
		LD1, 
		LD2, 
		LD3,
		LD4, 
		ST1, 
		ST2,
		ST3,
		ST4,
		PUSH_AC1,
		PUSH_AC2,
		POP_AC1,
		POP_AC2,
		CALL1,
		CALL2,
		CALL3,
		CALL4,
		CALL5,
		RET1,
		RET2,
		ADD_AC1,
		ADD_AC2,
		ADD_AC3,
		SUB_AC1,
		SUB_AC2,
		SUB_AC3,
		AND_AC1,
		AND_AC2, 
		AND_AC3, 
		OR_AC1,
		OR_AC2, 
		OR_AC3, 
		XOR_AC1, 
		XOR_AC2, 
		XOR_AC3,
		JC1,
		JZ1, 
		JO1,
		JS1,
		JMP1,
		IN1,
		IN2,
		IN3,
		OUT1,
		OUT2,
		OUT3,
		MOV_AC1,
		MOV_AC2,
		MOV_SP1,
		MOV_SP2,
		MOV_BR1, 
		MOV_BR2,
		MOV_CR1,
		MOV_CR2,
		LDI1,
		STI1:
      nextstate=state +1'b1;

		Fetch3: 
		begin
		  if(IRwire[7:0]==IR_LD) nextstate=LD1;
		  else if(IRwire[7:0]==IR_ST) nextstate=ST1;
		  else if(IRwire[7:0]==IR_PUSH_AC) nextstate=PUSH_AC1;
		  else if(IRwire[7:0]==IR_POP_AC) nextstate=POP_AC1;
		  else if(IRwire[7:0]==IR_CALL) nextstate=CALL1;
		  else if(IRwire[7:0]==IR_RET)         nextstate=RET1;
		  else if(IRwire[7:0]==IR_ADD_AC)      nextstate=ADD_AC1;
		  else if(IRwire[7:0]==IR_SUB_AC)      nextstate=SUB_AC1;
		  else if(IRwire[7:0]==IR_AND_AC)      nextstate=AND_AC1;
		  else if(IRwire[7:0]==IR_OR_AC)       nextstate=OR_AC1;
		  else if(IRwire[7:0]==IR_XOR_AC)      nextstate=XOR_AC1;
		  else if(IRwire[7:0]==IR_NOT_AC)      nextstate=NOT_AC1;
		  else if(IRwire[7:0]==IR_JC)      nextstate=  PSWwire[3] ? JC1:JC3;
		  else if(IRwire[7:0]==IR_JZ)      nextstate=  PSWwire[1] ? JZ1:JZ3;
		  else if(IRwire[7:0]==IR_JO)      nextstate=  PSWwire[2] ? JO1:JO3;
		  else if(IRwire[7:0]==IR_JS)      nextstate=  PSWwire[0] ? JS1:JS3;
		  else if(IRwire[7:0]==IR_JMP)      nextstate=JMP1;
		  else if(IRwire[7:0]==IR_IN)      nextstate=IN1;
		  else if(IRwire[7:0]==IR_OUT)      nextstate=OUT1;
		  else if(IRwire[7:0]==IR_MOV_AC)      nextstate=MOV_AC1;
		  else if(IRwire[7:0]==IR_MOV_SP)      nextstate=MOV_SP1;
		  else if(IRwire[7:0]==IR_MOV_BR)      nextstate=MOV_BR1;
		  else if(IRwire[7:0]==IR_MOV_CR)      nextstate=MOV_CR1;
		  else if(IRwire[7:0]==IR_MOV_SP2AC)      nextstate=MOV_SP2AC1;
		  else if(IRwire[7:0]==IR_MOV_BR2AC)      nextstate=MOV_BR2AC1;
		  else if(IRwire[7:0]==IR_MOV_CR2AC)      nextstate=MOV_CR2AC1;
		  else if(IRwire[7:0]==IR_MOV_AC2SP)      nextstate=MOV_AC2SP1;
		  else if(IRwire[7:0]==IR_MOV_AC2BR)      nextstate=MOV_AC2BR1;
		  else if(IRwire[7:0]==IR_MOV_AC2CR)      nextstate=MOV_AC2CR1;
		  else if(IRwire[7:0]==IR_LDI)      nextstate=LDI1;
		  else if(IRwire[7:0]==IR_STI)      nextstate=STI1;
		  else if(IRwire[7:0]==IR_INC_AC)      nextstate=INC_AC1;
		  else if(IRwire[7:0]==IR_DEC_AC)      nextstate=DEC_AC1;
		  else nextstate=Reset;
		end

		  LD5, 
		  ST5,
		  PUSH_AC3,
		  RET3, 
	   	ADD_AC4, 
		  SUB_AC4, 
	  	AND_AC4,
		  OR_AC4, 
		  XOR_AC4,
		  NOT_AC1,	
		JC2,
		JC3,
		JZ2,	
    JZ3,	
		JO2,
		JO3,
		JS2,
		JS3,
		JMP2,
		IN4, 
		OUT4,
		MOV_AC3, 
		MOV_SP3,
		MOV_BR3, 
		MOV_CR3, 
		MOV_SP2AC1,
		MOV_BR2AC1, 
		MOV_CR2AC1, 
		MOV_AC2SP1, 
		MOV_AC2BR1, 
		MOV_AC2CR1, 
		LDI2,
		STI2, 
		INC_AC1,
		DEC_AC1:       
      nextstate=Fetch1; 
		default: nextstate=Reset;
	endcase  
	 
	//以下是输出控制信号的组合逻辑电路 
	always @(*) 
	  case (state)
	  	LD1, 
		  LD3,
		  ST1,
		  ST3,
		  PUSH_AC2,
		  POP_AC1,
		  CALL1,
		  CALL4,
		  RET1,
		  AND_AC1,
		  AND_AC3,
		  OR_AC1,
		  OR_AC3,
		  XOR_AC1,
		  XOR_AC3,
		  JC1,
		  JZ1,
		  JO1,
		  JS1,
		  LDI1,
		  STI1,
	  	SUB_AC3,
	  	SUB_AC1,
	  	ADD_AC3,
	  	ADD_AC1,
	  	MOV_CR1,
	  	MOV_BR1,
	  	MOV_SP1,
	    Fetch1,
		MOV_AC1,
		JMP1,
		IN1,
	    IN3,
		OUT1,
		OUT3:		MARLD = 1;
		default: 	MARLD = 0;
	  endcase


	always @(*) 
	  case (state)
		  LD2, 
		  LD4,
		  ST2,
		  ST4,
		  CALL2,
		  AND_AC2,
		  OR_AC2,
		  XOR_AC2,
	  	SUB_AC2,
	  	ADD_AC2,
	    Fetch2,
	    IN2,
		OUT2:		MDRLD = 1;
		default: 	MDRLD = 0;
	  endcase


	always @(*) 
	  case (state)
	    LD3,
	    LD5,
	    ST3,
	    ST5,
	    CALL6,
	    AND_AC3,
	    OR_AC3,
	    XOR_AC3,
	  	SUB_AC3,
	  	ADD_AC3,
	    IN3,
		OUT3:		MDRbus = 1;
		default: 	MDRbus = 0;
	  endcase


	always @(*) 
	  case (state)
	    LD5,
	    POP_AC2,
	    AND_AC4,
	    OR_AC4,
	    XOR_AC4,
	    LDI2,
      SUB_AC4,
	  	ADD_AC4,
	  	MOV_CR2AC1,
	  	MOV_BR2AC1,
	  	MOV_SP2AC1,
	  	NOT_AC1,
	  	INC_AC1,
		DEC_AC1,
	    MOV_AC3,
		IN4:		ACLD = 1;
		default: 	ACLD = 0;
	  endcase


	always @(*) 
	  case (state)
	    AND_AC4,
	    OR_AC4,
	    XOR_AC4,
	  	MOV_AC3,
		  IN4,
	  	SUB_AC4,
	  	ADD_AC4,
	  	MOV_CR2AC1,
	  	MOV_BR2AC1,
	  	MOV_SP2AC1,
	  	NOT_AC1,
	    DEC_AC1,
	  	INC_AC1:	PSWLD = 1;
		default: 	PSWLD = 0;
	  endcase


	always @(*) 
	  case (state)
	    ST4,
	    PUSH_AC3,
	    MOV_AC2SP1,
	    MOV_AC2BR1,
	    MOV_AC2CR1,
	    STI2,
	  	OUT4:		ACbus = 1;
		default: 	ACbus = 0;
	  endcase


	always @(*) 
	  case (state)
	    AND_AC4:  ALUsel = 4'b0100;
	    OR_AC4:  ALUsel = 4'b0101;
	    XOR_AC4:  ALUsel = 4'b0110;
	  	SUB_AC4:	ALUsel = 4'b0001;
	  	LD5,
	  	POP_AC2,
      LDI2,
	  	MOV_CR2AC1,
	  	MOV_BR2AC1,
	  	MOV_SP2AC1: ALUsel = 4'b1000;
	  	NOT_AC1:	ALUsel = 4'b0111;
	    DEC_AC1:	ALUsel = 4'b0011;
	  	INC_AC1:	ALUsel = 4'b0010;
	    MOV_AC3,
		IN4:		ALUsel = 4'b1000;
		default: 	ALUsel = 4'b0000;
	  endcase


	always @(*) 
	  case (state)
	    MOV_AC2BR1,
	  	MOV_BR3:	BRLD = 1;
		default: 	BRLD = 0;
	  endcase

	always @(*) 
	  case (state)
	    LDI1,
	    STI1,
	  	MOV_BR2AC1: BRbus = 1;
		default: 	BRbus = 0;
	  endcase

	always @(*) 
	  case (state)
	    MOV_AC2CR1,
	  	MOV_CR3:	CRLD = 1;
		default: 	CRLD= 0;
	  endcase

	always @(*) 
	  case (state)
	  	MOV_CR2AC1: CRbus = 1;
		default: 	CRbus = 0;
	  endcase

	always @(*) 
	  case (state)
	    MOV_AC2SP1,
	  	MOV_SP3:	SPLD = 1;
		default: 	SPLD = 0;
	  endcase


	always @(*) 
	  case (state)
	    RET3,
	    POP_AC3: SPINC=1;
		default: 	SPINC = 0;
	  endcase


	always @(*) 
	  case (state)
	    CALL3,
	    PUSH_AC1:SPDEC=1;
		default: 	SPDEC = 0;
	  endcase


	always @(*) 
	  case (state)
	    PUSH_AC2,
	    POP_AC1,
	    CALL4,
	    RET1,
	  	MOV_SP2AC1: SPbus = 1;
		default: 	SPbus = 0;
	  endcase


	always @(*) 
	  case (state)
	  CALL6,
	  RET2,
	  JC2,
	  JZ2,
	  JO2,
	  JS2,
		JMP2:       PCLD = 1;
		default: 	PCLD = 0;
	  endcase


	always @(*) 
	  case (state)
	    LD1,
	    ST1,
	    CALL1,
	    CALL5,
	    AND_AC1,
	    OR_AC1,
	    XOR_AC1,
	    JC1,
	    JZ1,
	    JO1,
	    JS1,
	  	SUB_AC1,
	  	ADD_AC1,
	    MOV_CR1,
	  	MOV_BR1,
	  	MOV_SP1,
	    Fetch1,
		MOV_AC1,
		JMP1, 	
		IN1,
		OUT1:       PCbus = 1;
		default: 	PCbus = 0;
	  endcase


	always @(*) 
	  case (state)
	    LD3,
	    ST3,
	    CALL2,
	    AND_AC2,
	    OR_AC2,
	    XOR_AC2,
	    JC3,
	    JZ3,
	    JO3,
	    JS3,
	  	SUB_AC2,
	  	ADD_AC2,
	  	MOV_CR2,
	  	MOV_BR2,
	  	MOV_SP2,
	    Fetch3,
		MOV_AC2,
		IN2, 	
		OUT2:       PCinc = 1;
		default: 	PCinc = 0;
	  endcase

	always @(*) 
	  case (state)
		Fetch2: 	IRLD = 1;
		default: 	IRLD = 0;
	  endcase


	always @(*) 
	  case (state)
	  ST5,
	  PUSH_AC3,
	  CALL5,
	  STI2,
		OUT4: 		write = 1;
		default: 	write = 0;
	  endcase


	always @(*) 
	  case (state)
	    LD2,
	    LD4,
	    ST2,
	    POP_AC2,
	    CALL2,
	    RET2,
	    AND_AC2,
	    AND_AC4,
	    OR_AC2,
	    OR_AC4,
	    XOR_AC2,
	    XOR_AC4,
	    JC2,
	    JZ2,
	    JO2,
	    JS2,
	    LDI2,
	  	SUB_AC4,
	  	SUB_AC2,
	  	ADD_AC4,
	  	ADD_AC2,
	  	MOV_CR3,
	  	MOV_BR3,
	  	MOV_SP3,
	  	Fetch2,
		MOV_AC3,
		JMP2,
		IN2,
		IN4,
		OUT2: 		read = 1;
		default: 	read = 0;
	  endcase


	always @(*) 
	  case (state)
		IN4,
		OUT4: 		IO_M = 1;
		default: 	IO_M = 0;
	  endcase

 endmodule