// small+simple replacement for root histograms
//    Keep same code (add thisptr pointers), style is useful when having mutiple
//    histogram types/dimensions (otherwise clearer to convert to procedural)
// file IO is separate from histogram access: analyser dumps files at endofrun
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset
#include <time.h>   // time()
#include <math.h>   // NAN

#include "histogram2.h"

static char current_path[FOLDER_PATH_LENGTH];
static void *histogram_list[MAX_HISTOGRAMS];
static int next_histogram = 0;
static int check_folder(char *folder); // return valid length (or -1) if invalid

int Zero2_Histograms()
{
	int i;  TH2IHist *ptr;
	for(i=0; i<next_histogram; i++) {
		ptr = (TH2IHist *)histogram_list[i];
		TH2IHist_Reset(ptr);
	}
}

int TH2IHist_Reset(TH2IHist *thisptr)
{
	memset(thisptr->data, 0, thisptr->xbins*sizeof(float)); return(0);
	thisptr->valid_bins    = thisptr->nbins;
	thisptr->xunderflow     = 0;
	thisptr->xoverflow      = 0;
	thisptr->yunderflow     = 0;
	thisptr->yoverflow      = 0;
	thisptr->entries       = 0;
}

// Adds counts to the data array 
int TH2IHist_Fill(TH2IHist *thisptr, int binx, int biny, int count)
{
	int bin = thisptr->GetBin(thisptr, binx, biny); // overflow/underflow should be handled by the GetBin function
	(thisptr->data[bin])+=count;
	(thisptr->entries)+=count;
	return(0);
}

// returns the global bin number given x and y bins
int TH2IHist_GetBin(TH2IHist *thisptr, int binx, int biny) 
{
	if (binx < 0) {(thisptr->xunderflow)++; return(0);} // underflow x case
	if (biny < 0) {(thisptr->yunderflow)++; return(0);} // underflow y case
    if (binx > thisptr->xbins) {(thisptr->xoverflow)++; return(0);} //overflow x case
	if (biny > thisptr->ybins) {(thisptr->yoverflow)++; return(0);} //overflow y case 

	return((thisptr->xbins)*biny + binx); // 
}

int TH2IHist_SetBinContent(TH2IHist *thisptr, int binx, int biny, int value)
{
    int bin = thisptr->GetBin(thisptr, binx, biny);
	(thisptr->data[bin])=value; return(0);
}

int TH2IHist_GetBinContent(TH2IHist *thisptr, int binx, int biny)
{
	int bin = thisptr->GetBin(thisptr, binx, biny);
	return( (thisptr->data[bin]) );
}

// Sets the number of total bins 
int TH2IHist_SetValidLen(TH2IHist *thisptr, int bins)
{
	if( bins < 0 || bins >= thisptr->xbins ) { return(0); }
	(thisptr->valid_bins)=bins; return(0);
}

// Checks if the name matches any of the titles of the histograms in histogram_list 
/* if thisptr starts being slow - add names to hash table */
TH2IHist *hist2_querytitle(char *name)
{
	TH2IHist *ptr;   int i;
	for(i=0; i<next_histogram; i++) {
		//printf("Histogram that exists in histogram.c : %s", histogram_list[i]);
		ptr = (TH2IHist *)histogram_list[i];
		if( strcmp(name, ptr->title) == 0 ) { return( ptr ); }
	}	
	return( NULL );
}

// Checks if the name matches any of the handles of the histograms in histogram_list 
TH2IHist *hist2_queryhandle(char *name)
{
	TH2IHist *ptr;   int i;
	for(i=0; i<next_histogram; i++) {
		ptr = (TH2IHist *)histogram_list[i];
		if( strcmp(name, ptr->handle) == 0 ) { return( ptr ); }
	}
	return( NULL );
}
//TH2IHist *hist_querynum(int num){}

