#include <stdio.h>
#include <stdlib.h> /* rand() */
#include <time.h>
#include <unistd.h> /* usleep */
#include <math.h>
#include <string.h>  /* memset */
#include <stdint.h>
#include <iostream>
#include <random>
#include <fstream>
#include <math.h>
using namespace std;

#include "midas.h"
#include "histogram.h"
#include "histogram2.h"
#include "web_server.h"
#include "experim.h"

//Root Stuff
/*
#include "TH1.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TFile.h"
#include "TROOT.h"
*/

#include "common.h"


#ifdef USE_INFLUXDB
//#include "InfluxDBFactory.h"
//#include "Transport.h"
//#include "Point.h"


//influxdb_conn_mdpp->enableBuffering(100);
#endif

#define INTEGRATION_LENGTH  8

#define NUM_ODB_CHAN     16 // size of msc table in odb
#define MAX_SAMPLE_LEN  4096
//#define ENERGY_BINS    65536/* 65536 131072 262144 */
#define ENERGY_BINS     32768 // 8192/* 65536 131072 262144 */
#define TEST_BINS		64
#define TEST_2d_BINS	400
#define NUM_CLOVER        16
//#define MAX_CHAN        1024
#define MAX_CHAN        16

#define STRING_LEN       256
#define MIDAS_STRLEN      32
#define MAX_ADDRESS  0x10000
#define E_SPEC_LENGTH   8192
#define T_SPEC_LENGTH   8192
#define WV_SPEC_LENGTH  4096
#define N_HITPAT 5
#define N_SUM 5
#define RATE_COUNT_INTERVAL 1 // This is in seconds, controls how often the per channel rate data is updated

static short waveform[MAX_SAMPLE_LEN];
static int rate_data[MAX_CHAN];
//static float          gains[MAX_CHAN];
//static float        offsets[MAX_CHAN];
static char chan_name[MAX_CHAN][MIDAS_STRLEN];
static short chan_address[MAX_CHAN];
static int num_chanhist;
static short address_chan[MAX_ADDRESS];
static int addr_count_mdpp[MAX_CHAN + 1];// = {0};  // this will be used to calculate rates per channel
time_t last_update_mdpp = time(NULL);
unsigned int mdpp_event_count = 0;

float mdpp16_gains[NUM_ODB_CHAN];
float mdpp16_offsets[NUM_ODB_CHAN];

static int debug; // only accessible through gdb
//TH1IHist *hit_hist[N_HITPAT];

/*-- Module declaration --------------------------------------------*/
int mdpp16_event(EVENT_HEADER *, void *);
int mdpp16_init(void);
int mdpp16_bor(INT run_number);
int mdpp16_eor(INT run_number);
static void odb_callback(INT hDB, INT hseq, void *info) {
}

ANA_MODULE mdpp16_module = {
	"mdpp16",             /* module name           */
	"CP",                 /* author                */
	mdpp16_event,        /* event routine         */
	mdpp16_bor,          /* BOR routine           */
	mdpp16_eor,          /* EOR routine           */
	mdpp16_init,         /* init routine          */
	NULL,                 /* exit routine          */
	NULL, //&mdpp16_settings,         /* parameter structure   */
	0,  //sizeof(mdpp16_settings),  /* structure size        */
	NULL, //mdpp16_settings_str,      /* initial parameters    */
};
/*-------------------------------------------------------------------*/

#define NUM_CHAN    16
#define HIT_CHAN    64
#define ADC_CHAN    65536

// Declare histograms here. TH1F = 4 bytes/cell (float), TH1D = 8 bytes/cell (double)
//static TH1F *hHitPat, *hEnergy[NUM_CHAN], *hEnergy_flagsrm[NUM_CHAN], *hTDC[NUM_CHAN], *hRate[NUM_CHAN];
//static TH1F *hCalEnergy1[NUM_CHAN];
//static TH2F *hEnergy_vs_ts[NUM_CHAN];


extern TH1IHist **hit_hist;
extern TH1IHist **sum_hist;
//extern TH1IHist *ph_hist;
//extern TH1IHist **e_hist;
//extern TH1IHist **cfd_hist;
extern TH1IHist **wave_hist;

TH1IHist *ph_hist_mdpp[MAX_CHAN];
TH1IHist *e_hist_mdpp[MAX_CHAN];
TH1IHist *cfd_hist_mdpp[MAX_CHAN];
void read_mdpp16_odb_gains();
int hist_init_roody();
int hist_mdpp_init();
//std::ofstream mdpp_word_file("mdpp_word_output.txt");
//std::ofstream mdpp_hit_file("mdpp_hit_output.txt");

