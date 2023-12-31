USE_TOYBOX(NEWTOY(toybox, NULL, TOYFLAG_STAYROOT))
USE_TRUE(OLDTOY(:, true, TOYFLAG_NOFORK))
USE_ACPI(NEWTOY(acpi, "abctV", TOYFLAG_USR|TOYFLAG_BIN))
USE_GROUPADD(OLDTOY(addgroup, groupadd, TOYFLAG_NEEDROOT|TOYFLAG_SBIN))
USE_USERADD(OLDTOY(adduser, useradd, TOYFLAG_NEEDROOT|TOYFLAG_UMASK|TOYFLAG_SBIN))
USE_ARP(NEWTOY(arp, "vi:nDsdap:A:H:[+Ap][!sd]", TOYFLAG_USR|TOYFLAG_BIN))
USE_ARPING(NEWTOY(arping, "<1>1s:I:w#<0c#<0AUDbqf[+AU][+Df]", TOYFLAG_USR|TOYFLAG_SBIN))
USE_BASE64(NEWTOY(base64, "diw#<1[!dw]", TOYFLAG_USR|TOYFLAG_BIN))
USE_BASENAME(NEWTOY(basename, "<1>2", TOYFLAG_USR|TOYFLAG_BIN))
USE_BLKID(NEWTOY(blkid, 0, TOYFLAG_BIN))
USE_BLOCKDEV(NEWTOY(blockdev, "<1>1(setro)(setrw)(getro)(getss)(getbsz)(setbsz)#<0(getsz)(getsize)(getsize64)(flushbufs)(rereadpt)",TOYFLAG_USR|TOYFLAG_BIN))
USE_BOOTCHARTD(NEWTOY(bootchartd, 0, TOYFLAG_STAYROOT|TOYFLAG_USR|TOYFLAG_BIN))
USE_BRCTL(NEWTOY(brctl, "<1", TOYFLAG_USR|TOYFLAG_SBIN))
USE_BUNZIP2(NEWTOY(bunzip2, "cftkv", TOYFLAG_USR|TOYFLAG_BIN))
USE_BZCAT(NEWTOY(bzcat, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_CAL(NEWTOY(cal, ">2", TOYFLAG_USR|TOYFLAG_BIN))
USE_CAT(NEWTOY(cat, "u"USE_CAT_V("vte"), TOYFLAG_BIN))
USE_CATV(NEWTOY(catv, USE_CATV("vte"), TOYFLAG_USR|TOYFLAG_BIN))
USE_SH(NEWTOY(cd, NULL, TOYFLAG_NOFORK))
USE_CHATTR(NEWTOY(chattr, NULL, TOYFLAG_BIN))
USE_CHCON(NEWTOY(chcon, "<2hvR", TOYFLAG_USR|TOYFLAG_BIN))
USE_CHGRP(NEWTOY(chgrp, "<2hPLHRfv[-HLP]", TOYFLAG_BIN))
USE_CHMOD(NEWTOY(chmod, "<2?vRf[-vf]", TOYFLAG_BIN))
USE_CHOWN(OLDTOY(chown, chgrp, TOYFLAG_BIN))
USE_CHROOT(NEWTOY(chroot, "^<1", TOYFLAG_USR|TOYFLAG_SBIN))
USE_CHVT(NEWTOY(chvt, "<1", TOYFLAG_USR|TOYFLAG_BIN))
USE_CKSUM(NEWTOY(cksum, "HIPLN", TOYFLAG_BIN))
USE_CLEAR(NEWTOY(clear, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_CMP(NEWTOY(cmp, "<2>2ls", TOYFLAG_USR|TOYFLAG_BIN))
USE_COMM(NEWTOY(comm, "<2>2321", TOYFLAG_USR|TOYFLAG_BIN))
USE_COMPRESS(NEWTOY(compress, "zcd9lrg[-cd][!zgLr]", TOYFLAG_USR|TOYFLAG_BIN))
USE_COUNT(NEWTOY(count, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_CP(NEWTOY(cp, "<2"USE_CP_PRESERVE("(preserve):;")"RHLPp"USE_CP_MORE("rdaslvnF(remove-destination)")"fi[-HLP"USE_CP_MORE("d")"]"USE_CP_MORE("[-ni]"), TOYFLAG_BIN))
USE_CPIO(NEWTOY(cpio, "mduH:p:|i|t|F:v(verbose)o|[!pio][!pot][!pF]", TOYFLAG_BIN))
USE_CROND(NEWTOY(crond, "fbSl#<0=8d#<0L:c:[-bf][-LS][-ld]", TOYFLAG_USR|TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_CRONTAB(NEWTOY(crontab, "c:u:elr[!elr]", TOYFLAG_USR|TOYFLAG_BIN|TOYFLAG_STAYROOT))
USE_CUT(NEWTOY(cut, "b:|c:|f:|d:sn[!cbf]", TOYFLAG_USR|TOYFLAG_BIN))
USE_DATE(NEWTOY(date, "d:D:r:u[!dr]", TOYFLAG_BIN))
USE_DD(NEWTOY(dd, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_DEALLOCVT(NEWTOY(deallocvt, ">1", TOYFLAG_USR|TOYFLAG_BIN|TOYFLAG_NEEDROOT))
USE_GROUPDEL(OLDTOY(delgroup, groupdel, TOYFLAG_NEEDROOT|TOYFLAG_SBIN))
USE_USERDEL(OLDTOY(deluser, userdel, TOYFLAG_NEEDROOT|TOYFLAG_SBIN))
USE_DF(NEWTOY(df, "HPkht*a[-HPkh]", TOYFLAG_SBIN))
USE_DHCP(NEWTOY(dhcp, "V:H:F:x*r:O*A#<0T#<0t#<0s:p:i:SBRCaovqnbf", TOYFLAG_SBIN|TOYFLAG_ROOTONLY))
USE_DHCPD(NEWTOY(dhcpd, ">1P#<0>65535=67fi:S46[!46]", TOYFLAG_SBIN|TOYFLAG_ROOTONLY))
USE_DIFF(NEWTOY(diff, "<2>2B(ignore-blank-lines)d(minimal)b(ignore-space-change)ut(expand-tabs)w(ignore-all-space)i(ignore-case)T(initial-tab)s(report-identical-files)q(brief)a(text)L(label)*S(starting-file):N(new-file)r(recursive)U(unified)#<0=3", TOYFLAG_USR|TOYFLAG_BIN))
USE_DIRNAME(NEWTOY(dirname, "<1", TOYFLAG_USR|TOYFLAG_BIN))
USE_DMESG(NEWTOY(dmesg, "trs#<1n#c[!tr]", TOYFLAG_BIN))
USE_DOS2UNIX(NEWTOY(dos2unix, 0, TOYFLAG_BIN))
USE_DU(NEWTOY(du, "d#<0hmlcaHkKLsx[-HL][-kKmh]", TOYFLAG_USR|TOYFLAG_BIN))
USE_DUMPLEASES(NEWTOY(dumpleases, ">0arf:[!ar]", TOYFLAG_USR|TOYFLAG_BIN))
USE_ECHO(NEWTOY(echo, "^?en", TOYFLAG_BIN))
USE_EGREP(OLDTOY(egrep, grep, TOYFLAG_BIN))
USE_EJECT(NEWTOY(eject, ">1stT[!tT]", TOYFLAG_USR|TOYFLAG_BIN))
USE_ENV(NEWTOY(env, "^i", TOYFLAG_USR|TOYFLAG_BIN))
USE_SH(NEWTOY(exit, NULL, TOYFLAG_NOFORK))
USE_EXPAND(NEWTOY(expand, "t*", TOYFLAG_USR|TOYFLAG_BIN|TOYFLAG_LOCALE))
USE_EXPR(NEWTOY(expr, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_FACTOR(NEWTOY(factor, 0, TOYFLAG_USR|TOYFLAG_BIN))
USE_FALLOCATE(NEWTOY(fallocate, ">1l#|", TOYFLAG_USR|TOYFLAG_BIN))
USE_FALSE(NEWTOY(false, NULL, TOYFLAG_BIN))
USE_FDISK(NEWTOY(fdisk, "C#<0H#<0S#<0b#<512ul", TOYFLAG_SBIN))
USE_FGREP(OLDTOY(fgrep, grep, TOYFLAG_BIN))
USE_FIND(NEWTOY(find, "?^HL[-HL]", TOYFLAG_USR|TOYFLAG_BIN))
USE_FOLD(NEWTOY(fold, "bsuw#<1", TOYFLAG_USR|TOYFLAG_BIN))
USE_FREE(NEWTOY(free, "tgmkb[!tgmkb]", TOYFLAG_USR|TOYFLAG_BIN))
USE_FREERAMDISK(NEWTOY(freeramdisk, "<1>1", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_FSCK(NEWTOY(fsck, "?t:ANPRTVsC#", TOYFLAG_USR|TOYFLAG_BIN))
USE_FSFREEZE(NEWTOY(fsfreeze, "<1>1f|u|[!fu]", TOYFLAG_USR|TOYFLAG_SBIN))
USE_FSTYPE(NEWTOY(fstype, "<1", TOYFLAG_BIN))
USE_FSYNC(NEWTOY(fsync, "<1d", TOYFLAG_BIN))
USE_FTPGET(NEWTOY(ftpget, "<2cvu:p:P#<0=21>65535", TOYFLAG_BIN))
USE_FTPGET(OLDTOY(ftpput, ftpget, TOYFLAG_BIN))
USE_GETENFORCE(NEWTOY(getenforce, ">0", TOYFLAG_USR|TOYFLAG_SBIN))
USE_GETPROP(NEWTOY(getprop, ">2", TOYFLAG_USR|TOYFLAG_SBIN))
USE_GETTY(NEWTOY(getty, "<2t#<0H:I:l:f:iwnmLh",TOYFLAG_SBIN))
USE_GREP(NEWTOY(grep, "ZzEFHabhinorsvwclqe*f*m#x[!wx][!EFw]", TOYFLAG_BIN))
USE_GROUPADD(NEWTOY(groupadd, "<1>2g#<0S", TOYFLAG_NEEDROOT|TOYFLAG_SBIN))
USE_GROUPDEL(NEWTOY(groupdel, "<1>2", TOYFLAG_NEEDROOT|TOYFLAG_SBIN))
USE_GROUPS(NEWTOY(groups, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_GUNZIP(NEWTOY(gunzip, "cflqStv", TOYFLAG_USR|TOYFLAG_BIN))
USE_GZIP(NEWTOY(gzip, USE_GZIP_D("d")"19dcflqStvgLRz[!gLRz]", TOYFLAG_USR|TOYFLAG_BIN))
USE_REBOOT(OLDTOY(halt, reboot, TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_HEAD(NEWTOY(head, "?n#<0=10", TOYFLAG_USR|TOYFLAG_BIN))
USE_HELLO(NEWTOY(hello, 0, TOYFLAG_USR|TOYFLAG_BIN))
USE_HELP(NEWTOY(help, ""USE_HELP_EXTRAS("ah"), TOYFLAG_BIN))
USE_HEXEDIT(NEWTOY(hexedit, "<1>1r", TOYFLAG_USR|TOYFLAG_BIN|TOYFLAG_LOCALE))
USE_HOST(NEWTOY(host, "<1>2avt:", TOYFLAG_USR|TOYFLAG_BIN))
USE_HOSTID(NEWTOY(hostid, ">0", TOYFLAG_USR|TOYFLAG_BIN))
USE_HOSTNAME(NEWTOY(hostname, NULL, TOYFLAG_BIN))
USE_HWCLOCK(NEWTOY(hwclock, ">0(fast)f(rtc):u(utc)l(localtime)t(systz)s(hctosys)r(show)w(systohc)[-ul][!rtsw]", TOYFLAG_USR|TOYFLAG_BIN))
USE_ICONV(NEWTOY(iconv, "cst:f:", TOYFLAG_USR|TOYFLAG_BIN))
USE_ID(NEWTOY(id, ">1"USE_ID_Z("Z")"nGgru[!"USE_ID_Z("Z")"Ggu]", TOYFLAG_USR|TOYFLAG_BIN))
USE_IFCONFIG(NEWTOY(ifconfig, "^?a", TOYFLAG_SBIN))
USE_INIT(NEWTOY(init, "", TOYFLAG_SBIN))
USE_INOTIFYD(NEWTOY(inotifyd, "<2", TOYFLAG_USR|TOYFLAG_BIN))
USE_INSMOD(NEWTOY(insmod, "<1", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_INSTALL(NEWTOY(install, "<1cdDpsvm:o:g:", TOYFLAG_USR|TOYFLAG_BIN))
USE_IONICE(NEWTOY(ionice, "^tc#<0>3=2n#<0>7=5p#", TOYFLAG_USR|TOYFLAG_BIN))
USE_IORENICE(NEWTOY(iorenice, "?<1>3", TOYFLAG_USR|TOYFLAG_BIN))
USE_IP(NEWTOY(ip, NULL, TOYFLAG_SBIN))
USE_IP(OLDTOY(ipaddr, ip, TOYFLAG_SBIN))
USE_IPCRM(NEWTOY(ipcrm, "m*M*s*S*q*Q*", TOYFLAG_USR|TOYFLAG_BIN))
USE_IPCS(NEWTOY(ipcs, "acptulsqmi#", TOYFLAG_USR|TOYFLAG_BIN))
USE_IP(OLDTOY(iplink, ip, TOYFLAG_SBIN))
USE_IP(OLDTOY(iproute, ip, TOYFLAG_SBIN))
USE_IP(OLDTOY(iprule, ip, TOYFLAG_SBIN))
USE_IP(OLDTOY(iptunnel, ip, TOYFLAG_SBIN))
USE_KILL(NEWTOY(kill, "?ls: ", TOYFLAG_BIN))
USE_KILLALL(NEWTOY(killall, "?s:lqvi", TOYFLAG_USR|TOYFLAG_BIN))
USE_KILLALL5(NEWTOY(killall5, "?o*ls: [!lo][!ls]", TOYFLAG_SBIN))
USE_KLOGD(NEWTOY(klogd, "c#<1>8n", TOYFLAG_SBIN))
USE_LAST(NEWTOY(last, "f:W", TOYFLAG_BIN))
USE_LINK(NEWTOY(link, "<2>2", TOYFLAG_USR|TOYFLAG_BIN))
USE_LN(NEWTOY(ln, "<1vnfs", TOYFLAG_BIN))
USE_LOAD_POLICY(NEWTOY(load_policy, "<1>1", TOYFLAG_USR|TOYFLAG_SBIN))
USE_LOGGER(NEWTOY(logger, "st:p:", TOYFLAG_USR|TOYFLAG_BIN))
USE_LOGIN(NEWTOY(login, ">1f:ph:", TOYFLAG_BIN|TOYFLAG_NEEDROOT))
USE_LOGNAME(NEWTOY(logname, ">0", TOYFLAG_USR|TOYFLAG_BIN))
USE_LOSETUP(NEWTOY(losetup, ">2S(sizelimit)#s(show)ro#j:fdca[!afj]", TOYFLAG_SBIN))
USE_LS(NEWTOY(ls, USE_LS_COLOR("(color):;")"ZgoACFHLRSacdfhiklmnpqrstux1[-Cxm1][-Cxml][-Cxmo][-Cxmg][-cu][-ftS][-HL]", TOYFLAG_BIN|TOYFLAG_LOCALE))
USE_LSATTR(NEWTOY(lsattr, "vldaR", TOYFLAG_BIN))
USE_LSMOD(NEWTOY(lsmod, NULL, TOYFLAG_SBIN))
USE_LSOF(NEWTOY(lsof, "lp:t", TOYFLAG_USR|TOYFLAG_BIN))
USE_LSPCI(NEWTOY(lspci, "emkn"USE_LSPCI_TEXT("@i:"), TOYFLAG_USR|TOYFLAG_BIN))
USE_LSUSB(NEWTOY(lsusb, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_MAKEDEVS(NEWTOY(makedevs, "<1>1d:", TOYFLAG_USR|TOYFLAG_BIN))
USE_MD5SUM(NEWTOY(md5sum, "b", TOYFLAG_USR|TOYFLAG_BIN))
USE_MDEV(NEWTOY(mdev, "s", TOYFLAG_USR|TOYFLAG_BIN|TOYFLAG_UMASK))
USE_MIX(NEWTOY(mix, "c:d:l#r#", TOYFLAG_USR|TOYFLAG_BIN))
USE_MKDIR(NEWTOY(mkdir, "<1"USE_MKDIR_Z("Z:")"vpm:", TOYFLAG_BIN|TOYFLAG_UMASK))
USE_MKE2FS(NEWTOY(mke2fs, "<1>2g:Fnqm#N#i#b#", TOYFLAG_SBIN))
USE_MKFIFO(NEWTOY(mkfifo, "<1"USE_MKFIFO_Z("Z:")"m:", TOYFLAG_USR|TOYFLAG_BIN))
USE_MKNOD(NEWTOY(mknod, "<2>4m(mode):"USE_MKNOD_Z("Z:"), TOYFLAG_BIN|TOYFLAG_UMASK))
USE_MKPASSWD(NEWTOY(mkpasswd, ">2S:m:P#=0<0", TOYFLAG_USR|TOYFLAG_BIN))
USE_MKSWAP(NEWTOY(mkswap, "<1>1", TOYFLAG_SBIN))
USE_MKTEMP(NEWTOY(mktemp, ">1qd(directory)p(tmpdir):", TOYFLAG_BIN))
USE_MODINFO(NEWTOY(modinfo, "<1b:k:F:0", TOYFLAG_BIN))
USE_MODPROBE(NEWTOY(modprobe, "alrqvsDb", TOYFLAG_SBIN))
USE_MORE(NEWTOY(more, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_MOUNT(NEWTOY(mount, "?O:afnrvwt:o*[-rw]", TOYFLAG_BIN|TOYFLAG_STAYROOT))
USE_MOUNTPOINT(NEWTOY(mountpoint, "<1qdx[-dx]", TOYFLAG_BIN))
USE_MV(NEWTOY(mv, "<2"USE_CP_MORE("vnF")"fi"USE_CP_MORE("[-ni]"), TOYFLAG_BIN))
USE_NBD_CLIENT(OLDTOY(nbd-client, nbd_client, TOYFLAG_USR|TOYFLAG_BIN))
USE_NBD_CLIENT(NEWTOY(nbd_client, "<3>3ns", 0))
USE_NETCAT(OLDTOY(nc, netcat, TOYFLAG_USR|TOYFLAG_BIN))
USE_NETCAT(NEWTOY(netcat, USE_NETCAT_LISTEN("^tlL")"w#p#s:q#f:", TOYFLAG_BIN))
USE_NETSTAT(NEWTOY(netstat, "pWrxwutneal", TOYFLAG_BIN))
USE_NICE(NEWTOY(nice, "^<1n#", TOYFLAG_USR|TOYFLAG_BIN))
USE_NL(NEWTOY(nl, "v#<1=1l#b:n:s:w#<0=6E", TOYFLAG_BIN))
USE_NOHUP(NEWTOY(nohup, "<1^", TOYFLAG_USR|TOYFLAG_BIN))
USE_NPROC(NEWTOY(nproc, "(all)", TOYFLAG_USR|TOYFLAG_BIN))
USE_NSENTER(NEWTOY(nsenter, "<1F(no-fork)t#<1(target)i:(ipc);m:(mount);n:(net);p:(pid);u:(uts);U:(user);", TOYFLAG_USR|TOYFLAG_BIN))
USE_OD(NEWTOY(od, "j#vN#xsodcbA:t*", TOYFLAG_USR|TOYFLAG_BIN))
USE_ONEIT(NEWTOY(oneit, "^<1nc:p3[!pn]", TOYFLAG_SBIN))
USE_OPENVT(NEWTOY(openvt, "c#<1>63sw", TOYFLAG_BIN|TOYFLAG_NEEDROOT))
USE_PARTPROBE(NEWTOY(partprobe, "<1", TOYFLAG_SBIN))
USE_PASSWD(NEWTOY(passwd, ">1a:dlu", TOYFLAG_STAYROOT|TOYFLAG_USR|TOYFLAG_BIN))
USE_PASTE(NEWTOY(paste, "d:s", TOYFLAG_BIN))
USE_PATCH(NEWTOY(patch, USE_TOYBOX_DEBUG("x")"ulp#i:R", TOYFLAG_USR|TOYFLAG_BIN))
USE_PGREP(NEWTOY(pgrep, "?P# s# xvonlf[!sP]", TOYFLAG_USR|TOYFLAG_BIN))
USE_PIDOF(NEWTOY(pidof, "<1so:", TOYFLAG_BIN))
USE_PING(NEWTOY(ping, "<1>1t#<0>255c#<0s#<0>65535I:W#<0w#<0q46[-46]", TOYFLAG_ROOTONLY|TOYFLAG_USR|TOYFLAG_BIN))
USE_PIVOT_ROOT(NEWTOY(pivot_root, "<2>2", TOYFLAG_SBIN))
USE_PGREP(OLDTOY(pkill, pgrep, TOYFLAG_USR|TOYFLAG_BIN))
USE_PMAP(NEWTOY(pmap, "<1xq", TOYFLAG_BIN))
USE_REBOOT(OLDTOY(poweroff, reboot, TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_PRINTENV(NEWTOY(printenv, "0(null)", TOYFLAG_USR|TOYFLAG_BIN))
USE_PRINTF(NEWTOY(printf, "<1?^", TOYFLAG_USR|TOYFLAG_BIN))
USE_PS(NEWTOY(ps, "aAdeflo*[!ol][+Ae]", TOYFLAG_USR|TOYFLAG_BIN))
USE_PWD(NEWTOY(pwd, ">0LP[-LP]", TOYFLAG_BIN))
USE_PWDX(NEWTOY(pwdx, "<1a", TOYFLAG_USR|TOYFLAG_BIN))
USE_READAHEAD(NEWTOY(readahead, NULL, TOYFLAG_BIN))
USE_READLINK(NEWTOY(readlink, "<1>1fenq[-fe]", TOYFLAG_USR|TOYFLAG_BIN))
USE_REALPATH(NEWTOY(realpath, "<1", TOYFLAG_USR|TOYFLAG_BIN))
USE_REBOOT(NEWTOY(reboot, "fn", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_RENICE(NEWTOY(renice, "<1gpun#|", TOYFLAG_USR|TOYFLAG_BIN))
USE_RESET(NEWTOY(reset, 0, TOYFLAG_USR|TOYFLAG_BIN))
USE_RESTORECON(NEWTOY(restorecon, "<1DFnRrv", TOYFLAG_USR|TOYFLAG_SBIN))
USE_REV(NEWTOY(rev, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_RFKILL(NEWTOY(rfkill, "<1>2", TOYFLAG_USR|TOYFLAG_SBIN))
USE_RM(NEWTOY(rm, "fiRr[-fi]", TOYFLAG_BIN))
USE_RMDIR(NEWTOY(rmdir, "<1p", TOYFLAG_BIN))
USE_RMMOD(NEWTOY(rmmod, "<1wf", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_ROUTE(NEWTOY(route, "?neA:", TOYFLAG_BIN))
USE_RUNCON(NEWTOY(runcon, "<2", TOYFLAG_USR|TOYFLAG_SBIN))
USE_SED(NEWTOY(sed, "(version)e*f*inEr[+Er]", TOYFLAG_USR|TOYFLAG_BIN|TOYFLAG_LOCALE))
USE_SEQ(NEWTOY(seq, "<1>3?f:s:", TOYFLAG_USR|TOYFLAG_BIN))
USE_SETENFORCE(NEWTOY(setenforce, "<1>1", TOYFLAG_USR|TOYFLAG_SBIN))
USE_SETPROP(NEWTOY(setprop, "<2>2", TOYFLAG_USR|TOYFLAG_SBIN))
USE_SETSID(NEWTOY(setsid, "^<1t", TOYFLAG_USR|TOYFLAG_BIN))
USE_SH(NEWTOY(sh, "c:i", TOYFLAG_BIN))
USE_SHA1SUM(NEWTOY(sha1sum, "b", TOYFLAG_USR|TOYFLAG_BIN))
USE_SHRED(NEWTOY(shred, "<1zxus#<1n#<1o#<0f", TOYFLAG_USR|TOYFLAG_BIN))
USE_SKELETON(NEWTOY(skeleton, "(walrus)(blubber):;(also):e@d*c#b:a", TOYFLAG_USR|TOYFLAG_BIN))
USE_SKELETON_ALIAS(NEWTOY(skeleton_alias, "b#dq", TOYFLAG_USR|TOYFLAG_BIN))
USE_SLEEP(NEWTOY(sleep, "<1", TOYFLAG_BIN))
USE_SORT(NEWTOY(sort, USE_SORT_FLOAT("g")USE_SORT_BIG("S:T:m" "o:k*t:xbMcszdfi") "run", TOYFLAG_USR|TOYFLAG_BIN))
USE_SPLIT(NEWTOY(split, ">2a#<1=2>9b#<1l#<1", TOYFLAG_USR|TOYFLAG_BIN))
USE_STAT(NEWTOY(stat, "c:f", TOYFLAG_BIN)) 
USE_STRINGS(NEWTOY(strings, "an#=4<1fo", TOYFLAG_USR|TOYFLAG_BIN))
USE_SU(NEWTOY(su, "lmpc:s:", TOYFLAG_BIN|TOYFLAG_ROOTONLY))
USE_SULOGIN(NEWTOY(sulogin, "t#<0=0", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_SWAPOFF(NEWTOY(swapoff, "<1>1", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_SWAPON(NEWTOY(swapon, "<1>1p#<0>32767", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))
USE_SWITCH_ROOT(NEWTOY(switch_root, "<2c:h", TOYFLAG_SBIN))
USE_SYNC(NEWTOY(sync, NULL, TOYFLAG_BIN))
USE_SYSCTL(NEWTOY(sysctl, "^neNqwpaA[!ap][!aq][!aw][+aA]", TOYFLAG_SBIN))
USE_SYSLOGD(NEWTOY(syslogd,">0l#<1>8=8R:b#<0>99=1s#<0=200m#<0>71582787=20O:p:f:a:nSKLD", TOYFLAG_SBIN|TOYFLAG_STAYROOT))
USE_TAC(NEWTOY(tac, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_TAIL(NEWTOY(tail, "?fc-n-[-cn]", TOYFLAG_USR|TOYFLAG_BIN))
USE_TAR(NEWTOY(tar, "&(no-recursion)(numeric-owner)(no-same-permissions)(overwrite)(exclude)*(to-command):o(no-same-owner)p(same-permissions)k(keep-old)c(create)|h(dereference)x(extract)|t(list)|v(verbose)z(gzip)O(to-stdout)m(touch)X(exclude-from)*T(files-from)*C(directory):f(file):[!txc]", TOYFLAG_USR|TOYFLAG_BIN))
USE_TASKSET(NEWTOY(taskset, "<1^pa", TOYFLAG_BIN|TOYFLAG_STAYROOT))
USE_TCPSVD(NEWTOY(tcpsvd, "^<3c#=30<1C:b#=20<0u:l:hEv", TOYFLAG_USR|TOYFLAG_BIN))
USE_TEE(NEWTOY(tee, "ia", TOYFLAG_USR|TOYFLAG_BIN))
USE_TELNET(NEWTOY(telnet, "<1>2", TOYFLAG_BIN))
USE_TELNETD(NEWTOY(telnetd, "w#<0b:p#<0>65535=23f:l:FSKi[!wi]", TOYFLAG_USR|TOYFLAG_BIN))
USE_TEST(NEWTOY(test, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_TEST_HUMAN_READABLE(NEWTOY(test_human_readable, "<1>1ibs", 0))
USE_TFTP(NEWTOY(tftp, "<1b#<8>65464r:l:g|p|[!gp]", TOYFLAG_USR|TOYFLAG_BIN))
USE_TFTPD(NEWTOY(tftpd, "rcu:l", TOYFLAG_BIN))
USE_TIME(NEWTOY(time, "<1^p", TOYFLAG_USR|TOYFLAG_BIN))
USE_TIMEOUT(NEWTOY(timeout, "<2^vk:s: ", TOYFLAG_BIN))
USE_TOP(NEWTOY(top, ">0d#=3n#<1mb", TOYFLAG_USR|TOYFLAG_BIN))
USE_TOUCH(NEWTOY(touch, "acd:mr:t:h[!dtr]", TOYFLAG_BIN))
USE_SH(OLDTOY(toysh, sh, TOYFLAG_BIN))
USE_TR(NEWTOY(tr, "^>2<1Ccsd[+cC]", TOYFLAG_USR|TOYFLAG_BIN))
USE_TRACEROUTE(NEWTOY(traceroute, "<1>2i:f#<1>255=1z#<0>86400=0g*w#<0>86400=5t#<0>255=0s:q#<1>255=3p#<1>65535=33434m#<1>255=30rvndlIUF64", TOYFLAG_STAYROOT|TOYFLAG_USR|TOYFLAG_BIN))
USE_TRACEROUTE(OLDTOY(traceroute6,traceroute, TOYFLAG_STAYROOT|TOYFLAG_USR|TOYFLAG_BIN))
USE_TRUE(NEWTOY(true, NULL, TOYFLAG_BIN))
USE_TRUNCATE(NEWTOY(truncate, "<1s:|c", TOYFLAG_BIN))
USE_TTY(NEWTOY(tty, "s", TOYFLAG_USR|TOYFLAG_BIN))
USE_TCPSVD(OLDTOY(udpsvd, tcpsvd, TOYFLAG_USR|TOYFLAG_BIN))
USE_UMOUNT(NEWTOY(umount, "ndDflrat*v[!na]", TOYFLAG_BIN|TOYFLAG_STAYROOT))
USE_UNAME(NEWTOY(uname, "oamvrns[+os]", TOYFLAG_BIN))
USE_UNIQ(NEWTOY(uniq, "f#s#w#zicdu", TOYFLAG_USR|TOYFLAG_BIN))
USE_UNIX2DOS(NEWTOY(unix2dos, 0, TOYFLAG_BIN))
USE_UNLINK(NEWTOY(unlink, "<1>1", TOYFLAG_USR|TOYFLAG_BIN))
USE_UNSHARE(NEWTOY(unshare, "<1^rimnpuU", TOYFLAG_USR|TOYFLAG_BIN))
USE_UPTIME(NEWTOY(uptime, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_USERADD(NEWTOY(useradd, "<1>2u#<0G:s:g:h:SDH", TOYFLAG_NEEDROOT|TOYFLAG_UMASK|TOYFLAG_SBIN))
USE_USERDEL(NEWTOY(userdel, "<1>1r", TOYFLAG_NEEDROOT|TOYFLAG_SBIN))
USE_USLEEP(NEWTOY(usleep, "<1", TOYFLAG_BIN))
USE_UUDECODE(NEWTOY(uudecode, ">1o:", TOYFLAG_USR|TOYFLAG_BIN|TOYFLAG_UMASK))
USE_UUENCODE(NEWTOY(uuencode, "<1>2m", TOYFLAG_USR|TOYFLAG_BIN))
USE_VCONFIG(NEWTOY(vconfig, "<2>4", TOYFLAG_NEEDROOT|TOYFLAG_SBIN))
USE_VMSTAT(NEWTOY(vmstat, ">2n", TOYFLAG_BIN))
USE_W(NEWTOY(w, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_WATCH(NEWTOY(watch, "^<1n#<0=2te", TOYFLAG_USR|TOYFLAG_BIN))
USE_WC(NEWTOY(wc, USE_TOYBOX_I18N("m")"cwl", TOYFLAG_USR|TOYFLAG_BIN|TOYFLAG_LOCALE))
USE_WHICH(NEWTOY(which, "<1a", TOYFLAG_USR|TOYFLAG_BIN))
USE_WHO(NEWTOY(who, "a", TOYFLAG_USR|TOYFLAG_BIN))
USE_WHOAMI(OLDTOY(whoami, logname, TOYFLAG_USR|TOYFLAG_BIN))
USE_XARGS(NEWTOY(xargs, "^I:E:L#ptxrn#<1s#0", TOYFLAG_USR|TOYFLAG_BIN))
USE_XXD(NEWTOY(xxd, ">1c#<1>4096=16l#g#<1=2", TOYFLAG_USR|TOYFLAG_BIN))
USE_XZCAT(NEWTOY(xzcat, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_YES(NEWTOY(yes, NULL, TOYFLAG_USR|TOYFLAG_BIN))
USE_ZCAT(NEWTOY(zcat, 0, TOYFLAG_USR|TOYFLAG_BIN))
