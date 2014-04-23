#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <iostream>

#include <string>


//boost stuff
#include <boost/array.hpp>
#include <boost/asio.hpp>

#ifndef _NO_SSL
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/context.hpp>
#endif

#include <boost/algorithm/string/predicate.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <boost/signals2/mutex.hpp>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>


#include <boost/ref.hpp> 

#ifndef _NO_SSL
#include "sslDevice.h"
#endif

#include "postdata.h"
#include "cookiejar.h"
using namespace std;
using namespace boost::iostreams;
using namespace boost::asio::ip;
using namespace boost::asio;
using namespace boost;

#ifndef _NO_SSL
using namespace boost::asio::ssl;
#endif


namespace http
{
	
	
enum HttpVersion{
	HTTP1_0,
	HTTP1_1
};

enum HttpMethod{
	GET=0,
	HEAD,
	POST,
	PUT,
	DELETE,
};

const char* const _MethodStrings[] = {//Requests as string, trailing space included
	"GET ",
	"HEAD ",
	"POST ",
	"PUT ", 
	"DELETE ",
};


typedef map<string, string> HeaderMap ;
typedef map<string, string>::iterator HeaderIterator;
	
 class Request : public boost::iostreams::device<boost::iostreams::bidirectional> 
{
	public: //settings: better not change them halfway unless you know what you're doing
	typedef boost::iostreams::stream<boost::reference_wrapper<http::Request> > streamType;
	bool handleRedirects;// reconnect on 
	bool handleHeaders;
	
		
	
			protected:
		signals2::mutex& mx; // mutual exclusion for multithreading
		
		string url;// eg. http://www.example.org:80/index.html
		
		string portStr; // so we don't have to convert back and forth;
		string host;//www.example.org
		HttpMethod method; // GET / POST etc...
        string request; // /index.html
        HttpVersion version; // 1.1/1.0
		unsigned port; // 80
		unsigned lastReadRawSize;
		
		public:
		int responseCode;
		string responseMessage;
		
		//status: they speak for themselves
		bool connected;
		bool requestSent;
		bool headersSent;
		bool postDataSent; //postData is sent
		bool headersRead;
		bool eof;
//		union{
			int contentLength; //the result of the contentLength header
			int chunkLeft; // how much of the current chunk
//		};
		int contentRead;
		bool chunked;
		bool encryped;

		
		
		//some signals
		
		boost::signal<void (Request* )> onConnect; // connected but nothing sent
		boost::signal<void (Request* )> onRequestSent; // only "GET /bla HTTP/1.1" part sent
		boost::signal<void (Request* )> onHeaderSent; // all the headers sent
		boost::signal<void (Request* )> onError; // all the headers sent
		
		boost::signal<void (Request* )> onAddFilters; // when the filters are added, for instance for gzip decompressing
		
		//some streams
		protected:
		
		#ifndef _NO_SSL
			boost::asio::ssl::stream<boost::asio::ip::tcp::socket>* sslStream;
			ssl_iostream_device* sslDevice;
		#else
//			boost::asio::ip::tcp::socket* socket;
		#endif
		
		public:
		iostream* stream;
		istream* filter;
		
		
		
		PostBodyBase* postBody; //buffer where all the generated postinfo goes. only used if this is actually a post Request
		
