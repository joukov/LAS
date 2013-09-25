#include "LasTgBoardAccess.h"

#include <VMEAddressTableASCIIReader.hh>
//#include <VME64xAddressTableASCIIReader.hh>

#include <VMEDummyBusAdapter.hh>
#include "CAENLinuxBusAdapter.hh"

//FD 28/02/2006
//#define LINUX 1
//#include <CAEN2718LinuxPCIBusAdapter.hh> old version of HAL

using namespace HAL ;

LasTgBoardAccess::LasTgBoardAccess():
  halaccess(0),
  busadapter(0),
  addresstable(0)
{
;
}

LasTgBoardAccess::~LasTgBoardAccess()
{
  delete halaccess;
  delete busadapter;
  delete addresstable;
}

void LasTgBoardAccess::Initialize(const std::string & addresstablefile, unsigned long baseaddress)
{
  delete halaccess;
  delete busadapter;
  delete addresstable;

  //busadapter = new  VMEDummyBusAdapter();
  //FD 28/02/2006
  //busadapter = new CAEN2718LinuxPCIBusAdapter(); old version of HAL
  
  // Latest P5 version
  //busadapter = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718) ;

  // C_RACK version
  busadapter = new CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718, 0, 0, HAL::CAENLinuxBusAdapter::A3818);

  //std::cout << "BusAdapter has been created" << std::endl;

  VMEAddressTableASCIIReader ATReader(addresstablefile);
  //VME64xAddressTableASCIIReader ATReader(addresstablefile);
  addresstable = new VMEAddressTable("LAS Trigger Board", ATReader);
  //addresstable->print();

  halaccess= new LasTgBoardHalAccess(*addresstable, *busadapter, baseaddress);

  return;
}

void LasTgBoardAccess::SetDelay(xdata::UnsignedShort delay)
{
  if(!halaccess)return;

  halaccess->write("Delay",delay);
  return;
}

xdata::UnsignedShort LasTgBoardAccess::GetDelay()
{
  if(!halaccess)return 0;

  // KH, modified
  //   unsigned long result;
  uint32_t result;
  halaccess->read("Delay",&result);
  return result;
}

void LasTgBoardAccess::TestModeOn()
{
  if(!halaccess)return;

  halaccess->writePulse("TestModeOn");
  return ;
}

void LasTgBoardAccess::TestModeOff()
{
  if(!halaccess)return;

  halaccess->writePulse("TestModeOff");
  return;
}

xdata::Boolean LasTgBoardAccess::GetStatus()
{
  if(!halaccess)return 0;

  // KH modified
  //  unsigned long result;
  uint32_t result;
  halaccess->read("Status",&result);
  return result;
}

int LasTgBoardAccess::TriggerA()
{
  if(!halaccess)return -1;

  halaccess->writePulse("SimulateTriggerTTCA");
  return 0;
}

int LasTgBoardAccess::TriggerB()
{
  if(!halaccess)return -1;

  halaccess->writePulse("SimulateTriggerTTCB");
  return 0;
}

int LasTgBoardAccess::TriggerC()
{
  if(!halaccess)return -1;

  halaccess->writePulse("SimulateTriggerTTCC");
  return 0;
}

int LasTgBoardAccess::TriggerD()
{
  if(!halaccess)return -1;

  halaccess->writePulse("SimulateTriggerTTCD");
  return 0;
}
