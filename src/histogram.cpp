// small+simple replacement for root histograms
//    Keep same code (add thisptr pointers), style is useful when having mutiple
//    histogram types/dimensions (otherwise clearer to convert to procedural)
// file IO is separate from histogram access: analyser dumps files at endofrun
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memset
#include <time.h>   // time()
#include <math.h>   // NAN

#include "histogram.h"

static char current_path[FOLDER_PATH_LENGTH];
static void *histogram_list[MAX_HISTOGRAMS];
static int next_histogram;
static int check_folder(char *folder); // return valid length (or -1) if invalid

int open_folder(char* folder)
{
	char *path = current_path;
	int len;

	if( (len = check_folder(folder)) <= 0 ) { return(-1); }
	if( len + strlen(path) >= FOLDER_PATH_LENGTH ) {
		fprintf(stderr,"folder path longer than %d\n", FOLDER_PATH_LENGTH );
		return(-1);
	}
	if( path[0] == 0 ) { memcpy(path, folder, len); }
	else { sprintf(&path[len], "/%s", folder); }
	return(0);
}

int close_folder() // work back from end to next slash
{
	char *path = current_path;
	int i, len = strlen(path);
	if( path[len-1] == '/' ) { --len; } // remove trailing /
	for(i=len-1; i>=0; i--) {
		if( path[len] == '/' ) { break; }
	}
	path[len] = '\0'; return(0);
}

int Zero_Histograms()
{
	int i;  TH1IHist *ptr;
	for(i=0; i<next_histogram; i++) {
		ptr = (TH1IHist *)histogram_list[i];
		TH1IHist_Reset(ptr);
	}
}

int TH1IHist_Reset(TH1IHist *thisptr)
{
	memset(thisptr->data, 0, thisptr->xbins*sizeof(float)); return(0);
	thisptr->valid_bins    = thisptr->xbins;
	thisptr->underflow     = 0;
	thisptr->overflow      = 0;
	thisptr->entries       = 0;
}

int TH1IHist_Fill(TH1IHist *thisptr, int bin, int count)
{
	(thisptr->entries)++;

	if( bin <            0 ) { (thisptr->underflow)++; return(0); }
	if( bin >= thisptr->xbins ) { (thisptr->overflow)++; return(0); }
	(thisptr->data[bin])+=count;
	return(0);
}

int TH1IHist_SetBinContent(TH1IHist *thisptr, int bin, int value)
{
	if( bin < 0 || bin >= thisptr->xbins ) { return(-1); }
	(thisptr->data[bin])=value; return(0);
}

int TH1IHist_GetBinContent(TH1IHist *thisptr, int bin)
{
	if( bin < 0 || bin >= thisptr->xbins ) { return(0); }
	return( (thisptr->data[bin]) );
}

int TH1IHist_SetValidLen(TH1IHist *thisptr, int bins)
{
	if( bins < 0 || bins >= thisptr->xbins ) { return(0); }
	(thisptr->valid_bins)=bins; return(0);
}

/* if thisptr starts being slow - add names to hash table */
TH1IHist *hist_querytitle(char *name)
{
	TH1IHist *ptr;   int i;
	for(i=0; i<next_histogram; i++) {
		//printf("Histogram that exists in histogram.c : %s", histogram_list[i]);
		ptr = (TH1IHist *)histogram_list[i];
		if( strcmp(name, ptr->title) == 0 ) { return( ptr ); }
	}
	return( NULL );
}
TH1IHist *hist_queryhandle(char *name)
{
	TH1IHist *ptr;   int i;
	for(i=0; i<next_histogram; i++) {
		ptr = (TH1IHist *)histogram_list[i];
		if( strcmp(name, ptr->handle) == 0 ) { return( ptr ); }
	}
	return( NULL );
}
//TH1IHist *hist_querynum(int num){}

TH1IHist *H1_BOOK(char *name, char *title, int nbins, int arg2, int arg3)
{
	int tlen, hlen; TH1IHist *result;
	if( next_histogram >= MAX_HISTOGRAMS ) {
		fprintf(stderr,"H1_BOOK: max number of histograms:%d exceeded\n",
		        MAX_HISTOGRAMS );
		return(NULL);
	}
	if( (result = (TH1IHist *)malloc(sizeof(TH1IHist))) == NULL) {
		fprintf(stderr,"H1_BOOK: structure malloc failed\n");
		return(NULL);
	}
	if( (result->data = (int *)malloc(nbins*sizeof(int))) == NULL) {
		fprintf(stderr,"H1_BOOK: data malloc failed\n");
		free(result); return(NULL);
	}
	if( (tlen=strlen(title)) >= TITLE_LENGTH  ) { tlen = TITLE_LENGTH-1; }
	if( (hlen=strlen(name))  >= HANDLE_LENGTH ) { hlen = HANDLE_LENGTH-1; }
	//printf("Hist title in H1_BOOK webserver.c : %s\n", title);
	memcpy(result->path, current_path, strlen(current_path) );
	memcpy(result->handle, name, hlen);
	memcpy(result->title, title, tlen);
	memset(result->data, 0, nbins*sizeof(float) );
	result->underflow     = 0;
	result->overflow      = 0;
	result->entries       = 0;
	result->xbins         = nbins;
	result->valid_bins    = nbins;
	result->Reset         = &TH1IHist_Reset;
	result->Fill          = &TH1IHist_Fill;
	result->SetBinContent = &TH1IHist_SetBinContent;
	result->GetBinContent = &TH1IHist_GetBinContent;
	result->SetValidLen   = &TH1IHist_SetValidLen;
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
