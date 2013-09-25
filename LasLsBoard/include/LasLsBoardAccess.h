#ifndef LASLSBOARDACCESS
#define LASLSBOARDACCESS

#include "LasLsBoardHalAccess.h"
#include "xdata/UnsignedShort.h"
#include "xdata/Boolean.h"
#include <string>

class LasLsBoardAccess
{
 public:
  enum RAM_type {RAM_DELAY=0, RAM_WIDTH=1, RAM_AMPLI=2}; // RAM Tables for Laser Modules
  static const unsigned int TABLE_SIZE;

  LasLsBoardAccess(unsigned long baseaddress=0xD);
  virtual ~LasLsBoardAccess();

  static void Initialize(const std::string & addresstablefile);

  bool Accessible();  // Check if board is responding

  ///////////////////////////////////
  // Laser Board Command Registers //
  ///////////////////////////////////
  void RunModeOff();     // Switch Run Mode Off
  void RunModeOn();      // Switch Run Mode On
  void ManTriggerOff();  // Switch Manual Triggers Off
  void ManTriggerOn();   // Switch Manual Triggers On
  void TriggerGenOff();  // Switch Trigger Generator Off
  void TriggerGenOn();   // Switch Trigger Generator On
  void LsModTestOff();   // Switch Laser Module Test Off
  void LsModTestOn();    // Switch Laser Module Test On
  void ResetInterlock(); // Reset Interlock Flag

  ////////////////////////////////////
  // Laser Board Control Registers  //
  ////////////////////////////////////
  void SetLaserOnSel(uint32_t data);                    // Set Selected Laser Modules
  unsigned short GetLaserOnSel();                       // Get Selected Laser Modules
  void SetFaultIgnore(uint32_t data);                   // Set Fault Ignore Bit
  unsigned short GetFaultIgnore();                      // Get Fault Ignore Bit
  unsigned short GetLaserOn();                          // Check which Laser Modules are ON 
  unsigned short GetLaserDiodeTrip();                   // Laser Diode Trip
  unsigned short GetShotPtr();                          // Read Shot Pointer
  void SetRepeatCounter(uint32_t data);                 // Set RepeatCounter
  uint32_t GetRepeatCounter();                          // Get RepeatCounter
  unsigned short GetExecCounter();                      // Get Execution Counter
  unsigned short GetTablesLoaded03();                   // Check if Tables are loaded (Modules 0-3)
  unsigned short GetTablesLoaded47();                   // Check if Tables are loaded (Modules 4-7)
  unsigned short GetWriteFlags03();                     // Get Write Flags (Modules 0-3)
  unsigned short GetWriteFlags47();                     // Get Write Flags (Modules 4-7)
  unsigned short GetWritePtr();                         // Get Write Pointer
  unsigned short GetGenFlags();                         // Get General Flags
  bool GetRunMode();                                    // True if Run Mode is ON
  bool GetManTrigger();                                 // True if Manual Trigger is ON
  std::string  BoardId();                               // Return the Board ID


  void SetBeamType(std::string beamtype); 
  std::string GetBeamType();



/*   // Laser Module RAMs */
  void MemWriteEnable(unsigned short lasmod, unsigned short ram);                         // Memory Write Enable
  void MemWrtEnDelay(unsigned short lasmod);                                              // Delay Memory Write Enable
  void MemWrtEnWidth(unsigned short lasmod);                                              // Width Memory Write Enable
  void MemWrtEnAmp(unsigned short lasmod);                                                // Amplitude Memory Write Enable
  void MemWrite(unsigned short data);                                                     // Memory Write
  unsigned short MemRead(unsigned short lasmod, unsigned short ram, unsigned short addr); // Memory Read
  unsigned short MemReadDelay(unsigned short lasmod, unsigned short addr);                // Delay Memory Read
  unsigned short MemReadWidth(unsigned short lasmod, unsigned short addr);                // Width Memory Read
  unsigned short MemReadAmp(unsigned short lasmod, unsigned short addr);                  // Amplitude Memory Read
  void StartLdSeq();                                                                      // Start Load Sequence

  // Laser Module Registers
  void ClearDevice();      //Clear DACs
/*   void WriteLsModReg(unsigned short lasmod, unsigned short reg, unsigned short data); //Write Laser Module Register */
/*   unsigned short ReadLsModReg(unsigned short lasmod, unsigned short reg);             //Read Laser Module Register */
/*   unsigned short GetLsModDelay(unsigned short lasmod);                                // Get Laser Module Delay */
/*   unsigned short GetLsModWidth(unsigned short lasmod);                                // Get Laser Module Width */
/*   unsigned short GetLsModAmpl(unsigned short lasmod);                                 // Get Laser Module Amplitude */
  unsigned short GetLsModBias(unsigned short lasmod);                                 // Get Laser Module Bias
  unsigned short GetLsModThr(unsigned short lasmod);                                  // Get Laser Module Threshold
  void SetLsModBias(unsigned short lasmod,  uint32_t data);                      // Set Laser Module Bias 
  void SetLsModThr(unsigned short lasmod, uint32_t data);                       // Set Laser Module Threshold 
  void ClearLDT(unsigned short lasmod);                                               // Clear Laser Diode Trip

/*   // Higher Level Routines */
  //void loadFPGAs();
  int Dump(); // Dump some infos to stdout
  void LoadTable(unsigned short lasmod, RAM_type ram, const std::vector<unsigned short>& table); // Load a table into RAM 
/*   void ReadTable(unsigned short lasmod, RAM_type ram, std::vector<unsigned short>& table); // Read a table from RAM */

 private:

  LasLsBoardHalAccess * halaccess;
  static HAL::VMEBusAdapterInterface * busadapter;
  static HAL::VMEAddressTable * addresstable;
  //int beam_type;
  std::string beam_type;
  //int data [8][3][128]; // Buffer for FPGA RAMs
};

#endif
