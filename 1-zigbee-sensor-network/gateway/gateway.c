/**
 * @file    gateway.c
 * @brief   ZigBee 传感网网关节点
 * @author  Luan Tian
 * @date    2024
 *
 * 网关职责：
 *  1. 通过 RF 接收各终端节点的传感器数据包
 *  2. 解析并转发到 PC（通过串口）
 *
 * RF 配置：
 *  - 信道：11（2405 MHz）
 *  - PAN ID：0x1234
 *  - 调制：O-QPSK，250 kbps
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

/* ==================== RF 参数 ==================== */
#define PAN_ID      0x1234   /**< 网络 PAN ID */
#define CHANNEL     11       /**< 工作信道（2405MHz） */
#define TX_POWER    0xF5     /**< 发射功率 */

/* ==================== 网络数据包结构 ==================== */
typedef struct {
    uint16 node_id;          /**< 节点编号 */
    uint16 pan_id;           /**< PAN ID */
    uint16 air_quality;      /**< 空气质量 */
    uint16 co_level;         /**< 一氧化碳浓度 */
    uint16 light_level;      /**< 光照强度 */
    int16  temperature;      /**< 温度（有符号） */
    uint16 humidity;         /**< 湿度 */
    uint32 timestamp;        /**< 时间戳 */
} network_packet_t;

/* ==================== 全局变量 ==================== */
static uint8 rf_rx_buffer[64];          /**< RF 接收缓冲区 */
static volatile uint8 packet_received;   /**< 数据包接收标志 */
static network_packet_t current_packet;  /**< 当前数据包 */

/* ==================== 内部函数声明 ==================== */
static void board_init(void);
static void delay_ms(uint16 ms);
static void uart_init(void);
static void uart_send_char(char c);
static void uart_send_string(char *str);
static void int_to_str(uint16 num, char *str);
static void rf_init(void);
static void gateway_init(void);
static void process_received_packet(network_packet_t *packet);
static void forward_data_to_pc(network_packet_t *packet);

/* ==================== 系统初始化 ==================== */

/**
 * @brief   系统时钟初始化（32MHz）
 */
static void board_init(void)
{
    CLKCONCMD &= ~0x40;
    while (CLKCONSTA & 0x40);
    CLKCONCMD &= ~0x47;
}

/**
 * @brief   毫秒延时
 */
static void delay_ms(uint16 ms)
{
    uint16 i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 535; j++);
}

/* ==================== 串口通信 ==================== */

/**
 * @brief   UART0 初始化（115200）
 */
static void uart_init(void)
{
    PERCFG = 0x00;
    P0SEL  |= 0x0C;

    U0CSR |= 0x80;
    U0GCR |= 11;
    U0BAUD = 216;

    U0CSR |= 0x40;
}

/**
 * @brief   发送单字符
 */
static void uart_send_char(char c)
{
    U0DBUF = c;
    while (!(U0CSR & 0x02));
    U0CSR &= ~0x02;
}

/**
 * @brief   发送字符串
 */
static void uart_send_string(char *str)
{
    while (*str) {
        uart_send_char(*str++);
    }
}

/**
 * @brief   整数转字符串
 * @param   num  数值
 * @param   str  输出缓冲区
 */
static void int_to_str(uint16 num, char *str)
{
    uint8 i = 0;
    uint8 digits = 0;
    uint16 temp = num;

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    while (temp > 0) {
        digits++;
        temp /= 10;
    }

    str[digits] = '\0';
    for (i = 0; i < digits; i++) {
        str[digits - 1 - i] = '0' + (num % 10);
        num /= 10;
    }
}

/* ==================== RF 无线通信 ==================== */

/**
 * @brief   RF 初始化
 *
 * 配置自动 CRC、发射功率、信道频率、调制方式、PAN ID。
 */
static void rf_init(void)
{
    FRMCTRL0 = 0x08;                 /* 自动 CRC、自动 FCS */

    TXFILTCFG = TX_POWER;            /* 发射功率 */

    /* 信道频率：2405 + 5*(channel-11) MHz */
    FREQCTRL = 0x0B + (CHANNEL - 11);

    /* 调制方式：O-QPSK，250 kbps */
    MDMCTRL0 = 0x85;
    MDMCTRL1 = 0x14;

    /* 设置 PAN ID */
    PANID0 = (uint8)(PAN_ID & 0xFF);
    PANID1 = (uint8)((PAN_ID >> 8) & 0xFF);

    /* 本机短地址为 0（网关不参与寻址，仅接收） */
    SHORTADDR0 = 0x00;
    SHORTADDR1 = 0x00;

    /* 使用默认同步字 */
    SYNCWORD = 0x0F;

    /* 使能 RF 接收中断 */
    RFIRQM0 |= 0x02;
    IEN0 |= 0x80;
}

