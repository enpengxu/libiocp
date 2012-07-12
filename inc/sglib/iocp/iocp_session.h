#ifndef __iocp_session_h
#define __iocp_session_h

#include "iocp_inc.h"
#include "iocp.h"
#include "iocp_queue.h"
#include "iocp_session_ioT.h"
//#include "iocp_session_boss.h"

namespace sglib{
namespace net{
class iocp_session_boss;

class iocp_session : public session_ioT<iocp_session> {

	friend class session_ioT<iocp_session> ;
public:
	iocp_session ( int id
				 , SOCKET s
				 , SOCKADDR_IN * addr
				 , iocp_session_boss * boss
				 , msg_queue * queue_pk
				 , int send_bufsize = 0xffff*2
				 , int recv_bufsize = 0xffff*2
				 )
			: session_ioT<iocp_session>(s, addr, send_bufsize, recv_bufsize)
			, _id(id)
			, _boss(boss)
			, _queue_pk(queue_pk)
			, _debug_disconnected(false) 
			, _session_data(0) {
	}

	virtual ~iocp_session (){
	}
	void set_session_data(void * p){
		_session_data = p;
	}
	void * get_session_data(){
		return _session_data;
	}

	inline int get_id(){
		return _id;
	}
	int join_iocp(HANDLE iocp){
		_iocp = CreateIoCompletionPort( 
				HANDLE(_socket), // handle
		        iocp,			 // iocp handle 
				(ULONG_PTR)this, // key
				0);
		if(NULL == _iocp){
			_err = GetLastError();
			return 0;
		}
		// now session is ready and connected, do something now. 
		// connected!
		// send a package to queue_package
		connected();
		return 1;
	}

	// app call session->quit() to quit talk.
	// quit() ---> 
	//	iocp::thread () ---> 
	//		session::process_iocp()--->
	//			session::disconnect() --->
	//				put disconnected into queue
	//	...  
	//  ...
	// user get disconected package
	// delete session.
	// session's life is over..
	void quit(){
		ULONG_PTR key = (ULONG_PTR)this;
		_recv_olp->operation = IO_QUIT;
		PostQueuedCompletionStatus(_iocp, 0, key, (LPOVERLAPPED)_recv_olp);
	}
	
protected:

	void init_package(package_t * pack, NET_status s){
		memset (pack, 0, sizeof(package_t));
		pack->status	 = s;
		pack->session	 = (void *)this;
		pack->pack_size	 = _pack_size;
	}
	
	inline void disconected();


	void connected(){
		package_t pack;
		init_package(&pack, NET_connected);
		_queue_pk->put(pack);
	}
	void send_finished(){
		if(_socket){
			package_t pack;
			init_package(&pack, NET_send_finished);
			_queue_pk->put(pack);
			ASSERT(!_debug_disconnected);
		}
	}
	void recv_finished(){
		if(_socket){
			package_t pack;
			init_package(&pack, NET_recv_finished);
			_queue_pk->put(pack);
			ASSERT(!_debug_disconnected);
		}
	}
	void recv_failed(){
		if(_socket){
			package_t pack;
			init_package(&pack, NET_recv_failed);
			_queue_pk->put(pack);
			ASSERT(!_debug_disconnected);
		}
	}
	void protocol_error(){
		if(_socket){
			package_t pack;
			init_package(&pack, NET_protocol_error);
			_queue_pk->put(pack);
			ASSERT(!_debug_disconnected);
		}
	}
	void send_error(int err){
		if(_socket){
			package_t pack;
			init_package(&pack, NET_send_error);
			pack.err = err;
			_queue_pk->put(pack);
			ASSERT(!_debug_disconnected);
		}
	}
	void recv_error(int err){
		if(_socket){
			package_t pack;
			init_package(&pack, NET_recv_error);
			pack.err = err;
			_queue_pk->put(pack);
			ASSERT(!_debug_disconnected);
		}
	}
protected:
	HANDLE _iocp;
	iocp_session_boss * _boss;
	//--queue_io * _queue_io;
	msg_queue * _queue_pk;
	unsigned long _err;
	int _id;
	void * _session_data;
	bool  _debug_disconnected;
};

class iocp_session_boss {
protected:
	CRITICAL_SECTION _cs;

public:
	iocp_session_boss( int send_bufsize = 0xffff*2
					 , int recv_bufsize = 0xffff*2
					 ) 
					 : _count(0)
					 , _send_bufsize(send_bufsize) 
					 , _recv_bufsize(recv_bufsize)
	{ 
		InitializeCriticalSection(&_cs);  
	}
    virtual ~iocp_session_boss() {      
	   DeleteCriticalSection(&_cs); 
    }
	
