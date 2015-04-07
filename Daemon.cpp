#include <Daemon.hpp>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>




using namespace netio;



Daemon::init(OnTerminate termcb) {
  setupRLimit();
  setupSignal(OnTerminate termcb);
}

Daemon::setupSignal(OnTerminate termcb) {
  struct sigaction sa;
  sigset_t sset;

  memset(&sa, 0, sizeof(sa));
  //  sa.sa_handler = sigterm_handler;
  sa.sa_sigaction = termcb;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);

  sigemptyset(&sset);
  sigaddset(&sset, SIGSEGV);
  sigaddset(&sset, SIGBUS);
  sigaddset(&sset, SIGABRT);
  sigaddset(&sset, SIGILL);
  sigaddset(&sset, SIGCHLD);
  sigaddset(&sset, SIGFPE);
  sigprocmask(SIG_UNBLOCK, &sset, &sset);
}

Daemon::setupRLimit() {
  struct rlimit rlim;

  // file fds raise to system limit
  rlim.rlim_cur = MAXFDS;
  rlim.rlim_max = MAXFDS;
  setrlimit(RLIMIT_NOFILE, &rlim);

  // enable core dump
  rlim.rlim_cur = RLIM_INFINITY;
  rlim.rlim_max = RLIM_INFINITY;
  setrlimit(RLIMIT_CORE, &rlim);
}

Daemon::startWork() {
  _msgLooper.startLoop();
}

Daemon::stopWork() {
  _msgLooper.stopLoop();
}















