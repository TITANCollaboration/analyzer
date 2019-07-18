/******************************************************************************
 *       Simple GRIFFIN analyser
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h> /* rand() */
#include <time.h>
#include <unistd.h> /* usleep */
#include <math.h>
#include <string.h>  /* memset */

#include "midas.h"
#include "histogram.h"
#include "web_server.h"

#define TRUE 1
#define FALSE 0

int griffin(EVENT_HEADER *, void *);
int griffin_init();
int griffin_bor(INT run_number);
int griffin_eor(INT run_number);
void read_odb_gains();  int read_odb_histinfo();  int hist_init();
float spread(int val){ return( val + rand()/(1.0*RAND_MAX) ); }

ANA_MODULE griffin_module = {
   "Griffin",              /* module name           */
   "UrMom",                /* author                */
   griffin,                /* event routine         */
   griffin_bor,            /* BOR routine           */
   griffin_eor,            /* EOR routine           */
   griffin_init,           /* init routine          */
   NULL,                   /* exit routine          */
   NULL,                   /* parameter structure   */
   0,                      /* structure size        */
   NULL,                   /* initial parameters    */
};

#define NUM_ODB_CHAN     459 // size of msc table in odb
#define MAX_SAMPLE_LEN  4096
#define ENERGY_BINS    65536 /* 65536 131072 262144 */
#define NUM_CLOVER        16
#define MAX_CHAN        1024 
#define STRING_LEN       256
#define MIDAS_STRLEN      32
#define MAX_ADDRESS  0x10000
#define E_SPEC_LENGTH   8192
#define T_SPEC_LENGTH   8192
#define WV_SPEC_LENGTH  4096

typedef struct griffin_fragment_struct {
   int     address;   int  chan;       int    dtype;
   int      energy;   int e_bad;       int    integ;   int cfd;
   int     energy2;   int e2_bad;      int   integ2;   int cc_long;
   int     energy3;   int e3_bad;      int   integ3;   int cc_short;
   int     energy4;   int e4_bad;      int   integ4;   int nhit;
   int    trig_req;   int trig_acc;    int   pileup;
   long  timestamp;   int deadtime;
   int   master_id;   int master_pattern;
   int      net_id;   int trigger_num;
   int  wf_present;   short waveform_length;  
} Grif_event;

static int process_waveforms = 1;

static Grif_event  grif_event;
static Grif_event *ptr = &grif_event;
static short       waveform[MAX_SAMPLE_LEN];
static int        rate_data[MAX_CHAN];
//static float          gains[MAX_CHAN];
//static float        offsets[MAX_CHAN];
static char       chan_name[MAX_CHAN][MIDAS_STRLEN];
static short   chan_address[MAX_CHAN];
static int     num_chanhist;
static short   address_chan[MAX_ADDRESS];

extern HNDLE hDB; // Odb Handle

float   gains[NUM_ODB_CHAN];
float offsets[NUM_ODB_CHAN];

int decode_griffin_event( unsigned int *evntbuf, int evntbuflen);
int process_decoded_fragment(Grif_event *ptr);
int unpack_griffin_bank(unsigned *buf, int len);

/////////////////////////////////////////////////////////////////////////////

int griffin_init()
{  int i;
   read_odb_gains();  // Print the loaded gains and offsets
   fprintf(stdout,"\nRead Gain/Offset values from ODB\nIndex\tGain\tOffset\n");
   for(i=0; i<NUM_ODB_CHAN; i++){
     fprintf(stdout,"%d\t%f\t%f\n",i,gains[i],offsets[i]);
   }
   fprintf(stdout,"\n\n");

   if( (num_chanhist = read_odb_histinfo()) <= 0 ){
      ;
   }
   hist_init();
   return SUCCESS;
}
int griffin_eor(INT run_number){ return SUCCESS; }

void dump_event(unsigned int *data, int len)
{
   int i;
   for(i=0; i<len; i++){
      printf(" 0x%08x", data[i] );
      if( ((i+1)%6) == 0 ){ printf("\n   "); }
   }
   if( i % 6 ){ printf("\n   "); }
}

