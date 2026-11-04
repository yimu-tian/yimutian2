# Verilog 8 位 CPU 设计

基于 SystemVerilog 设计的 8 位中央处理器，计算机组成原理课程设计。

## CPU 组成

| 模块 | 文件 | 功能 |
|------|------|------|
| 控制器 | controller.sv | 解析指令，产生控制信号 |
| 数据通路 | datapath.sv | 寄存器间数据流动 |
| 译码器 | decoder.sv | 指令译码 |
| ALU | computer.sv | 算术逻辑运算 |
| 存储器 | memory.sv | 指令与数据存储 |
| 寄存器组 | top.sv | IR/PC/AC/SP/MAR/MDR 等寄存器 |

## 寄存器组

- IR（指令寄存器）、PSW（程序状态字）
- AC（累加器）、PC（程序计数器）、SP（栈指针）
- MAR（存储器地址寄存器）、MDR（存储器数据寄存器）

## 功能

- 取指-译码-执行三段流水
- 支持指令执行与 UART 通信
- 七段数码管 / LED 显示寄存器状态
