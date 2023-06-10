/*
 * hostapd / Interface client monitor
 * Copyright (c) 2016 Google, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */


#include "includes.h"
#include "common/defs.h"
#include "common/ieee802_11_defs.h"
#include "common/ieee802_11_common.h"
#include "common.h"
#include "monitor_sta.h"

struct monitor_sta_info_element {
	struct monitor_sta_info_element *next;
	struct monitor_sta_info_element *hnext;
	struct os_time timestamp; /* timestamp of sta entry to list */
	struct monitor_sta_info info;
};

struct monitor_sta {
        struct monitor_sta_info_element *mon_list;  /* STA info list head */
#define STA_HASH_SIZE 256
#define STA_HASH(sta) (sta[5])
        struct monitor_sta_info_element *mon_hash[STA_HASH_SIZE];
        u16 count; /* number of station entered to list */
	u32 hyst; /* configurable parameter, used to generate event */
};

/*
 * monitor_sta_get - get pointer to mon station
 * mon_sta: poiinter to station monitor structure
 * mac: mac address of station to be searched
 * return: pointer to monitor_sta_info_element
 */
static struct monitor_sta_info_element *monitor_sta_get(mon_sta_t mon_sta,
							const u8 *mac)
{
	struct monitor_sta_info_element *sta;

	sta = mon_sta->mon_hash[STA_HASH(mac)];
	while (sta != NULL && os_memcmp(sta->info.addr, mac, 6) != 0)
		sta = sta->next;
	return sta;
}

/*
 * montor_sta_list_del - delete station entry from list
 * mon_sta: pointer to station monitor structure
 * sta: pointer to monitor_sta_info_element
 * function remove station entry from mon station list
 * return: none
 */
static void monitor_sta_list_del(mon_sta_t mon_sta,
				 struct monitor_sta_info_element *sta)
{
	struct monitor_sta_info_element *tmp;

	if (mon_sta->mon_list == sta) {
		mon_sta->mon_list = sta->next;
		mon_sta->count--;
		return;
	}

	tmp = mon_sta->mon_list;
	while (tmp != NULL && tmp->next  != sta) {
		tmp = tmp->next;
	}

	if (tmp == NULL) {
		wpa_printf(MSG_DEBUG, "client monitor: list delete failed");
	} else {
		tmp->next = sta->next;
		mon_sta->count--;
	}

}

/*
 * monitor_sta_hash_del - delete hash entry
 * mon_sta: pointer to station monitor structure
 * sta: pointer to monitor_sta_info_element
 * function removes entry from hash list
 * return: none
 */
static void monitor_sta_hash_del(mon_sta_t mon_sta,
				 struct monitor_sta_info_element *sta)
{
	struct monitor_sta_info_element *tmp;

	tmp  = mon_sta->mon_hash[STA_HASH(sta->info.addr)];
	if (!tmp)
		return;

	if (os_memcmp(tmp->info.addr, sta->info.addr, 6) == 0) {
		mon_sta->mon_hash[STA_HASH(sta->info.addr)] = tmp->hnext;
		return;
	}

	while(tmp->hnext != NULL &&
			os_memcmp(tmp->hnext->info.addr,
				  sta->info.addr, 6)  != 0 )
		tmp = tmp->hnext;

	if (tmp->hnext != NULL)
		tmp->hnext = tmp->hnext->hnext;
	else
		wpa_printf(MSG_DEBUG, "client monitor: hash delete failed");
}

/*
 * monitor_sta_deinit - deinitialize client monitor
 * mon_sta: pointer to station monitor structure
 * function deinitialize client monitor module
 * return : none
 */
void monitor_sta_deinit(mon_sta_t mon_sta)
{
	struct monitor_sta_info_element *s1, *s2;

	if (!mon_sta)
		return;

	s1 = mon_sta->mon_list;
	while(s1) {
		s2 = s1;
		s1 = s1->next;
		monitor_sta_hash_del(mon_sta, s2);
		monitor_sta_list_del(mon_sta, s2);
		os_free(s2);
	}

	os_free(mon_sta);
}

/*
 * mon_sta_free - search and delete station
 * mon_sta: pointer to station monitor structure
 * function seaches for the station and delete's entry
 * return: none
 */
void monitor_sta_free(mon_sta_t mon_sta, const u8 *mac)
{
	struct monitor_sta_info_element *sta;

	if (!mon_sta)
		return;

	sta = monitor_sta_get(mon_sta, mac);
	if (sta == NULL) {
		wpa_printf(MSG_DEBUG, "client monitor: sta free failed for "
				MACSTR, MAC2STR(mac));
		return;
	}

	monitor_sta_hash_del(mon_sta, sta);
	monitor_sta_list_del(mon_sta, sta);
	os_free(sta);
}

/* monitor_sta_config_hysteresis - configure mon hysteresis
 * signal: signal hysteresis to be configured
 * return: 0 - Success, -1 - Failure
 */
int monitor_sta_set_hysteresis(mon_sta_t mon_sta, int hyst)
{
	if (!mon_sta)
		return -1;

	if (hyst < 0 || hyst > 100)
		return -1;

	mon_sta->hyst = hyst;

	return 0;
}

