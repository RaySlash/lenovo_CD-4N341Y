#!/bin/sh
# Copyright (C) 2009 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

SKIP_WITH_LVMLOCKD=1
SKIP_WITH_LVMPOLLD=1

. lib/inittest

aux prepare_vg 4

lvcreate -an -Zn -l 1 -n $lv1 $vg
lvcreate -an -Zn -l 2 --type mirror -m 1 -n $lv2 $vg

vgcfgbackup -f bak0 $vg
sed -e 's,striped,unstriped,;s,mirror,unmirror,' -i.orig bak0
vgcfgrestore -f bak0 $vg

# we have on-disk metadata with unknown segments now
not lvchange -aey $vg/$lv1 # check that activation is refused

# try once more to catch invalid memory access with valgrind
# when clvmd flushes cmd mem pool
not lvchange -aey $vg/$lv1 # check that activation is refused

vgcfgbackup -f bak1 $vg
cat bak1
sed -e 's,unstriped,striped,;s,unmirror,mirror,' -i.orig bak1
vgcfgrestore -f bak1 $vg
vgcfgbackup -f bak2 $vg

egrep -v 'description|seqno|creation_time|Generated' < bak0.orig > a
egrep -v 'description|seqno|creation_time|Generated' < bak2 > b
diff -u a b

vgremove -ff $vg
