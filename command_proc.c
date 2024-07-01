#include "sapphire_pub.h"

#include "inc/hw_types.h"
#include "utils/ringbuf.h"
#include "device_api.h"
#include "crc.h"
#include "ecg_adc.h"
#include "ecg_data_capture.h"
#include "ecg_gpio.h"
#include "ecg_init.h"
#include "ecg_packet.h"
#include "ecg_post.h"
#include "ecg_led.h"
#include "ecg_pwm.h"
#include "ecg_power_ctl.h"
#include "ecg_speaker.h"
#include "ecg_status.h"
#include "ecg_test.h"
#include "ecg_usb.h"
#include "ecg_version.h"
#include "flash.h"
#include "firmware_download.h"
#include "gpio.h"
#include "../Bootloader/ecg_boot_version.h"
#include "interrupt.h"
#include "serial_num.h"
#include "test_transfer.h"
#include "ecg_unique_id.h"

#include "ADS1x9x.h"

#ifdef ECG_HARDWARE_BATTERY
#include "ecg_battery.h"
#endif

#ifdef ECG_HARDWARE_BLUETOOTH
#include "bluetooth.h"
#include "bt_transport.h"
#include "BTVSAPI.h"
#include "RFCOMAPI.h"
#include "SPPAPI.h"
#include "HCIAPI.h"
#endif

typedef struct ECG_Command_Proc_struct
{
    uint32_t    Command;
    void        (*proc)(const CardeaDeviceCommand_t * command);
} ECG_Command_Proc_t;

static void proc_battery_status_command(const CardeaDeviceCommand_t * command);
static void proc_beep_command(const CardeaDeviceCommand_t *command);
static void proc_data_start_command(const CardeaDeviceCommand_t *command);
static void proc_data_stop_command(const CardeaDeviceCommand_t * command);
static void proc_debug_command(const CardeaDeviceCommand_t *command);
static void proc_enable_test_mode_command(const CardeaDeviceCommand_t * command);
static void proc_get_post_result_command(const CardeaDeviceCommand_t * command);
static void proc_get_secret_code_command(const CardeaDeviceCommand_t * command);
static void proc_led_command(const CardeaDeviceCommand_t *command);
static void proc_revision_command(const CardeaDeviceCommand_t *command);
static void proc_rssi_command(const CardeaDeviceCommand_t *command);
static void proc_save_pairing_command(const CardeaDeviceCommand_t * command);
static void proc_set_device_id_info(const CardeaDeviceCommand_t * command);
static void proc_set_tx_power_command(const CardeaDeviceCommand_t * command);
static void proc_update_firmware_command(const CardeaDeviceCommand_t * command);
static void proc_update_firmware_done_command(const CardeaDeviceCommand_t * command);
static void proc_get_unique_id_command(const CardeaDeviceCommand_t * command);
static void proc_begin_keepalive_command(const CardeaDeviceCommand_t * command);
static void proc_keepalive_command(const CardeaDeviceCommand_t * command);
static void proc_end_keepalive_command(const CardeaDeviceCommand_t * command);


static Word_t spp_hci_connection_handle = 0;
#if defined(__ECG_DEBUG__) && defined(ECG_HARDWARE_BATTERY)
    static uint8_t batt_charge_status = 0;
#endif

static const ECG_Command_Proc_t command_proc_list[] =
{
    {
        CARDEA_CMD_LED_CONTROL,
        proc_led_command
    },
    {
        CARDEA_CMD_BEEP,
        proc_beep_command
    },
    {
        CARDEA_CMD_START_DATA,
        proc_data_start_command
    },
    {
        CARDEA_CMD_STOP_DATA,
        proc_data_stop_command
    },
    {
        CARDEA_CMD_SAVE_PAIRING,
        proc_save_pairing_command
    },
    {
        CARDEA_CMD_GET_REVISION,
        proc_revision_command
    },
    {
        CARDEA_CMD_GET_BATTERY_STATUS,
        proc_battery_status_command
    },
    {
        CARDEA_CMD_GET_POST_RESULT,
        proc_get_post_result_command
    },
    {
        CARDEA_CMD_UPDATE_FIRMWARE,
        proc_update_firmware_command
    },
    {
        CARDEA_CMD_UPDATE_FIRMWARE_DONE,
        proc_update_firmware_done_command
    },
    {
        CARDEA_CMD_ENABLE_TEST_MODE,
        proc_enable_test_mode_command
    },
    {
        CARDEA_CMD_SET_XMIT_POWER,
        proc_set_tx_power_command
    },
    {
        CARDEA_CMD_GET_LINK_QUALITY,
        proc_rssi_command
    },
    {
        CARDEA_CMD_SET_DEVICE_ID_INFO,
        proc_set_device_id_info
    },
    {
        CARDEA_CMD_GET_SECRET_CODE,
        proc_get_secret_code_command
    },
	{
		CARDEA_CMD_GET_UNIQUE_ID,
		proc_get_unique_id_command
	},
	{
		CARDEA_CMD_BEGIN_KEEPALIVE,
		proc_begin_keepalive_command
	},
	{
		CARDEA_CMD_KEEPALIVE,
		proc_keepalive_command
	},
	{
		CARDEA_CMD_END_KEEPALIVE,
		proc_end_keepalive_command
	},
};


