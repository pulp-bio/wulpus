#include "iis2dh.h"
#include "us_spi.h"
#include "us_defines.h"


/* TWI instance */
static const nrf_drv_twi_t IIS2DH_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

/* Flag to know when a I2C transfer has been completed */
static volatile bool IIS2DH_xfer_done = false;
/* Whether that transfer actually succeeded (false on address/data NACK) */
static volatile bool IIS2DH_xfer_ok = false;

extern volatile bool flag_add_IMU;

extern int buffer_content;
extern int buffer_counter;
extern int BLE_packet_ready;
extern ArrayList_type m_rx_buf[NUMBER_OF_XFERS*MAX_BUFFER_NUMBER_OF_US_FRAMES];

/* Layout of the accelerometer block inside a US frame.
 *
 * The six accelerometer bytes occupy the tail of the LAST packet of a frame, which is
 * the tail of the 804-byte frame the host finally receives.
 *
 * The offset must keep the block inside that packet. buffer[] is BYTES_PR_XFER_RX long,
 * and the byte one past its end is m_rx_buf[NUMBER_OF_XFERS*(k+1)].buffer[0] - the 0xFF
 * start-of-frame marker of the NEXT frame. The dongle keys on that marker to find a frame
 * boundary, so overwriting it makes the dongle discard all four packets of that frame.
 *
 * The previous offset (202 - ACC_BLOCK_BYTES) did exactly that: it clobbered the next
 * frame's marker, dropped the sixth accelerometer byte off the end of the transmitted
 * packet, and on the last ring slot wrote past m_rx_buf entirely. */
#define ACC_BLOCK_BYTES   6
#define ACC_BLOCK_OFFSET  (BYTES_PR_XFER_RX - ACC_BLOCK_BYTES)

STATIC_ASSERT(ACC_BLOCK_OFFSET + ACC_BLOCK_BYTES <= BYTES_PR_XFER_RX,
              "Accelerometer block overruns the last packet of the US frame");

uint8_t IIS2DH_buffer[805] = {};
uint16_t IIS2DH_buffer_index =0;
/* Frame counter for IIS2DH over bluetooth */
uint16_t IIS2DH_frame_number =0;


//Event Handler
static void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    /* Every terminal event has to release the waiters, not just EVT_DONE.
     * ADDRESS_NACK and DATA_NACK used to fall through without setting the flag,
     * which left IIS2DH_register_read()/_write() spinning forever and took the
     * whole main loop - and therefore the US stream - down with them. */
    IIS2DH_xfer_ok   = (p_event->type == NRF_DRV_TWI_EVT_DONE);
    IIS2DH_xfer_done = true;
}


/**
 * @brief TWI initialization function for the MCP9808 sensor
 *
 */
static void iis2dh_twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = PIN_IIS2DH_SCL,
       .sda                = PIN_IIS2DH_SDA,
       .frequency          = NRF_DRV_TWI_FREQ_400K, 
       .interrupt_priority = APP_IRQ_PRIORITY_LOWEST, 
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&IIS2DH_twi, &twi_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&IIS2DH_twi);
}

void IIS2DH_init()
{
    iis2dh_twi_init();
    IIS2DH_buffer[0] = 0xFF;
    IIS2DH_buffer[1] = 128;
    //((uint16_t*)IIS2DH_buffer)[1] = IIS2DH_frame_number++;
        IIS2DH_buffer[3] = 0;
    IIS2DH_buffer[2] = IIS2DH_frame_number++;

    IIS2DH_buffer_index = 4;
    nrf_delay_ms(500);
}

