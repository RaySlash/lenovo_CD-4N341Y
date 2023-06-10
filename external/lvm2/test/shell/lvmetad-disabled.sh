#!/bin/sh
# Copyright (C) 2012 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

SKIP_WITH_LVMLOCKD=1
SKIP_WITHOUT_LVMETAD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_devs 2

kill $(< LOCAL_LVMETAD)
while test -e "$TESTDIR/lvmetad.socket"; do echo -n .; sleep .1; done # wait for the socket close
test ! -e "$LVM_LVMETAD_PIDFILE"

aux lvmconf "global/use_lvmetad = 0"

pvcreate --metadatatype 1 "$dev1"
pvcreate "$dev2"
vgcreate -M1 $vg1 "$dev1"
vgcreate $vg2 "$dev2"

pvs 2>&1 | tee out
grep "$dev1" out
grep "$dev2" out

vgs 2>&1 | tee out
grep $vg1 out
grep $vg2 out

aux lvmconf "global/use_lvmetad = 1"
lvmetad
while ! test -e "$TESTDIR/lvmetad.socket"; do echo -n .; sleep .1; done # wait for the socket
test -e "$LVM_LVMETAD_PIDFILE"
cp "$LVM_LVMETAD_PIDFILE" LOCAL_LVMETAD

pvscan --cache 2>&1 | tee out
grep "WARNING: Disabling lvmetad cache" out

pvs  2>&1 | tee out
grep "$dev1" out
grep "$dev2" out
grep "WARNING: Not using lvmetad" out

vgs  2>&1 | tee out
grep $vg1 out
grep $vg2 out
grep "WARNING: Not using lvmetad" out

vgremove $vg1 2>&1 | tee out
grep "WARNING: Not using lvmetad" out

pvremove "$dev1" 2>&1 | tee out
grep "WARNING: Not using lvmetad" out

pvscan --cache  2>&1 | tee out
not grep "WARNING: Disabling lvmetad cache" out

pvs 2>&1 | tee out
not grep "$dev1" out
grep "$dev2" out
not grep "WARNING: Not using lvmetad" out

vgs 2>&1 | tee out
not grep $vg1 out
grep $vg2 out
not grep "WARNING: Not using lvmetad" out

pvs --config 'global/use_lvmetad=0' 2>&1 | tee out
not grep "$dev1" out
grep "$dev2" out
grep "WARNING: Not using lvmetad" out
grep "use_lvmetad=0" out

vgs --config 'global/use_lvmetad=0' 2>&1 | tee out
not grep $vg1 out
grep $vg2 out
grep "WARNING: Not using lvmetad" out
grep "use_lvmetad=0" out

