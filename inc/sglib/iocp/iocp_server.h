#ifndef __iocp_server_h
#define __iocp_server_h

#include "iocp_inc.h"
#include "iocp.h"
#include "iocp_socket.h"
#include "iocp_session.h"
#include "iocp_queue.h"
#include "iocp_loop.h"

//#include <boost/thread/thread.hpp>
#include "sglib/thread/sglib_thread.h"


namespace sglib{
namespace net{

void listerner_thread_proc(void *p);

class iocp_server : public listener_socketT<iocp_server>
				  , public iocp_session_boss {

#if ENABLE_IO_THREAD	
	//--int io_thread_open(){
	//--	//start threads 
	//--	queue_io * qio = new sglib::net::queue_io();
	//--	_io_loop   = new io_loop(qio);
	//--	_io_thread = new boost::thread(*_io_loop);
	//--	return 1;
	//--}
	//--int io_thread_close(){
	//--	if(_io_thread){
	//--		_io_loop->post_quit_io();
	//--		_io_thread->join();
	//--		delete _io_loop;
	//--		delete _io_thread;
	//--		delete sglib::net::queue_io::get_singleton();
	//--		_io_thread = 0;
	//--		_io_loop = 0;
	//--	}
	//--	return 1;
	//--}
	//--boost::thread * _io_thread;
	//--io_loop       * _io_loop;
#endif
	
public:
	iocp_server( int port
			   , int client_limited=100
			   , int send_bufsize = 0xffff*2
			   , int recv_bufsize = 0xffff*2
			   , int iocp_threads = -1
			   )
			   : listener_socketT<iocp_server>(port, client_limited) 
			   , iocp_session_boss(send_bufsize, recv_bufsize)
			   , _closed(false)
			   , _iocp_threads(iocp_threads)
	{
	}

	virtual ~iocp_server(){
		close();
	}
	
	int  get_client_count(){
		return session_count();
	}

	int open(){
		if(!socket_open()){
			socket_close();
			return 0;
		}
		int ref = msg_queue::get_singleton()->ref();

		SYSTEM_INFO si={0};
		GetSystemInfo(&si);
		int thread_count = 2 * si.dwNumberOfProcessors + 2;
		if(_iocp_threads>0)
			thread_count  = _iocp_threads;

		if(!iocp_open(thread_count)){
			socket_close();
			iocp_close();
			//--io_thread_close();
			return 0;
		}
		_closed = false;
		return 1;
	}
 
	int close(){
		if(!_closed){
			_closed = true;
			sessions_close_sockets();
			iocp_close();
			sessions_clear();
			socket_close(); // listener 
			wait_listener();
			msg_queue::get_singleton()->post_quit();
			int ref = msg_queue::get_singleton()->deref();
		}
		return 1;
	}

	// provided for boost::thread only.
	void thread_listerner() {
		_listerner.run(listerner_thread_proc, this);
	}
	int wait_listener(){
		sglib::interlocked_inc(&_stop);
		_listerner.wait();
		return 1;
	}

protected:
	bool _closed;
	sglib::thread _listerner;
	int _iocp_threads;
};	

inline void listerner_thread_proc(void *p){
	iocp_server * server = (iocp_server *)p;
	server->listener_loop();
}

}} // net, sglib
#endif

