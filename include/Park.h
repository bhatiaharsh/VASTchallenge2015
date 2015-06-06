#ifndef _PARK_H
#define _PARK_H

#include <set>
#include <map>
#include <vector>
#include <QString>

#include "time.h"

typedef std::pair<unsigned short int, unsigned short int> coords;

enum EventType {CHECKIN = 1, MOVEMENT = 2};

/// ----------------------------------------------------------------
/// Objects for handling customers and their events!
struct CustomerEvent {

    unsigned short int type;    // event type
    time_t timestamp;
    coords location;

    CustomerEvent(){}
    CustomerEvent(unsigned short int type_, time_t timestamp_, coords location_){
        type = type_;
        timestamp = timestamp_;
        location = location_;
    }

    /*bool operator< (const CustomerEvent &rhs) const{
        return this->timestamp < rhs.timestamp;
    }*/

    void print() const {
        printf(" %s : (%d, %d) -- %d\n", (type==CHECKIN ? "check-in" : "movement"), location.first, location.second, timestamp);
    }
};

class ParkCustomer {
public:
    unsigned long int cust_id;
    std::vector<CustomerEvent> all_events;

    ParkCustomer(unsigned long int cid){
        cust_id = cid;
    }

    // summary
    void analyze();
};

/// ----------------------------------------------------------------
/// Objects for handling rides and their events!
struct RideEvent {

    unsigned long int cust_id;
    time_t time_checkin;
    time_t time_checkout;

    RideEvent(){}
    RideEvent(unsigned long int cust_id_, time_t time_checkin_, time_t time_checkout_){
        cust_id = cust_id_;
        time_checkin = time_checkin_;
        time_checkout = time_checkout_;
    }
    /*bool operator< (const RideEvent &rhs) const{
        return this->time_checkin < rhs.time_checkin;
    }*/

    void print() const {
        printf(" cust %d check-in at %d, time spent = %d\n", cust_id, time_checkin, (time_checkout-time_checkin));
    }
};

struct ParkRide {
    coords ride_coords;
    std::vector<RideEvent> all_events;

    std::map<time_t, int> ride_load;
    int max_ride_load;

    ParkRide(coords loc_){
        ride_coords = loc_;
    }

    void analyze();
    int get_load(time_t time_) const;
};


/// ---------- Park Event (in raw form read from the data file)
struct ParkEvent {

    time_t timestamp;
    coords location;
    unsigned short int eventtype;
    unsigned long int cust_id;

    ParkEvent(QString rline);

    void print() const {
        printf(" time %d, loc (%d,%d), type %s, cust %d\n",
               timestamp, location.first, location.second, (eventtype==1 ? "check-in" : "movement"), cust_id);
    }

};

class Park {

public:
    std::vector<ParkEvent> events;

    std::map<coords, ParkRide> rides;
    std::map<unsigned long int, ParkCustomer> customers;

    time_t time_beg, time_end;

    int max_ride_load;

    void read_events(std::string filename);
    void parse_events();
};

#endif
