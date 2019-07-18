/******************************************************************************
 *       Simple DESCANT analyser
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h> /* rand() */
#include <time.h>
#include <unistd.h> /* usleep */
#include <math.h>
#include <cstring> /* memset */
#include <stdint.h>

#include "midas.h"
#include "web_server.h"

int descant(EVENT_HEADER *, void *);
int descant_init(void);
int descant_bor(INT run_number);
int descant_eor(INT run_number);
void load_gains_from_ODB();

ANA_MODULE descant_module = {
	"Descant",              /* module name           */
	"UrMom",                /* author                */
	descant,                /* event routine         */
	descant_bor,            /* BOR routine           */
	descant_eor,            /* EOR routine           */
	descant_init,           /* init routine          */
	NULL,                   /* exit routine          */
	NULL,                   /* parameter structure   */
	0,                      /* structure size        */
	NULL,                   /* initial parameters    */
};

/*-- module-local variables ----------------------------------------*/

#define MAX_SAMPLE_LEN  4096
#define ENERGY_BINS    65536 /* 65536 131072 262144 */
//#define NUM_CLOVER         16
#define NUM_CHAN         NSPECS
#define HITPAT_SIZE ((int)((NUM_CHAN+31)/32)) /* size in int32 */
#define RESET_SECONDS 10

typedef struct caen_event_struct {
	int                     trigger_num;  int     chan_hitpattern[HITPAT_SIZE];
	int  energy_hitpattern[HITPAT_SIZE];  int      cfd_hitpattern[HITPAT_SIZE];
	int  tstamp_hitpattern[HITPAT_SIZE];  int tstamphi_hitpattern[HITPAT_SIZE];
	int trigReq_hitpattern[HITPAT_SIZE];  int trig_acc_hitpattern[HITPAT_SIZE];
	int  pileup_hitpattern[HITPAT_SIZE];  int livetime_hitpattern[HITPAT_SIZE];
	short     waveform_length[NUM_CHAN];  int                 energy[NUM_CHAN];
	int                   cfd[NUM_CHAN];  int              timestamp[NUM_CHAN];
	int          timestamp_up[NUM_CHAN];  int               trig_req[NUM_CHAN];
	int              trig_acc[NUM_CHAN];  int               livetime[NUM_CHAN];
	int                         address;  int                 kvalue[NUM_CHAN];
	int ppg_pattern; int master_pattern;  int                     zc[NUM_CHAN];
	unsigned master_id; unsigned net_id;  int                cc_long[NUM_CHAN];
	int               cc_short[NUM_CHAN];
} Caen_event;

extern int rate_data[SIZE_OF_ODB_MSC_TABLE];

static Caen_event caen_event;
static Caen_event* event = &caen_event;
static short waveform[NUM_CHAN][MAX_SAMPLE_LEN];

extern HNDLE hDB; // Odb Handle

extern float gains[SIZE_OF_ODB_MSC_TABLE];
extern float offsets[SIZE_OF_ODB_MSC_TABLE];

// Declaration of the spectrum store
extern int spec_store_address[NSPECS];
extern int spec_store_type[NSPECS]; // 0=energy, 1=time, 2=waveform, 3=hitpattern
extern char spec_store_Ename[NSPECS][32];
extern char spec_store_Pname[NSPECS][32];
extern char spec_store_Wname[NSPECS][32];
extern char spec_store_Tname[NSPECS][32];
extern char spec_store_Zname[NSPECS][32]; // zero-crossing
extern char spec_store_Lname[NSPECS][32]; // cc_long
extern char spec_store_Sname[NSPECS][32]; // cc_short
extern int spec_store_Edata[NSPECS][SPEC_LENGTH];
extern int spec_store_Tdata[NSPECS][SPEC_LENGTH];
extern int spec_store_Wdata[NSPECS][SPEC_LENGTH];
extern int spec_store_Pdata[NSPECS][SPEC_LENGTH];
extern int spec_store_Zdata[NSPECS][SPEC_LENGTH]; // zero-crossing
extern int spec_store_Ldata[NSPECS][SPEC_LENGTH]; // cc_long
extern int spec_store_Sdata[NSPECS][SPEC_LENGTH]; // cc_short
extern int spec_store_hit_type[NHITS];
extern char spec_store_hit_name[NHITS][32];
extern int spec_store_hit_data[NHITS][SIZE_OF_ODB_MSC_TABLE];
extern char spec_store_sum_name[NHITS][32];
extern int spec_store_sum_data[NHITS][SPEC_LENGTH];

