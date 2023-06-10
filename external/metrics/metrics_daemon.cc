// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "metrics_daemon.h"

#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <base/file_util.h>
#include <base/logging.h>
#include <base/string_util.h>
#include <base/stringprintf.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "counter.h"

using base::Time;
using base::TimeDelta;
using base::TimeTicks;
using std::map;
using std::string;
using std::vector;


#define SAFE_MESSAGE(e) (e.message ? e.message : "unknown error")
#define DBUS_IFACE_CRASH_REPORTER "org.chromium.CrashReporter"
#define DBUS_IFACE_POWER_MANAGER "org.chromium.PowerManager"
#define DBUS_IFACE_SESSION_MANAGER "org.chromium.SessionManagerInterface"

static const int kSecondsPerMinute = 60;
static const int kMinutesPerHour = 60;
static const int kHoursPerDay = 24;
static const int kMinutesPerDay = kHoursPerDay * kMinutesPerHour;
static const int kSecondsPerDay = kSecondsPerMinute * kMinutesPerDay;
static const int kDaysPerWeek = 7;
static const int kSecondsPerWeek = kSecondsPerDay * kDaysPerWeek;

// The daily use monitor is scheduled to a 1-minute interval after
// initial user activity and then it's exponentially backed off to
// 10-minute intervals. Although not required, the back off is
// implemented because the histogram buckets are spaced exponentially
// anyway and to avoid too frequent metrics daemon process wake-ups
// and file I/O.
static const int kUseMonitorIntervalInit = 1 * kSecondsPerMinute;
static const int kUseMonitorIntervalMax = 10 * kSecondsPerMinute;

const char kKernelCrashDetectedFile[] = "/tmp/kernel-crash-detected";
static const char kUncleanShutdownDetectedFile[] =
      "/tmp/unclean-shutdown-detected";

// static metrics parameters
const char MetricsDaemon::kMetricDailyUseTimeName[] =
    "Logging.DailyUseTime";
const int MetricsDaemon::kMetricDailyUseTimeMin = 1;
const int MetricsDaemon::kMetricDailyUseTimeMax = kMinutesPerDay;
const int MetricsDaemon::kMetricDailyUseTimeBuckets = 50;

// crash interval metrics
const char MetricsDaemon::kMetricKernelCrashIntervalName[] =
    "Logging.KernelCrashInterval";
const char MetricsDaemon::kMetricUncleanShutdownIntervalName[] =
    "Logging.UncleanShutdownInterval";
const char MetricsDaemon::kMetricUserCrashIntervalName[] =
    "Logging.UserCrashInterval";

const int MetricsDaemon::kMetricCrashIntervalMin = 1;
const int MetricsDaemon::kMetricCrashIntervalMax =
    4 * kSecondsPerWeek;
const int MetricsDaemon::kMetricCrashIntervalBuckets = 50;

// crash frequency metrics
const char MetricsDaemon::kMetricAnyCrashesDailyName[] =
    "Logging.AnyCrashesDaily";
const char MetricsDaemon::kMetricAnyCrashesWeeklyName[] =
    "Logging.AnyCrashesWeekly";
const char MetricsDaemon::kMetricKernelCrashesDailyName[] =
    "Logging.KernelCrashesDaily";
const char MetricsDaemon::kMetricKernelCrashesWeeklyName[] =
    "Logging.KernelCrashesWeekly";
const char MetricsDaemon::kMetricUncleanShutdownsDailyName[] =
    "Logging.UncleanShutdownsDaily";
const char MetricsDaemon::kMetricUncleanShutdownsWeeklyName[] =
    "Logging.UncleanShutdownsWeekly";
const char MetricsDaemon::kMetricUserCrashesDailyName[] =
    "Logging.UserCrashesDaily";
const char MetricsDaemon::kMetricUserCrashesWeeklyName[] =
    "Logging.UserCrashesWeekly";
const char MetricsDaemon::kMetricCrashFrequencyMin = 1;
const char MetricsDaemon::kMetricCrashFrequencyMax = 100;
const char MetricsDaemon::kMetricCrashFrequencyBuckets = 50;

// disk stats metrics

// The {Read,Write}Sectors numbers are in sectors/second.
// A sector is usually 512 bytes.

const char MetricsDaemon::kMetricReadSectorsLongName[] =
    "Platform.ReadSectorsLong";
const char MetricsDaemon::kMetricWriteSectorsLongName[] =
    "Platform.WriteSectorsLong";
const char MetricsDaemon::kMetricReadSectorsShortName[] =
    "Platform.ReadSectorsShort";
const char MetricsDaemon::kMetricWriteSectorsShortName[] =
    "Platform.WriteSectorsShort";

const int MetricsDaemon::kMetricStatsShortInterval = 1;  // seconds
const int MetricsDaemon::kMetricStatsLongInterval = 30;  // seconds