//---------------------------------------------------------------------

int mdpp16_bor(INT run_number) {
  // JONR : Put code here eventually to tag the database with the run number
	return SUCCESS;
}
int mdpp16_eor(INT run_number) {
	return SUCCESS;
}

extern HNDLE hDB;                     /* Odb Handle */
MDPP16_ANALYSER_PARAMETERS ana_param;                    /* Odb settings */
MDPP16_ANALYSER_PARAMETERS_STR(ana_param_str);     /* Book Setting space */


void read_mdpp16_odb_gains()
{
	int status, size;
	char tmp[STRING_LEN];
	HNDLE hKey;

	// readgains and offsets from ODB
	sprintf(tmp,"/DAQ/MSC/mdpp16_gain");
	if( (status=db_find_key(hDB, 0, tmp, &hKey)) != DB_SUCCESS) {
		cm_msg(MINFO,"FE","Key %s not found", tmp); return;
	}
	size=sizeof(mdpp16_gains);
	if( (db_get_data(hDB,hKey,&mdpp16_gains,&size,TID_FLOAT)) != DB_SUCCESS) {
		cm_msg(MINFO,"FE","Can't get data for Key %s", tmp); return;
	}
	sprintf(tmp,"/DAQ/MSC/mdpp16_offset");
	if( (status=db_find_key(hDB, 0, tmp, &hKey)) != DB_SUCCESS) {
		cm_msg(MINFO,"FE","Key %s not found", tmp); return;
	}
	size=sizeof(mdpp16_offsets);
	if( (db_get_data(hDB,hKey,&mdpp16_offsets,&size,TID_FLOAT)) != DB_SUCCESS) {
		cm_msg(MINFO,"FE","Can't get data for Key %s", tmp); return;
	}
}


int mdpp16_init(void)
{
	char odb_str[128], errmsg[128];
	char set_str[80];
	int size, status;
	HNDLE hSet;

  read_mdpp16_odb_gains();     // Print the loaded gains and offsets
  fprintf(stdout,"\nMDPP16: Read Gain/Offset values from ODB\nIndex\tGain\tOffset\n");

  for(int i=0; i<NUM_ODB_CHAN; i++) {
		fprintf(stdout,"%d\t%f\t%f\n",i,mdpp16_gains[i],mdpp16_offsets[i]);
	}
	printf("Were init'ing the crap out of the MDPP16...\n");
	size = sizeof(MDPP16_ANALYSER_PARAMETERS);

	sprintf(set_str, "/mdpp16_analyser/Parameters");
	status = db_create_record(hDB, 0, set_str, strcomb(ana_param_str));
	if ( (status = db_find_key (hDB, 0, set_str, &hSet)) != DB_SUCCESS ) {
		cm_msg(MINFO, "FE", "Key %s not found", set_str);
	}
	if ((status = db_open_record(hDB, hSet, &ana_param, size, /* enable link */
	                             MODE_READ, NULL, NULL)) != DB_SUCCESS ) {
		cm_msg(MINFO, "FE", "Failed to enable ana param hotlink", set_str);

	}

	hist_mdpp_init();
//  influxdb_conn_mdpp->batchOf(1000);
	return SUCCESS;
}

int report_counts_mdpp(int interval)
{
  report_counts(interval, "mdpp16", MAX_CHAN, addr_count_mdpp, mdpp_event_count);
	memset(addr_count_mdpp, 0, sizeof(addr_count_mdpp) );
	return (0);
}

