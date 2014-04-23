#ifndef SSDP_DEVICE_LIST
#define SSDP_DEVICE_LIST
#include "device.h"
#include <set>
//#include <boost/asio/udp.hpp>

using namespace boost::asio::ip;

namespace ssdp
{

class DeviceList: public set<Device>
{
//typedef set<Device>::iterator iterator;


	public:
	bool found;

	DeviceList(){	}
	
	~DeviceList(){}
	
	/**
	 * find by udn;
	 **/
	DeviceList::iterator find(string udn){
		iterator res = find(Device(udn));
		found=(res==end());
		return res;
		
	}
	
	DeviceList::iterator find(udp::endpoint ep){
		for (iterator i = begin(); i!=end(); i++)
		if(i->ep.address()==ep.address())
			return i;
		return end();
	}

	
	
	iterator add(Device candidate){
		return insert(candidate).first;
	}
	
	template<typename T>
	inline DeviceList::iterator operator[](T by){
		return find(by);
	}
};

}

#endif // SSDP_DEVICE_LIST
