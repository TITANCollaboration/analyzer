#ifndef  __COMMON_H__
#define  __COMMON_H__

#include "InfluxDBFactory.h"
#include "Transport.h"
#include "Point.h"
#include <iostream>

#include "TFile.h"
#include "TTree.h"
#include "TNtuple.h"

using namespace influxdb;

typedef struct {Int_t chan, pulse_height, timestamp, flags;} Pulse_Event;

extern TFile *root_file;
extern TNtuple *myntuple;
extern TTree *myttree;
extern Pulse_Event pevent;

int report_counts(int interval, std::unique_ptr<InfluxDB> &influxdb_conn, std::string daq_prefix, int MAX_CHANNELS, int addr_count[], unsigned int event_count);
// int write_pulse_height_event(std::unique_ptr<InfluxDB> &influxdb_conn, std::string daq_prefix, int run_num, int daq_chan, int flags, int timestamp, int evadcdata);
int write_pulse_height_event(std::string daq_prefix, int daq_chan, int flags, int timestamp, int evadcdata);
int write_pulse_height_event_influxdb(std::unique_ptr<InfluxDB> &influxdb_conn, std::string daq_prefix, int daq_chan, int flags, int timestamp, int evadcdata);
int write_pulse_height_event_root(int chan, int flags, int timestamp, int evadcdata);

#endif
