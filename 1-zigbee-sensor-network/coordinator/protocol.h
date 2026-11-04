/**
 * @file    protocol.h
 * @brief   ZigBee 传感网无线通信协议定义
 * @author  Luan Tian
 * @date    2024
 *
 * 定义协调器与终端节点之间的无线通信协议：
 * 数据包格式、消息类型、传感器数据类型、校验规则。
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "hal_types.h"

/* ==================== 设备地址 ==================== */
#define MASTER_ADDR     0x0001   /**< 协调器地址 */
#define SLAVE1_ADDR     0x0002   /**< 终端节点 1 地址 */
#define SLAVE2_ADDR     0x0003   /**< 终端节点 2 地址 */

/* ==================== 消息类型 ==================== */
typedef enum {
    MSG_SENSOR_DATA = 0x01,      /**< 传感器数据上报 */
    MSG_CMD_REQUEST = 0x02,      /**< 命令请求 */
    MSG_ACK         = 0x03       /**< 应答 */
} msg_type_t;

/* ==================== 传感器数据类型 ==================== */
typedef enum {
    DATA_AIR_QUALITY = 0x10,     /**< 空气质量 */
    DATA_CO          = 0x11,     /**< 一氧化碳 */
    DATA_LIGHT       = 0x12,     /**< 光照强度 */
    DATA_TEMP_HUMID  = 0x13      /**< 温湿度 */
} data_type_t;

/* ==================== 无线数据包结构 ==================== */
typedef struct {
    uint16 src_addr;             /**< 源地址 */
    uint16 dest_addr;            /**< 目标地址 */
    uint8  msg_type;             /**< 消息类型，见 msg_type_t */
    uint8  data_type;            /**< 数据类型，见 data_type_t */
    uint8  seq_num;              /**< 序列号，用于防重 */
    uint8  length;               /**< 数据负载长度 */
    uint8  data[10];             /**< 数据负载 */
    uint16 checksum;             /**< 校验和 */
} rf_packet_t;

/* ==================== 传感器数据结构 ==================== */
typedef struct {
    uint16 air_quality;          /**< 空气质量 */
    uint16 co_level;             /**< 一氧化碳浓度 */
    uint16 light_level;          /**< 光照强度 */
    int16  temperature;          /**< 温度（有符号） */
    uint16 humidity;             /**< 湿度 */
} sensor_data_t;

/* ==================== 协议接口 ==================== */
uint16 protocol_checksum(uint8 *data, uint8 len);
uint8  protocol_verify(rf_packet_t *pkt);

#endif /* PROTOCOL_H */