	int session_join(SOCKET s, SOCKADDR_IN * addr) {
		lock l(&_cs);
		//TODO. add more here
		iocp_session * ses = new iocp_session( _count ++ 
					, s
					, addr
					, this 
					, sglib::net::msg_queue::get_singleton()
					, _send_bufsize
					, _recv_bufsize
					);

		if( ses->join_iocp(_iocp.get_handle()) ){
			_sessions.push_back(ses);
			return 1;
		}
		ASSERT(0);
		delete ses;
		return 0;
	}

	// only remove. boss will not delete session.
	// session should be deleted in disconnect function
	void session_remove(iocp_session * s){
		lock l(&_cs);
		std::vector<iocp_session *>::iterator item ;
		item = std::find(_sessions.begin(), _sessions.end(), s);
		if(item != _sessions.end()){
			_sessions.erase(item);
			return ;
		}
		// removed twice?
		ASSERT(0);
	}

	int sessions_close_sockets(){
		std::vector<iocp_session *>::iterator i = _sessions.begin();
		for(; i != _sessions.end(); i++){
			(*i)->close_socket();
		}
		return 1;
	}
	int sessions_clear(){
		lock l(&_cs);
		_sessions.clear();
		return 1;
	}
	int session_count(){
		lock l(&_cs);
		return 	(int)_sessions.size();
	}
	
protected:
	int iocp_open(int thread_count){
		return _iocp.open(thread_count);
	}

	int iocp_close(){
		return _iocp.close();
	}
	
protected:
	std::vector<iocp_session *> _sessions;
	iocp<iocp_session> _iocp;
	int _count;
    int _send_bufsize;
	int _recv_bufsize;

};


	/*
     * --------------------------------------------
	 * if we get a disconnected notify from iocp 
	 * thread. then it means an io operation is failed. 
	 *
	 * so we have to close socket first and set socket 
	 * to zero.  
	 * next time even other iocp thread get an io 
	 * notify, but it can not be added into the 
	 * package queue, so we can make sure the 
	 * disconnected error package is the last 
	 * pack in the queue. so user can delete session
	 * after they get the disconnected pack.
	 * --------------------------------------------
	 * if we multi-thread to send | recv on same 
	 * session(socket), we should add a lock here.
	 * because maybe one iocp thread send a disconnect
	 * notify. but the same time other iocp maybe send  
	 * an io complete notify. 
 	 * --------------------------------------------
	 * if we use the io sequence like this:
	 * send ===>io_process(send_notify)===>on_send_xxx
	 * send ===>io_process(send_notify)===>on_send_xxx
	 * ...
	 * recv ===>io_process(recv_notify)===>on_recv_xxx
	 * recv ===>io_process(recv_notify)===>on_recv_xxx
	 * ...
	 * it will be safe, we don't use lock here.
	 * because iocp thread enter session's io_process 
	 * is one by one. 
	 --------------------------------------------
	 */
	 inline void iocp_session::disconected(){
		close_socket();
		package_t pack;
		init_package(&pack, NET_disconnected);
		_queue_pk->put(pack);
		// just remove pointer, do not delete session.
		// user will delete session if they get a 
		// disconnected package.
		_boss->session_remove(this);	
		_debug_disconnected = true;
	};


} } //iocp, sglib
#endif