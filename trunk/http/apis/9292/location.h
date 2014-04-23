
#include <containers/jsonobject.h>

#ifndef _9292_H
#define _9292_H


using namespace containers::json;
using namespace std;

namespace http
{

namespace apis
{

namespace ov
{
// one of these: http://api.9292.nl/0.1/locations?lang=en-GB&latlong=52.227,5.181

class Location
{
	public:
	string id;
	string type;
	string houseNr;// string because of housenumbers like "12a"
	string houseNrEnd;//for ranges of housenumbers
	string city;
	string name;
	string regionCode;
	string regionName;
	string countryCode;
	string countryName;
	
	double lat, lon;

	Location(const Location& other){
		id=other.id;
		type=other.type;
		houseNr=other.houseNr;
		city=other.city;
		name=other.name;
		regionCode=other.regionCode;
		regionName=other.regionName;
		countryCode=other.countryCode;
		countryName=other.countryName;
		lat = other.lat;
		lon = other.lon;
	}


	Location(JsonTree& tree)
	{
//		tree.write(cout);
		tree >> "id" >id;
		tree >> "type" > type;
		tree >> "name" > name;
		if(tree >> "houseNr"){
			tree >> "houseNr" > houseNr;
			houseNrEnd=houseNr;
		}
		else if(tree >> "houseNrFrom"){
			tree >> "houseNrFrom" > houseNr;
			tree >> "houseNrTo" > houseNrEnd;
		}
		else
			houseNrEnd = "0",  houseNr = "0";
			
		tree >> "place" >> "name" > city;
		tree >> "place" >> "regionCode" > regionCode;
		tree >> "place" >> "regionName" > regionName;
		tree >> "place" >> "countryCode"> countryCode;
		tree >> "place" >> "countryName"> countryName;
	}
	~Location()
	{
	}


static vector<Location> find(double lat, double lon){
	ostringstream ss;
	ss << "http://api.9292.nl/0.1/locations?lang=en-GB";
	ss << "&latlong=" << lat << ',' << lon;
	
	http::Request r(ss.str());
	r.send();

	JsonTree tree(r);
	JsonArray& locations = (JsonArray&) (tree >> "locations");
	vector<Location> retval;
	for(JsonArray::iterator i= locations.begin(); i<locations.end(); i++){
		JsonTree subtree(*i);
		retval.push_back(subtree);
		subtree.write(cout);
	}
	return retval;
}
static vector<Location> find(string query){
	ostringstream ss;
	ss << "http://api.9292.nl/0.1/locations?lang=en-GB";
    ss << "&q=" << util::urlEscape(query);

	http::Request r(ss.str());
	r.send();

	JsonTree tree(r);
	JsonArray& locations = (JsonArray&) (tree >> "locations");
	vector<Location> retval;
	for(JsonArray::iterator i= locations.begin(); i<locations.end(); i++){
		JsonTree subtree(*i);
		retval.push_back(subtree);
		subtree.write(cout);
	}
	return retval;
}

};


}}}

#endif // _9292_H