		HeaderMap requestHeaders;
		HeaderMap responseHeaders;
//		vector<PostData> postData;
		CookieJar& cookiejar;
		
	
	public:
    Request(string u, HttpMethod method = GET, CookieJar& cookieJar = CookieJar::getGlobalCookies()):mx(*new signals2::mutex),url(u), method(method), cookiejar(cookieJar){
		connected=false;
		requestSent=false;
		headersSent=false;
		postDataSent=false;
		handleRedirects=true;
		handleHeaders=true;
		eof=false;
		
		chunked = false;
		
		parseUrl(u);
		
		contentRead=0;
		postBody = 0;
	}
	
	
	protected:
    void parseUrl(string url){
		size_t hostStart=0;
		port=80;
		portStr="80";
		if(starts_with(url, "http://")){ //http
			hostStart+=7;
			encryped=false;
		}else if(starts_with(url, "https://")){ //https
			#ifndef _NO_SSL
			hostStart+=8;
			port = 443; //default https port
			portStr= "443";
			encryped=true;
			#else
			cerr << "this program is compiled without _NO_SSL defined, and therefore, https is not supported" << endl;
			return ;
			#endif
			
		}
		size_t colonPos = url.find(':', hostStart ); //position of first ':' character
		size_t hostEnd  = url.find('/', hostStart ); //end of "host:port" part
		if(hostEnd==string::npos) //no request specified: should default to "/"
			hostEnd=url.size();

		if( colonPos>hostEnd || colonPos == string::npos)//colon after first '/' or not found so no port defined
		{
			host=url.substr(hostStart, hostEnd-hostStart);
			//leave port as it is: 80 or 443
		}
		else//there is a port defined
		{
			host=url.substr(hostStart, colonPos-hostStart);
			const char* p = url.c_str()+colonPos+1;
			port=atoi(p);
			portStr=url.substr(colonPos+1, hostEnd-colonPos-1);
		}
		request=url.substr(hostEnd); 
		if(request.empty())
			request='/';
		
		addHeader("Host", host);
		addHeader("Connection", "close");
	}
public:
    Request(string host, string request, int port=80, bool encryped=false, HttpMethod method = GET, CookieJar& jar=CookieJar::getGlobalCookies()):mx(*new signals2::mutex), url(url), host(host), method(method), request(request), port(port), encryped(encryped),cookiejar(jar){
		
		postBody=0;
		
		
		char buf[12];
		sprintf(buf, "%u", port);//make portstr
		portStr=buf;
		
		
		addHeader("Host", host);
		addHeader("Connection", "close");
		if(request.empty())
			request='/';
		
		
	}
	
    Request(iostream& stream, string request, string host="", HttpMethod method=GET, CookieJar& jar=CookieJar::getGlobalCookies()): mx(*new signals2::mutex), method(method), request(request), cookiejar(jar){
		connected=true;//if we get a stream, assume we are connected
		
		requestSent=false;
		headersSent=false;

		postDataSent=false;
		handleRedirects=true;
		handleHeaders=true;
		eof=false;
		
		this->stream = &stream;
		this->filter = this->stream;
		if(!host.empty())
			addHeader("Host", host);
		
	}

	void reset(){

		if(!(stream && stream->good() && !stream->eof()) || responseHeaders["Connection"]=="close" )
			connected=false;
		requestSent=false;
		headersSent=false;
		postDataSent=false;
		eof=false;
		contentRead=0;
		responseHeaders.clear();
		responseCode=0;
		responseMessage="";
	}
	operator bool(){
		return connected && !eof;
	}
	protected:
	string getLineRaw(){
		string res="";
		std::getline(*filter, res);
		lastReadRawSize=res.size()+1;
		
		if(res[res.size()-1] == '\r' )
			res.resize(res.size()-1);
		return res;
	}
		
	
	
	public:
	string getLine(){
		if(!connected || eof)
			return "";
		mx.lock();
		string res="";
		
		if(chunked){
			if(chunkLeft>0){
				res=getLineRaw();
				contentRead+= lastReadRawSize;
			}
			chunkLeft-=lastReadRawSize;
			if(chunkLeft==-2){ // it should be -2, because the '\r\n' trailing the chunk are also read
				contentRead-=2; // those '\r\n' don't count as content
				string count=getLineRaw();
				istringstream ss(count);
				ss >> std::hex >> chunkLeft;
				if(!chunkLeft){
					eof=true;//if we get a 0 we're at the end;
					contentLength=contentRead;//for ease of use, set contentlength now we now its value
				}
			}
		}
		//not chunked
		else{
			if(contentRead < contentLength)
				res=getLineRaw();
			eof=(contentRead>=contentLength);
		}
		
		mx.unlock();
		return res;
	}

