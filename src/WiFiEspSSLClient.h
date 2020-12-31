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

#ifndef WiFiEspSSKClient_h
#define WiFiEspSSLClient_h


#include "Arduino.h"
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"



class WiFiEspSSLClient : public Client
{
public:
  WiFiEspSSLClient();
  WiFiEspSSLClient(uint8_t sock);
  
  
  // override Print.print method
  
  size_t print(const __FlashStringHelper *ifsh);
  size_t println(const __FlashStringHelper *ifsh);

  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual int connect(const char* host, uint16_t port, const char* client_cert, const char* client_key);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual uint8_t status();
  virtual operator bool();
  virtual void setCertificate(const char *client_ca);
  virtual void setPrivateKey(const char *private_key);
  
  // needed to correctly handle overriding
  // see http://stackoverflow.com/questions/888235/overriding-a-bases-overloaded-function-in-c
  using Print::write;
  using Print::print;
  using Print::println;

  virtual IPAddress remoteIP();
  virtual uint16_t remotePort();
  

  friend class WiFiEspServer;

private:
  uint8_t _sock;     // connection id
  int connect(const char* host, uint16_t port, uint8_t protMode);
  size_t printFSH(const __FlashStringHelper *ifsh, bool appendCrLf);

};

#endif
