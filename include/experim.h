/********************************************************************\

  Name:         experim.h
  Created by:   ODBedit program

  Contents:     This file contains C structures for the "Experiment"
                tree in the ODB and the "/Analyzer/Parameters" tree.

                Additionally, it contains the "Settings" subtree for
                all items listed under "/Equipment" as well as their
                event definition.

                It can be used by the frontend and analyzer to work
                with these information.

                All C structures are accompanied with a string represen-
                tation which can be used in the db_create_record function
                to setup an ODB structure which matches the C structure.

  Created on:   Fri Oct 26 15:04:21 2018

\********************************************************************/

#ifndef EXCL_SIS3302

#define SIS3302_PARAM_DEFINED

typedef struct {
  INT       internal_trigger;
  INT       negative_input;
  INT       eventsize_threshold;
  INT       pretrigger_delay;
  INT       trigger_gate_length;
  INT       energy_gate_length;
  INT       rawbuf_start;
  INT       raw_samples;
  INT       energy_sample_length;
  INT       energy_sample_index1;
  INT       energy_sample_index2;
  INT       energy_sample_index3;
  struct {
    INT       threshold[8];
    INT       peakingtime[8];
    INT       gaptime[8];
    INT       decimation[8];
    INT       pulse_length;
    INT       internal_gate_length;
    INT       internal_delay;
  } triggersettings;
  struct {
    INT       peakingtime[4];
    INT       gaptime[4];
    INT       decimation[4];
    INT       tau[8];
  } energysampling;
  INT       dac[8];
  INT       clockmhz;
} SIS3302_PARAM;

#define SIS3302_PARAM_STR(_name) const char *_name[] = {\
"[.]",\
"internal trigger = INT : 16",\
"negative input = INT : 255",\
"eventsize threshold = INT : 448",\
"pretrigger delay = INT : 50",\
"trigger Gate Length = INT : 600",\
"Energy Gate Length = INT : 800",\
"RawBuf Start = INT : 0",\
"Raw Samples = INT : 0",\
"Energy Sample Length = INT : 0",\
"Energy Sample Index1 = INT : 0",\
"Energy Sample index2 = INT : 0",\
"Energy Sample index3 = INT : 0",\
"",\
"[triggersettings]",\
"threshold = INT[8] :",\
"[0] 15",\
"[1] 40",\
"[2] 15",\
"[3] 55",\
"[4] 20",\
"[5] 45",\
"[6] 20",\
"[7] 0",\
"peakingtime = INT[8] :",\
"[0] 8",\
"[1] 8",\
"[2] 8",\
"[3] 8",\
"[4] 8",\
"[5] 8",\
"[6] 8",\
"[7] 0",\
"gaptime = INT[8] :",\
"[0] 16",\
"[1] 16",\
"[2] 16",\
"[3] 16",\
"[4] 16",\
"[5] 16",\
"[6] 16",\
"[7] 0",\
"decimation = INT[8] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"[4] 1",\
"[5] 1",\
"[6] 1",\
"[7] 0",\
"pulse length = INT : 10",\
"internal gate length = INT : 2",\
"internal delay = INT : 0",\
"",\
"[energysampling]",\
"peakingtime = INT[4] :",\
"[0] 150",\
"[1] 150",\
"[2] 150",\
"[3] 150",\
"gaptime = INT[4] :",\
"[0] 250",\
"[1] 250",\
"[2] 250",\
"[3] 250",\
"decimation = INT[4] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"tau = INT[8] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"",\
"[.]",\
"dac = INT[8] :",\
"[0] 50000",\
"[1] 50000",\
"[2] 50000",\
"[3] 50000",\
"[4] 50000",\
"[5] 50000",\
"[6] 50000",\
"[7] 50000",\
"clockMHz = INT : 10",\
"",\
NULL }

#endif

#ifndef EXCL_SIS3302

#define SIS3302_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} SIS3302_COMMON;

#define SIS3302_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 2",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 500",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] lxebit2.triumf.ca",\
"Frontend name = STRING : [32] feSis",\
"Frontend file name = STRING : [256] fesis.c",\
"Status = STRING : [256] feSis@lxebit2.triumf.ca",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"",\
NULL }

#define SIS3302_SETTINGS_DEFINED

