#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>

#include "sntpd.c"

static uint64_t g_total_sleep_ms = 0;
static struct timeval g_updated_tv;
static struct timeval g_tv_to_return;

//override nanosleep
int nanosleep_mock(const struct timespec *tv_up, struct timespec *rem) {
  (void)rem;
  g_total_sleep_ms += ts_to_ms(tv_up);
  return 0;
}

//override settimeofday
int settimeofday (const struct timeval* tv,
                  const struct timezone* tz) {
  assert(tv);
  g_updated_tv = *tv;
  return 0;
}

int try_sync_time_mock_error(bool synced, sntp_gettime_fn sntp_gettime_fn_ptr) {
  return -1;
}

int try_sync_time_mock_success(bool synced, sntp_gettime_fn sntp_gettime_fn_ptr) {
  return 0;
}

int sntp_gettime_mock(struct timeval *tv_upd) {
  assert(tv_upd);
  *tv_upd = g_tv_to_return;
  return 0;
}

void test_sync_loop_on_error() {
  g_total_sleep_ms = 0;
  bool synced = false;
  unsigned long long retry_delay = INITIAL_RETRY_DELAY;
  for(int i = 0; i < 20; i++) {
    do_sync(&synced, &retry_delay, try_sync_time_mock_error, nanosleep_mock, NULL);
  }
  fprintf(stdout, "Total sleep time is %" PRId64 " ms.\n", g_total_sleep_ms);
  assert(g_total_sleep_ms >= 10239500 && g_total_sleep_ms < 20479000);
}

void test_sync_loop_on_success() {
  g_total_sleep_ms = 0;
  bool synced = false;
  unsigned long long retry_delay = INITIAL_RETRY_DELAY;
  for(int i = 0; i < 20; i++) {
    do_sync(&synced, &retry_delay, try_sync_time_mock_success, nanosleep_mock, NULL);
  }
  fprintf(stdout, "Total sleep time is %" PRId64 " ms.\n", g_total_sleep_ms);
  assert(g_total_sleep_ms >= 12000000 && g_total_sleep_ms < 24000000);
}

void test_try_sync_time() {
  //Set the time when synced is false.
  unsigned long long time_ms;
  g_updated_tv.tv_sec = 0;
  g_updated_tv.tv_usec = 0;
  g_tv_to_return.tv_usec = 0;
  g_tv_to_return.tv_sec = 0;
  int rv = try_sync_time(false, sntp_gettime_mock);
  assert(rv == 0);
  assert(g_updated_tv.tv_sec == 0);
  assert(g_updated_tv.tv_usec == 0);

  //Synced is true, and diff > MAX_TIME_DRIFT
  //settimeofday should not be called.
  g_updated_tv.tv_sec = 1234;
  g_updated_tv.tv_usec = 123456;
  g_tv_to_return.tv_sec = 0;
  g_tv_to_return.tv_usec = 0;
  rv = try_sync_time(true, sntp_gettime_mock);
  assert(rv == 0);
  assert(g_updated_tv.tv_sec == 1234);
  assert(g_updated_tv.tv_usec == 123456);

  //Synced is true, local clock lagging, diff < UPDATE_INTERVAL
  //settimeofday should not be called.
  g_updated_tv.tv_sec = 1234;
  g_updated_tv.tv_usec = 123456;
  assert(!gettimeofday(&g_tv_to_return, NULL));
  time_ms = tv_to_ms(&g_tv_to_return);
  time_ms += 456;
  ms_to_tv(&g_tv_to_return, time_ms);
  rv = try_sync_time(true, sntp_gettime_mock);
  assert(rv == 0);
  assert(g_updated_tv.tv_sec == 1234);
  assert(g_updated_tv.tv_usec == 123456);

  //Synced is true, local clock ahead, diff < UPDATE_INTERVAL
  //settimeofday should not be called.
  g_updated_tv.tv_sec = 1234;
  g_updated_tv.tv_usec = 123456;
  assert(!gettimeofday(&g_tv_to_return, NULL));
  time_ms = tv_to_ms(&g_tv_to_return);
  time_ms -= 456;
  ms_to_tv(&g_tv_to_return, time_ms);
  rv = try_sync_time(true, sntp_gettime_mock);
  assert(rv == 0);
  assert(g_updated_tv.tv_sec == 1234);
  assert(g_updated_tv.tv_usec == 123456);

  //Synced is true, local clock lagging, UPDATE_INTERVAL < diff < MAX_TIME_DRIFT
  g_updated_tv.tv_sec = 1234;
  g_updated_tv.tv_usec = 123456;
  assert(!gettimeofday(&g_tv_to_return, NULL));
  time_ms = tv_to_ms(&g_tv_to_return);
  time_ms += 501;
  ms_to_tv(&g_tv_to_return, time_ms);
  rv = try_sync_time(true, sntp_gettime_mock);
  assert(rv == 0);
  assert(g_updated_tv.tv_sec == g_tv_to_return.tv_sec);
  assert(g_updated_tv.tv_usec == g_tv_to_return.tv_usec);

  //Synced is true, local clock ahead, UPDATE_INTERVAL < diff < MAX_TIME_DRIFT
  g_updated_tv.tv_sec = 1234;
  g_updated_tv.tv_usec = 123456;
  assert(!gettimeofday(&g_tv_to_return, NULL));
  time_ms = tv_to_ms(&g_tv_to_return);
  time_ms -= 501;
  ms_to_tv(&g_tv_to_return, time_ms);
  rv = try_sync_time(true, sntp_gettime_mock);
  assert(rv == 0);
  assert(g_updated_tv.tv_sec == g_tv_to_return.tv_sec);
  assert(g_updated_tv.tv_usec == g_tv_to_return.tv_usec);

  //Synced is true, local clock ahead, UPDATE_INTERVAL < diff < MAX_TIME_DRIFT
  g_updated_tv.tv_sec = 1234;
  g_updated_tv.tv_usec = 123456;
  assert(!gettimeofday(&g_tv_to_return, NULL));
  time_ms = tv_to_ms(&g_tv_to_return);
  time_ms -= 17975;
  ms_to_tv(&g_tv_to_return, time_ms);
  rv = try_sync_time(true, sntp_gettime_mock);
  assert(rv == 0);
  assert(g_updated_tv.tv_sec == g_tv_to_return.tv_sec);
  assert(g_updated_tv.tv_usec == g_tv_to_return.tv_usec);

  //Synced is true, local clock lagging, UPDATE_INTERVAL < diff < MAX_TIME_DRIFT
  g_updated_tv.tv_sec = 1234;
  g_updated_tv.tv_usec = 123456;
  assert(!gettimeofday(&g_tv_to_return, NULL));
  time_ms = tv_to_ms(&g_tv_to_return);
  time_ms += 11221;
  ms_to_tv(&g_tv_to_return, time_ms);
  rv = try_sync_time(true, sntp_gettime_mock);
  assert(rv == 0);
  assert(g_updated_tv.tv_sec == g_tv_to_return.tv_sec);
  assert(g_updated_tv.tv_usec == g_tv_to_return.tv_usec);
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  test_sync_loop_on_success();
  test_sync_loop_on_error();
  test_try_sync_time();
  return 0;
}