typedef struct hitpat_specinfo_struct {
	int bins; int offset; char handle[16]; char title[64];
} Hitpat_specinfo;

extern void load_gains_from_ODB();
extern void load_spec_table_from_ODB();

int descant_init(void){ load_gains_from_ODB(); load_spec_table_from_ODB(); return SUCCESS; }
int descant_eor(INT run_number)
{ 
	return SUCCESS;
}

extern float spread(int val);

extern void dump_event(unsigned int *data, int len);

static time_t bor_time=-1, start_time=-1;
static time_t last_reset;
static unsigned start_pkt;
int descant_bor(INT run_number)
{

	bor_time = time(NULL); start_time = -1;
	load_gains_from_ODB();

	// Zero the web spectra
	memset(spec_store_hit_data,0,sizeof(spec_store_hit_data));
	memset(spec_store_sum_data,0,sizeof(spec_store_sum_data));
	memset(spec_store_Edata,0,sizeof(spec_store_Edata));
	memset(spec_store_Pdata,0,sizeof(spec_store_Pdata));
	memset(spec_store_Tdata,0,sizeof(spec_store_Tdata));
	memset(spec_store_Wdata,0,sizeof(spec_store_Wdata));
	memset(spec_store_Zdata,0,sizeof(spec_store_Zdata));
	memset(spec_store_Ldata,0,sizeof(spec_store_Ldata));
	memset(spec_store_Sdata,0,sizeof(spec_store_Sdata));
	memset(rate_data,0,sizeof(rate_data));

	printf("Success!\n");
	return SUCCESS;
}