bool IIS2DH_register_read(uint8_t register_address, uint8_t * rx_buffer, uint8_t number_of_bytes)
{
    ret_code_t err_code;

    //Set the flag to false to show the receiving is not yet completed
    IIS2DH_xfer_done = false;

    // Send the register address where we want to read the data from.
    // Check the return code BEFORE waiting: if the transfer never started, no
    // event will ever arrive and the wait below would never finish.
    err_code = nrf_drv_twi_tx(&IIS2DH_twi, IIS2DH_ADDRESS, &register_address, 1, true);
    if (NRF_SUCCESS != err_code)
    {
        return false;
    }

    //Wait for the transmission to be completed
    while (IIS2DH_xfer_done == false){}
    if (IIS2DH_xfer_ok == false)
    {
        return false;
    }

    //reset the flag so that we can read data from the IIS2DH's internal register
    IIS2DH_xfer_done = false;

    // Receive the data from the IIS2DH
    err_code = nrf_drv_twi_rx(&IIS2DH_twi, IIS2DH_ADDRESS, rx_buffer, number_of_bytes);
    if (NRF_SUCCESS != err_code)
    {
        return false;
    }

    //wait until the transmission is completed
    while (IIS2DH_xfer_done == false){}

    return IIS2DH_xfer_ok;
}
bool IIS2DH_register_write(uint8_t register_address, uint8_t * wx_buffer, uint8_t number_of_bytes)
{
    ret_code_t err_code;

    //Set the flag to false to show the receiving is not yet completed
    IIS2DH_xfer_done = false;

    uint8_t wxbuffer_with_address[10];
    wxbuffer_with_address[0] = register_address;
    if (number_of_bytes>8) return false;
    for (int i = 0; i<number_of_bytes; i++){
      wxbuffer_with_address[i+1] = wx_buffer[i];
    }
   
    // Send the register address followed by the data (see the note in
    // IIS2DH_register_read() on why err_code is checked before waiting).
    err_code = nrf_drv_twi_tx(&IIS2DH_twi, IIS2DH_ADDRESS, wxbuffer_with_address, number_of_bytes+1, true);
    if (NRF_SUCCESS != err_code)
    {
        return false;
    }

    //Wait for the transmission to be completed
    while (IIS2DH_xfer_done == false){}

    return IIS2DH_xfer_ok;
}

bool setupTemp(){
  uint8_t wx_buffer[1] = {0};
  wx_buffer[0] = 0b11000000;
  bool check = IIS2DH_register_write(IIS2DH_REG_TEMP_CFG_REG,wx_buffer , 1);
  if (!check){
    return false;
  }
  uint8_t rx_buffer[1] = {0};

  check = IIS2DH_register_read(IIS2DH_REG_CTRL_REG4,rx_buffer , 1);
  if (!check){
    return false;
  }
  wx_buffer[0] = rx_buffer[0] | 0x80;
  check = IIS2DH_register_write(IIS2DH_REG_CTRL_REG4,wx_buffer , 1);
  if (!check){
    return false;
  }


}

int8_t getTemp(){
  uint8_t rx_buffer[1] = {0};
  bool check = IIS2DH_register_read(IIS2DH_REG_OUT_TEMP_H, rx_buffer, 1);
  if (!check){
    return 0;
  }
  int8_t res = rx_buffer[0];
  check = IIS2DH_register_read(IIS2DH_REG_OUT_TEMP_L, rx_buffer, 1);
  if (!check){
    return 0;
  }
  //res|= rx_buffer[0];
  return res;
  

}

bool setupAccelormeter(IIS2DH_OperatingModes mode, IIS2DH_DataRate rate, IIS2DH_FullScale fs){
  uint8_t wx_reg1_buffer[1];
  uint8_t wx_reg4_buffer[1];

  if (mode == IIS2DH_LowPowerMode){
    wx_reg1_buffer[0] = 0x0F;
  }else{
    wx_reg1_buffer[0] = 0x07;
  }
  wx_reg1_buffer[0] = wx_reg1_buffer[0] | rate; // set the odr bits
  /* BDU holds the OUT_* registers until both halves of a sample have been read,
   * so a burst read can never pair the high byte of one sample with the low byte
   * of the next. Without it the 400 Hz ODR tears samples against our frame rate. */
  wx_reg4_buffer[0] = IIS2DH_CTRL_REG4_BDU | fs;
  if (mode == IIS2DH_HighResolutionMode){
    wx_reg4_buffer[0] |= IIS2DH_CTRL_REG4_HR;
  }

  bool res = IIS2DH_register_write(IIS2DH_REG_CTRL_REG1, wx_reg1_buffer, 1);
  res &= IIS2DH_register_write(IIS2DH_REG_CTRL_REG4, wx_reg4_buffer, 1);
  return res;
}


double convert2mg(uint16_t data_in, IIS2DH_FullScale range){
  if (range == IIS2DH_Precision_2g){
    return ((double) data_in)*0.06125;
  }else if (range == IIS2DH_Precision_4g){
    return ((double) data_in)*0.121875;
  }else if (range == IIS2DH_Precision_8g){
    return ((double) data_in)*0.244375;
  }else if (range == IIS2DH_Precision_16g){
    return ((double) data_in)*0.7325;
  }
}

