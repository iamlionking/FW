#include <string.h>

#include "sapphire_pub.h"

#include "ADS1x9x.h"
#include "BKRNLAPI.h"
#include "device_api.h"
#include "crc.h"
#include "ecg_ads.h"
#include "ecg_adc.h"
#include "ecg_data_capture.h"
#include "ecg_gpio.h"
#include "ecg_packet.h"
#include "ecg_power_ctl.h"
#include "ecg_pwm.h"
#include "ecg_status.h"
#include "ecg_ssi.h"
#include "gpio.h"
#include "interrupt.h"
#include "utils/ringbuf.h"
#include "ssi.h"
#include "sysctl.h"
#include "timer.h"

#ifdef ECG_HARDWARE_USB
#include "ecg_usb.h"
#endif

#ifdef ECG_HARDWARE_BLUETOOTH
#include "bt_transport.h"
#endif

static uint16_t packet_num;
static uint8_t drdy_int_state;
static uint8_t int_count;
static uint8_t status;
static Data_Capture_Rate_t8 capture_rate;
static uint32_t data_rate_load_val;
static uint8_t buf[4096];

static tRingBufObject ads_data_buffer;

static void ads_stop(void);
static void data_ready_handler();
static void data_rate_int();
static void init_data_rate_timer(void);
static void set_data_rate(void);


/* Public - Initializes the internal variables for the data capture system.
 * 
 * Examples
 * 
 *      ECG_Data_Capture_Init();
 * 
 * Returns nothing
 */
void ECG_Data_Capture_Init(void)
{
    RingBufInit(&ads_data_buffer, buf, 4096);
    status = 0;
	data_rate_load_val = 0;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
} /* ECG_Data_Capture_Init() */


/* Public - Gets the data capture status
 * 
 * Examples
 * 
 * 		uint8_t data_cap_in_progress = 0;
 * 		data_cap_in_progress = ECG_Data_Capture_In_Progress();
 * 		if(data_cap_in_progress)
 * 			//do something about it
 * 
 * Returns whether or not a data capture is in progress.
 */
uint8_t ECG_Data_Capture_In_Progress(void)
{
    return status;
}

//TODO: Remove if not used in the near future
uint8_t ECG_Data_Capture_Get_Queued_Data_Count(void)
{
    return int_count;
}

//TODO: Remove is not used in the near future
Data_Capture_Rate_t8 ECG_Data_Capture_Get_Rate(void)
{
    return capture_rate;
}


/* Public - Start a data captures session at the specified sample rate.  The 
 * call to this function initializes and configures the ADS hardware.  
 * Additionally, an interrupt handler is configured to handle the data ready 
 * interrupts from the ADS.  Finally, the data capture is started.
 * 
 * Note: The data capture runs indefinitely and is only stopped due to system 
 * reset or by calling the Stop function.
 * 
 * Examples
 * 
 *      ECG_Data_Capture_Start(Data_Capture_Rate_1000_SPS);
 * 
 * Returns nothing
 */
void ECG_Data_Capture_Start(Data_Capture_Rate_t8 data_rate)
{
    packet_num = 0;
	capture_rate = data_rate;
    
#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Clear_Buffer();
#endif

    RingBufFlush(&ads_data_buffer);
    
	/* Init timer for verifying data rate */
	init_data_rate_timer();
    
    /* Set SSI Bus to ADS mode */
    if(ECG_SSI_Get_Mode() != SSI_ADS_Mode)
        ECG_SSI_Set_Mode(SSI_ADS_Mode);
        
    /* Initialize ADS */
    init_ADS1x9x();
    
	/* Set ADS Data Rate */
	set_data_rate();
    
    /* Initialize Data Ready Interrupt */
    drdy_int_state = GPIO_HIGH_LEVEL;
    GPIOIntTypeSet(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN, GPIO_HIGH_LEVEL);
    GPIOIntEnable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    GPIOIntRegister(GPIO_ADS_N_DRDY_PORT, data_ready_handler);
    
    int_count = 0;
    status = 1;
    capture_rate = data_rate;
    
    /* Start ADS Conversion */
    enable_ADS1x9x_Conversion();
    
    ECG_Status_Set(ECG_Status_Data_Capture);
	TimerEnable(TIMER1_BASE, TIMER_A);
} /* ECG_Data_Capture_Start() */