/*
 * monitor_sta_info_update - update mon structure info
 * mon_sta: pointer to station monitor structure
 * event: mon event
 * mac: mac address from which probe request received
 * signal: signal value of probe request
 * elems: ieee80211 elements
 * nss : number of spatial streams
 * function update data to station monitor structure on receiving probe request if
 * table has entry for it
 * return: TRUE - Success, FALSE - Failure to update/signal not changed
 */
Boolean monitor_sta_info_update(mon_sta_t mon_sta, mon_sta_event event,
				const u8 *mac, int signal,
				struct ieee802_11_elems elems, u16 *nss)
{
	struct monitor_sta_info_element *sta;
	struct os_time now;
	u32 hyst_diff = 0;

	if (!mon_sta)
		return FALSE;

	sta = monitor_sta_get(mon_sta, mac);
	if (sta == NULL) {
		wpa_printf(MSG_DEBUG, "client monitor: update failed for "
				"station " MACSTR, MAC2STR(mac));
		return FALSE;
	}

	os_get_time(&now);

	if (elems.vht_capabilities)
		*nss = ieee802_11_find_vht_nss(elems.vht_capabilities);
	else if (elems.ht_capabilities)
		*nss = ieee802_11_find_ht_nss(elems.ht_capabilities);

	os_memcpy(sta->info.addr, mac, 6);

	sta->info.nss = *nss;
	sta->info.mode = elems.vht_capabilities ? MODE_VHT :
				elems.ht_capabilities ? MODE_HT : MODE_LEGACY;
	sta->info.signal = signal;

	os_get_time(&(sta->info.timestamp));

	/* Need to do for the first time when sta added to list */
	if (!sta->info.last_event_signal)
		sta->info.last_event_signal = signal;

	hyst_diff = abs(abs(signal) - abs(sta->info.last_event_signal));
	if (hyst_diff >= mon_sta->hyst) {
		sta->info.last_event_signal = signal;
		return TRUE;
	}

	return FALSE;
}

/*
 * monitor_sta_hash_add - add station to hash list
 * mon_sta: pointer to station monitor structure
 * sta: pointer to monitor_sta_info_element
 * function makes entry to hash list
 * return: none
 */
static void monitor_sta_hash_add(mon_sta_t mon_sta,
				 struct monitor_sta_info_element *sta)
{
	sta->hnext = mon_sta->mon_hash[STA_HASH(sta->info.addr)];
	mon_sta->mon_hash[STA_HASH(sta->info.addr)] = sta;
}

/*
 * monitor_sta_del_oldest - delete oldest station
 * mon_sta: pointer to station monitor structure
 * function find's the oldest station from list
 * return: none
 */
static void monitor_sta_del_oldest(mon_sta_t mon_sta)
{
	struct monitor_sta_info_element *s1;
	u8 mac[6];
	os_time_t largest = 0;

	s1 = mon_sta->mon_list;
	while(s1) {
		if (s1->timestamp.sec > largest) {
			largest = s1->timestamp.sec;
			os_memcpy(mac, s1->info.addr, 6);
		}

		s1 = s1->next;
	}

	monitor_sta_free(mon_sta, mac);
}

/* monitor_sta_add - add station to list
 * mon_sta: pointer to station monitor structure
 * mac : mac address of station to be entered to the list
 * function makes sta entry to sta list on receiving iapp notification
 * if sta entry not exists
 * return : TRUE - Success, FALSE - Failure
 */
Boolean monitor_sta_add(mon_sta_t mon_sta, const u8 *mac)
{
	struct monitor_sta_info_element *sta;

	if (!mon_sta)
		return FALSE;

	sta = monitor_sta_get(mon_sta, mac);
	if (sta) {
		os_get_time(&(sta->timestamp));
		return TRUE;
	}

	if (mon_sta->count >= MAX_MONITOR_STA)
		monitor_sta_del_oldest(mon_sta);

	sta = os_zalloc(sizeof(struct monitor_sta_info_element));
	if (sta == NULL) {
		wpa_printf(MSG_ERROR, "client monitor: malloc failure");
		return FALSE;
	}

	os_memcpy(sta->info.addr, mac, 6);
	sta->next = mon_sta->mon_list;
	mon_sta->mon_list = sta;
	mon_sta->count++;

	monitor_sta_hash_add(mon_sta, sta);

	os_get_time(&(sta->timestamp));

	return TRUE;
}

/*
 * monitor_sta_init - initializes client monitor
 * function initializes monitor structure for each radio interface
 * return : pointer to monitor_sta - Success, NULL - Failure
 */
mon_sta_t monitor_sta_init()
{
	struct monitor_sta *mon_sta;

	mon_sta  = os_zalloc(sizeof(*mon_sta));
	if (mon_sta == NULL) {
		wpa_printf(MSG_ERROR, "client monitor: malloc failure");
		return NULL;
	}

	mon_sta->hyst = MONITOR_STA_RSSI_HYSTERESIS;

	return mon_sta;
}