void Process_Command(const CardeaDeviceCommand_t * command)
{   
    uint8_t command_val;
    
    command_val = (command->command & 0xff000000) >> 24;
    if(command_val >= MAX_COMMAND_VALUE && command_val != 0xBB)
    {
        return;
    }
    
#ifdef ECG_HARDWARE_BATTERY
    ECG_Power_Reset_Off_Timer();
#endif
    
    //Display(("Processing Command %d with param %d\r\n", command->command, command->param));
    if(command->command == CARDEA_CMD_DEBUG)
        proc_debug_command(command);
    else
        command_proc_list[command_val].proc(command);
    
} /* Process_Command() */


void Set_HCI_Connection_Handle(Word_t conn_handle)
{
    spp_hci_connection_handle = conn_handle;
} /* Set_HCI_Connection_Handle() */


static void proc_battery_status_command(const CardeaDeviceCommand_t * command)
{
    CardeaCmdGetBatteryStatusResponse_t response;

#ifdef ECG_HARDWARE_BATTERY
    uint32_t batt_level = 0;
	uint16_t batt_adc = 0;
    uint8_t status = 0;
    /* Check if external 5V or USB connection is present.  If not, then the status should be 11 and not the status
     * bits read from the charger. */
    
    Display(("Received Battery Status Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    ECG_Battery_Get_Level(&batt_level);
    ECG_Battery_Get_Status(&status);
	ECG_ADC_GetRaw(ECG_Adc_Battery_Voltage, &batt_adc);
	
	Display(("Status Bits: S1: %d  S2: %d\r\n", (status & 0x01), (status & 0x02) >> 1));
    
    response.cmd = CARDEA_CMD_GET_BATTERY_STATUS;
    response.status = batt_level;
	response.status |= (batt_adc << 8);
    response.status |= (status << 24);
	
	Display(("Status after shift: %x\r\n", response.status));
    Display(("Battery Percentage: %d\r\n", response.status & 0xff));
	Display(("Battery ADC Val: %d\r\n", (response.status & 0x0003ff00) >> 8));
#else
	response.status = 0;
	response.cmd = CARDEA_CMD_GET_BATTERY_STATUS;
#endif	

#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&response, sizeof(CardeaCmdGetBatteryStatusResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&response, sizeof(CardeaCmdGetBatteryStatusResponse_t));
#endif
} /* proc_battery_status() */


static void proc_beep_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received Beep Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    if(ECG_Speaker_Get_State() == SPEAKER_OFF)
    {
        ECG_Speaker_Enable(command->param);
    }
} /* proc_beep_command() */


static void proc_data_start_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received Data Start Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    if(ECG_Status_Check(ECG_Status_Error) & ECG_Status_Error)
    {
        Display(("Device is in Error State; Cannot start data acquisition\r\n"));
        return;
    }    
    if(command->param == 250)
        ECG_Data_Capture_Start(Data_Capture_Rate_250_SPS);
    else if(command->param == 500)
        ECG_Data_Capture_Start(Data_Capture_Rate_500_SPS);
    else if(command->param == 1000)
        ECG_Data_Capture_Start(Data_Capture_Rate_1000_SPS);
    else if(command->param == 2000)
        ECG_Data_Capture_Start(Data_Capture_Rate_2000_SPS);
} /* proc_data_start_command() */


static void proc_data_stop_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received Data Stop Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    
    ECG_Data_Capture_Stop();
} /* proc_data_stop_command() */


