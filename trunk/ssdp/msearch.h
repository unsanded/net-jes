#ifndef SSDP_MSEARCH
#define SSDP_MSEARCH
#include "listener.h"
#include "message.h"
#include <stdlib.h>
#include <cstdlib>
#include <boost/algorithm/string/predicate.hpp>

namespace ssdp
{

class MSearch
{
	friend class Listener;
		protected:
	
	Message msg;
	
	int mx;
	string st;
	string man;
	
	bool listening;
	boost::asio::ip::udp::socket* sock;

	boost::thread* thread;

	boost::signal<void (char* /*message */, udp::endpoint /*from whom*/, char* /*service*/ , udp::socket& /*the socket*/ )> onResponse;

	public:

	MSearch(string searchFor):msg("250.255.255.239", M_SEARCH), st(searchFor)
	{
		msg.addHeader("Host", "239.255.255.250:1900", true);
		man = "\"ssdp:discover\"";
		mx=5;
		thread=0;
		
	}
	void search(){
		
		msg.addHeader("MX", mx);
		msg.addHeader("ST", st);
		msg.addHeader("MAN", man);
		
		msg.connect(true);
		msg.send();
		sock=msg.getSocket();
		boost::thread* t = new boost::thread (boost::function< void (void)>( boost::bind( &MSearch::listen, this ) ) );
//		t->start_thread();
		
	}
	
	void listen(){
		listening = true;
		char buffer[512];
		while(listening){
			sock->receive(boost::asio::buffer(buffer, 512));
			stringstream ss(buffer);

			while(! ss.eof()){
				string line;
				getline(ss, line);
			if(line[line.size()-1]=='\r')
					line.resize(line.size()-1);
					
				size_t colonpos = line.find(':');
				
				string name=line;
				name.resize(colonpos);
				
				if(name=="USN"){
					string value = line.substr(colonpos+1 + (line[colonpos+1]==' '));
					if (!boost::starts_with(value, "uuid:"))
						goto nextMessage; //invalid usn
					value=value.substr(5);
					colonpos=value.find("::");
				}
				
			}

nextMessage: 
asm ("nop");

		}
		thread=0;
	}

	~MSearch()
	{
		listening=false;
		if(thread)
			thread->join();
	}

};

}

#endif // SSDP_MSEARCH
