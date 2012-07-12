#ifndef __iocp_queue_h
#define __iocp_queue_h

#include "iocp_inc.h"
#include "sglib/sglib_refT.h"
#include "sglib/sglib_queueT.h"
#include "sglib/sglib_singletonT.h"

namespace sglib{
	namespace net{
	
	enum NET_status{
		NET_connected=0,
		NET_disconnected,
		
		NET_send_finished,
		NET_send_failed,

		NET_recv_finished,
		NET_recv_failed,
		
		NET_noop,

		NET_send_error,
		NET_recv_error,
		
		NET_protocol_error,
		NET_quit
	};
	typedef struct {
		NET_status status;		// pack's type
		void   *  session;		// iocp_session
		int       pack_size;	// pack's total size.
		int       err;			// for error happened.
	}package_t;
	
	class msg_queue
		: public queueT<package_t>
		, public sglib::singletonT<msg_queue>
		, public sglib::threadrefT<msg_queue> {
		friend class sglib::threadrefT<msg_queue>;
	protected:
		virtual void destroy(){
			quit();
			clear();
			delete this;
		}
		msg_queue(){
		}
	public:
		static void init(){
			new msg_queue();
		}
		void post_quit(){
			package_t pack;
			pack.status = NET_quit;
			put(pack);
		}
	};


	//inline bool iocp_init(){
	//	sglib::net::queue_pk::init();
	//	return true;
	//}
	//inline bool iocp_release(){
	//	sglib::net::queue_pk::get_singleton()->deref();
	//	return true;
	//}

/*

	enum IO_type {
		IO_quit=0,
		IO_send,
		IO_recv
	};
	typedef struct {
		//IO_type type;
		int trans;
		void  * session;
	}socket_io_t;

	class queue_io  
		: public queueT<socket_io_t>
		, public sglib::singletonT<queue_io> {
	};
*/
	//inline bool init_res(){
	//	sglib::net::queue_pk::init();
	//	return true;
	//}
	//inline void release_res(){
	//	// release consumer & producer threads.
	//	sglib::net::queue_pk::get_singleton()->quit(); 
	//}
	//inline void shutdown(){
	//	sglib::net::queue_pk::get_singleton()->deref();
	//
	//}

	}// net
}// sglib
#endif