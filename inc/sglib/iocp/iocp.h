#ifndef __iocp_h
#define __iocp_h

#include "iocp_inc.h"

namespace sglib{
namespace net{


template<class sessionT>
class iocp {

public:
	static DWORD WINAPI iocp_thread_proc(LPVOID context)
	{
		DWORD  trans;
		sessionT * session = 0;
		sglib::net::OVERLAPPED_EX  * olp = 0;
		sglib::net::iocp<sessionT> * io  = (sglib::net::iocp<sessionT> *)context;
		HANDLE hiocp = io->get_handle();
		
		while(true) {
			trans = 0;
			if( GetQueuedCompletionStatus(hiocp, 
					&trans,					// bytes
					(PULONG_PTR)&session,	// key
            		(LPOVERLAPPED *)&olp,	// overlapped	
					INFINITE)) {
				if( 0 == session) {	
					//if session == 0. means iocp is being required to close
					goto exit;
				}
				session->process_iocp(trans, olp);
			}
			else {
				DWORD err = GetLastError();
				//TODO. add extra error handle here	
				//Here IOCP will return failed io package if we got IO_PENDING 
				//when send or recv data(WSASend or WSARecv). so we must handle 
				//the error case else we will lost chance to detect errors or disconnect
				//events.
				ASSERT((trans ==0) && session && olp);
				session->process_iocp(trans, olp);
			}
			//if( sglib::net::IO_QUIT == io->check_condition() )
			//	break;
		}
	exit:
		io->notify_thread_quit();
		return 0;
	}
public:
	iocp(): _err(0), _iocp(0), _thread_semaphore(0) {
	}
	~iocp(){
	}

	int open(int thread_count){
		// create an empty iocp 
		_iocp = CreateIoCompletionPort (INVALID_HANDLE_VALUE, NULL, 0, thread_count/*0*/);
		if( !_iocp ) 
			return 0;
		// SYSTEM_INFO si={0};
		// GetSystemInfo(&si);
		// _thread_count = 2 * si.dwNumberOfProcessors + 2;
		_thread_count = thread_count;
		for(int i=0;i<_thread_count; i++){
			QueueUserWorkItem (sglib::net::iocp<sessionT>::iocp_thread_proc, this, WT_EXECUTELONGFUNCTION);
		}
		_thread_semaphore = CreateSemaphore (NULL, 0, _thread_count, NULL);
		return 1;
	}

	int close(){
		if(!_iocp) return 1;
		
		for(int i=0;i<_thread_count; i++){
			ULONG_PTR key = 0;
			if( 0 == PostQueuedCompletionStatus(_iocp,0, key, 0)){
				_err = GetLastError();
				return 0;
			}
		}
		//wait until all threads quit
		WaitForSingleObject(_thread_semaphore, INFINITE);
		CloseHandle(_thread_semaphore);

		CloseHandle(_iocp);
		_iocp = 0;
		return 1;
	}
	
	void notify_thread_quit(){
		ReleaseSemaphore(_thread_semaphore, 1, 0);
	}

	HANDLE get_handle(){
		return _iocp;
	}
	
protected:
	int _err;
	int _thread_count;
	HANDLE _iocp;
	HANDLE _thread_semaphore;
};

}
}
#endif