/**
 * @file    dht11.c
 * @brief   DHT11 温湿度传感器驱动实现
 * @author  Luan Tian
 * @date    2024
 *
 * 单总线时序（关键）：
 *  1. 主机拉低总线 >18ms（开始信号）
 *  2. 主机释放总线，DHT11 拉低 80us + 拉高 80us（响应）
 *  3. 40 位数据：湿度整数 + 湿度小数 + 温度整数 + 温度小数 + 校验和
 *  4. 每 bit：50us 低电平开始，高电平 26~28us=0，70us=1
 *
 * 注意：读取过程中需关闭全局中断，防止协议栈中断
 * 打断微秒级时序导致数据错误。
 */

#include "dht11.h"
#include "OnBoard.h"

/* ==================== 内部函数声明 ==================== */
static void dht11_delay_us(uint8 us);
static void dht11_delay_ms(uint16 ms);
static uint8 dht11_read_byte(void);

/* ==================== 延时函数 ==================== */

/**
 * @brief   微秒延时
 * @param   us  微秒数
 */
static void dht11_delay_us(uint8 us)
{
    while (us--) {
        MicroWait(1);
    }
}

/**
 * @brief   毫秒延时
 * @param   ms  毫秒数
 */
static void dht11_delay_ms(uint16 ms)
{
    uint8 i;
    while (ms--) {
        for (i = 0; i < 100; i++) {
            dht11_delay_us(10);
        }
    }
}

/* ==================== 单总线通信 ==================== */

/**
 * @brief   读取一个字节（8 位）
 * @return  读取到的字节
 *
 * 每 bit：50us 低电平开始，高电平 26~28us=0，70us=1
 */
static uint8 dht11_read_byte(void)
{
    uint8 i;
    uint8 byte = 0;
    uint8 flag;

    for (i = 0; i < 8; i++) {
        /* 等待低电平结束（每 bit 以 50us 低电平开始） */
        flag = 0;
        while (!DHT11_PIN && flag++ < 100);

        /* 延时 30us 后采样，此时高电平已稳定 */
        dht11_delay_us(30);

        /* 采样：高电平 >26us 说明是 1，否则是 0 */
        if (DHT11_PIN) {
            byte |= (1 << (7 - i));
        }

        /* 等待高电平结束 */
        flag = 0;
        while (DHT11_PIN && flag++ < 100);
    }

    return byte;
}

/* ==================== 对外接口 ==================== */

/**
 * @brief   读取 DHT11 温湿度数据
 * @param   data  输出数据指针
 * @return  0=成功，1=失败
 */
uint8 dht11_read(dht11_data_t *data)
{
    uint8 rh_h, rh_l, temp_h, temp_l, checksum;
    uint8 sum;

    /* ===== 1. 发送开始信号：拉低 >18ms ===== */
    DHT11_PIN_OUTPUT;
    DHT11_PIN = 0;
    dht11_delay_ms(19);

    /* ===== 2. 释放总线，切换为输入 ===== */
    DHT11_PIN = 1;
    DHT11_PIN_INPUT;
    dht11_delay_us(40);

    /* ===== 3. 等待 DHT11 响应（拉低 80us + 拉高 80us） ===== */
    if (DHT11_PIN) {
        /* 总线仍为高，说明 DHT11 无响应 */
        DHT11_PIN_OUTPUT;
        DHT11_PIN = 1;
        return 1;
    }

    /* 等待响应低电平结束 */
    while (!DHT11_PIN);
    /* 等待响应高电平结束 */
    while (DHT11_PIN);

    /* ===== 4. 读取 40 位数据（5 字节） =====
     * 关键：读取期间关闭全局中断，防止时序被中断打断 */
    EA = 0;  /* 关全局中断（临界区） */

    rh_h     = dht11_read_byte();  /* 湿度整数 */
    rh_l     = dht11_read_byte();  /* 湿度小数 */
    temp_h   = dht11_read_byte();  /* 温度整数 */
    temp_l   = dht11_read_byte();  /* 温度小数 */
    checksum = dht11_read_byte();  /* 校验和 */

    EA = 1;  /* 恢复全局中断 */

    /* ===== 5. 校验数据 ===== */
    sum = rh_h + rh_l + temp_h + temp_l;
    if (sum != checksum) {
        /* 校验失败 */
        DHT11_PIN_OUTPUT;
        DHT11_PIN = 1;
        return 1;
    }

    /* ===== 6. 保存结果 ===== */
    data->temperature = temp_h;
    data->humidity    = rh_h;

    DHT11_PIN_OUTPUT;
    DHT11_PIN = 1;
    return 0;
}