const int MetricsDaemon::kMetricMeminfoInterval = 30;        // seconds

// Assume a max rate of 250Mb/s for reads (worse for writes) and 512 byte
// sectors.
const int MetricsDaemon::kMetricSectorsIOMax = 500000;  // sectors/second
const int MetricsDaemon::kMetricSectorsBuckets = 50;    // buckets
// Page size is 4k, sector size is 0.5k.  We're not interested in page fault
// rates that the disk cannot sustain.
const int MetricsDaemon::kMetricPageFaultsMax = kMetricSectorsIOMax / 8;
const int MetricsDaemon::kMetricPageFaultsBuckets = 50;

// Major page faults, i.e. the ones that require data to be read from disk.

const char MetricsDaemon::kMetricPageFaultsLongName[] =
    "Platform.PageFaultsLong";
const char MetricsDaemon::kMetricPageFaultsShortName[] =
    "Platform.PageFaultsShort";

// persistent metrics path
const char MetricsDaemon::kMetricsPath[] = "/var/log/metrics";


// static
const char* MetricsDaemon::kDBusMatches_[] = {
  "type='signal',"
  "interface='" DBUS_IFACE_CRASH_REPORTER "',"
  "path='/',"
  "member='UserCrash'",

  "type='signal',"
  "interface='" DBUS_IFACE_POWER_MANAGER "',"
  "path='/'",

  "type='signal',"
  "sender='org.chromium.SessionManager',"
  "interface='" DBUS_IFACE_SESSION_MANAGER "',"
  "path='/org/chromium/SessionManager',"
  "member='SessionStateChanged'",
};

// static
const char* MetricsDaemon::kPowerStates_[] = {
#define STATE(name, capname) #name,
#include "power_states.h"
};

// static
const char* MetricsDaemon::kSessionStates_[] = {
#define STATE(name, capname) #name,
#include "session_states.h"
};

// Memory use stats collection intervals.  We collect some memory use interval
// at these intervals after boot, and we stop collecting after the last one,
// with the assumption that in most cases the memory use won't change much
// after that.
static const int kMemuseIntervals[] = {
  1 * kSecondsPerMinute,    // 1 minute mark
  4 * kSecondsPerMinute,    // 5 minute mark
  25 * kSecondsPerMinute,   // 0.5 hour mark
  120 * kSecondsPerMinute,  // 2.5 hour mark
  600 * kSecondsPerMinute,  // 12.5 hour mark
};

MetricsDaemon::MetricsDaemon()
    : power_state_(kUnknownPowerState),
      session_state_(kUnknownSessionState),
      user_active_(false),
      usemon_interval_(0),
      usemon_source_(NULL),
      memuse_initial_time_(0),
      memuse_interval_index_(0),
      read_sectors_(0),
      write_sectors_(0),
      page_faults_(0),
      stats_state_(kStatsShort),
      stats_initial_time_(0) {}

MetricsDaemon::~MetricsDaemon() {
  DeleteFrequencyCounters();
}

double MetricsDaemon::GetActiveTime() {
  struct timespec ts;
  int r = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (r < 0) {
    PLOG(WARNING) << "clock_gettime(CLOCK_MONOTONIC) failed";
    return 0;
  } else {
    return ts.tv_sec + ((double) ts.tv_nsec) / (1000 * 1000 * 1000);
  }
}

void MetricsDaemon::DeleteFrequencyCounters() {
  for (FrequencyCounters::iterator i = frequency_counters_.begin();
       i != frequency_counters_.end(); ++i) {
    delete i->second;
    i->second = NULL;
  }
}

void MetricsDaemon::Run(bool run_as_daemon) {
  if (run_as_daemon && daemon(0, 0) != 0)
    return;

  if (CheckSystemCrash(kKernelCrashDetectedFile)) {
    ProcessKernelCrash();
  }

  if (CheckSystemCrash(kUncleanShutdownDetectedFile)) {
    ProcessUncleanShutdown();
  }

  Loop();
}

FilePath MetricsDaemon::GetHistogramPath(const char* histogram_name) {
  return FilePath(kMetricsPath).Append(histogram_name);
}

void MetricsDaemon::ConfigureCrashIntervalReporter(
    const char* histogram_name,
    std::unique_ptr<chromeos_metrics::TaggedCounterReporter>* reporter) {
  reporter->reset(new chromeos_metrics::TaggedCounterReporter());
  FilePath file_path = GetHistogramPath(histogram_name);
  (*reporter)->Init(file_path.value().c_str(),
                    histogram_name,
                    kMetricCrashIntervalMin,
                    kMetricCrashIntervalMax,
                    kMetricCrashIntervalBuckets);
}