// TH2IHist constructor 
TH2IHist *H2_BOOK(char *name, char *title, int xbins, int ybins)
{
	int tlen, hlen; TH2IHist *result;
	int nbins = xbins * ybins;
	if( next_histogram >= MAX_HISTOGRAMS ) {
		fprintf(stderr,"H2_BOOK: max number of histograms:%d exceeded\n",
		        MAX_HISTOGRAMS );
		return(NULL);
	}
	if( (result = (TH2IHist *)malloc(sizeof(TH2IHist))) == NULL) {
		fprintf(stderr,"H2_BOOK: structure malloc failed\n");
		return(NULL);
	}
	if( (result->data = (int *)malloc(nbins*sizeof(int))) == NULL) {
		fprintf(stderr,"H2_BOOK: data malloc failed\n");
		free(result); return(NULL);
	}
	if( (tlen=strlen(title)) >= TITLE_LENGTH  ) { tlen = TITLE_LENGTH-1; }
	if( (hlen=strlen(name))  >= HANDLE_LENGTH ) { hlen = HANDLE_LENGTH-1; }
	//printf("Hist title in H1_BOOK webserver.c : %s\n", title);
	memcpy(result->path, current_path, strlen(current_path) );
	memcpy(result->handle, name, hlen);
	memcpy(result->title, title, tlen);
	memset(result->data, 0, nbins*sizeof(float) );
	result->xunderflow    = 0;
	result->xoverflow     = 0;
	result->yunderflow    = 0;
	result->yoverflow     = 0;
	result->entries       = 0;
	result->xbins         = xbins;
	result->ybins         = ybins;
	result->nbins		  = nbins;
	result->valid_bins    = nbins;
	result->Reset         = &TH2IHist_Reset;
	result->Fill          = &TH2IHist_Fill;
	result->GetBin        = &TH2IHist_GetBin;
	result->SetBinContent = &TH2IHist_SetBinContent;
	result->GetBinContent = &TH2IHist_GetBinContent;
	result->SetValidLen   = &TH2IHist_SetValidLen;
	result->type          = FLOAT_1D;
	histogram_list[next_histogram++] = (void *)result;
	return(result);
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////     histogram file IO etc.  //////////////////////////
// ----------------------------------------------------------------------------
// if not using root, need another way of storing histograms on disk
//    do not want old method of 1 file per histogram
//    want multi-histogram archive format
//    tar,zip,7zip,rar,cpio[=rpm],...,... don't like any except tar
// histo file format - tar?
//   can chain-seek to read single files
//   could also add index file at end - not really needed though
//   **by chain-seeking to eof to get index, will already have obtained it!!
//   **if uncompressed** can live update disk file
//      even if compressed - can add new entry at end, and mark old invalid
//         will have to sort out when closing file - not that important
//         so could defer thisptr (e.g. if analyzer crashes)
// compression (not only gzip etc, use cubesort methods)
//    [could use more working code for compressing spartan config file!]
//    empty spectra - zero length!
//    sparse - store chan/count pairs
//    rle - for non-sparse but mainly same value
//    *maybe* finally gzip if full (and large)
//       - not worth it if small - will gain nothing
//       - may not gain enough even if big
//    (large spectra need blocks with different methods for each block)
// histo format ...
//    series of bins, over+underflow bins are also useful
//    axis-scale different to bin-scale seemed useful but almost never used
//    int16/int32/float32, 1d or 2d, don't bother with 3+d
//griffin histogram xbins=x_xxx_xxx ybins=x_xxx_xxx comp=0 binformat=float32\n
//             V2                                        1           int16__\n
//                                                                   ascii3col
// first file "griffin histogram archive file"[zero length]
//   other fields for communication between programs having file open
//   memory map thisptr entry - doesn't actually reduce disk-reads when checking?
//     could do similar with unused bits of indiv. spec headers (unnecessary?)
//
// use name[100] for [path]/handle [=>short name in tar content listings]
// use uid/gid for xbin/ybin - also shown in listing
// use real size and mtime, linkflag=0 to allow extract file, title->linkname
// encode bin-format in mode [shown in listing], but also store name in ownr
// store compression format in group, over/underflow in devmaj/min?
//
// have tree since store pathnames, also don't need to store directory entries
// as histo folders don't have any attributes that need saving
//
//    **also need handle, title, maybe calibration etc? (probably not)
//    **thisptrptr plus mtime etc can be stored in tar header**
//    file itself has owner/group - could use these fields for bins
// extension .grif not .tar - to distinguish
// tar headers ...     **size OCTAL**    linkname[100] follows type -> 257bytes
//    name[100] mode[8] uid[8] gid[8] size[12] mtime[12] cksum[8] linktype[1]
//    at offset=257 are extension fields (ignored by earlier versions)
//  "ustar "[6] version[2] owner[32] group[32] devmaj[6] devmin[6] prefix[155]
//  [which leaves 12 bytes of padding]
//     checksum is sum of header values [0-255]x512 with space[32] for cksum
// tar extensions ...
//    gnu tar - numeric fields can be bin not ascii [set msb of leading byte]
//    prefix[155] -> atime[12] ctime[12] offset[12] longname[4] pad[1]
//    4 sparse entries:{offset[12] bytes[12]} realsize[12] extended[1] pad[17]
//
// tar includes plenty of unused header space plus standard extension method
// - so can include everything in tar header
// => minimum size is 0.5k (300k per 600 histo) (1-2Mbyte per tigress run!)
// tigress eta files have 2600 spectra in 1150k (would be ~*2)
//
// will be trivial to convert these files to root files
// (using script which will work on any root version, not compiled program)
// ----------------------------------------------------------------------------

// max length ~256, no odd characters or /
int check_folder(char *folder) // return valid length (or -1) if invalid
{
	int i, len;

	for(i=0; i<FOLDER_PATH_LENGTH; i++) {
		if( folder[i] == '\0' ) { break; }
	}
	if( (len=i) >= FOLDER_PATH_LENGTH ) {
		fprintf(stderr,"folder longer than %d\n", FOLDER_PATH_LENGTH );
		return(-1);
	}
	if( folder[len-1] == '/' ) { --len; } // remove trailing /
	for(i=0; i<len; i++) { // check input - no '/'
		if( folder[i] == '/' ) { break; }
		if( folder[i]  < ' ' ) { break; }// space = 32
		if( folder[i]  > '~' ) { break; }//     ~ = 127
	}
	if( i != len ) {
		fprintf(stderr,"invalid characters in folder:%s\n", folder );
		return(-1);
	}
	return(len);
}
