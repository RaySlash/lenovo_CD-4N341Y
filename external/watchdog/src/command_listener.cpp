#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#if USE_SYSLOG
#include <pwd.h>
#include <sys/types.h>
#include <syslog.h>
#endif

#include <sysutils/SocketClient.h>
#include <bootloader.h>

#include "command_listener.h"
#include "sync_fts_prop.h"
#include "watchdog_service.h"

// Reboot command format
//      "reboot now"
//      "reboot set FDR"
//      "reboot set OTA"
//
// Reboot response format
//      success:    "000 Success"
//      failure:    "xxx Failure yyyy"
//                      xxx is 3-digit error code (255 = -1)
//                      yyyy is error string generated by strerror().

namespace {
class RebootServiceCommand : public FrameworkCommand {
public:
    explicit RebootServiceCommand(const char *cmd);
    virtual ~RebootServiceCommand() {}
    virtual int runCommand(SocketClient *c, int argc, char ** argv);

private:
    int sendResponse(SocketClient *cli, int ret) const;
#if USE_SYSLOG
    void printCommand(SocketClient *cli, int argc, char *argv[]) const;
#endif
    int rebootNow(SocketClient *cli);
    int doRecovery(const char *cmd);
    int writeRecoveryParameter() const;

    bool rebooting_;
    bool set_fdr_for_next_reboot_;
    bool set_ota_for_next_reboot_;
};

// command format and command file for /cache/recovery
const char OTA_PARAMETER[] = "--update_package=/cache/ota.zip";
const char FDR_PARAMETER[] = "--wipe_data";
const char OTA_FDR_PARAMETER[] = "--update_package=/cache/ota.zip\n--wipe_data";


}
// stub function to suppress compilation error. It should never be called.
// librecovery calls set_bootloader_message_default(), which is defined in
// bootable/recovery/bootlaoder.c. It's a fallback implementation when fts
// is not available.
extern "C" int set_bootloader_message_default(
        const struct bootloader_message* /* in */) {
#if USE_SYSLOG
    syslog(LOG_ERR, "should use fts and never come here");
#endif
    // Some system doens't support fts, but that is OK.
    return 0;
}

// commandlistener to socket /dev/socket/watchdog
CommandListener::CommandListener() :
                 FrameworkListener("watchdog") {
    registerCmd(new RebootServiceCommand("reboot"));
}

RebootServiceCommand::RebootServiceCommand(const char *cmd)
    : FrameworkCommand(cmd),
      rebooting_(false),
      set_fdr_for_next_reboot_(false),
      set_ota_for_next_reboot_(false) {
}

int RebootServiceCommand::runCommand(SocketClient *cli,
                   int argc, char **argv) {
    int ret;

#if USE_SYSLOG
    printCommand(cli, argc, argv);
#endif

    if (argc < 2 || argc > 3) {
        sendResponse(cli, -EINVAL);
        return 0;
    }

    if (rebooting_) {
        syslog(LOG_INFO, "already rebooting");
        sendResponse(cli, 0);
        return 0;
    }

    if (argc == 2) {
        // "reboot now"
        if (!strcmp("now", argv[1])) {
            return rebootNow(cli);
        } else {
            return sendResponse(cli, -EINVAL);
        }
    } else if (argc == 3) {
        // "reboot set FDR"
        // "reboot set OTA"
        if (!strcmp("set", argv[1]) &&
            (!strcmp("FDR", argv[2]) || !strcmp("OTA", argv[2]))) {
            ret = doRecovery(argv[2]);
            return sendResponse(cli, ret);
        } else {
            return sendResponse(cli, -EINVAL);
        }
    } else {
        return sendResponse(cli, -EINVAL);
    }
}

int RebootServiceCommand::sendResponse(SocketClient *cli, int ret) const {
    if (ret) {
        if (cli->sendMsg(ret, "Failure", true)) {
            return -1;
        }
    } else {
        if (cli->sendMsg(ret, "Success", false)) {
            return -1;
        }
    }
    return 0;
}

#if USE_SYSLOG
void RebootServiceCommand::printCommand(SocketClient *cli, int argc,
                                        char *argv[]) const {
    int i;
    pid_t pid = cli->getPid();
    uid_t uid = cli->getUid();
    struct passwd *pwd = getpwuid(uid);

    syslog(LOG_INFO, "received command from process %d, uid %d %s.",
           pid, uid, pwd ? pwd->pw_name : "unknown");
    for (i = 0; i < argc; ++i) {
        syslog(LOG_INFO, "       %s", argv[i]);
    }
}
#endif

int RebootServiceCommand::rebootNow(SocketClient *cli) {
    // reboot is guaranteed to succeed.
    // send response first.
    int ret = sendResponse(cli, 0);
    ret =  reboot_service_request();
    rebooting_ = true;
    return ret;
}

// handler for "reboot set FDR" and "reboot set OTA" command.
// both boots in to recovery mode next time.
int RebootServiceCommand::doRecovery(const char *cmd) {
    if (!cmd) {
#if USE_SYSLOG
        syslog(LOG_ERR, "doRecovery cmd is NULL");
#endif
        return -1;
    }

#if USE_SYSLOG
    syslog(LOG_INFO, "reboot service: recovery cmd is %s", cmd);
#endif

    if (!strcmp("FDR", cmd)) {
        set_fdr_for_next_reboot_ = true;
        increment_fdr_count();
    } else if (!strcmp("OTA", cmd)) {
        set_ota_for_next_reboot_ = true;
    } else {
        return -1;
    }

    if (writeRecoveryParameter()) {
        return -1;
    }
    return 0;
}

// write boot reocvery parameters to bootloader
int RebootServiceCommand::writeRecoveryParameter() const {
    struct bootloader_message boot;
    memset(&boot, 0, sizeof(boot));

    const char* param;
    if (set_fdr_for_next_reboot_ && set_ota_for_next_reboot_) {
      param = OTA_FDR_PARAMETER;
    } else if (set_fdr_for_next_reboot_ && !set_ota_for_next_reboot_) {
      param = FDR_PARAMETER;
    } else if (set_ota_for_next_reboot_ && !set_fdr_for_next_reboot_) {
      param = OTA_PARAMETER;
    } else {
      return -1;
    }

    strncpy(boot.command, "boot-recovery", sizeof(boot.command) - 1);
    snprintf(boot.recovery, sizeof(boot.recovery) - 1, "recovery\n%s\n", param);
    return set_bootloader_message(&boot);
}