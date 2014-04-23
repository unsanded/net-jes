#ifndef _9292_ROUTE_H
#define _9292_ROUTE_H
#include "location.h"
#include <containers/jsonobject.h>
#include <boost/date_time/gregorian/parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <algorithm>

using namespace std;

using namespace boost::posix_time;
using namespace boost::gregorian;

using namespace containers::json;

namespace http
{

namespace apis
{

namespace ov
{

inline const string makeTime(ptime time){
	stringstream ss;
	ss << time.date().year() << '-';
	if(time.date().month()<10)
		ss << '0';
	ss << ((unsigned short) time.date().month()) << '-';

	if(time.date().day()<10)
		ss << '0';
	ss << time.date().day().as_number();

        ss << 'T';
        if(time.time_of_day().hours()<10){
		ss << '0';
        }

        ss << (int) (time.time_of_day().hours());
	
	if(time.time_of_day().minutes()<10)
		ss << '0';
	ss << time.time_of_day().minutes();
	return ss.str();

}
	
inline const ptime parseTime(string timestr){
    if(timestr.size()!=16)
        return boost::posix_time::ptime();
    std::remove(timestr.begin(), timestr.end(), '-');
    std::remove(timestr.begin(), timestr.end(), ':');

    timestr.resize(timestr.size()-3);
    return from_iso_string(timestr) ;
}

//https://api.9292.nl/0.1/journeys?lang=en-GB&before=1&after=5&from=delfgauw/delftsestraatweg&to=station-alkmaar&sequence=1&dateTime=2013-09-26T1358&byFerry=true&bySubway=true&byBus=true&byTram=true&byTrain=true
struct Route
{
        public:

	struct Leg{
        boost::posix_time::time_duration duration;
        struct Stop: public Location{
               ptime departure;
               ptime arrival;
                string platform;
                double lat;
                double lon;

                Stop(JsonTree& tree): Location(* new JsonTree( &( tree>>"location")) ) { // yes this is a memory leak, and i dont care... for now
                        tree >> "platform" > platform;


                        departure = parseTime( (tree >> "departure").tostring());
                        arrival = parseTime((tree >> "arrival").tostring());



            tree >> "LatLong" >> "lat" > lat;
            tree >> "LatLong" >> "lon" > lon;

        }
        };
    Leg(JsonTree& tree){
        tree >> "destination" > destination;
        tree >> "mode" >> "type" > mode;
        tree >> "mode" >> "name" > modeName;
        tree >> "stops" > stops;
        tree >> "service">service;

    }

    Leg(const Leg& other){
        destination=other.destination;
        platform=other.platform;
        mode=other.mode;
        modeName=other.modeName;
        stops=other.stops;
        service=other.service;
    }
    void print(ostream& str=cout){
        str << "from: " << stops[0].id;
        str << " (" << stops[0].platform << ")" << endl;

        str << "to  : " << stops.back().id;
        str << " (" << stops.back().platform << ")" << endl;
        str << "direction: " << destination << endl;
    }
    std::pair<int,int> getTravelledFrac(double lat, double lon){
        int index;
        //get the two closest stops;
        std::pair<int,int> indexes=getClosestIndexes(lat,lon);
        //get the lowest one: ie the one we have passed already.
        index=min(indexes.first, indexes.second);


        return make_pair(index,stops.size());
    }
    bool isGoingOn(ptime time = boost::posix_time::second_clock::local_time()){
        if(time<stops[0].arrival)
            return false;
        return (time < stops.rbegin()->arrival);
    }

    const inline Stop& firststop(){
        return *stops.begin();
    }
    const inline Stop& laststop(){
        return stops[stops.size()-1];
    }

    std::pair<int,int> getClosestIndexes(double lat, double lon){
        double minDistance ;
        int minDistanceIndex,minDistanceIndex2;
        for(unsigned i=0; i<stops.size(); i++){

            double distance=pow(stops[i].lat - lat, 2) + pow(stops[i].lon - lon,2);
            if(distance < minDistance){
                minDistanceIndex2=minDistanceIndex;

                minDistance=distance;
                minDistanceIndex=i;
            }
        }
        return make_pair(minDistanceIndex, minDistanceIndex2);
    }

        const inline ptime arrivalTime(){
            return laststop().arrival;
        }
        const inline ptime departureTime(){
            return firststop().departure;
        }

        vector<Stop> stops;
        string direction;
        string platform;
        string mode;
        string modeName;
        string destination;
        string service;
	};

	public:
	string id;
	ptime departure;
	long changes;
	ptime arrival;
	



	vector<Leg> legs;
     Route(const Route& other){
         cout << "route cc" << endl;
        this->id = other.id;
        this->legs=other.legs;
        this->arrival=other.arrival;
        this->departure=other.departure;
        this->changes=other.changes;
    }
    Route(JsonTree tree){
        tree >> "numberOfChanges" > changes;
    tree >> "id" > id;
        tree >> "legs" > legs;//so awesome that this actually works;



    departure = parseTime( (tree >> "departure").tostring());
    arrival = parseTime((tree >> "arrival").tostring());

    }
	void print(ostream& str=cout){
		for(vector<Leg>::iterator i= legs.begin(); i< legs.end(); i++){
			 i->print(str);
			str << "----------------------------------" << endl;
		}
    }
	~Route()
	{
	}

static vector<Route> find(Location& from, Location& to,ptime time = second_clock::local_time(), bool isDepartureTime=true , bool byFerry=true, bool bySubway=true, bool byTram=true, bool byTrain=true, bool byBus=true){
	vector<Route> res;
		stringstream url;
        url << "http://api.9292.nl/0.1/journeys?lang=en-GB&before=1&after=5&sequence=1&interchangeTime=standard";
		url << "&from=" << from.id;
		url << "&to="   <<   to.id;

		url << "&dateTime=" << makeTime(time);

		url << "&byFerry=" << ((byFerry)?"true":"false");
		url << "&bySubway=" << ((bySubway)?"true":"false");
		url << "&byTram=" << ((byTram)?"true":"false");
		url << "&byTrain=" << ((byTrain)?"true":"false");
        url << "&byBus=" << ((byBus)?"true":"false");
        url << "&searchType="  << ((isDepartureTime)?"departure":"arrival");



		http::Request req(url.str());
		req.send();
		JsonTree tree(req);
                #ifdef _DEBUG
                cout << "Journey-Tree" << endl;
            tree.write(cout);
		#endif
        if(tree>>"error")
                    return res;
        if(tree>>"exception")
            return res;
		tree >> "journeys" > res;
		return res;
}

};//class route


}}}//http::apis::ov

#endif // _9292_ROUTE_H