int process_decoded_event(Caen_event *ptr)
{
	//printf("inside process_decoded_event\n");
	int i, j, web_chan, ECal;
	time_t current_time = time(NULL);
	short sample;

	if( bor_time == -1 ){ bor_time = current_time; }
	if( start_time == -1 ){
		start_time=current_time; start_pkt=ptr->net_id-1;
	}

	if(difftime(current_time, last_reset) > RESET_SECONDS) {
		for(i = 0; i < SIZE_OF_ODB_MSC_TABLE; ++i) {
			spec_store_hit_data[4][i] = rate_data[i];
		}
		memset(rate_data,0,sizeof(rate_data));
		last_reset = current_time;
	}

	//   printf("address: %x\n",ptr->address);
	//   printf("web_chan: %i\n",GetIDfromAddress(ptr->address));
	for(i=0; i<NUM_CHAN; i++){
		// Fill the web spectra
		web_chan = GetIDfromAddress(ptr->address);
		if(web_chan>=0 && web_chan<NUM_CHAN){
			if(ptr->cfd[i]/256>5 && ptr->cfd[i]/256<SPEC_LENGTH){ spec_store_Tdata[web_chan][ptr->cfd[i]/256]++; spec_store_hit_data[1][web_chan]++;}
			if(ptr->kvalue[i]!=0){
				if((ptr->energy[i]/ptr->kvalue[i])>0 && (ptr->energy[i]/ptr->kvalue[i])<SPEC_LENGTH) {
					spec_store_Pdata[web_chan][ptr->energy[i]/ptr->kvalue[i]]++; spec_store_hit_data[3][web_chan]++;
				}
			}

			if(ptr->kvalue[i]!=0) {
				ECal = (int)(spread( ((float)ptr->energy[i])/ptr->kvalue[i] ) * gains[i] + offsets[i]);
				if(ECal>3 && ECal<SPEC_LENGTH){
					spec_store_Edata[web_chan][ECal]++;
					spec_store_hit_data[0][web_chan]++;
					rate_data[web_chan]++;
					if(spec_store_address[i]<0x400){   spec_store_sum_data[0][ECal]++; } // Make the sum just of HPGe
					////build timestamp from 14 high bits and 28 low bits
					//unsigned long ts = ptr->timestamp_up[i];
					//ts = (ts << 28) | ptr->timestamp[i];
					//if(ts != 0) {
					//   //calculate time difference between this event and the NUM_RATE_EVENTSth previous event
					//   long tdiff = ts - stored_ts[i][counter[i]%NUM_RATE_EVENTS];
					//   printf("rate:  current ts %ld, stored_ts[%d][%d] %ld => tdiff %ld",ts,i,counter[i]%NUM_RATE_EVENTS,stored_ts[i][counter[i]%NUM_RATE_EVENTS],tdiff);
					//   //save the current timestamp and advance counter of this channel
					//   stored_ts[i][counter[i]%NUM_RATE_EVENTS] = ts;
					//   counter[i]++;
					//   if(tdiff > 0) {
					//      printf(", webchan %d, old rate %d, new rate %f\n", web_chan, spec_store_hit_data[4][web_chan], (NUM_RATE_EVENTS*1e8)/tdiff);
					//      spec_store_hit_data[4][web_chan] = (NUM_RATE_EVENTS*1e8)/tdiff;
					//   } else {
					//      printf("\n");
					//   }
					//}
				}
			}

			if(strlen(spec_store_Zname[i])>0) {// if a channel is DESCANT...
				if (ptr->zc[i]!=0 && ptr->zc[i]<SPEC_LENGTH) {spec_store_Zdata[web_chan][ptr->zc[i]]++;}
				if (ptr->cc_long[i]!=0 && ptr->cc_long[i]<SPEC_LENGTH) {spec_store_Ldata[web_chan][ptr->cc_long[i]]++;}
				if (ptr->cc_short[i]!=0 && ptr->cc_short[i]<SPEC_LENGTH) {spec_store_Sdata[web_chan][ptr->cc_short[i]]++;}
			}
		}


		if( ptr->waveform_length[i] == 0 ){ continue; }
		spec_store_hit_data[2][web_chan]++;
		for(j=0; j<ptr->waveform_length[i] && j<MAX_SAMPLE_LEN; j++){
			sample = waveform[i][j]; /* Need to extend sign bit14 -> 16 bits */
			if( sample & (1<<13) ){ sample |= 0x3 << 14; } 
			if(web_chan>=0 && web_chan<256){ spec_store_Wdata[web_chan][j] = (int)(sample+8192); }
		}
	}

	//analysed_spectra(ptr);

	return SUCCESS;
}

