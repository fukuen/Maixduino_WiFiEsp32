/*--------------------------------------------------------------------
Copyright 2020 fukuen

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

#include <inttypes.h>

#include "WiFiEsp32.h"

#include "utility/debug.h"


WiFiEspSSLClient::WiFiEspSSLClient() : _sock(255)
{
}

WiFiEspSSLClient::WiFiEspSSLClient(uint8_t sock) : _sock(sock)
{
}


////////////////////////////////////////////////////////////////////////////////
// Overrided Print methods
////////////////////////////////////////////////////////////////////////////////

// the standard print method will call write for each character in the buffer
// this is very slow on ESP
size_t WiFiEspSSLClient::print(const __FlashStringHelper *ifsh)
{
	printFSH(ifsh, false);
}

// if we do override this, the standard println will call the print
// method twice
size_t WiFiEspSSLClient::println(const __FlashStringHelper *ifsh)
{
	printFSH(ifsh, true);
}


////////////////////////////////////////////////////////////////////////////////
// Implementation of Client virtual methods
////////////////////////////////////////////////////////////////////////////////

int WiFiEspSSLClient::connect(const char* host, uint16_t port)
{
    return connect(host, port, TLS_MODE);
}

int WiFiEspSSLClient::connect(const char* host, uint16_t port, const char* client_cert, const char* client_key)
{
	esp32_set_certificate((char *)client_cert);
	esp32_set_private_key((char *)client_key);

    return connect(host, port, TLS_MODE);
}

int WiFiEspSSLClient::connect(IPAddress ip, uint16_t port)
{
	char s[16];
	sprintf_P(s, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);

	return connect(s, port, TLS_MODE);
}


/* Private method */
int WiFiEspSSLClient::connect(const char* host, uint16_t port, uint8_t protMode)
{
	LOGINFO1(F("Connecting to"), host);

	_sock = WiFiEspClass::getFreeSocket();

    if (_sock != NO_SOCKET_AVAIL)
    {
		if (esp32_spi_socket_connect(_sock, (uint8_t *)host, 1, port, (esp32_socket_mode_enum_t)protMode))
			return 0;

    	WiFiEspClass::allocateSocket(_sock);
    }
	else
	{
    	LOGERROR(F("No socket available"));
    	return 0;
    }
    return 1;
}



size_t WiFiEspSSLClient::write(uint8_t b)
{
	  return write(&b, 1);
}

size_t WiFiEspSSLClient::write(const uint8_t *buf, size_t size)
{
	if (_sock >= MAX_SOCK_NUM or size==0)
	{
		setWriteError();
		return 0;
	}

	uint32_t r = esp32_spi_socket_write(_sock, (uint8_t *)buf, size);
	if (!r)
	{
		setWriteError();
		LOGERROR1(F("Failed to write to socket"), _sock);
		delay(4000);
		stop();
		return 0;
	}

	return size;
}



int WiFiEspSSLClient::available()
{
	if (_sock != 255)
	{
		int bytes = esp32_spi_socket_available(_sock);
		if (bytes>0)
		{
			return bytes;
		}
	}

	return 0;
}

int WiFiEspSSLClient::read()
{
	uint8_t b;
	if (!available())
		return -1;

	bool connClose = false;
	b = esp32_spi_get_data(_sock);

	if (connClose)
	{
		WiFiEspClass::releaseSocket(_sock);
		_sock = 255;
	}

	return b;
}

int WiFiEspSSLClient::read(uint8_t* buf, size_t size)
{
	if (!available())
		return -1;
	return esp32_spi_socket_read(_sock, buf, size);
}

int WiFiEspSSLClient::peek()
{
	uint8_t b;
	if (!available())
		return -1;

	b = esp32_spi_get_data(_sock);

	return b;
}


void WiFiEspSSLClient::flush()
{
	while (available())
		read();
}



void WiFiEspSSLClient::stop()
{
	if (_sock == 255)
		return;

	LOGINFO1(F("Disconnecting "), _sock);

	esp32_spi_socket_close(_sock);

	WiFiEspClass::releaseSocket(_sock);
	_sock = 255;
}


uint8_t WiFiEspSSLClient::connected()
{
	return (status() == SOCKET_ESTABLISHED);
}


WiFiEspSSLClient::operator bool()
{
  return _sock != 255;
}


void WiFiEspSSLClient::setCertificate(const char *client_ca)
{
	esp32_set_certificate((char *)client_ca);
}


void WiFiEspSSLClient::setPrivateKey(const char *private_key)
{
	esp32_set_private_key((char *)private_key);
}


////////////////////////////////////////////////////////////////////////////////
// Additional WiFi standard methods
////////////////////////////////////////////////////////////////////////////////


uint8_t WiFiEspSSLClient::status()
{
	if (_sock == 255)
	{
//	LOGINFO1(F("SOCKET_CLOSED 1! "), _sock);
		return SOCKET_CLOSED;
	}

	if (esp32_spi_socket_available(_sock) == 0)
	{
//	LOGINFO1(F("SOCKET_ESTABLISHED 1! "), _sock);
		return SOCKET_ESTABLISHED;
	}

	if (esp32_spi_socket_status(_sock) == SOCKET_ESTABLISHED)
	{
//	LOGINFO1(F("SOCKET_ESTABLISHED 2! "), _sock);
		return SOCKET_ESTABLISHED;
	}

//	LOGINFO1(F("SOCKET_CLOSED 2! "), _sock);
	WiFiEspClass::releaseSocket(_sock);
	_sock = 255;

	return SOCKET_CLOSED;
}

IPAddress WiFiEspSSLClient::remoteIP()
{
	IPAddress ret;
	uint8_t ip[4];
	uint16_t port = 0;
	uint16_t *p = &port;
	esp32_spi_get_remote_info(_sock, ip, p);
	ret = ip;
	return ret;
}

uint16_t WiFiEspSSLClient::remotePort()
{
	uint8_t ip[4];
	uint16_t port = 0;
	uint16_t *p = &port;
	esp32_spi_get_remote_info(_sock, ip, p);
	return port;
}

////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////

size_t WiFiEspSSLClient::printFSH(const __FlashStringHelper *ifsh, bool appendCrLf)
{
	size_t size = strlen_P((char*)ifsh);
	
	if (_sock >= MAX_SOCK_NUM or size==0)
	{
		setWriteError();
		return 0;
	}

	uint32_t r = esp32_spi_socket_write(_sock, (uint8_t *)ifsh, size);
	if (!r)
	{
		setWriteError();
		LOGERROR1(F("Failed to write to socket"), _sock);
		delay(4000);
		stop();
		return 0;
	}

	return size;
}
