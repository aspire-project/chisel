#ifndef INCLUDE_COUNTING_H_
#define INCLUDE_COUNTING_H_

class Counter {
public:
  Counter();
  void increment();
  unsigned int count();

private:
  unsigned int c = 0;
};

#endif // INCLUDE_COUNTING_H_
