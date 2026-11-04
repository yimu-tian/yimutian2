/**
 * @file    protocol.c
 * @brief   协议校验实现
 * @author  Luan Tian
 */

#include "protocol.h"
#include "hal_board.h"
#include <string.h>

/**
 * @brief   计算校验和（累加和，取低 16 位）
 * @param   data  数据指针
 * @param   len   数据长度
 * @return  16 位校验和
 */
uint16 protocol_checksum(uint8 *data, uint8 len)
{
    uint16 sum = 0;
    uint8 i;

    for (i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

/**
 * @brief   校验数据包完整性
 * @param   pkt  数据包指针
 * @return  1=校验通过，0=校验失败
 *
 * 校验方法：将 checksum 字段清零后计算累加和，
 * 与原始 checksum 字段比较。相等则数据完整。
 */
uint8 protocol_verify(rf_packet_t *pkt)
{
    uint16 original_checksum;    /* 保存原始校验和 */
    uint16 calculated_checksum;  /* 重新计算的校验和 */

    original_checksum = pkt->checksum;

    /* 计算校验和时，校验字段本身置零 */
    pkt->checksum = 0;
    calculated_checksum = protocol_checksum((uint8 *)pkt, 8 + pkt->length);

    /* 恢复原始校验和，避免影响后续使用 */
    pkt->checksum = original_checksum;

    return (calculated_checksum == original_checksum);
}
