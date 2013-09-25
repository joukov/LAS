#include "LasLsBoardAccess.h"

#include <VMEAddressTableASCIIReader.hh>
//#include <VME64xAddressTableASCIIReader.hh>

#include <VMEDummyBusAdapter.hh>
#include "CAENLinuxBusAdapter.hh"

#include <fstream>
#include <sstream>

//FD 28/02/2006
//#define LINUX 1
//#include <CAEN2718LinuxPCIBusAdapter.hh> old version of HAL

using namespace HAL ;

// Initialize static members
HAL::VMEBusAdapterInterface* LasLsBoardAccess::busadapter=0;
HAL::VMEAddressTable * LasLsBoardAccess::addresstable=0;
const unsigned int LasLsBoardAccess::TABLE_SIZE = 128;

std::runtime_error no_hal_access("No HAL Access! Did you initialize the board?");

LasLsBoardAccess::LasLsBoardAccess(unsigned long baseaddress):
  halaccess(0)
{
  if(busadapter != 0 && addresstable!=0){
    halaccess= new LasLsBoardHalAccess(*addresstable, *busadapter, baseaddress);
  }
}

LasLsBoardAccess::~LasLsBoardAccess()
{
  delete halaccess;
}

void LasLsBoardAccess::Initialize(const std::string & addresstablefile)
{
  delete busadapter;
  delete addresstable;

  VMEAddressTableASCIIReader ATReader(addresstablefile);
  addresstable = new VMEAddressTable("LAS Laser Board", ATReader);
  //addresstable->print();


  // Latest P5 version
  //busadapter = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718) ;

  // C_RACK version
  busadapter = new CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718, 0, 0, HAL::CAENLinuxBusAdapter::A3818);

  //std::cout << "BusAdapter has been created" << std::endl;

  return;
}

// Check if board is responding
bool LasLsBoardAccess::Accessible()
{
  if(!halaccess) return false;

  try{
    halaccess->readPulse("ReadBoardId");
  }
  catch(HAL::BusAdapterException & e){
    return false;
  }
  return true;
}


///////////////////////////////////
// Laser Board Command Registers //
///////////////////////////////////

void LasLsBoardAccess::RunModeOff()
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("RunModeOff");
  return;
}

void LasLsBoardAccess::RunModeOn()
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("RunModeOn");
  return ;
}

void LasLsBoardAccess::ManTriggerOff()  // Switch Manual Triggers Off
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("ManTriggerOff");
  return ;
}

void LasLsBoardAccess::ManTriggerOn()   // Switch Manual Triggers On
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("ManTriggerOn");
  return ;
}

void LasLsBoardAccess::TriggerGenOff()  // Switch Trigger Generator Off
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("TriggerGenOff");
  return ;
}

void LasLsBoardAccess::TriggerGenOn()   // Switch Trigger Generator On
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("TriggerGenOn");
  return ;
}

void LasLsBoardAccess::LsModTestOff()   // Switch Laser Module Test Off
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("LsModTestOff");
  return ;
}

void LasLsBoardAccess::LsModTestOn()    // Switch Laser Module Test On
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("LsModTestOn");
  return ;
}

void LasLsBoardAccess::ResetInterlock() // Reset Interlock Flag
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("ResetInterlock");
  return ;
}


////////////////////////////////////
// Laser Board Control Registers //
////////////////////////////////////

void LasLsBoardAccess::SetLaserOnSel(uint32_t data)                   // Select Laser Modules
{
  if(!halaccess)throw(no_hal_access);
  halaccess->write("LaserOnSel",data);
  return;
}

unsigned short LasLsBoardAccess::GetLaserOnSel()                            // Get selected Laser Modules
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result=0;
  halaccess->read("LaserOnSel",&result);
  return result;
}

void LasLsBoardAccess::SetFaultIgnore(uint32_t data)                   // Set Fault Ignore Bit
{
  if(!halaccess)throw(no_hal_access);
  halaccess->write("FaultIgnore",data);
  return;
}

unsigned short LasLsBoardAccess::GetFaultIgnore()                            // Get Fault Ignore Bit
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("FaultIgnore",&result);
  return result;
}

unsigned short LasLsBoardAccess::GetLaserOn()                              // Check which Laser Modules are ON
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("LaserOn",&result);
  return result;
}