int hist_mdpp_init()
{
	char hit_titles[N_HITPAT][32] = {
		"HITPATTERN_Energy",   "HITPATTERN_Time",
		"HITPATTERN_Waveform", "HITPATTERN_Pulse_Height", "HITPATTERN_Rate"
	};
	char sum_titles[N_SUM][32] = { "SUM_Singles_Low_gain_Energy", "SUM_Singles_High_gain_Energy", "SUM_Addback_Energy" };
	char hit_names[N_HITPAT][32] = {"e_hit", "t_hit", "w_hit", "q_hit", "r_hit"};
	char sum_names[N_SUM][32] = {"el_sum", "eh_sum", "a_sum", "p_sum", "l_sum"};
	char title[STRING_LEN], handle[STRING_LEN];
	int i;

	// Adding the labels for the dummy 1d data
	TH1IHist *test_hist; 
	test_hist = H1_BOOK("test_handle", "test_chan", TEST_BINS, 0, TEST_BINS);

	// Generating random numbers for the test histogram
	// Create a random number generator engine
    std::random_device rd;  // Initialize with a hardware entropy source if available
    std::mt19937 mt(rd());  // Mersenne Twister pseudo-random number generator

    // Define a distribution (e.g., for integers between 1 and 50)
    std::uniform_int_distribution<int> dist(1, 50);

	for(int i=0; i<1000; i++) {
		int random_number = dist(mt); // Generate the random number
    	test_hist->Fill(test_hist, random_number, 1);
	}

	// Adding the labels for the dummy 2d data
	TH2IHist *test_2d_hist;
	int rowl = 20;
	test_2d_hist = H2_BOOK("test_2d_handle", "test_2d_chan", TEST_2d_BINS, rowl, TEST_2d_BINS);
	
	for(int i=0; i<1000; i++) {
	        int rand_x = rand() % rowl; // Generate the random number for x axis
		int rand_y = rand() % (TEST_2d_BINS / rowl); // Generate the random number for y axis
		test_2d_hist->Fill(test_2d_hist, rand_x, rand_y, 1);
        }	

	for (i = 0; i < MAX_CHAN; i++) { // Create each histogram for this channel

		sprintf(chan_name[i], "mdpp16_%i", i);
		printf("%d = %d[0x%08x]: %s\n", i, chan_address[i], chan_address[i], chan_name[i]);
		sprintf(title,  "%s_Pulse_Height",   chan_name[i] );
		sprintf(handle, "%s_Q",              chan_name[i] );
		ph_hist_mdpp[i] = H1_BOOK(handle, title, ENERGY_BINS, 0, ENERGY_BINS);

//    ph_hist[i] = H1_BOOK(handle, title, E_SPEC_LENGTH, 0, E_SPEC_LENGTH);
		sprintf(title,  "%s_Energy",         chan_name[i] );
		sprintf(handle, "%s_E",              chan_name[i] );
		e_hist_mdpp[i] = H1_BOOK(handle, title, E_SPEC_LENGTH, 0, E_SPEC_LENGTH);

		sprintf(title,  "%s_Time",           chan_name[i] );
		sprintf(handle, "%s_T",              chan_name[i] );
		cfd_hist_mdpp[i] = H1_BOOK(handle, title, T_SPEC_LENGTH, 0, T_SPEC_LENGTH);
		/* JONR

		   sprintf(title,  "%s_Waveform",       chan_name[i] );
		   sprintf(handle, "%s_w",              chan_name[i] );
		   wave_hist[i] = H1_BOOK(handle, title, WV_SPEC_LENGTH, 0, WV_SPEC_LENGTH);
		   }
		   for (i = 0; i < N_HITPAT; i++) { // Create Hitpattern spectra
		   hit_hist[i] = H1_BOOK(hit_names[i], hit_titles[i], MAX_CHAN, 0, MAX_CHAN);
		   }
		   for (i = 0; i < N_SUM; i++) { // Create Sum spectra
		   sum_hist[i] = H1_BOOK(sum_names[i], sum_titles[i], E_SPEC_LENGTH, 0, E_SPEC_LENGTH);
		   } JONR END*/
	}
	return (0);
}

