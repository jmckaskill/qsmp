#ifndef QSMP_LUATCPCONSOLE
#define QSMP_LUATCPCONSOLE

#include "qsmp/common.h"

QSMP_BEGIN

#define QSMP_LUA_CONSOLE_PORT 12345

class LuaTcpSocket;

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

int LuaTcpSocket_Print(lua_State* l);
int LuaTcpSocket_RawInput(lua_State* l);

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
QSMP_END

#endif