typedef struct {
  INT       internal_trigger;
  INT       negative_input;
  INT       eventsize_threshold;
  INT       pretrigger_delay;
  INT       trigger_gate_length;
  INT       energy_gate_length;
  INT       rawbuf_start;
  INT       raw_samples;
  INT       energy_sample_length;
  INT       energy_sample_index1;
  INT       energy_sample_index2;
  INT       energy_sample_index3;
  struct {
    INT       threshold[8];
    INT       peakingtime[8];
    INT       gaptime[8];
    INT       decimation[8];
    INT       pulse_length;
    INT       internal_gate_length;
    INT       internal_delay;
  } triggersettings;
  struct {
    INT       peakingtime[4];
    INT       gaptime[4];
    INT       decimation[4];
    INT       tau[8];
  } energysampling;
  INT       dac[8];
  INT       clockmhz;
} SIS3302_SETTINGS;

#define SIS3302_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"internal trigger = INT : 32",\
"negative input = INT : 0",\
"eventsize threshold = INT : 50",\
"pretrigger delay = INT : 50",\
"trigger Gate Length = INT : 600",\
"Energy Gate Length = INT : 800",\
"RawBuf Start = INT : 0",\
"Raw Samples = INT : 1",\
"Energy Sample Length = INT : 0",\
"Energy Sample Index1 = INT : 0",\
"Energy Sample index2 = INT : 0",\
"Energy Sample index3 = INT : 0",\
"",\
"[triggersettings]",\
"threshold = INT[8] :",\
"[0] 8",\
"[1] 8",\
"[2] 6",\
"[3] 24",\
"[4] 25",\
"[5] 3",\
"[6] 20",\
"[7] 3",\
"peakingtime = INT[8] :",\
"[0] 8",\
"[1] 8",\
"[2] 7",\
"[3] 8",\
"[4] 8",\
"[5] 8",\
"[6] 8",\
"[7] 8",\
"gaptime = INT[8] :",\
"[0] 16",\
"[1] 10",\
"[2] 10",\
"[3] 10",\
"[4] 10",\
"[5] 10",\
"[6] 12",\
"[7] 10",\
"decimation = INT[8] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"pulse length = INT : 10",\
"internal gate length = INT : 2",\
"internal delay = INT : 0",\
"",\
"[energysampling]",\
"peakingtime = INT[4] :",\
"[0] 125",\
"[1] 125",\
"[2] 125",\
"[3] 125",\
"gaptime = INT[4] :",\
"[0] 50",\
"[1] 50",\
"[2] 50",\
"[3] 50",\
"decimation = INT[4] :",\
"[0] 1",\
"[1] 1",\
"[2] 1",\
"[3] 1",\
"tau = INT[8] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"",\
"[.]",\
"dac = INT[8] :",\
"[0] 50000",\
"[1] 50000",\
"[2] 50000",\
"[3] 50000",\
"[4] 50000",\
"[5] 50000",\
"[6] 50000",\
"[7] 50000",\
"clockMHz = INT : 10",\
"",\
NULL }

#endif

#ifndef EXCL_SCALER3302

#define SCALER3302_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} SCALER3302_COMMON;

#define SCALER3302_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 2",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 377",\
"Period = INT : 5000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 20",\
"Frontend host = STRING : [32] lxebit2.triumf.ca",\
"Frontend name = STRING : [32] feSis",\
"Frontend file name = STRING : [256] fesis.c",\
"Status = STRING : [256] feSis@lxebit2.triumf.ca",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"",\
NULL }

#endif

#ifndef EXCL_BEAMLINE

#define BEAMLINE_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} BEAMLINE_COMMON;

#define BEAMLINE_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 11",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 16",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 255",\
"Period = INT : 10000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 10",\
"Frontend host = STRING : [32] titan01.triumf.ca",\
"Frontend name = STRING : [32] scEpics",\
"Frontend file name = STRING : [256] scepics.c",\
"Status = STRING : [256] ",\
"Status color = STRING : [32] ",\
"Hidden = BOOL : n",\
"",\
NULL }

#define BEAMLINE_SETTINGS_DEFINED

typedef struct {
  struct {
    struct {
      char      channel_name[34][32];
    } beamline;
  } devices;
  char      names[34][32];
  float     update_threshold_measured[34];
} BEAMLINE_SETTINGS;

