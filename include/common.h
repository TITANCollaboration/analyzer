#ifndef  __COMMON_H__
#define  __COMMON_H__

#include "InfluxDBFactory.h"
#include "Transport.h"
#include "Point.h"
#include <iostream>
//#include "TH1.h"
//#include "TH1F.h"
#include "TFile.h"
//#include <TH1F.h>
using namespace influxdb;

extern int test_root_var;
//extern TFile *root_file;

extern int report_counts(int interval, std::unique_ptr<InfluxDB> &influxdb_conn, std::string daq_prefix, int MAX_CHANNELS, int addr_count[], unsigned int event_count);
int write_pulse_height_event(std::unique_ptr<InfluxDB> &influxdb_conn, int run_number, std::string daq_prefix, int run_num, int daq_chan, int flags, int timestamp, int evadcdata);
int write_pulse_height_event_influxdb(std::unique_ptr<InfluxDB> &influxdb_conn, std::string daq_prefix, int run_num, int daq_chan, int flags, int timestamp, int evadcdata);
int write_pulse_height_event_root(int run_num, int daq_chan, int flags, int timestamp, int evadcdata);

#endif
