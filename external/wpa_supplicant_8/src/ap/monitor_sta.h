/*
 * hostapd / Interface client monitor
 * Copyright (c) 2016 Google, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef CLIENT_MONITOR_H
#define CLIENT_MONITOR_H

#define MAX_MONITOR_STA 100
#define INVALID_STEER_SIGNAL 0
#define INVALID_NSS 255
#define MONITOR_STA_RSSI_HYSTERESIS 10

/*
 * montor_sta_info :  monitor info per station
 */
struct monitor_sta_info{
	u8 addr[6]; /* station mac address */
	int signal; /* signal strength of probe request */
	u8 nss; /* number of spatial streams */
	u8 mode; /* HT/VHT/Legacy */
	struct os_time timestamp; /* timestamp of last probe received */
	int last_event_signal; /* signal when last event generated */
	/* flag represents station status such as connected,
	 * disconnected, probe request received, iapp notifed (other ap)
	 * and the same will be used to update monitor station info
	 */
	u8 flag;
};

typedef struct monitor_sta *mon_sta_t;

/* mon events */
typedef enum  {
       PROBE_REQ, /* indicates probe req received */
       CONNECTED, /* indicates successful connection */
       DISCONNECTED, /* indicates disconnection */
       OTHER_AP_CONNECTED /* indicates iapp notification */
} mon_sta_event;

/* supported mode i.e legacy, HT or VHT */
typedef enum {
	MODE_UNKNOWN,
	MODE_HT,
	MODE_VHT,
	MODE_LEGACY
}mode;


/*
 *  monitor_sta_deinit - deinitialize client monitor
 */
void monitor_sta_deinit(mon_sta_t mon_sta);

/*
 * monitor_sta_free - del and free mon station entry
 */
void monitor_sta_free(mon_sta_t mon_sta,  const u8 *mac);

/* monitor_sta_set_hyst - configure monitor hysteresis*/
int monitor_sta_set_hysteresis(mon_sta_t mon_sta, int hyst);

/*
 * monitor_sta_info_update - update mon structure info
 */
Boolean monitor_sta_info_update(mon_sta_t mon_sta, mon_sta_event event,
				const u8 *mac, int signal,
				struct ieee802_11_elems elems, u16 *nss);

/* monitor_sta_add - add station to list */
Boolean monitor_sta_add(mon_sta_t mon_sta, const u8 *mac);

/*
 * monitor_sta_init - initializes client monitoring
 */
mon_sta_t monitor_sta_init();

#endif
