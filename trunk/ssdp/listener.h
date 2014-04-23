#ifndef _SSDP_LISTENER
#define _SSDP_LISTENER
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <string>
#include <set>
#include "devicelist.h"
#include "msearch.h" 
#include <boost/signals2.hpp>
#include <boost/signals.hpp>
#include <boost/asio.hpp>

using namespace boost::asio::ip;


namespace ssdp
{

class Listener
{
	boost::signals2::mutex mx;
	unsigned sleepTime;
	bool active;
	
	unsigned short port;
	
	DeviceList deviceList;
	typedef typename set<Device>::iterator deviceIterator;
	public:
	
	udp::socket sock;

	boost::signal<void (char*, udp::endpoint, udp::socket& )> onMSearch;
	
	Listener(unsigned port = 1900, unsigned sleeptime = 50): sock(getService()), sleepTime(sleeptime), port(port)
	{
		active=false;
		
		sock.open( udp::v4()) ;

		sock.set_option(udp::socket::broadcast(true));
		sock.set_option(udp::socket::reuse_address(true));
		sock.bind(udp::endpoint(udp::v4(), 1900));
		
	}
		
	void work(){
		char buffer[512];
		udp::endpoint alice; // i'm bob
		active=true;
		while(active){
			sock.receive_from(boost::asio::buffer(buffer, 512), alice);
			mx.lock();
			std::string s(buffer);
			
			if(boost::starts_with(buffer, "M-SEARCH")){
				onMSearch(buffer, alice, *&sock);
			}
			else if(boost::starts_with(s, "NOTIFY")){
				deviceIterator i = deviceList.find(alice);
				if(i==deviceList.end()){//not found, so do nothing
					i=deviceList.add(Device(alice));
				}
				((Device*) &*i)->onNotify(buffer, sock);
			}
			#ifdef _DEBUG
			else
				cout << "received unrecognised:\n\t" << buffer <<endl;
			#endif
			
			mx.unlock();
		}
	}
	
	bool removeDevice(Device& d){
		deviceIterator dev = deviceList.find(d);
		if(dev==deviceList.end())
			return false;
		deviceList.erase(dev);
		return true;
	}

	~Listener()
	{
	}

	void start(bool async=true){
		if(async){
			boost::thread  t (boost::function< void (void)>( boost::bind( &Listener::work , this ) ) );
			t.start_thread();
		}else
			work();
	}
	
	void doMsearch(){
		MSearch ms("*");
		ms.search();
//		ms.onResponse boost::function< void (void)>( boost::bind( &Listener::work , this ) )
	}


	boost::asio::io_service& getService(){
		static io_service service;
		return service;
	}

};

}

#endif // _SSDP_LISTENER