void read_odb_gains()
{
   int status, size;
   char tmp[STRING_LEN];
   HNDLE hKey;

   // readgains and offsets from ODB
   sprintf(tmp,"/DAQ/MSC/gain");
   if( (status=db_find_key(hDB, 0, tmp, &hKey)) != DB_SUCCESS){
     cm_msg(MINFO,"FE","Key %s not found", tmp); return;
   }
   size=sizeof(gains);
   if( (db_get_data(hDB,hKey,&gains,&size,TID_FLOAT)) != DB_SUCCESS){
      cm_msg(MINFO,"FE","Can't get data for Key %s", tmp); return;
   }
   sprintf(tmp,"/DAQ/MSC/offset");
   if( (status=db_find_key(hDB, 0, tmp, &hKey)) != DB_SUCCESS){
     cm_msg(MINFO,"FE","Key %s not found", tmp); return;
   }
   size=sizeof(offsets);
   if( (db_get_data(hDB,hKey,&offsets,&size,TID_FLOAT)) != DB_SUCCESS){
      cm_msg(MINFO,"FE","Can't get data for Key %s", tmp); return;
   }
}

int read_odb_histinfo()
{
   char tmp[STRING_LEN];
   int i, status, size;  
   HNDLE hKey;

   sprintf(tmp,"/DAQ/MSC/MSC"); // addresses
   if( (status=db_find_key(hDB, 0, tmp, &hKey)) != DB_SUCCESS){
     cm_msg(MINFO,"FE","Key %s not found", tmp); return(-1);
   }
   size=0;
   db_get_record_size(hDB, hKey, 0, &size);
   if( (db_get_record(hDB, hKey, &chan_address, &size, 0)) != DB_SUCCESS){
      cm_msg(MINFO,"FE","Can't get data for Key %s", tmp); return(-1);
   }
   memset(address_chan, 0xFF, sizeof(address_chan)); // set to -1
   for(i=0; i<NUM_ODB_CHAN && i<(size/sizeof(short)); i++){
      if( chan_address[i] >= 0 && chan_address[i] < MAX_ADDRESS ){
         address_chan[ chan_address[i] ] = i;
      }
   }

   sprintf(tmp,"/DAQ/MSC/Chan"); // names
   if( (status=db_find_key(hDB, 0, tmp, &hKey)) != DB_SUCCESS){
     cm_msg(MINFO,"FE","Key %s not found", tmp); return(-1);
   }
   size=0;
   db_get_record_size(hDB, hKey, 0, &size);
   if( (db_get_record(hDB, hKey,  &chan_name, &size, 0)) != DB_SUCCESS){
      cm_msg(MINFO,"FE","Can't get record for Key %s", tmp); return(-1);
   }
   return(size/MIDAS_STRLEN);
}

#define N_HITPAT 5
#define N_SUM 5
char hit_titles[N_HITPAT][32]={
   "HITPATTERN_Energy",   "HITPATTERN_Time",
   "HITPATTERN_Waveform", "HITPATTERN_Pulse_Height", "HITPATTERN_Rate"};
char sum_titles[N_SUM][32]={ "SUM_Singles_Low_gain_Energy","SUM_Singles_High_gain_Energy","SUM_Addback_Energy","SUM_PACES_Energy","SUM_LaBr3_Energy"};
char hit_names[N_HITPAT][32]={"e_hit","t_hit","w_hit","q_hit","r_hit"};
char sum_names[N_SUM][32]={"el_sum","eh_sum","a_sum","p_sum","l_sum"};

TH1I *hit_hist[N_HITPAT];
TH1I *sum_hist[N_SUM];
TH1I *ph_hist[MAX_CHAN], *e_hist[MAX_CHAN], *cfd_hist[MAX_CHAN];
TH1I *wave_hist[MAX_CHAN];

int hist_init()
{
   char title[STRING_LEN], handle[STRING_LEN];
   int i;
   for(i=0; i<MAX_CHAN; i++){ // Create each histogram for this channel
      if( i >= num_chanhist ){ break; }
      printf("%d = %d[0x%08x]: %s\n", i, chan_address[i], chan_address[i], chan_name[i]);
      sprintf(title,  "%s_Pulse_Height",   chan_name[i] );
      sprintf(handle, "%s_Q",              chan_name[i] );
      ph_hist[i] = H1_BOOK(handle, title, E_SPEC_LENGTH, 0, E_SPEC_LENGTH);
      sprintf(title,  "%s_Energy",         chan_name[i] );
      sprintf(handle, "%s_E",              chan_name[i] );
      e_hist[i] = H1_BOOK(handle, title, E_SPEC_LENGTH, 0, E_SPEC_LENGTH);
      sprintf(title,  "%s_Time",           chan_name[i] );
      sprintf(handle, "%s_T",              chan_name[i] );
      cfd_hist[i] = H1_BOOK(handle, title, T_SPEC_LENGTH, 0, T_SPEC_LENGTH);
      sprintf(title,  "%s_Waveform",       chan_name[i] );
      sprintf(handle, "%s_w",              chan_name[i] );
      wave_hist[i] = H1_BOOK(handle, title, WV_SPEC_LENGTH, 0, WV_SPEC_LENGTH);
   }
   for(i=0; i<N_HITPAT; i++){ // Create Hitpattern spectra
      hit_hist[i] = H1_BOOK(hit_names[i],hit_titles[i], MAX_CHAN, 0,MAX_CHAN);
   }
   for(i=0; i<N_SUM; i++){ // Create Sum spectra
      sum_hist[i] = H1_BOOK(sum_names[i],sum_titles[i], E_SPEC_LENGTH, 0,E_SPEC_LENGTH);
   }
   return(0);
}

