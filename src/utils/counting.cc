#include "counting.h"

Counter::Counter() { c = 0; }

unsigned int Counter::count() { return c; }

void Counter::increment() { c++; }
