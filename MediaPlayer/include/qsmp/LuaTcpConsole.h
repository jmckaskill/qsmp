/******************************************************************************
 * Copyright (C) 2008 James McKaskill <jmckaskill@gmail.com>                  *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#ifndef QSMP_LUATCPCONSOLE
#define QSMP_LUATCPCONSOLE


#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <lua.h>
#include <qsmp/common.h>
#include <QtCore/qobject.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qtcpserver.h>
#include <string>
#include <vector>


#define QSMP_LUA_CONSOLE_PORT 12345


QSMP_BEGIN

class LuaTcpSocket;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class LuaTcpServer : public QObject
{
  Q_OBJECT
public:
  LuaTcpServer();
private Q_SLOTS:
  void onNewConnection();
private:
  void onDisconnect(boost::shared_ptr<LuaTcpSocket> socket);
  QTcpServer server_;
  std::vector<boost::shared_ptr<LuaTcpSocket> > sockets_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int LuaTcpSocket_Print(lua_State* l);
int LuaTcpSocket_RawInput(lua_State* l);

//-----------------------------------------------------------------------------

class LuaTcpSocket : public QObject
{
  Q_OBJECT
public:
  LuaTcpSocket(QTcpSocket* socket, boost::function<void ()> disconnect_callback);
  ~LuaTcpSocket();
private Q_SLOTS:
  void onDisconnect(){disconnect_callback_();}
  void onReadyRead();
private:
  friend int LuaTcpSocket_Print(lua_State* l);
  friend int LuaTcpSocket_RawInput(lua_State* l);
  void write(const char* buf, size_t len){socket_->write(buf,len);}
  void write(const char* str){socket_->write(str,strlen(str));}
  lua_State*                    lua_;
  boost::scoped_ptr<QTcpSocket> socket_;
  boost::function0<void>        disconnect_callback_;
  typedef std::vector<char> read_buffer_t;
  read_buffer_t                 read_buffer_;
  std::string                   terminator_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif