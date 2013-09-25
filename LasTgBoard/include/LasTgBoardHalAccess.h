#ifndef LASTGBOARDHALACCESS_H
#define LASTGBOARDHALACCESS_H

#include "VMEDevice.hh"
#include "VMEAddressTable.hh"
#include "VMEBusAdapterInterface.hh"

class LasTgBoardHalAccess : public HAL::VMEDevice
{
 public:
  LasTgBoardHalAccess(HAL::VMEAddressTable & addresstable, HAL::VMEBusAdapterInterface & busadapter, unsigned long baseaddress);
 private:
};

#endif
