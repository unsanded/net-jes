#ifndef COOKIE_JAR_H
#define COOKIE_JAR_H
#include <string>
#include  <set>
#include <boost/algorithm/string/predicate.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;
using namespace boost::posix_time;

namespace http
{



class CookieJar
{

public:
	struct Cookie {
		string name;
		string value;
		string reverseDomain;//reverse, so sorting makes sense
		string path;
		string domain;
		bool wildcardEnd;
		ptime expires;
		bool secure;


		Cookie(string n, string v, string domain, string path):	reverseDomain(domain.rbegin(), domain.rend()), path(path), domain(domain) {
			secure=false;
			
			name=n, value=v;
			if (wildcardEnd = (reverseDomain[reverseDomain.size()-1]=='*'))
				reverseDomain.resize(reverseDomain.size()-(reverseDomain.size()!=1)?2:1);

		}

		inline bool operator< (Cookie other) const { // for sorting
			if( reverseDomain < other.reverseDomain)
				return true; //first sort on domain name
			//then on path
			return(reverseDomain==other.reverseDomain && path<other.path);

		}
		inline bool operator> (Cookie other) const {
			return reverseDomain> other.reverseDomain;
		}

		bool match(Cookie other) const {
			return match(other.reverseDomain, other.path);
		}
		bool match(string revdomain, string path) const {
			// *.example.org
			// gro.elpmaxe.*
			// gro.elpmexa.emtset
			if(wildcardEnd)
				if(boost::starts_with(revdomain, reverseDomain))
					goto checkpath;
			if(revdomain==reverseDomain)
				goto checkpath;
			return false;

checkpath:

                        return boost::starts_with(this->path,path);
		}

		operator string() {
			return name + "=" + value;
		}
		
		void serialize(ostream& output){
			output << domain << '\t' << "FALSE" << '\t' << path << '\t' << (secure?"TRUE":"FALSE") << '\t' << expires << '\t' << name << '\t' << value << endl;
		}

	};


	typedef set<Cookie>::iterator cookieIterator;
	set<Cookie> cookies;

	CookieJar() {}

	CookieJar(CookieJar& other):cookies(other.cookies) {}


	inline cookieIterator begin() const {
		return cookies.begin();
	}
	inline cookieIterator end() const {
		return cookies.end();
	}

	pair<string, string> makeHeader() const {
		string name,value;
		name="Cookie";
		cookieIterator i;
		for(i=begin(); i!=end(); i++)
			value += i->name + "=" + i->value + ";";
//	if(value.size()>0)
//		value.resize(value.size()-1);
		return make_pair(name, value);
	}
	pair<string, string> makeHeader(string domain, string path="/") const {
		//string domain(d.rbegin(), d.rend());
		string name,value;
		name="Cookie";
		cookieIterator i;
		Cookie match("","", domain, path);

		for(i=cookies.find(match); i!=end() && i->match(match.reverseDomain, "/") ; i++)
			if(i->match(match))
				value += i->name + "=" + i->value + ";";
		return make_pair(name, value);

	}

	void add(string headerValue, string domain="") {
		istringstream ss(headerValue);
		string name;
		getline(ss, name, ';');

		size_t eqpos = name.find('=');
		string value = name.substr(eqpos+1);

		name.resize(eqpos);

		string line;
		string path="/";
		string expires;
		while(!ss.eof()) {
			getline(ss, line, ';');
			if(boost::starts_with(line, " domain="))
				domain=line.substr(8);
			else if(boost::starts_with(line, " path="))
				path=line.substr(6);
			else if(boost::starts_with(line, " expires="))
				expires=line.substr(9);

		}
		Cookie newcookie(name, value, domain,path);
		if(!expires.empty()) {
			stringstream ss(expires);
			ss >> newcookie.expires;
		}
	
		cookies.insert(newcookie);
	}


	inline static CookieJar& getGlobalCookies() {
		static CookieJar jar;
		return jar;
	}


	int cookieCount() const {
		return cookies.size();
	}


	~CookieJar() {
	}

};

} //namespace

#endif // COOKIE_JAR_H
