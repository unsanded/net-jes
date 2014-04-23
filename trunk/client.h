#ifndef __DROPBOX_CLIENT
#define __DROPBOX_CLIENT
#include

namespace http
{

namespace apis
{

namespace Dropbox
{

class Client: public http::oauth::app
{
public:
	Client(string appkey, string secret):app(appkey, secret){
		 
	}
	
	
	
	
	~Client()
	{
	}

};

}

}

}

#endif // __DROPBOX_CLIENT
