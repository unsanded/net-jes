#ifndef JSONOBJECT
#define JSONOBJECT
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <string.h>

#include <http/request.h>
namespace containers{
namespace json{

	using namespace std;

	
enum JsonType{
	integer,
	text,
	doublePrecision,
	object, // object/map/accociative array
	array,
	boolean, 
	null
};



class JsonArray;

class JsonObject
{
		JsonType type;
	
	
public:
	JsonObject(JsonType type):type(type)
	{
	}
	
    virtual JsonObject& operator[](std::string /*name*/){
		throw "not implemented";
	}
	
	
    virtual JsonObject& operator[](int   /* pos */){
		throw "not implemented";
	}
	
	
	
	virtual void write(ostream& s)=0;
	inline operator std::string (){
			return tostring();
	}
	virtual std::string tostring(){
			ostringstream ss;
			write(ss);
			return ss.str();
	}

	virtual ~JsonObject()
	{
	}
	
	virtual operator long(){
		throw "not implemented";
	}
	virtual operator double(){
		throw "not implemented";
	}
	virtual operator bool(){
		return true;
	}
	virtual operator const char*(){
		throw "not implemented";
	}
	virtual bool isLeaf(){
		switch( type) {
			case object:
			case array:
				return false;
			default:
				return true;
		}
	}
	virtual JsonObject& operator>>(const char* ref){
		return operator[](ref);
	}
	virtual JsonObject& operator>>(string ref){
		return operator[](ref);
	}
	virtual JsonObject& operator>>(int ref){
		return operator[](ref);
	}
    virtual JsonObject& operator >(string& /*target*/){
		throw "not implemented";
	}

	template<class TYPE>
	JsonObject& operator >(std::vector<TYPE>& target);

	template<class TYPE>
	JsonObject& operator >(std::map<std::string, TYPE>& target){
		throw "not implemented";
	}

    virtual JsonObject& operator >(long& /*target*/){
		throw "not implemented";
	}
    virtual JsonObject& operator >(double& /*target*/){
		throw "not implemented";
	}
    virtual JsonObject& operator >(bool& /*target*/){
		throw "not implemented";
	}
	
	
	
};

struct JsonInt:public JsonObject {
	long value;
    JsonInt(long value): JsonObject(integer), value(value){
	}

	virtual void write(ostream& ss){
		ss << value;
	}
	virtual operator long(){
		return value;
	}
	virtual operator double(){
		return value;
	}
	virtual JsonObject& operator>(long& dest){
		dest=value;
		return *this;
	}
	virtual JsonObject& operator >(string& target){
		stringstream ss;
		ss << value;
		target=ss.str();
                return *this;
	}

};
struct JsonString:public JsonObject{
	std::string value;
    JsonString(std::string v):JsonObject(text), value(v) {
		
		}
	virtual void write(ostream& s){
		s << '"' << value << '"';
	}
	virtual std::string tostring(){
		return value;
	}
	virtual JsonObject& operator>(string& dest){
		dest=value;
		return *this;
	}
};
struct JsonDouble: public JsonObject{
	double value;
	JsonDouble(double val):JsonObject(doublePrecision){
		value = val;
	}
	virtual void write(ostream& s){
		s << value ;
	}
	virtual operator int(){
		return value;
	}
	virtual operator double(){
		return value;
	}
	virtual JsonObject& operator>(bool& dest){
		dest=value;
                return *this;
    }
	virtual JsonObject& operator>(double& dest){
		dest=value;
                return *this;
    }
};

struct JsonBool: public JsonObject{
	bool value;
	JsonBool(double val):JsonObject(boolean){
		value = val;
	}
	virtual void write(ostream& s){
		if (value)
			s << "true";
		else
			s << "false";
	}
	virtual JsonObject& operator>(bool& dest){
         dest=value;
         return *this;
	}

	virtual operator bool(){
		return value;
	}
	virtual operator int(){
		return value;
	}

};
struct JsonNull: public JsonObject{
	JsonNull():JsonObject(null){
		
	}
	virtual void write(ostream& s){
			s << "null";
	}


