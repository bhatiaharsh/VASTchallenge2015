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

#include "VASTApp.h"
#include "Park.h"

#include <fstream>
/// -----------------------------------------------------------------
using namespace std;

int get_num_lines(std::string filename){

    std::string cmd("wc -l ");
    cmd.append(filename);

    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        printf("Failed to run command %s\n", cmd.c_str());
        return -1;
    }

    int num_lines = -1;
    char cmd_output[100];
    char cmd_remain[100];

    fgets(cmd_output, sizeof(cmd_output), fp);
    sscanf(cmd_output, "%d %s\n", &num_lines, cmd_remain);

    pclose(fp);
    return num_lines;
}

#define WRITE_DATA
void Park::read_events(std::string infilename){

    create_ride_map();
    max_ride_load = 0;

    FILE* input = fopen(infilename.c_str(), "r");
    if(!input){
        printf(" VASTApp::read_events -- could not open file --%s--\n", infilename.c_str());
        return;
    }

#ifdef WRITE_DATA
    string wfilename = infilename;
    uint id = wfilename.find(".csv");
    wfilename.insert(id, "-cleaned");

    FILE* output = fopen(wfilename.c_str(), "w");
    if(!output){
        printf(" VASTApp::read_events -- could not open file for writing --%s--\n", wfilename.c_str());
        return;
    }
#endif

    clock_t st = clock();
    printf(" Reading %s...", infilename.c_str());
    fflush(stdout);

    // count the number of events using wc command!
    int num_lines = get_num_lines(infilename);
    if(num_lines > 0){
        events.reserve(num_lines+1);
    }

    // start reading
    char line[200];
    fgets(line,200,input);      // first row

    //printf(" line = %s\n", line);

    while (!feof(input)){

        if(fgets(line, 200, input) == NULL)
            break;

        //if(events.empty())
          //  printf(" first line = %s\n", line);

        //printf(" line = %s\n", line);

        ParkEvent pe(line);
        if(pe.eventtype != MOVEMENT && pe.eventtype != CHECKIN)
            continue;

        pe.ride_id = -100;
        if(pe.eventtype == CHECKIN){
            pe.ride_id = ridemap.find(pe.location)->second;
        }

        events.push_back(pe);

        // create customers and rides!
        customers.insert(pair<unsigned long int, ParkCustomer>(pe.cust_id, ParkCustomer(pe.cust_id)));
        if(pe.eventtype == CHECKIN)
            rides.insert(pair<int, ParkRide>(pe.ride_id, ParkRide(pe.ride_id, pe.location)));


#ifdef WRITE_DATA
        QString rline(line);
        rline = rline.left(rline.size()-1);
        rline.append(",").append(QString::number(pe.ride_id)).append("\n");
        fputs(rline.toLatin1().data(), output);
#endif

        //if(events.size() == 1000)
          //  break;
    }

    //printf(" last line = %s\n", line);

    time_beg = events.front().timestamp;
    time_end = events.back().timestamp;

    printf(" Done in %.3f sec!\n", ((float)(clock() - st))/CLOCKS_PER_SEC);
    printf("   Read %,d events for %,d simulation seconds! Found %d customers and %d rides!\n",
           events.size(), time_end-time_beg, customers.size(), rides.size());
    printf("\t Start time - %d: %s", time_beg, ctime(&time_beg));
    printf("\t End time   - %d: %s", time_end, ctime(&time_end));
    fclose(input);
#ifdef WRITE_DATA
    fclose(output);
#endif

}


void Park::parse_events(string dayname){

    clock_t st = clock();
    printf(" Parsing events...");
    fflush(stdout);

    // these events are ordered in time!
    uint num_events = events.size();
    for(uint i = 0; i < num_events; i++){

        const ParkEvent &currevent = events[i];

        CustomerEvent custevent(currevent.eventtype, currevent.timestamp, currevent.ride_id, currevent.location);
        customers.find(currevent.cust_id)->second.all_events.push_back(custevent);
    }

    // now go through the customers and create ride events
    // i cannot use the original event list for this, since I need to know
    // how long a customer takes on each ride. that information is readily
    // available in the customer list

    std::map<unsigned long int, ParkCustomer>::const_iterator citer = customers.begin();
    for(; citer != customers.end(); citer++){

        const ParkCustomer &c = citer->second;
        uint num_events = c.all_events.size();
        for(uint i = 0; i < num_events; i++){

            const CustomerEvent &custevent = c.all_events[i];
            if(custevent.type == MOVEMENT)
                continue;

            unsigned int time_chkout = (i == c.all_events.size()-1) ? custevent.timestamp : c.all_events[i+1].timestamp;
            RideEvent rideevent (c.cust_id, custevent.timestamp, time_chkout);
            rides.find(custevent.ride_id)->second.all_events.push_back(rideevent);
        }
    }

    printf(" Done in %.3f sec!\n", ((float)(clock() - st))/CLOCKS_PER_SEC);

    {
        std::string rfilename = "rides-";
        rfilename.append(dayname);
        rfilename.append(".csv");

        FILE *outfile = fopen(rfilename.c_str(), "w");
        fprintf(outfile, "ride_id,X,Y,num_events,num_custs,avg_time,min_time,max_time,max_load\n");

        std::map<int, ParkRide>::iterator riter = rides.begin();
        for(; riter != rides.end(); riter++){

            ParkRide &ride = riter->second;
            ride.analyze(outfile);

            if(ride.max_ride_load > max_ride_load)
                max_ride_load = ride.max_ride_load;
            //printf(" ride (%d,%d) has %d events!\n", riter->first.first, riter->first.second, riter->second.all_events.size());
        }

        fclose(outfile);

        std::string cfilename = "customers-";
        cfilename.append(dayname);
        cfilename.append(".csv");

        outfile = 0;
        outfile = fopen(cfilename.c_str(), "w");
        fprintf(outfile, "cust_id,num_events,num_rides,total_time,ride_time,move_time\n");

        std::map<unsigned long int, ParkCustomer>::iterator citer = customers.begin();
        for(; citer != customers.end(); citer++){

            ParkCustomer &cust = citer->second;
            cust.analyze(outfile);
            //printf(" customer %d has %d events!\n", citer->first, citer->second.positions.size());
        }

        fclose(outfile);
    }

    //printf(" max ride load = %d\n", max_ride_load);
    //customers.find(2038469)->second.analyze();
    //exit(1);
}

VASTApp::VASTApp(std::string filename){

    // ----------- initialize ui and viewer

    ui.setupUi(this);
    viewer = new VASTViewer(this);
    setCentralWidget(viewer);
    setWindowTitle("VAST Challenge Viewer");

    aPark.read_events(filename);

    std::string dayname = (filename.find("Fri") != -1) ? "Fri" :
                          (filename.find("Sat") != -1) ? "Sat" :
                          (filename.find("Sun") != -1) ? "Sun" : "Unknown";

    aPark.parse_events(dayname);

    show();
}


/// ===========================================================
/// main function
/// ===========================================================

int main(int argc, char** argv){

    if(argc != 2){
        printf(" Usage: %s <movement_datafile>\n", argv[0]);
        exit(1);
    }



    // Read command lines arguments.
    QApplication application(argc,argv);

    VASTApp atApp(argv[1]);

    // Run main loop.
    return application.exec();
}