static void proc_debug_command(const CardeaDeviceCommand_t * command)
{
#if defined(__ECG_DEBUG__)
    uint8_t i;
    uint16_t adc_raw;
    uint16_t adc_val;
    uint8_t adc_name[15];
    
    Display(("Received Debug Command %d %c.\r\n", ((command->command & 0xff000000) > 24), (uint8_t)(command->param)));
    if(command->param == (uint32_t)'5')
    {
        /* Toggle 5V supply */
        if(GPIOPinRead(GPIO_PS5V0_EN_PORT, GPIO_PS5V0_EN_PIN))
        {
            Display(("Turing 5V Supply Off.\r\n"));
            GPIOPinWrite(GPIO_PS5V0_EN_PORT, GPIO_PS5V0_EN_PIN, ~GPIOPinRead(GPIO_PS5V0_EN_PORT, GPIO_PS5V0_EN_PIN));
        }
        else
        {
            Display(("Turing 5V Supply On.\r\n"));
            GPIOPinWrite(GPIO_PS5V0_EN_PORT, GPIO_PS5V0_EN_PIN, ~GPIOPinRead(GPIO_PS5V0_EN_PORT, GPIO_PS5V0_EN_PIN));
        }
    }
    
    if(command->param == (uint32_t)'v')
    {
        for(i = 0; i < ECG_Adc_Count; i++)
        {
            ECG_ADC_GetValue(i, &adc_val);
            ECG_ADC_GetName(i, adc_name);
            ECG_ADC_GetRaw(i, &adc_raw);
            
            Display(("%s: %dmV %x Raw\r\n", adc_name, adc_val, adc_raw));
        }
    }
    if(command->param == (uint32_t)'t')
    {
        /* Run Tests */
        Test_ECG_Flash();
    }
    if(command->param == (uint32_t)'r')
    {
        /* Toggle reset line */
		GPIOPinWrite(GPIO_ADS_N_RESET_PORT, GPIO_ADS_N_RESET_PIN, ~GPIO_ADS_N_RESET_PIN);
		BTPS_Delay(1);
		GPIOPinWrite(GPIO_ADS_N_RESET_PORT, GPIO_ADS_N_RESET_PIN, GPIO_ADS_N_RESET_PIN);
    }
    if(command->param == (uint32_t)'b')
    {
#ifdef ECG_HARDWARE_BATTERY
        uint32_t batt_level;
        uint8_t status;
        ECG_Battery_Get_Level(&batt_level);
        ECG_Battery_Get_Status(&status);
        
        Display(("Battery Perc: %d\r\n", batt_level));
        if(status == 3)
            Display(("Not Charging\r\n"));
        else if(status == 2)
            Display(("Fully Charged\r\n"));
        else if(status == 1)
            Display(("Charging\r\n"));
        else if(status == 0)
            Display(("Battery Bad; Replace\r\n"));
#else
		Display(("Firmware compiled without battery support\r\n"));
#endif
    }
    if(command->param == (uint32_t)'n')
    {
        FlashErase(SERIAL_NUMBER_ADDRESS);
    }
    if(command->param == (uint32_t)'s')
    {
        Serial_Code_t serial_code;
        
        Read_Serial_Code(&serial_code);
        Display(("Read Serial Number: %s\r\n", serial_code.serial_num));
        Display(("Read Secret Code: %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\r\n", serial_code.secret_code[0], serial_code.secret_code[1], 
                serial_code.secret_code[2], serial_code.secret_code[3], serial_code.secret_code[4], serial_code.secret_code[5], 
                serial_code.secret_code[6], serial_code.secret_code[7], serial_code.secret_code[8], serial_code.secret_code[9], 
                serial_code.secret_code[10], serial_code.secret_code[11], serial_code.secret_code[12], serial_code.secret_code[13], 
                serial_code.secret_code[14], serial_code.secret_code[15]));
    }
    if(command->param == (uint32_t)'d')
    {
        if(g_TestTransfer == 0)
        {
            g_TestTransfer = 1;
            Display(("Enabling Data Transfer Test Mode\r\n"));
        }
        else
        {
            g_TestTransfer = 0;
            Display(("Disabling Data Transfer Test Mode\r\n"));
        }
    }
    if(command->param == (uint32_t)'L')
    {
        g_TestTransfer = 1;
        Display(("Enabling Low Battery Status\r\n"));
        ECG_Status_Set(ECG_Status_Low_Battery);
    }
    if(command->param == (uint32_t)'l')
    {
        g_TestTransfer = 0;
        Display(("Disabling Low Battery Status\r\n"));
        ECG_Status_Clear(ECG_Status_Low_Battery);
    }
    if(command->param == (uint32_t)'C')
    {
#ifdef ECG_HARDWARE_BATTERY
        g_TestTransfer = 0;
        batt_charge_status = Battery_Charge_Full;
        ECG_Battery_Set_Charge_Status(batt_charge_status);
        Display(("Setting Battery Charge Status to Full; no longer in test mode.\r\n"));
#else
		Display(("Firmware compiled without battery support.\r\n"));
#endif

    }
    if(command->param == (uint32_t)'c')
    {
#ifdef ECG_HARDWARE_BATTERY
        g_TestTransfer = 1;
        if(batt_charge_status == Battery_Charge_Full)
        {
            batt_charge_status = Battery_Charge_Low;
            Display(("Setting Battery Charge Status to Low\r\n"));
        }
        else if(batt_charge_status == Battery_Charge_Low)
        {
            batt_charge_status = Battery_Charge_High;
            Display(("Setting Battery Charge Status to High\r\n"));
        }
        else if(batt_charge_status == Battery_Charge_High)
        {
            batt_charge_status = Battery_Charge_Full;
            Display(("Setting Battery Charge Status to Full\r\n"));
        }
        else
        {
            batt_charge_status = Battery_Charge_Low;
            Display(("Setting Battery Charge Status to Low\r\n"));
        }
        ECG_Battery_Set_Charge_Status(batt_charge_status);
#else
		Display(("Firmware compiled without battery support.\r\n"));
#endif
    }
    if(command->param == (uint32_t)'E')
    {
        ECG_Status_Set(ECG_Status_Error);
    }
    if(command->param == (uint32_t)'e')
    {
        ECG_Status_Set(ECG_Status_Normal);
    }
    if(command->param == (uint32_t)'S')
    {
        Erase_Serial_Code();
    }
	if(command->param == (uint32_t)'u')
	{
		USB_Reset();
	}
#endif
} /* proc_debug_command() */