#define BEAMLINE_SETTINGS_STR(_name) const char *_name[] = {\
"[Devices/Beamline]",\
"Channel name = STRING[34] :",\
"[32] TITAN:SYS:TC1",\
"[32] MPET:PLTP:RDVOL",\
"[32] CCS2ISAC:BL2ACURRENT",\
"[32] TLN2:LED1:RDVOL",\
"[32] TLN2:LED2:RDVOL",\
"[32] TLN2:LED3:RDVOL",\
"[32] TLN2:LED4:RDVOL",\
"[32] TLN2:LED5:RDVOL",\
"[32] TLN2:LED6:RDVOL",\
"[32] TLN2:LED7:RDVOL",\
"[32] TLN2:LED8:RDVOL",\
"[32] TLN2:LEDX:RDVOL",\
"[32] EBIT:TC1:RDTEMP",\
"[32] EBIT:TC2:RDTEMP",\
"[32] EBIT:TC3:RDTEMP",\
"[32] EBIT:TC4:RDTEMP",\
"[32] EBIT:DT7E6PL:RDVOL",\
"[32] EBIT:DT7E5PL:RDVOL",\
"[32] EBIT:DT7E4PL:RDVOL",\
"[32] EBIT:CATHCUR:RDCUR",\
"[32] EBIT:COLLCUR:RDCUR",\
"[32] EBIT:GUNCATH:RDVOL",\
"[32] IMS:MQRATIO:MB1",\
"[32] IMS:MQRATIO:MB2",\
"[32] IIS:FC2:SCALECUR",\
"[32] EBIT:IG1:RDVAC",\
"[32] EBIT:IG2:RDVAC",\
"[32] EBIT:DT74PH:RDVOL",\
"[32] TRFC:IG5:RDVAC",\
"[32] EBIT:GUNFOCUS:RDVOL",\
"[32] EBIT:GUNANOD:RDVOL",\
"[32] EBIT:COLLSUP:RDCUR",\
"[32] EBIT:GUNANOD:RDCUR",\
"[32] EBIT:GUNCATH:RDCUR",\
"",\
"[.]",\
"Names = STRING[34] :",\
"[32] PLATF%TC1",\
"[32] EBIT%MPETPLTP",\
"[32] Beamline%ProtonCurrent",\
"[32] LN2%LED1",\
"[32] LN2%LED2",\
"[32] LN2%LED3",\
"[32] LN2%LED4",\
"[32] LN2%LED5",\
"[32] LN2%LED6",\
"[32] LN2%LED7",\
"[32] LN2%LED8",\
"[32] LN2%LEDmanifold",\
"[32] Default%TEMP1",\
"[32] Default%TEMP2",\
"[32] Default%TEMP3",\
"[32] Default%TEMP4",\
"[32] EBIT%DT7E6PL",\
"[32] EBIT%DT7E5PL",\
"[32] EBIT%DT7E4PL",\
"[32] EBIT%CathodeCurrent",\
"[32] EBIT%CollectorCurrent",\
"[32] EBIT%GunCathVoltage",\
"[32] Beamline%MQRatioMB1",\
"[32] Beamline%MQRatioMB2",\
"[32] Beamline%BeamDumpCur",\
"[32] EBIT%MagPressureIG1",\
"[32] EBIT%GunPressureIG2",\
"[32] EBIT%DT74PH",\
"[32] EBIT%IG5",\
"[32] EBIT%GunFocus",\
"[32] EBIT%GunAnode",\
"[32] EBIT%CollSupCurrent",\
"[32] EBIT%GunAnodeCurrent",\
"[32] EBIT%GunCathodeCurrent",\
"Update Threshold Measured = FLOAT[34] :",\
"[0] 0",\
"[1] 0",\
"[2] 0",\
"[3] 0",\
"[4] 0",\
"[5] 0",\
"[6] 0",\
"[7] 0",\
"[8] 0",\
"[9] 0",\
"[10] 0",\
"[11] 0",\
"[12] 0",\
"[13] 0",\
"[14] 0",\
"[15] 0",\
"[16] 0",\
"[17] 0",\
"[18] 0",\
"[19] 0",\
"[20] 0",\
"[21] 0",\
"[22] 0",\
"[23] 0",\
"[24] 0",\
"[25] 0",\
"[26] 0",\
"[27] 0",\
"[28] 0",\
"[29] 0",\
"[30] 0",\
"[31] 0",\
"[32] 0",\
"[33] 0",\
"",\
NULL }

#endif

#ifndef EXCL_LABVIEW

#define LABVIEW_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} LABVIEW_COMMON;

