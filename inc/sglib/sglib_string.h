#ifndef __sglib_string_h
#define	__sglib_string_h

#include <string>
#include <stdio.h>
//#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>
#include <algorithm>

#include "sglib_assert.h"

#pragma warning(disable : 4996)

namespace sglib{

class string{

public:
	string() {}
	~string() {}

	inline static void tolower(std::string& str){
		std::transform (str.begin(), str.end(), str.begin(), ::tolower);
	}
	inline static void toupper(std::string& str){
		std::transform (str.begin(), str.end(), str.begin(), ::toupper);
	}

	template<typename rtT> 
	inline  static rtT convertT(const std::string & value, rtT failed){
		rtT ret;
		std::istringstream is(value);
		is >> (rtT)ret ;
		if( is.fail())
			return failed;
		return ret;
	};
	template<typename rtT> 
	inline  static rtT convertT(const std::string & value, bool &failed){
		rtT ret;
		std::istringstream is(value);
		is >> (rtT)ret ;
		if( is.fail()){
			failed = true;
			return 0;
		}
		failed = false;
		return ret;
	};
	template<typename rtT> 
	inline static std::string to_string(rtT t, std::string dft=""){
		std::ostringstream os;
		os << (rtT)t;
		if(os.fail())
			return dft;
		return os.str();
	};

	static std::string format(const char * format, ...){
		char buf[1024];
		va_list arglist;
		va_start(arglist, format);
	
		_vsnprintf(buf, 1024, format, arglist);
		va_end(arglist);
		return std::string(buf);
	}

	//#define printf format

	// "    xxxx  xx   " ---> "xxxx  xx   "
	static inline std::string trim_left(const std::string& str, char ch=' ' ){
		size_t off = str.find_first_not_of(ch);
		return str.substr(off);
	}
	// "    xxxx  xx   " ---> "   xxxx  xx"
	static inline std::string trim_right(const std::string& str, char ch=' ' ){
		size_t off = str.find_last_not_of(ch);
		return str.substr(0, off+1);
	}

	// "    xxxx  xx   " ---> "xxxx  xx"
	static inline std::string trim(const std::string& str, char ch=' ' ){
		size_t off_head = str.find_first_not_of(ch);
		size_t off_tril = str.find_last_not_of(ch);
		if (off_head == std::string::npos){
			return "";
		}
		if (off_tril == std::string::npos){
			off_tril = str.length()-1;
		}
		return str.substr(off_head, off_tril-off_head+1);
	}

	// "xxxx  xx" ===> "xxxx", "xx"
	static inline bool split ( const std::string & str
							 , char ch
							 , std::string & str1
							 , std::string & str2
							 ){
		size_t pos = str.find_first_of(ch);	
		if(pos == std::string::npos){
			str1 = str;
			str2 = "";
			return false;
		}
		str1 = str.substr(0, pos);
		str2 = str.substr(pos+1);
		return true;
	}

	static inline std::string path2file(const std::string & path){
		std::string file;
		size_t pos = path.rfind('\\');
		if(pos == std::string::npos)
			file = path;
		else 
			file = path.substr(pos+1);
		
		pos = file.rfind('/');
		if(pos != std::string::npos){
			file = file.substr(pos+1);
		}
		return file;
	}

	static std::string path_to_unix(const std::string & _path){
		std::string path = _path;
		std::replace(path.begin(), path.end(), '\\', '/' );
        if(path[path.length() - 1] != '/' )
            path += '/';
	#ifdef _DEBUG
		std::string::size_type pos = path.find('\\');
		ASSERT(pos == std::string::npos);
	#endif
		return path;
	}

	static void path_split( const std::string _fullpath
						  , std::string & path
						  , std::string & filename){
        std::string fullpath = _fullpath;
        // Replace \ with / first
        std::replace( fullpath.begin(), fullpath.end(), '\\', '/' );
        // split based on final /
        std::string::size_type i = fullpath.find_last_of('/');
        if (i == std::string::npos){
            path.clear();
			filename = fullpath;
        }
        else{
            filename = fullpath.substr(i+1, fullpath.size() - i - 1);
            path = fullpath.substr(0, i+1);
			if(path[path.length()-1] != '/')
				path += '/';
        }
	}
	

	// vsscanf support. vc do not support it. although it's one of c99 standard. 
	static int vsscanf(const char *str, const char *p_format, va_list arg_list){
		char *nformat = _strdup(p_format);
		char *format = nformat;
		int i, n=0, convtot=0;

		for (i=0; format[i]; i++){
			if (format[i] == '%'){
				if (format[i+1] == '%' || format[i+1] == '*' || format[i+1] == '\0')
					i++; 
				else { 
					if (n==0) 
						n++;
					else { 
						char f[3];
						void * p;
						f[1]=format[i+1]; f[2]=format[i+2];
						format[i+1]='n'; format[i+2]='\0'; // %n = nombre de caract¨¨res lus dans l'entr¨¦e
						p = va_arg(arg_list, void *);
						int lu=-1,conv;
						conv = sscanf(str, format, p, &lu);
						if (lu==-1){ 
							convtot += conv; 
							break; 
						}
						convtot++; 
						str += lu; 
						format[i+1]=f[1]; format[i+2]=f[2]; 
						format += i; i = -1; 
						n=0; 
					}
				}
			}
		}
		if (format[i]=='\0' && n>0) {
			void * p = va_arg(arg_list, void *);
			int conv;
			conv = sscanf(str, format, p);
			convtot += conv;
		}
		free(nformat);
		return convtot;
	}
	
	static int scanf(const char * buf, char *fmt, ...){
		va_list argptr;
		int cnt;
		va_start(argptr, fmt);
		cnt = vsscanf(buf, fmt, argptr);
		va_end(argptr);
		return(cnt);
	}	 

}; // string

#if 0
	__declspec(naked)
		static char * __cdecl sprint(const char * format, ...) {
		static char buf[1024]; 
		static int addr;
		
		__asm mov eax, 1024
		__asm push eax
		__asm lea eax, malloc
		__asm call [eax]
		
		__asm pop ebx				  
		__asm pop ebx				  
		__asm mov [eax], ebx		  // buf[0] = return address

		__asm add eax, 4;			  // dword ptr[buf] // 
		__asm push eax				  // stack : buf, format, ....	
		__asm lea eax, sprintf		  
		__asm call [eax]			  
		//__asm call sprintf	
		
		__asm pop eax
		__asm sub eax, 4
		__asm push [eax]
		__asm add eax, 4
		//__asm mov ebx, addr			  // [esp] = buf
		//__asm push ebx		
		//__asm lea eax, buf			  // return pointer
		__asm ret
	}
#endif


} // sglib
#endif