int mdpp16_event(EVENT_HEADER *pheader, void *pevent)
{
	/* BeginTime needs to be global? startTime should be set to 0 at the beginning of each event? and then
	   something something...  */
	int i, ecal = 0, bank_len, err = 0;
	DWORD *data;
	int hsig, subhead = 0, mod_id, tdc_res, adc_res, nword;
	int dsig = 0, flags = 0, t, chan = 100, evdata;
	uint32_t ts = 0, extts = 0; // needed for 30-bit ts
  uint64_t full_ts = 0;
	int esig, counter;
	uint64_t evadcdata = 0, evtdcdata, evrstdata, trigchan;
	static int evcount = 0;

	/* Added these to give a time interval to accrue counts. Current bugs:
	   - drop in count rate at regular intervals. For INTERVAL=5, counts drop every 5 bins... odd
	       DOES depend on interval and can start in the middle of the 5 bin set
	   - does not reset to 0 between runs, requires analyzer be reset.
	 */
	static time_t startTime, beginTime;
	//time_t currentTime = time(NULL);
	static int rates[NUM_CHAN];
	float cal_energy;
	time_t curr_time = time(NULL); // Get current unix time
	if ( (curr_time - last_update_mdpp) >= RATE_COUNT_INTERVAL ) {
		report_counts_mdpp(curr_time - last_update_mdpp);
		last_update_mdpp = curr_time;
	}
	if ( (bank_len = bk_locate(pevent, "MDPP", &data) ) == 0 ) { return (0); }
	++evcount;
	debug = 0;
	if ( debug ) {
		printf("Event Dump ...\n");
		for (i = 0; i < bank_len; i += 4) {
			printf("   word[%d] = 0x%08x    0x%08x    ", i, data[i], data[i + 1]);
			printf("0x%08x    0x%08x    \n", data[i + 2], data[i + 3] );
		}
	}

	// hsig 2 + subhead 2 + xxxx + mod_id 8 + tdc_res 3 + adc_res 3 + num of following 4byte words incl EOE 10
	hsig    = (data[0] >> 30) & 0x3;
	subhead = (data[0] >> 24) & 0x3F;
	mod_id  = (data[0] >> 16) & 0xFF;
	tdc_res = (data[0] >> 13) & 0x7;
	adc_res = (data[0] >> 10) & 0x7;
	nword   = (data[0] >> 0) & 0x3FF;

	if ( debug ) {
		printf("Header ...\n");
		printf("   hsig  =%d    subheader=%d    mod_id=%d\n", hsig, subhead, mod_id);
		printf("   tdc_res=%d    adc_res=%d    nword=%d\n", tdc_res, adc_res, nword);
	}
  //printf("------------ Bank Len %i\n", bank_len);
	for (i = 0; i < bank_len; i++) { // covers both ADC and TDC event words
    evadcdata = 0;
/*    if(i == 0) {
      mdpp_word_file << data[i];
    } else {
      mdpp_word_file << "," << data[i];
    }*/
		/* Highest 4 bits:
		      0100 for header
		      0001 for data event
		      0010 for ext ts
		      11   for EOE mark (30-bit ts)
		      0000 for fill dummy */
		dsig    = (data[i] >> 30) & 0x3;
    subhead = (data[i] >> 28) & 0x3;
		// DATA word for TDC, ADC or reset event. 0b0001
    if((data[i] == 4294967295) || (data[i] == 0)) {
      continue;
    }
    if ( dsig == 3 ) {
  			ts =  ((data[i] >> 0) & 0x3FFFFFFF);// concatenate 14 bits and 16 bits...
        continue;
  	}
		if ( subhead == 1 ) {

			flags    = (data[i] >> 22) & 0x3;// pileup or overflow/underflow
			trigchan = (data[i] >> 16) & 0x3F; // All 6 bits for determining ADC, TDC or trig0/trig1 (reset)
      evadcdata = 0;

			if ( trigchan < MAX_CHAN ) { // ADC value caught
				chan      = (data[i] >> 16) & 0x3F;
				evadcdata = (data[i] >> 0 ) & 0xFFFF;
				++addr_count_mdpp[chan];

        if (flags == 0 && evadcdata <= ADC_CHAN && chan < MAX_CHAN) {
            //printf("Adding entry for energy hit %i on channel : %i\n", evadcdata, chan);

            evadcdata = evadcdata/2; //note to self: here is the online analyzer's factor of 2 bin size.
            ecal   = evadcdata * mdpp16_gains[chan] + mdpp16_offsets[chan];

            ph_hist_mdpp[chan]->Fill(ph_hist_mdpp[chan],  (int)evadcdata,     1);
            e_hist_mdpp[chan]->Fill(e_hist_mdpp[chan],  (int)ecal,     1);
            //mdpp_hit_file << trigchan << "," << chan << "," << flags << "," << (int)evadcdata << "\n";

            mdpp_event_count = mdpp_event_count + 1;
            #ifdef USE_REDIS
              write_pulse_height_event("mdpp16", chan, flags, 0, evadcdata); //, mdpp16_temporal_hist);
            #endif
        };
			} else {
        continue;
      }
		// Extended timestamp word. If dsig+fix==0010, ext ts caught
    }
    else if ( subhead == 2 ) {
      extts = (data[i] >> 0) & 0xFFFF;
      continue;
	  }
	} // End of for loop
  //mdpp_word_file << "\n";

  mdpp16_tdc_last_time = ts + (extts * pow(2, 30));
  //cout << "TDC Timestamp low: " << ts << "high: " << extts <<  "- Full: " << mdpp16_tdc_last_time << "\n";
	esig    = (data[i] >> 30) & 0x3;
	counter = (data[i] >> 0) & 0x3FFFFFFF; // low 30bits
	if ( debug ) {
		printf("Trailer ...\n");
		printf("   esig  =%d    counter=%d\n", esig, counter);
		printf("\n");
	}

	if ( hsig != 1 || esig != 3 || t != 0 || subhead != 0 || mod_id != 1 ) {
		err = 1;
	}
	return (0);
}