//  to 2015jun14:     0x81240001 0x0bbd0000 0x01d00000 0x0cad0000 0x90000000
//     0xa0000000 0xb0000000 0xc000020e 0x50f68138 0x4f3f3bd5 0xe0000000 
//  after 2015jun14   0x81240011 0x0bbd0000 0x01d00000 0x0cad0000 0x90000241
//     0xaf6ba2af 0xc00d8036 0xb0960021 0x400cd53a 0x036ba2da 0xe0000241 
// after 2015June30 multiple DESCANT fragments per MIDAS event
int decode_descant_event(uint32_t* data, int size)
{
	int w = 0;
	int chan = -1;
	short *wave_len = NULL;

	// Clear event at start of buffer and check the first word is a good header
	memset(event, 0, sizeof(Caen_event) );

   for(int board = 0; w < size; ++board) {
      // read board aggregate header
      if(data[w]>>28 != 0xa) {
         return 0;
      }
      int32_t numWordsBoard = data[w++]&0xfffffff; // this is the number of 32-bit words from this board
      if(w - 1 + numWordsBoard > size) {
         return 0;
      }
      uint8_t boardId = data[w]>>27; // GEO address of board (can be set via register 0xef08 for VME)
      //uint16_t pattern = (data[w]>>8) & 0x7fff; // value read from LVDS I/O (VME only)
      uint8_t channelMask = data[w++]&0xff; // which channels are in this board aggregate
      ++w;//uint32_t boardCounter = data[w++]&0x7fffff; // ??? "counts the board aggregate"
      ++w;//uint32_t boardTime = data[w++]; // time of creation of aggregate (does not correspond to a physical quantity)
		// Loop over all even channels (odd channels are grouped with the corresponding even channel)
      for(uint8_t channel = 0; channel < 16; channel += 2) {
			// skip channels not in the channel mask
         if(((channelMask>>(channel/2)) & 0x1) == 0x0) {
            continue;
         }
         // read channel aggregate header
         if(data[w]>>31 != 0x1) {
				// bad header, skip rest of data
				return 0;
         }
         int32_t numWords = data[w++]&0x3fffff;//per channel
         if(w >= size) {
				return 0;
         }
         if(((data[w]>>29) & 0x3) != 0x3) {
				return 0;
         }

			// we don't care about dual trace, everything is written to one waveform anyways
			//bool dualTrace        = ((data[w]>>31) == 0x1);
         bool extras           = (((data[w]>>28) & 0x1) == 0x1);
         bool waveformPresent  = (((data[w]>>27) & 0x1) == 0x1);
         uint8_t extraFormat   = ((data[w]>>24) & 0x7);
         //for now we ignore the information which traces are stored:
         //bits 22,23: if(dualTrace) 00 = "Input and baseline", 01 = "CFD and Baseline", 10 = "Input and CFD"
         //            else          00 = "Input", 01 = "CFD"
         //bits 19,20,21: 000 = "Long gate",  001 = "over thres.", 010 = "shaped TRG", 011 = "TRG Val. Accept. Win.", 100 = "Pile Up", 101 = "Coincidence", 110 = reserved, 111 = "Trigger"
         //bits 16,17,18: 000 = "Short gate", 001 = "over thres.", 010 = "TRG valid.", 011 = "TRG HoldOff",           100 = "Pile Up", 101 = "Coincidence", 110 = reserved, 111 = "Trigger"
         int numSampleWords = 4*(data[w++]&0xffff);// this is actually the number of samples divided by eight, 2 sample per word => 4*
         if(w >= size) {
				return 0;
         }
         int eventSize = numSampleWords+2; // +2 = trigger time words and charge word
         if(extras) ++eventSize;
         if(numWords%eventSize != 2 && !(eventSize == 2 && numWords%eventSize == 0)) { // 2 header words plus n*eventSize should make up one channel aggregate
				return 0;
         }

         // read channel data
         for(int ev = 0; ev < (numWords-2)/eventSize; ++ev) { // -2 = 2 header words for channel aggregate
            //event->SetDaqTimeStamp(boardTime);
            event->address = (0x8000 + (boardId * 0x100) + channel + (data[w]>>31)); // highest bit indicates odd channel
				chan = GetIDfromAddress(event->address);
				if(chan < 0 || chan >= NUM_CHAN ){ return 0; }
				wave_len  = &event->waveform_length[chan];     /* should be zero here */
            // these timestamps are in 2ns units
            event->timestamp[chan] = (data[w] & 0x7fffffff);
				event->tstamp_hitpattern[(int)chan/32] |= (1<<(chan%32));
            ++w;
            if(waveformPresent) {
               if(w + numSampleWords >= size) { // need to read at least the sample words plus the charge/extra word
						return 0;
               }
               for(int s = 0; s < numSampleWords && w < size; ++s, ++w) {
						if(wave_len == NULL) {
							fprintf(stderr,"descant_decode: no memory for waveform\n");
							dump_event(data,size);
							return 0;
						}
						waveform[chan][*(wave_len)  ] =  data[w]      & 0xffff;
						waveform[chan][*(wave_len)+1] = (data[w]>>16) & 0xffff;
						(*wave_len) += 2;
               }
            } else {
               if(w >= size) { // need to read at least the sample words plus the charge/extra word
						return 0;
               }
            }
            if(extras) {
               switch(extraFormat) {
                  case 0: // [31:16] extended time stamp, [15:0] baseline*4
                     event->timestamp_up[chan] = data[w]>>16;
							event->tstamphi_hitpattern[(int)chan/32] |= (1<<(chan%32));
                     break;
                  case 1: // [31:16] extended time stamp, 15 trigger lost, 14 over range, 13 1024 triggers, 12 n lost triggers
                     event->timestamp_up[chan] = data[w]>>16;
							event->tstamphi_hitpattern[(int)chan/32] |= (1<<(chan%32));
                     event->net_id = (data[w]>>12)&0xf;
                     break;
                  case 2: // [31:16] extended time stamp,  15 trigger lost, 14 over range, 13 1024 triggers, 12 n lost triggers, [9:0] fine time stamp
                     event->timestamp_up[chan] = data[w]>>16;
							event->tstamphi_hitpattern[(int)chan/32] |= (1<<(chan%32));
                     event->cfd[chan] = data[w]&0x3ff;
							event->cfd_hitpattern[(int)chan/32] |= (1<<(chan%32));
                     event->net_id = (data[w]>>12)&0xf;
                     break;
                  case 4: // [31:16] lost trigger counter, [15:0] total trigger counter
                     //event->SetAcceptedChannelId(data[w]&0xffff); // this is actually the lost trigger counter!
                     event->trig_acc[chan] = data[w]>>16;
							event->trig_acc_hitpattern[(int)chan/32] |= (1<<(chan%32));
                     break;
                  case 5: // [31:16] CFD sample after zero cross., [15:0] CFD sample before zero cross.
                     w++;
                     break;
                  case 7: // fixed value of 0x12345678
                     if(data[w] != 0x12345678) {
								fprintf(stderr,"Failed to get debug data word 0x12345678, got 0x%08x\n", data[w]);
                        break;
                     }
                     break;
                  default:
                     break;
               }
               ++w;
            }
            event->cc_short[chan] = data[w]&0x7fff;
            event->cc_long[chan] = (data[w]>>15) & 0x1;//this is actually the over-range bit!
            event->energy[chan] = data[w++]>>16;
				event->energy_hitpattern[(int)chan/32] |= (1<<(chan%32));
				if(!(process_decoded_event(event))) return -1;
				memset(event, 0, sizeof(Caen_event) );
         } // while(w < size)
      } // for(uint8_t channel = 0; channel < 16; channel += 2)
   } // for(int board = 0; w < size; ++board)

	return(SUCCESS); 
}

