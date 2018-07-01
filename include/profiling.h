#ifndef INCLUDE_PROFILING_H_
#define INCLUDE_PROFILING_H_

class Profiler {
public:
  Profiler();
  void startTimer();
  void stopTimer();
  double getElapsedTime();

private:
  __int128_t begin;
  __int128_t end;
  __int128_t elapsed = 0;
  __int128_t getTimeMs();
};

#endif // INCLUDE_PROFILING_H_
