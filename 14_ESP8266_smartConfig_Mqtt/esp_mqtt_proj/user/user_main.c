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

//MQTT�ͻ���
MQTT_Client mqttClient;
typedef unsigned long u32_t;
static ETSTimer sntp_timer;

//���wifi�Ƿ��Ѿ����ӵĶ�ʱ��
os_timer_t checkTimer_wifistate;

//��������
#define GPIO_KEY_NUM                            1
#define KEY_0_IO_MUX                            PERIPHS_IO_MUX_GPIO2_U
#define KEY_0_IO_NUM                            2
#define KEY_0_IO_FUNC                           FUNC_GPIO2
LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];
LOCAL keys_typedef_t keys;

void sntpfn() {
	u32_t ts = 0;
	ts = sntp_get_current_timestamp();
	os_printf("current time : %s\n", sntp_get_real_time(ts));
	if (ts != 0) {
		os_timer_disarm(&sntp_timer);
		MQTT_Connect(&mqttClient);
	}
}

void wifiConnectCb(uint8_t status) {
	if (status == STATION_GOT_IP) {
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

void mqttConnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	MQTT_Subscribe(client, "/xuhong/LED/in", 0); //��������/xuhong/LED/in
}

void mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*) args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len,
		const char *data, uint32_t data_len) {

	MQTT_Client* client = (MQTT_Client*) args;

	//������յ�ָ����1��GPIO15���Ϊ��,Ҳ����LED���ƣ�ͬʱ������Ϣ��������/xuhong/LED/out����Ϣ��LED status is open ...
	if (data[0] == '1') {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 0);
		MQTT_Publish(client, "/xuhong/LED/out", "LED status is open ...",
				strlen("LED status is open ..."), 0, 0);
	}

	//������յ�ָ����0��GPIO15Ϊ��,Ҳ����LED�صƣ�ͬʱ������Ϣ��������/xuhong/LED/out����Ϣ��LED status is off ...
	if (data[0] == '0') {
		GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1);
		MQTT_Publish(client, "/xuhong/LED/out", "LED status is off ...",
				strlen("LED status is off ..."), 0, 0);
	}

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

void Check_WifiState(void) {
	uint8 getState;
	struct ip_info ipConfig;
	wifi_get_ip_info(STATION_IF, &ipConfig);
	getState = wifi_station_get_connect_status();

	//��ѯ ESP8266 WiFi station �ӿ����� AP ��״̬
	if (getState == STATION_GOT_IP && ipConfig.ip.addr != 0) {

		os_printf("----- wifi ���ӳɹ��� ��ƹر�������---\r\n");
		GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);

		os_printf("----- wifi ���ӳɹ��� �Ͽ���ʱ������---\r\n");
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
	os_printf("---------- �������� ����ʼ��ȥSmartConfig���� \n\n\n-----");
	smartconfig_init();
}

//������ʼ��
LOCAL void ICACHE_FLASH_ATTR keyInit(void) {
	singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
			keyLongPress, keyShortPress);
	keys.singleKey = singleKey;
	keyParaInit(&keys);
}

void user_init(void) {

	uart_init(115200, 115200);

	os_delay_us(60000);

	os_printf("SDK version:%s\n", system_get_sdk_version());

	//LED��ʼ��
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);//ѡ��GPIO15����GPIO�������̵�
	GPIO_OUTPUT_SET(GPIO_ID_PIN(15), 1); //Ĭ��GPIO15Ϊ��,Ҳ���ǹص�

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); //ѡ��GPIO12����GPIO�����Ǻ��
	GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0); //Ĭ��GPIO12Ϊ��,Ҳ���ǿ��ƣ���ʾ�������ɹ���


	keyInit();

	CFG_Load();
	//��ʼMQTT����
	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port,
			sysCfg.security);
	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user,
			sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
  //  WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
	os_timer_disarm(&checkTimer_wifistate); //������ʱ��ǰ��ȡ����ʱ����ʱ
	os_timer_setfn(&checkTimer_wifistate, (os_timer_func_t *) Check_WifiState,
	NULL); //���ö�ʱ���ص�����
	os_timer_arm(&checkTimer_wifistate, 1000, 1); //������ʱ��

	INFO("\r\nSystem started ...\r\n");
}
