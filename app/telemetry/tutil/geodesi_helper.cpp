#include "geodesi_helper.h"

#include <QtGlobal>

extern "C" {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(Q_OS_WIN)
#include "lib/geographiclib-c-2.0/src/geodesic.h"
#else
#include "geodesic.h"
#endif
}

double distance_between(double lat1, double lon1, double lat2, double lon2){
    double s12;
    //double azi1;
    //double azi2;

    geod_geodesic geod{};
    // from https://manpages.ubuntu.com/manpages/bionic/man3/geodesic.3.html
    const double a = 6378137, f = 1/298.257223563; // WGS84
    geod_init(&geod,a,f);
    geod_inverse(&geod,lat1,lon1,lat2,lon2,&s12,0,0); //&azi1,&azi2
    return s12;
}
