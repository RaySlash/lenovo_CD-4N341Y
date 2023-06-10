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

# Test 'Found duplicate' is shown
SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 3

pvcreate "$dev1"
UUID1=$(get pv_field "$dev1" uuid)
pvcreate --config "devices{filter=[\"a|$dev2|\",\"r|.*|\"]} global/use_lvmetad=0" -u "$UUID1" --norestorefile "$dev2"
pvcreate --config "devices{filter=[\"a|$dev3|\",\"r|.*|\"]} global/use_lvmetad=0" -u "$UUID1" --norestorefile "$dev3"

pvscan --cache 2>&1 | tee out

if test -e LOCAL_LVMETAD; then
	grep "was already found" out
	grep "WARNING: Disabling lvmetad cache which does not support duplicate PVs." out
fi

pvs -o+uuid 2>&1 | tee out

grep    WARNING out > warn || true
grep -v WARNING out > main || true

test $(grep $UUID1 main | wc -l) -eq 1

COUNT=$(grep --count "was already found" warn)
[ "$COUNT" -eq 2 ]

pvs -o+uuid --config "devices{filter=[\"a|$dev2|\",\"r|.*|\"]}" 2>&1 | tee out

rm warn main || true
grep    WARNING out > warn || true
grep -v WARNING out > main || true

not grep "$dev1" main
grep "$dev2" main
not grep "$dev3" main

not grep "was already found" warn

