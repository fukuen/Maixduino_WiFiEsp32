/*--------------------------------------------------------------------
This file is part of the Arduino WiFiEsp library.

The Arduino WiFiEsp library is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

The Arduino WiFiEsp library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with The Arduino WiFiEsp library.  If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------*/

#include "WiFiEsp32.h"


int16_t 	WiFiEspClass::_state[MAX_SOCK_NUM] = { NA_STATE, NA_STATE, NA_STATE, NA_STATE };
uint16_t 	WiFiEspClass::_server_port[MAX_SOCK_NUM] = { 0, 0, 0, 0 };


uint8_t WiFiEspClass::espMode = 0;

esp32_spi_aps_list_t *aps_list;


WiFiEspClass::WiFiEspClass()
{

}

//void WiFiEspClass::init(Stream* espSerial)
void WiFiEspClass::init()
{
    LOGINFO(F("Initializing ESP module"));
//	EspDrv::wifiDriverInit(espSerial);
    fpioa_set_function(25, FUNC_GPIOHS10);
    fpioa_set_function(8, FUNC_GPIOHS11);
    fpioa_set_function(9, FUNC_GPIOHS12);
    fpioa_set_function(28, FUNC_GPIOHS13);
    fpioa_set_function(26, FUNC_GPIOHS14);
    fpioa_set_function(27, FUNC_GPIOHS15);

    esp32_spi_config_io(10, 11, 12, 13, 14, 15);
    esp32_spi_init();
}



char* WiFiEspClass::firmwareVersion()
{
//	return EspDrv::getFwVersion();
    char version[32];
    return esp32_spi_firmware_version(version);
}


int WiFiEspClass::begin(const char* ssid, const char* passphrase)
{
    espMode = 1;
//	if (EspDrv::wifiConnect(ssid, passphrase))
	if (esp32_spi_connect_AP((uint8_t *)ssid, (uint8_t *)passphrase, 5) == 0)
		return WL_CONNECTED;

	return WL_CONNECT_FAILED;
}


int WiFiEspClass::beginAP(const char* ssid, uint8_t channel, const char* pwd, uint8_t enc, bool apOnly)
{
	if(apOnly)
        espMode = 2;
    else
        espMode = 3;
    
//    if (EspDrv::wifiStartAP(ssid, pwd, channel, enc, espMode))
    if (esp32_spi_ap_pass_phrase((uint8_t *)ssid, (uint8_t *)pwd, channel) == 0)
		return WL_CONNECTED;

	return WL_CONNECT_FAILED;
}

int WiFiEspClass::beginAP(const char* ssid)
{
	return beginAP(ssid, 10, "", 0);
}

int WiFiEspClass::beginAP(const char* ssid, uint8_t channel)
{
	return beginAP(ssid, channel, "", 0);
}


void WiFiEspClass::config(IPAddress ip)
{
//	EspDrv::config(ip);
	uint8_t _ip[4];
	_ip[0] = ip[0];
	_ip[1] = ip[1];
	_ip[2] = ip[2];
	_ip[3] = ip[3];
	esp32_spi_ip_address(_ip);
}

void WiFiEspClass::configAP(IPAddress ip)
{
//	EspDrv::configAP(ip);
	config(ip);
}



int WiFiEspClass::disconnect()
{
//    return EspDrv::disconnect();
	return esp32_spi_disconnect_from_AP();
}

uint8_t* WiFiEspClass::macAddress(uint8_t* mac)
{
	// TODO we don't need _mac variable
//	uint8_t* _mac = EspDrv::getMacAddress();
	uint8_t* _mac = esp32_spi_MAC_address();
	memcpy(mac, _mac, WL_MAC_ADDR_LENGTH);
    return mac;
}