unsigned short LasLsBoardAccess::GetLaserDiodeTrip()                   // Get Laser Diode Trip
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("LaserDiodeTrip",&result);
  return result;
}

unsigned short LasLsBoardAccess::GetShotPtr()                               // Read Shot Pointer
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("ShotPtr",&result);
  return result;
}

void LasLsBoardAccess::SetRepeatCounter(uint32_t data)                 // Set RepeatCounter
{
  if(!halaccess)throw(no_hal_access);
  halaccess->write("RepeatCounter",data);
  return;
}

uint32_t LasLsBoardAccess::GetRepeatCounter()                          // Get RepeatCounter
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("RepeatCounter",&result);
  return result;
}

void LasLsBoardAccess::SetBeamType(std::string beamtype)                 // Set RepeatCounter
{
  beam_type=beamtype;
}

std::string LasLsBoardAccess::GetBeamType()
{
  return beam_type;
}

unsigned short LasLsBoardAccess::GetExecCounter()                            // Get Execution Counter
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("ExecCounter",&result);
  return result;
}


unsigned short LasLsBoardAccess::GetTablesLoaded03()                         // Check if Tables are loaded (Modules 0-3)
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("TablesLoaded03",&result);
  return result;
}

unsigned short LasLsBoardAccess::GetTablesLoaded47()                         // Check if Tables are loaded (Modules 4-7)
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("TablesLoaded47",&result);
  return result;
}

unsigned short LasLsBoardAccess::GetWriteFlags03()                           // Get Write Flags (Modules 0-3)
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("WriteFlags03",&result);
  return result;
}

unsigned short LasLsBoardAccess::GetWriteFlags47()                           // Get Write Flags (Modules 4-7)
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("WriteFlags47",&result);
  return result;
}

unsigned short LasLsBoardAccess::GetWritePtr()                               // Get Write Pointer
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("WritePtr",&result);
  return result;
}

unsigned short LasLsBoardAccess::GetGenFlags()                               // Get General Flags
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("GenFlags",&result);
  return result;
}

bool LasLsBoardAccess::GetRunMode()                                          // True if Run Mode is ON and Enabled
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("GenFlags",&result);
  return (result & 0x0C) == 0x0C;
}

bool LasLsBoardAccess::GetManTrigger()                                      // True if Manual Trigger is ON
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  halaccess->read("GenFlags",&result);
  return result & 0x10;
}

std::string  LasLsBoardAccess::BoardId()
{
  if(!halaccess)throw(no_hal_access);

  uint32_t data=0;
  std::string boardid;

  for(int i=0; i<10; i++){
    halaccess->read("ReadBoardId",&data,i*2);
    if(data >= 32 && data <= 126)boardid=boardid + (char)data; // Add only printable ASCII characters
  }

  return boardid;
}


///////////////////////
// Laser Module RAMs //
///////////////////////
void LasLsBoardAccess::MemWriteEnable(unsigned short lasmod, unsigned short ram)                         // Memory Write Enable
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13 | ram << 9;
  halaccess->writePulse("MemWriteEnable",offset);
  return;
}

void LasLsBoardAccess::MemWrtEnDelay(unsigned short lasmod)                                              // Delay Memory Write Enable
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13;
  halaccess->writePulse("MemWrtEnDelay",offset);
  return;
}

void LasLsBoardAccess::MemWrtEnWidth(unsigned short lasmod)                                              // Width Memory Write Enable
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13;
  halaccess->writePulse("MemWrtEnWidth",offset);
  return;
}

void LasLsBoardAccess::MemWrtEnAmp(unsigned short lasmod)                                                // Amplitude Memory Write Enable
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13;
  halaccess->writePulse("MemWrtEnAmp",offset);
  return;
}

void LasLsBoardAccess::MemWrite(unsigned short data)                                                     // Memory Write
{
  if(!halaccess)throw(no_hal_access);
  halaccess->write("MemWrite",data);
  return;
}

unsigned short LasLsBoardAccess::MemRead(unsigned short lasmod, unsigned short ram, unsigned short addr) // Memory Read
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  unsigned long offset= lasmod << 13 | ram << 9 | addr << 1;
  halaccess->read("MemRead",&result,offset);
  return result;
}

