/*
 * Copyright (C) 2005-2015 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib.h"
#include "dmeventd_lvm.h"
#include "libdevmapper-event.h"

struct dso_state {
	struct dm_pool *mem;
	char cmd_lvscan[512];
	char cmd_lvconvert[512];
	int failed;
};

DM_EVENT_LOG_FN("raid")

/* FIXME Reformat to 80 char lines. */

static int _process_raid_event(struct dso_state *state, char *params, const char *device)
{
	struct dm_status_raid *status;
	const char *d;

	if (!dm_get_status_raid(state->mem, params, &status)) {
		log_error("Failed to process status line for %s.", device);
		return 0;
	}

	if ((d = strchr(status->dev_health, 'D'))) {
		if (state->failed)
			goto out; /* already reported */

		log_error("Device #%d of %s array, %s, has failed.",
			  (int)(d - status->dev_health),
			  status->raid_type, device);

		state->failed = 1;
		if (!dmeventd_lvm2_run_with_lock(state->cmd_lvscan))
			log_warn("WARNING: Re-scan of RAID device %s failed.", device);

		/* if repair goes OK, report success even if lvscan has failed */
		if (!dmeventd_lvm2_run_with_lock(state->cmd_lvconvert)) {
			log_info("Repair of RAID device %s failed.", device);
			dm_pool_free(state->mem, status);
			return 0;
		}
	} else {
		state->failed = 0;
		log_info("%s array, %s, is %s in-sync.",
			 status->raid_type, device,
			 (status->insync_regions == status->total_regions) ? "now" : "not");
	}
out:
	dm_pool_free(state->mem, status);

	return 1;
}

void process_event(struct dm_task *dmt,
		   enum dm_event_mask event __attribute__((unused)),
		   void **user)
{
	struct dso_state *state = *user;
	void *next = NULL;
	uint64_t start, length;
	char *target_type = NULL;
	char *params;
	const char *device = dm_task_get_name(dmt);

	do {
		next = dm_get_next_target(dmt, next, &start, &length,
					  &target_type, &params);

		if (!target_type) {
			log_info("%s mapping lost.", device);
			continue;
		}

		if (strcmp(target_type, "raid")) {
			log_info("%s has non-raid portion.", device);
			continue;
		}

		if (!_process_raid_event(state, params, device))
			log_error("Failed to process event for %s.",
				  device);
	} while (next);
}

int register_device(const char *device,
		    const char *uuid __attribute__((unused)),
		    int major __attribute__((unused)),
		    int minor __attribute__((unused)),
		    void **user)
{
	struct dso_state *state;

	if (!dmeventd_lvm2_init_with_pool("raid_state", state))
		goto_bad;

	if (!dmeventd_lvm2_command(state->mem, state->cmd_lvscan, sizeof(state->cmd_lvscan),
				   "lvscan --cache", device) ||
	    !dmeventd_lvm2_command(state->mem, state->cmd_lvconvert, sizeof(state->cmd_lvconvert),
				   "lvconvert --config devices{ignore_suspended_devices=1} "
				   "--repair --use-policies", device)) {
		dmeventd_lvm2_exit_with_pool(state);
		goto_bad;
	}

	*user = state;

	log_info("Monitoring RAID device %s for events.", device);

	return 1;
bad:
	log_error("Failed to monitor RAID %s.", device);

	return 0;
}

int unregister_device(const char *device,
		      const char *uuid __attribute__((unused)),
		      int major __attribute__((unused)),
		      int minor __attribute__((unused)),
		      void **user)
{
	struct dso_state *state = *user;

	dmeventd_lvm2_exit_with_pool(state);
	log_info("No longer monitoring RAID device %s for events.",
		 device);

	return 1;
}