	virtual operator int(){
		return 0;
	}
	virtual operator double(){
		return 0;
	}
	virtual operator bool(){
		return false;
	}
    virtual JsonObject& operator[](std::string ){
		return *this; // return a null( this one) so something line tree >> "someobj" >> "someotherobj" return null even if someobj doesn't exist
	}
	virtual JsonObject& operator >(string& target){
		target="";
                return *this;
	}

	virtual JsonObject& operator >(double& target){

		target=0.0;
                return *this;
	}


};



class JsonMap: public JsonObject, public std::map<std::string, JsonObject*>{
	typedef std::map<std::string, JsonObject*>::iterator iterator;
		JsonNull* notFoundNull;
		public:
	JsonMap(): JsonObject(object){
		notFoundNull = new JsonNull;
	}
	virtual ~JsonMap(){
		JsonMap::iterator iter;
		for(iter=begin(); iter != end(); iter++)
			delete iter->second;
		delete notFoundNull; 
	}
	virtual void write(ostream& s){
		s << '{' << endl;
		iterator i;
		for(i=begin(); i!=end(); i++){
			s << '\t';
			s << i->first;
			s << "\t: ";
			i->second->write(s);
			s << endl;
		}
		s << '}' << endl;
	}
	
	virtual JsonObject& operator[](std::string name){
		iterator i = find(name);
		if(i==end()){
			return *notFoundNull;
			#ifdef _DEBUG
				cerr << "[json] \"" << name << "\"not found in map";
			#endif

		}
		return *i->second;
	}
};

struct JsonArray: public JsonObject, public std::vector<JsonObject*>{
	JsonArray(): JsonObject(array){
		
	}
	JsonArray& operator<<(JsonObject* obj){
		push_back(obj);
		return *this;
	}
	virtual void write(ostream& s){
		s << '[' << endl;
		iterator i;
		for(i=begin(); i!=end(); i++){
			s << '\t';
			(*i)->write(s);
			s << ',' << endl;
		}
		s << ']' << endl;
	}
	
	virtual JsonObject& operator[](int    pos ){
		return *vector<JsonObject*>::operator[](pos);
	}