unsigned short LasLsBoardAccess::MemReadDelay(unsigned short lasmod, unsigned short addr)                // Delay Memory Read
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  unsigned long offset= lasmod << 13 | addr << 1;
  halaccess->read("MemReadDelay",&result,offset);
  return result;
}

unsigned short LasLsBoardAccess::MemReadWidth(unsigned short lasmod, unsigned short addr)                // Width Memory Read
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  unsigned long offset= lasmod << 13 | addr << 1;
  halaccess->read("MemReadWidth",&result,offset);
  return result;
}

unsigned short LasLsBoardAccess::MemReadAmp(unsigned short lasmod, unsigned short addr)                  // Amplitude Memory Read
{
  if(!halaccess)throw(no_hal_access);
  uint32_t result;
  unsigned long offset= lasmod << 13 | addr << 1;
  halaccess->read("MemReadAmp",&result,offset);
  return result;
}

void LasLsBoardAccess::StartLdSeq()                                                                       // Start Load Sequence
{
  if(!halaccess)throw(no_hal_access);
  halaccess->writePulse("StartLdSeq");
  return ;
}

////////////////////////////
// Laser Module Registers //
////////////////////////////

void LasLsBoardAccess::ClearDevice()      //Clear DACs
{
  if(!halaccess)throw(no_hal_access);

  halaccess->writePulse("ClearDevice");
  return ;
}

// void LasLsBoardAccess::WriteLsModReg(unsigned short lasmod, unsigned short reg, unsigned short data) //Write Laser Module Register
// {
//   if(!halaccess)return;
//   unsigned long offset = lasmod << 12 | reg << 8;
//   halaccess->write("WriteLsModReg",data,HAL_NO_VERIFY,offset);
//   return;
// }

// unsigned short LasLsBoardAccess::ReadLsModReg(unsigned short lasmod, unsigned short reg)             //Read Laser Module Register
// {
//   if(!halaccess)return 0;
//   unsigned long offset = lasmod << 12 | reg << 8;
//   unsigned long result;
//   halaccess->read("ReadLsModReg",&result,offset);
//   return result;
// }

// unsigned short LasLsBoardAccess::GetLsModDelay(unsigned short lasmod)                                // Get Laser Module Delay
// {
//   if(!halaccess)return 0;
//   unsigned long offset = lasmod << 12;
//   unsigned long result;
//   halaccess->read("GetLsModDelay",&result,offset);
//   return result;
// }

// unsigned short LasLsBoardAccess::GetLsModWidth(unsigned short lasmod)                                // Get Laser Module Width
// {
//   if(!halaccess)return 0;
//   unsigned long offset = lasmod << 12;
//   unsigned long result;
//   halaccess->read("GetLsModWidth",&result,offset);
//   return result;
// }

// unsigned short LasLsBoardAccess::GetLsModAmpl(unsigned short lasmod)                                 // Get Laser Module Amplitude
// {
//   if(!halaccess)return 0;
//   unsigned long offset = lasmod << 12;
//   unsigned long result;
//   halaccess->read("GetLsModAmpl",&result,offset);
//   return result;
// }


unsigned short LasLsBoardAccess::GetLsModBias(unsigned short lasmod)                                 // Get Laser Module Bias
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13;
  uint32_t result;

  halaccess->read("GetLsModBias",&result,offset);
 
  return result;
}

unsigned short LasLsBoardAccess::GetLsModThr(unsigned short lasmod)                                  // Get Laser Module Threshold
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13;
  uint32_t result;
  halaccess->read("GetLsModThr",&result,offset);
  return result;
}

void LasLsBoardAccess::SetLsModBias(unsigned short lasmod, uint32_t data)                      // Set Laser Module Bias
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13;
  halaccess->write("SetLsModBias",data,HAL_NO_VERIFY,offset);
  return;
}

void LasLsBoardAccess::SetLsModThr(unsigned short lasmod, uint32_t data)                       // Set Laser Module Threshold
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13;
  halaccess->write("SetLsModThr",data,HAL_NO_VERIFY,offset);
  return;
}

void LasLsBoardAccess::ClearLDT(unsigned short lasmod)                                               // Clear Laser Diode Trip
{
  if(!halaccess)throw(no_hal_access);
  unsigned long offset = lasmod << 13;
  halaccess->writePulse("ClearLDT",offset);
  return;
}

// ///////////////////////////
// // Higher Level Routines //
// ///////////////////////////