/* Public - Stops the ADS data capture session.  If a capture session is not in
 * progress, this function does nothing.
 * 
 * Examples
 * 
 *      ECG_Data_Capture_Init(Data_Capture_Rate_1000_SPS);
 *      //Some time later . . .
 *      ECG_Data_Capture_Stop();
 * 
 * Returns nothing
 */
void ECG_Data_Capture_Stop(void)
{
	if(status == 0)
		return;
    
	ads_stop();
	
#ifdef ECG_HARDWARE_BLUETOOTH
    /* Send any data that might remain */
    BTT_Transport_Packets();
#endif
    
    ECG_Status_Set(ECG_Status_Normal);
} /* ECG_Data_Capture_Stop() */


/* Public - Stops the ADS data capture session.  If a capture session is not in
 * progress, this function does nothing.  This function should be called when
 * there is a potential failure in the system (specifically BT transmission).
 * The main difference is that this function does not attempt to send all
 * remaining data and simply stops the ADS from recording further information.
 * 
 * Examples
 * 
 * 		//Bluetooth transmission error occurs or the SPP port is closed
 *      if(ECG_Data_Capture_In_Progress() == 1)
 *      	ECG_Data_Capture_Hard_Stop();
 * 
 * Returns nothing
 */
void ECG_Data_Capture_Hard_Stop(void)
{
	if(status == 0)
		return;

	ads_stop();
    
    ECG_Status_Set(ECG_Status_Error);
} /* ECG_Data_Capture_Hard_Stop() */


/* Public - Restarts a data capture session after an error
 * 
 * Examples
 * 
 * 		//After a data rate error occurs
 * 		ECG_Data_Capture_Restart();
 * 
 * Returns nothing.
 */
void ECG_Data_Capture_Restart(void)
{
	
	ads_stop();
	
#ifdef ECG_HARDWARE_BLUETOOTH
	BTT_Clear_Buffer();
#endif

    RingBufFlush(&ads_data_buffer);
	
	init_data_rate_timer();
	
	Display(("Initialize ADS\r\n"));
	/* Initialize ADS */
    init_ADS1x9x();
    
	/* Set ADS Data Rate */
	set_data_rate();
    
    /* Initialize Data Ready Interrupt */
    drdy_int_state = GPIO_HIGH_LEVEL;
    GPIOIntTypeSet(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN, GPIO_HIGH_LEVEL);
    GPIOIntEnable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    GPIOIntRegister(GPIO_ADS_N_DRDY_PORT, data_ready_handler);
    
    int_count = 0;
    status = 1;
    
	Display(("Start Conversion\r\n"));
    
	/* Start ADS Conversion */
    enable_ADS1x9x_Conversion();
    
    ECG_Status_Set(ECG_Status_Data_Capture);
	TimerEnable(TIMER1_BASE, TIMER_A);
} /* ECG_Data_Capture_Restart() */


/* Public - Queues data read from the ADS device into the Bluetooth transport queue.
 * This function is typically called on an interval at a higher rate than data is
 * transported to the PC Application (1ms for this firmware).
 * 
 * Examples
 * 
 *      //Inside a 1 ms timer loop
 *      ECG_Data_Capture_Queue_Data();
 * 
 * Returns nothing
 */