/**
 * @brief   网关初始化
 */
static void gateway_init(void)
{
    rf_init();

    /* 进入接收状态 */
    RFST = RFST_SRX;

    uart_send_string("=== 网关节点启动 ===\r\n");
    uart_send_string("PAN ID: 0x1234, 信道: 11\r\n");
    uart_send_string("等待接收节点数据...\r\n");
}

/* ==================== 数据包处理 ==================== */

/**
 * @brief   解析并打印接收到的数据包
 */
static void process_received_packet(network_packet_t *packet)
{
    char buffer[64];

    uart_send_string("收到节点数据 - 节点");

    int_to_str(packet->node_id, buffer);
    uart_send_string(buffer);

    uart_send_string(": AQ=");
    int_to_str(packet->air_quality, buffer);
    uart_send_string(buffer);

    uart_send_string(" CO=");
    int_to_str(packet->co_level, buffer);
    uart_send_string(buffer);

    uart_send_string(" Light=");
    int_to_str(packet->light_level, buffer);
    uart_send_string(buffer);

    uart_send_string(" Temp=");
    int_to_str(packet->temperature, buffer);
    uart_send_string(buffer);

    uart_send_string(" Humid=");
    int_to_str(packet->humidity, buffer);
    uart_send_string(buffer);

    uart_send_string("\r\n");
}

/**
 * @brief   转发数据到 PC（C# 上位机可识别的格式）
 */
static void forward_data_to_pc(network_packet_t *packet)
{
    char buffer[10];

    uart_send_string("NODE:");
    int_to_str(packet->node_id, buffer);
    uart_send_string(buffer);

    uart_send_string(",AQ:");
    int_to_str(packet->air_quality, buffer);
    uart_send_string(buffer);

    uart_send_string(",CO:");
    int_to_str(packet->co_level, buffer);
    uart_send_string(buffer);

    uart_send_string(",LIGHT:");
    int_to_str(packet->light_level, buffer);
    uart_send_string(buffer);

    uart_send_string(",TEMP:");
    int_to_str(packet->temperature, buffer);
    uart_send_string(buffer);

    uart_send_string(",HUMID:");
    int_to_str(packet->humidity, buffer);
    uart_send_string(buffer);

    uart_send_string(",TIME:");
    int_to_str(packet->timestamp, buffer);
    uart_send_string(buffer);

    uart_send_string("\r\n");
}

/* ==================== RF 中断服务 ==================== */

/**
 * @brief   RF 接收中断服务函数
 */
#pragma vector = RF_VECTOR
__interrupt void RF_ISR(void)
{
    if (RFIRQF0 & 0x02) {                    /* RX 完成中断 */
        uint8 len = RFD;                     /* 读取数据长度 */

        if (len == sizeof(network_packet_t)) {
            uint8 i;
            uint8 *packet_ptr = (uint8 *)&current_packet;

            /* 逐字节读取数据包 */
            for (i = 0; i < len; i++) {
                packet_ptr[i] = RFD;
            }

            packet_received = 1;             /* 置接收标志 */
        } else {
            uint8 i;
            for (i = 0; i < len; i++) {
                (void)RFD;                   /* 数据长度不匹配，丢弃 */
            }
            uart_send_string("数据包长度错误\r\n");
        }

        RFIRQF0 &= ~0x02;                    /* 清中断标志 */
        RFST = RFST_SRX;                     /* 重新进入接收状态 */
    }
}

/* ==================== 主程序 ==================== */

/**
 * @brief   主函数
 */
void main(void)
{
    board_init();
    uart_init();
    gateway_init();

    uart_send_string("网关已就绪，等待数据...\r\n");

    while (1) {
        if (packet_received) {
            packet_received = 0;

            process_received_packet(&current_packet);  /* 打印数据 */
            forward_data_to_pc(&current_packet);       /* 转发到 PC */
        }

        delay_ms(100);
    }
}
