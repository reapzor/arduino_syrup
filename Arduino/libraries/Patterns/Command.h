//Chux

#ifndef Command_h
#define Command_h

#include "Arduino.h"

class CommandBase
{
  public:
    virtual ~CommandBase() {}
    virtual void execute() = 0;
};

template <class T>
class Command : public CommandBase
{
  protected:
    T *m_pReceiver;
  public:
    Command(T &pReceiver):
      m_pReceiver(&pReceiver) {}
	  virtual ~Command() {}
    virtual void execute() = 0;
};

/*
class MyReceiver
{
public:
  void result()
  {
    Serial.println("HI");
  }
};

class MyCommand : public Command<MyReceiver>
{
public:
  MyCommand(MyReceiver &pReceiver):
    Command<MyReceiver>(pReceiver)
  {
  }
  ~MyCommand() {}
  void execute()
  {
    m_pReceiver->result();
  }
};
*/


#endif