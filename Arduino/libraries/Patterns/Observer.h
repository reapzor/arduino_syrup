//Chux
#ifndef Observer_h
#define Observer_h

#include "Arduino.h"

template <class T>
class Observer
{
  public:
    virtual void update(T *subject) = 0;
};

#endif