    std::streamsize read(char* s, std::streamsize n){
		int read=0;
		if(!chunked){
			if(n<(contentLength-contentRead) )
			{
				filter -> read(s, n);
				read=filter->gcount();
				contentRead+=read;
				return read;
			}
			else
			{
				filter->read(s, contentLength-contentRead );
				read = filter->gcount();
				contentRead+=read;
				eof=(contentRead==contentLength);
				return read;
			}
		}
			
		
		if(chunkLeft>n){//the request fits in the current chunk
			filter->read(s, n);//read all requested data
			read=filter->gcount();
			chunkLeft-=read;//now we have not enough chunk left
			
			return read;
		}else{
			filter->read(s, chunkLeft); //read the remains of the chunk
			read=filter->gcount();
			getLineRaw();
			string line=getLineRaw();
			istringstream ss(line);
			
			n-=read;
			
			ss >> chunkLeft;
			filter->read(&s[read], n);
			read += filter->gcount();
		}

		
		return read;
	
	}


	std::streamsize write(const char* s, std::streamsize n){
	#ifdef _NO_SSL
//		return boost::asio::write( stream, boost::asio::buffer(s, n));
		stream->write(s, n);
		return n;
	#else
		return boost::asio::write(*sslStream, boost::asio::buffer(s, n));
	#endif
		
	}
public:
/*Request(Request& other){
	this->filter=other.filter;
	this->stream=other.stream;
	this->responseHeaders=other.responseHeaders;
	this->requestHeaders=other.requestHeaders;
	
}
	*/
	
public:
	istream* operator->(){
		return filter;
	}
	
	bool connect(){
		if(!connected){
			mx.lock();
			#ifndef _NO_SSL
				boost::asio::ssl::context* ctx =  new ssl::context(getService(), ssl::context::sslv23);
			#endif
			tcp::resolver r(getService());
			tcp::resolver::query q(host, portStr);
			
			#ifndef _NO_SSL
			sslStream = new boost::asio::ssl::stream<boost::asio::ip::tcp::socket> (getService(), *ctx);
			#endif
			
			
			tcp::resolver::iterator ep ;
			tcp::resolver::iterator enditer;
			try{
				ep = r.resolve(q);
				if(ep==enditer){
					mx.unlock();
					return true;
				}
			}catch(...){
				cerr << "unable to resolve hostname" << endl;
				return false;
			}
			#ifndef _NO_SSL
				boost::asio::basic_socket<boost::asio::ip::tcp, boost::asio::stream_socket_service<boost::asio::ip::tcp> >* sock= &sslStream->lowest_layer();
				boost::system::error_code c;
				sock->connect(*ep,c);
//				boost::asio::connect(sslStream->lowest_layer(), ep);
				sslDevice = new ssl_iostream_device(*sslStream, encryped);
				stream = new boost::iostreams::stream<ssl_iostream_device>(*sslDevice);
			#else
				stream = new ip::tcp::iostream(host, portStr);
			#endif
			
			if (!(*stream)){
				mx.unlock();
                cerr << "connection to " << host << "failed:\n" ;//<< ((ip::tcp::iostream*) stream)->error().message() << endl;
				cerr.flush();
				mx.unlock();
				return false;
			}
			connected=true;
			filter=stream;
			
			onConnect(this);
			mx.unlock();
			return true;
		}
		return true;// we're already connected
	}
	
	
	void send(bool readResponseHeaders=true){
		#ifdef _DEBUG
		cout << "connecting...\n";
		#endif
		if(!connect())
		return;
		
		#ifdef _DEBUG
		cout << "sending request...\n";
		#endif
		
		sendRequest();
		if(postBody){
			long postlength = postBody->getLength();
			addHeader("Content-Type", postBody->getContentType());
			addHeader("Content-Length", postlength);
		}
		
		if(cookiejar.cookieCount()){
			pair<string, string> cookieHeader = cookiejar.makeHeader();
			addHeader(cookieHeader.first, cookieHeader.second);
		}
		#ifdef _DEBUG
		cout << "sending headers...\n";
		#endif

		sendHeaders();
		if(postBody){
			//boost::iostreams::stream<boost::reference_wrapper<Request> > s(*this);
			#ifdef _DEBUG
			cout << "posting data...\n";
			#endif
			postBody->write(*stream);
			stream->flush();
		}
		
		#ifdef _DEBUG
		if(postBody)
			postBody->write(cout) ,
		cout << endl;
		cout << "reading headers...\n"; // note the comma
		#endif
		if(readResponseHeaders)
			readHeaders(handleHeaders);
	}
	
	
	
