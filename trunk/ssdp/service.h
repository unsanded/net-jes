#ifndef _SSDP_SERVICE
#define _SSDP_SERVICE
#include<boost/asio.hpp>
class Device;


using namespace boost::asio::ip;
using namespace std;
namespace ssdp
{

enum NotificationType{
	ALIVE,
	PROPCHANGE,
	BYEBYE
};
	
class Service:public string
{
	friend class Device;
	string deviceStatus;
	
		public:

	virtual void onNotify(char* buffer, udp::socket& sock){
		std::istringstream ss(buffer);
		string line;
		std::getline(ss, line);// we don't care about the first line: it should always be "NOTIFY * HTTP/1.1" at this point;
		NotificationType subtype;
		
		while(! ss.eof()){
			std::getline(ss, line);
			if(line.empty()) continue;
			if(line[line.size()-1]=='\r')
				line.resize(line.size()-1);
			size_t colonpos = line.find(':');
			if (colonpos==string::npos) continue;
			
			//split using the first ':' 
			
			string name=line;
			name.resize(colonpos);
			string value = line.substr(colonpos+1 + (line[colonpos+1]==' '));
			
			if(name == "NT")
				string::operator=(value);
			else if (name=="NTS"){
				if(value=="upnp:propchange")
					subtype=PROPCHANGE;
				else if(value=="ssdp:byebye")
					subtype=BYEBYE;
				else
					subtype=ALIVE;
				
			}
			
		}
	}
	


	Service(string serviceType): string(serviceType)
	{
		
	}
	~Service()
	{
	}
	
	bool operator<(Service& other){
		return compare(other)<0;
	}
	bool operator>(Service& other){
		return compare(other)>0;
	}

};

bool operator<(Service a, Service b){
	return a.operator<(b);
}
bool operator>(Service a, Service b){
	return a.operator>(b);
}

}

#endif // _SSDP_SERVICE
