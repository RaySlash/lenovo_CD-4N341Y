#!/bin/sh
# Copyright (C) 2014 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Test autoextension of thin metadata volume

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

export LVM_TEST_THIN_REPAIR_CMD=${LVM_TEST_THIN_REPAIR_CMD-/bin/false}

. lib/inittest

meta_percent_() {
	get lv_field $vg/pool metadata_percent | cut -d. -f1
}

wait_for_change_() {
	# dmeventd only checks every 10 seconds :(
	for i in $(seq 1 12) ; do
		test "$(meta_percent_)" -lt "$1" && return
		sleep 1
	done

	return 1  # timeout
}

#
# Temporary solution to create some occupied thin metadata
# This heavily depends on thin metadata output format to stay as is.
# Currently it expects 2MB thin metadata and 200MB data volume size
# Argument specifies how many devices should be created.
fake_metadata_() {
	echo '<superblock uuid="" time="0" transaction="'$2'" data_block_size="128" nr_data_blocks="3200">'
	echo ' <device dev_id="1" mapped_blocks="0" transaction="0" creation_time="0" snap_time="0">'
	echo ' </device>'
	echo ' <device dev_id="2" mapped_blocks="0" transaction="0" creation_time="0" snap_time="0">'
	echo ' </device>'
	for i in $(seq 10 $1)
	do
		echo ' <device dev_id="'$i'" mapped_blocks="30" transaction="0" creation_time="0" snap_time="0">'
		echo '  <range_mapping origin_begin="0" data_begin="0" length="29" time="0"/>'
		echo ' </device>'
		set +x
	done
	echo "</superblock>"
	set -x
}

test -n "$LVM_TEST_THIN_RESTORE_CMD" || LVM_TEST_THIN_RESTORE_CMD=$(which thin_restore) || skip
"$LVM_TEST_THIN_RESTORE_CMD" -V || skip
aux have_thin 1 10 0 || skip

aux prepare_dmeventd

aux lvmconf "activation/thin_pool_autoextend_percent = 10" \
	    "activation/thin_pool_autoextend_threshold = 70"

aux prepare_pvs 3 256

vgcreate -s 1M $vg $(cat DEVICES)

# Testing dmeventd autoresize
lvcreate -L200M -V500M -n thin -T $vg/pool 2>&1 | tee out
not grep "WARNING: Sum" out
lvcreate -V2M -n thin2 $vg/pool
lvcreate -L2M -n $lv1 $vg
lvchange -an $vg/thin $vg/thin2 $vg/pool

# Prepare some fake metadata with unmatching id
# Transaction_id is lower by 1 and there are no messages -> ERROR
fake_metadata_ 10 0 >data
"$LVM_TEST_THIN_RESTORE_CMD" -i data -o "$DM_DEV_DIR/mapper/$vg-$lv1"
lvconvert -y --thinpool $vg/pool --poolmetadata $vg/$lv1
not vgchange -ay $vg 2>&1 | tee out
grep expected out

check inactive $vg pool_tmeta

# Transaction_id is higher by 1
fake_metadata_ 10 3 >data
"$LVM_TEST_THIN_RESTORE_CMD" -i data -o "$DM_DEV_DIR/mapper/$vg-$lv1"
lvconvert -y --thinpool $vg/pool --poolmetadata $vg/$lv1
not vgchange -ay $vg 2>&1 | tee out
grep expected out

check inactive $vg pool_tmeta

# Prepare some fake metadata prefilled to ~81% (>70%)
fake_metadata_ 400 2 >data
"$LVM_TEST_THIN_RESTORE_CMD" -i data -o "$DM_DEV_DIR/mapper/$vg-$lv1"

# Swap volume with restored fake metadata
lvconvert -y --chunksize 64k --thinpool $vg/pool --poolmetadata $vg/$lv1

vgchange -ay $vg

# Check dmeventd resizes metadata via timeout (nothing is written to pool)
pre=$(meta_percent_)
wait_for_change_ $pre

lvchange -an $vg

#
fake_metadata_ 350 2 >data
lvchange -ay $vg/$lv1
"$LVM_TEST_THIN_RESTORE_CMD" -i data -o "$DM_DEV_DIR/mapper/$vg-$lv1"

lvconvert -y --chunksize 64k --thinpool $vg/pool --poolmetadata $vg/$lv1
lvchange -ay $vg/pool  $vg/$lv1
lvs -a $vg

lvcreate -s -Ky -n $lv2 $vg/thin
pre=$(meta_percent_)

# go over thin metadata threshold
echo 2 >"$DM_DEV_DIR/mapper/$vg-$lv2"

wait_for_change_ $pre

lvs -a $vg

vgremove -f $vg
