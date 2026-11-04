/**
 * @file    dht11.h
 * @brief   DHT11 温湿度传感器驱动（单总线）
 * @author  Luan Tian
 * @date    2024
 *
 * 单总线（1-Wire）协议：一根信号线同时传输数据和时钟，
 * 通过精确的微秒级延时区分逻辑 0 和 1。
 */

#ifndef DHT11_H
#define DHT11_H

#include "ioCC2530.h"

/* ==================== 引脚定义 ==================== */
#define DHT11_PIN          P0_7        /**< 数据引脚（P0.7） */
#define DHT11_PIN_INPUT    (P0DIR &= ~0x80)   /**< 设为输入 */
#define DHT11_PIN_OUTPUT   (P0DIR |= 0x80)    /**< 设为输出 */

/* ==================== 数据结构 ==================== */
typedef struct {
    uint8 temperature;   /**< 温度整数部分（℃） */
    uint8 humidity;      /**< 湿度整数部分（%RH） */
} dht11_data_t;

/* ==================== 接口函数 ==================== */

/**
 * @brief   读取 DHT11 温湿度数据
 * @param   data  输出数据指针
 * @return  0=成功，1=失败（无响应或校验错误）
 */
uint8 dht11_read(dht11_data_t *data);

#endif /* DHT11_H */
