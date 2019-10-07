#ifndef __BLE_UART_TYPES_H
#define __BLE_UART_TYPES_H

#include "stdint.h"

typedef struct conn_param_info_s {
    uint8_t  requested_mode;
    uint8_t  current_mode;
    uint16_t interval;
    uint16_t supervision;
    uint8_t  latency;
} __attribute__((packed)) conn_param_info_t;

typedef struct request_missing_pkg_s {

} __attribute__((packed)) request_missing_pkg_t;
#endif

