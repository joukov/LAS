#ifndef LASLSBOARDSUPERVISOR
#define LASLSBOARDSUPERVISOR

#include "xdaq/WebApplication.h"
#include "xgi/Method.h"
#include "xdata/UnsignedShort.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Integer.h"
#include "xdata/String.h"
#include "xdata/Vector.h"
#include "xdata/Bag.h"

#include "LasLsBoardAccess.h"
#include <string>
#include <sstream>


#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/NamespaceURI.h"

#include "toolbox/task/Timer.h"

///////////
// SOAP
///////////

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/domutils.h"
#include "xoap/Method.h"

#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"

#include "xgi/WSM.h"
#include "xgi/Utils.h"
#include "xgi/Method.h"

#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"


class BadConversion : public std::runtime_error {
 public:
  BadConversion(const std::string& s)
    : std::runtime_error(s)
    { }
};

template<typename T>
inline void convert(const std::string& s, T& x,
		    bool failIfLeftoverChars = true)
{
  std::istringstream i(s);
  char c;
  if (!(i >> x) || (failIfLeftoverChars && i.get(c)))
    throw BadConversion(s);
}

template<typename T>
inline T convertTo(const std::string& s,
		   bool failIfLeftoverChars = true)
{
  T x;
  convert(s, x, failIfLeftoverChars);
  return x;
}

template<class T>
std::string to_hex(T& val)
{
  std::ostringstream hex_output;
  hex_output << "0x" << std::hex << val;
  return hex_output.str();
}

// Container for Module settings
class ModuleSettings
{
 public: 
  ModuleSettings(): module_delay(0) {;}
  void registerFields(xdata::Bag<ModuleSettings> * bag)
    {               
      bag->addField("Bias", &bias);
      bag->addField("Threshold", &threshold);
      bag->addField("IntensityValues", &intensity_values);
      bag->addField("Selected", &selected);
      bag->addField("ModuleDelay", &module_delay);
      bag->addField("SensorDelays", &sensor_delays);
    }
  xdata::UnsignedInteger bias;
  xdata::UnsignedInteger threshold;
  xdata::Vector<xdata::UnsignedInteger> intensity_values;
  xdata::Vector<xdata::Integer> sensor_delays;
  xdata::Boolean selected;
  xdata::Integer module_delay; // Module specific Delay

  // Create the TECR4 pattern
  std::vector<unsigned short> PatternTECR4();
  // Create the TECR6 pattern
  std::vector<unsigned short> PatternTECR6();
  // Create the AT pattern
  std::vector<unsigned short> PatternAT();
  // Expand a pattern to 128 values
  void PatternExpand(std::vector<unsigned short>& pattern);
  // Convert the pattern to intensity values (a table that can be loaded)
  std::vector<unsigned short> PatternConvertIntensity(std::vector<unsigned short>& pattern);
};

// Container for Board settings
class BoardSettings
{
 public: 
  BoardSettings(): board_delay(0) {;}
  void registerFields(xdata::Bag<BoardSettings> * bag)
    {               
      bag->addField("BoardNr", &boardnr);
      bag->addField("ModuleSettings", &module_settings);
      bag->addField("BeamType", &beam_type);
      bag->addField("TableFile", &table_file);
      bag->addField("RepeatCounter", &repeat_counter);
      bag->addField("BoardDelay", &board_delay);
    }
  xdata::UnsignedInteger boardnr; // Board Identification (Also hardwired on each board and in its address)
  xdata::String beam_type; // Specifies the type of alignment beam (TEC_R4, TEC_R6, AT or FILE)
  xdata::String table_file; // Filename with tables for FILE beam type
  xdata::Vector<xdata::Bag<ModuleSettings> > module_settings; // Individual settings for each of the 8 Laser Modules on the Board
  xdata::UnsignedInteger repeat_counter; // Number of times the table is cycled, before the board stops
  xdata::Integer board_delay; // Delay applied to all Laser Modules on the Board
};

// Interface for Laser Board Operations
class LasLsBoardControl
{
 public:
  typedef std::auto_ptr<LasLsBoardControl> Ptr;
  LasLsBoardControl(unsigned int board_nr, BoardSettings& configuration);
 private:
  unsigned int the_board_nr;
  std::auto_ptr<LasLsBoardAccess> the_LsBoard;
  BoardSettings& the_Settings;
};