#define LABVIEW_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 6",\
"Trigger mask = WORD : 64",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 16777215",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 511",\
"Period = INT : 1",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 1",\
"Frontend host = STRING : [32] lxebit2.triumf.ca",\
"Frontend name = STRING : [32] felabview",\
"Frontend file name = STRING : [256] felabview.cxx",\
"Status = STRING : [256] felabview@lxebit2.triumf.ca",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"",\
NULL }

#define LABVIEW_SETTINGS_DEFINED

typedef struct {
  DWORD     tcp_port;
} LABVIEW_SETTINGS;

#define LABVIEW_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"tcp_port = DWORD : 12021",\
"",\
NULL }

#endif

#ifndef EXCL_IO32

#define IO32_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} IO32_COMMON;

#define IO32_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 13",\
"Trigger mask = WORD : 8192",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 257",\
"Period = INT : 10000",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 1",\
"Frontend host = STRING : [32] lxebit2.triumf.ca",\
"Frontend name = STRING : [32] feio32",\
"Frontend file name = STRING : [256] feio32.cxx",\
"Status = STRING : [256] feio32@lxebit2.triumf.ca",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"",\
NULL }

#endif

#ifndef EXCL_SNMP

#define SNMP_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} SNMP_COMMON;

#define SNMP_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 2",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 1",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 255",\
"Period = INT : 10",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 1",\
"Frontend host = STRING : [32] titan01.triumf.ca",\
"Frontend name = STRING : [32] fesnmp",\
"Frontend file name = STRING : [256] fesnmp.cxx",\
"Status = STRING : [256] Communication problem",\
"Status color = STRING : [32] #800000",\
"Hidden = BOOL : n",\
"",\
NULL }

#define SNMP_SETTINGS_DEFINED

typedef struct {
  char      snmpwalkcommand[200];
  INT       readperiod;
  struct {
    char      ___example___[200];
    char      upsidentmanufacturer_0[200];
    char      upsidentmodel_0[200];
    char      upsidentupssoftwareversion_0[200];
    char      upsidentagentsoftwareversion_0[200];
    char      upsidentname_0[200];
    char      upsidentattacheddevices_0[200];
    char      upsbatterystatus_0[200];
    char      upssecondsonbattery_0[200];
    char      upsestimatedminutesremaining_0[200];
    char      upsestimatedchargeremaining_0[200];
    char      upsbatteryvoltage_0[200];
    char      upsbatterytemperature_0[200];
    char      upsinputlinebads_0[200];
    char      upsinputnumlines_0[200];
    char      upsinputlineindex_1[200];
    char      upsinputfrequency_1[200];
    char      upsinputvoltage_1[200];
    char      upsoutputsource_0[200];
    char      upsoutputfrequency_0[200];
    char      upsoutputnumlines_0[200];
    char      upsoutputlineindex_1[200];
    char      upsoutputvoltage_1[200];
    char      upsoutputcurrent_1[200];
    char      upsoutputpower_1[200];
    char      upsoutputpercentload_1[200];
    char      upsbypassfrequency_0[200];
    char      upsbypassnumlines_0[200];
    char      upsbypasslineindex_1[200];
    char      upsbypassvoltage_1[200];
    char      upsalarmspresent_0[200];
    char      upsalarmid_1[200];
    char      upsalarmdescr_1[200];
    char      upsalarmtime_1[200];
    char      upsalarmid_2[200];
    char      upsalarmdescr_2[200];
    char      upsalarmtime_2[200];
    char      upsalarmid_3[200];
    char      upsalarmdescr_3[200];
    char      upsalarmtime_3[200];
  } history;
} SNMP_SETTINGS;

