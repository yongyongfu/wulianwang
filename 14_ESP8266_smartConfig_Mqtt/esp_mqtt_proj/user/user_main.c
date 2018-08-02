#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "hal_key.h"
#include "sntp.h"
typedef unsigned   char    u8_t;
//MQTT客户端
MQTT_Client mqttClient;
typedef unsigned long u32_t;
static ETSTimer sntp_timer;

//检查wifi是否已经连接的定时器
os_timer_t checkTimer_wifistate;
static ETSTimer key_timer;
static ETSTimer wendu_timer;
static ETSTimer working_timer;
u8_t bz=0,key=0;

//按键定义
#define GPIO_KEY_NUM                            1
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_GPIO2_U
#define KEY_0_IO_NUM                            2
#define KEY_0_IO_FUNC                           FUNC_GPIO2
LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];
LOCAL keys_typedef_t keys;

void sntpfn()
{
    u32_t ts = 0;
    ts = sntp_get_current_timestamp();
    os_printf("current time : %s\n", sntp_get_real_time(ts));
    if (ts == 0) {
        //os_printf("did not get a valid time from sntp server\n");
    } else {
            os_timer_disarm(&sntp_timer);
            MQTT_Connect(&mqttClient);
    }
}
#include <string.h>
static char  senddata[100];
#define CMD_SIZE 	100
char userqrcode[32]="ABCDEFG1234567890";
int temperature=1.4,humidity=1.8,light=1,pressure=2.8;
int randval(int low, int high)
{
	int val;
   val = ((int)(rand()/(RAND_MAX-0.0))*(high - low)) + low;
   return(val);
}

void mqttConnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    INFO("MQTT: Connected\r\n");
    MQTT_Subscribe(client, "in000000000001", 0);