int descant(EVENT_HEADER *pheader, void *pevent)
{
	unsigned int *idata, *evt_data;
	int bank_len, evt_len;

	if ((bank_len = bk_locate(pevent, "GRF0", (DWORD *)&idata)) > 0 ){
		evt_data = idata; evt_len = bank_len;
	} 
	if ((bank_len = bk_locate(pevent, "GRF3", (DWORD *)&idata)) > 0 ){
		evt_data = idata; evt_len = bank_len;
		if( !(decode_descant_event( evt_data, evt_len))){ return(-1); }
		return SUCCESS;
	} 
	return(-1);
}

//////////////////////////////////////////////////////////////////////////
/////////////////       2d and addback spectra        ////////////////////
//////////////////////////////////////////////////////////////////////////

// have to build coincidence events before histogramming them
int analysed_spectra(Caen_event *ptr)
{
	//float ab_energy[NUM_CLOVER];
	//int i, fold, clover;

	//for(i=0; i<NUM_CLOVER; i++){ ab_energy[i] = 0; }
	//for(i=0; i<NUM_CHAN; i++){ // NUM_CHAN includes non-HPGe channels - is this the right thing to use?
	//	if( ptr->energy[i] == 0 ){ continue; }
	//	clover = i/4;
	//	ab_energy[clover] += 1332 * spread(ptr->energy[i]) / gains[i];
	//}

	//fold = 0;
	//for(i=0; i<NUM_CLOVER; i++){
	//	if( ab_energy[i]>0 ){ ++fold;
	//		// Web spectrum Addback sum
	//		if((int)ab_energy[i]>1 && (int)ab_energy[i]<SPEC_LENGTH){ spec_store_sum_data[4][(int)ab_energy[i]]++; }
	//	} 
	//}

	return(0);
}
