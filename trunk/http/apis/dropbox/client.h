#ifndef __DROPBOX_CLIENT
#define __DROPBOX_CLIENT
#include "http/request.h"
#include <containers/jsonobject.h>
#include "http/oauth/app.h"
using namespace containers::json;
using namespace std;

#define DROPBOX_API_CONTENT "https://api-content.dropbox.com/1/"


namespace http
{

namespace apis
{

namespace dropbox
{
struct AccountInfo {
	bool set;
	long normalBytes;
	long sharedBytes;
	long dataStoreBytes;
	long maxBytes;
	long uid;
	string displayName;
	string email;
	string country;
	string refLink;

	AccountInfo& operator=(AccountInfo other) {
		set=other.set;
		normalBytes=other.normalBytes;
		sharedBytes=other.sharedBytes;
		dataStoreBytes=other.dataStoreBytes;
		maxBytes=other.maxBytes;
		uid=other.uid;
		displayName=other.displayName;
		email=other.email;
		country=other.country;
		refLink=other.refLink;
		set=true;
	}
	AccountInfo() {
		set=false;
	}
	AccountInfo(istream& s) {
		JsonTree info(s);
		info >> "quota_info" >> "normal" > normalBytes;
		info >> "quota_info" >> "shared" > maxBytes;
		info >> "quota_info" >> "datastores" > dataStoreBytes;
		info >> "quota_info" >> "quota" > dataStoreBytes;
		info >> "display_name" > displayName ;
		info >> "email" > email ;
		info >> "country" > country;
		info >> "referral_link" > refLink;
		info >> "uid" > uid;
		set=true;
	}
	AccountInfo(JsonTree& info) {
		info >> "quota_info" >> "normal" > normalBytes;
		info >> "quota_info" >> "shared" > sharedBytes;
		info >> "quota_info" >> "datastores" > dataStoreBytes;
		info >> "quota_info" >> "quota" > maxBytes;
		info >> "display_name" > displayName ;
		info >> "email" > email ;
		info >> "country" > country;
		info >> "referral_link" > refLink;
		info >> "uid" > uid;
		set=true;
	}
};

class Client: public http::oauth::App
{
	AccountInfo accountInfo;
				public:
	int lastError;
	string lastErrorMessage;
	Client(string appkey, string secret):App(appkey, secret, "https://api.dropbox.com/1/oauth/", true) {
		lastError=0;
		if(haveAccess) {
			http::Request accInfoReq("https://api.dropbox.com/1/account/info");
			authorise(accInfoReq);
			accInfoReq.send();
			lastError = accInfoReq.responseCode;
			if(lastError!=200) return;
			JsonTree infoTree(accInfoReq);
			accountInfo= AccountInfo(infoTree);
		}
	}
	AccountInfo& getAccountInfo(){
		return accountInfo;
	}
	
	
	

	~Client() {
	}

};

}

}

}

#endif // __DROPBOX_CLIENT
