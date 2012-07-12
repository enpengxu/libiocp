#ifndef __iocp_client_h
#define __iocp_client_h

#include "iocp_inc.h"
#include "iocp_socket.h"
#include "iocp_session.h"
#include "iocp_queue.h"
#include "iocp_loop.h"

namespace sglib{
namespace net{

class iocp_client : public iocp_session_boss
{
#if 0
		 io_loop  * _io_loop;
	boost::thread * _io_thread;
	int io_thread_open(){
		// init queues
		queue_io * qio = new sglib::net::queue_io();
		queue_pk * qpk = new sglib::net::queue_pk();

		//start threads 
		_io_loop   = new io_loop(qio);
		_io_thread = new boost::thread(*_io_loop);
		return 1;
	}
	int io_thread_close(){
		if(_io_thread){
			_io_loop->post_quit_io();
			_io_thread->join();
			delete _io_loop;
			delete _io_thread;
			delete sglib::net::queue_io::get_singleton();
			delete sglib::net::queue_pk::get_singleton();	
			_io_thread = 0;
			_io_loop = 0;
		}
		return 1;
	}
#endif

public:
	iocp_client( std::string ip
		       , int port
			   , int send_bufsize = 0xffff*2
			   , int recv_bufsize = 0xffff*2
			   )
			   : iocp_session_boss(send_bufsize, recv_bufsize)
			   , _ip(ip)
			   , _port(port) 
	{
	}
	virtual ~iocp_client() {
		close();
	}
	int open() {
		if(!iocp_open())
			return 0;
		return 1;
	}
	int close(){
		sessions_close_sockets();
		iocp_close();
		sessions_clear();
		return 1;
	}
	bool sessions_open(int count=1) {
		// open connections
		for(int i=0; i<count; i++){
			peer_socket s;
			if( 0 == s.socket_open(_ip, _port))
				return false;  //connected error
			// joint session
			session_join(s.get_socket(), s.get_socket_addr());
		}
		return true;
	}
protected:
	int iocp_open(){
		// create an empty iocp 
		return _iocp.open(1);
	}
	int iocp_close(){
		return _iocp.close();
	}
protected:
	int _port;
	std::string _ip;
};

	}
}
#endif