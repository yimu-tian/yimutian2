/**
 * @file    main.c
 * @brief   ZigBee 传感网终端节点（Router）主程序
 * @author  Luan Tian
 * @date    2024
 *
 * 终端节点职责：
 *  1. 通过 ADC 采集空气质量（MQ-135）与一氧化碳（MQ-7）传感器
 *  2. 周期采集并通过串口上报数据
 *
 * 引脚分配：
 *  - MQ-135：P0.5 (AO 模拟输出，ADC 通道 5)
 *  - MQ-7：P0.4 (AO 模拟输出，ADC 通道 4)
 */

#include "ioCC2530.h"
#include <string.h>

/* ==================== 类型定义 ==================== */
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef signed   char  int8;
typedef signed   short int16;
typedef signed   long  int32;

/* ==================== 传感器引脚定义 ==================== */
#define MQ135_ADC_CHANNEL   5    /**< MQ-135 ADC 通道（P0.5） */
#define MQ7_ADC_CHANNEL     4    /**< MQ-7 ADC 通道（P0.4） */

/* ==================== 全局变量 ==================== */
static uint32 system_timer = 0;    /**< 系统时间（毫秒） */
static uint8  send_interval = 5000; /**< 上报间隔（毫秒） */
static uint8  auto_send = 1;       /**< 自动上报使能 */

/* ==================== 内部函数声明 ==================== */
static void board_init(void);
static void delay_ms(uint16 ms);
static void uart_init(void);
static void uart_send_char(char c);
static void uart_send_string(char *str);
static void send_number(uint16 num);
static uint16 adc_read(uint8 channel);
static void mq135_init(void);
static void mq7_init(void);
static uint16 read_mq135(void);
static uint16 read_mq7(void);
static void read_and_send_sensor_data(void);

/* ==================== 系统初始化 ==================== */

/**
 * @brief   系统时钟初始化（32MHz）
 */
static void board_init(void)
{
    CLKCONCMD &= ~0x40;              /* 系统时钟设为 32MHz */
    while (CLKCONSTA & 0x40);        /* 等待时钟稳定 */
    CLKCONCMD &= ~0x47;              /* 选择系统时钟源 */
}

/**
 * @brief   毫秒延时（软件延时，仅用于非精确场景）
 * @param   ms  延时毫秒数
 */
static void delay_ms(uint16 ms)
{
    uint16 i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 535; j++);
}

/* ==================== 串口通信 ==================== */

/**
 * @brief   UART0 初始化（115200 波特率）
 */
static void uart_init(void)
{
    PERCFG = 0x00;                   /* UART0 映射到备用位置1 */
    P0SEL  |= 0x0C;                  /* P0.2/P0.3 设为外设功能（UART） */

    U0CSR |= 0x80;                   /* UART 模式 */
    U0GCR |= 11;                     /* 波特率指数 */
    U0BAUD = 216;                    /* 波特率尾数 → 115200 */

    U0CSR |= 0x40;                   /* 使能接收 */
}

/**
 * @brief   发送单个字符
 * @param   c  字符
 */
static void uart_send_char(char c)
{
    U0DBUF = c;
    while (!(U0CSR & 0x02));         /* 等待发送完成 */
    U0CSR &= ~0x02;                  /* 清除发送完成标志 */
}

/**
 * @brief   发送字符串
 * @param   str  字符串指针
 */
static void uart_send_string(char *str)
{
    while (*str) {
        uart_send_char(*str++);
    }
}

/**
 * @brief   发送无符号整数（十进制）
 * @param   num  数值
 */
static void send_number(uint16 num)
{
    char num_str[6];
    uint8 i = 0;
    uint8 digits = 0;
    uint16 temp = num;

    if (num == 0) {
        uart_send_string("0");
        return;
    }

    while (temp > 0) {
        digits++;
        temp /= 10;
    }

    num_str[digits] = '\0';
    for (i = 0; i < digits; i++) {
        num_str[digits - 1 - i] = '0' + (num % 10);
        num /= 10;
    }

    uart_send_string(num_str);
}