#define SNMP_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"SnmpwalkCommand = STRING : [200] /usr/bin/snmpwalk -v 2c -c public -m +TRIPPLITE-MIB titan-ups1 upsMIB",\
"ReadPeriod = INT : 10",\
"",\
"[History]",\
"***example*** = STRING : [200] variableIsEnabled(y/n),variableUpdatePeriod(sec),variableName",\
"upsIdentManufacturer.0 = STRING : [200] n,1,upsIdentManufacturer.0",\
"upsIdentModel.0 = STRING : [200] n,1,upsIdentModel.0",\
"upsIdentUPSSoftwareVersion.0 = STRING : [200] n,1,upsIdentUPSSoftwareVersion.0",\
"upsIdentAgentSoftwareVersion.0 = STRING : [200] n,1,upsIdentAgentSoftwareVersion.0",\
"upsIdentName.0 = STRING : [200] n,1,upsIdentName.0",\
"upsIdentAttachedDevices.0 = STRING : [200] n,1,upsIdentAttachedDevices.0",\
"upsBatteryStatus.0 = STRING : [200] y,1,upsBatteryStatus.0",\
"upsSecondsOnBattery.0 = STRING : [200] y,1,upsSecondsOnBattery.0",\
"upsEstimatedMinutesRemaining.0 = STRING : [200] n,1,upsEstimatedMinutesRemaining.0",\
"upsEstimatedChargeRemaining.0 = STRING : [200] n,1,upsEstimatedChargeRemaining.0",\
"upsBatteryVoltage.0 = STRING : [200] y,1,upsBatteryVoltage.0",\
"upsBatteryTemperature.0 = STRING : [200] y,1,upsBatteryTemperature.0",\
"upsInputLineBads.0 = STRING : [200] n,1,upsInputLineBads.0",\
"upsInputNumLines.0 = STRING : [200] n,1,upsInputNumLines.0",\
"upsInputLineIndex.1 = STRING : [200] n,1,upsInputLineIndex.1",\
"upsInputFrequency.1 = STRING : [200] y,1,upsInputFrequency.1",\
"upsInputVoltage.1 = STRING : [200] y,1,upsInputVoltage.1",\
"upsOutputSource.0 = STRING : [200] n,1,upsOutputSource.0",\
"upsOutputFrequency.0 = STRING : [200] y,1,upsOutputFrequency.0",\
"upsOutputNumLines.0 = STRING : [200] n,1,upsOutputNumLines.0",\
"upsOutputLineIndex.1 = STRING : [200] n,1,upsOutputLineIndex.1",\
"upsOutputVoltage.1 = STRING : [200] y,1,upsOutputVoltage.1",\
"upsOutputCurrent.1 = STRING : [200] y,1,upsOutputCurrent.1",\
"upsOutputPower.1 = STRING : [200] y,1,upsOutputPower.1",\
"upsOutputPercentLoad.1 = STRING : [200] y,1,upsOutputPercentLoad.1",\
"upsBypassFrequency.0 = STRING : [200] n,1,upsBypassFrequency.0",\
"upsBypassNumLines.0 = STRING : [200] n,1,upsBypassNumLines.0",\
"upsBypassLineIndex.1 = STRING : [200] n,1,upsBypassLineIndex.1",\
"upsBypassVoltage.1 = STRING : [200] n,1,upsBypassVoltage.1",\
"upsAlarmsPresent.0 = STRING : [200] y,1,upsAlarmsPresent.0",\
"upsAlarmId.1 = STRING : [200] n,1,upsAlarmId.1",\
"upsAlarmDescr.1 = STRING : [200] n,1,upsAlarmDescr.1",\
"upsAlarmTime.1 = STRING : [200] n,1,upsAlarmTime.1",\
"upsAlarmId.2 = STRING : [200] n,1,upsAlarmId.2",\
"upsAlarmDescr.2 = STRING : [200] n,1,upsAlarmDescr.2",\
"upsAlarmTime.2 = STRING : [200] n,1,upsAlarmTime.2",\
"upsAlarmId.3 = STRING : [200] n,1,upsAlarmId.3",\
"upsAlarmDescr.3 = STRING : [200] n,1,upsAlarmDescr.3",\
"upsAlarmTime.3 = STRING : [200] n,1,upsAlarmTime.3",\
"",\
NULL }

#define SNMP_READBACK_DEFINED

typedef struct {
  char      upsidentmanufacturer_0[11];
  char      upsidentmodel_0[15];
  char      upsidentupssoftwareversion_0[12];
  char      upsidentagentsoftwareversion_0[11];
  char      upsidentname_0[11];
  char      upsidentattacheddevices_0[1];
  INT       upsbatterystatus_0;
  INT       upssecondsonbattery_0;
  INT       upsestimatedminutesremaining_0;
  INT       upsestimatedchargeremaining_0;
  INT       upsbatteryvoltage_0;
  INT       upsbatterytemperature_0;
  INT       upsinputlinebads_0;
  INT       upsinputnumlines_0;
  INT       upsinputlineindex_1;
  INT       upsinputfrequency_1;
  INT       upsinputvoltage_1;
  INT       upsoutputsource_0;
  INT       upsoutputfrequency_0;
  INT       upsoutputnumlines_0;
  INT       upsoutputlineindex_1;
  INT       upsoutputvoltage_1;
  INT       upsoutputcurrent_1;
  INT       upsoutputpower_1;
  INT       upsoutputpercentload_1;
  INT       upsbypassfrequency_0;
  INT       upsbypassnumlines_0;
  INT       upsbypasslineindex_1;
  INT       upsbypassvoltage_1;
  INT       upsalarmspresent_0;
  INT       upsalarmid_1;
  char      upsalarmdescr_1[13];
  INT       upsalarmtime_1;
  INT       upsalarmid_2;
  char      upsalarmdescr_2[29];
  INT       upsalarmtime_2;
  INT       upsalarmid_3;
  char      upsalarmdescr_3[19];
  INT       upsalarmtime_3;
} SNMP_READBACK;

