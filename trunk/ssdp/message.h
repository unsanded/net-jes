#ifndef SSDP_MESSAGE
#define SSDP_MESSAGE
#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>
#include "../http/request.h" 
using namespace http;

namespace ba=boost::asio::ip;

using namespace boost;
using namespace std;

namespace ssdp
{
	
enum Method{
	M_SEARCH,
	NOTIFY,
	HTTP200
};

const char* _MethodStrings[] = { //Requests as string, trailing space included
	"M-SEARCH",
	"NOTIFY",
	"HTTP/1.1 200 OK"
};

class Message
{
	public:
	unsigned short port;
	string portStr;
	string request;
	string host;
	Method method;
	
	HeaderMap headers;
	
	string msg;
	
	bool messageMade;
	bool hasEndpoint;
	bool connected;
		protected:
	ba::udp::socket sock;
	ba::udp::endpoint destination;


public:
	
	
	Message(string url, Method method):sock(getService()), method(method)
	{
		hasEndpoint=false;
		parseUrl(url);
		messageMade=false;
		connected=false;
	}
	
	Message(ba::udp::endpoint& ep, Method method ):sock(getService()), method(method)
	{
		messageMade=false;
		connected=false;
	}


	
	void connect(bool broadcast=false){
		
		if(broadcast){
			boost::system::error_code error;
			sock.open(ba::udp::v4(), error);
			sock.set_option(ba::udp::socket::broadcast(true));

		}
		
		if(!hasEndpoint){
			ba::udp::resolver res(getService());
			ba::udp::resolver::query q(host, portStr);
			destination= *res.resolve(q);
		}
		sock.connect(destination);
		connected=true;
	}
	
	void parseUrl(string url){
		size_t  hostStart=0;
		port=1900;
		portStr="1900";
		if(starts_with(url, "http://")){ //http: one shouldn't use http in ssdp urls but we'll handle it anyways
			hostStart += 7;
		}
		size_t colonPos = url.find(':', hostStart ); //position of first ':' character

		if( colonPos == string::npos)// no colon found so no port defined
		{
			host=url.substr(hostStart);
		}
		else//there is a port defined
		{
			host=url.substr(hostStart, colonPos-hostStart);
			const char* p = url.c_str()+colonPos+1;
			port=atoi(p);
			portStr=url.substr(colonPos+1);
		}
		
		addHeader("Host", host);
	}

	void makeMessage(){
		ostringstream ss(msg);
		if(method==HTTP200)
			ss << _MethodStrings[method];
		else
			ss << _MethodStrings[method] << " * HTTP/1.1\r\n";
		HeaderIterator i;
		for(i=headers.begin(); i != headers.end(); i++)
			ss << i->first << ": " << i->second << "\r\n";
		ss << "\r\n";
		messageMade=true;
		msg=ss.str();
	}

	void send(bool broadcast=false){
		if(!connected && !broadcast)
			connect();
		if(!messageMade)
			makeMessage();
			
		sock.send(
		boost::asio::buffer(msg.c_str(), msg.size())
		);
		
	}

	template<typename T>
	bool addHeader(string name, T value, bool replace=true){
		ostringstream ss;
		ss << value;
		HeaderIterator iter = headers.find(ss.str());
		if(iter==headers.end())//add new
			headers.insert(make_pair(name, ss.str()));
		else if(replace)//replace existing
				headers[name] = ss.str();
		else return false;//don't replace existing
		
		return true;
	}

	ba::udp::socket* getSocket(){
		return &sock;
	}

	~Message()
	{
	}


	io_service& getService(){
		static io_service service;
		return service;
	}

};

}

#endif // SSDP_MESSAGE
