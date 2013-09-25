#ifndef LASCOMMISSIONER
#define LASCOMMISSIONER

#include "xdaq/WebApplication.h"

#include "xdata/String.h"
#include "xdata/UnsignedShort.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Integer.h"
#include "xdata/Boolean.h"

class LasCommissioner: public xdaq::Application , public xdata::ActionListener
{
 public:
  XDAQ_INSTANTIATOR();

  LasCommissioner(xdaq::ApplicationStub * s); // Constructor
  
  //---------------// 
  // XGI Callbacks //
  //---------------//
  void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
  void StartTriggerBoardDelayScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);  // Start the Trigger Board Delay Scan
  void StopTriggerBoardDelayScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);   // Stop the ongoing Trigger Board Delay Scan
  void StartLaserBoardScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);  // Start the Laser Board Scan
  void StopLaserBoardScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);   // Stop the ongoing Laser Board Scan

  //--------------------//
  // Listener Callbacks //
  //--------------------//
  void actionPerformed (xdata::Event& e) ;        // Callback for Action Listener

 private:
  //---------------//
  // SOAP Messages //
  //---------------//
  // Specific Laser Board Commands
  void SwitchDiodes(int boardid, bool dstate);
  void ScanSettings(int boardid, int level, int delay_first, int delay_last);
  void SetToCounter(int boardid, int level, int delay);

  // Methods for handling Trigger Board Delay Scans
  void StartTgBoardDelayRun();
  void FinishTgBoardDelayRun();
  void StartTgBoardDelayStep();
  void FinishTgBoardDelayStep();

  // Methods for handling Laser Board Scans
  void StartLsBoardRun();
  void FinishLsBoardRun();
  void StartLsBoardStep();
  void FinishLsBoardStep();

 private:
  // Laser Alignment Infospace for communication between XDAQ applications
   xdata::InfoSpace * is;

   bool automatic_refresh;


   // Flags reflecting the availability of Board information in the InfoSpace

   bool tgboarddelay_available;
   bool lsboardlasersarmed_available;



   // Waiting for Laser Boards to finish firing
   bool waiting_for_lsboard_completion;



   // Variables for Trigger Board Delay Scan

   bool running_tgboard_delay_scan;

   xdata::UnsignedShort tgboard_delay_first;
   xdata::UnsignedShort tgboard_delay_last;
   xdata::UnsignedShort tgboard_delay_current;
   xdata::UnsignedShort tgboard_level;



   // Variables for Laser Board Scan

   bool running_lsboard_scan;

   xdata::UnsignedShort lsboard_tgboard_delay;

   xdata::UnsignedShort lsboard_delay_first;
   xdata::UnsignedShort lsboard_delay_last;

   xdata::UnsignedShort lsboard_level_first;
   xdata::UnsignedShort lsboard_level_last;
   xdata::UnsignedShort lsboard_level_current;

   xdata::UnsignedShort lsboard_id_first;
   xdata::UnsignedShort lsboard_id_last;
   xdata::UnsignedShort lsboard_id_current;

   xdata::UnsignedShort lsboard_level_ctr;
   xdata::UnsignedShort lsboard_delay_ctr;

  // Table defining the state of the LsBoards for the scan
  // This should go into some configuration or into the web interface
   enum board_state {OFF, SCAN, CTR};
   std::vector< std::vector<int> > scan_table; 

};

class BadConversion : public std::runtime_error {
public:
  BadConversion(std::string const& s)
    : std::runtime_error(s)
  { }
};

template <class T>
inline std::string stringify(T x)
{
  std::ostringstream o;
  if (!(o << x))
    throw BadConversion("stringify");
  return o.str();
} 



#endif