#define SNMP_READBACK_STR(_name) const char *_name[] = {\
"[.]",\
"upsIdentManufacturer.0 = STRING : [11] Tripp Lite",\
"upsIdentModel.0 = STRING : [15] SU1000RTXLCD2U",\
"upsIdentUPSSoftwareVersion.0 = STRING : [12] FW2567R09_1",\
"upsIdentAgentSoftwareVersion.0 = STRING : [11] 12.04.0055",\
"upsIdentName.0 = STRING : [11] titan-ups1",\
"upsIdentAttachedDevices.0 = STRING : [1] ",\
"upsBatteryStatus.0 = INT : 2",\
"upsSecondsOnBattery.0 = INT : 0",\
"upsEstimatedMinutesRemaining.0 = INT : 18",\
"upsEstimatedChargeRemaining.0 = INT : 100",\
"upsBatteryVoltage.0 = INT : 272",\
"upsBatteryTemperature.0 = INT : 32",\
"upsInputLineBads.0 = INT : 0",\
"upsInputNumLines.0 = INT : 1",\
"upsInputLineIndex.1 = INT : 1",\
"upsInputFrequency.1 = INT : 590",\
"upsInputVoltage.1 = INT : 118",\
"upsOutputSource.0 = INT : 3",\
"upsOutputFrequency.0 = INT : 599",\
"upsOutputNumLines.0 = INT : 1",\
"upsOutputLineIndex.1 = INT : 1",\
"upsOutputVoltage.1 = INT : 121",\
"upsOutputCurrent.1 = INT : 3",\
"upsOutputPower.1 = INT : 303",\
"upsOutputPercentLoad.1 = INT : 36",\
"upsBypassFrequency.0 = INT : 599",\
"upsBypassNumLines.0 = INT : 1",\
"upsBypassLineIndex.1 = INT : 1",\
"upsBypassVoltage.1 = INT : 118",\
"upsAlarmsPresent.0 = INT : 1",\
"upsAlarmId.1 = INT : 1",\
"upsAlarmDescr.1 = STRING : [13] \"Output Off\"",\
"upsAlarmTime.1 = INT : 1391443134",\
"upsAlarmId.2 = INT : 2",\
"upsAlarmDescr.2 = STRING : [29] \"Load Level Above Threshold\"",\
"upsAlarmTime.2 = INT : -2065523746",\
"upsAlarmId.3 = INT : 3",\
"upsAlarmDescr.3 = STRING : [19] \"Self Test Failed\"",\
"upsAlarmTime.3 = INT : -1435265999",\
"",\
NULL }

#endif

#ifndef EXCL_ER3302

#define ER3302_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} ER3302_COMMON;

#define ER3302_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 22867",\
"Trigger mask = WORD : 21587",\
"Buffer = STRING : [32] EM",\
"Type = INT : 0",\
"Source = INT : 1094994253",\
"Format = STRING : [8] S",\
"Enabled = BOOL : y",\
"Read on = INT : 5000",\
"Period = INT : 0",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] orchid.triumf.ca",\
"Frontend name = STRING : [32] feSis",\
"Frontend file name = STRING : [256] fesis.c",\
"Status = STRING : [256] feSis@orchid.triumf.ca",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"",\
NULL }

#endif

#ifndef EXCL_MDPP16

#define MDPP16_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} MDPP16_COMMON;

#define MDPP16_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 1",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 2",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : y",\
"Read on = INT : 1",\
"Period = INT : 500",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] lxebit2.triumf.ca",\
"Frontend name = STRING : [32] feMdpp16",\
"Frontend file name = STRING : [256] femdpp.c",\
"Status = STRING : [256] feMdpp16@lxebit2.triumf.ca",\
"Status color = STRING : [32] greenLight",\
"Hidden = BOOL : n",\
"",\
NULL }

