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

aux prepare_pvs 2
vgcreate $vg1 "$dev1" "$dev2"
lvcreate -n bar -l 1 $vg1

(echo | aux lvmetad_talk) || skip
aux lvmetad_dump | tee lvmetad.txt

grep $vg1 lvmetad.txt

vgremove -ff $vg1
