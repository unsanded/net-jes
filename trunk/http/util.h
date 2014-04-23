#ifndef HTTP_UTIL
#define HTTP_UTIL

#ifndef _NO_SSL
#include <openssl/md5.h>
#include <openssl/sha.h>
#endif

using namespace std;

namespace http{
namespace util{
inline string urlEscape(string in){
		unsigned i=0;
		string output;
		char buf[4];
		while (i<in.length())
		if(in[i]<0x30 || (in[i] & 0x1f)>0x1a || in[i]==':' )
			sprintf(buf, "%%%x", in[i++]),
			output += buf;
		else
			output+=in[i++];
		return output;
	}
	inline string urlUnescape(string in){
		string output;
		unsigned i=0;
		char buf[3];
		buf[2]=0;
		while(i<in.length())
		if(in[i]=='%'){
			buf[0]=in[++i];
			buf[1]=in[++i];
			output += (char) strtol(buf ,0, 16);
			i++;
		}else{
			output+=in[i++];
		}
		return output;
	}
	
	/**
	 * parse an url-list. Eg. "name1=value1&name2=value2"
	 * */
	

	
	inline map<string, string> parseUrlList(istream& ss, int count=INT_MAX){
		map<string,string> res;
		string line;
        while(!ss.eof() && count){
			getline(ss, line, '&');
			size_t eqPos = line.find("=");
                        if(eqPos==string::npos){
				res.insert(make_pair(line, ""));
                                count --;
            }else{
				string value=line.substr(eqPos+1);
				line.resize(eqPos);
				res.insert(make_pair(line, value));
                                count --;
			}
		}
		return res;
	}
    inline map<string, string> parseUrlList(string list, int count=INT_MAX){
		istringstream ss(list);
                return parseUrlList(ss, count);
	}
	#ifndef _NO_SSL
	inline string md5Hash(string data){
		unsigned char rawRes[32];
		char strRes[65]; // two characters per byte plus a '\0' at the end
		MD5((unsigned char* ) data.c_str(),data.size(), rawRes);
		unsigned char* initer=rawRes;
		char* outiter=strRes;
		
		for(; initer<rawRes+32; initer++){
			int count;
			outiter+= count=sprintf(outiter, "%2.2x",  *initer);
//			printf("%2.2x\n",  *initer);
		}
		*outiter=0;
		return string(strRes);
	}
	#endif
	
}}

#endif
