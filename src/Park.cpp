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

    if(sl.size() != 5){
        printf("\n Invalid event: read line %s, num tokens = %d\n", rline.toLatin1().data(), sl.size());
        eventtype = 0;
        return;
    }
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
        printf("\n Invalid event: read line %s, num tokens = %d\n", sl[2].toLatin1().data(), sl.size());
        return;
    }

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
void ParkRide::analyze(FILE *outfile){

    //printf(" --> Analyzing ride (%d,%d)...", ride_coords.first, ride_coords.second);
    //printf(" %d,", 100*ride_coords.first+ride_coords.second);
    //fflush(stdout);

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

    //printf("%d,%d,%.3f,%d,%d,%d\n",
            //all_events.size(), cust_id_counts.size(), avg_ride_time, min_ride_time, max_ride_time, max_ride_load);

    fprintf(outfile, "%d,%d,%d,%d,%d,%.3f,%d,%d,%d\n", ride_id, ride_coords.first,ride_coords.second,
                all_events.size(), cust_id_counts.size(), avg_ride_time, min_ride_time, max_ride_time, max_ride_load);

    //printf(" Done! # events = %d, # custs = %d, ride_time : avg = %.3f, min = %d, max = %d, max ride load = %d!\n",
      //     all_events.size(), cust_id_counts.size(), avg_ride_time, min_ride_time, max_ride_time, max_ride_load);
}

// analyze the behavior of this customer
void ParkCustomer::analyze(FILE *outfile){

    //printf(" --> Analyzing customer %d...", cust_id);
    //fflush(stdout);

    unsigned int time_ride = 0, time_move = 0;
    unsigned int time_total = (all_events.back().timestamp - all_events.front().timestamp);
    unsigned short int num_rides = 0;

    QString ridesequence;
    for(uint i = 0; i < all_events.size()-1; i++){

        const CustomerEvent &e = all_events[i];
        unsigned int this_time = all_events[i+1].timestamp - e.timestamp;

        if(e.type == CHECKIN){
            time_ride += this_time;
            ridesequence.append(QString::number(e.ride_id));
            ridesequence.append("/");
            num_rides++;
        }

        else
            time_move += this_time;
    }

    ridesequence.chop(1);

    float timeperc_ride = 100.0f * (float)time_ride / (float)time_total;
    float timeperc_move = 100.0f * (float)time_move / (float)time_total;

    fprintf(outfile, "%d,%d,%d,%d,%.3f,%.3f,%s\n", cust_id,
           all_events.size(), num_rides, time_total, timeperc_ride, timeperc_move, ridesequence.toLatin1().data());

    //printf(" Done! # events = %d, # rides = %d, total time = %d, ride_time = %.3f, move_time = %.3f\n",
      //     all_events.size(), num_rides, time_total, timeperc_ride, timeperc_move);

    return;
    if(all_events.back().type == 1){
        printf(" \t last event is a check in -- "); all_events.back().print();
    }
}

void Park::create_ride_map(){

    if(!ridemap.empty())
        return;

    ridemap.insert( pair<coords,int>(coords(0,67),-1));
    ridemap.insert( pair<coords,int>(coords(6,43),20));
    ridemap.insert( pair<coords,int>(coords(16,49),23));
    ridemap.insert( pair<coords,int>(coords(16,66),5));
    ridemap.insert( pair<coords,int>(coords(17,43),7));
    ridemap.insert( pair<coords,int>(coords(17,67),22));
    ridemap.insert( pair<coords,int>(coords(23,54),28));
    ridemap.insert( pair<coords,int>(coords(26,59),25));
    ridemap.insert( pair<coords,int>(coords(27,15),2));
    ridemap.insert( pair<coords,int>(coords(28,66),26));
    ridemap.insert( pair<coords,int>(coords(32,33),32));
    ridemap.insert( pair<coords,int>(coords(34,68),21));
    ridemap.insert( pair<coords,int>(coords(38,90),3));
    ridemap.insert( pair<coords,int>(coords(42,37),-10));
    ridemap.insert( pair<coords,int>(coords(43,56),24));
    ridemap.insert( pair<coords,int>(coords(43,78),31));
    ridemap.insert( pair<coords,int>(coords(45,24),8));
    ridemap.insert( pair<coords,int>(coords(47,11),1));
    ridemap.insert( pair<coords,int>(coords(48,87),27));
    ridemap.insert( pair<coords,int>(coords(50,57),62));
    ridemap.insert( pair<coords,int>(coords(60,37),-11));
    ridemap.insert( pair<coords,int>(coords(63,99),-2));
    ridemap.insert( pair<coords,int>(coords(67,37),-12));
    ridemap.insert( pair<coords,int>(coords(69,44),81));
    ridemap.insert( pair<coords,int>(coords(73,79),11));
    ridemap.insert( pair<coords,int>(coords(73,84),12));
    ridemap.insert( pair<coords,int>(coords(76,22),63));
    ridemap.insert( pair<coords,int>(coords(76,88),14));
    ridemap.insert( pair<coords,int>(coords(78,37),30));
    ridemap.insert( pair<coords,int>(coords(78,48),4));
    ridemap.insert( pair<coords,int>(coords(79,87),13));
    ridemap.insert( pair<coords,int>(coords(79,89),15));
    ridemap.insert( pair<coords,int>(coords(81,77),10));
    ridemap.insert( pair<coords,int>(coords(82,80),16));
    ridemap.insert( pair<coords,int>(coords(83,88),17));
    ridemap.insert( pair<coords,int>(coords(85,86),18));
    ridemap.insert( pair<coords,int>(coords(86,44),6));
    ridemap.insert( pair<coords,int>(coords(87,48),29));
    ridemap.insert( pair<coords,int>(coords(87,63),64));
    ridemap.insert( pair<coords,int>(coords(87,81),19));
    ridemap.insert( pair<coords,int>(coords(92,81),9));
    ridemap.insert( pair<coords,int> (coords(99,77),-3));

    printf(" Created ride map (coords->id). size = %d\n", ridemap.size());
}

