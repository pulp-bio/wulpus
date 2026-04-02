#ifndef IIS2DH_Breakout
#define IIS2DH_Breakout

#include <stdio.h>
#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "us_ble.h"


#include "nrf_log_backend_rtt.h"
#include "nrf_log.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_ctrl.h"

#define IIS2DH_REG_WHOAMI 0x0F
#define IIS2DH_REG_OUT_X_L 0x28
#define IIS2DH_REG_OUT_X_H 0x29
#define IIS2DH_REG_OUT_Y_L 0x2A
#define IIS2DH_REG_OUT_Y_H 0x2B
#define IIS2DH_REG_OUT_Z_L 0x2C
#define IIS2DH_REG_OUT_Z_H 0x2D

#define IIS2DH_REG_CTRL_REG1  0x20
#define IIS2DH_REG_CTRL_REG2  0x21
#define IIS2DH_REG_CTRL_REG3  0x22
#define IIS2DH_REG_CTRL_REG4  0x23
#define IIS2DH_REG_CTRL_REG5  0x24
#define IIS2DH_REG_CTRL_REG6  0x25

#define IIS2DH_REG_STATUS_REG_AUX 0x07
#define IIS2DH_REG_OUT_TEMP_L 0x0C
#define IIS2DH_REG_OUT_TEMP_H 0x0D

#define IIS2DH_REG_INT_COUNTER_REG
#define IIS2DH_REG_TEMP_CFG_REG 0x1F
#define IIS2DH_REG_REFERENCE
#define IIS2DH_REG_STATUS_REG


// 
#define IIS2DH_ADDRESS     0x18


#define PIN_IIS2DH_SCL     31
#define PIN_IIS2DH_SDA     30

#define PIN_IIS2DH_INT1   12
#define PIN_IIS2DH_INT2   13

// TWI instance ID
#if TWI0_ENABLED
#define TWI_INSTANCE_ID     0
#elif TWI1_ENABLED
#define TWI_INSTANCE_ID     1
#endif


extern uint8_t IIS2DH_buffer[805];
extern uint16_t IIS2DH_buffer_index;
/* Frame counter for IIS2DH over bluetooth */
extern uint16_t IIS2DH_frame_number;


typedef enum {
  IIS2DH_LowPowerMode = 0,
  IIS2DH_NormalMode = 1,
  IIS2DH_HighResolutionMode = 2
} IIS2DH_OperatingModes;

typedef enum {
  AllModes_1Hz = 0x10,
  AllModes_10Hz = 0x20,
  AllModes_25Hz = 0x30,
  AllModes_50Hz = 0x40,
  AllModes_100Hz = 0x50,
  AllModes_200Hz = 0x60,
  AllModes_400Hz = 0x70,
  LowPower_1620Hz = 0x80,
  LowPower_5376Hz = 0x90,
  HighResolution_1344Hz = 0x90,
  Normal_1344Hz = 0x90
} IIS2DH_DataRate;

typedef enum {
  IIS2DH_Precision_2g = 0x00,
  IIS2DH_Precision_4g = 0x10,
  IIS2DH_Precision_8g = 0x20,
  IIS2DH_Precision_16g = 0x30,
}IIS2DH_FullScale;

/**
 * @brief Function for initializing the GPIO and TWI peripheral for the IIS2DH sensor
 *
 */
void IIS2DH_init();

/**
 * @brief Function for reading a register from the IIS2DH sensor
 *
 * @param[in] register_address The address of the register to read from
 * @param[out] rx_buffer The buffer to store the received data
 * @param[in] number_of_bytes The number of bytes to read
 *
 * @retval true If the read was successful
 * @retval false If the read was not successful
 *
 */
bool IIS2DH_register_read(uint8_t register_address, uint8_t * rx_buffer, uint8_t number_of_bytes);

bool IIS2DH_register_write(uint8_t register_address, uint8_t * wx_buffer, uint8_t number_of_bytes);

bool setupTemp();
int8_t getTemp();

bool setupAccelormeter(IIS2DH_OperatingModes mode, IIS2DH_DataRate rate, IIS2DH_FullScale fs);
bool getAccelerationData(uint16_t* X, uint16_t* Y, uint16_t* Z, IIS2DH_OperatingModes mode, IIS2DH_FullScale range);

bool IIS2DH_set_streaming_enabled(bool enable);
void finalizeFrameWithoutIMU(void);

void getIIS2DHData2Buffer();
#endif


