#ifndef LASTGBOARDACCESS
#define LASTGBOARDACCESS

#include "LasTgBoardHalAccess.h"
#include "xdata/UnsignedShort.h"
#include "xdata/Boolean.h"
#include <string>

class LasTgBoardAccess
{
 public:
  LasTgBoardAccess();
  virtual ~LasTgBoardAccess();
  void Initialize(const std::string & addresstablefile, unsigned long baseaddress=0xD);
  void SetDelay(xdata::UnsignedShort delay);
  xdata::UnsignedShort GetDelay();
  void TestModeOn();
  void TestModeOff();
  xdata::Boolean GetStatus();
  int TriggerA();
  int TriggerB();
  int TriggerC();
  int TriggerD();

 private:
  LasTgBoardHalAccess * halaccess;
  HAL::VMEBusAdapterInterface * busadapter;
  HAL::VMEAddressTable * addresstable;
};


#endif
