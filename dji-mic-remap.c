/*
 * dji-mic-remap: when the DJI Wireless Mic Rx (USB 2CA3:4011) is attached,
 * remap its Consumer Control "Volume Increment" (0x0C/0xE9) to Right Control (0x07/0xE4)
 * on that device only. The Link button on the DJI Mic emits that event.
 *
 * The daemon applies the mapping at start and every time launchd wakes it
 * with an IOKit matching event (receiver plugged in), then keeps running so
 * the launchd event stream stays consumed.
 *
 * Build: cc -std=c11 -fblocks -Wall -Wextra -Werror dji-mic-remap.c -o dji-mic-remap
 */
#include <dispatch/dispatch.h>
#include <errno.h>
#include <spawn.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <xpc/xpc.h>

extern char **environ;

static void log_line(const char *msg) {
  char ts[32];
  time_t now = time(NULL);
  strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", localtime(&now));
  fprintf(stderr, "%s %s\n", ts, msg);
  fflush(stderr);
}

static void apply_mapping(void) {
  char *const argv[] = {
      "/usr/bin/hidutil", "property",
      "--matching", "{\"VendorID\":11427,\"ProductID\":16401}",
      "--set",
      "{\"UserKeyMapping\":[{\"HIDKeyboardModifierMappingSrc\":51539607785,"
      "\"HIDKeyboardModifierMappingDst\":30064771300}]}",
      NULL};
  pid_t pid;
  int err = posix_spawn(&pid, argv[0], NULL, NULL, argv, environ);
  if (err != 0) {
    errno = err;
    perror("posix_spawn hidutil");
    return;
  }
  int status;
  while (waitpid(pid, &status, 0) == -1 && errno == EINTR) {}
  if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    log_line("applied: DJI Rx 0x0C/0xE9 -> Right Ctrl");
  else
    fprintf(stderr, "hidutil failed: status=%d\n", status);
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--once") == 0) {
    apply_mapping();
    return 0;
  }
  xpc_set_event_stream_handler(
      "com.apple.iokit.matching",
      dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
      ^(xpc_object_t event) {
        (void)event;
        log_line("receiver attached");
        /* HID services can publish slightly before the property is settable. */
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 2),
                       dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                       ^{ apply_mapping(); });
      });
  log_line("daemon start");
  apply_mapping(); /* receiver may already be attached */
  dispatch_main();
}
