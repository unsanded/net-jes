#ifndef OAUTH_APP
#define OAUTH_APP
#include "token.h"
#include "../request.h"
#include <fstream>
#include <string>

namespace http
{

namespace oauth
{

class App
{
protected:
string authorisationToken;
string authorisationSecret;

string appKey;
string appSecret;

string oauthUrl;
CookieJar cookies;
Token token;
bool haveAccess; //are we authenticate?


public:
	App(string key, string secret, string oauthUrl, bool autoLoadToken=true, string tokenfile=""):oauthUrl(oauthUrl), appKey(key), appSecret(secret)
	{
		if(autoLoadToken)
			loadToken(tokenfile);
	}
	App(string key, string secret, string oauthUrl, Token* token):oauthUrl(oauthUrl), appKey(key), appSecret(secret)
	{
		haveAccess=token!=0;
		if(token){
			this->token=*token;
		}
	}
	~App()
	{
	}
	/** First step in authorisation. This has to be done only once. This returns an url where the user has to give the app access.
	 * \param string requestTokenUrl this is the url where the temporary token will be requested. this defaults to the oauthUrl given in the constructor appended with "request_token"
	 * \return string authorisation url
	 **/
	
	
	string authorizeStart(string requestTokenUrl = "", string resultBase=""){
		if(requestTokenUrl.empty())
			requestTokenUrl= oauthUrl + "request_token";
		if(resultBase.empty())
			resultBase=oauthUrl + "authorize?oauth_token=";
			
		Request tokenRequest(requestTokenUrl, POST, cookies);
		
		PostBody<UrlEncodedPostData> tokenPostBody;
		unsigned int timestamp = time(0);
		
		string timestamphash;
		{
			ostringstream ss;
			ss<< timestamp;
			#ifdef _NO_SSL
			timestamphash = ss.str();
			#else
			timestamphash = util::md5Hash(ss.str());
			#endif
		}
		
		tokenPostBody.addData(new UrlEncodedPostData("oauth_consumer_key", appKey));
		tokenPostBody.addData(new UrlEncodedPostData("oauth_signature", appSecret + "&"));
		tokenPostBody.addData(new UrlEncodedPostData("oauth_timestamp", timestamp));
		tokenPostBody.addData(new UrlEncodedPostData("oauth_nonce", timestamphash ));
		
		
		
		tokenRequest.postBody=&tokenPostBody;
		tokenRequest.send();
		string line= tokenRequest.getLine();
		
		map<string, string> retVal= util::parseUrlList(line);
		
		
		(authorisationSecret = retVal["oauth_token_secret"]) ;
		(authorisationToken  = retVal["oauth_token"       ]) ;
		
		return resultBase + authorisationToken;
	}

	/**
	 * authoriseEnd
	 * This is the second part of authorisation. this is the part that should be called after the user has allowed the app access via the link returned from authorizeStart
	 * after this the token member is valid, and a pointer to it returned.
	 * \return Token* accesstoken. Save this token somewhere, from now on this is your key into the api or whatever.
	 **/


	Token* authorizeEnd(string requestTokenUrl = ""){
		if(requestTokenUrl.empty())
			requestTokenUrl= oauthUrl + "access_token";
			
		Request tokenRequest(requestTokenUrl, POST, cookies);
		
		PostBody<UrlEncodedPostData> tokenPostBody;
		unsigned int timestamp = time(0);
		
		string timestamphash;
		{
			ostringstream ss;
			ss<< timestamp;
			#ifdef _NO_SSL
			timestamphash = ss.str();
			#else
			timestamphash = util::md5Hash(ss.str());
			#endif
		}
		tokenPostBody.addData(new UrlEncodedPostData("oauth_signature_method", "PLAINTEXT"));
		tokenPostBody.addData(new UrlEncodedPostData("oauth_consumer_key", appKey));
		tokenPostBody.addData(new UrlEncodedPostData("oauth_signature", appSecret + "&" + authorisationSecret));
		tokenPostBody.addData(new UrlEncodedPostData("oauth_timestamp", timestamp));
		tokenPostBody.addData(new UrlEncodedPostData("oauth_nonce", timestamphash ));
		tokenPostBody.addData(new UrlEncodedPostData("oauth_token", authorisationToken));
		
		

		tokenRequest.postBody=&tokenPostBody;
		tokenRequest.send();
		string line= tokenRequest.getLine();
		map<string, string> retVal= util::parseUrlList(line);
		
		cout << line << endl;
		
		return &(token = *new Token(retVal["oauth_token"], retVal["oauth_token_secret"]));
	}

	pair<string, string> makeAuthHeader(){ // sorry for the oneliner;
		return make_pair("Authorization", "OAuth oauth_signature_method=\"PLAINTEXT\", oauth_consumer_key=\"" + appKey + "\", oauth_token=\"" + token.token + "\", oauth_signature=\"" + appSecret + "%26" + token.secret + "\"");
	}
	
	void authorise(Request& req){
		pair<string, string> header = makeAuthHeader();
		req.addHeader(header.first, header.second);
	}
	
	Token& getToken(){
		return token;
	}
	
	inline bool authorized(){
		return haveAccess;
	}
	bool loadToken(string filename=""){
		if(filename.empty())
			filename=typeid(*this).name();//
			filename+=".token";
		std::ifstream tokenfile(filename.c_str());
		if(!tokenfile.good()){
			return haveAccess=false;
		}
		tokenfile >> token; // yes, it is that easy;
		haveAccess=true;
	}
	void saveToken(string filename=""){
		if(filename.empty())
			filename=typeid(*this).name();
			filename+=".token";
		std::ofstream file(filename.c_str());
		file << getToken();
		
	}
	
};
}
}

#endif // OAUTH_APP
