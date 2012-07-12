// client.cpp : Defines the entry point for the console application.
//
#include "sglib/iocp/iocp_client.h"
#include "sglib/iocp/iocp_loop.h"
#include "sglib/thread/sglib_thread.h"

#include <windows.h>
#define sleep   //Sleep(10)

#define RECV_BUF_LEN  512

typedef struct {
	int id;
	char * buf;
	int    buf_len;
	int    recv_count;
	FILE * pf;
}session_data_t;

class client_loop : public sglib::net::iocp_loop<client_loop>
{
	friend class sglib::net::iocp_loop<client_loop>;
protected:

	void on_connected() {
		session_data_t * sdata = new session_data_t;
		memset(sdata, 0, sizeof(session_data_t));
		sdata->id = _session->get_id();
		sdata->buf_len = RECV_BUF_LEN;
		sdata->buf = new char [sdata->buf_len];

		std::cout << "\nsession["<< sdata->id<<"] NET_connected";

		char path[128];
		sprintf(path, "test\\client_session_%d.txt", sdata->id);
		sdata->pf = fopen(path, "w");
		_session->set_session_data(sdata);
		_session->recv(sdata->buf, RECV_BUF_LEN);
		sleep;
	}
	void on_disconnected() {
		session_data_t * sdata = (session_data_t *)_session->get_session_data();
		std::cout << "\nsession["<< sdata->id <<"] NET_disconnected";
		if(sdata->pf)   // too many files opened, maybe failed
			fclose(sdata->pf);

		delete sdata;
		delete _session; //!!!

		if (0 == connections())
			quit_loop(true);
		sleep;
	}
	void on_send_finished() {
		assert(0);
	}

	void on_recv_finished() {
		session_data_t * sdata = (session_data_t *)_session->get_session_data();
		std::cout << "\nsession["<< sdata->id <<"] NET_recv_finished";
			
		if(sdata->pf)   // too many files opened, maybe failed
			fwrite(sdata->buf, 1, _pack.pack_size, sdata->pf);
		if (strcmp(sdata->buf, "quit") == 0)
			quit_cur_session(); //_session->quit();
		else {//continue receive
			_session->recv(sdata->buf, sdata->buf_len);
			sleep;	
		}
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
		std::cout << "\nsession["<< _session->get_id()<<"] NET_send_error";
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

	//   create the package queue first. 
	//   server or clients will create io threads to read or write data from the queue.
	//   the loop also read & remove items from the queue.
	//   server and client are the producer of the queue
	//   the loop is the consumer of the queue
	//   so the queue should be created first and be deleted at last! 
	sglib::net::msg_queue::init();

	sglib::net::iocp_client client("127.0.0.1", port);
	client_loop loop;
	loop.init();

	if (!client.open()) { // 5000 clients
		client.close();	
		return 0;
	}
	if(false== client.sessions_open(1)){
		client.close();	
		return 0;
	}

	int count = 0;
	int quit  = 0;

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
	client.close();
	sglib::net::msg_queue::get_singleton()->deref();
	return 0;
}

