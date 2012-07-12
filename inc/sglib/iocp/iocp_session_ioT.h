#ifndef __iocp_session_ioT_h
#define __iocp_session_ioT_h

#include <malloc.h>
#include <stdlib.h>
#include "iocp_inc.h"
#include "iocp.h"
#include "iocp_queue.h"
#include "iocp_buffer.h"
#include "sglib/sglib_string.h"
/* ------------------------------------------------
 * class sglib::net::stream. 
 * ------------------------------------------------
 */
namespace sglib{
namespace net{

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * class session_ioT. provide message based data for sending and recving.
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
template<class sessionT>  // default size is 64k, tcp/ip window's size
class session_ioT 
{
	#define MAGIC	((unsigned short)0xeffe)
	typedef struct header_t{
		unsigned short magic;	// fixed flag
		//int id;				// session id. peer to peer. hold this during a talk session.
		//unsigned short size;  // content size. not include header_t
		int   size;	// content size. not include header_t
	};
protected:
	char     * _io_send_buf;		// user provided data for sending.
	int        _io_send_buf_len;	// user provided data for sending.
	
	char     * _io_recv_buf;		// user provided data for recving.
	int        _io_recv_buf_len;	// user provided data for recving.
	
	int		   _pack_size;   	
	int		   _io_sended; 

	iobuf	   _recv_buf;
	iobuf	   _send_buf;
	
	OVERLAPPED_EX * _recv_olp; 
	OVERLAPPED_EX * _send_olp; 
public:
	session_ioT ( SOCKET s
				, SOCKADDR_IN * addr
				, int send_bufsize = 0xffff*2
				, int recv_bufsize = 0xffff*2
				) 
				: _socket(s)
				, _addr(*addr)
				, _io_recv_buf(0)
				, _io_send_buf(0)
				, _io_recv_buf_len(0) 
				, _io_send_buf_len(0) 
				, _recv_buf(recv_bufsize) 
				, _send_buf(send_bufsize) {

		_recv_olp = new OVERLAPPED_EX; 
		_send_olp = new OVERLAPPED_EX; 
	}
    virtual ~session_ioT(){
	    ASSERT(_socket ==0);
	   	delete _recv_olp;
		delete _send_olp;
    }

	int io_send_buf_size(){
		return _send_buf.size();
	}
	int io_recv_buf_size(){
		return _recv_buf.size();
	}

	std::string ip_addr(){
		return sglib::string::format("%d.%d.%d.%d"
			, _addr.sin_addr.S_un.S_un_b.s_b1
			, _addr.sin_addr.S_un.S_un_b.s_b2
			, _addr.sin_addr.S_un.S_un_b.s_b3
			, _addr.sin_addr.S_un.S_un_b.s_b4 );
	}

    void close_socket(){
	   	if(_socket){
			int ret = shutdown(_socket, SD_BOTH);	
			closesocket(_socket);	
			_socket = 0;
		}
    }

    // send -------------------------------------------
	int send(char * data, int len) {
		send_init();

		if(!send_add(data, len)){
			ASSERT(0);
			return -1;
		}
		return send_packages();
	}
    // send -------------------------------------------
	int send(char * data1, int len1, char * data2, int len2) {
		send_init();
		// can we hold the whole message?
		int total = len1 + len2 + sizeof(header_t);
		if (!_send_buf.detect_add (total))
			return -1;

		char * buf = _send_buf.trail();
		header_t * head = (header_t *)buf ;
		head->magic = (unsigned short)MAGIC;
		head->size  = (int)(len1 + len2);

		//todo. fast memcpy	
		memcpy (buf+sizeof(header_t),	   data1, len1);
		memcpy (buf+sizeof(header_t)+len1, data2, len2);

		_send_buf.trail_move (total);
		return send_packages();
	}

	// send multi-packages ----------------------------
	void send_init(){
		_io_send_buf     = _send_buf.reset();
		_io_send_buf_len = _send_buf.size();
	}
	bool send_add (char *data, int len){
		int total = len+sizeof(header_t);
		if (!_send_buf.detect_add (total))
			return false;
		
		// buf is: head+content | head+content| ....
		// so operate trail directly to fill data is safe
		char * buf = _send_buf.trail();
		header_t * head = (header_t *)buf ;
		head->magic = (__int16)MAGIC;
		head->size  = (int)len;
		//todo. fast memcpy	
		memcpy (buf+sizeof(header_t), data, len);
		_send_buf.trail_move (total);
		return true;
	}
	inline int send_packages(){
		return send();
	}

