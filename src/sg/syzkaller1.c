// autogenerated by syzkaller (http://github.com/google/syzkaller)
// modified to take sg device as argument

#ifndef __NR_read
#define __NR_read 0
#endif
#ifndef __NR_mmap
#define __NR_mmap 9
#endif
#ifndef __NR_syz_open_dev
#define __NR_syz_open_dev 1000002
#endif
#ifndef __NR_write
#define __NR_write 1
#endif

#define _GNU_SOURCE

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <linux/capability.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/kvm.h>
#include <linux/sched.h>
#include <linux/tcp.h>
#include <net/if_arp.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kErrorStatus = 68;
const int kRetryStatus = 69;

const char *dev_sg;

__attribute__((noreturn)) void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

__attribute__((noreturn)) void exitf(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

static int flag_debug;

void debug(const char* msg, ...)
{
  if (!flag_debug)
    return;
  va_list args;
  va_start(args, msg);
  vfprintf(stdout, msg, args);
  va_end(args);
  fflush(stdout);
}

__thread int skip_segv;
__thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    debug("SIGSEGV on %p, skipping\n", addr);
    _longjmp(segv_env, 1);
  }
  debug("SIGSEGV on %p, exiting\n", addr);
  doexit(sig);
  for (;;) {
  }
}

static void install_segv_handler()
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                \
  {                                                                    \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
    if (_setjmp(segv_env) == 0) {                                      \
      __VA_ARGS__;                                                     \
    }                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
  }

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                          \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)              \
  if ((bf_off) == 0 && (bf_len) == 0) {                                \
    *(type*)(addr) = (type)(val);                                      \
  } else {                                                             \
    type new_val = *(type*)(addr);                                     \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));             \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);  \
    *(type*)(addr) = new_val;                                          \
  }

struct csum_inet {
  uint32_t acc;
};

void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                      size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf)));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static uintptr_t execute_syscall(int nr, uintptr_t a0, uintptr_t a1,
                                 uintptr_t a2, uintptr_t a3,
                                 uintptr_t a4, uintptr_t a5,
                                 uintptr_t a6, uintptr_t a7,
                                 uintptr_t a8)
{
  switch (nr) {
  default:
    return syscall(nr, a0, a1, a2, a3, a4, a5);
  case __NR_syz_open_dev:
    return syz_open_dev(a0, a1, a2);
  }
}

static void setup_main_process()
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  install_segv_handler();

  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_IO);
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  int pid = fork();
  if (pid)
    return pid;

  sandbox_common();

  loop();
  doexit(1);
}

static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exitf("opendir(%s) failed due to NOFILE, exiting");
    }
    exitf("opendir(%s) failed", dir);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    struct stat st;
    if (lstat(filename, &st))
      exitf("lstat(%s) failed", filename);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      debug("unlink(%s)\n", filename);
      if (unlink(filename) == 0)
        break;
      if (errno == EROFS) {
        debug("ignoring EROFS\n");
        break;
      }
      if (errno != EBUSY || i > 100)
        exitf("unlink(%s) failed", filename);
      debug("umount(%s)\n", filename);
      if (umount2(filename, MNT_DETACH))
        exitf("umount(%s) failed", filename);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    debug("rmdir(%s)\n", dir);
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EROFS) {
        debug("ignoring EROFS\n");
        break;
      }
      if (errno == EBUSY) {
        debug("umount(%s)\n", dir);
        if (umount2(dir, MNT_DETACH))
          exitf("umount(%s) failed", dir);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exitf("rmdir(%s) failed", dir);
  }
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[256];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      fail("failed to mkdir");
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      if (chdir(cwdbuf))
        fail("failed to chdir");
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
    remove_dir(cwdbuf);
  }
}

long r[15];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = execute_syscall(__NR_mmap, 0x20000000ul, 0x5000ul, 0x3ul,
                         0x32ul, 0xfffffffffffffffful, 0x0ul, 0, 0, 0);
  NONFAILING(memcpy((void*)0x20000000,
                    dev_sg, strlen(dev_sg)));
  r[2] = execute_syscall(__NR_syz_open_dev, 0x20000000ul, 0x0ul, 0x2ul,
                         0, 0, 0, 0, 0, 0);
  NONFAILING(*(uint64_t*)0x20001fdc = (uint64_t)0x0);
  NONFAILING(*(uint64_t*)0x20001fe4 = (uint64_t)0x0);
  NONFAILING(*(uint16_t*)0x20001fec = (uint16_t)0x3);
  NONFAILING(*(uint16_t*)0x20001fee = (uint16_t)0x4);
  NONFAILING(*(uint32_t*)0x20001ff0 = (uint32_t)0x9);
  NONFAILING(*(uint64_t*)0x20001ff4 = (uint64_t)0x0);
  NONFAILING(*(uint64_t*)0x20001ffc = (uint64_t)0x2710);
  NONFAILING(*(uint16_t*)0x20002004 = (uint16_t)0x7);
  NONFAILING(*(uint16_t*)0x20002006 = (uint16_t)0x7);
  NONFAILING(*(uint32_t*)0x20002008 = (uint32_t)0x8);
  r[13] = execute_syscall(__NR_write, r[2], 0x20001fdcul, 0x30ul, 0, 0,
                          0, 0, 0, 0);
  r[14] = execute_syscall(__NR_read, r[2], 0x20002f36ul, 0x0ul, 0, 0, 0,
                          0, 0, 0);
}
int main(int argc, char **argv)
{
  if (argc == 2)
    dev_sg = strdup(argv[1]);
  else
    dev_sg = "/dev/sg0";

  setup_main_process();
  int pid = do_sandbox_none(0, false);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