void ECG_Data_Capture_Queue_Data(void)
{
    uint8_t i;
    uint8_t j;
    ECG_Packet_t packet;
    uint8_t temp_buf[ADS1x9x_DATA_SIZE];
    uint16_t bytes_avail;
    uint8_t packets_avail;
    uint8_t int_status;
    uint8_t loff_statp;
    uint8_t loff_statn;
    uint16_t loff_status;
    uint8_t pacer;
	uint8_t invalid_voltage;
    uint16_t voltage = 0;
    
    int_status = IntMasterDisable();
    bytes_avail = RingBufUsed(&ads_data_buffer);
    int_count = 0;
    if(!int_status)
        IntMasterEnable();
    
    packets_avail = bytes_avail / ADS1x9x_DATA_SIZE;
    BTPS_MemInitialize(&packet, 0x00, sizeof(ECG_Packet_t));

	ECG_ADC_GetValue(ECG_Adc_External_5V, &voltage);

	if(voltage > 4400)
	{
		invalid_voltage = 0x00;
	} else {
		invalid_voltage = 0x01;
	}

    for(i = 0; i < packets_avail; i++)
    {
        RingBufRead(&ads_data_buffer, temp_buf, ADS1x9x_DATA_SIZE);
        
        loff_statp = ((temp_buf[0] & 0x0f) << 4) + ((temp_buf[1] & 0xf0) >> 4);
        loff_statn = ((temp_buf[1] & 0x0f) << 4) + ((temp_buf[2] & 0xf0) >> 4);
        loff_status = loff_statp + (loff_statn << 8);
        pacer = (temp_buf[2] & 0x01);
		ECG_PUT_STATUS_DATA(packet.status, invalid_voltage, VOLTAGE_BIT_MASK, VOLTAGE_SHIFT);
        ECG_PUT_STATUS_DATA(packet.status, pacer, PACER_BIT_MASK, PACER_SHIFT);
        ECG_PUT_STATUS_DATA(packet.status, loff_status, LEAD_OFF_STATUS_MASK, 0);
        ECG_PUT_STATUS_DATA(packet.status, packet_num++, SEQUENCE_NUM_MASK, SEQUENCE_NUM_SHIFT);
        
        if(packet_num > 4095)
            packet_num = 0;

        for(j = 0; j < 8; j++)
        {
            packet.msb[j] = (int8_t)temp_buf[3 + j * 3];
            packet.lsw[j] = (temp_buf[4 + j * 3] << 0x08) + temp_buf[5 + j * 3];
        }
        
#ifdef ECG_HARDWARE_USB
		USB_Queue_Packet(&packet);
#elif ECG_HARDWARE_BLUETOOTH
        BTT_Queue_Packet(&packet);
#endif
    }
} /* ECG_Data_Capture_Queue_Captured_Data() */


/* Internal - Performs necessary actions for stop data
 * recption from the ADS.
 * 
 * Returns nothing
 */
static void ads_stop(void)
{
	TimerIntDisable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    TimerDisable(TIMER1_BASE, TIMER_A);
    
	IntMasterDisable();
    
    Soft_Stop_ADS1x9x();
    Stop_Read_Data_Continuous();
    
	GPIOIntDisable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    GPIOIntUnregister(GPIO_ADS_N_DRDY_PORT);
    
	IntMasterEnable();
	
	status = 0;
} /* ads_stop() */


/* Internal - Handles interrupts from the ADS and puts the data read from the ADS
 * in a buffer for later queueing.
 * 
 * Returns nothing
 */
