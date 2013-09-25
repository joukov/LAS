#ifndef LASTGBOARDSUPERVISOR
#define LASTGBOARDSUPERVISOR

#include "LasTgBoardAccess.h"

#include "xdaq/WebApplication.h"
#include "toolbox/task/Timer.h"

#include "xdata/String.h"
#include "xdata/UnsignedShort.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Integer.h"

class LasTgBoardSupervisor: public xdaq::Application , public xdata::ActionListener, public toolbox::task::TimerListener
{
 public:
  XDAQ_INSTANTIATOR();

  // Constructor  
  LasTgBoardSupervisor(xdaq::ApplicationStub * s);
  
  ////////////////////
  // SOAP Callbacks //
  ////////////////////
  xoap::MessageReference SetSwTriggers     (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetTestMode       (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetTestModeOn     (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetTestModeOff    (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetInteractiveOn  (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetInteractiveOff (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetDelay          (xoap::MessageReference msg) throw (xoap::exception::Exception);
 
  ///////////////////
  // XGI Callbacks //
  ///////////////////
  void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
  void failurePage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
  // Specific XGI Callbacks
  void TgBoardInit(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception); // Initialization
  void SetDelay(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);    // Set the delay of the Trigger Board
  void TestModeOn(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);  // Switch the test mode On
  void TestModeOff(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception); // Switch the test mode Off
  void TriggerA(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);    // Simulate a Trigger on Partition A
  void TriggerB(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);    // Simulate a Trigger on Partition B
  void TriggerC(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);    // Simulate a Trigger on Partition C
  void TriggerD(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);    // Simulate a Trigger on Partition D
  void TriggerRep(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);  // Repetitive Trigger

  ////////////////////////
  // Listener Callbacks //
  ////////////////////////
  void actionPerformed (xdata::Event& e) ;        // Callback for Action Listener
  void timeExpired(toolbox::task::TimerEvent& e); // Callback for Timer Listener

 private:

  xdata::Integer AutoInit_;              // Self-Initialization Code

  bool address_table_loaded; // Flag for address table loading
  bool board_initialized; // Flag for board initialization

  xdata::String LocalConfigFile_;
  xdata::Boolean interactive_mode;

  LasTgBoardAccess lasTgBoardAccess;
  xdata::UnsignedLong BaseAddress_;      // Base Address of VME Board
  xdata::String AddressTableFileName_;   // File containing Address Table
  xdata::UnsignedShort LASTgBoardDelay_; // Delay of Triggere Board Output
  xdata::UnsignedShort repeat_trg;
  xdata::InfoSpace * is;

  xdata::UnsignedShort repeat_trg_ctr;

  toolbox::task::Timer* timer; // Timer for simulated Triggers
};

#endif
