#include "LasTgBoardHalAccess.h"
#include "VMEAddressTableASCIIReader.hh"

LasTgBoardHalAccess::LasTgBoardHalAccess(HAL::VMEAddressTable & addresstable, HAL::VMEBusAdapterInterface & busadapter, unsigned long baseaddress) : 
  HAL::VMEDevice(addresstable, busadapter,baseaddress)
{
  ;
}
