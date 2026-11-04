/**
 * @file    main.c
 * @brief   ZigBee 传感网协调器（Master）主程序
 * @author  Luan Tian
 * @date    2024
 *
 * 协调器职责：
 *  1. 周期请求终端节点上报传感器数据（OSAL 定时器驱动）
 *  2. 汇聚多路传感器数据，OLED 显示
 *  3. 温湿度超标蜂鸣器报警
 *  4. 通过自定义串口协议将数据上传至上位机
 */

#include "protocol.h"
#include "hal_uart.h"
#include "hal_oled.h"
#include "hal_buzzer.h"

/* ==================== 全局变量 ==================== */
static sensor_data_t sensor_data;   /**< 汇聚的传感器数据 */
static uint8 uart_buffer[64];       /**< 串口接收缓冲区 */

/* ==================== 内部函数声明 ==================== */
static void oled_init(void);
static void buzzer_control(uint8 state);
static void uart_init(void);
static void uart_rx_callback(uint8 port, uint8 event);
static void process_uart_command(uint8 *data, uint8 len);
static void send_sensor_data_to_pc(void);
static void rf_rx_callback(uint16 src_addr, uint8 *data, uint8 len);
static void process_sensor_data(rf_packet_t *pkt);
static void update_oled_display(void);
static void send_ack(uint16 dest_addr);
static void request_sensor_data(uint16 slave_addr);

/* ==================== 硬件初始化 ==================== */

/**
 * @brief   OLED 显示初始化
 */
static void oled_init(void)
{
    OLED_Init_GPIO();
    OLED_Init_Device();
    OLED_Clear();
    OLED_ShowString(0, 0, "Master Ready", 12);
}

/**
 * @brief   蜂鸣器控制
 * @param   state  1=响，0=停
 */
static void buzzer_control(uint8 state)
{
    if (state) {
        HAL_BUZZER_ON();    /* P0.7 高电平，蜂鸣器响 */
    } else {
        HAL_BUZZER_OFF();   /* P0.7 低电平，蜂鸣器停 */
    }
}

/* ==================== 串口通信 ==================== */

/**
 * @brief   串口初始化（115200 波特率）
 */
static void uart_init(void)
{
    halUARTCfg_t uartConfig;

    uartConfig.baudRate             = HAL_UART_BR_115200;
    uartConfig.flowControl          = FALSE;
    uartConfig.flowControlThreshold = 0;
    uartConfig.idleTimeout          = 0;
    uartConfig.intEnable            = TRUE;
    uartConfig.callBackFunc         = uart_rx_callback;

    HalUARTOpen(HAL_UART_PORT_0, &uartConfig);
}

/**
 * @brief   串口接收回调（中断中调用）
 * @param   port   串口端口号
 * @param   event  事件类型
 */
static void uart_rx_callback(uint8 port, uint8 event)
{
    uint8 len = Hal_UART_RxBufLen(HAL_UART_PORT_0);

    if (len > 0 && len < sizeof(uart_buffer)) {
        HalUARTRead(HAL_UART_PORT_0, uart_buffer, len);
        process_uart_command(uart_buffer, len);
    }
}

/**
 * @brief   解析上位机命令
 * @param   data  命令数据
 * @param   len   命令长度
 *
 * 支持命令：
 *   - "GET"：请求上传传感器数据
 *   - "BUZ"：测试蜂鸣器
 */
static void process_uart_command(uint8 *data, uint8 len)
{
    if (len < 3) {
        return;                     /* 命令过短，忽略 */
    }

    if (data[0] == 'G' && data[1] == 'E' && data[2] == 'T') {
        send_sensor_data_to_pc();   /* 上传数据 */
    } else if (data[0] == 'B' && data[1] == 'U' && data[2] == 'Z') {
        buzzer_control(1);          /* 蜂鸣器测试 */
        HalMcuWaitMs(500);
        buzzer_control(0);
    }
}

/**
 * @brief   将传感器数据通过串口发送给 PC
 *
 * 数据格式：AQ:xxx,CO:xxx,LIGHT:xxx,TEMP:xxx,HUM:xxx
 */