// Error class to notify board creation failure
class LsBoardCreationFailed : public std::runtime_error {
 public:
   LsBoardCreationFailed() : std::runtime_error("Board Creation Failed") { }
};

// Laser Board Supervisor
// Able to can address space of boards and create LasLsBoardAccess instances for each board that is found
class LasLsBoardSupervisor: public xdaq::Application , public xdata::ActionListener, public toolbox::task::TimerListener
{
 public:
  XDAQ_INSTANTIATOR();
  static const unsigned int MAX_BOARD_NR;

  // Constructor  
  LasLsBoardSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);

  // Callback for Action Listener
  void actionPerformed (xdata::Event& e) ;
  void timeExpired(toolbox::task::TimerEvent& e); // Callback for Timer Listener

  void UpdateBoards();

  ////////////////////
  // SOAP Callbacks //
  ////////////////////

  // Supervisor Control
  xoap::MessageReference SetInteractiveOn  (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetInteractiveOff (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference BackDoor          (xoap::MessageReference msg) throw (xoap::exception::Exception);

  // Board Control
  xoap::MessageReference SetBoards            ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference SelectBoard          ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference SetRunModeOn         ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference SetRunModeOff        ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference SetStartLoadSequence ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference ArmBoards            ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference TurnOffBoards        ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference SetRunMode           ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference SwitchDiodes         ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference ScanSettings         ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );
  xoap::MessageReference SetToCounter         ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );

  // Module Settings
  xoap::MessageReference SetModuleSettings    (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetThresholdDefault  (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetThresholdScan     (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetBiasDefault       (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetBiasAllZero       (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetBiasScan          (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetRAMIntensityZero  (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetRAMRing4          (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetRAMRing6          (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetRAMAlignmentTubes (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetIntensity1500     (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference SetRAMCustom         (xoap::MessageReference msg) throw (xoap::exception::Exception);

/*   // SOAP utilities (could be private)  */
/*   void SendSOAPmessage (std::string soapMessageToSend, std::string whichSupervisor); */
/*   xoap::MessageReference SOAPReply(const std::string& message); */
/*   std::string GetSoapCommand (xoap::MessageReference msg); */
/*   std::string GetSoapAttribute(xoap::MessageReference msg, const std::string& attribute); */


  ///////////////////
  // XGI Callbacks //
  ///////////////////  

  // Main Callback of XDAQ Application (Creates the HTML Page)
  void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // when hardware fails show this page
  void failurePage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Generic Callback for executing simple commands
  void Command(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Specific Callbacks for more complex actions
  // Initialization
  void LsBoardInit(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Scans for existing Boards and puts them in LasBoardList
  void BoardScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Arm all Boards
  void ArmBoards(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Turn all Boards Off
  void TurnOffBoards(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Select the Board
  void SelectBoard(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Apply the Laser Module Settings
  //void SetModules2(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Callback for Laser Module Settings
  void SetModules(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  //void SetModules() throw (xgi::exception::Exception);

  // Load File into buffer
  //void LsBoardLoadFile(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

 private:
  enum LsBoard_cmd {
    EMPTY, 
    RUNMODE_ON, 
    RUNMODE_OFF, 
    REGDUMP, 
    MANTRIGGER_ON, 
    MANTRIGGER_OFF, 
    REFRESH,
    CLEAR_LDT,
    STARTLDSEQ,
    THRESH_DEFAULT,
    THRESH_SCAN,
    BIAS_DEFAULT,
    BIAS_ZERO,
    BIAS_SCAN,
    INTENSITY_ZERO,
    INTENSITY_1500,
    INTENSITY_TECR4,
    INTENSITY_TECR6,
    INTENSITY_AT,
    INTENSITY_FILE,
    DELAY_SCAN,
    INTENSITY_SCAN,
    LATENCY_SCAN,
    LAS_RUN,
    CUSTOM
  }; // Commands for the Buttons

  typedef std::vector<std::vector<unsigned short> > RAMTable;

  // Helpers for creating the different forms
  void FormBoardInit(xgi::Output * out );   // Form for Supervisor Initialization
  void FormBoardScan(xgi::Output * out );   // Form for Board Scan

  void FormRunType(xgi::Output * out ); // Form for Selecting the Run Type
  void FormBoardSelect(xgi::Output * out ); // Form for Selecting the Board
  void FormBoardSettings(xgi::Output * out ); // Form for the Board Settings
  void FormBoardControl(xgi::Output * out );   // Buttons for Board Control
  void BoardStatus(xgi::Output * out );   // Display Board Status
  void FormModuleSettings(xgi::Input * in, xgi::Output * out );   // Form for Module Settings

  std::string checked(bool flag){return flag ? " checked = \"checked\" " : "";}
  std::string disabled(bool flag){return flag ? " disabled = \"disabled\" " : "";}
  std::string enabled(bool flag){return flag ? "" : " disabled = \"disabled\" ";}
  void MakeButton(xgi::Output * out, const std::string& label, const std::string& method, bool dis=false, LsBoard_cmd command=EMPTY);
  void MakeSwitch(xgi::Output * out, const std::string& title, bool state, LsBoard_cmd on_command, LsBoard_cmd off_command, bool dis=false);
  void MakeSwitch2(xgi::Output * out, const std::string& title, bool state, LsBoard_cmd on_command, LsBoard_cmd off_command, bool dis=false);

  // Methods for some more complex actions
  // Scans for existing Boards and puts them in LasBoardList
  void Scan();
  // Looks up configuration and associates it to boards
  //void LookUpConfig();
  // Settings for Laser Modules
  void SetBoards();
  // Scan for proper threshold settings
  void ScanThresh();
  // Scan for proper bias settings
  void ScanBias();
  // Load a file with Laser Module Settings
  void LoadFile(const std::string& filename, RAMTable& delay, RAMTable& width, RAMTable& ampli);
  //Arm all Boards
  void ArmBoards();
  // Turn all Boards Off
  void TurnOffBoards();

  // Create the TECR4 pattern
  std::vector<unsigned short> PatternTECR4();

  // Create the TECR6 pattern
  std::vector<unsigned short> PatternTECR6();

  // Create the AT pattern
  std::vector<unsigned short> PatternAT();

  // Duplicate every entry in the pattern
  void DoublePattern(std::vector<unsigned short>& pattern);

  // Expand a pattern to 128 values
  void PatternExpand(std::vector<unsigned short>& pattern);

  // Convert the pattern to intensity values (a table that can be loaded)
  std::vector<unsigned short> PatternConvertIntensity(std::vector<unsigned short>& pattern, int lasmod);

  // Convert the pattern to delay values (a table that can be loaded)
  std::vector<unsigned short> PatternConvertDelay(std::vector<unsigned short>& pattern, int lasmod, unsigned short offset=0);

  // Convert the level to an intensity value
  unsigned short LevelConvert(int boardid, int lasmod, unsigned short level);

  int GetBoardIdx(int board_id); // Return the index for a given board id (to be used with the lists LsBoardList, BoardNrList and ConfigIndexList

 private:
  xdata::Boolean expert_mode;
  bool boards_initialized;
  bool address_table_loaded; // Flag for address table loading

  xdata::String LocalConfigFile_;
  xdata::String AddressTableFileName_;
  xdata::Integer AutoInit_;              // Self-Initialization Code
  xdata::String RunType_;


  xdata::Boolean double_entries; // If true, all entries in the tables will be duplicated

  xdata::Vector<xdata::Bag<BoardSettings> > board_settings_; // Container for the Board Settings

  // The following lists are all of the same size and selected_board_index points to a valid entry or is -1 otherwise
  int selected_board_index; // index of board that has been selected
  std::vector<LasLsBoardAccess*> LsBoardList;  // List with Laser Boards
  std::vector<LasLsBoardControl*> LsBoardList2;  // List with Laser Boards
  std::vector<xdata::Integer> BoardNrList; // List with Laser Board Numbers
  std::vector<int> ConfigIndexList; // index of board_settings_ for each board (-1 if non-existent)

  // Variables for storing the Form information
  LsBoard_cmd thresh_choice;    // Threshold radio button that has been selected
  LsBoard_cmd bias_choice;      // Bias radio button that has been selected
  LsBoard_cmd intensity_choice; // RAM Table radio button that has been selected
  LsBoard_cmd runtype_choice; // Run Type radio button that has been selected

  std::string TableFileName;

  xdata::InfoSpace * is;
  xdata::Boolean lasers_armed; // Is set to true when lasers are armed

  toolbox::task::Timer* timer; // Timer for updating board runmode

};

#endif
