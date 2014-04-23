#include <http/apis/9292/route.h>
using namespace std;

using namespace http::apis;

int main(int argc, char **argv)
{
#ifdef _SSDP_H
	ssdp::MSearch ms("*");
	ms.search();
	ssdp::Listener l;
	l.start(false);
#endif


std::vector<ov::Location> locs = ov::Location::find("alkmaar");

for(std::vector<ov::Location>::iterator i=locs.begin(); i!=locs.end(); i++){
	cout << i->id <<endl;
}


scanf("\n");
}
