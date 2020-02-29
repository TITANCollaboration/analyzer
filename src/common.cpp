
#include "InfluxDBFactory.h"
#include "Transport.h"
#include "Point.h"

#include <iostream>

using namespace influxdb;


int report_counts(int interval, std::unique_ptr<InfluxDB> &influxdb_conn, std::string daq_prefix, int MAX_CHANNELS, int addr_count[])
{
	std::string daq_chan = "";
  int rate_data_exists = 0;
#ifdef USE_INFLUXDB
  std::string point_name = daq_prefix + "_rate";
	influxdb::Point mypoint = influxdb::Point{point_name};
#endif
	for (int i = 0; i <= MAX_CHANNELS; i++) {
		if ( addr_count[i] == 0 ) { continue; }
    rate_data_exists = 1;
		daq_chan = daq_prefix + '_' + std::to_string(i);

#ifdef USE_INFLUXDB
  		mypoint.addField(daq_chan, addr_count[i] / interval);
      std::cout << "Got on event : " << addr_count[i] / interval << "Daq chan : " << daq_chan<< "Point name: " << point_name << '\n';
  #endif

	}
#ifdef USE_INFLUXDB
  try {
    if(rate_data_exists == 1) {
      std::cout << "Going to write to Influx!" << '\n';
	     influxdb_conn->write(std::move(mypoint));
     }
  }
  catch (...)
   {
     std::cout << "A MDPP write exception occurred. Exception Nr. Interval : " << interval  << '\n';
   }
#endif
	//memset(addr_count, 0, sizeof(addr_count) );
	return (0);
}
