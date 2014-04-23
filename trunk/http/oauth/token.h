#ifndef TOKEN_H
#define TOKEN_H
#include "../util.h"
using namespace std;

namespace http
{

namespace oauth
{

class Token
{
	public:
	Token(){
		
	}
	string secret;
	string token;
	string apptoken;
	string appsecret;
	public:
	Token(string token, string secret)://, string apptoken, string appsecret): 
		secret(secret) 
		,token(token)
//		,apptoken(apptoken)
//		,appsecret(appsecret)
	{
		
		
		
	}
	Token(Token& token):
		token(token.token)
		,secret(token.secret)
	{
	}
	Token(istream& source)
	{
		map<string, string> m = http::util::parseUrlList(source);
		secret = http::util::urlUnescape(m["secret"]);
		token = http::util::urlUnescape(m["token"]);
//		res.appsecret = m["appsecret"];
//		res.apptoken = m["apptoken"];

	}
	Token& operator=(Token& other){
		token=other.token;
		secret=other.secret;
	}
	
	~Token()
	{
	}
	void serialize(ostream& stream){
//		stream << "apptoken=" << http::util::urlEscape(apptoken) << "&appsecret=" << appsecret;
		stream << "token=" << http::util::urlEscape(token) << "&secret=" << http::util::urlEscape(secret);
	}

	
};

ostream& operator <<(std::ostream& stream, http::oauth::Token& token){
	token.serialize(stream);
	return stream;
}

istream& operator >> (std::istream& stream, http::oauth::Token& token){
	http::oauth::Token tokenFromStream(stream);
	token = tokenFromStream;
	return stream;
}
}}//namespaces


#endif // TOKEN_H
