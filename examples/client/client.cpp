// client.cpp : Defines the entry point for the console application.
//

#include "sglib/iocp/iocp_client.h"
#include "sglib/iocp/iocp_loop.h"

#include <windows.h>
#define sleep   //Sleep(1)


#define CLIENT_COUNT  2
#define COPY_COUNT    2

#define RECV_BUF_LEN  (1024*64+100)

typedef struct {
	int pos;
	int count;
	int id;
	char reply[2];
	FILE * pf;
	char buf[RECV_BUF_LEN];
	int  recv_count;
}session_data_t;


class logic_loop : public sglib::net::iocp_loop<logic_loop>
{
	friend class sglib::net::iocp_loop<logic_loop>;
protected:
	void on_connected() {
		session_data_t * sdata = new session_data_t;
		memset(sdata, 0, sizeof(session_data_t));
		sdata->id = connections();

		std::cout << "\nsession["<< sdata->id<<"] NET_connected";

		char path[128];
		sprintf(path, "test\\client_session_%d_%d.jpg", sdata->id, sdata->count);
		sdata->pf = fopen(path, "wb");
		_session->set_session_data(sdata);
		_session->recv(sdata->buf, RECV_BUF_LEN);
		sleep;
	}
	void on_disconnected() {
		session_data_t * sdata = (session_data_t *)_session->get_session_data();

		std::cout << "\nsession["<< sdata->id <<"] NET_disconnected";
		if(sdata->pf)
			fclose(sdata->pf);
		delete sdata;
		delete _session; //!!!

		if (0 == connections())
			quit_loop(true);
		sleep;
	}
	void on_send_finished() {
		session_data_t * sdata = (session_data_t *)_session->get_session_data();

		std::cout << "\nsession["<< sdata->id <<"] NET_send_finished";
		
		sdata->recv_count = 0;
		_session->recv(sdata->buf, RECV_BUF_LEN);
		sleep;
	}
	void on_recv_finished() {
		session_data_t * sdata = (session_data_t *)_session->get_session_data();
		
		std::cout << "\nsession["<< sdata->id <<"] NET_recv_finished";
		
		if(_pack.pack_size == 1){
			if(sdata->pf)
				fclose(sdata->pf);
			++sdata->count;
			if(sdata->count < COPY_COUNT){
				char tmp[128];
				sprintf(tmp, "test\\client_session_%d_%d.jpg", sdata->id, sdata->count);
				sdata->pf = fopen(tmp, "wb");
				sdata->pos = 0;
			}
			else {
				quit_cur_session(); //_session->quit();
				return ;
			}
		}
		else{
			if(sdata->pf)   // too many files opened, maybe failed
				fwrite(sdata->buf, 1, _pack.pack_size, sdata->pf);
		}
		
		sdata->reply[0] = 'o';
		sdata->reply[1] = 'k';
		_session->send(sdata->reply, 2);
		sleep;	
	}
	void on_recv_failed() {

	}
	void on_noop(){

	}
	void on_protocal_error(){
		
	}
	void on_send_error() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_send_error";
		sleep;
	}
	void on_recv_error() {
		std::cout << "\nsession["<< _session->get_id()<<"] NET_recv_error";
		sleep;
	}
	void on_unhandled() {
		std::cout << "\nsession["<< _session->get_id()<<"] **** NET_unhandled ! ****";		
	}
};
			

int main(int argc, char * argv[])
{
	int port = 5000;

	//   create the package queue first. 
	//   the server or clients will create io threads to read or write the queue.
	//   the loop also read & remove items from the queue.
	//   so the server or client is the producer of the queue
	//   the loop is the consumer of the queue
	//   so the queue should be deleted at last! created first!
	sglib::net::msg_queue::init();
	sglib::net::iocp_client client( "127.0.0.1"
								  , port
								  , RECV_BUF_LEN    // send buffer size
								  , RECV_BUF_LEN);	// send buffer size
	
	if (!client.open()) { 
		client.close();	
		return 0;
	}
	if(false== client.sessions_open(CLIENT_COUNT)){
		client.close();	
		return 0;
	}

	logic_loop loop;
	loop.init();

	int count = 0;
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
	sglib::net::msg_queue::get_singleton()->quit(); 
	client.close();
	sglib::net::msg_queue::get_singleton()->deref();
	return 0;
}