static time_t bor_time=-1, start_time=-1;
static unsigned start_pkt;
int griffin_bor(INT run_number)
{
   bor_time = time(NULL); start_time = -1;
   read_odb_gains();
   Zero_Histograms();
   memset(rate_data, 0, sizeof(rate_data));
   printf("Success!\n");
   return SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////            per event functions           //////////////////
/////////////////////////////////////////////////////////////////////////////

int griffin(EVENT_HEADER *pheader, void *pevent)
{
   unsigned int *data;
   int words, evlen;

   if(        (words = bk_locate(pevent, "GRF0", (DWORD *)&data)) > 0 ){
   } else if( (words = bk_locate(pevent, "GRF3", (DWORD *)&data)) > 0 ){
   } else if( (words = bk_locate(pevent, "GRF4", (DWORD *)&data)) > 0 ){
   } else {
      return(-1);
   }
   while( words > 0 ){  // there may be many fragments in each event ...
      if( (evlen = unpack_griffin_bank( data, words )) < 0 ){ break; }
      data += evlen; words -= evlen;
      if( process_decoded_fragment( &grif_event ) ){
         fprintf(stderr,"skipping %d items after error ...\n", words);
         words = 0; break;
      }
   } 
   return SUCCESS;
}

// find fragment boundaries in bank
int unpack_griffin_bank(unsigned *buf, int len)
{
   unsigned *bufend = buf+len, *ptr = buf;

   while( buf < bufend ){
      if( ((*(ptr++)>>28)&0xff) == 0xE ){
	 if( decode_griffin_event( buf, ptr-buf ) ){ return(-1); }
	 return(ptr-buf);
      }
   }
   return(0);
}

int GetIDfromAddress(int addr)
{
   int id;
   //for(i=0; i<NUM_ODB_CHAN; i++){
   //   if( chan_address[i] == addr ){ return(i); }
   // }
   if( (id = address_chan[addr]) == -1 ){
      fprintf(stderr,"getID: unknown address: %7d[0x%04x]\n", addr, addr);
   }
   return(id);
}
int decode_griffin_event( unsigned int *evntbuf, int evntbuflen)
{
  unsigned int *evntbufend = evntbuf+evntbuflen, *evstrt=evntbuf, val32;
   int type, value, qtcount=0, chan;
   static int event_count;
   short *wave_len = NULL;

   // Clear ptr at start of buffer and check the first word is a good header
   memset(ptr, 0, sizeof(Grif_event) );
   if( ((*evntbuf) & 0x80000000) != 0x80000000 ){
      fprintf(stderr,"griffin_decode: bad header in event %d\n", event_count );
      return(-1);
   }
   while( evntbuf < evntbufend ){
      type = *evntbuf >> 28; val32 = *(evntbuf++); value = val32 & 0xffffffff;
      switch( type ){
      case 0x8: ++event_count;                           /*  Event header */
         if( evntbuf != evstrt+1 ){
            fprintf(stderr,"Event 0x%x(chan%d) 0x8 not at start of data\n",
               event_count, ptr->chan );
	 }
         ptr->dtype  = ((value & 0x0000F) >>  0);
         ptr->address= ((value & 0xFFFF0) >>  4);
         chan = ptr->chan = GetIDfromAddress(ptr->address);
         //ptr->chan   = ((value & 0x00FF0) >>  4);
         //grifc_port  = ((value & 0x0F000) >> 12);
         //master_port = ((value & 0xF0000) >> 16);
	 qtcount = 0;
         wave_len  = &ptr->waveform_length;     /* should be zero here */
 	 break;
      case 0x9:                      /* Channel Trigger Counter [AND MODE] */
         ptr->trig_req =  value & 0x0fffffff; break;
      case 0xa:                                           /*  Time Stamp Lo */
         ptr->timestamp   |= ( value & 0x0fffffff ); break;
      case 0xb:                               /* Time Stamp Hi and deadtime */
	 ptr->timestamp   |= ( (value & 0x0003fff) << 28);
	 ptr->deadtime     = ( (value & 0xfffc000) >> 14);
	 break;
      case 0xc:                                             /* waveform data */
         if( wave_len == NULL ){
            fprintf(stderr,"griffin_decode: no memory for waveform\n");
         } else if( process_waveforms == 1 ){ /* + 14->16bit sign extension */
	  //short x =  value & 0x3fff        |(((value>>13) & 1) ? 0xC000 : 0);
	  //short y = (value & 0xfffc000)>>14|(((value>>27) & 1) ? 0xC000 : 0);
	    waveform[(*wave_len)  ]   = value & 0x3fff;
            waveform[(*wave_len)++] |= ((value>>13) & 1) ? 0xC000 : 0;
    	    waveform[(*wave_len)  ]   =(value & 0xfffc000) >> 14;
            waveform[(*wave_len)++] |= ((value>>27) & 1) ? 0xC000 : 0;
	  //fprintf(stderr,"   %4d 0x%08x %04x %04x %5d %5d\n",
	  //	    (*wave_len)-2, val32, x, y, x, y);
	 }
	 break;
      case 0xd: ptr->net_id = val32;              /* network packet counter */
         // next 2 words are [mstpat/ppg mstid] in filtered data
	 if( ( *(evntbuf) >> 31 ) == 0 ){ val32 = *(evntbuf++);
	    ptr->wf_present = (val32 & 0x8000) >> 15;
	    ptr->pileup     = (val32 & 0x001F);
	 }
	 if( ( *(evntbuf) >> 31 ) == 0 ){ ptr->master_id   = *(evntbuf++); }
	 break;
      case 0xe:                                             /* Event Trailer */
         if( evntbuf != evntbufend ){
            fprintf(stderr,"Event 0x%x(chan%d) 0xE before End of data\n",
               event_count, ptr->chan );
	 }
         break;
      case 0x0: case 0x1: case 0x2: case 0x3:
      case 0x4: case 0x5: case 0x6: case 0x7: 
	 if( evntbuf - evstrt < 4 ){  // header stuff (with no 0xd present)
	    // next 2 words are [mstpat/ppg mstid] in filtered data
	    ptr->wf_present = (val32 & 0x8000) >> 15;
	    ptr->pileup     = (val32 & 0x001F);
	    if( ( *(evntbuf) >> 31 ) == 0 ){ ptr->master_id   = *(evntbuf++); }
	    break;
	 } else { // if dtype=6, maybe RF - extend sign from 30 to 32bits
	    if( ptr->dtype == 6 && (val32 & (1<<29)) ){ val32 |= 0xC0000000; }
	    if( ++qtcount == 1 ){                                /* Energy */
	       ptr->energy  = (ptr->dtype==6) ? val32 : val32 & 0x01ffffff;
	       ptr->e_bad   = (value >> 25) & 0x1;
	       ptr-> integ |= ((val32 & 0x7c000000) >> 17); ptr->nhit = 1;
	    } else if( qtcount == 2 ){                         /* CFD Time */
               ptr->cfd     = (ptr->dtype==6) ? val32 : val32 & 0x003fffff;
	       ptr-> integ |= ((val32 & 0x7FC00000) >> 22);
            } else if( qtcount == 3 ){
	       if(ptr->dtype==6){ ptr->cc_long  = val32; }  /* descant long*/
	       else { ptr->integ2 =   val32 & 0x003FFF;
		      ptr->nhit   = ((val32 & 0xFF0000) >> 16); }
	    } else if( qtcount == 4 ){
	       if(ptr->dtype==6){ ptr->cc_short  = val32; } /* descant short*/
	       else { ptr->energy2 =  val32 & 0x3FFFFF;
		      ptr->e2_bad  = (val32 >> 25) & 0x1; }
            } else if( qtcount == 5 ){
	       ptr->integ3 =   val32 & 0x00003FFF;
	       ptr->integ4 = ((val32 & 0x3FFF0000) >> 16);
	    } else if( qtcount == 6 ){ ptr->energy3 =  val32 & 0x3FFFFF;
	                               ptr->e4_bad  = (val32 >> 25) & 0x1;
	    } else if( qtcount == 7 ){ ptr->energy4 =  val32 & 0x3FFFFF;
	                               ptr->e4_bad  = (val32 >> 25) & 0x1;
	    } else {
	       fprintf(stderr,"Event 0x%x(chan%d) excess PH words\n",
               event_count, ptr->chan );
	    }
            break;
         }
      case 0xf: fprintf(stderr,"griffin_decode: 0xF.......\n");
                /* Unassigned packet identifier */ return(-1);
      default:  fprintf(stderr,"griffin_decode: default case\n"); return(-1);
      }
   }
   return(0); 
}

void checkdata(int len, unsigned int *data) // check test event data
{
   static unsigned int prevword; static int event;
   unsigned int count = prevword & 0x0ffffff;
   int i, j, gap, tag = prevword >> 28;

   for(i=0; i<len; i++){
      if( tag != 8 && (data[i]>>28) != 8 ){
	 gap = (data[i] & 0x0ffffff) - ((count+1) &  0x0ffffff);
	 if( gap ){
	    printf("Lost %d words at word %4d of event %10d ... 0x%08x\n   ",
	       gap, i, event, prevword);
	    for(j=0; j<len; j++){
	       printf(" 0x%08x", data[j] );
               if( ((j+1)%6) == 0 ){ printf("\n   "); }
	    }
            if( j % 6 ){ printf("\n   "); }
	    break;
	 }
      }
      count = data[i] & 0x0ffffff; tag = data[i] >> 28;
   }
   ++event; prevword = data[len-1];
}

#define RESET_SECONDS     10
static time_t last_reset;
#define TEST_HITPAT(var,bit) (var[(int)bit/32] & (1<<(bit%32)))
int process_decoded_fragment(Grif_event *ptr)
{
   time_t current_time = time(NULL);
   static int last_sample, event;
   int i, chan, ecal, len;
   float energy;
   short sample;

   if( bor_time == -1 ){ bor_time = current_time; }
   if( start_time == -1 ){
      start_time=current_time; start_pkt=ptr->net_id-1; event=0;
   } else { ++event; }

   // this should be done in a scalar routine
   if( current_time - last_reset > RESET_SECONDS ){
      for(i=0; i<MAX_CHAN; i++){
	//spec_store_hit_data[4][i] = rate_data[i];
      }
      memset(rate_data, 0, sizeof(rate_data));
      last_reset = current_time;
   }

   if( (chan = ptr->chan) == -1 ){ return(0); } // msg printed above for these
   if( chan >= num_chanhist ){
      fprintf(stderr,"process_event: ignored event in chan:%d [0x%04x]\n",
            	                                      chan, ptr->address );
      return(0);
   }
   energy = ( ptr->integ == 0 ) ? 0 : ptr->energy/ptr->integ;
   ecal   = spread(energy) * gains[chan] + offsets[chan];

   ++rate_data[chan];
   ph_hist[chan] -> Fill(ph_hist[chan],  (int)energy,     1);
   hit_hist[3]   -> Fill(hit_hist[3],    chan,            1);
   e_hist[chan]  -> Fill(e_hist[chan],   (int)ecal,       1);
   hit_hist[0]   -> Fill(hit_hist[0],    chan,            1);
   if(chan<64){sum_hist[0]   -> Fill(sum_hist[0],    (int)ecal,       1);} //Quick hack for only LO gain channels
   if(chan>63 && chan<128){sum_hist[1]   -> Fill(sum_hist[1],    (int)ecal,       1);} //Quick hack for only HI gain channels
   if(chan>99 && chan<106){sum_hist[3]   -> Fill(sum_hist[3],    (int)ecal,       1);} //Quick hack for only PACES channels
   if(chan>91 && chan<100){sum_hist[4]   -> Fill(sum_hist[4],    (int)ecal,       1);} //Quick hack for only LaBr3 channels
   cfd_hist[chan]-> Fill(cfd_hist[chan], ptr->cfd/256,    1);
   hit_hist[1]   -> Fill(hit_hist[1],    chan,            1);


   if( (len = ptr->waveform_length) != 0 ){
     if( len > MAX_SAMPLE_LEN ){ len = MAX_SAMPLE_LEN; }

     //fprintf(stderr,"%4d - %5d %5d %5d ... %5d %5d %5d ", len,
     //	 waveform[0]+8192,     waveform[1]+8192,     waveform[2]+8192,
     //    waveform[len-3]+8192, waveform[len-2]+8192, waveform[len-1]+8192 );
      if( last_sample == 0 ){ last_sample = len; }
      else if( last_sample != len ){
 	 fprintf(stderr,"event %4d - samples changed %d to %d\n",
	    event, last_sample, len);
      }
     //fprintf(stderr,"\n");

      hit_hist[2]   -> Fill(hit_hist[2],    chan,            1);
      wave_hist[chan]->Reset(wave_hist[chan]);
      wave_hist[chan]->SetValidLen(wave_hist[chan], len);
      for(i=0; i<len; i++){
         sample = waveform[i] += 8192; // shift from -8192:+8192 to 0:16384
         wave_hist[chan]  -> SetBinContent(wave_hist[chan], i, sample );
      }
   }

   return(0);
}
