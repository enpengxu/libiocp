// server.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "sglib/iocp/iocp_server.h"
#include "sglib/iocp/iocp_loop.h"
//#include <boost/thread/thread.hpp>
//#include <windows.h>

#define sleep  //Sleep(80)

typedef struct {
	int pos;
	int count;
	int id;
	char reply[2];
}session_data_t;

#define BUF_LEN  1024*64 // 32k

class logic_loop : public sglib::net::iocp_loop<logic_loop>
{
	FILE * _pf;
	//char   _buf[BUF_LEN];
	char  *_buf;
	int    _piece_size;
	bool   _quit;

	friend class sglib::net::iocp_loop<logic_loop>;

	int read (session_data_t * s){
		int len;
		if(!_pf) 
			_pf = fopen("test\\server.jpg", "rb");
		
		if(_pf){
			fseek(_pf, s->pos, SEEK_SET);
			len = (int)fread(_buf, 1, _piece_size, _pf);
			if(0 == len){
				s->pos = 0;			
				s->count ++;
				DWORD err = GetLastError();
				return 0;
			}
			s->pos = ftell(_pf);	
		}
		return len;
	}
	
public:
	logic_loop() : _pf(0), _quit(false){
		_piece_size = BUF_LEN;
		_buf = new char[BUF_LEN];
	}
	~logic_loop(){
		if(_pf)
			fclose(_pf);
		if(_buf) delete []_buf;
	}
protected:
	void on_connected() {
		int sessionid = _session->get_id() ;
		std::cout << "\nsession[" << sessionid <<"] NET_connected";

		session_data_t * sdata = new session_data_t;
		memset(sdata, 0, sizeof(session_data_t));
		
		_session->set_session_data(sdata);
		sdata->pos = 0;
		sdata->count = 0;
		sdata->id = 0;
		
		if(sessionid == 1){
			int foo = 1;
		}
		int len = read(sdata);
		assert(len > 0);
		_session->send(_buf, len);
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
		std::cout << "\nsession["<< _session->get_id()<<"] NET_send_finished";
		
		session_data_t * sdata =(session_data_t *) _session->get_session_data();
		_session->recv(sdata->reply, 2);
		sleep;
	}
	void on_recv_finished() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_recv_finished";
		session_data_t * sdata =(session_data_t *) _session->get_session_data();
		char * data = sdata->reply;
		//ASSERT(data == sdata->reply);
		if(data[0] == 'o' && data[1] == 'k' ) {
			int len = read(sdata);
			sdata->id ++;
			if(len == 0){
				_buf[0] = '0';
				len = 1;
			}
			_session->send(_buf, len);
		}
		else {
			std::cout << "\nsession["<< _session->get_id()<<"] reply failed";
		}
		sleep;
	}
	void on_send_error() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_send_error";
		// ***** DO NOT CALL quit here. 
		//_session->quit();
		sleep;
	}
	void on_recv_error() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_recv_error";
		// ***** DO NOT CALL quit here. 
		//_session->quit();
		sleep;
	}
	void on_recv_failed() {
		// buf's size is not enough!
		std::cout << "\nsession["<< _session->get_id()<<"] NET_recv_failed";
		session_data_t * sdata = (session_data_t *)_session->get_session_data();
		int pack_size = _pack.pack_size;
		//ASSERT(pack_size > sdata->buf_len);
		//sdata->buf_len = pack_size * 2;
		//delete []sdata->buf;
		//sdata->buf = new char[sdata->buf_len] ;
		//_session->recv(sdata->buf, sdata->buf_len);
	}
	void on_protocal_error(){
		std::cout << "\n ******* session["<< _session->get_id()<<"] NET_protocol_error ******";
		sleep;
	}
	void on_unhandled() {
		std::cout << "\nsession["<< _session->get_id()<<"] **** NET_unhandled ! ****";		
	}
	void on_noop(){
	}

};


int main(int argc, char * argv[])
{
	int port = 5000;
	int client_limit = 500;
	
	//   create the package queue first. 
	//   the server or clients will create io threads to read or write the queue.
	//   the loop also read & remove items from the queue.
	//   so the server or client is the producer of the queue
	//   the loop is the consumer of the queue
	//   so the queue should be deleted at last! created first!
	sglib::net::msg_queue::init();
	sglib::net::iocp_server _server( port
								   , client_limit
								   , BUF_LEN + 64 // send buffer size
								   , BUF_LEN + 64); // recv buffer size

	int ret = _server.open();
	if(!ret)  
		return 0;
	
	//_server.listener_loop();
	_server.thread_listerner();

	logic_loop loop;
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