/*
    	strcpy(senddata,"{\n\"method\":\"verifyStaff\",\n\"staffQRcode\":\"");
    	strcat(senddata,userqrcode);
    	strcat(senddata,"\",\n\"temperature\":\"");
    	strcat(senddata,temperature);
    	strcat(senddata,"\",\n\"humidity\":\"");
    	strcat(senddata,humidity);
    	/////
    	strcat(senddata,"\",\n\"light\":");
    	strcat(senddata,light);
    	strcat(senddata,",\n\"pressure\":");
    	strcat(senddata,pressure);
*/
  	//获取传感器测量数据，该示例使用随机数
  	//将数据合成为JSON格式数据
 //   memset(senddata,0x0,200);
	os_sprintf(senddata,"{\"temperature\":%d,\"humidity\":%d,\"light\":%d,\"pressure\":%d}",temperature,humidity,light,pressure);// {"temperature":1.4,"humidity":3.9,"light":1,"pressure":12}
	os_printf("%s",senddata);
	 MQTT_Publish(client, "out000000000001",senddata, 100, 0, 0);


}
void wifiConnectCb(uint8_t status) {
	if (status == STATION_GOT_IP) {
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}


void mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
    char *topicBuf = (char*)os_zalloc(topic_len+1),
            *dataBuf = (char*)os_zalloc(data_len+1);

    MQTT_Client* client = (MQTT_Client*)args;

    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);

    /*
    Receive topic: in000000000001, data: {"led":"green","value":1}
    TCP: data received 44 bytes
    Receive topic: in000000000001, data: {"led":"yellow","value":1}
    TCP: data received 42 bytes
    Receive topic: in000000000001, data: {"led":"blue","value":1}
    TCP: data received 41 bytes
    Receive topic: in000000000001, data: {"led":"red","value":1}
    TCP: data received 41 bytes
    Receive topic: in000000000001, data: {"led":"red","value":0}
    TCP: data received 43 bytes
    Receive topic: in000000000001, data: {"led":"green","value":0}
    TCP: data received 44 bytes
    Receive topic: in000000000001, data: {"led":"yellow","value":0}
    TCP: data received 42 bytes
    Receive topic: in000000000001, data: {"led":"blue","value":0}
    */
  //  if (os_strncmp(strstr, "page", 4) == 0)
    if ( (char *)os_strstr(dataBuf, "{\"led\":\"green\",\"value\":1}")!=0)
    {
       GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);           //GPIO12为高
    }
    if ( (char *)os_strstr(dataBuf, "{\"led\":\"green\",\"value\":0}")!=0)
    {
       GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);           //GPIO12为高
    }

    if ( (char *)os_strstr(dataBuf, "{\"led\":\"yellow\",\"value\":1}")!=0)
    {
         GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);           //GPIO12为高
    }
    if ( (char *)os_strstr(dataBuf, "{\"led\":\"yellow\",\"value\":0}")!=0)
    {
         GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);           //GPIO12为高
    }

    if ( (char *)os_strstr(dataBuf, "{\"led\":\"red\",\"value\":1}")!=0)
    {
           GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);           //GPIO12为高
    }
    if ( (char *)os_strstr(dataBuf, "{\"led\":\"red\",\"value\":0}")!=0)
    {
           GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0);           //GPIO12为高
    }
    if ( (char *)os_strstr(dataBuf, "{\"led\":\"blue\",\"value\":1}")!=0)
     {
            GPIO_OUTPUT_SET(GPIO_ID_PIN(16), 1);           //GPIO12为高
     }
    if ( (char *)os_strstr(dataBuf, "{\"led\":\"blue\",\"value\":0}")!=0)
     {
            GPIO_OUTPUT_SET(GPIO_ID_PIN(16), 0);           //GPIO12为高
     }
	 MQTT_Publish(client, "/LED/out", "LED status is open ...",strlen("LED status is open ..."), 0, 0);
    os_free(topicBuf);
    os_free(dataBuf);
}

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
	enum flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map) {
	case FLASH_SIZE_4M_MAP_256_256:
		rf_cal_sec = 128 - 5;
		break;

	case FLASH_SIZE_8M_MAP_512_512:
		rf_cal_sec = 256 - 5;
		break;

	case FLASH_SIZE_16M_MAP_512_512:
	case FLASH_SIZE_16M_MAP_1024_1024:
		rf_cal_sec = 512 - 5;
		break;

	case FLASH_SIZE_32M_MAP_512_512:
	case FLASH_SIZE_32M_MAP_1024_1024:
		rf_cal_sec = 1024 - 5;
		break;

	case FLASH_SIZE_64M_MAP_1024_1024:
		rf_cal_sec = 2048 - 5;
		break;
	case FLASH_SIZE_128M_MAP_1024_1024:
		rf_cal_sec = 4096 - 5;
		break;
	default:
		rf_cal_sec = 0;
		break;
	}

	return rf_cal_sec;
}
void ICACHE_FLASH_ATTR revUartData(uint8 * pData,int len)

{
	MQTT_Publish(&mqttClient, "out000000000001", pData, len, 0, 0);
}
//char wendushidu [50]= {"temperature":1.4,"humidity":3.9,"light":1,"pressure":12}

void Check_WifiState(void) {
	uint8 getState;
	struct ip_info ipConfig;
	wifi_get_ip_info(STATION_IF, &ipConfig);
	getState = wifi_station_get_connect_status();

	//查询 ESP8266 WiFi station 接口连接 AP 的状态
	if (getState == STATION_GOT_IP && ipConfig.ip.addr != 0) {

		os_printf("----- wifi 连接成功！ 红灯关闭啦啦！---\r\n");
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);

		os_printf("----- wifi 连接成功！ 断开定时器啦！---\r\n");
		os_timer_disarm(&checkTimer_wifistate);

		sntp_setservername(0, "pool.ntp.org"); // set sntp server after got ip address
		sntp_init();
		os_timer_disarm(&sntp_timer);
		os_timer_setfn(&sntp_timer, (os_timer_func_t *) sntpfn, NULL);
		os_timer_arm(&sntp_timer, 1000, 1); //1s

	}
}

LOCAL void ICACHE_FLASH_ATTR keyLongPress(void) {

}

LOCAL void ICACHE_FLASH_ATTR keyShortPress(void) {
	os_printf("---------- 按键触发 ，开始进去SmartConfig配网 \n\n\n-----");
	smartconfig_init();
}