static void send_sensor_data_to_pc(void)
{
    uint8 buffer[32];
    uint8 len = 0;

    len += sprintf((char *)buffer + len, "AQ:%d,",  sensor_data.air_quality);
    len += sprintf((char *)buffer + len, "CO:%d,",  sensor_data.co_level);
    len += sprintf((char *)buffer + len, "LIGHT:%d,", sensor_data.light_level);
    len += sprintf((char *)buffer + len, "TEMP:%d,", sensor_data.temperature);
    len += sprintf((char *)buffer + len, "HUM:%d",  sensor_data.humidity);

    HalUARTWrite(HAL_UART_PORT_0, buffer, len);
    HalUARTWrite(HAL_UART_PORT_0, (uint8 *)"\r\n", 2);
}

/* ==================== RF 无线通信 ==================== */

/**
 * @brief   RF 接收回调
 * @param   srcAddr  源地址
 * @param   data     数据指针
 * @param   len      数据长度
 */
static void rf_rx_callback(uint16 src_addr, uint8 *data, uint8 len)
{
    rf_packet_t *pkt = (rf_packet_t *)data;

    if (pkt->msg_type == MSG_SENSOR_DATA) {
        process_sensor_data(pkt);   /* 解析传感器数据 */
        update_oled_display();      /* 刷新显示 */
        send_ack(src_addr);         /* 回应答 */
    }
}

/**
 * @brief   解析传感器数据包
 * @param   pkt  数据包指针
 */
static void process_sensor_data(rf_packet_t *pkt)
{
    switch (pkt->data_type) {
        case DATA_AIR_QUALITY:
            sensor_data.air_quality = (pkt->data[0] << 8) | pkt->data[1];
            break;

        case DATA_CO:
            sensor_data.co_level = (pkt->data[0] << 8) | pkt->data[1];
            break;

        case DATA_LIGHT:
            sensor_data.light_level = (pkt->data[0] << 8) | pkt->data[1];
            break;

        case DATA_TEMP_HUMID:
            sensor_data.temperature = (pkt->data[0] << 8) | pkt->data[1];
            sensor_data.humidity    = (pkt->data[2] << 8) | pkt->data[3];
            break;

        default:
            break;
    }
}

/**
 * @brief   更新 OLED 显示
 */
static void update_oled_display(void)
{
    char display_buf[16];

    OLED_Clear();
    OLED_ShowString(0, 0, "Sensor Data:", 12);

    sprintf(display_buf, "AQ:%d", sensor_data.air_quality);
    OLED_ShowString(0, 2, display_buf, 12);

    sprintf(display_buf, "CO:%d", sensor_data.co_level);
    OLED_ShowString(0, 3, display_buf, 12);

    sprintf(display_buf, "L:%d T:%d", sensor_data.light_level, sensor_data.temperature);
    OLED_ShowString(0, 4, display_buf, 12);

    sprintf(display_buf, "H:%d", sensor_data.humidity);
    OLED_ShowString(0, 5, display_buf, 12);
}

/**
 * @brief   发送应答包
 * @param   dest_addr  目标地址
 */
static void send_ack(uint16 dest_addr)
{
    rf_packet_t ack_pkt;

    ack_pkt.src_addr  = MASTER_ADDR;
    ack_pkt.dest_addr = dest_addr;
    ack_pkt.msg_type  = MSG_ACK;
    ack_pkt.length    = 0;

    RF_SendData((uint8 *)&ack_pkt, sizeof(ack_pkt));
}

/**
 * @brief   请求终端节点上报数据
 * @param   slave_addr  终端节点地址
 */
static void request_sensor_data(uint16 slave_addr)
{
    rf_packet_t request_pkt;

    request_pkt.src_addr  = MASTER_ADDR;
    request_pkt.dest_addr = slave_addr;
    request_pkt.msg_type  = MSG_CMD_REQUEST;
    request_pkt.length    = 0;

    RF_SendData((uint8 *)&request_pkt, sizeof(request_pkt));
}

/* ==================== 主程序 ==================== */

/**
 * @brief   主函数
 */
void main(void)
{
    /* 硬件初始化 */
    Board_Init();
    RF_Init();
    uart_init();
    oled_init();
    Buzzer_Init();

    /* 注册 RF 接收回调 */
    RF_SetRxCallback(rf_rx_callback);

    OLED_ShowString(0, 0, "Master Started", 14);

    /* 主循环：周期轮询各终端节点，上传数据 */
    while (1) {
        request_sensor_data(SLAVE1_ADDR);
        HalMcuWaitMs(1000);

        request_sensor_data(SLAVE2_ADDR);
        HalMcuWaitMs(1000);

        send_sensor_data_to_pc();
        HalMcuWaitMs(2000);
    }
}