int LasLsBoardAccess::Dump()
{
  if(!halaccess)throw(no_hal_access);

  std::cout << std::hex;
  std::cout << "GetLaserOnSel: 0x" << GetLaserOnSel() << std::endl;
  std::cout << "GetFaultIgnore: 0x" << GetFaultIgnore() << std::endl;
  std::cout << "GetLaserOn: 0x" << GetLaserOn() << std::endl;
  std::cout << "GetLaserDiodeTrip: 0x" << GetLaserDiodeTrip() << std::endl;
  std::cout << std::dec;
  std::cout << "GetShotPtr: " << GetShotPtr() << std::endl;
  std::cout << "GetRepeatCounter: " << GetRepeatCounter() << std::endl;
  std::cout <<   "GetExecCounter: " << GetExecCounter() << std::endl;
  std::cout << std::hex;
  std::cout <<   "GetTablesLoaded03: 0x" << GetTablesLoaded03() << std::endl;
  std::cout <<   "GetTablesLoaded47: 0x" << GetTablesLoaded47() << std::endl;
  std::cout <<   "GetWriteFlags03: 0x" << GetWriteFlags03() << std::endl;
  std::cout <<   "GetWriteFlags47: 0x" << GetWriteFlags47() << std::endl;
  std::cout <<   "GetWritePtr: 0x" << GetWritePtr() << std::endl;
  std::cout <<   "GetGenFlags: 0x" << GetGenFlags() << std::dec << std::endl;

  std::ofstream out("FPGALOAD.out");
  for(int lasmod=0; lasmod < 8; lasmod++){
    for(int ram=0; ram < 3; ram++){
      for(unsigned int addr=0; addr < TABLE_SIZE; addr++){
	out << lasmod << "\t" << ram << "\t" << addr << "\t" << MemRead(lasmod, ram, addr) << "\n";
      }
    }    
  }

  return 0;
}

void LasLsBoardAccess::LoadTable(unsigned short lasmod, RAM_type ram, const std::vector<unsigned short>& table) // Load a table into RAM
{
  std::vector<unsigned short>::size_type size=table.size();
  if(size < TABLE_SIZE)
    std::cerr << "Warning: The table size for RAM " << ram << " of module " << lasmod << " is less than " << TABLE_SIZE << std::endl;
  else if(size > TABLE_SIZE)
    std::cerr << "Warning: The table size for RAM " << ram << " of module " << lasmod << " is larger than " << TABLE_SIZE << std::endl;

  //std::cout << "Loading Table" << std::endl;
  switch(ram){
  case RAM_DELAY:
    MemWrtEnDelay(lasmod);
    break;
  case RAM_WIDTH:
    MemWrtEnWidth(lasmod);
    break;
  case RAM_AMPLI:
    MemWrtEnAmp(lasmod);
    break;
  default:
    throw std::runtime_error("Unknown RAM type");
  }
  for(std::vector<unsigned short>::size_type i=0; i < TABLE_SIZE; i++){
    unsigned short value = 0;
    if(i<size) value = table[i];
 
    unsigned short result = value+1;

    MemWrite(value);
    result = MemRead(lasmod, ram, i); // Read now what was written

    if(result != value){
      std::ostringstream message;
      message << "Mismatch when programming FPGA lasmod " << lasmod << " ram " << ram << " pos " << i << "\n value " << value << "  reads back: " << result; 
      throw std::runtime_error(message.str());
    }
  }
  return;
}

// void LasLsBoardAccess::ReadTable(unsigned short lasmod, RAM_type ram, std::vector<unsigned short>& table) // Read a table from RAM
// {
//   table.resize(TABLE_SIZE);
//   std::cout << "Reading Table" << std::endl;
//   for(std::vector<unsigned short>::size_type i=0; i < TABLE_SIZE; i++){
//     switch(ram){
//     case Delay:
//       table[i]=MemReadDelay(lasmod, i);
//       break;
//     case Width:
//       table[i]=MemReadWidth(lasmod, i);
//       break;
//     case Ampl:
//       table[i]=MemReadAmp(lasmod, i);
//       break;
//     default:
//       cerr << "Unknown RAM type" << std::endl;
//       break;
//     }
//     std::cout << table[i] << "\t" << std::endl;
//   }
//   return;
// }
