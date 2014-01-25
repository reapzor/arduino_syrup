//Chux

#ifndef Subject_h
#define Subject_h

#include "Arduino.h"
#include <Observer.h>
#include <StandardCplusplus.h>
#include <vector>
   
template <class T>
class Subject   
{
  public:
    typedef typename std::vector<Observer<T> *>::iterator t_iterator;
    Subject() {}
    virtual ~Subject() {}
    void attach(Observer<T> *observer)
    {
      m_observers.push_back(observer);
    }
    void detach(Observer<T> *observer)
    {
      t_iterator it;
      for (it = m_observers.begin(); it != m_observers.end(); it++) {
        if ((*it) == observer) {
          m_observers.erase(it);
          break;
        }
      }
    }
    void notify()
    {
      t_iterator it;
      for (it = m_observers.begin(); it != m_observers.end(); it++) {
        (*it)->update(static_cast<T *>(this));
      }
    }
  private:
    std::vector<Observer<T> *> m_observers;
};

#endif