	template<class TYPE>
	JsonObject& operator >(std::vector<TYPE>& target);


};


//brace yourself, this is a very long function. parses any json from a stream. Reads one charcter after
inline JsonObject* parseObject(istream& source){
	char curchar=1;
	while (source.good() && curchar){
		source >> ws >> curchar;
		switch(curchar){
			case ']':
			case '}':
				return 0;
			case '[': // we're dealing with an array;
			{
				JsonArray* newArr = new JsonArray();
				
				do{
					JsonObject* arrayMember = parseObject(source);
					if(arrayMember)
						newArr->push_back(arrayMember);
					else// a ']' has been read
						return newArr;
				source >> ws >> curchar;
				}while(curchar== ',');
				
				if(curchar == ']')
					return newArr;
				else 
					throw "Error, falsely terminated array";
					
			}
			case '{':// we're dealing with a map. Json/ javascript calls this an object, but this is confusing, because everyghing is objects. So we'll call it a map
			{
				JsonMap* res = new JsonMap;
				while(1){
					std::string name;
					source >> ws >> curchar;
					if(curchar !='"')
						throw "Error, '\"' expected." ;
					std::getline(source, name, '"');
					source >> ws >> curchar;
					if(curchar!=':')
						throw "error, ':' expected";
					
					JsonObject* obj = parseObject(source);
					res->insert(make_pair(name, obj));
					source >> ws ;
					char nxt=source.peek();
					if(nxt == ','){
						source >> curchar;
						continue;
					}
					if(nxt == '}'){
						source>>curchar;
						return res;
					}
					else
						throw "error, unexpected token";
				}
			}
			case '"':
			case '\'':
			//string
			{
				std::string res;
				std::getline(source, res, curchar);
				return new JsonString(res);
			}
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '-':
			case '+':
			//number
			{
				long intpart=0;
				bool intneg=0;
				bool expneg=0;
				intneg = (curchar=='-');
				if(curchar >'0' && curchar <= '9')
					intpart=curchar-'0';
				double fractpart=0;
				double fractMult=0.1;
				int exppart=0;

				char nxt;
				intp:
				nxt=source.peek();
				if(nxt>='0' && nxt <='9'){
					intpart=intpart*10+(nxt-'0');
					source>>curchar;
					goto intp;
				}else if(nxt == '.'){
					source >> curchar;
					goto fractp;
				}
				else if((nxt|0x20)=='e'){
					source >> curchar;
					goto expp;
				}
				return new JsonInt(intpart);// no point detected, so integer
				fractp:
				nxt=source.peek();
				if(nxt>='0' && nxt <='9'){
					fractpart+=(nxt-'0')*fractMult;
					fractMult/=10;
					source>>curchar;
					goto fractp;
				}
				else if((nxt|0x20)=='e'){
					source >> curchar;
					goto expp;
				}
				
				if(intneg)
					return new JsonDouble(0-intpart-fractpart);
				else
					return new JsonDouble(intpart + fractpart);
					
				expp:
				nxt=source.peek();
				if(nxt>='0' && nxt <='9'){
					exppart=exppart*10+(nxt-'0');
					source>>curchar;
					goto expp;
				}else if(nxt=='-'){
					expneg=true;
					source>>curchar;
					goto expp;
				}
				else if(nxt=='+'){
					source>>curchar;
					goto expp;
				}
				
				if(expneg)
					exppart=-exppart;
				if(intneg)
					return new JsonDouble((-intpart-fractpart) * pow(10.0,(double)exppart));
				else
					return new JsonDouble(( intpart+fractpart) * 	pow(10.0,(double)exppart));
			}
			case 'T':
			case 't': //true
			{
				char check[5];
				char* n=check;
				*n=curchar;    //t
				source >> *++n;//r;
				*n |= 0x20;
				source >> *++n;//u
				*n |= 0x20;
				source >> *++n;//e
				*n |= 0x20;
				*++n=0;
				if(strcmp(check, "true"))
					throw "Error, unexpected token: " ;
				return new JsonBool(true);
			}
			case 'F':
			case 'f'://false
			{
				char check[6];
				char* n=check;
				*n=curchar;    //f
				source >> *++n;//a;
				*n |= 0x20;
				source >> *++n;//l;
				*n |= 0x20;
				source >> *++n;//s
				*n |= 0x20;
				source >> *++n;//e
				*n |= 0x20;
				*++n=0;
				if(strcmp(check, "false"))
					throw "Error, unexpected token: " ;
				return new JsonBool(false);
			}
			
			case 'N':
				curchar='n';
			case 'n':
			// null;
			{
				char check[5];
				check[0]=curchar;
				check[4]=0;
				int n=0;
				source >> check[++n];
				check[n] |= 0x20;
				source >> check[++n];
				check[n] |= 0x20;
				source >> check[++n];
				check[n] |= 0x20;
				if(strcmp("null", check))
					throw "Json error: unexpected token after n";
				else 
					return new JsonNull();
				
			}
			  
		}
	}
	cout << "error at " << curchar;
	return 0;
}
class JsonTree{
	JsonObject* obj;
	bool deleteObj;
			public:
	JsonTree(istream& source){
		obj=parseObject(source);
		deleteObj=true;
	}
	JsonTree(JsonObject* o):obj(o){
		deleteObj=false;
	}
	JsonTree(http::Request& req){
		http::Request::streamType* s = req.makeStream();
		obj=parseObject(*s);
		deleteObj=true;
		delete s;
	}

	JsonObject& operator[](std::string name){
		return (*obj)[name];
	}
	JsonObject& operator[](int num){
		return (*obj)[num];
	}
	template<typename T>
	JsonObject& operator>>(T ref){
		return obj->operator[](ref);
	} 

	inline void write(ostream& s){
		obj->write(s);
	}
	
	inline std::string tostring(){
		return  obj->tostring();
	}
	~JsonTree(){
		if(deleteObj)
			delete obj;
	}
};


	template<class TYPE>
	JsonObject& JsonObject::operator >(std::vector<TYPE>& target){
		if(type==array)
			return ((JsonArray*) this)->JsonArray::operator>(target);
		else
			throw "not array to array conversion";
	}
	
	template<class TYPE>
	JsonObject& JsonArray::operator >(std::vector<TYPE>& target){
		for(iterator i=begin(); i<end(); i++){
			JsonTree tree(*i);
			target.push_back(tree);
		}
		return *this;
	}



}}//namespace containers::json

#endif // JSONOBJECT
