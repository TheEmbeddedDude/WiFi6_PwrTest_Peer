/**
  ******************************************************************************
  * @file    main.c
  * @author  The Embedded Dude
  * @brief   WiFi6 power saving test  
  * @date    Git controlled
  * @version Git controlled  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy;
  * The MIT License (MIT) 
  * Copyright (c) 2024, The Embedded Dude, ToDo: Add github link
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  * 
  * The above copyright notice and this permission notice shall be included in
  * all copies or substantial portions of the Software.
  * 
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  * THE SOFTWARE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "sdkconfig.h"
#include "led_strip.h"


/* Private constants ---------------------------------------------------------*/
const char *TAG_APP = "MAIN_APP";


/* Private defines -----------------------------------------------------------*/
#define RGB_LED_PIN             8
#define ESPNOW_QUEUE_SIZE       6
#define ESPNOW_MAXDELAY       512
#define IS_BROADCAST_ADDR(addr) (memcmp(addr, broadcast_mac, ESP_NOW_ETH_ALEN) == 0)


/* Private typedef -----------------------------------------------------------*/
typedef struct 
{
    uint8_t  u8_mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t *pu8_Data;
    size_t   u32_Data_len;

} ESPNOW_RECEIVE_DATA_t;

/*RGB data 0-255 where 255 = 100%*/
typedef struct 
{
    uint8_t u8_Red;
    uint8_t u8_Green; 
    uint8_t u8_Blue;   
    bool    b_led_state;

}RGB_data_t;


/* Private variables ---------------------------------------------------------*/
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static QueueHandle_t s_espnow_queue;
static RGB_data_t OnBoard_RGB_LED;
static led_strip_handle_t led_strip;



/* Private function prototypes -----------------------------------------------*/
static esp_err_t init_sys( void );
static void init_wifi( void );
static void init_espnow( void );
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
static void Configure_RGB_LED(RGB_data_t *p_RGB);
static void Turn_LED_OnOff( bool b_OnOff, RGB_data_t *p_RGB );
static void Toggle_LED( RGB_data_t *p_RGB );


/* Exported functions --------------------------------------------------------*/
void app_main(void)
{
    uint8_t u8_Mac[6];
    ESPNOW_RECEIVE_DATA_t RxData;

    float *pf_Lux;     
    float *pf_Temp_C;  
    float *pf_Humi_PCT;

    ESP_ERROR_CHECK( init_sys( ));
    init_wifi( );
    init_espnow( );
    Configure_RGB_LED( &OnBoard_RGB_LED );    

    while(1)
    {   
        ESP_LOGI(TAG_APP, "Waiting for ESP-Now data..");
       
        ESP_ERROR_CHECK(esp_read_mac(u8_Mac, ESP_MAC_BASE));       
        ESP_LOGI(TAG_APP, "My MAC Addr is: "MACSTR"", MAC2STR(u8_Mac));

        Turn_LED_OnOff( true, &OnBoard_RGB_LED );  

        while (xQueueReceive(s_espnow_queue, &RxData, portMAX_DELAY) == pdTRUE) 
        {
            ESP_LOGW(TAG_APP, "Received data from: "MACSTR", len: %d", MAC2STR(RxData.u8_mac_addr), RxData.u32_Data_len);

            pf_Temp_C   = (float*)(RxData.pu8_Data);
            pf_Humi_PCT = (float*)(RxData.pu8_Data+4);
            pf_Lux      = (float*)(RxData.pu8_Data+8);

            ESP_LOGI(TAG_APP, "Temperature: %.2f", *pf_Temp_C);
            ESP_LOGI(TAG_APP, "Humidity:    %.2f", *pf_Humi_PCT);
            ESP_LOGI(TAG_APP, "Light:       %.2f", *pf_Lux);

            free( RxData.pu8_Data );
            
            Toggle_LED( &OnBoard_RGB_LED );   
            vTaskDelay(pdMS_TO_TICKS(100));

            Toggle_LED( &OnBoard_RGB_LED );
            vTaskDelay(pdMS_TO_TICKS(100));
            
            Toggle_LED( &OnBoard_RGB_LED );
            vTaskDelay(pdMS_TO_TICKS(100));

            Toggle_LED( &OnBoard_RGB_LED );           
            
        }
    }

}


/* Private functions ---------------------------------------------------------*/

/// @brief  Initialize the system. E.g. NVS, Queue for inter-task communication
/// @param  void
/// @return ESP_OK if no error, ESP_FAIL if queue could not be created
static esp_err_t init_sys( void )
{  
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
  

    s_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(ESPNOW_RECEIVE_DATA_t));
    if (s_espnow_queue == NULL) 
    {
        ESP_LOGE(TAG_APP, "Create queue failed!");
        return ESP_FAIL;
    }

    return ESP_OK;    
}