	int recv(char * data, int len){
		_io_recv_buf		= data;
		_io_recv_buf_len	= len;
		
		return recv_package(0);
	}
	// called from iocp worker thread.
	void process_iocp (int trans, OVERLAPPED_EX * olp) { 
		int ret = 0;
		sessionT * this_ses = static_cast<sessionT*>(this);
	
		switch(olp->operation){
			case IO_SEND:
				ASSERT(olp == _send_olp);
				if(trans == 0){
					// peer has been closed
					_err = GetLastError();
					this_ses->send_error(_err);
					this_ses->disconected();
				}
				else{
					ret = send_package(trans);
				}
				break;
			case IO_RECV:
				ASSERT(olp == _recv_olp);
				if(trans == 0){
					// peer has been closed
					_err = GetLastError();
					this_ses->recv_error(_err);
					this_ses->disconected();
				}else {
					ret = recv_package(trans);
				}
				break;
			case IO_QUIT:
				this_ses->disconected();
				// can't be here
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	
protected:
	// --------------------------------------------------------------------
	int receive() {
		char * buf[2]; 
		int    len[2]; 
	
		sessionT * this_ses = static_cast<sessionT*>(this);
	
		memset(_recv_olp, 0, sizeof(OVERLAPPED_EX));
		_recv_olp->operation = IO_RECV; 
		
		_recv_buf.get_unused_bufs (&buf[0], &len[0], &buf[1], &len[1]); 

		ASSERT(len[0] > 0);

		int bufs = 1;
		_recv_olp->wsa_buf[0].buf = buf[0];
		_recv_olp->wsa_buf[0].len = len[0]; 
		_recv_olp->wsa_buf[1].buf = buf[1];
		_recv_olp->wsa_buf[1].len = len[1];
		
		if (len[1])
			bufs = 2;

		DWORD receved = 0;
		DWORD flag = 0;
		// async receive. 
		int r = WSARecv (_socket, &(_recv_olp->wsa_buf[0]), bufs, &receved, &flag, (LPWSAOVERLAPPED)_recv_olp, 0);
		if( r == SOCKET_ERROR ){
			_err = WSAGetLastError();
			if(WSA_IO_PENDING == _err){
				//it's ok.
				return 0;
			}
			
			if(WSAEWOULDBLOCK == _err){
				//too many outstanding overlapped socket now
			}

			if(WSAENOTSOCK == _err){
				// the socket has been closed. 
			}
			if(WSAECONNRESET == _err){  // 10054 error. 
				// peer closed	
			}
			this_ses->recv_error(_err);
			this_ses->disconected();
			return -1;
		}
		// recv ok.generate a notify package.
		ASSERT (r == 0);
		return 0;
	}

	// --------------------------------------------------------------------
	int send () {
		sessionT * this_ses = static_cast<sessionT*>(this);
		
		char * buf[2]; 
		int    len[2];  
		int		bufs = 1;

		_send_buf.get_used_bufs (&buf[0], &len[0], &buf[1], &len[1]); 

		memset(_send_olp, 0, sizeof(OVERLAPPED_EX));
		_send_olp->operation = IO_SEND; 
		
		_send_olp->wsa_buf[0].buf = buf[0];    
		_send_olp->wsa_buf[0].len = len[0]; 
		_send_olp->wsa_buf[1].buf = buf[1];    
		_send_olp->wsa_buf[1].len = len[1]; 
		
		ASSERT(len[1] == 0);

		DWORD send = 0;
		DWORD flag = 0;

		int r = WSASend (_socket, &(_send_olp->wsa_buf[0]), bufs, &send, flag, (LPWSAOVERLAPPED)_send_olp, 0);  
		if( r == SOCKET_ERROR ){
			_err = WSAGetLastError();
			if(WSA_IO_PENDING == _err){
				//it's ok.
				return 1;
			}
			//we get a trouble here!
			//TODO check return value here
			if(WSAECONNRESET == _err){  // 10054 error. 
				// remove disconnected
				// sessionT * this_ses = static_cast<sessionT*>(this);
				//int foo = 1;
			}
			this_ses->send_error(_err);
			this_ses->disconected(); //TODO. check more error codes. 
			return 0;
		}
		return 1;
	}
	
	int send_package (int trans){
		sessionT * this_ses = static_cast<sessionT*>(this);
		// skip the block which has been send out
		_send_buf.head_move(trans);
		int left = _send_buf.length();
		if (left == 0){ 
			// no more data left. all data has been send out.
			this_ses->send_finished();
			return 1;
		}
		return send();
	}

	int recv_package(int trans){
		sessionT * this_ses = static_cast<sessionT*>(this);
		_recv_buf.trail_move(trans);
		
		header_t head;
		int r = recv_extract_package(&head);
		_pack_size = head.size;
		if(r == -1 ) { // error!
			this_ses->protocol_error();
			ASSERT(0);
			return -1;
		}

		if(r == 0){
			// continue receive
			receive();  
			return 0;
		}
		if(_pack_size > _io_recv_buf_len){ 
			// no enough buffer, just return failed. 
			this_ses->recv_failed();
			return 0;
		}
		
		//now it's ok, we get a complete package
		//1. remove header
		_recv_buf.head_move(sizeof(header_t));
		//2. copy contents. todo: fast speed copy?
		_recv_buf.fast_copy(_io_recv_buf, _pack_size);
		//3. remove recved block
		_recv_buf.head_move (_pack_size);
		//4. generate io complete package
		this_ses->recv_finished();
		return 1;
	}

	// parse & extract a package.
	int recv_extract_package (header_t * h){
		char * pack = 0;
		memset(h, 0, sizeof(header_t));

		int recved = _recv_buf.length() - sizeof(header_t);
		// not big enough
		// we should continue receive to get at least a complete package.
		if (recved <= 0) 
			return 0;

		_recv_buf.safe_copy((char *)h, sizeof(header_t));

		// check to see it's the correct head or not.
		if (h->magic != MAGIC){
			// get a error 
			ASSERT(0);
			return -1;
		}
		if (h->size <= 0 || h->size >= _recv_buf.size() ){
			// get a error 
			ASSERT(0);
			return -1;
		}

		// to see get a whole package already?
		if (h->size > recved ){ 
			return 0;
		}
		return 1;
	}
protected:
	int _err;
	SOCKET _socket;
	SOCKADDR_IN _addr; 
};

} } // net, sglib
#endif