uint16_t getHighandLow(uint8_t register_address_L, uint8_t register_address_H, IIS2DH_OperatingModes mode){
  uint8_t rx_buffer[1] ={0};

  IIS2DH_register_read(register_address_H, rx_buffer, 1);
  uint16_t res = rx_buffer[0]<<8;
  rx_buffer[0] =0;
  IIS2DH_register_read(register_address_L, rx_buffer, 1);
  res += rx_buffer[0];

  if (mode == IIS2DH_LowPowerMode){
    res = res>>8;
  }else if (mode == IIS2DH_NormalMode){
    res = res>>6;
  }else if (mode == IIS2DH_HighResolutionMode){
    res = res>>4;
  }
  return res;

}

bool getAccelerationData(uint16_t* X, uint16_t* Y, uint16_t* Z, IIS2DH_OperatingModes mode, IIS2DH_FullScale range){
  *X = getHighandLow(IIS2DH_REG_OUT_X_L, IIS2DH_REG_OUT_X_H, mode);  
  *Y = getHighandLow(IIS2DH_REG_OUT_Y_L, IIS2DH_REG_OUT_Y_H, mode);
  *Z = getHighandLow(IIS2DH_REG_OUT_Z_L, IIS2DH_REG_OUT_Z_H, mode);
  //*X = convert2mg(X_in, range);
  //*Y = convert2mg(Y_in, range);
  //*Z = convert2mg(Z_in, range);


  return true;

}

/* Destination of the accelerometer block for the frame currently being filled. */
static uint8_t * accel_block_ptr(void)
{
    return &m_rx_buf[(NUMBER_OF_XFERS - 1) + buffer_counter * NUMBER_OF_XFERS]
                .buffer[ACC_BLOCK_OFFSET];
}

static void finalizeCurrentFrame(void)
{
    buffer_counter++;
    if (buffer_counter == MAX_BUFFER_NUMBER_OF_US_FRAMES)
    {
        buffer_counter = 0;
    }

    BLE_packet_ready = 1;
}

bool IIS2DH_set_streaming_enabled(bool enable)
{
    if (enable)
    {
        return setupAccelormeter(IIS2DH_HighResolutionMode,
                                 AllModes_400Hz,
                                 IIS2DH_Precision_2g);
    }
    else
    {
        // Power-down mode: CTRL_REG1 = 0
        uint8_t ctrl_reg1 = 0x00;
        return IIS2DH_register_write(IIS2DH_REG_CTRL_REG1, &ctrl_reg1, 1);
    }
}

void finalizeFrameWithoutIMU(void)
{
    while(flag_add_IMU == false) {}

    flag_add_IMU = false;

    /* Accelerometer streaming is off, so the tail of this frame holds real RF
     * samples. Leave it alone: the host contract for meas_mode == 0 is a full
     * ACQ_LENGTH_SAMPLES of ultrasound, not 397 samples plus three zeroed ones.
     * (This used to be zeroed, which silently discarded the last three
     * samples of every A-scan even with the IMU disabled.) */

    finalizeCurrentFrame();
}

void getIIS2DHData2Buffer(){

  /* One auto-incrementing burst read of OUT_X_L..OUT_Z_H replaces six separate
   * single-register reads. Each of those cost an address write plus a data read,
   * about 2.4 ms in total at 100 kHz, every millisecond of which blocked the main
   * loop and so stalled the BLE drain. The burst is a single transaction: roughly
   * 0.2 ms at 400 kHz. */
  uint8_t axes[ACC_BLOCK_BYTES];

  if (IIS2DH_register_read(IIS2DH_REG_OUT_X_L | IIS2DH_AUTO_INCREMENT,
                           axes, ACC_BLOCK_BYTES) == false)
  {
      // Sensor did not answer - send zeroes rather than a stale sample.
      memset(axes, 0, sizeof(axes));
  }

  /* The burst returns low byte first per axis (X_L, X_H, Y_L, ...). The wire
   * format is high byte first, which is what the host decoder expects, so swap
   * each pair on the way into the frame. */
  IIS2DH_buffer[0] = axes[1];   // X_H
  IIS2DH_buffer[1] = axes[0];   // X_L
  IIS2DH_buffer[2] = axes[3];   // Y_H
  IIS2DH_buffer[3] = axes[2];   // Y_L
  IIS2DH_buffer[4] = axes[5];   // Z_H
  IIS2DH_buffer[5] = axes[4];   // Z_L
  IIS2DH_buffer_index = ACC_BLOCK_BYTES;

  while(flag_add_IMU == false) {}

  flag_add_IMU = false;

  memcpy(accel_block_ptr(), &IIS2DH_buffer[0], ACC_BLOCK_BYTES);

  finalizeCurrentFrame();
}
