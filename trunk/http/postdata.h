#ifndef _POSTDATA_H
#define _POSTDATA_H

#include <string>
#include <ios>
#include <iostream>

#include <boost/iostreams/copy.hpp>
#include "util.h"


using namespace std;



namespace http
{

struct PostBodyBase{
	virtual void write(ostream& dest)=0;
	virtual long getLength()=0;
	virtual string getContentType()=0;
	string boundary;
	PostBodyBase(): boundary("HttpPostBoundaryString---WhyWouldYouHaveThisInYourData"){}
};


class PostData
{
	
	
	public:
	long length;
	
	string name;
	string boundary;
	virtual int getLength(bool last=false)=0;
	virtual void write(ostream& to, bool last=false)=0;
	
	PostData(string name):name(name){
		length=-1;
	}
};


class MultipartPostData: public PostData{
	
	
	
	protected:
	public:
	
	istream* input;
	string header;
	stringstream strdata; //stringstream for sending simple data
	
	MultipartPostData(std::string name, istream& input,  string disposition):PostData(name), input(&input)
	{
		header="Content-Disposition: " + disposition + "; name=\"" + name + "\""; 
	}
	
	
	template<typename T>
	MultipartPostData(std::string name, T data):PostData(name)
	{
		header="Content-Disposition: form_data; name=\"" + name + "\""; 
		input = &strdata;
		strdata<<(data);
	}
	
	inline static string getContentType(PostBodyBase* body){
		return "multipart/form-data, boundary=" + body->boundary;
	}

	
	 int getLength(bool last=false){
		if(length==-1){
			std::streamoff pos=input->tellg();
			input->seekg(0, input->end);
			
			length=input->tellg() - pos;
			
			input->seekg(pos);
		}
		if (last)
			return length+ (2*boundary.size()) + 14 + header.size();
		else
			return length + boundary.size() + 8 /* '\r\n'*4 and '--' */ + header.size();
	}
	
	 void write(ostream& dest, bool last=false){
		dest << "--" << boundary << "\r\n";
		dest << header << "\r\n\r\n";
		boost::iostreams::copy(*input, dest);
		if(last)
			dest<< "\r\n--" << boundary << "--\r\n";
		else
			dest<< "\r\n";
	}
	
	~MultipartPostData()
	{
	}

};


/**
 * Postdata that will be urlencoded.
 * */
class UrlEncodedPostData: public PostData{
	
//	friend class PostBody<UrlEncodedPostData>;
	protected:
	string value;
		public:
	
	template<typename T>
	UrlEncodedPostData(std::string name, T value):PostData(name){
		ostringstream ss;
		ss<< value;
		this->value=util::urlEscape(ss.str());
	}
	
    inline static string getContentType(PostBodyBase* /*body*/){
		return "application/x-www-form-urlencoded";
	}
	
	virtual int getLength(bool last = false){
		int len = name.length() + 1/* '=' */ + value.length() + ((last)?0:1);
		return len;
	}
	void write(ostream& dest, bool last=false){
		dest << name << '=' << value;
		if(!last)// not the last postdata, so write an ampersand
			dest<< '&';
	}
	
	
	~UrlEncodedPostData()
	{
	}

};


template<class PTYPE = MultipartPostData>
	class PostBody: public PostBodyBase{
	
	//todo: make this into a set, and add an operator[] to access and insert stuff
	vector<PTYPE* > values;
	typedef typename vector<PTYPE* >::iterator postIterator;
	
	
	virtual void write(ostream& dest){
		postIterator iter;
		if(values.empty()) return;
		for(iter=values.begin(); iter+1!=values.end(); iter++)
			(*iter)->write(dest);
		(*iter)->write(dest, true);
	}
public:
	void addData(PTYPE* data){
		data->boundary=boundary;
		values.push_back(data);
	}
	
	//todo: make it easier to add data
	
	virtual string getContentType(){
		return PTYPE::getContentType(this);
	}
	
	virtual long getLength(){
		if(values.empty()) return 0;
		long res=0;
		postIterator iter;
		for(iter=values.begin(); iter+1 != values.end(); iter++)
			res+=(*iter)->getLength();
		return res+(*iter)->getLength(true);
		
	}
	
	~PostBody(){
		for (postIterator iter=values.begin(); iter< values.end(); iter++)
			delete *iter;
	}
	
};

}


#endif // _POSTDATA_H