IPAddress WiFiEspClass::localIP()
{
	IPAddress ret;
//	if(espMode==1)
//		EspDrv::getIpAddress(ret);
//	else
//		EspDrv::getIpAddressAP(ret);
	esp32_spi_net_t *net = esp32_spi_get_network_data();
	ret = net->localIp;
	return ret;
}

IPAddress WiFiEspClass::subnetMask()
{
	IPAddress mask;
//	if(espMode==1)
//    EspDrv::getNetmask(mask);
	esp32_spi_net_t *net = esp32_spi_get_network_data();
	mask = net->subnetMask;
	return mask;
}

IPAddress WiFiEspClass::gatewayIP()
{
	IPAddress gw;
//	if(espMode==1)
//		EspDrv::getGateway(gw);
	esp32_spi_net_t *net = esp32_spi_get_network_data();
	gw = net->gatewayIp;
	return gw;
}


char* WiFiEspClass::SSID()
{
//    return EspDrv::getCurrentSSID();
	return esp32_spi_get_ssid();
}

uint8_t* WiFiEspClass::BSSID(uint8_t* bssid)
{
	// TODO we don't need _bssid
//	uint8_t* _bssid = EspDrv::getCurrentBSSID();
	uint8_t _bssid[6] = { 0, 0, 0, 0, 0, 0 };
	memcpy(bssid, _bssid, WL_MAC_ADDR_LENGTH);
    return bssid;
}

int32_t WiFiEspClass::RSSI()
{
//    return EspDrv::getCurrentRSSI();
	return esp32_spi_get_rssi();
}


int8_t WiFiEspClass::scanNetworks()
{
//	return EspDrv::getScanNetworks();
	aps_list = esp32_spi_scan_networks();
	return aps_list->aps_num;
}

char* WiFiEspClass::SSID(uint8_t networkItem)
{
//	return EspDrv::getSSIDNetoworks(networkItem);
	esp32_spi_ap_t **aps = aps_list[networkItem].aps;
	return (char *)(*aps)->ssid;
}

int32_t WiFiEspClass::RSSI(uint8_t networkItem)
{
//	return EspDrv::getRSSINetoworks(networkItem);
	esp32_spi_ap_t **aps = aps_list[networkItem].aps;
	return (*aps)->rssi;
}

uint8_t WiFiEspClass::encryptionType(uint8_t networkItem)
{
//    return EspDrv::getEncTypeNetowrks(networkItem);
	esp32_spi_ap_t **aps = aps_list[networkItem].aps;
	return (*aps)->encr;
}


uint8_t WiFiEspClass::status()
{
//	return EspDrv::getConnectionStatus();
	return esp32_spi_status();
}



////////////////////////////////////////////////////////////////////////////
// Non standard methods
////////////////////////////////////////////////////////////////////////////

void WiFiEspClass::reset(void)
{
//	EspDrv::reset();
    esp32_spi_init();
}


/*
void ESP8266::hardReset(void)
{
connected = false;
strcpy(ip, "");
digitalWrite(ESP8266_RST, LOW);
delay(ESP8266_HARD_RESET_DURATION);
digitalWrite(ESP8266_RST, HIGH);
delay(ESP8266_HARD_RESET_DURATION);
}
*/


bool WiFiEspClass::ping(const char *host)
{
//	return EspDrv::ping(host);
	return (esp32_spi_ping((uint8_t *)host, 1, 1) != -1);
}

uint8_t WiFiEspClass::getFreeSocket()
{
  // ESP Module assigns socket numbers in ascending order, so we will assign them in descending order
    for (int i = MAX_SOCK_NUM - 1; i >= 0; i--)
	{
      if (_state[i] == NA_STATE)
      {
          return i;
      }
    }
    return SOCK_NOT_AVAIL;
}

void WiFiEspClass::allocateSocket(uint8_t sock)
{
  _state[sock] = sock;
}

void WiFiEspClass::releaseSocket(uint8_t sock)
{
  _state[sock] = NA_STATE;
}


WiFiEspClass WiFi;
