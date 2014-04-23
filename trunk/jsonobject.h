#ifndef JSONOBJECT
#define JSONOBJECT
#include <stdio.h>



namespace containers{
namespace json{

	using namespace std;

	
enum JsonType{
	integer,
	string,
	doublePrecision,
	object, // object/map/accociative array
	array,
	boolean, 
	null
};


class JsonObject
{
		JsonType type;
	
	
public:
	JsonObject(JsonType type):type(type)
	{
	}
	
	virtual JsonObject& operator[](std::string name){
		throw "not implemented";
	}
	
	
	virtual JsonObject& operator[](int    pos ){
		throw "not implemented";
	}
	
	virtual void write(ostream& s)=0;
	operator std::string (){
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
	virtual bool isLeaf(){
		switch( type) {
			case object:
			case array:
				return false;
			default:
				return true;
		}
	}
	template<typename T>
	JsonObject& operator>>(T ref){
		return operator[](ref);
	}
};

struct JsonInt:public JsonObject {
	long value;
	JsonInt(long value): value(value), JsonObject(integer){
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
};
struct JsonString:public JsonObject{
	std::string value;
	JsonString(std::string v):value(v), JsonObject(string){
		
		}
	virtual void write(ostream& s){
		s << '"' << value << '"';
	}
	virtual operator std::string (){
		return value;
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
	virtual operator int(){
		return value;
	}
	virtual operator double(){
		return value> 0.01;
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
};


class JsonMap: public JsonObject, public std::map<std::string, JsonObject*>{
	typedef std::map<std::string, JsonObject*>::iterator iterator;
	
		public:
	JsonMap(): JsonObject(object){
		
	}
	virtual ~JsonMap(){
		JsonMap::iterator iter;
		for(iter=begin(); iter != end(); iter++)
			delete iter->second;
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
		return *map::operator[](name);
	}
};

struct JsonArray: public JsonObject, public std::vector<JsonObject*>{
//	typedef iterator std::vector<JsonObject*>::iterator;
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
		s << '}' << endl;
	}
	
	virtual JsonObject& operator[](int    pos ){
		return *vector::operator[](pos);
	}

};


//brace yourself, this is a very long function. parses any json from a stream. Reads one charcter after
JsonObject* parseObject(istream& source){
	JsonObject* result;
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
				else if(nxt|0x20=='e'){
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
					return new JsonDouble((-intpart-fractpart) * pow(10,exppart));
				else
					return new JsonDouble(( intpart+fractpart) * pow(10,exppart));
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
	
}
class JsonTree{
	JsonObject* obj;
			public:
	JsonTree(istream& source){
		obj=parseObject(source);
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
	
	inline operator std::string(){
		return (std::string) *obj;
	}
};

}}//namespace containers::json

#endif // JSONOBJECT