static void proc_enable_test_mode_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received ADS Test Mode Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    ADS1x9x_Enable_Test_Mode(command->param);
} /* proc_enable_test_mode_command() */


static void proc_get_post_result_command(const CardeaDeviceCommand_t * command)
{
    CardeaCmdGetPOSTResult_t post_result;
	uint32_t result;
	
    Display(("Received Get POST Result Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    post_result.cmd = CARDEA_CMD_GET_POST_RESULT;
	result = ECG_POST_Get_Result();
    post_result.status = ECG_POST_Get_Result();
#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&post_result, sizeof(CardeaCmdGetPOSTResult_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&post_result, sizeof(CardeaCmdGetPOSTResult_t));
#endif
	Display(("Result Data: %x", result));
} /* proc_get_post_result_command() */


static void proc_get_secret_code_command(const CardeaDeviceCommand_t * command)
{
    CardeaCmdGetSecretCodeResponse_t response;
    Serial_Code_t serial_code;
    
    response.cmd = CARDEA_CMD_GET_SECRET_CODE;
    if(Read_Serial_Code(&serial_code) == 1)
    {
        BTPS_MemCopy(response.secret_code, serial_code.secret_code, SECRET_CODE_LEN);
    }
    else
    {
        BTPS_MemInitialize(response.secret_code, 0x00, SECRET_CODE_LEN);
    }
    
#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&response, sizeof(CardeaCmdGetSecretCodeResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&response, sizeof(CardeaCmdGetSecretCodeResponse_t));
#endif
} /* proc_get_secret_code_command() */


static void proc_led_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received LED Control Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    if(command->param == 0)
    {
        ECG_Status_Set(ECG_Status_Normal);
        ECG_LED_Disable(ECG_LED_1);
    }
    else if(command->param == 1)
    {
        ECG_Status_Set(ECG_Status_Normal);
        ECG_LED_Enable(ECG_LED_1);
    }
    else if(command->param == 2)
    {
        ECG_Status_Set(ECG_Status_Flash);
    }
} /* proc_led_command() */


static void proc_revision_command(const CardeaDeviceCommand_t * command)
{
    CardeaCmdGetRevisionResponse_t revision;
    char hardware_rev = 0x00;
    char hw_rev_2 = 0x30;
    
#ifdef ECG_HARDWARE_BLUETOOTH
    tDeviceInfo sDeviceInfo;
#else
	static const char* bt_version = "N/A";
#endif

    hardware_rev |= (GPIOPinRead(GPIO_HARD_REV_B0_PORT, GPIO_HARD_REV_B0_PIN)) +
                    (GPIOPinRead(GPIO_HARD_REV_B1_PORT, GPIO_HARD_REV_B1_PIN)) +
                    (GPIOPinRead(GPIO_HARD_REV_B2_PORT, GPIO_HARD_REV_B2_PIN)) +
                    (GPIOPinRead(GPIO_HARD_REV_B3_PORT, GPIO_HARD_REV_B3_PIN));

    hardware_rev = hardware_rev >> 2;
        
    hardware_rev = ((~hardware_rev) & 0x0f) + 0x01;
    
	if(hardware_rev <= 0x08)
	{
		// Sapphire Device
		// Sapphire revisions count up from 0000 to 0111
        hw_rev_2 = hardware_rev + 0x30;
		BTPS_SprintF(revision.hw_version, "Sapphire Rev 0%c", hw_rev_2);
	}
	else
	{
		// Ruby Device
		// Ruby revisions count down from 1111 to 1000
        hw_rev_2 = (hardware_rev - 17) * -1 + 0x30;
		BTPS_SprintF(revision.hw_version, "Ruby Rev 0%c", hw_rev_2);
	}
/*
    if(hardware_rev > 0x09)
    {
        hw_rev_2 = 0x31;
        hardware_rev = hardware_rev - 0x0a;
    }
    
    hardware_rev += 0x30;
*/  
    Display(("Received Get Revision Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    BTPS_SprintF(revision.app_version, "%d.%d.%d", ECG_VERSION_MAJOR, ECG_VERSION_MINOR, ECG_VERSION_BUILD);
    BTPS_SprintF(revision.boot_version, "%d.%d.%d", ECG_BOOT_VERSION_MAJOR, ECG_BOOT_VERSION_MINOR, ECG_BOOT_VERSION_BUILD);

#ifdef ECG_HARDWARE_BLUETOOTH
    GetLocalDeviceInformation(&sDeviceInfo);
    BTPS_SprintF(revision.bt_version, g_pcHCIVersionStrings[sDeviceInfo.ucHCIVersion]);
#else
	BTPS_SprintF(revision.bt_version, bt_version);
#endif
/*
    BTPS_SprintF(revision.hw_version, "Rev %c%c", hw_rev_2, hardware_rev);
*/
    revision.command = CARDEA_CMD_GET_REVISION;
    
#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&revision, sizeof(CardeaCmdGetRevisionResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&revision, sizeof(CardeaCmdGetRevisionResponse_t));
#endif
} /* proc_revision_cmd() */


static void proc_rssi_command(const CardeaDeviceCommand_t * command)
{
    CardeaCmdGetLinkQualityResponse_t rssi_response;
    int8_t rssi = 0;
    uint8_t link_quality = 0;

#ifdef ECG_HARDWARE_BLUETOOTH
    uint8_t status = 0;
    Word_t handle_result = 0;
#endif
    
    Display(("Received Get RSSI Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    
    rssi_response.cmd = command->command;
    
#ifdef ECG_HARDWARE_BLUETOOTH
    HCI_Read_RSSI(g_uiBluetoothStackID, spp_hci_connection_handle, &status, &handle_result, (uint8_t*)&rssi);
    HCI_Get_Link_Quality(g_uiBluetoothStackID, spp_hci_connection_handle, &status, &handle_result, &link_quality);
#endif
    
    rssi_response.rssi = rssi;
    rssi_response.link_quaility = link_quality;
    
#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&rssi_response, sizeof(CardeaCmdGetLinkQualityResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&rssi_response, sizeof(CardeaCmdGetLinkQualityResponse_t));
#endif
} /* proc_rssi_cmd() */


static void proc_save_pairing_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received Save Pairing Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
} /* proc_save_pairing_command() */


static void proc_set_device_id_info(const CardeaDeviceCommand_t * command)
{
    CardeaCmdSetDeviceIdInfo_t dev_info;
    CardeaCmdSetDeviceIdInfoResponse_t dev_info_response;
    Serial_Code_t serial_code;
    uint8_t result;
    
#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Get_Device_Info_Packet(&dev_info);
#elif ECG_HARDWARE_USB
    USB_Get_Device_Info_Packet(&dev_info);
#endif
    
    Display(("Received Set Device Info Command %d %d.\r\n", ((dev_info.cmd & 0xff000000) >> 24), dev_info.param ));
    
    BTPS_MemCopy(serial_code.serial_num, dev_info.serial_number, SERIAL_NUM_LEN);
    BTPS_MemCopy(serial_code.secret_code, dev_info.secret_code, SECRET_CODE_LEN);
    serial_code.crc = dev_info.crc;
    
    /* Validate CRC */
    if(crc(&serial_code, sizeof(serial_code), ~0) != 0)
    {
		Display(("Received serial code %s\r\n", dev_info.serial_number));
		Display(("Received crc %d\r\n", dev_info.crc));
        Display(("CRC on Device Info is not valid!\r\n"));
        result = 1;
    }
    else
    {
        Display(("Attempting to set serial number to %s.\r\n", serial_code.serial_num));
        
        result = Write_Serial_Code(&serial_code);
    }
    
    dev_info_response.cmd = CARDEA_CMD_SET_DEVICE_ID_INFO;
    if(result == 1)
        dev_info_response.status = 0;
    else
        dev_info_response.status = 1;

#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&dev_info_response, sizeof(CardeaCmdSetDeviceIdInfoResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&dev_info_response, sizeof(CardeaCmdSetDeviceIdInfoResponse_t));

	// Restart to communicate the new serial number to the USB host
    g_iResetAfterUpdate = 1;
#endif
} /* proc_set_device_id_info() */


static void proc_set_tx_power_command(const CardeaDeviceCommand_t * command)
{
#ifdef ECG_HARDWARE_BLUETOOTH
    int32_t result;
    if(command->param >= 0 && command->param < 13)
    {
        result = VS_Set_Max_Output_Power(g_uiBluetoothStackID, (uint8_t)command->param);
        if(result < 0)
            Display(("Error setting Max TX Power!\r\n"));
    }
    else
        Display(("Invalid TX Power Setting!!\r\n"));
#endif
} /* proc_set_tx_power_command() */


static void proc_update_firmware_command(const CardeaDeviceCommand_t * command)
{
    CardeaFirmwareUpdateBlock_t firmware_packet;
    if(command->param == 0)
    {
#ifdef ECG_HARDWARE_BLUETOOTH
        BTT_Get_Firmware_Packet(&firmware_packet);
#elif ECG_HARDWARE_USB
        USB_Get_Firmware_Packet(&firmware_packet);
#endif
        ECG_Firmware_Download_Proc(&firmware_packet);
    }
    else
    {
        ECG_Firmware_Download_Proc((void*)&command->param);
    }
} /* proc_update_firmware_command() */


static void proc_update_firmware_done_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received Firmware Download Finished Command\r\n"));
    ECG_Firmware_Download_Finalize();
} /* proc_update_firmware_done_command() */


static void proc_get_unique_id_command(const CardeaDeviceCommand_t * command)
{
    CardeaCmdGetUniqueIdResponse_t id_response;

    Display(("Received Get Unique Id Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));
    
    id_response.cmd = command->command;
    
	get_unique_id(&id_response);

#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&id_response, sizeof(CardeaCmdGetUniqueIdResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&id_response, sizeof(CardeaCmdGetUniqueIdResponse_t));
#endif
} /* proc_get_unique_id_command() */

static void proc_begin_keepalive_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received Begin Keepalive Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));

	g_KeepaliveCounter = KEEPALIVE_INTERVAL;
	g_KeepaliveEnabled = true;
} /* proc_begin_keepalive_command() */

static void proc_keepalive_command(const CardeaDeviceCommand_t * command)
{
	CardeaCmdKeepaliveResponse_t keepalive_response;

    Display(("Received Keepalive Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));

	keepalive_response.cmd = command->command;

	g_KeepaliveCounter = KEEPALIVE_INTERVAL;	

#ifdef ECG_HARDWARE_BLUETOOTH
    BTT_Send_Command_Response(&keepalive_response, sizeof(CardeaCmdKeepaliveResponse_t));
#elif ECG_HARDWARE_USB
    USB_Send_Command_Response(&keepalive_response, sizeof(CardeaCmdKeepaliveResponse_t));
#endif
} /* proc_keepalive_command() */

static void proc_end_keepalive_command(const CardeaDeviceCommand_t * command)
{
    Display(("Received End Keepalive Command %d %d.\r\n", ((command->command & 0xff000000) > 24), command->param ));

	g_KeepaliveCounter = KEEPALIVE_INTERVAL;
	g_KeepaliveEnabled = false;
}
