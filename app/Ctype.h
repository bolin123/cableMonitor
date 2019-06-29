#ifndef CTPYE_H
#define CTPYE_H

#include <stdarg.h>
#include <math.h>
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sockets.h"
#include "hal_uart.h"
#include "hal_eint.h"
#include "hal_gpio.h"
#include "ril.h"
#include "hal_pwm.h"
#include "serial_port.h"
#include "timers.h"
#define socklen_t int
#include "other.h"
#include "spi.h"
#include "opencpu_onenet.h"
#include "opencpu_ct.h"
#include <time.h>

#undef bool
#define bool unsigned char
#undef uint8_t 
#define uint8_t unsigned char
#undef uint16_t 
#define uint16_t unsigned short
#undef uint32_t
#define uint32_t unsigned int
#undef int8_t 
#define int8_t signed char
#undef int16_t 
#define int16_t signed short
#undef int32_t
#define int32_t signed int
#undef uint64_t
#define uint64_t unsigned long long int
#undef int64_t
#define int64_t long long int

#undef true
#define true (1)
#undef false
#define false (0)

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef void(* rtc_sw_timer_callback_t)(void *);
typedef enum {
    PING_PACKET_RESULT,
    PING_TOTAL_RESULT
} ping_result_type_t;
//typedef  unsigned char u8;
//typedef  unsigned int  u32;
typedef void (* ping_request_result_t)(ping_result_type_t type, void* result);
typedef void (* dns_request_result_t)(unsigned char*);

typedef enum {
    HAL_I2C_FREQUENCY_50K  = 0,          /**<  50kbps. */
    HAL_I2C_FREQUENCY_100K = 1,          /**<  100kbps. */
    HAL_I2C_FREQUENCY_200K = 2,          /**<  200kbps. */
    HAL_I2C_FREQUENCY_300K = 3,          /**<  300kbps. */
    HAL_I2C_FREQUENCY_400K = 4,          /**<  400kbps. */
    HAL_I2C_FREQUENCY_1M   = 5,          /**<  1mbps. */
    HAL_I2C_FREQUENCY_MAX                /**<  The total number of supported I2C frequencies (invalid I2C frequency).*/
} hal_i2c_frequency_t;
typedef struct _ping_result
{
    uint32_t min_time;
    uint32_t max_time;
    uint32_t avg_time;
    uint32_t total_num;
    uint32_t lost_num;
    uint32_t recv_num;
    ip4_addr_t ping_target;
} ping_result_t;
typedef struct {
    bool is_timeout;         /*When it is true, other data is invalid.*/
    uint32_t rtt;            /*The unit is ms.*/
    uint32_t ttl;            /*The TTL value in ping response packet.*/
    uint32_t packet_size;    /*The unit is byte.*/
    bool is_ipv4;            /*ipv4 is true, ipv6 is false.*/
    uint16_t ip_address[8];  /*The address has been translated by ping thread.*/
} ping_packet_result_t;

#define CMDMP_MAX_APPKEY	15
#define CMDMP_MAX_PSWD		40
typedef struct {
    uint8_t  addr_mode; 	//!< 服务器信息，0:商用服务器，1:测试服务器
	uint8_t  mode;		//!< 0:disable 1:CMCC
	uint16_t interval;	//!< 心跳间隔，分钟
	uint16_t version;		//!< DM版本号，无效参数（兼容性设置)，默认V2.0
    uint8_t appkey[CMDMP_MAX_APPKEY]; //!< APPKEY，厂商唯一
	uint8_t pswd[CMDMP_MAX_PSWD];		//!< password，厂商唯一
} dm_config_t;

#define ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN  __attribute__ ((__section__(".noncached_zidata"),__aligned__(4)))

typedef void (*oc_sntp_callback_t)       (unsigned char * info);

#endif