//按键初始化
LOCAL void ICACHE_FLASH_ATTR keyInit(void) {
	singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
			keyLongPress, keyShortPress);
	keys.singleKey = singleKey;
	keyParaInit(&keys);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "gpio.h"  //端口控制需要的头文件

void delay_ms(uint16 x)
{
    for(;x>0;x--)
    {
      os_delay_us(1000);
    }
}

void  led_init()//初始化
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);//选择GPIO12
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);//GPIO14为高

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);//选择GPIO13
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);//GPIO14为高

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);//选择GPIO14
    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0);//GPIO14为高

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);//选择GPIO14
    GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);//GPIO14为高

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);//选择GPIO2
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(2)) ; // 设置GPIO2为输入
}
uint8 workcon=0;
void working(void)
{
    workcon++;
//system_soft_wdt_feed();//这里我们喂下看门狗  ，不让看门狗复位
    if(workcon%2)
    {
		 GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);           //GPIO14为高
    }
    else
    {
         GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);           //GPIO14为高
    }
}

void lenkey(void)
{
	char GPIO_PIN=0;
	switch(bz)
	{
		case 0:if(GPIO_INPUT_GET(GPIO_ID_PIN(2))==0x00) bz=1;
			break;
		case 1:if(GPIO_INPUT_GET(GPIO_ID_PIN(2))==0x00)
		       {
			     delay_ms(20); //延时20MS，去抖
			     if(GPIO_INPUT_GET(GPIO_ID_PIN(2))==0x00)
			       {
						 key=~key;
						 bz=2;
						 GPIO_PIN=2;
					//     memset(senddata,0x0,200);
						 os_sprintf(senddata,"{\"GPIO_INPUT\":%d}",GPIO_PIN);
						 os_printf("%s",senddata);
						 MQTT_Publish(&mqttClient, "out000000000001", senddata, 20, 0, 0);
					//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);           //GPIO12为高
					//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);           //GPIO13为高
					//	 GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);           //GPIO14为高
			       }
		       }
		       else
		       {
		    	         bz=0;

		       }
			break;
		case 2:if(GPIO_INPUT_GET(GPIO_ID_PIN(2))!=0x00) bz=0;
			break;
	}
	if(key)
	   {
			GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);           //GPIO12Îª¸ß
			GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);           //GPIO13Îª¸ß
			GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0);           //GPIO14Îª¸ß
		}
	else{
		    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);           //GPIO12为高
			GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);           //GPIO13为高
			GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);           //GPIO14为高
		}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *
 * 读取温湿度数据，然后发送到MQTT服务器上去
 *
*/

#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "mem.h"
#include "user_interface.h"
#include "c_types.h"

void wendu(void)
{
	temperature=rand()%40;
	humidity=rand()%90;
	light=rand()%5000;
	pressure=rand()%6000;
//	memset(senddata,0x0,200);
	os_sprintf(senddata,"{\"temperature\":%d,\"humidity\":%d,\"light\":%d,\"pressure\":%d}",temperature,humidity,light,pressure);
    MQTT_Publish(&mqttClient, "out000000000001",senddata, 100, 0, 0);
}

void user_init(void) {

	uart_init(115200, 115200);

	os_delay_us(60000);

	os_printf("SDK version:%s\n", system_get_sdk_version());

	keyInit();

	CFG_Load();
	//开始MQTT连接
    led_init();
	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port,sysCfg.security);
	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user,sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
  //  WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
	os_timer_disarm(&checkTimer_wifistate);  //启动定时器前先取消定时器定时
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState,
	NULL); //设置定时器回调函数
	os_timer_arm(&checkTimer_wifistate, 1000, 1); //启动定时器

	INFO("\r\nSystem started ...\r\n");
	os_timer_disarm(&wendu_timer);
    os_timer_setfn(&wendu_timer, (os_timer_func_t *)wendu, NULL);
    os_timer_arm(&wendu_timer, 10000, 1);//1ms
	
	os_timer_disarm(&working_timer);
    os_timer_setfn(&working_timer, (os_timer_func_t *)working, NULL);
    os_timer_arm(&working_timer, 2000, 1);//1ms
}
