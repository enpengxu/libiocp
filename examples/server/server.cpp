/*
	 server.cpp : 
	This demo shows:
	Server send data to client continuely, 
	Client recev data and save it to hard disk.

*/

#include <iostream>
#include "sglib/iocp/iocp_server.h"
#include "sglib/iocp/iocp_loop.h"
#include "sglib/thread/sglib_thread.h"

#include <windows.h>
#define sleep   // Sleep(50)

typedef struct {
	int id;
	char * buf;
	int  buf_len;
}session_data_t;

class server_loop : public sglib::net::iocp_loop<server_loop>
{
	friend class sglib::net::iocp_loop<server_loop>;

public:
	server_loop() {
	}
	~server_loop(){
	}
protected:
	void on_connected() {
		std::cout << "\nsession[" << _session->get_id() <<"] NET_connected";

		session_data_t * sdata = new session_data_t;
		memset(sdata, 0, sizeof(session_data_t));
		sdata->buf_len = 512;
		sdata->buf = new char[sdata->buf_len];
		sdata->id = 0;

		_session->set_session_data(sdata);
		sprintf(sdata->buf, "session::%d:: string from server NO. %d\n", _session->get_id(), sdata->id );
		_session->send(sdata->buf, strlen(sdata->buf));
		sleep;
	}

	void on_disconnected() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_disconnected " ;
		session_data_t * sdata = (session_data_t *)_session->get_session_data();
		delete sdata;
		delete _session; //!!!

		if (0 == connections())
			quit_loop(true);
	}

	void on_send_finished() {
		int sid = _session->get_id();
		ASSERT(sid >= 0);
		std::cout << "\nsession["<< sid <<"] NET_send_finished";
		
		session_data_t * sdata =(session_data_t *) _session->get_session_data();
		sdata->id ++;
		sprintf(sdata->buf, "session::%d:: string from server NO. %d\n", _session->get_id(), sdata->id );
		sleep;

		// !!!important!!!!, if we quit, we can't invoke any io operations any more. 
		if(sdata->id == 300)
			quit_cur_session(); 
		else
			_session->send(sdata->buf, strlen(sdata->buf));
	}
	void on_recv_finished() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_recv_finished";
		sleep;
	}
	
	void on_recv_failed(){
		// buf's size is not enough!
		session_data_t * sdata = (session_data_t *)_session->get_session_data();
		int pack_size = _pack.pack_size;

		ASSERT(pack_size > sdata->buf_len);
		sdata->buf_len = pack_size * 2;
		delete []sdata->buf;
		sdata->buf = new char[sdata->buf_len] ;
		_session->recv(sdata->buf, sdata->buf_len);
	}


	void on_send_error() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_send_error: error code = " << _pack.err;
		sleep;
	}
	void on_recv_error() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_recv_error";
		sleep;
	}

	void on_protocal_error(){
		std::cout << "\n ******* session["<< _session->get_id()<<"] NET_protocol_error ******";
		sleep;
	}
	void on_noop(){
	}

	void on_unhandled() {
		std::cout << "\nsession["<< _session->get_id()<<"] **** NET_unhandled ! ****";		
	}
};


int main(int argc, char * argv[])
{
	int port = 5001;
	int client_limit = 5000;
	
	//   create the package queue first. 
	//   server or clients will create io threads to read or write data from the queue.
	//   the loop also read & remove items from the queue.
	//   server and client are the producer of the queue
	//   the loop is the consumer of the queue
	//   so the queue should be created first and be deleted at last! 
	sglib::net::msg_queue::init();
	sglib::net::iocp_server _server(port, client_limit);
	
	int ret = _server.open();
	if(!ret){  
		sglib::net::msg_queue::get_singleton()->deref();
		return 0;
	}
	
	_server.thread_listerner();

	server_loop loop;
	loop.init();

	int count = 0 ;
	int quit = 0;
	while(true){
		if(loop.isquit())
			break;
		if(!loop.peek()){
			continue;
		}
		if(!loop.tick())
			break;
	}
	sglib::net::msg_queue::get_singleton()->quit(); // release consumer & producer threads.
	_server.close();
	sglib::net::msg_queue::get_singleton()->deref();
	return 0;
}

