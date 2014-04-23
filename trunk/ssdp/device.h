#ifndef _SSDP_DEVICE
#define _SSDP_DEVICE
#include "service.h"
#include <string>
#include <set> 
using namespace boost::asio::ip;
using namespace std;
namespace ssdp
{
/**
 * Device, a device containing one or more services. A device can be uniquely identified by it's ip adres.
 */

struct Device
{
	friend class Service;
	set<Service> services;
	typedef typename set<Service>::iterator serviceIterator;
	
	udp::endpoint ep;
	
	
	string friendlyName;
	string udn; // unique device name
	
	bool hasUdn;
	
	virtual void onNotify(char* buffer, udp::socket& sock){
			std::istringstream ss(buffer);
			string line;
			std::getline(ss, line);// we don't care about the first line: it should always be "NOTIFY * HTTP/1.1" at this point;
			
			string serviceType;
			
			while(! ss.eof()){
				
				getline(ss, line);
				if(line.empty()) continue;
				if(line[line.size()-1]=='\r')
					line.resize(line.size()-1);
				size_t colonpos = line.find(':');
				if (colonpos==string::npos) continue;
				string name=line;
				name.resize(colonpos);
				if(name=="NT"){
					string value = line.substr(colonpos+1 + (line[colonpos+1]==' '));
					serviceIterator foundService = services.find(value);
					if(foundService==services.end()){
						foundService=services.insert(value).first;
						#ifdef _DEBUG
						cout << "\tdiscovered service on " << friendlyName << " named " << value << endl;
						#endif
					}
					((Service*) &*foundService)->onNotify(buffer, sock);
				}
			}
				
	}
	
	Device(string udn) :udn(udn){

	}

	Device(udp::endpoint alice, string name="") :friendlyName(name), ep(alice)
	{
		if(friendlyName.empty()){
			ostringstream ss;
			ss << ep;
			friendlyName = ss.str();
		}
	}
	
	
	operator string(){
		return udn;
	}
	bool operator>(Device& other){
		return (udn > other.udn);
	}
	bool operator<(Device& other){
		return (udn<other.udn);
	}
	bool operator==(string otherudn){
		return udn==otherudn;
	}
	bool operator==(Device& other){
		return (udn==other.udn);
	}
	
	~Device()
	{
	}

};

bool operator<(Device one, Device other){
	return one.operator <(other);
}
bool operator>(Device one, Device other){
	return one.operator>(other);
}


}

#endif // _SSDP_DEVICE
