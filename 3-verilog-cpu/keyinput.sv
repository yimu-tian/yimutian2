//原始的设计存在的问题：输入接口先缓存在value中，下一次读取才能读到，因此暂时修改为下面这样的代码，后续思考怎么改进
module keyinput(input clk,reset_n,
		input cs,
		input read,
		input [7:0] KEY,
		output [7:0] KEY2CPU);
		
	reg [7:0] value;

//1.有问题的描述，寄存器第一缓存了开关输入，第一次读到的是上次缓存的开关输入，而不是当前开关状态，第二次执行IN指令才能读到开关输入	
// 	always@(posedge clk,negedge reset_n)
// 		if(~reset_n) value <=0;
// 		else if(cs&read) value <= KEY;
// 	assign KEY2CPU = value;//(cs&read) ? KEY : 8'b00000000;

//2.简单的修正能够解决上面的问题，但似乎没有必要写这个模块了，直接将开关连接到KEY2CPU不是更简单	
// 	assign KEY2CPU = KEY;

//3. 还是使用寄存的描述也能解决上面的问题，就是当开关状态变化时，读到接口内部，每个时钟上升沿都会判断
// 	always@(posedge clk,negedge reset_n)
// 		if(~reset_n) value <=0;
// 		else if(value != KEY) value <= KEY;
// 	assign KEY2CPU = (cs&read) ? value : 8'b00000000;

//4. 下面也是一个简单的解决办法，仅仅是组合逻辑，接口不做缓冲
	assign KEY2CPU = (cs&read) ? KEY : 8'b00000000;
endmodule