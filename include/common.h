#include "InfluxDBFactory.h"
#include "Transport.h"
#include "Point.h"
#include <iostream>


using namespace influxdb;

int report_counts(int interval, std::unique_ptr<InfluxDB> &influxdb_conn, std::string daq_prefix, int MAX_CHANNELS, int addr_count[]);