void MetricsDaemon::ConfigureCrashFrequencyReporter(
    const char* histogram_name) {
  std::unique_ptr<chromeos_metrics::TaggedCounterReporter> reporter(
      new chromeos_metrics::TaggedCounterReporter());
  FilePath file_path = GetHistogramPath(histogram_name);
  reporter->Init(file_path.value().c_str(),
                 histogram_name,
                 kMetricCrashFrequencyMin,
                 kMetricCrashFrequencyMax,
                 kMetricCrashFrequencyBuckets);
  std::unique_ptr<chromeos_metrics::FrequencyCounter> new_counter(
      new chromeos_metrics::FrequencyCounter());
  time_t cycle_duration = strstr(histogram_name, "Weekly") != NULL ?
      chromeos_metrics::kSecondsPerWeek :
      chromeos_metrics::kSecondsPerDay;
  new_counter->Init(
      static_cast<chromeos_metrics::TaggedCounterInterface*>(
          reporter.release()),
      cycle_duration);
  frequency_counters_[histogram_name] = new_counter.release();
}

void MetricsDaemon::Init(bool testing, MetricsLibraryInterface* metrics_lib,
                         const string& diskstats_path,
                         const string& vmstats_path) {
  testing_ = testing;
  DCHECK(metrics_lib != NULL);
  metrics_lib_ = metrics_lib;
  chromeos_metrics::TaggedCounterReporter::
      SetMetricsLibraryInterface(metrics_lib);

  static const char kDailyUseRecordFile[] = "/var/log/metrics/daily-usage";
  daily_use_.reset(new chromeos_metrics::TaggedCounter());
  daily_use_->Init(kDailyUseRecordFile, &ReportDailyUse, this);

  ConfigureCrashIntervalReporter(kMetricKernelCrashIntervalName,
                                 &kernel_crash_interval_);
  ConfigureCrashIntervalReporter(kMetricUncleanShutdownIntervalName,
                                 &unclean_shutdown_interval_);
  ConfigureCrashIntervalReporter(kMetricUserCrashIntervalName,
                                 &user_crash_interval_);

  DeleteFrequencyCounters();
  ConfigureCrashFrequencyReporter(kMetricAnyCrashesDailyName);
  ConfigureCrashFrequencyReporter(kMetricAnyCrashesWeeklyName);
  ConfigureCrashFrequencyReporter(kMetricKernelCrashesDailyName);
  ConfigureCrashFrequencyReporter(kMetricKernelCrashesWeeklyName);
  ConfigureCrashFrequencyReporter(kMetricUncleanShutdownsDailyName);
  ConfigureCrashFrequencyReporter(kMetricUncleanShutdownsWeeklyName);
  ConfigureCrashFrequencyReporter(kMetricUserCrashesDailyName);
  ConfigureCrashFrequencyReporter(kMetricUserCrashesWeeklyName);

  diskstats_path_ = diskstats_path;
  vmstats_path_ = vmstats_path;
  StatsReporterInit();

  // Start collecting meminfo stats.
  ScheduleMeminfoCallback(kMetricMeminfoInterval);
  ScheduleMemuseCallback(true, 0);

  // Don't setup D-Bus and GLib in test mode.
  if (testing)
    return;

  g_thread_init(NULL);
  g_type_init();
  dbus_g_thread_init();

  DBusError error;
  dbus_error_init(&error);

  DBusConnection* connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
  LOG_IF(FATAL, dbus_error_is_set(&error)) <<
      "No D-Bus connection: " << SAFE_MESSAGE(error);

  dbus_connection_setup_with_g_main(connection, NULL);

  // Registers D-Bus matches for the signals we would like to catch.
  for (unsigned int m = 0; m < arraysize(kDBusMatches_); m++) {
    const char* match = kDBusMatches_[m];
    DLOG(INFO) << "adding dbus match: " << match;
    dbus_bus_add_match(connection, match, &error);
    LOG_IF(FATAL, dbus_error_is_set(&error)) <<
        "unable to add a match: " << SAFE_MESSAGE(error);
  }

  // Adds the D-Bus filter routine to be called back whenever one of
  // the registered D-Bus matches is successful. The daemon is not
  // activated for D-Bus messages that don't match.
  CHECK(dbus_connection_add_filter(connection, MessageFilter, this, NULL));
}

void MetricsDaemon::Loop() {
  GMainLoop* loop = g_main_loop_new(NULL, false);
  g_main_loop_run(loop);
}

