module  computer(
		input 				clock,
		input 				reset_n,
		input		[7:0]	 	switches,
		output 	[7:0]  	leds,
		output   [7:0]		irReg,
		output   [3:0]		pswReg,
		output   [7:0]    crReg,
		output   [7:0]    brReg,
		output   [7:0]    acReg,
		output   [7:0]    spReg,
		output   [7:0]    pcReg,
		output   [7:0]    mdrReg,
		output   [7:0]    marReg,
		output   [28:0]   controlSignals		
);
	wire marld, mdrld, mdrbus, acld, acbus, brld,brbus,crld,crbus,spld,spinc,spdec,spbus,pcld,pcbus,pcinc,irld,pswld,wr,rd,io_m;
	wire [3:0] alusel;
	wire [3:0] pswwire;
	wire [7:0] irwire;
	
	wire cs0,cs1;
	
	wire [7:0] key2cpu,mem2cpu,dbout,addr;
	
	
	assign irReg = irwire;
	assign pswReg = pswwire;
	assign marReg = addr;

	assign controlSignals = {
							rd,
							wr,
							rd,
							io_m,
							wr,
							rd,
							io_m,
							alusel,
							crbus,
							brbus,
							acbus,
							spbus,
							pcbus,
							mdrbus,
							marld, 
							mdrld, 
	            pcinc,
							pcld,
							spdec,
							spinc,
							spld,
							acld,
							brld,
							crld,
							pswld,
							irld
							};
	
	controller 		controller1(
			.clk(clock),
			.reset_n(reset_n),
			.PSWwire(pswwire),
			.IRwire(irwire),
			.MARLD(marld),
			.MDRLD(mdrld),
			.MDRbus(mdrbus),
			.ACLD(acld),
			.ACbus(acbus),
			.ALUsel(alusel),
			.BRLD(brld),
			.BRbus(brbus),
			.CRLD(crld),
			.CRbus(crbus),
			.SPLD(spld),
			.SPINC(spinc),
			.SPDEC(spdec),
			.SPbus(spbus),
			.PCLD(pcld),
			.PCbus(pcbus),
			.PCinc(pcinc),
			.IRLD(irld),
			.PSWLD(pswld),
			.write(wr),
			.read(rd),
			.IO_M(io_m)			
	);
	
	datapath 		datapath1(
			.clk(clock),
			.Reset_n(reset_n),
			.MARLD(marld),
			.MDRLD(mdrld),
			.MDRbus(mdrbus),
			.PCLD(pcld),
			.PCINC(pcinc),
			.PCbus(pcbus),
			.SPLD(spld),
			.SPINC(spinc),
			.SPDEC(spdec),
			.SPbus(spbus),
			.ACLD(acld),
			.ACbus(acbus),
			.BRLD(brld),
			.BRbus(brbus),
			.CRLD(crld),
			.CRbus(crbus),
			.PSWLD(pswld),
			.IRLD(irld),
			.ALUsel(alusel),
			.IO_M(io_m),
			.Read(rd),
			.Write(wr),
			.PSWwire(pswwire),
			.IRwire(irwire),
			.Addr(addr),
			.MEM2CPU(mem2cpu),
			.DBout(dbout),
			.Key2CPU(key2cpu),
			.crValue(crReg),
			.brValue(brReg),
			.acValue(acReg),
			.spValue(spReg),
			.pcValue(pcReg),
			.mdrValue(mdrReg)
	);
	
	memory			Memory1(
			.clk(clock),
			.reset_n(reset_n),
			.cs_n(io_m),
			.read(rd),
			.write(wr),
			.Addr(addr),
			.DBin(dbout),
			.DBout(mem2cpu)
	);
	
	keyinput 		KEYinput1(
			.clk(clock),
			.reset_n(reset_n),
			.cs(cs1),
			.read(rd),
			.KEY(switches),
			.KEY2CPU(key2cpu)	
	);
	
	ledoutput		Ledoutput1(
			.clk(clock),
			.reset_n(reset_n),
			.cs(cs0),
			.write(wr),
			.DB(dbout),
			.LED(leds)	
	);
	
	decoder			Decoder1(
		   .IO_M(io_m),
		   .Addr(addr),
		   .cs0(cs0),
			.cs1(cs1)
	);

endmodule
