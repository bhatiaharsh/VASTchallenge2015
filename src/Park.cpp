/// ----------------------------
/// std header files

#include <cstdlib>
#include <cstdio>
#include <string>
#include <numeric>
#include <float.h>
#include "time.h"

/// ----------------------------
/// my header files

#include "Park.h"
#include "VASTApp.h"

/// -----------------------------------------------------------------
using namespace std;

ParkEvent::ParkEvent(QString rline){

    rline = rline.left(rline.size()-1);
    QStringList sl = rline.split(',');

    tm calendar;
    memset(&calendar, 1, sizeof(struct tm));
    strptime(sl[0].toLatin1().data(), "%Y-%m-%d %H:%M:%S", &calendar);

    timestamp = mktime(&calendar);

    /*printf(" new event at %d -- %s", timestamp, sl[0].toLatin1().data());
    printf("\t recovered = %s", ctime(&timestamp));*/

    cust_id = sl[1].toInt();
    location = coords( sl[3].toInt(), sl[4].toInt() );

    if(sl[2].compare("check-in") == 0){         eventtype = CHECKIN;  }
    else if(sl[2].compare("movement") == 0){    eventtype = MOVEMENT;  }
    else{                                       eventtype = 0;
                printf(" event type = %s\n", sl[2].toLatin1().data());  exit(1);  }

    //printf(" New event at %s and [%d, %d]. Type = %d, Cust = %d\n", timestring.c_str(), X, Y, eventtype, cust_id);
}

int ParkRide::get_load(time_t time_) const{

    map<time_t,int>::const_iterator iter = this->ride_load.begin();
    for( ; iter != ride_load.end(); iter++){
        if(iter->first >= time_)
            return iter->second;
    }
    return 0;
}

// analyze the behavior of this ride
void ParkRide::analyze(){

    printf(" --> Analyzing ride (%d,%d)...", ride_coords.first, ride_coords.second);
    fflush(stdout);

    vector<unsigned int> ride_time (all_events.size(), 0);
    map<unsigned long int, unsigned short int> cust_id_counts;

    for(uint i = 0; i < all_events.size(); i++){

        const RideEvent &e = all_events[i];

        map<time_t, int>::iterator ci = ride_load.find(e.time_checkin);
        if(ci == ride_load.end())       ride_load.insert(pair<time_t, int>(e.time_checkin, 1));
        else                            ci->second++;

        map<time_t, int>::iterator co = ride_load.find(e.time_checkout);
        if(co == ride_load.end())       ride_load.insert(pair<time_t, int>(e.time_checkout, -1));
        else                            ci->second--;

        ride_time[i] = (e.time_checkout-e.time_checkin);

        map<unsigned long int, unsigned short int>::iterator iter = cust_id_counts.find(e.cust_id);
        if(iter == cust_id_counts.end())
            cust_id_counts.insert(pair<unsigned long int, unsigned short int>(e.cust_id, 1));
        else
            iter->second++;
    }

    // accumulate the current load at the ride over time!

    map<time_t, int>::iterator it = ride_load.begin();

    max_ride_load = it->second;
    it++;

    for(; it != ride_load.end(); it++){
        map<time_t, int>::const_iterator prv_it = it;
        prv_it--;
        it->second += prv_it->second;
        //printf(" %d -- %d\n", it->first, it->second);

        if(max_ride_load < it->second)
            max_ride_load = it->second;
    }

    //int min_ride_time = *std::min_element(ride_time.begin(), ride_time.end());
    //int max_ride_time = *std::max_element(ride_time.begin(), ride_time.end());
    //float avg_ride_time = (float)std::accumulate(ride_time.begin(), ride_time.end(), 0) / (float)ride_time.size();

    int min_ride_time = ride_time[0];
    int max_ride_time = ride_time[0];
    float avg_ride_time = ride_time[0];
    for(uint i = 1; i < ride_time.size(); i++){

        if(min_ride_time > ride_time[i])    min_ride_time = ride_time[i];
        if(max_ride_time < ride_time[i])    max_ride_time = ride_time[i];

        avg_ride_time += ride_time[i];
    }
    avg_ride_time /= (float)ride_time.size();

    printf(" Done! # events = %d, # custs = %d, ride_time : avg = %.3f, min = %d, max = %d, max ride load = %d!\n",
           all_events.size(), cust_id_counts.size(), avg_ride_time, min_ride_time, max_ride_time, max_ride_load);
}

// analyze the behavior of this customer
void ParkCustomer::analyze(){

    printf(" --> Analyzing customer %d...", cust_id);
    fflush(stdout);

    unsigned int time_ride = 0, time_move = 0;
    unsigned int time_total = (all_events.back().timestamp - all_events.front().timestamp);
    unsigned short int num_rides = 0;

    for(uint i = 0; i < all_events.size()-1; i++){

        const CustomerEvent &e = all_events[i];
        unsigned int this_time = all_events[i+1].timestamp - e.timestamp;

        if(e.type == 1){
            time_ride += this_time;
            num_rides++;
        }

        else
            time_move += this_time;
    }

    float timeperc_ride = 100.0f * (float)time_ride / (float)time_total;
    float timeperc_move = 100.0f * (float)time_move / (float)time_total;

    printf(" Done! # events = %d, # rides = %d, total time = %d, ride_time = %.3f, move_time = %.3f\n",
           all_events.size(), num_rides, time_total, timeperc_ride, timeperc_move);

    return;
    if(all_events.back().type == 1){
        printf(" \t last event is a check in -- "); all_events.back().print();
    }
}


