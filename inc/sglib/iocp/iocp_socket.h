#ifndef __iocp_socket_h
#define __iocp_socket_h
#include "iocp_inc.h"

namespace sglib{
namespace net{

template <class ownerT>
class listener_socketT {
public:
	listener_socketT(int port, int session_limit)
		: _err(0)
		, _session_limit(session_limit)
		, _port(port)
		, _socket(INVALID_SOCKET) 
		, _stop(0) {
	}
	~listener_socketT(){
	}

	// provided for boost::thread only.
	void operator () (){
		listener_loop();
	}
	int listener_loop(){
		ASSERT(_socket != INVALID_SOCKET);

		SOCKADDR_IN addr = { 0 };
		addr.sin_family  = AF_INET;
		addr.sin_port    = htons(_port);
		int len			 = sizeof(SOCKADDR_IN);
		if(_session_limit == -1)
			_session_limit = 0x7fffffff;

		ownerT * pthis = (ownerT * )this;
		// listen for clients. 
		// the loop will exit if following conditions occured:
		// 1. the listener socket is closed
		// 2. if stop falg be set
		// 3. ...
		while(!_stop && _session_limit){
			memset(&addr,0,sizeof(SOCKADDR_IN));
			SOCKET client = accept (_socket, (sockaddr *)&addr, &len);
			if(client ==INVALID_SOCKET ){
				int err = WSAGetLastError();

				if (err == WSAECONNRESET)	// An incoming connection was indicated, but was subsequently terminated by the remote peer prior to accepting the call.
					continue;

				if (err == WSAEMFILE) // run out socket's handle. quit
					break;
				if (err == WSAENETDOWN) // network shutdown. quit
					break;
				if (err == WSAENETDOWN) // The network subsystem has failed.
					break;
				if (err == WSAENOBUFS)  // No buffer space is available.
					break;
				if (err == WSAENOTSOCK) // _socket is not a socket, it has been closed. quit
					break;
				else
					break;
				
			}
			// check to see the owner allow add new mission or not.
			if(_session_limit > 0){
				if(pthis->session_join(client, &addr)){
					_session_limit--;
				}
			}
		}
		return 1;
	}
protected:
	int socket_open() {
		WSADATA data={0};
		WSAStartup(MAKEWORD(2, 2),&data);  // version: 2.2
		
		_socket = socket (AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(_socket==INVALID_SOCKET){
			_err = WSAGetLastError();
			return 0;
		}
		
		SOCKADDR_IN addr = {0};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(_port);
		
		if(bind(_socket, (const sockaddr *)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR){
			_err = WSAGetLastError();
			return 0;
		}
		
		if(listen(_socket,SOMAXCONN)==SOCKET_ERROR){
			_err = WSAGetLastError();
			socket_close();
			return 0;
		}
		return 1;
	}

	int socket_close(){
		if(_socket != INVALID_SOCKET){
			shutdown(_socket, SD_BOTH); 
			closesocket(_socket);
		}
		WSACleanup();
		_socket = INVALID_SOCKET;
		return 1;
	}

protected:
	SOCKET _socket;
	int	   _port;
	int	   _err; 
	int    _session_limit;
	int    _stop;
};

class peer_socket {
public:
	peer_socket() : _err (0), _socket(INVALID_SOCKET) { 
	}
	~peer_socket(){
	}

	int socket_open(std::string ip, int port){
		WSADATA data;	
		
		int ret = WSAStartup(MAKEWORD(2, 2), &data);
		if( ret != 0){
			_err = WSAGetLastError();
			return 0;
		}

		_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(INVALID_SOCKET == _socket){
			_err = WSAGetLastError();
			return 0;
		}	
		
		if(!set_keep_alive(true))
			return 0;

		unsigned long addr; 
		memset(&_sock_addr, 0, sizeof(SOCKADDR_IN));
		addr = inet_addr(ip.c_str());
		memcpy(&_sock_addr.sin_addr, &addr, sizeof(addr));
		_sock_addr.sin_port   = htons((u_short)port);	
		_sock_addr.sin_family = AF_INET;

		if( SOCKET_ERROR == connect( _socket, 
				(const struct sockaddr *)&_sock_addr, 
				sizeof(SOCKADDR_IN)) ){
			_err = WSAGetLastError();
			return 0;
		}
		return 1;
	}
	
	int socket_close(){
		if(_socket)
			closesocket(_socket);
		_socket = 0;

		WSACleanup();
		return 1;
	}
	
	SOCKET get_socket() {
		return _socket;
	}
	SOCKADDR_IN * get_socket_addr(){
		return &_sock_addr;
	}

private:
	bool set_keep_alive(bool b){
		if (false == b) {
			// cancel keep alive setting on the socket.
			BOOL bOptVal = FALSE;
			int bOptLen = sizeof(BOOL);
			if (setsockopt(_socket
					, SOL_SOCKET
					, SO_KEEPALIVE
					, (char*)&bOptVal
					, bOptLen) == SOCKET_ERROR) {
				_err = WSAGetLastError();
				return false;
			}
			return true;
		}
		else {
			struct TCP_KEEPALIVE {  
				u_long onoff;  
				u_long keepalivetime;  
				u_long keepaliveinterval;  
			} ;  
		#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)  
			 //KeepAlive实现  
			 TCP_KEEPALIVE keep_alive_i = {0}; //输入参数  
			 unsigned long len = sizeof(TCP_KEEPALIVE);  
			 TCP_KEEPALIVE keep_alive_o = {0}; //输出参数  
			 unsigned long bytes_ret = 0;  
			 //设置socket的keep alive为15秒，并且发送次数为3次  
			 keep_alive_i.onoff = 1;  
			 keep_alive_i.keepaliveinterval = 65000; //两次KeepAlive探测间的时间间隔  
			 keep_alive_i.keepalivetime	    = 65000; //开始首次KeepAlive探测前的TCP空闭时间  

			 if (WSAIoctl((unsigned int)_socket
				 , SIO_KEEPALIVE_VALS
				 , (LPVOID)&keep_alive_i
				 , len
				 , (LPVOID)&keep_alive_o
				 , len
				 , &bytes_ret
				 , NULL
				 , NULL) == SOCKET_ERROR)  {
				_err = WSAGetLastError();
				return false;
			 }
			return true;
		}
	}
protected:
	SOCKET _socket;
	SOCKADDR_IN  _sock_addr;
	int	   _err; 
};

}
}
#endif