/* ==================== ADC 采集 ==================== */

/**
 * @brief   读取指定通道的 ADC 值（12 位）
 * @param   channel  ADC 通道号
 * @return  12 位 ADC 值（0~4095）
 */
static uint16 adc_read(uint8 channel)
{
    uint16 adc_value;

    ADCCON3 = (channel << 4) | 0x0E;  /* 选择通道 + 12 位分辨率 */

    ADCCON1 |= 0x30;                  /* 启动转换模式 */
    ADCCON1 |= 0x40;                  /* 开始转换 */

    while (!(ADCCON1 & 0x80));        /* 等待转换完成 */

    adc_value = ADCL;                 /* 读低字节 */
    adc_value |= (ADCH << 8);         /* 读高字节 */

    return adc_value >> 2;            /* 右移得到 12 位有效值 */
}

/* ==================== 传感器驱动 ==================== */

/**
 * @brief   MQ-135 空气质量传感器初始化
 *
 * AO 接 P0.5（ADC 通道 5），DO 接 P2.1（数字输出）
 */
static void mq135_init(void)
{
    P0SEL |= 0x20;                   /* P0.5 设为外设功能（ADC） */

    P2SEL &= ~0x02;                  /* P2.1 设为通用 IO */
    P2DIR &= ~0x02;                  /* P2.1 设为输入 */
}

/**
 * @brief   MQ-7 一氧化碳传感器初始化
 *
 * AO 接 P0.4（ADC 通道 4），DO 接 P2.2（数字输出）
 */
static void mq7_init(void)
{
    P0SEL |= 0x10;                   /* P0.4 设为外设功能（ADC） */

    P2SEL &= ~0x04;                  /* P2.2 设为通用 IO */
    P2DIR &= ~0x04;                  /* P2.2 设为输入 */
}

/**
 * @brief   读取 MQ-135 空气质量值
 * @return  ADC 值
 */
static uint16 read_mq135(void)
{
    return adc_read(MQ135_ADC_CHANNEL);
}

/**
 * @brief   读取 MQ-7 一氧化碳值
 * @return  ADC 值
 */
static uint16 read_mq7(void)
{
    return adc_read(MQ7_ADC_CHANNEL);
}

/* ==================== 数据上报 ==================== */

/**
 * @brief   采集并上报传感器数据
 */
static void read_and_send_sensor_data(void)
{
    uint16 mq135_value = read_mq135();
    uint16 mq7_value   = read_mq7();

    /* 上报数据：AQ:xxx,CO:xxx */
    uart_send_string("AQ:");
    send_number(mq135_value);
    uart_send_string(",CO:");
    send_number(mq7_value);
    uart_send_string("\r\n");

    /* 调试信息 */
    uart_send_string("Slave1: AQ=");
    send_number(mq135_value);
    uart_send_string(", CO=");
    send_number(mq7_value);
    uart_send_string("\r\n");
}

/* ==================== 主程序 ==================== */

/**
 * @brief   主函数
 */
void main(void)
{
    /* 硬件初始化 */
    board_init();
    uart_init();
    mq135_init();
    mq7_init();

    uart_send_string("=== Slave1 Started ===\r\n");
    uart_send_string("MQ135:P0.5(AO),P2.1(DO); MQ7:P0.4(AO),P2.2(DO)\r\n");
    uart_send_string("Auto send every 5 seconds\r\n");

    /* 初始延时，确保传感器稳定 */
    delay_ms(2000);

    /* 先采集一次 */
    read_and_send_sensor_data();

    while (1) {
        /* 周期上报传感器数据 */
        if (auto_send && system_timer % send_interval == 0) {
            read_and_send_sensor_data();
        }

        delay_ms(100);
        system_timer += 100;
    }
}
