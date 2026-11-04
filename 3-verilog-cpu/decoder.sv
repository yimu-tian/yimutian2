module decoder( input IO_M,
              input [7:0] Addr,
              output cs0,cs1
);
 assign cs0=IO_M & (Addr == 8'b0000_0000);
 assign cs1=IO_M & (Addr == 8'b0000_0001);
endmodule