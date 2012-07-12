#ifndef __sglib_socket_h
#define	__sglib_socket_h

#include <winsock2.h>

namespace sglib{
namespace net{

template<int max_bufT>
class libsocket {
	
	enum { MAX_BUF_LEN = max_bufT };
#define MAGIC   0x10101010

	typedef struct header_s {
		int magic;
		int length; 
	}header_t;

public:
	static bool startup(){
		WSADATA data;
		if( 0 == WSAStartup(MAKEWORD(2, 2), &data))
			return true;
		return false;
	}
	static bool shutdown(){
		if( SOCKET_ERROR == WSACleanup())
			return false;
		return true;
	}

	libsocket() : _socket(0) {
		_raw_buf  = new char [MAX_BUF_LEN + sizeof(header_t)];
		_user_buf = _raw_buf + sizeof(header_t);
		_user_len = 0;
		_head     = (header_t *)_raw_buf;
		_head->magic = MAGIC;
		memset(_user_buf, 0, sizeof(char)*MAX_BUF_LEN);
	}
	~libsocket(){
		disconnect();
	}

	bool connect(char * ipaddr, int port){
		unsigned long addr; 
		SOCKADDR_IN sock_addr;
		
		memset(&sock_addr, 0, sizeof(SOCKADDR_IN));

		addr = inet_addr(ipaddr);
		memcpy(&sock_addr.sin_addr, &addr, sizeof(addr));
		sockAddr.sin_port = htons((u_short)port);	
		sockAddr.sin_family = AF_INET;

		_socket = socket(AF_INET, SOCK_STREAM, 0);
		if(INVALID_SOCKET == _socket)
			return false;

		if( SOCKET_ERROR == connect (_socket
				, (const struct sockaddr *)&sock_addr
				, sizeof(struct sockaddr)) ){
				disconnect();
				return false;
		}
		return true;
	}
	
	void disconnect(){
		if(_socket){
			closesocket(_socket);
			_socket = 0;
		}
	}

	char * get_buf(){
		return _user_buf;
	}

	bool send(int length){
		_head->magic  = MAGIC;	
		_head->length = length;

		int total   = _head->length + sizeof(header_t);
		int sended  = 0;
		int sending = total;
		char * buf  = _raw_buf;
		int ret;
		do {
			ret = ::send(_socket, buf, sending , 0); 
			if(ret == SOCKET_ERROR)
				return false;
			sended  += ret;
			sending -= sended;
			buf     += sended;
		}while(sending>0);
		return true;	
	}
	
	bool check_recv(){
		int ret;
		FD_ZERO(_fd_set);
		FD_SET(_fd_set, _socket);
		
		timeval timeout = { 0, 0 };
		ret = ::select(0, &_fd_set, 0, 0, &timeout);
		if (SOCKET_ERROR == ret)
			return false;
		if (ret > 0){
			if( FD_ISSET(_socket, &_fd_set)) 
				return true;
		}
		return false;
	}
	
	bool recv(int * size){
		int recved = 0;
		char * buf = _raw_buf;
		int maxlen = MAX_BUF_LEN + sizeof(header_t);

		while(true){
			int ret = ::recv(_socket, buf, maxlen, 0);
			if( ret == SOCKET_ERROR )
				return false;

			if( _head->magic != MAGIC)
				return false;

			recved += ret;
			maxlen -= ret;
			buf    += ret;
			if(recved == _head->length)
				break;
		}
		*size = _head->length;
		return true;
	}

protected:
	SOCKET	   _socket;
	char	 * _raw_buf;
	char	 * _user_buf;
	int		   _user_len;
	header_t * _head;
	fd_set	   _fd_set;
};



}}
#endif