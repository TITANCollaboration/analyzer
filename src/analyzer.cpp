#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "common.h"
#include "midas.h"
#include "web_server.h"

/*#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"*/
#undef USE_INFLUXDB
extern ANA_MODULE griffin_module;
extern ANA_MODULE mdpp16_module;

// Global variables to write root files
//TFile *root_file;
//TTree *myttree;
//TNtuple *myntuple;

#ifdef USE_INFLUXDB
//Influx DB stuff
//static std::string influxdb_hostname = "titan05.triumf.ca";
//static std::string influxdb_port = "8086";
//static std::string influxdb_dbname = "titan";
//std::string influx_connection_string = "http://" + influxdb_hostname + ":" + influxdb_port + "/?db=" + influxdb_dbname;
//std::unique_ptr<InfluxDB> influxdb_conn = influxdb::InfluxDBFactory::Get(influx_connection_string);
#endif


// Path to where we will write the root file
//std::string path_to_root_file = "/home/ebit/daq/analyzer_root_files/";

std::string root_file_name;
char *analyzer_name = "Analyzer"; /* The analyzer name (client name)   */
INT analyzer_loop_period = 0; /* analyzer_loop call interval[ms](0:disable) */
INT odb_size = DEFAULT_ODB_SIZE; /* default ODB size */

//ANA_MODULE *trigger_module[] = { &griffin_module, NULL };
ANA_MODULE *griffin_trigger_module[] = { &griffin_module, NULL };
ANA_MODULE *mdpp_trigger_module[] = { &mdpp16_module, NULL };

BANK_LIST griffin_ana_trigger_bank_list[] = { /* online banks */
        {"GRF4", TID_DWORD, 256, NULL}, {""},
};
BANK_LIST mdpp_ana_trigger_bank_list[] = { /* online banks */
        {"MDPP", TID_DWORD, 256, NULL}, {""},
};
//BANK_LIST ana_trigger_bank_list[] = { /* online banks */
//  {"GRF3", TID_DWORD, 256, NULL}, {""} , {"GRF4", TID_DWORD, 256, NULL}, {"MDPP", TID_DWORD, 256, NULL}, {""}
//};

ANALYZE_REQUEST analyze_request[] = {
        {"GRIF16",               /* equipment name */
         {200,                    /* event ID */
       TRIGGER_ALL,             /* trigger mask */
       //GET_SOME,                  /* get some events */ /* Removed ?? */
       GET_NONBLOCKING,         /* get some events */ /* later midas's */
       "SYSTEM",                /* event buffer */
       TRUE,                    /* enabled */
       "", "",},
         NULL,                  /* analyzer routine */
         griffin_trigger_module,        /* module list */
         griffin_ana_trigger_bank_list, /* bank list */
         1000,                  /* RWNT buffer size */
         TRUE,                  /* Use tests for this event */
        },

        {"MDPP16",              /* equipment name */
         {400,                    /* event ID */
      TRIGGER_ALL,              /* trigger mask */
      //GET_SOME,                  /* get some events */ /* Removed ?? */
      GET_NONBLOCKING,          /* get some events */ /* later midas's */
      "SYSTEM",                 /* event buffer */
      TRUE,                     /* enabled */
      "", "",},
         NULL,                  /* analyzer routine */
         mdpp_trigger_module,        /* module list */
         mdpp_ana_trigger_bank_list, /* bank list */
         1000,                  /* RWNT buffer size */
         TRUE,                  /* Use tests for this event */
        },

        {""}
};

/*--------------------------------------------------------------------------*/

volatile int shutdown_webserver;
static pthread_t web_thread;
INT analyzer_init(){
        // Create new thread which launches the Web server

        int a1=1;
        pthread_create(&web_thread, NULL,(void* (*)(void*))web_server_main, &a1);

        return SUCCESS;
}

INT analyzer_exit(){
        printf("waiting for server to shutdown ...\n");
        shutdown_webserver = 1;
        pthread_join(web_thread, NULL);
        return CM_SUCCESS;
}

INT ana_begin_of_run(INT run_number, char *error){
  printf("Starting Run: %i", run_number);
  //root_file_name = path_to_root_file + std::to_string(run_number) + ".root";
  //root_file = new TFile(root_file_name.c_str(),"NEW"); // Lets create our root file to write to
  //myntuple = new TNtuple("EVENT_NTUPLE","NTuple of events", "chan:pulse_height:timestamp:flags");

  #ifdef USE_INFLUXDB
//  influxdb_conn->batchOf(1000);
  #endif
  return CM_SUCCESS;
}
INT ana_end_of_run(INT run_number, char *error){
  //root_file->Write();
  //root_file->Close();
  return CM_SUCCESS;
}
INT ana_pause_run(INT run_number, char *error){
        return CM_SUCCESS;
}
INT ana_resume_run(INT run_number, char *error){
        return CM_SUCCESS;
}
INT analyzer_loop(){
        return CM_SUCCESS;
}