/// @brief  Init wifi in station mode without connection to an AP
/// @param  void
static void init_wifi( void )
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT( );

    ESP_ERROR_CHECK( esp_netif_init( ));
    ESP_ERROR_CHECK( esp_event_loop_create_default( ));
    ESP_ERROR_CHECK( esp_wifi_init( &cfg ));
    ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_RAM ));
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ));
    ESP_ERROR_CHECK( esp_wifi_start( ));
    ESP_ERROR_CHECK( esp_wifi_set_channel( CONFIG_APP_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE ));

#if CONFIG_APP_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
}


/// @brief  Init ESP-NOW and register callback when data has been received
/// @param  void
/// @note   We dont have a send callback because we only want to listen for data
static void init_espnow( void )
{
    ESP_ERROR_CHECK( esp_now_init() );    
    ESP_ERROR_CHECK( esp_now_register_recv_cb(espnow_recv_cb) );
#if CONFIG_APP_ESPNOW_ENABLE_POWER_SAVE
    ESP_ERROR_CHECK( esp_now_set_wake_window(CONFIG_ESPNOW_WAKE_WINDOW) );
    ESP_ERROR_CHECK( esp_wifi_connectionless_module_set_wake_interval(CONFIG_ESPNOW_WAKE_INTERVAL) );
#endif
    /* Set primary master key. */
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_APP_ESPNOW_PMK) );    
}


/// @brief           ESP-NOW data received callback function
/// @param recv_info ESPNOW packet information
/// @param data      Pointer to ESP-NOW data
/// @param len       Length in bytes of received data
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{    
    ESPNOW_RECEIVE_DATA_t RxData;

    uint8_t * mac_addr = recv_info->src_addr;
    uint8_t * des_addr = recv_info->des_addr;
 
    if (mac_addr == NULL || data == NULL || len <= 0) 
    {
        ESP_LOGE(TAG_APP, "Receive cb arg error");
        return;
    }

    if (IS_BROADCAST_ADDR(des_addr)) 
    {
        /* If added a peer with encryption before, the receive packets may be
         * encrypted as peer-to-peer message or unencrypted over the broadcast channel.
         * Users can check the destination address to distinguish it.
         */
        ESP_LOGD(TAG_APP, "Received broadcast ESPNOW data");
    } 
    else 
    {
        ESP_LOGD(TAG_APP, "Received unicast ESPNOW data from: %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);
    }
    
    memcpy( RxData.u8_mac_addr, mac_addr, ESP_NOW_ETH_ALEN );
    RxData.pu8_Data = malloc( len );

    if( RxData.pu8_Data == NULL ) 
    {
        ESP_LOGE(TAG_APP, "Malloc receive data fail");
        return;
    }

    memcpy(RxData.pu8_Data, data, len);
    RxData.u32_Data_len = (size_t)(len);
    if (xQueueSend(s_espnow_queue, &RxData, ESPNOW_MAXDELAY) != pdTRUE) 
    {
        ESP_LOGW(TAG_APP, "Send receive queue fail");
        free( RxData.pu8_Data );
    }    
}


/// @brief       Configure the on-board RGB LED (ESP32-C6-DevKitC-1)
/// @param p_RGB RGB_LED object
/// @note        Color of RGB LED will be set in this function as well (Not pretty but we dont need more right now)
static void Configure_RGB_LED( RGB_data_t *p_RGB )
{
    ESP_LOGI(TAG_APP, "Configure addressable RGB LED");

    p_RGB->u8_Red      = 0;
    p_RGB->u8_Green    = 50;
    p_RGB->u8_Blue     = 0;
    p_RGB->b_led_state = false;

    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = 
    {
        .strip_gpio_num = RGB_LED_PIN,
        .max_leds = 1, // at least one LED on board
    };
    
    led_strip_rmt_config_t rmt_config = 
    {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}


/// @brief         Function to turn RGB LED on or off
/// @param b_OnOff true = turn on, false = turn off
/// @param p_RGB   RGB LED object
static void Turn_LED_OnOff( bool b_OnOff, RGB_data_t *p_RGB )
{
    if( p_RGB == NULL)
        return;

    if( b_OnOff == true)
    {   
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */  
        led_strip_set_pixel(led_strip, 0, (uint32_t)(p_RGB->u8_Red), (uint32_t)(p_RGB->u8_Green), (uint32_t)(p_RGB->u8_Blue));     
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
        p_RGB->b_led_state = true;
    }
    else
    {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
        p_RGB->b_led_state = false;
    }
}


/// @brief       Toggle RGB LED
/// @param p_RGB RGB LED object
static void Toggle_LED( RGB_data_t *p_RGB )
{    
    if (p_RGB->b_led_state == true)             
        Turn_LED_OnOff(false, p_RGB );            
    else    
        Turn_LED_OnOff(true, p_RGB );    
}