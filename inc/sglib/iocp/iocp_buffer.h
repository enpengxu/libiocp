#ifndef __iocp_buffer_h
#define __iocp_buffer_h

namespace sglib{
namespace net{

class iobuf {

public:
	iobuf(int size = 0xffff) : _size(size) {
		_buf = new char [size];
		_buf_end = _buf + size;
		_head = _buf;
		_trail= _buf;
	}
	~iobuf(){
		if(_buf)
			delete _buf;
	}
	/*
	   used   ******   
	   unused ------ 

	   case 1:      h************t-----
	   case 2:      ------h******t-----
	   case 3:	    ******t------h*****

	 */ 
	void get_used_bufs(char ** buf1, int * len1, char ** buf2, int *len2){
		*buf1 = _head;
		if(_trail >= _head){
			*len1 = (int)(_trail - _head);
			*len2 = 0;
		}
		else{
			*len1 = (int)(_buf_end - _head);
			*buf2 = _buf;
		    *len2 = (int)(_trail - _buf);
		}
	}

	void get_unused_bufs(char ** buf1, int * len1, char ** buf2, int *len2){
		*buf1 = _trail;
		if(_trail < _head){
			*len1 = (int)(_head - _trail);
			*len2 = 0;
		}
		else {
			*len1 = (int)(_buf_end - _trail);
			*buf2 = _buf;
			*len2 = (int)(_head - _buf);
		}
	}
	
	inline char * head_move(int step){
		_head += step;
		if(_head >= _buf_end)
			_head -= _size;
		return _head;
	}
	inline char * trail_move(int step){
		_trail += step;
		if(_trail >= _buf_end)
			_trail -= _size;
		return _trail;
	}
	inline int length(){
		int len = (int)(_trail - _head);
		if (len < 0)
			len += _size;
		return len;
	}

	bool detect_add (int addlen) {
		if ((_trail + addlen) >= _buf_end)
			return false;
		return true;
	}

	// copy head's n bytes content
	void safe_copy(char *dst, int n){
		if ( (_head + n) <= _buf_end){
			// todo. use fast memory copy
			memcpy(dst, _head, n);
			return ;
		}
		int base = (int)(_head - _buf);
		int idx ;
		for(int i=0; i<n; i++){
			idx = (base + i) % _size;
			dst[i] = _buf[idx];
		}
	}

	void fast_copy(char * dst, int n){
		char *buf[2];
		int   len[2];

		get_used_bufs(&buf[0], &len[0], &buf[1], &len[1]);
		ASSERT(len[0] > 0);
		int n1 = n >= len[0] ? len[0] : n;
		int n2 = n - n1;
		memcpy(dst, buf[0], n1);
		if(n2){
			memcpy(dst+n1, buf[1], n2);
		}
	}

public:
	char * trail(){
		return _trail;
	}
	char * head(){
		return _head;
	}

	char * reset(){
		_head = _trail = _buf;
		return _head;
	}
	int size(){
		return _size;
	}
protected:
	char * _buf;
	char * _buf_end;
	int    _size;

	char * _head;
	char * _trail;
};

}}
#endif