#define MDPP16_SETTINGS_DEFINED

typedef struct {
  DWORD     multi_event;
  DWORD     max_datalen;
  DWORD     window_width;
  DWORD     window_start;
  DWORD     tdc_resln;
  DWORD     adc_resln;
  DWORD     trig_source;
  DWORD     nim2_input;
  DWORD     pulser_ctrl;
  DWORD     pulser_amp;
  DWORD     mark_type;
  DWORD     trig_output;
  DWORD     ts_source;
  char      temp[17];
  DWORD     tf_int_diff;
  DWORD     gain;
  DWORD     threshold0;
  DWORD     threshold1;
  DWORD     shaping_time;
  DWORD     blr;
  DWORD     reset_time;
  DWORD     rise_time;
} MDPP16_SETTINGS;

#define MDPP16_SETTINGS_STR(_name) const char *_name[] = {\
"[.]",\
"multi_event = DWORD : 0",\
"max_datalen = DWORD : 0",\
"window_width = DWORD : 512",\
"window_start = DWORD : 16128",\
"tdc_resln = DWORD : 2",\
"adc_resln = DWORD : 0",\
"trig_source = DWORD : 256",\
"nim2_input = DWORD : 2",\
"pulser_ctrl = DWORD : 0",\
"pulser_amp = DWORD : 400",\
"mark_type = DWORD : 1",\
"trig_output = DWORD : 256",\
"ts_source = DWORD : 0",\
"temp = STRING : [17] channel settings",\
"tf_int_diff = DWORD : 20",\
"gain = DWORD : 218",\
"threshold0 = DWORD : 112",\
"threshold1 = DWORD : 112",\
"shaping_time = DWORD : 240",\
"blr = DWORD : 1",\
"reset_time = DWORD : 4000",\
"rise_time = DWORD : 80",\
"",\
NULL }

#endif

#ifndef EXCL_MDPP16_ANALYSER

#define MDPP16_ANALYSER_COMMON_DEFINED

typedef struct {
  WORD      event_id;
  WORD      trigger_mask;
  char      buffer[32];
  INT       type;
  INT       source;
  char      format[8];
  BOOL      enabled;
  INT       read_on;
  INT       period;
  double    event_limit;
  DWORD     num_subevents;
  INT       log_history;
  char      frontend_host[32];
  char      frontend_name[32];
  char      frontend_file_name[256];
  char      status[256];
  char      status_color[32];
  BOOL      hidden;
} MDPP16_ANALYSER_COMMON;

#define MDPP16_ANALYSER_COMMON_STR(_name) const char *_name[] = {\
"[.]",\
"Event ID = WORD : 0",\
"Trigger mask = WORD : 0",\
"Buffer = STRING : [32] SYSTEM",\
"Type = INT : 0",\
"Source = INT : 0",\
"Format = STRING : [8] MIDAS",\
"Enabled = BOOL : n",\
"Read on = INT : 0",\
"Period = INT : 0",\
"Event limit = DOUBLE : 0",\
"Num subevents = DWORD : 0",\
"Log history = INT : 0",\
"Frontend host = STRING : [32] ",\
"Frontend name = STRING : [32] ",\
"Frontend file name = STRING : [256] ",\
"Status = STRING : [256] ",\
"Status color = STRING : [32] ",\
"Hidden = BOOL : n",\
"",\
NULL }
#define MDPP16_ANALYSER_PARAMETERS_DEFINED

typedef struct {
  float      peak1_channel;
  float      peak1_energy;
  float      peak2_channel;
  float      peak2_energy;
  INT        LOWCHANNEL;
  INT        HIGHCHANNEL;
  float      update_interval;
} MDPP16_ANALYSER_PARAMETERS;

#define MDPP16_ANALYSER_PARAMETERS_STR(_name) const char *_name[] = {\
"[.]",\
"Peak1_Channel = FLOAT : 7000.0",\
"Peak1_Energy = FLOAT : 81.0",\
"Peak2_Channel = FLOAT : 30000.0",\
"Peak2_Energy = FLOAT : 356.0",\
"LOWCHANNEL = INT : 100",\
"HIGHCHANNEL = INT : 10000",\
"update_interval = FLOAT : 1.0",\
"",\
NULL }

#endif