	void setMethod(HttpMethod method){
		mx.lock();
		this->method = method;
		mx.unlock();
	}
	
	void sendRequest(){
		mx.lock();
		(*stream) << _MethodStrings[method] << request << " HTTP/1.1\r\n" ;
		onRequestSent(this);
		requestSent=true;
		mx.unlock();
	}
	
	void sendHeaders(){
		mx.lock();
		HeaderIterator i;
		for(i=requestHeaders.begin(); i != requestHeaders.end(); i++){
			#ifdef _DEBUG
			cout<<'\t'<< i->first << ": " << i->second << endl;
			#endif
			(*stream) << i->first << ": " << i->second << "\r\n";
			
		}
		onHeaderSent(this);
		(*stream) << "\r\n";
		(*stream).flush();
		mx.unlock();
	}

	void readHeaders(bool handleThem=true){
		mx.lock();
		string line;
		std::getline(*filter, line);
		//HTTP/1.1 200 OK
		if (boost::starts_with(line, "HTTP/1.0"))
			version=HTTP1_0;
		else
			version=HTTP1_1;//default to 1.1 it's the higest supported (and available)
		
		responseCode = atoi(line.c_str() + 9);
		
		#ifdef _DEBUG
		cout << "\t RESPONSE CODE " << responseCode << endl;
		#endif
		
		while(1) {
			line=getLineRaw();
			if(line.empty())
				break;
			size_t colonpos = line.find(':');
			if (colonpos==string::npos) continue;
			
			string name=line;
			name.resize(colonpos);
			string value = line.substr(colonpos+1 + (line[colonpos+1]==' '));
			
			if(name=="Set-Cookie"){
				cookiejar.add(value, host);
				#ifdef _DEBUG
				cout << "\t  adding cookie: " << value << endl;
				#endif
			}
			else{
				responseHeaders.insert(make_pair(name, value));
				#ifdef _DEBUG
				cout << "\t" << name << ": " << value << endl;
				#endif
			}
		}
		
		
		if (responseHeaders["Transfer-Encoding"]=="chunked"){
			chunked=true;
			string cstr;
			cstr=getLineRaw();

			istringstream countString(cstr);
			chunkLeft=0;
			countString >> std::hex >> chunkLeft; // read the first chunk-length // this should be Hex
		}else{
 			contentLength=atoi(responseHeaders["Content-Length"].c_str());
		}

		
		onAddFilters(this);
		
		if(handleThem){
			string encoding = responseHeaders["Content-Encoding"];
			if(encoding=="gzip"){
				filtering_istream* newfilter=new filtering_istream;
				//create a new filter for unzipping
				newfilter->push(gzip_decompressor());
				newfilter->push(*filter);
				filter=newfilter;
			}
		}
		
		
		

		headersRead=true;
		mx.unlock();
	}
	
	template<typename T>
	bool addHeader(string name, T val, bool replace=true){
		ostringstream ss;
		ss<<val;
		string value=ss.str();
		HeaderIterator iter = requestHeaders.find(value);
		if(iter==requestHeaders.end())//add new
			requestHeaders.insert(make_pair(name, value));
		else if(replace)//replace existing
				requestHeaders[name] = value;
		else return false;//don't replace existing
		
		return true;

	}
	

	inline streamType* makeStream(){
		http::Request* dptr = this;

			boost::iostreams::stream<reference_wrapper<http::Request> >* s = new streamType(ref(*dptr));
		return  s;
	}
	
	//*/
	 
	
	~Request(){
		mx.lock();
		if(connected){
			boost::iostreams::close(*filter);//this closes stream automatically
			delete stream;
		}
//		if(headersRead)
//			delete filter; // only have a filter if headers are read
//		delete sslDevice;
		
		mx.unlock(); 
		
		
	}
io_service& getService(){
	static io_service service;
	
	return service;
}



};

inline std::ostream& operator<<(std::ostream& s, Request& r){
	return s << r.getLine();

}
}




#endif // HTTPREQUEST_H


