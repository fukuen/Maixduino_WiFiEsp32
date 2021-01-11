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
#include "WiFiEspUdp.h"

#include "utility/debug.h"

/* Constructor */
WiFiEspUDP::WiFiEspUDP() : _sock(NO_SOCKET_AVAIL) {}




/* Start WiFiUDP socket, listening at local port PORT */

uint8_t WiFiEspUDP::begin(uint16_t port)
{
    uint8_t sock = WiFiEspClass::getFreeSocket();
    if (sock != NO_SOCKET_AVAIL)
    {
		esp32_spi_socket_open(sock, (uint8_t *)"0", 1, port, UDP_MODE);
		
        WiFiEspClass::allocateSocket(sock);  // allocating the socket for the listener
        WiFiEspClass::_server_port[sock] = port;
        _sock = sock;
        _port = port;
        return 1;
    }
    return 0;

}


/* return number of bytes available in the current packet,
   will return zero if parsePacket hasn't been called yet */
int WiFiEspUDP::available()
{
	 if (_sock != NO_SOCKET_AVAIL)
	 {
		int bytes = esp32_spi_socket_available(_sock);
		if (bytes>0)
		{
			return bytes;
		}
	}

	return 0;
}

/* Release any resources being used by this WiFiUDP instance */
void WiFiEspUDP::stop()
{
	  if (_sock == NO_SOCKET_AVAIL)
	    return;

      // Discard data that might be in the incoming buffer
      flush();
      
      // Stop the listener and return the socket to the pool
	  esp32_spi_socket_close(_sock);
      WiFiEspClass::_state[_sock] = NA_STATE;
      WiFiEspClass::_server_port[_sock] = 0;

	  _sock = NO_SOCKET_AVAIL;
}

int WiFiEspUDP::beginPacket(const char *host, uint16_t port)
{
  if (_sock == NO_SOCKET_AVAIL)
	  _sock = WiFiEspClass::getFreeSocket();
  if (_sock != NO_SOCKET_AVAIL)
  {
	  esp32_spi_socket_connect(_sock, (uint8_t *)host, 1, port, UDP_MODE);
	  _remotePort = port;
	  strcpy(_remoteHost, host);
	  WiFiEspClass::allocateSocket(_sock);
	  return 1;
  }
  return 0;
}


int WiFiEspUDP::beginPacket(IPAddress ip, uint16_t port)
{
//	char s[18];
//	sprintf_P(s, PSTR("%d.%d.%d.%d"), ip[0], ip[1], ip[2], ip[3]);
//
//	return beginPacket(s, port);
  if (_sock == NO_SOCKET_AVAIL)
	  _sock = WiFiEspClass::getFreeSocket();
  if (_sock != NO_SOCKET_AVAIL)
  {
	  uint8_t _ip[4];
	  _ip[0] = ip[0];
	  _ip[1] = ip[1];
	  _ip[2] = ip[2];
	  _ip[3] = ip[3];
	  esp32_spi_socket_connect(_sock, _ip, 0, port, UDP_MODE);
	  _remotePort = port;
	  memcpy(_remoteHost, _ip, 4);
	  WiFiEspClass::allocateSocket(_sock);
	  return 1;
  }
  return 0;
}


int WiFiEspUDP::endPacket()
{
	return 1; //ServerDrv::sendUdpData(_sock);
}

size_t WiFiEspUDP::write(uint8_t byte)
{
  return write(&byte, 1);
}

size_t WiFiEspUDP::write(const uint8_t *buffer, size_t size)
{
	int8_t r = esp32_spi_add_udp_data(_sock, (uint8_t *)buffer, size);
	r = esp32_spi_send_udp_data(_sock);
	if (!r)
	{
		return 0;
	}

	return size;
}

int WiFiEspUDP::parsePacket()
{
	return available();
}

int WiFiEspUDP::read()
{
	uint8_t b;
	if (!available())
		return -1;

    // Read the data and handle the timeout condition
	b = esp32_spi_get_data(_sock);

	return b;
}

int WiFiEspUDP::read(uint8_t* buf, size_t size)
{
	if (!available())
		return -1;
	return esp32_spi_socket_read(_sock, buf, size);
}

int WiFiEspUDP::peek()
{
  uint8_t b;
  if (!available())
    return -1;

  return b;
}

void WiFiEspUDP::flush()
{
	  // Discard all input data
	  int count = available();
	  while (count-- > 0)
	    read();
}


IPAddress  WiFiEspUDP::remoteIP()
{
	IPAddress ret;
	uint8_t ip[4];
	uint16_t port = 0;
	uint16_t *p = &port;
	esp32_spi_get_remote_info(_sock, ip, p);
	ret = ip;
	return ret;
}

uint16_t  WiFiEspUDP::remotePort()
{
	uint8_t ip[4];
	uint16_t port = 0;
	uint16_t *p = &port;
	esp32_spi_get_remote_info(_sock, ip, p);
	return port;
}

uint8_t WiFiEspUDP::beginMulticast(IPAddress ip, uint16_t port)
{
  if (_sock == NO_SOCKET_AVAIL)
	  _sock = WiFiEspClass::getFreeSocket();
  if (_sock != NO_SOCKET_AVAIL)
  {
	  uint8_t _ip[4];
	  _ip[0] = ip[0];
	  _ip[1] = ip[1];
	  _ip[2] = ip[2];
	  _ip[3] = ip[3];
	  esp32_spi_start_server(_sock, _ip, 0, port, UDP_MODE_2);
	  _remotePort = port;
	  memcpy(_remoteHost, _ip, 4);
	  WiFiEspClass::allocateSocket(_sock);
	  return 1;
  }
  return 0;
}



////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////


