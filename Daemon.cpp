
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>

#include "Daemon.hpp"



using namespace netio;



void Daemon::init(OnTerminate termcb) {
  setupRLimit();
  setupSignal(termcb);
}

void Daemon::setupSignal(OnTerminate termcb) {
  struct sigaction sa;
  sigset_t sset;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = termcb;
  // sa.sa_sigaction = termcb;

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

void Daemon::setupRLimit() {
  struct rlimit rlim;

  // file fds raise to system limit
  rlim.rlim_cur = 65535;
  rlim.rlim_max = 65535;
  setrlimit(RLIMIT_NOFILE, &rlim);

  // enable core dump
  rlim.rlim_cur = RLIM_INFINITY;
  rlim.rlim_max = RLIM_INFINITY;
  setrlimit(RLIMIT_CORE, &rlim);
}

void Daemon::startWork() {
  _msgLooper.startLoop();
}

void Daemon::stopWork() {
  _msgLooper.stopLoop();
}