// static
DBusHandlerResult MetricsDaemon::MessageFilter(DBusConnection* connection,
                                               DBusMessage* message,
                                               void* user_data) {
  Time now = Time::Now();
  DLOG(INFO) << "message intercepted @ " << now.ToInternalValue();

  int message_type = dbus_message_get_type(message);
  if (message_type != DBUS_MESSAGE_TYPE_SIGNAL) {
    DLOG(WARNING) << "unexpected message type " << message_type;
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  // Signal messages always have interfaces.
  const char* interface = dbus_message_get_interface(message);
  CHECK(interface != NULL);

  MetricsDaemon* daemon = static_cast<MetricsDaemon*>(user_data);

  DBusMessageIter iter;
  dbus_message_iter_init(message, &iter);
  if (strcmp(interface, DBUS_IFACE_CRASH_REPORTER) == 0) {
    CHECK(strcmp(dbus_message_get_member(message),
                 "UserCrash") == 0);
    daemon->ProcessUserCrash();
  } else if (strcmp(interface, DBUS_IFACE_POWER_MANAGER) == 0) {
    const char* member = dbus_message_get_member(message);
    if (strcmp(member, "ScreenIsLocked") == 0) {
      daemon->SetUserActiveState(false, now);
    } else if (strcmp(member, "ScreenIsUnlocked") == 0) {
      daemon->SetUserActiveState(true, now);
    } else if (strcmp(member, "PowerStateChanged") == 0) {
      char* state_name;
      dbus_message_iter_get_basic(&iter, &state_name);
      daemon->PowerStateChanged(state_name, now);
    }
  } else if (strcmp(interface, DBUS_IFACE_SESSION_MANAGER) == 0) {
    CHECK(strcmp(dbus_message_get_member(message),
                 "SessionStateChanged") == 0);

    char* state_name;
    dbus_message_iter_get_basic(&iter, &state_name);
    daemon->SessionStateChanged(state_name, now);
  } else {
    DLOG(WARNING) << "unexpected interface: " << interface;
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

void MetricsDaemon::PowerStateChanged(const char* state_name, Time now) {
  DLOG(INFO) << "power state: " << state_name;
  power_state_ = LookupPowerState(state_name);

  if (power_state_ != kPowerStateOn)
    SetUserActiveState(false, now);
}

MetricsDaemon::PowerState
MetricsDaemon::LookupPowerState(const char* state_name) {
  for (int i = 0; i < kNumberPowerStates; i++) {
    if (strcmp(state_name, kPowerStates_[i]) == 0) {
      return static_cast<PowerState>(i);
    }
  }
  DLOG(WARNING) << "unknown power state: " << state_name;
  return kUnknownPowerState;
}

void MetricsDaemon::SessionStateChanged(const char* state_name, Time now) {
  DLOG(INFO) << "user session state: " << state_name;
  session_state_ = LookupSessionState(state_name);
  SetUserActiveState(session_state_ == kSessionStateStarted, now);
}

MetricsDaemon::SessionState
MetricsDaemon::LookupSessionState(const char* state_name) {
  for (int i = 0; i < kNumberSessionStates; i++) {
    if (strcmp(state_name, kSessionStates_[i]) == 0) {
      return static_cast<SessionState>(i);
    }
  }
  DLOG(WARNING) << "unknown user session state: " << state_name;
  return kUnknownSessionState;
}

void MetricsDaemon::SetUserActiveState(bool active, Time now) {
  DLOG(INFO) << "user: " << (active ? "active" : "inactive");

  // Calculates the seconds of active use since the last update and
  // the day since Epoch, and logs the usage data.  Guards against the
  // time jumping back and forth due to the user changing it by
  // discarding the new use time.
  int seconds = 0;
  if (user_active_ && now > user_active_last_) {
    TimeDelta since_active = now - user_active_last_;
    if (since_active < TimeDelta::FromSeconds(
            kUseMonitorIntervalMax + kSecondsPerMinute)) {
      seconds = static_cast<int>(since_active.InSeconds());
    }
  }
  TimeDelta since_epoch = now - Time();
  int day = since_epoch.InDays();
  daily_use_->Update(day, seconds);
  user_crash_interval_->Update(0, seconds);
  kernel_crash_interval_->Update(0, seconds);

  // Flush finished cycles of all frequency counters.
  for (FrequencyCounters::iterator i = frequency_counters_.begin();
       i != frequency_counters_.end(); ++i) {
    i->second->FlushFinishedCycles();
  }

  // Schedules a use monitor on inactive->active transitions and
  // unschedules it on active->inactive transitions.
  if (!user_active_ && active)
    ScheduleUseMonitor(kUseMonitorIntervalInit, /* backoff */ false);
  else if (user_active_ && !active)
    UnscheduleUseMonitor();

  // Remembers the current active state and the time of the last
  // activity update.
  user_active_ = active;
  user_active_last_ = now;
}

void MetricsDaemon::ProcessUserCrash() {
  // Counts the active use time up to now.
  SetUserActiveState(user_active_, Time::Now());

  // Reports the active use time since the last crash and resets it.
  user_crash_interval_->Flush();

  frequency_counters_[kMetricUserCrashesDailyName]->Update(1);
  frequency_counters_[kMetricUserCrashesWeeklyName]->Update(1);
  frequency_counters_[kMetricAnyCrashesDailyName]->Update(1);
  frequency_counters_[kMetricAnyCrashesWeeklyName]->Update(1);
}

void MetricsDaemon::ProcessKernelCrash() {
  // Counts the active use time up to now.
  SetUserActiveState(user_active_, Time::Now());

  // Reports the active use time since the last crash and resets it.
  kernel_crash_interval_->Flush();

  frequency_counters_[kMetricKernelCrashesDailyName]->Update(1);
  frequency_counters_[kMetricKernelCrashesWeeklyName]->Update(1);
  frequency_counters_[kMetricAnyCrashesDailyName]->Update(1);
  frequency_counters_[kMetricAnyCrashesWeeklyName]->Update(1);
}

void MetricsDaemon::ProcessUncleanShutdown() {
  // Counts the active use time up to now.
  SetUserActiveState(user_active_, Time::Now());

  // Reports the active use time since the last crash and resets it.
  unclean_shutdown_interval_->Flush();

  frequency_counters_[kMetricUncleanShutdownsDailyName]->Update(1);
  frequency_counters_[kMetricUncleanShutdownsWeeklyName]->Update(1);
  frequency_counters_[kMetricAnyCrashesDailyName]->Update(1);
  frequency_counters_[kMetricAnyCrashesWeeklyName]->Update(1);
}

bool MetricsDaemon::CheckSystemCrash(const string& crash_file) {
  FilePath crash_detected(crash_file);
  if (!file_util::PathExists(crash_detected))
    return false;

  // Deletes the crash-detected file so that the daemon doesn't report
  // another kernel crash in case it's restarted.
  file_util::Delete(crash_detected,
                    false);  // recursive
  return true;
}

// static
gboolean MetricsDaemon::UseMonitorStatic(gpointer data) {
  return static_cast<MetricsDaemon*>(data)->UseMonitor() ? TRUE : FALSE;
}

bool MetricsDaemon::UseMonitor() {
  SetUserActiveState(user_active_, Time::Now());

  // If a new monitor source/instance is scheduled, returns false to
  // tell GLib to destroy this monitor source/instance. Returns true
  // otherwise to keep calling back this monitor.
  return !ScheduleUseMonitor(usemon_interval_ * 2, /* backoff */ true);
}

bool MetricsDaemon::ScheduleUseMonitor(int interval, bool backoff)
{
  if (testing_)
    return false;

  // Caps the interval -- the bigger the interval, the more active use
  // time will be potentially dropped on system shutdown.
  if (interval > kUseMonitorIntervalMax)
    interval = kUseMonitorIntervalMax;

  if (backoff) {
    // Back-off mode is used by the use monitor to reschedule itself
    // with exponential back-off in time. This mode doesn't create a
    // new timeout source if the new interval is the same as the old
    // one. Also, if a new timeout source is created, the old one is
    // not destroyed explicitly here -- it will be destroyed by GLib
    // when the monitor returns FALSE (see UseMonitor and
    // UseMonitorStatic).
    if (interval == usemon_interval_)
      return false;
  } else {
    UnscheduleUseMonitor();
  }

  // Schedules a new use monitor for |interval| seconds from now.
  DLOG(INFO) << "scheduling use monitor in " << interval << " seconds";
  usemon_source_ = g_timeout_source_new_seconds(interval);
  g_source_set_callback(usemon_source_, UseMonitorStatic, this,
                        NULL); // No destroy notification.
  g_source_attach(usemon_source_,
                  NULL); // Default context.
  usemon_interval_ = interval;
  return true;
}

void MetricsDaemon::UnscheduleUseMonitor() {
  // If there's a use monitor scheduled already, destroys it.
  if (usemon_source_ == NULL)
    return;

  DLOG(INFO) << "destroying use monitor";
  g_source_destroy(usemon_source_);
  usemon_source_ = NULL;
  usemon_interval_ = 0;
}

void MetricsDaemon::StatsReporterInit() {
  DiskStatsReadStats(&read_sectors_, &write_sectors_);
  VmStatsReadStats(&page_faults_);
  // The first time around just run the long stat, so we don't delay boot.
  stats_state_ = kStatsLong;
  stats_initial_time_ = GetActiveTime();
  if (stats_initial_time_ < 0) {
    LOG(WARNING) << "not collecting disk stats";
  } else {
    ScheduleStatsCallback(kMetricStatsLongInterval);
  }
}

void MetricsDaemon::ScheduleStatsCallback(int wait) {
  if (testing_) {
    return;
  }
  g_timeout_add_seconds(wait, StatsCallbackStatic, this);
}

bool MetricsDaemon::DiskStatsReadStats(long int* read_sectors,
                                       long int* write_sectors) {
  int nchars;
  int nitems;
  bool success = false;
  char line[200];
  if (diskstats_path_.empty()) {
    return false;
  }
  int file = HANDLE_EINTR(open(diskstats_path_.c_str(), O_RDONLY));
  if (file < 0) {
    PLOG(WARNING) << "cannot open " << diskstats_path_;
    return false;
  }
  nchars = HANDLE_EINTR(read(file, line, sizeof(line)));
  if (nchars < 0) {
    PLOG(WARNING) << "cannot read from " << diskstats_path_;
    return false;
  } else {
    LOG_IF(WARNING, nchars == sizeof(line))
        << "line too long in " << diskstats_path_;
    line[nchars] = '\0';
    nitems = sscanf(line, "%*d %*d %ld %*d %*d %*d %ld",
                    read_sectors, write_sectors);
    if (nitems == 2) {
      success = true;
    } else {
      LOG(WARNING) << "found " << nitems << " items in "
                   << diskstats_path_ << ", expected 2";
    }
  }
  HANDLE_EINTR(close(file));
  return success;
}

bool MetricsDaemon::VmStatsParseStats(char* stats, long int* page_faults) {
  static const char kPageFaultSearchString[] = "\npgmajfault ";
  bool success = false;
  /* Each line in the file has the form
   * <ID> <VALUE>
   * for instance:
   * nr_free_pages 213427
   */
  char* s = strstr(stats, kPageFaultSearchString);
  if (s == NULL) {
    LOG(WARNING) << "cannot find page fault entry in vmstats";
  } else {
    char* endp;
    /* Skip <ID> and space.  Don't count the terminating null. */
    s += sizeof(kPageFaultSearchString) - 1;
    *page_faults = strtol(s, &endp, 10);
    if (*endp == '\n') {
      success = true;
    } else {
      LOG(WARNING) << "error parsing vmstats";
    }
  }
  return success;
}

bool MetricsDaemon::VmStatsReadStats(long int* page_faults) {
  char buffer[4000];
  int nchars;
  int success = false;
  if (testing_) {
    return false;
  }
  int file = HANDLE_EINTR(open(vmstats_path_.c_str(), O_RDONLY));
  if (file < 0) {
    PLOG(WARNING) << "cannot open " << vmstats_path_;
    return false;
  }
  nchars = HANDLE_EINTR(read(file, buffer, sizeof(buffer) - 1));
  LOG_IF(WARNING, nchars == sizeof(buffer) - 1)
      << "file too large in " << vmstats_path_;
  if (nchars < 0) {
    PLOG(WARNING) << "cannot read from " << vmstats_path_;
  } else if (nchars == 0) {
    LOG(WARNING) << vmstats_path_ << " is empty";
  } else {
    buffer[nchars] = '\0';
    success = VmStatsParseStats(buffer, page_faults);
  }
  HANDLE_EINTR(close(file));
  return success;
}

// static
gboolean MetricsDaemon::StatsCallbackStatic(void* handle) {
  (static_cast<MetricsDaemon*>(handle))->StatsCallback();
  return false;  // one-time callback
}

// Collects disk and vm stats alternating over a short and a long interval.

void MetricsDaemon::StatsCallback() {
  long int read_sectors_now, write_sectors_now, page_faults_now;
  double time_now = GetActiveTime();
  double delta_time = time_now - stats_initial_time_;
  if (testing_) {
    // Fake the time when testing.
    delta_time = stats_state_ == kStatsShort ?
        kMetricStatsShortInterval : kMetricStatsLongInterval;
  }
  bool diskstats_success = DiskStatsReadStats(&read_sectors_now,
                                              &write_sectors_now);
  int delta_read = read_sectors_now - read_sectors_;
  int delta_write = write_sectors_now - write_sectors_;
  int read_sectors_per_second = delta_read / delta_time;
  int write_sectors_per_second = delta_write / delta_time;
  bool vmstats_success = VmStatsReadStats(&page_faults_now);
  int delta_faults = page_faults_now - page_faults_;
  int page_faults_per_second = delta_faults / delta_time;

  switch (stats_state_) {
    case kStatsShort:
      if (diskstats_success) {
        SendMetric(kMetricReadSectorsShortName,
                   read_sectors_per_second,
                   1,
                   kMetricSectorsIOMax,
                   kMetricSectorsBuckets);
        SendMetric(kMetricWriteSectorsShortName,
                   write_sectors_per_second,
                   1,
                   kMetricSectorsIOMax,
                   kMetricSectorsBuckets);
      }
      if (vmstats_success) {
        SendMetric(kMetricPageFaultsShortName,
                   page_faults_per_second,
                   1,
                   kMetricPageFaultsMax,
                   kMetricPageFaultsBuckets);
      }
      // Schedule long callback.
      stats_state_ = kStatsLong;
      ScheduleStatsCallback(kMetricStatsLongInterval -
                            kMetricStatsShortInterval);
      break;
    case kStatsLong:
      if (diskstats_success) {
        SendMetric(kMetricReadSectorsLongName,
                   read_sectors_per_second,
                   1,
                   kMetricSectorsIOMax,
                   kMetricSectorsBuckets);
        SendMetric(kMetricWriteSectorsLongName,
                   write_sectors_per_second,
                   1,
                   kMetricSectorsIOMax,
                   kMetricSectorsBuckets);
        // Reset sector counters.
        read_sectors_ = read_sectors_now;
        write_sectors_ = write_sectors_now;
      }
      if (vmstats_success) {
        SendMetric(kMetricPageFaultsLongName,
                   page_faults_per_second,
                   1,
                   kMetricPageFaultsMax,
                   kMetricPageFaultsBuckets);
        page_faults_ = page_faults_now;
      }
      // Set start time for new cycle.
      stats_initial_time_ = time_now;
      // Schedule short callback.
      stats_state_ = kStatsShort;
      ScheduleStatsCallback(kMetricStatsShortInterval);
      break;
    default:
      LOG(FATAL) << "Invalid stats state";
  }
}

void MetricsDaemon::ScheduleMeminfoCallback(int wait) {
  if (testing_) {
    return;
  }
  g_timeout_add_seconds(wait, MeminfoCallbackStatic, this);
}

// static
gboolean MetricsDaemon::MeminfoCallbackStatic(void* handle) {
  return (static_cast<MetricsDaemon*>(handle))->MeminfoCallback();
}

bool MetricsDaemon::MeminfoCallback() {
  string meminfo_raw;
  const FilePath meminfo_path("/proc/meminfo");
  if (!file_util::ReadFileToString(meminfo_path, &meminfo_raw)) {
    LOG(WARNING) << "cannot read " << meminfo_path.value().c_str();
    return false;
  }
  return ProcessMeminfo(meminfo_raw);
}

bool MetricsDaemon::ProcessMeminfo(const string& meminfo_raw) {
  static const MeminfoRecord fields_array[] = {
    { "MemTotal", "MemTotal" },  // SPECIAL CASE: total system memory
    { "MemFree", "MemFree" },
    { "Buffers", "Buffers" },
    { "Cached", "Cached" },
    // { "SwapCached", "SwapCached" },
    { "Active", "Active" },
    { "Inactive", "Inactive" },
    { "ActiveAnon", "Active(anon)" },
    { "InactiveAnon", "Inactive(anon)" },
    { "ActiveFile" , "Active(file)" },
    { "InactiveFile", "Inactive(file)" },
    { "Unevictable", "Unevictable", 1 },
    // { "Mlocked", "Mlocked" },
    // { "SwapTotal", "SwapTotal" },
    // { "SwapFree", "SwapFree" },
    // { "Dirty", "Dirty" },
    // { "Writeback", "Writeback" },
    { "AnonPages", "AnonPages" },
    { "Mapped", "Mapped" },
    { "Shmem", "Shmem", 1 },
    { "Slab", "Slab", 1 },
    // { "SReclaimable", "SReclaimable" },
    // { "SUnreclaim", "SUnreclaim" },
  };
  vector<MeminfoRecord> fields(fields_array,
                               fields_array + arraysize(fields_array));
  if (!FillMeminfo(meminfo_raw, &fields)) {
    return false;
  }
  int total_memory = fields[0].value;
  if (total_memory == 0) {
    // this "cannot happen"
    LOG(WARNING) << "borked meminfo parser";
    return false;
  }
  // Send all fields retrieved, except total memory.
  for (unsigned int i = 1; i < fields.size(); i++) {
    string metrics_name = StringPrintf("Platform.Meminfo%s", fields[i].name);
    if (fields[i].log_scale) {
      // report value in kbytes, log scale, 4Gb max
      SendMetric(metrics_name, fields[i].value, 1, 4 * 1000 * 1000, 100);
    } else {
      // report value as percent of total memory
      int percent = fields[i].value * 100 / total_memory;
      SendLinearMetric(metrics_name, percent, 100, 101);
    }
  }
  return true;
}

bool MetricsDaemon::FillMeminfo(const string& meminfo_raw,
                                vector<MeminfoRecord>* fields) {
  vector<string> lines;
  unsigned int nlines = Tokenize(meminfo_raw, "\n", &lines);

  // Scan meminfo output and collect field values.  Each field name has to
  // match a meminfo entry (case insensitive) after removing non-alpha
  // characters from the entry.
  unsigned int ifield = 0;
  for (unsigned int iline = 0;
       iline < nlines && ifield < fields->size();
       iline++) {
    vector<string> tokens;
    Tokenize(lines[iline], ": ", &tokens);
    if (strcmp((*fields)[ifield].match, tokens[0].c_str()) == 0) {
      // Name matches. Parse value and save.
      char* rest;
      (*fields)[ifield].value =
          static_cast<int>(strtol(tokens[1].c_str(), &rest, 10));
      if (*rest != '\0') {
        LOG(WARNING) << "missing meminfo value";
        return false;
      }
      ifield++;
    }
  }
  if (ifield < fields->size()) {
    // End of input reached while scanning.
    LOG(WARNING) << "cannot find field " << (*fields)[ifield].match
                 << " and following";
    return false;
  }
  return true;
}

void MetricsDaemon::ScheduleMemuseCallback(bool new_callback,
                                           double time_elapsed) {
  if (testing_) {
    return;
  }
  int interval = kMemuseIntervals[memuse_interval_index_];
  int wait;
  if (new_callback) {
    memuse_initial_time_ = GetActiveTime();
    wait = interval;
  } else {
    wait = ceil(interval - time_elapsed);  // round up
  }
  g_timeout_add_seconds(wait, MemuseCallbackStatic, this);
}

// static
gboolean MetricsDaemon::MemuseCallbackStatic(void* handle) {
  MetricsDaemon* daemon = static_cast<MetricsDaemon*>(handle);
  daemon->MemuseCallback();
  return false;
}

void MetricsDaemon::MemuseCallback() {
  // Since we only care about active time (i.e. uptime minus sleep time) but
  // the callbacks are driven by real time (uptime), we check if we should
  // reschedule this callback due to intervening sleep periods.
  double now = GetActiveTime();
  double active_time = now - memuse_initial_time_;
  if (active_time < kMemuseIntervals[memuse_interval_index_]) {
    // Not enough active time has passed.  Reschedule the callback.
    ScheduleMemuseCallback(false, active_time);
  } else {
    // Enough active time has passed.  Do the work, and (if we succeed) see if
    // we need to do more.
    if (MemuseCallbackWork() &&
        memuse_interval_index_ < arraysize(kMemuseIntervals)) {
      memuse_interval_index_++;
      ScheduleMemuseCallback(true, 0);
    }
  }
}

bool MetricsDaemon::MemuseCallbackWork() {
  string meminfo_raw;
  const FilePath meminfo_path("/proc/meminfo");
  if (!file_util::ReadFileToString(meminfo_path, &meminfo_raw)) {
    LOG(WARNING) << "cannot read " << meminfo_path.value().c_str();
    return false;
  }
  return ProcessMemuse(meminfo_raw);
}

bool MetricsDaemon::ProcessMemuse(const string& meminfo_raw) {
  static const MeminfoRecord fields_array[] = {
    { "MemTotal", "MemTotal" },  // SPECIAL CASE: total system memory
    { "ActiveAnon", "Active(anon)" },
    { "InactiveAnon", "Inactive(anon)" },
  };
  vector<MeminfoRecord> fields(fields_array,
                               fields_array + arraysize(fields_array));
  if (!FillMeminfo(meminfo_raw, &fields)) {
    return false;
  }
  int total = fields[0].value;
  int active_anon = fields[1].value;
  int inactive_anon = fields[2].value;
  if (total == 0) {
    // this "cannot happen"
    LOG(WARNING) << "borked meminfo parser";
    return false;
  }
  string metrics_name = StringPrintf("Platform.MemuseAnon%d",
                                     memuse_interval_index_);
  SendLinearMetric(metrics_name, (active_anon + inactive_anon) * 100 / total,
                   100, 101);
  return true;
}

// static
void MetricsDaemon::ReportDailyUse(void* handle, int tag, int count) {
  if (count <= 0)
    return;

  MetricsDaemon* daemon = static_cast<MetricsDaemon*>(handle);
  int minutes = (count + kSecondsPerMinute / 2) / kSecondsPerMinute;
  daemon->SendMetric(kMetricDailyUseTimeName, minutes,
                     kMetricDailyUseTimeMin,
                     kMetricDailyUseTimeMax,
                     kMetricDailyUseTimeBuckets);
}

void MetricsDaemon::SendMetric(const string& name, int sample,
                               int min, int max, int nbuckets) {
  DLOG(INFO) << "received metric: " << name << " " << sample << " "
             << min << " " << max << " " << nbuckets;
  metrics_lib_->SendToUMA(name, sample, min, max, nbuckets);
}

void MetricsDaemon::SendLinearMetric(const string& name, int sample,
                                     int max, int nbuckets) {
  DLOG(INFO) << "received linear metric: " << name << " " << sample << " "
             << max << " " << nbuckets;
  // TODO(semenzato): add a proper linear histogram to the Chrome external
  // metrics API.
  LOG_IF(FATAL, nbuckets != max + 1) << "unsupported histogram scale";
  metrics_lib_->SendEnumToUMA(name, sample, max);
}