static void data_ready_handler(void)
{
    uint8_t i;
    uint32_t temp;
    
    /* Make sure we are looking at the correct interrupt */
    uint32_t status = GPIOIntStatus(GPIO_ADS_N_DRDY_PORT, true);
    
    /* Don't handle interrupt if not data ready pin */
    if(!(status & GPIO_ADS_N_DRDY_PIN))
    {
        return;
    }
    
    GPIOIntClear(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    GPIOIntDisable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
    if(drdy_int_state == GPIO_HIGH_LEVEL)
    {
        GPIOIntTypeSet(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN, GPIO_LOW_LEVEL);
        drdy_int_state = GPIO_LOW_LEVEL;
    }
    else
    {
        //TimerDisable(TIMER1_BASE, TIMER_A);
        /* Set Interrupt to wait for high level so we don't constantly trigger */
        GPIOIntTypeSet(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN, GPIO_HIGH_LEVEL);
        drdy_int_state = GPIO_HIGH_LEVEL;
        
        Set_ADS1x9x_Chip_Enable();
        
        for(i = 0; i < 8; i++)
        {
            SSIDataPut(SSI0_BASE, 0x00);
        }
        
        while(SSIBusy(SSI0_BASE));
        
        for(i = 0; i < 8; i++)
        {
            SSIDataGet(SSI0_BASE, &temp);
            RingBufWriteOne(&ads_data_buffer, (uint8_t)temp);
        }
        
        for(i = 0; i < 8; i++)
        {
            SSIDataPut(SSI0_BASE, 0x00);
        }
        
        while(SSIBusy(SSI0_BASE));
        
        for(i = 0; i < 8; i++)
        {
            SSIDataGet(SSI0_BASE, &temp);
            RingBufWriteOne(&ads_data_buffer, (uint8_t)temp);
        }
        
        for(i = 0; i < 8; i++)
        {
            SSIDataPut(SSI0_BASE, 0x00);
        }
        
        while(SSIBusy(SSI0_BASE));
        
        for(i = 0; i < 8; i++)
        {
            SSIDataGet(SSI0_BASE, &temp);
            RingBufWriteOne(&ads_data_buffer, (uint8_t)temp);
        }
        
        for(i = 0; i < 3; i++)
        {
            SSIDataPut(SSI0_BASE, 0x00);
        }
        
        while(SSIBusy(SSI0_BASE));
        
        for(i = 0; i < 3; i++)
        {
            SSIDataGet(SSI0_BASE, &temp);
            RingBufWriteOne(&ads_data_buffer, (uint8_t)temp);
        }

        Clear_ADS1x9x_Chip_Enable();        
        int_count++;
        
		TimerPrescaleSet(TIMER1_BASE, TIMER_A, 0x08);
        TimerLoadSet(TIMER1_BASE, TIMER_A, data_rate_load_val);
        TimerEnable(TIMER1_BASE, TIMER_A);
    }
    GPIOIntEnable(GPIO_ADS_N_DRDY_PORT, GPIO_ADS_N_DRDY_PIN);
} /* data_ready_handler() */


/* Internal - Data Rate Interrupt Handler
 *            If this interrupt occurs, then a data rate error has occurred.
 * 
 * Returns nothing
 */
void data_rate_int(void)
{
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    
    /* If this is entered, then the data rate is not correct */
    TimerDisable(TIMER1_BASE, TIMER_A);
    g_DataRateError = 1;
} /* data_rate_int() */


/* Internal - Initializes the data rate error checking timer
 * 
 * Returns nothing
 */
void init_data_rate_timer(void)
{
//    TimerConfigure(TIMER1_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT));
    TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT | TIMER_CFG_B_ONE_SHOT);
    TimerPrescaleSet(TIMER1_BASE, TIMER_A, 250);
	TimerLoadSet(TIMER1_BASE, TIMER_A, 64000);
    TimerIntRegister(TIMER1_BASE, TIMER_A, data_rate_int);
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
} /* init_data_rate_timer() */


/* Internal - Sets the ADS data rate
 * 
 * Returns nothing
 */
void set_data_rate(void)
{
	uint8_t ads_rate;
	
	/* Set ADS Data Rate */
    if(capture_rate == Data_Capture_Rate_2000_SPS)
    {
        ads_rate = 0x03;
        data_rate_load_val = 1200 * 10;
    }
    else if(capture_rate == Data_Capture_Rate_1000_SPS)
    {
        ads_rate = 0x04;
		data_rate_load_val = 1500 * 10;
    }
    else if(capture_rate == Data_Capture_Rate_500_SPS)
    {
        ads_rate = 0x05;
        data_rate_load_val = 2500 * 10;
    }
    else if(capture_rate == Data_Capture_Rate_250_SPS)
    {
        ads_rate = 0x06;
        data_rate_load_val = 4500 * 10;
    }
    else
    {
        ads_rate = 0x06;
        data_rate_load_val = 4500 * 10;
    }
	
	ADS1x9x_SPI_Address_Byte_Count(WRITE_CONFIG_1_REGISTER, SINGLE_BYTE_READ_WRITE);
    ADS1x9x_SPI_Data(ads_rate);
} /* set_data_rate() */
