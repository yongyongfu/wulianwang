#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__

#define CFG_HOLDER	0x00FF55A4	/* Change this value to load default configurations */
#define CFG_LOCATION	0x79	/* Please don't change or if you know what you doing */
#define MQTT_SSL_ENABLE

/*DEFAULT CONFIGURATIONS*/

#define MQTT_HOST           "120.79.165.74" // "192.168.1.1" the IP address or domain name of your MQTT server or MQTT broker ,such as "mqtt.yourdomain.com"
#define MQTT_PORT           1883    // the listening port of your MQTT server or MQTT broker
#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		120	 /*second*/
#define MQTT_CLIENT_ID        "000001"    // the ID of yourself, any string is OK,client would use this ID register itself to MQTT server
#define MQTT_USER            "zerone" // your MQTT login name, if MQTT server allow anonymous login,any string is OK, otherwise, please input valid login name which you had registered
#define MQTT_PASS            "\0" // you MQTT login password, same as above
//#define STA_SSID "UDOT005"    // your AP/router SSID to config your device networking
//#define STA_PASS "udot12345678" // your AP/router password

#define STA_SSID "DVES_HOME"
#define STA_PASS "yourpassword"
#define STA_TYPE AUTH_WPA2_PSK

#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/

#define DEFAULT_SECURITY	0
#define QUEUE_BUFFER_SIZE		 		2048

// PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
#define PROTOCOL_NAMEv311			/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#endif // __MQTT_CONFIG_H__
