#ifndef __iocp_loop_h
#define __iocp_loop_h

#include "iocp_inc.h"
#include "iocp_queue.h"
#include "iocp_session.h"

#define IOCP_ENABLE_IO_THREAD  1

namespace sglib{
	namespace net{
		template<class impT>
		class iocp_loop {
		protected:
			sglib::net::package_t _pack;
			sglib::net::iocp_session * _session;
			sglib::net::msg_queue    * _queue; 
			bool _quiting;
			int  _connect_count;

			int connections() {
				return _connect_count;
			}
		public:
			iocp_loop() 
				: _quiting(false)
				, _session(0)
				, _connect_count(0) 
				, _queue(0) {
			}
			virtual ~iocp_loop(){
				int ref = _queue->deref();
			}

			virtual bool init(){
				_queue = sglib::net::msg_queue::get_singleton();
				int ref = _queue->ref();
				return true;
			}
			
			void quit_cur_session(){
				ASSERT(_session);
				_session->quit();
			}

			void quit_loop(bool b){
				_quiting = b;
			}
			
			bool isquit(){
				return _quiting;
			}

			void post_quit_pack() {
				sglib::net::package_t pack;
				pack.status  = sglib::net::NET_quit;
				_queue->put(pack);		
			}
			void post_noop_pack(){
				sglib::net::package_t pack;
				pack.status	 = sglib::net::NET_noop;
				pack.session = _session;
				_queue->put(pack);		
			}
			void operator () (){
				while(tick()){
					if(_quiting)
						break;
				}
			}
			bool peek() {
				return _queue->size() > 0 ? true : false;
			}
			int tick() {
				impT * imp = (impT *)this;
				_queue->get(&_pack);
				_session = (sglib::net::iocp_session *)_pack.session;
				
				switch(_pack.status){
				case sglib::net::NET_quit:
					//	imp->on_quit();
					//	return 0; //goto exit;
					quit_loop(true);
					break;

				// connect ------------------------
				case sglib::net::NET_connected:
					_connect_count ++;
					imp->on_connected();
					break;

				// disconnect ---------------------
				case sglib::net::NET_disconnected:
					_connect_count --;
					imp->on_disconnected();
					break;
				// send --------------------------
				case sglib::net::NET_send_finished:
					imp->on_send_finished();
					break;

				// recv --------------------------
				case sglib::net::NET_recv_finished:
					imp->on_recv_finished();
					break;
				case sglib::net::NET_recv_failed:
					imp->on_recv_failed();
					break;
				// noop --------------------------
				case sglib::net::NET_noop:
					imp->on_noop();
					break;
				// network errors -----------------
				case sglib::net::NET_recv_error:
					imp->on_recv_error();
					break;
				case sglib::net::NET_send_error:
					imp->on_send_error();
					break;
				// software errors -----------------
				case sglib::net::NET_protocol_error:
					imp->on_protocal_error();
					break;
				default:
					imp->on_unhandled();
					break;
				}
				return 1;
			}
		};	

#if 0   // unused class
		/* perform all the async send & recv io operation. */
		class io_loop {
		public:
			io_loop(queue_io * qio) 
				: _queue_io(qio) {
			}
			void post_quit_io(){
				socket_io_t io;
				io.trans   = -1;
				io.session = 0;
				_queue_io->put(&io);
			}
			void operator ()(){
				iocp_session * ses = 0;
				socket_io_t sio;
				while(true){
					_queue_io->get(&sio);
					ses = (iocp_session *)sio.session;
					if(sio.trans == -1 && ses == 0)
						break; //goto exit;
					ses->process_io(&sio);
				}
			//exit:
				return ;
			}
		protected:
			queue_io * _queue_io;
		};
#endif

	} // netl
} // sglib
#endif