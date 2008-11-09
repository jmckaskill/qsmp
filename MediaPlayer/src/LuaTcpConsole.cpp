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

#include "stdafx.h"
#include <qsmp/LuaTcpConsole.h>
#include <qsmp/LuaTcpConsole.moc>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

LuaTcpServer::LuaTcpServer()
{
  connect(&server_,SIGNAL(newConnection()),SLOT(onNewConnection()));
  bool listening = server_.listen(QHostAddress::Any,QSMP_LUA_CONSOLE_PORT);
  assert(listening);
}

//-----------------------------------------------------------------------------

void LuaTcpServer::onNewConnection()
{
  boost::shared_ptr<LuaTcpSocket> socket;
  socket.reset(new LuaTcpSocket(server_.nextPendingConnection(),
                                boost::bind(&LuaTcpServer::onDisconnect,this,socket)));
  sockets_.push_back(socket);
}

//-----------------------------------------------------------------------------

void LuaTcpServer::onDisconnect(boost::shared_ptr<LuaTcpSocket> socket)
{
  sockets_.erase(std::remove(sockets_.begin(),sockets_.end(),socket),sockets_.end());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace
{
  LuaTcpSocket* getSocket(lua_State* L)
  {
    lua_getfield(L,LUA_REGISTRYINDEX,"LuaTcpSocket");
    const LuaTcpSocket* socket = reinterpret_cast<const LuaTcpSocket*>(lua_topointer(L,-1));
    assert(socket);
    lua_pop(L,1);
    return const_cast<LuaTcpSocket*>(socket);
  }

  //-----------------------------------------------------------------------------

  void setSocket(lua_State* L, LuaTcpSocket* socket)
  {
    lua_pushlightuserdata(L,reinterpret_cast<void*>(socket));
    lua_setfield(L,LUA_REGISTRYINDEX,"LuaTcpSocket");
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

LuaTcpSocket::LuaTcpSocket(QTcpSocket* socket,boost::function<void ()> disconnect_callback)
: socket_(socket),
  lua_(lua_open()),
  disconnect_callback_(disconnect_callback)
{
  connect(socket_.get(),SIGNAL(readyRead()),SLOT(onReadyRead()));
  connect(socket_.get(),SIGNAL(disconnected()),SLOT(onDisconnect()));

  luaopen_base(lua_);
  luaopen_table(lua_);
  luaopen_string(lua_);
  luaopen_math(lua_);
  luaopen_debug(lua_);
  setSocket(lua_,this);
  lua_register(lua_,"print",&LuaTcpSocket_Print);
  lua_register(lua_,"raw_input",&LuaTcpSocket_RawInput);
}

//-----------------------------------------------------------------------------

LuaTcpSocket::~LuaTcpSocket()
{
  lua_close(lua_);
}

//-----------------------------------------------------------------------------

int LuaTcpSocket_RawInput(lua_State* L)
{
  LuaTcpSocket* socket = getSocket(L);
  const char* s = lua_tostring(L, -1);
  socket->terminator_ = s;
  lua_pop(L, 1);
  return 0;
}

//-----------------------------------------------------------------------------

// This is essentially a copy of luaB_print (print function provided with lua)
// except that instead of printing to stdout, it prints to our socket
int LuaTcpSocket_Print(lua_State* L){
  LuaTcpSocket* socket = getSocket(L);

  int n = lua_gettop(L);  /* number of arguments */
  int i;
  lua_getglobal(L, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    lua_pushvalue(L, -1);  /* function to be called */
    lua_pushvalue(L, i);   /* value to print */
    lua_call(L, 1, 1);
    s = lua_tostring(L, -1);  /* get result */
    if (s == NULL)
      return luaL_error(L, LUA_QL("tostring") " must return a string to "
                           LUA_QL("print"));
    if (i>1) socket->write("\t");
    socket->write(s);
    lua_pop(L, 1);  /* pop result */
  }
  socket->write("\r\n");
  return 0;
}

//-----------------------------------------------------------------------------

void LuaTcpSocket::onReadyRead()
{
  quint64 to_read = socket_->bytesAvailable();
  size_t size = read_buffer_.size();
  read_buffer_.resize(size + to_read);
  read_buffer_.resize(size + socket_->read(&read_buffer_[size],to_read));

  read_buffer_t::iterator ii = read_buffer_.begin();
  std::vector<read_buffer_t::iterator> to_replace;
  for(;;)
  {
    if (!terminator_.empty())
    {
      boost::iterator_range<read_buffer_t::iterator> trange = 
        boost::find_first(read_buffer_,terminator_);
      if (boost::size(trange) != 0)
      { 
        ii = end(trange);
        --ii;
        terminator_ = "";
        for(read_buffer_t::iterator i = boost::begin(trange);i != boost::end(trange);++i)
          to_replace.push_back(i);
      }
      else
      {
        ii = read_buffer_.end();
      }
      break;
    }
    else
    {
      ii = find(ii,read_buffer_.end(),'\n');
      //We want the user to be able to escape the newline
      //By putting \\\r\n or \\\n, the [\r]\n is escaped
      //Note: need to check for != begin to ensure ii-1/ii-2 is valid
      if (ii != read_buffer_.end() && 
	  ii != read_buffer_.begin())
      {
        if (*(ii-1) == '\\')
        {
          to_replace.push_back(ii-1);
          ++ii;
          continue;
        }
        if (ii-1 != read_buffer_.begin() &&
	    *(ii-1) == '\r' &&
	    *(ii-2) == '\\')
        {
          to_replace.push_back(ii-2);
          ++ii;
          continue;
        }
      }
      break;
    }
  }
  if (ii != read_buffer_.end())
  {
    //want to include the '\n' itself
    ++ii;

    //Lua doesn't like '\\' so we need to filter them out
    //Also need to filter out our terminator
    std::vector<read_buffer_t::iterator>::iterator jj;
    for(jj = to_replace.begin();jj != to_replace.end(); ++jj)
      **jj = ' ';

    luaL_loadbuffer(lua_,&*read_buffer_.begin(),std::distance(read_buffer_.begin(),ii),"TCP Data");

    int status = lua_pcall(lua_,0,0,0);

    switch(status)
    {
    case LUA_ERRRUN:
      {
        size_t length;
        const char* err_string = lua_tolstring(lua_,-1,&length);
        if (err_string)
        {
          socket_->write(err_string,length);
          socket_->write("\r\n");
        }

      }
      break;//should have already been handled in LuaTcpSocket_Error
    case LUA_ERRMEM:
      std::cerr << "Lua Error: Memory" << std::endl;
      break;
    case LUA_ERRERR:
      std::cerr << "Lua Error: Error Function" << std::endl;
      break;
    }
    read_buffer_.erase(read_buffer_.begin(),ii);
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END
