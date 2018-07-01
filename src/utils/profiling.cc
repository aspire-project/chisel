#include <sys/time.h>

#include <ctime>

#include "profiling.h"

Profiler::Profiler() {
  begin = 0;
  end = 0;
  elapsed = 0;
}

void Profiler::startTimer() { begin = getTimeMs(); }

void Profiler::stopTimer() {
  end = getTimeMs();
  elapsed += end - begin;
}

double Profiler::getElapsedTime() { // seconds
  return elapsed / 1000.0;
}

__int128_t Profiler::getTimeMs() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  __int128_t ret = tv.tv_usec;
  ret /= 1000;
  ret += (tv.tv_sec * 1000);
  return ret;
}
