#include "LasLsBoardSupervisor.h"

#include "cgicc/HTMLClasses.h"

#include <sstream>
#include <iostream>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>

#include "toolbox/task/TimerFactory.h"

#include <xdata/XMLDOM.h>
#include <InfoSpaceFactory.h>
#include <xdaq/exception/ConfigurationError.h>

#include "LasCommon.h"
using namespace LaserAlignment;

// Initialize static class members
const unsigned int LasLsBoardSupervisor::MAX_BOARD_NR = 8;

LasLsBoardControl::LasLsBoardControl(unsigned int board_nr, BoardSettings& configuration) :
  the_board_nr(board_nr),
  the_LsBoard(new LasLsBoardAccess(board_nr << 28)),
  the_Settings(configuration)
{
  // Try to access the board
  std::string boardid_ (the_LsBoard->BoardId());
  // One could add a crosscheck here, to see if the boardid matches board_nr
}


XDAQ_INSTANTIATOR_IMPL(LasLsBoardSupervisor);

LasLsBoardSupervisor::LasLsBoardSupervisor(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception): 
  xdaq::Application(s),
  expert_mode(true),
  boards_initialized(false),
  address_table_loaded(false),
  AddressTableFileName_(""),
  AutoInit_(-1),
  double_entries(false),
  selected_board_index(-1),
  thresh_choice(THRESH_DEFAULT),
  bias_choice(BIAS_ZERO),
  intensity_choice(INTENSITY_FILE),
  runtype_choice(LAS_RUN),
  TableFileName(""),
  lasers_armed(false)
{

   /////////////////////////
  // bind SOAP  meesages
  /////////////////////////

  xoap::bind(this, &LasLsBoardSupervisor::BackDoor,               "BackDoor", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::SetInteractiveOn,       "SetInteractiveOn",  XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetInteractiveOff,      "SetInteractiveOff", XDAQ_NS_URI );

  xoap::bind(this, &LasLsBoardSupervisor::ArmBoards,              "ArmBoards", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::TurnOffBoards,          "TurnOffBoards", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::SwitchDiodes,           "SwitchDiodes", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::ScanSettings,           "ScanSettings", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::SetToCounter,           "SetToCounter", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::SetBoards,              "SetBoards", XDAQ_NS_URI ); 

  // The following should be reviewed
  xoap::bind(this, &LasLsBoardSupervisor::SelectBoard,            "SelectBoard", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::SetRunModeOn,           "SetRunModeOn", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetRunModeOff,          "SetRunModeOff", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetStartLoadSequence,   "SetStartLoadSequence", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::SetModuleSettings,      "SetModuleSettings", XDAQ_NS_URI );

  xoap::bind(this, &LasLsBoardSupervisor::SetThresholdDefault,    "SetThresholdDefault", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::SetThresholdScan,       "SetThresholdScan", XDAQ_NS_URI );  
  xoap::bind(this, &LasLsBoardSupervisor::SetBiasDefault,         "SetBiasDefault", XDAQ_NS_URI ); 
  xoap::bind(this, &LasLsBoardSupervisor::SetBiasAllZero,         "SetBiasAllZero", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetBiasScan,            "SetBiasScan", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetRAMIntensityZero,    "SetRAMIntensityZero", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetRAMRing4,            "SetRAMRing4", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetRAMRing6,            "SetRAMRing6", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetRAMAlignmentTubes,   "SetRAMAlignmentTubes", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetIntensity1500,       "SetIntensity1500", XDAQ_NS_URI );
  xoap::bind(this, &LasLsBoardSupervisor::SetRAMCustom,           "SetRAMCustom", XDAQ_NS_URI ); 
  // Review until here

  // Bind the XGI callbacks 
  xgi::bind(this,&LasLsBoardSupervisor::Default, "Default");  
  xgi::bind(this,&LasLsBoardSupervisor::LsBoardInit, "LsBoardInit");
  xgi::bind(this,&LasLsBoardSupervisor::BoardScan, "BoardScan");
  xgi::bind(this,&LasLsBoardSupervisor::ArmBoards, "ArmBoards");
  xgi::bind(this,&LasLsBoardSupervisor::TurnOffBoards, "TurnOffBoards");
  xgi::bind(this,&LasLsBoardSupervisor::SelectBoard, "SelectBoard");
  xgi::bind(this,&LasLsBoardSupervisor::Command, "Command");
  xgi::bind(this,&LasLsBoardSupervisor::SetModules, "SetModules");

  // Set default values for Address Table Filename
  char *basic = getenv ("ENV_CMS_TK_LASLSBOARD") ;
  if (basic != NULL) {
    std::ostringstream vmeFileOff ;
    vmeFileOff << basic << "LASLsBoardAddressTable.txt" ;
    AddressTableFileName_ = vmeFileOff.str() ;
  }
  else{
    AddressTableFileName_="./LasLsBoard/LASLsBoardAddressTable.txt";
  }

  // Export Variables that should be visible
  getApplicationInfoSpace()->fireItemAvailable("ExpertMode", &expert_mode);
  getApplicationInfoSpace()->fireItemAvailable("LocalConfigFile", &LocalConfigFile_);
  getApplicationInfoSpace()->fireItemAvailable("AddressTableFileName", &AddressTableFileName_);
  getApplicationInfoSpace()->fireItemAvailable("AutoInit", &AutoInit_);
  getApplicationInfoSpace()->fireItemAvailable("BoardSettings", &board_settings_);
  getApplicationInfoSpace()->fireItemAvailable("RunType", &RunType_);
  getApplicationInfoSpace()->fireItemAvailable("DoubleEntries", &double_entries);

  // Add Listener for configuration
  getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");

  // Publish to InfoSpace

  try{
    is = xdata::InfoSpaceFactory::getInstance()->get("TrackerLaserAlignmentInfoSpace");
  }
  catch(xdata::exception::Exception){
    is = xdata::InfoSpaceFactory::getInstance()->create("TrackerLaserAlignmentInfoSpace");
  }
  is->fireItemAvailable("LsBoardLasersArmed", &lasers_armed, this);

  //is = xdata::InfoSpaceFactory::getInstance()->get("TrackerLaserAlignmentInfoSpace");
  //is->fireItemAvailable("LsBoardLasersArmed", &lasers_armed, this);

  // Install a timer to verify the runmode of the boards
  timer = toolbox::task::TimerFactory::getInstance()->createTimer("LasLsBoardUpdater");
  toolbox::TimeVal start(0,0);
  toolbox::TimeInterval period(1,0);
  timer->scheduleAtFixedRate ( start, this, period, 0, "BoardUpdate");
  //timer->scheduleAtFixedRate ( start, this, period , 0, "EvtTrg");
  //timer->stop();
}


// Callback for Events
void LasLsBoardSupervisor::actionPerformed (xdata::Event& e) 
{
  static bool local_config_loaded = false; // This variable avoids infinite recursive calls when local configuration is applied

  if ( e.type() == "urn:xdaq-event:setDefaultValues" ){
    LOG4CPLUS_INFO (getApplicationLogger(), "Checking if local config needs to be loaded " << "AutoInit_: " << AutoInit_ << "  local_config_loaded: " << (local_config_loaded ? "true" : "false"));
    if(!local_config_loaded){
      try{
	LOG4CPLUS_INFO( getApplicationLogger(), 
			"\nDefault values have been set for LasLsBoardSupervisor by XDAQ server\nLoading local xml configuration  " << std::string(LocalConfigFile_)
			);
	//xdata::XMLDOMLoader loader;
	xdata::XMLDOMLoader loader (AbstractDOMParser::Val_Never);

	DOMDocument * document = loader.load(LocalConfigFile_);
	local_config_loaded = true; // Loading local configuration should be tried only once, since there is a recursive call (see below)
	setDefaultValues(document->getDocumentElement()); // WARNING: This line will generate a recursive call of this function
      }
      catch(xdaq::exception::ParameterSetFailed & e){
	LOG4CPLUS_INFO (getApplicationLogger(), 
			"\nException 'xdaq::exception::ParameterSetFailed' when trying to initialize LasLsBoardSupervisor with file " << std::string(LocalConfigFile_)
			<< "\nexception error message: " << e.message()
			<< "\nline in which the exception occurred: " << e.line()
			<< "\nfunction in which the exception occurred: " << e.function()
			<< "\nmodule in which the exception occurred: " << e.module()
			<< "\n Exception reporting: " << e.what() << "\n");
      }
      catch(xdaq::exception::ConfigurationError & e){
	LOG4CPLUS_INFO (getApplicationLogger(), 
			"\nException 'xdaq::exception::ConfigurationError' when trying to initialize LasLsBoardSupervisor with file " << std::string(LocalConfigFile_)
			<< "\nexception error message: " << e.message()
			<< "\nline in which the exception occurred: " << e.line()
			<< "\nfunction in which the exception occurred: " << e.function()
			<< "\nmodule in which the exception occurred: " << e.module()
			<< "\n Exception reporting: " << e.what() << "\n");
      }
      catch(xdaq::exception::Exception & e){
	LOG4CPLUS_INFO (getApplicationLogger(),
			"\nException 'xdaq::exception::Exception' when trying to initialize LasLsBoardSupervisor with file " << std::string(LocalConfigFile_)
			<< "\nexception error message: " << e.message()
			<< "\nline in which the exception occurred: " << e.line()
			<< "\nfunction in which the exception occurred: " << e.function()
			<< "\nmodule in which the exception occurred: " << e.module()
			<< "\n Exception reporting: " << e.what() << "\n");
      }
      catch(std::exception & e){
	LOG4CPLUS_INFO (getApplicationLogger(),
			"\nException 'std::exception' when trying to initialize LasLsBoardSupervisor with file " << std::string(LocalConfigFile_)
			<< "\n Exception reporting: " << e.what() << "\n");
      }

    }

    LOG4CPLUS_INFO (getApplicationLogger(), "Checking if boards need to be initialized " << "AutoInit_: " << AutoInit_ << "  boards_initialized: " << (boards_initialized ? "true" : "false"));
    if(AutoInit_ == 1 && !boards_initialized ){
      try {
	// Evaluate Run Type
	LOG4CPLUS_INFO (getApplicationLogger(),"Run Type " << std::string(RunType_));
	if(RunType_ == "LAS_RUN") runtype_choice = LAS_RUN;
	if(RunType_ == "DELAY_SCAN") runtype_choice = DELAY_SCAN;
	if(RunType_ == "INTENSITY_SCAN") runtype_choice = INTENSITY_SCAN;
	if(RunType_ == "LATENCY_SCAN") runtype_choice = LATENCY_SCAN;

	// Initialize the Supervisor
        LasLsBoardAccess::Initialize(AddressTableFileName_);
	address_table_loaded = true;
	// Scan for existing Boards
        Scan();
	// Load Board Settings from configuration
        SetBoards();

        //time_t rawtime;
        //time ( &rawtime );
      }
      catch (const std::exception & e){
	//XCEPT_RAISE(xgi::exception::Exception, e.what());
	//SendSOAPmessage("failedLaserBoard",    "LasSupervisor"); // This function can throw an exception!!!!!!!!!!!!!!!!!!!!
	LOG4CPLUS_INFO (getApplicationLogger(), "Laser Board could not be initialized. Check if it is On/Operating properly. \nException: " << e.what());
      }    
    }
  }// End of initialization after default values have been set
}

void LasLsBoardSupervisor::UpdateBoards()
{
  // Check if runmode has been disabled (Max number of Triggers was reached) and runmode is still on
  // If yes, turn runmode off
  // It would be better to move this to a separate method called something like UpdateBoards
  bool flag1 = false;
  bool flag2 = true;
  for(unsigned short i=0; i < LsBoardList.size(); i++){
    LasLsBoardAccess* lsb = LsBoardList[i];// Pointer to Laser Board
    bool runmodeen = (lsb->GetGenFlags() & 0x04); // Runmode Enabled
    bool runmodeon = (lsb->GetGenFlags() & 0x08); // Runmode ON
    if(runmodeon && !runmodeen){
      lsb->RunModeOff();
      flag1 = true;
    }
    else flag2 = false;
  }
  if(flag1 && flag2){
    lasers_armed = false;
    is->fireItemValueChanged("LsBoardLasersArmed", this);
  }
}

// Default Callback of XDAQ Application (Creates HTML page)
void LasLsBoardSupervisor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
//   // Check if runmode has been disabled (Max number of Triggers was reached) and runmode is still on
//   // If yes, turn runmode off
//   // It would be better to move this to a separate method called something like UpdateBoards
//   bool flag1 = false;
//   bool flag2 = true;
//   for(unsigned short i=0; i < LsBoardList.size(); i++){
//     LasLsBoardAccess* lsb = LsBoardList[i];// Pointer to Laser Board
//     bool runmodeen = (lsb->GetGenFlags() & 0x04); // Runmode Enabled
//     bool runmodeon = (lsb->GetGenFlags() & 0x08); // Runmode ON
//     if(runmodeon && !runmodeen){
//       lsb->RunModeOff();
//       flag1 = true;
//     }
//     else flag2 = false;
//   }
//   if(flag1 && flag2){
//     lasers_armed = false;
//     is->fireItemValueChanged("LsBoardLasersArmed", this);
//   }

  // HTML header stuff
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict);
  *out << cgicc::html().set("lang", "en").set("dir","ltr");
  *out << cgicc::title("LAS Laser Board Supervisor");
  //  *out << cgicc::a("Visit our Web site").set("href","http://accms04.physik.rwth-aachen.de/~cms/Tracker/Alignment-Hardware/") << endl;
  *out << cgicc::h1("LAS Laser Board  Supervisor");
  //  *out << cgicc::hr();


  if(!expert_mode){
    // Automatic Reloading of HTML page every 3 seconds
    // Added string manipulation to remove xgi commands
    *out << "<script>";
    *out << "window.onload=function(){";
    *out << "var expression = window.location.host + \"/.*/\";";
    *out << "var thematch = window.location.href.match(\".*\" + expression);";
    *out << "var thecommand = 'window.location.replace(\"' + thematch + '\")';";
    //*out << "window.alert(thecommand);";
    *out << "window.setTimeout(thecommand,3000)";
    *out << "};";
    *out << "</script>";
  }


  // Create a table for organizing the HTML Page
  *out << "<table border=0 cellpadding=0 cellspacing=0 width=\"100%\" bgcolor=\"#FFFFFF\">";
  //*out << "<colgroup><col width=\"200\"><col><col width=\"200\"></colgroup>";

//   if(expert_mode){
//     *out << cgicc::tr(); // Begin Row 1
//     *out << cgicc::td();
//     FormRunType(out);
//     *out << cgicc::td();
//     *out << cgicc::tr(); // End Row 1
//   }

  *out << cgicc::tr(); // Begin Row 2
  *out << cgicc::td();
  FormBoardSelect(out);
  *out << cgicc::td();
  *out << cgicc::tr(); // End Row 2


  *out << cgicc::tr(); // Begin Row 3
  *out << cgicc::td();
  *out << "<table border=0 cellpadding=0 cellspacing=0 width=\"100%\" bgcolor=\"#FFFFFF\">";
  *out << "<colgroup><col><col width=\"200\"></colgroup>";
  *out << "<tr><td>";
  BoardStatus(out);
  *out << "</td><td>";
  FormBoardControl(out);
  *out << "</td></tr>";
  *out << "</table>";
  *out << cgicc::td();
  *out << cgicc::tr(); // End Row 3


  if(expert_mode){
    *out << cgicc::tr(); // Begin Row 4
    *out << cgicc::td().set("colspan","3"); //Columns 1 + 2 + 3 together
    FormModuleSettings(in, out);
    *out << cgicc::td();
    *out << cgicc::tr(); // End Row 4

    *out << cgicc::tr(); // Begin Row 5
    *out << cgicc::td();
    *out << "<table border=0 cellpadding=0 cellspacing=0 width=\"100%\" bgcolor=\"#FFFFFF\">";
    *out << "<colgroup><col width=\"200\"><col><col width=\"200\"></colgroup>";
    *out << "<tr><td>";
    FormBoardScan(out);
    *out << "</td><td>";
    FormBoardInit(out);
    *out << "</td></tr>";
    *out << "</table>";
    *out << cgicc::td();
    *out << cgicc::tr(); // End Row 5
  }

  *out << "</table>";  // End table for organizing the HTML Page
  return;
}

void LasLsBoardSupervisor::failurePage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("LAS Laser Board Supervisor") << std::endl;
  *out << cgicc::hr() << std::endl;
  *out << cgicc::h1("LAS Laser Board Supervisor: Hardware failure ") << std::endl;
  *out << cgicc::h2("The Laser Board could not be initialized. Check if it is ON/operating properly ") << std::endl;
  *out << cgicc::hr() << std::endl;
}


// Form for Supervisor Initialization
void LasLsBoardSupervisor::FormBoardInit(xgi::Output * out )
{
  // String containing the method for the form
  std::string init_method = toolbox::toString("/%s/LsBoardInit",getApplicationDescriptor()->getURN().c_str());

  // Toggle enabling/disabling of fields	
  std::string disabled="";
  if (address_table_loaded) disabled="disabled";

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  //  *out << std::endl;
  *out << cgicc::legend("Initialize the Board");
  *out << cgicc::form().set("method","GET").set("action", init_method);

  *out << "<table border=0 cellpadding=0 cellspacing=5>"; // Fieldset formatting table
  *out << "<tr><td>";
  *out << "Address Table File Name "; 
  *out << "</td><td>";
  *out << cgicc::input().set("type","text").set(disabled,disabled).set("name","LsBoardAddressTableFile").set("value", AddressTableFileName_.toString()).set("size","70");
  *out << "</td><td>";
  *out << cgicc::input().set("type","submit").set(disabled,disabled).set("value","Initialize");
  *out << "</td></tr></table>";

  *out << cgicc::form();
  *out << cgicc::fieldset();
}

// Form for Board Scan
void LasLsBoardSupervisor::FormBoardScan(xgi::Output * out )
{
  // String containing the method for the form
  std::string boardscan_method = toolbox::toString("/%s/BoardScan",getApplicationDescriptor()->getURN().c_str());

  // Toggle enabling/disabling of fields	
  std::string disabled="disabled";
  if (address_table_loaded) disabled="";

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << cgicc::legend("Scan for existing Boards");
  *out << cgicc::form().set("method","GET").set("action", boardscan_method);
  *out << cgicc::input().set("type","submit").set(disabled,disabled).set("value","Scan");
  *out << cgicc::form();
  *out << cgicc::fieldset();
}


// Forms for selecting the run type
void LasLsBoardSupervisor::FormRunType(xgi::Output * out )
{
  //cgicc::Cgicc cgi(in);

  // Strings containing the methods for the forms
  //std::string command_method = toolbox::toString("/%s/Command",getApplicationDescriptor()->getURN().c_str());
  //std::string setmodules_method = toolbox::toString("/%s/SetModules",getApplicationDescriptor()->getURN().c_str());
    std::string refresh_method = toolbox::toString("/%s/Default",getApplicationDescriptor()->getURN().c_str());

  std::string disabled="";
  if (!boards_initialized) disabled="disabled";

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << cgicc::legend("Run Type");

  *out << cgicc::form().set("method","GET").set("action", refresh_method);


  *out << "<table border=0 cellpadding=0 cellspacing=0>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"runtype\" value=" << LAS_RUN << checked(runtype_choice == LAS_RUN) << "> LAS Run<br>";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"runtype\" value=" << DELAY_SCAN << checked(runtype_choice == DELAY_SCAN) << "> Delay Scan<br>";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"runtype\" value=" << INTENSITY_SCAN << checked(runtype_choice == INTENSITY_SCAN) << "> Intensity Scan<br>";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"runtype\" value=" << LATENCY_SCAN << checked(runtype_choice == LATENCY_SCAN) << "> Latency Scan<br>";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << cgicc::input().set(disabled,disabled).set("type","submit").set("value","APPLY");
  *out << "</td></tr>";
  *out << "</table>";

  *out << cgicc::form();
  *out << cgicc::fieldset();
}


// Form for Selecting the Board
void LasLsBoardSupervisor::FormBoardSelect(xgi::Output * out )
{
  // String with board selection callback
  std::string board_select_method = toolbox::toString("/%s/SelectBoard",getApplicationDescriptor()->getURN().c_str());

  // Toggle enabling/disabling of fields	
  std::string disabled="disabled";
  if (boards_initialized) disabled="";

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << cgicc::legend("Select the Laser Board");

  *out << "<table border=0 cellpadding=0 cellspacing=0><tr>"; // Fieldset formatting table

  *out << "<td>";
  // Table: One row with selection buttons and one row which indicates the board that is actually selected
  // It would be nice to integrate everything into the button label...
  // added Row to display automatical selection
  *out << "<table border=0 cellpadding=10 cellspacing=5>";

  // Row with selection Buttons
  *out << "<tr>";
  for(unsigned short i=0; i < LsBoardList.size(); i++){
    //*out << "<td>";

    if(LsBoardList[i]->GetGenFlags() & 0x08)
      *out << "<td  bgcolor=\"#00FF00\">";
    else
      *out << "<td  bgcolor=\"#FF0000\">";

    std::ostringstream label;
    label << "Board " << BoardNrList[i];

    *out << cgicc::form().set("method","GET").set("action", board_select_method);
    *out << cgicc::input().set("type","hidden").set("name","boardselect").set("value",toolbox::toString("%d",i));
    *out << cgicc::input().set(disabled,disabled).set("type","submit").set("value",label.str());
    *out << cgicc::form();

    *out << "</td>";
  }
  *out << "</tr>";

  // Row that indicates, which board is actually selected
  *out << "<tr align=\"center\">";
  for(unsigned short i=0; i < LsBoardList.size(); i++){
    *out << "<td>";
    if(selected_board_index == i) 
      *out << "V--sel--V";
    *out << "</td>";
  }
  *out << "</tr>";
  *out << "</table>"; // End of board selection table
  *out << "</td>";

  // Spacer
  //*out << "<td>------------</td>";

  if(expert_mode){
    *out << "<td align=\"center\" width=100>";
    std::string armboards_method = toolbox::toString("/%s/ArmBoards",getApplicationDescriptor()->getURN().c_str());
    MakeButton(out, "Arm Boards", armboards_method, !boards_initialized);
    *out << "</td>";
  
    *out << "<td align=\"center\" width=100>";
    std::string turnoffboards_method = toolbox::toString("/%s/TurnOffBoards",getApplicationDescriptor()->getURN().c_str());
    MakeButton(out, "Boards Off", turnoffboards_method, !boards_initialized);
    *out << "</td>";
  }

  *out << "</tr></table>"; // End of fieldset formatting table

  *out << cgicc::fieldset();
}

// Buttons for Board Control
void LasLsBoardSupervisor::FormBoardControl(xgi::Output * out )
{
  if(selected_board_index <0)return;

  // Strings containing the methods for the forms
  std::string command_method = toolbox::toString("/%s/Command",getApplicationDescriptor()->getURN().c_str());
  std::string scanbias_method = toolbox::toString("/%s/ScanBias",getApplicationDescriptor()->getURN().c_str());

  LasLsBoardAccess* lsb = LsBoardList[selected_board_index];// Pointer to selected Laser Board
  bool runmodeen = (lsb->GetGenFlags() & 0x04); // Runmode Enabled
  bool runmodeon = (lsb->GetGenFlags() & 0x08); // Runmode ON
  bool mantrg  = (lsb->GetGenFlags() & 0x10);   // Manual Trigger State

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << cgicc::legend("Board Control");

  *out << "<table border=0 cellpadding=0 cellspacing=10>";

  *out << "<tr><td>";
  // RunMode Switch
  MakeSwitch2(out, "RunMode", runmodeon, RUNMODE_ON, RUNMODE_OFF, !boards_initialized || !runmodeen || !expert_mode);
  *out << "</td></tr>";

  *out << "<tr><td>";
  // ManTrigger Switch
  MakeSwitch2(out, "ManTrigger", mantrg, MANTRIGGER_ON, MANTRIGGER_OFF, !boards_initialized || !expert_mode);
  *out << "</td></tr>";

  *out << "<tr><td>";
  // Clear StartLdSeq
  MakeButton(out, "StartLdSeq", command_method, !boards_initialized || runmodeon || !expert_mode , STARTLDSEQ);
  *out << "</td></tr>";

  *out << "<tr><td>";
  // Clear LDT
  MakeButton(out, "Clear Trips", command_method, !boards_initialized, CLEAR_LDT);
  *out << "</td></tr>";

  *out << "</table>";

  *out << cgicc::fieldset();
}


// Helper to create rows for status table (see below)
// The eight lowest bits of 'flags' are tested and a corresponding enty for the table row is created
void MakeTableRow(xgi::Output * out , std::string header, std::string text_true, std::string text_false, unsigned short flags){
  // Write the header (1st column)
  *out << "<tr><th>" << header << "</th>";
  // loop over the eight lowest bits of 'flags'
  for(int i=0; i<8;i++){ // Test the bit for this module
    std::string text_=text_false;
    std::string bgcolor_="FF0000";
    if(flags & (1<<i)){
      text_=text_true;
      bgcolor_="00FF00";
    }
    *out << "<td  bgcolor=\"#" << bgcolor_ << "\">" << text_ << "</td>";
  }
  *out << "</tr>";
}

// Display Board Status
void LasLsBoardSupervisor::BoardStatus(xgi::Output * out )
{
  if(selected_board_index <0)return; // Selection Option 'All' is not implemented yet, so just return

  LasLsBoardAccess* lsb = LsBoardList[selected_board_index]; // Pointer to selected Laser Board

  try{
    // Retrieve the Board Status
    xdata::UnsignedShort sel_       = lsb->GetLaserOnSel();     // Laser Selected
    xdata::UnsignedShort fltign_    = lsb->GetFaultIgnore();
    xdata::UnsignedShort on_        = lsb->GetLaserOn();        // LaserON
    xdata::UnsignedShort trip_      = lsb->GetLaserDiodeTrip(); // LaserDiodeTrip
    xdata::UnsignedShort shtptr_    = lsb->GetShotPtr();
    xdata::UnsignedShort rptctr_    = lsb->GetRepeatCounter();
    xdata::UnsignedShort exectr_    = lsb->GetExecCounter();
    xdata::UnsignedShort tblld03_   = lsb->GetTablesLoaded03();
    xdata::UnsignedShort tblld47_   = lsb->GetTablesLoaded47();
    xdata::UnsignedShort wrtflg03_  = lsb->GetWriteFlags03();
    xdata::UnsignedShort wrtflg47_  = lsb->GetWriteFlags47();
    xdata::UnsignedShort wrtptr_    = lsb->GetWritePtr();
    xdata::UnsignedShort genflags_  = lsb->GetGenFlags();
    std::string boardid_            = lsb->BoardId();

    // Toggle enabling/disabling of fields	
    std::string disabled="disabled";
    if (boards_initialized) disabled="";

    *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
    *out << cgicc::legend("Board Status " + boardid_);

    // Create tables for organizing the output
    *out << "<table border=0 cellpadding=10 cellspacing=10>";// Start invisible table with 2 columns

    *out << "<td><table border=0>"; // Column with Trigger counter Refresh button
    *out << "<tr><td>Received " << std::dec << 100*exectr_+shtptr_-1 << "</td></tr>";
    *out << "<tr><td>Limit: " << rptctr_*100 << "</td></tr>";
    *out << "<tr><td>";
    std::string refresh_method = toolbox::toString("/%s/Default",getApplicationDescriptor()->getURN().c_str());
    MakeButton(out, "Refresh", refresh_method, !boards_initialized);
    *out << "</td></tr>";
    *out << "<tr><td>";
    std::string command_method = toolbox::toString("/%s/Command",getApplicationDescriptor()->getURN().c_str());
    MakeButton(out, "Dump", command_method, !boards_initialized , REGDUMP);  // Dump Button
    *out << "</td></tr>";
    *out << "</table></td>";

    *out << "<td><table border=1>"; // Start Table for Module Status
    // Header
    *out << cgicc::tr() << "<th>Module</th><th>1</th><th>2</th><th>3</th><th>4</th><th>5</th><th>6</th><th>7</th><th>8</th>" << cgicc::tr();

    // Module ON
    MakeTableRow(out, "On", "YES", "NO", on_);
    
    // Module Selected
    MakeTableRow(out, "Selected", "YES", "NO", sel_);
    
    // Diode Tripped
    MakeTableRow(out, "Trip", "OK", "Trip", ~trip_);

    // Fault Ignore
    MakeTableRow(out, "Fault Ignore", "OFF", "ON", ~fltign_);

    // Bias
    *out << "<tr><th>Bias</th>";
    for(int i=0; i<8;i++){
      *out << "<td>" << lsb->GetLsModBias(i) << "</td>";
    }
    *out << "</tr>";

    // Threshold
    *out << "<tr><th>Threshold</th>";
    for(int i=0; i<8;i++){
      *out << "<td>" << lsb->GetLsModThr(i) << "</td>";
    }
    *out << "</tr>";

    *out << "</table></td>"; // End Table for Module Status

    //*out << "<td><table border=0>"; // Start Table for register Values
    //*out << "<tr><td>GetShotPointer(): " << std::dec << shtptr_ << "</td></tr>";
    //*out << "<tr><td>GetRepeatCounter(): " << rptctr_ << "</td></tr>";
    //*out << "<tr><td>GetExecCounter(): " << exectr_ << "</td></tr>";
    //*out << "<tr><td>GetTableLoaded03: " << std::hex << "0x" << tblld03_ << "</td></tr>";
    //*out << "<tr><td>GetTableLoaded47(): " << "0x" << tblld47_ << "</td></tr>";
    //*out << "<tr><td>GetWriteFlags03: " << std::hex << "0x" << wrtflg03_ << "</td></tr>";
    //*out << "<tr><td>GetWriteFlags47: " << std::hex << "0x" << wrtflg47_ << "</td></tr>";
    //*out << "<tr><td>GetWritePointer(): " << wrtptr_ << "</td></tr>";
    //*out << "<tr><td>GetGenFlags(): " << "0x" << genflags_ << std::dec << "</td></tr>";
    //*out << "</table></td>"; // End Table for register Values
        
    *out << "</table>";// End invisible table with 3 columns
    
    *out << cgicc::fieldset();
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }

}


// Forms for Laser Module Settings
void LasLsBoardSupervisor::FormModuleSettings(xgi::Input * in, xgi::Output * out )
{
  cgicc::Cgicc cgi(in);

  //std::cout << "FormModuleSettings Entry threshold: " << cgi["threshold"]->getValue() << std::endl;

  // Strings containing the methods for the forms
  //std::string command_method = toolbox::toString("/%s/Command",getApplicationDescriptor()->getURN().c_str());
  std::string setmodules_method = toolbox::toString("/%s/SetModules",getApplicationDescriptor()->getURN().c_str());

  std::string disabled="";
  if (!boards_initialized) disabled="disabled";

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << cgicc::legend("Module Settings");

  *out << cgicc::form().set("method","GET").set("action", setmodules_method);

  *out << "<table border=0 cellpadding=0 cellspacing=20>";

  // Column for Thresholds
  *out << "<tr><td valign=\"top\">";

  *out << "<table border=0 cellpadding=0 cellspacing=0>";
  *out << "<tr><th align=\"left\">";
  *out << "Thresholds";
  *out << "</th></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"threshold\" value=" << THRESH_DEFAULT << checked(thresh_choice == THRESH_DEFAULT) << "> Default<br>";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"threshold\" value=" << THRESH_SCAN << checked(thresh_choice == THRESH_SCAN) << "> Scan<br>";
  *out << "</td></tr>";
  *out << "</table>";
  *out << "</td>";

  // Column for Bias
  *out << "<td valign=\"top\">";
  *out << "<table border=0 cellpadding=0 cellspacing=0>";
  *out << "<tr><th align=\"left\">";
  *out << "Bias";
  *out << "</th></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"bias\" value=" << BIAS_DEFAULT << checked(bias_choice == BIAS_DEFAULT) << "> Default<br>";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"bias\" value=" << BIAS_ZERO << checked(bias_choice == BIAS_ZERO) << "> All Zero<br>";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"bias\" value=" << BIAS_SCAN << checked(bias_choice == BIAS_SCAN) << "> Scan<br>";
  *out << "</td></tr>";
  *out << "</table>";
  *out << "</td>";

  // Column for RAM Tables
  *out << "<td valign=\"top\">";
  *out << "<table border=0 cellpadding=0 cellspacing=0>";
  *out << "<tr><th align=\"left\">";
  *out << "RAM Tables";
  *out << "</th></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"intensity\" value=" << INTENSITY_ZERO << checked(intensity_choice == INTENSITY_ZERO) << "> Intensity Zero";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"intensity\" value=" << INTENSITY_TECR4 << checked(intensity_choice == INTENSITY_TECR4) << "> TEC Ring4";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"intensity\" value=" << INTENSITY_TECR6 << checked(intensity_choice == INTENSITY_TECR6) << "> TEC Ring6";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"intensity\" value=" << INTENSITY_AT << checked(intensity_choice == INTENSITY_AT) << "> Alignment Tubes";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"intensity\" value=" << INTENSITY_1500 << checked(intensity_choice == INTENSITY_1500) << "> Intensity 1500";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "<input type=\"radio\" name=\"intensity\" value=" << INTENSITY_FILE << checked(intensity_choice == INTENSITY_FILE) << "> Custom";
  *out << "</td></tr>";
  *out << "<tr><td>";
  *out << "File: ";
  *out << cgicc::input().set("type","text").set("name","LsBoardInputFile").set("value", TableFileName).set("size","30");  *out << "</td></tr>";
  *out << "</table>";
  *out << "</td>";

  // Column for Commands
  *out << "<td>";
  *out << cgicc::input().set(disabled,disabled).set("type","submit").set("value","APPLY");
  *out << "</td></tr>";

  //*out << "<td>";
  //SetModules2();
  //*out << "</td></tr>";

  *out << "</table>";

  *out << cgicc::form();
  *out << cgicc::fieldset();
 
  
}

void LasLsBoardSupervisor::MakeButton(xgi::Output * out, const std::string& label, const std::string& method, bool dis, LsBoard_cmd command)
{
  // Toggle enabling/disabling of the button
  std::string disabled="";
  if (dis) disabled="disabled";

  *out << cgicc::form().set("method","GET").set("action", method);
  *out << cgicc::input().set("type","hidden").set("name","Command").set("value",toolbox::toString("%d",command));
  *out << cgicc::input().set(disabled,disabled).set("type","submit").set("value",label);
  *out << cgicc::form();
}

void LasLsBoardSupervisor::MakeSwitch(xgi::Output * out, const std::string& title, bool state, LsBoard_cmd on_command, LsBoard_cmd off_command, bool dis)
{
  std::string command_method = toolbox::toString("/%s/Command",getApplicationDescriptor()->getURN().c_str());
  std::string bgcolor_="FF0000";
  if(state==true)  bgcolor_="00FF00";

  *out << "<fieldset style=\"font-size: 10pt;  font-family: arial;\">";
  *out << cgicc::legend(title);
  *out << "<table border=0 cellpadding=10 cellspacing=0>";
  *out << "<tr><td  bgcolor=\"#" << bgcolor_ << "\">";
  // On Button
  MakeButton(out, "ON", command_method, dis || state, on_command);
  *out << "</td><td  bgcolor=\"#" << bgcolor_ << "\">";
  // Off Button
  MakeButton(out, "OFF", command_method, dis || !state, off_command);
  *out << "</td></tr>";
  *out << "</table>";
  *out << "</fieldset>";
}

void LasLsBoardSupervisor::MakeSwitch2(xgi::Output * out, const std::string& title, bool state, LsBoard_cmd on_command, LsBoard_cmd off_command, bool dis)
{
  std::string command_method = toolbox::toString("/%s/Command",getApplicationDescriptor()->getURN().c_str());

  *out << "<fieldset style=\"font-size: 10pt;  font-family: arial;\">";
  *out << cgicc::legend(title);
  *out << "<table border=0 cellpadding=10 cellspacing=0>";

  if(state==true){
    *out << "<tr><td  bgcolor=\"#00FF00\">";
    // Button to turn Off
    MakeButton(out, "ON", command_method, dis, off_command);
  }
  else{
    *out << "<tr><td  bgcolor=\"#FF0000\">";
    // Button to turn On
    MakeButton(out, "OFF", command_method, dis, on_command);
  }
  *out << "</td></tr>";
  *out << "</table>";
  *out << "</fieldset>";
}

// Scans for existing Boards and puts them in LasBoardList
void LasLsBoardSupervisor::BoardScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    Scan();
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	

  // Call again the Default callback
  this->Default(in,out);		
}

// Scans for existing Boards and puts them in LasBoardList
void LasLsBoardSupervisor::Scan()
{
  // Empty the existing Lists
  while(LsBoardList.size() > 0){
    delete LsBoardList.back();
    LsBoardList.pop_back();
  }
  BoardNrList.clear();
  ConfigIndexList.clear();
  selected_board_index=-1;

  // Number of boards found in configuration
  int conf_size = board_settings_.size();
  LOG4CPLUS_INFO (getApplicationLogger(), "Found " << conf_size << " boards in configuration"); 

  // Loop over possible board numbers
  for(unsigned int bnr=1; bnr <= MAX_BOARD_NR; bnr++){
    // Try to create and access the board
    LasLsBoardAccess* lba = new LasLsBoardAccess(bnr << 28);
    if( ! (lba->Accessible()) ){
      delete lba;
      LOG4CPLUS_INFO (getApplicationLogger(), "Board Nr." << bnr <<  " is not accessible"); 
    }
    else{
      LOG4CPLUS_INFO (getApplicationLogger(), "Board Nr." << bnr <<  " can be created");
      LsBoardList.push_back(lba);
      BoardNrList.push_back(bnr);

      // Look for configuration of this board
      int config_index = -1;
      for(int cfi=0; cfi < conf_size; cfi++){ // Loop over all boards found in the configuration
	if(bnr == board_settings_[cfi].bag.boardnr){
	  LOG4CPLUS_INFO (getApplicationLogger(),"Found Board Nr: " <<  board_settings_[cfi].bag.boardnr << " in configuration"); 
	  config_index = cfi;
	  break;
	}
      }
      ConfigIndexList.push_back(config_index);
    }
  }
  if(LsBoardList.size()!=0) selected_board_index=0;  // Select first board by default
  boards_initialized = true;

  return;

  //New implementation that is meant to replace the previous one
//     try{
//       int conf_size=board_settings_.size(); // Number of boards found in configuration
//       int cfi=0;
//       for(cfi=0; cfi < conf_size; cfi++){ // Loop over all boards found in the configuration
// 	if(bnr == board_settings_[cfi].bag.boardnr){
// 	  //std::cout << "Found Board Nr: " << board_settings_[cfi].bag.boardnr << " in configuration" << std::endl;
// 	  LOG4CPLUS_INFO (getApplicationLogger(),"Found Board Nr: " <<  board_settings_[cfi].bag.boardnr << " in configuration");
// 	  break;
// 	}
//       }
//       LasLsBoardControl* lbpt = new LasLsBoardControl(bnr,board_settings_[cfi].bag);
//       LsBoardList2.push_back(lbpt);
//       LOG4CPLUS_INFO (getApplicationLogger(), "LasLsBoardControl for Board Nr." << bnr <<  " can be created");
//     }
//     catch(HAL::HardwareAccessException& e){
//       LOG4CPLUS_INFO (getApplicationLogger(), "LasLsBoardControl for Board Nr." << bnr <<  " could not be created"); 
//     }
  
}

// Set Boards according to values from XML configuration
void LasLsBoardSupervisor::SetBoards()
{
  // Create delay table
  // This should move into the loop and be specific for each Laser Module
  // Then it should be configurable via xml
  std::vector<unsigned short> delay(LasLsBoardAccess::TABLE_SIZE,   0);
  switch(runtype_choice){
  case DELAY_SCAN:
    for(std::vector<unsigned short>::size_type i=0; i < 100; i++) delay[i]=(i*5)%100;
    break;
  default:
    break;
  }

  int selected_board_index_backup = selected_board_index;

  for(unsigned int sci=0; sci < BoardNrList.size(); sci++){ // Lopp over all boards found in the scan
    int cfi=ConfigIndexList[sci];
    if(cfi >= 0){
      selected_board_index = sci;
      LasLsBoardAccess* lsb = LsBoardList[sci];
      LOG4CPLUS_INFO (getApplicationLogger(),"Configuring Board " << BoardNrList[sci]);
      lsb->RunModeOff(); // Switch Run mode Off
      lsb->ClearDevice(); // Clear Bias and threshold
      lsb->SetRepeatCounter(board_settings_[cfi].bag.repeat_counter); // Set Repeat Counter

      int board_delay = board_settings_[cfi].bag.board_delay;

      // Create intensity table
      // Evaluate Beam Type
      std::string bt = board_settings_[cfi].bag.beam_type;
      LsBoardList[sci]->SetBeamType(bt); // Set Beam type (not used so far)
      std::vector<unsigned short> pattern(100,0);
      if(bt == "TECR4") pattern = PatternTECR4();
      if(bt == "TECR6") pattern = PatternTECR6();
      if(bt == "AT")    pattern = PatternAT();
      if(bt == "FIXED")    pattern = std::vector<unsigned short>(100,43) ; // Not very nice, there is the magic number 43, should go into configuration
      if(bt == "DELAY") {
	for(std::vector<unsigned short>::size_type i=0; i < 100; i++) pattern[i]= i/20+1;
      }

      if(double_entries) DoublePattern(pattern);
      PatternExpand(pattern);
      LOG4CPLUS_INFO (getApplicationLogger(),"Beam Type " << bt << "  Board Delay: "  << board_delay);


      // Set the individual modules
      if(board_settings_[cfi].bag.module_settings.size() != 8){
	throw(std::runtime_error("Incorrect Number of Modules in configuration"));
      }
      unsigned short LD_selection=0;
      for(int lasmod=0; lasmod < 8; lasmod++){

	int module_delay = board_delay + board_settings_[cfi].bag.module_settings[lasmod].bag.module_delay; // Add module delay
	// !!!!!!!!!!!!! Here we should check if module_delay is within a valid range
	//std::cout << "module_delay: " << module_delay << std::endl;
	//delay = std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE, (unsigned short)module_delay);

	LsBoardList[sci]->SetLsModThr(lasmod, board_settings_[cfi].bag.module_settings[lasmod].bag.threshold); // Set Module Thresholds
	LsBoardList[sci]->ClearLDT(lasmod); // Clear Trips
	LsBoardList[sci]->SetLsModBias(lasmod, board_settings_[cfi].bag.module_settings[lasmod].bag.bias); // Set bias values
	if(board_settings_[cfi].bag.module_settings[lasmod].bag.selected) LD_selection += (1 << lasmod);
	//lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_DELAY, delay);
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_DELAY, PatternConvertDelay(pattern, lasmod, (unsigned short)module_delay));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_WIDTH, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,  50));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_AMPLI, PatternConvertIntensity(pattern, lasmod));
      }
      LsBoardList[sci]->SetLaserOnSel(LD_selection);
    }
  }
  selected_board_index = selected_board_index_backup;
}





// Initialize the LAS Laser Board Acces
void LasLsBoardSupervisor::LsBoardInit(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
 
try{
    // Gather all input in one CGI object
    cgicc::Cgicc cgi(in);

    // Get the name of the file with the address table
    AddressTableFileName_ = cgi["LsBoardAddressTableFile"]->getValue();
    LasLsBoardAccess::Initialize(AddressTableFileName_);
    address_table_loaded = true;

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){

   LOG4CPLUS_INFO (getApplicationLogger(), "Laser Board could not be initialized. Check if it is On/Operating properly" );
   //SendSOAPmessage("failedLaserBoard",    "LasSupervisor"); // This function can throw an exception!!!!!!!!!!!!!!!!!!!!
   failurePage(in,out);
   //XCEPT_RAISE(xgi::exception::Exception, e.what());
 
  }	

}

// Arm all Boards xgi (Web) interface
void LasLsBoardSupervisor::ArmBoards(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    ArmBoards();
    this->Default(in,out);		
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

// Arm all Boards 
void LasLsBoardSupervisor::ArmBoards()
{
  for(unsigned short i=0; i < LsBoardList.size(); i++){
    LasLsBoardAccess* lsb=LsBoardList[i];
    lsb->RunModeOff();
    lsb->StartLdSeq();
    lsb->RunModeOn();
  }
  lasers_armed = true;
  is->fireItemValueChanged("LsBoardLasersArmed", this);

}

// Turn all Boards Off XGI Interface
void LasLsBoardSupervisor::TurnOffBoards(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    TurnOffBoards();
    this->Default(in,out);		
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

// Turn all Boards Off SOAP interface
xoap::MessageReference LasLsBoardSupervisor::TurnOffBoards(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"TurnOffBoards "); 
  TurnOffBoards();

  return SOAPReply("TurnOffBoardsOK");

}

// Turn all Boards Off
void LasLsBoardSupervisor::TurnOffBoards()
{
  for(unsigned short i=0; i < LsBoardList.size(); i++){
    LasLsBoardAccess* lsb=LsBoardList[i];
    lsb->RunModeOff();
  }
  lasers_armed = false;
  is->fireItemValueChanged("LsBoardLasersArmed", this);
}

// Select the Board 
void LasLsBoardSupervisor::SelectBoard(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    selected_board_index = cgi["boardselect"]->getIntegerValue();
    // Call again the Default callback
    this->Default(in,out);		
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}


// Issue a Board Command
void LasLsBoardSupervisor::Command(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input in one CGI object
    cgicc::Cgicc cgi(in);

    // Get the name of the Command
    LsBoard_cmd comm = LsBoard_cmd(cgi["Command"]->getIntegerValue());
    //std::cout << "Received Command: " << comm << std::endl;

    if(selected_board_index >=0){
      LasLsBoardAccess* lsb=LsBoardList[selected_board_index];
      switch(comm){
      case RUNMODE_ON:

        time_t rawtime2;
        time ( &rawtime2 );
        //std::cout << "RUNMODE_ON starting at " <<  ctime (&rawtime2)  << std::endl;  

	lsb->RunModeOn();
	break;
      case RUNMODE_OFF:
        //std::cout << "RUNMODE_OFF .. "  << std::endl;
        lsb->RunModeOff();
	break;
      case REGDUMP:
	lsb->Dump();
	break;
      case MANTRIGGER_ON:
	lsb->ManTriggerOn();
	break;
      case MANTRIGGER_OFF:
	lsb->ManTriggerOff();
	break;
      case STARTLDSEQ:
	lsb->StartLdSeq();
	break;
      case CLEAR_LDT:
	for(int i=0; i<8; i++)	lsb->ClearLDT(i);
	break;
      case THRESH_DEFAULT:
	SetBoards();
	break;
      case THRESH_SCAN:
	ScanThresh();
	break;
      case BIAS_DEFAULT:
	SetBoards();
	break;
      case BIAS_ZERO:
	for(int i=0; i<8; i++)	lsb->SetLsModBias(i, 0);
	break;
      case BIAS_SCAN:
	ScanBias();
	break;
      default:
	throw(std::runtime_error("Empty Command for this Button"));
	break;
      }
    }

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

// Scan for Threshold Values
void LasLsBoardSupervisor::ScanThresh()
{
  try{
    if(selected_board_index >=0){
      //std::cout << "Executing Threshold Scan" << std::endl;
      LOG4CPLUS_INFO (getApplicationLogger(),"Executing Threshold Scan");      
      LasLsBoardAccess* lsb=LsBoardList[selected_board_index];
      lsb->RunModeOff(); // Runmode has to be OFF to access Threshold registers
      for(int mod=0; mod < 8; mod++){ // Loop through all Modules
	std::cout << "Module: " << mod << std::endl;
	// Set Threshold to upper limit
	int thr=1000;
	lsb->SetLsModThr(mod, thr);
	lsb->ClearLDT(mod); // Clear trips
	//std::cout << "lsb->GetLaserDiodeTrip() & (1 << mod)" << (lsb->GetLaserDiodeTrip() & (1 << mod)) << std::endl;

	// Decrease Threshold until there is a trip
	while(!(lsb->GetLaserDiodeTrip() & (1 << mod)) && --thr > 0){
	  lsb->SetLsModThr(mod, thr);
	  //log4cplus::helpers::sleepmillis(2);
	  usleep(2000);
	}
	// Go 30% above the limit
	thr*=13;
	thr/=10;
	lsb->SetLsModThr(mod, thr);
	//std::cout << "Threshold set to: " << thr << std::endl;
        LOG4CPLUS_INFO (getApplicationLogger(),"Threshold set to: "<< thr); 
	lsb->ClearLDT(mod);
      }
    }
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Scan for Bias Values
void LasLsBoardSupervisor::ScanBias()
{
  try{
    if(selected_board_index >=0){
      //std::cout << "Executing Bias Scan" << std::endl;
      LOG4CPLUS_INFO (getApplicationLogger(), "Executing Bias Scan"); 
      LasLsBoardAccess* lsb=LsBoardList[selected_board_index];
      lsb->RunModeOff();
      //lsb->ClearDevice();      //Clear DACs
      for(int mod=0; mod < 8; mod++){ // Loop through all Modules
 ///	std::cout << "Module: " << mod << std::endl;
        LOG4CPLUS_INFO (getApplicationLogger(),  "Module: ");
// 	int bias=0;
// 	// Increase Bias until there is a trip
//  	while(!(lsb->GetLaserDiodeTrip() & (1 << mod)) && (bias+=10) < 4096){
//  	  lsb->SetLsModBias(mod, bias);
// 	  lsb->RunModeOn();
//  	  log4cplus::helpers::sleepmillis(20);
// 	  lsb->RunModeOff();
//  	}
//  	bias*=9;
//  	bias/=10;
//  	lsb->SetLsModBias(mod, bias);
//  	std::cout << "Bias set to: " << bias << std::endl;
// 	log4cplus::helpers::sleepmillis(10);
// 	lsb->ClearLDT(mod);
      }
    }
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Callback for Laser Module Settings
void LasLsBoardSupervisor::SetModules(xgi::Input * in=0, xgi::Output * out=0 ) throw (xgi::exception::Exception)
{
  try{
    // Retrieve the selected board
    if(selected_board_index < 0) return;
    LasLsBoardAccess* lsb = LsBoardList[selected_board_index];
    int cfi=ConfigIndexList[selected_board_index];
    //xdata::Bag<BoardSettings>&  lb_conf = board_settings_[cfi];
    lsb->RunModeOff();   // Runmode has to be OFF to access threshold and bias registers
    lsb->ClearDevice();      //Clear DACs
    
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);
    
    // Get theThreshold Choice
    thresh_choice = LsBoard_cmd(cgi["threshold"]->getIntegerValue());
    // Get the Bias Choice
    bias_choice = LsBoard_cmd(cgi["bias"]->getIntegerValue());
    // Get the Intensity Choice
    intensity_choice = LsBoard_cmd(cgi["intensity"]->getIntegerValue());

    // Evaluate the choices
    LOG4CPLUS_INFO (getApplicationLogger(),"Received Threshold choice: " << thresh_choice);    

    switch(thresh_choice){
    case THRESH_DEFAULT:
      LOG4CPLUS_INFO (getApplicationLogger(), " (Default)");
      if(cfi >= 0){
	for(int lasmod=0; lasmod < 8; lasmod++){
	  lsb->SetLsModThr(lasmod, board_settings_[cfi].bag.module_settings[lasmod].bag.threshold);
	  lsb->ClearLDT(lasmod);      //Clear Trip is necessary, because Threshold were set to 0 in the beginning
	}
      }
      break;
    case THRESH_SCAN:
      LOG4CPLUS_INFO (getApplicationLogger(), " (Scan)");
      ScanThresh();
      break;
    default:
      LOG4CPLUS_INFO (getApplicationLogger(), " (Unknown)");
    }

    LOG4CPLUS_INFO (getApplicationLogger(), "Received Bias choice: " << bias_choice);
    switch(bias_choice){
    case BIAS_DEFAULT:
      LOG4CPLUS_INFO (getApplicationLogger()," (Default)");
      if(cfi >= 0)
	for(int lasmod=0; lasmod < 8; lasmod++) 
	  lsb->SetLsModBias(lasmod, board_settings_[cfi].bag.module_settings[lasmod].bag.bias);
      break;
    case BIAS_ZERO:
      LOG4CPLUS_INFO (getApplicationLogger(),"  (All Zero)"); 
      for(int i=0; i<8; i++)	lsb->SetLsModBias(i, 0);
      break;
    case BIAS_SCAN:
      LOG4CPLUS_INFO (getApplicationLogger()," (Scan)");
      ScanBias();
      break;
    default:
      LOG4CPLUS_INFO (getApplicationLogger(), " (Unknown)");
    }
    
    LOG4CPLUS_INFO (getApplicationLogger(), "Received Intensity choice: " << intensity_choice); 
    RAMTable delay, width, ampli;
    switch(intensity_choice){
    case INTENSITY_ZERO:
      LOG4CPLUS_INFO (getApplicationLogger(), " (All Zero)");
      for(int lasmod=0; lasmod < 8; lasmod++){
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_DELAY, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,0));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_WIDTH, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,0));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_AMPLI, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,0));
      }
      break;
    case INTENSITY_1500:
      LOG4CPLUS_INFO (getApplicationLogger(), " (All 1500)");
      for(int lasmod=0; lasmod < 8; lasmod++){
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_DELAY, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,   0));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_WIDTH, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,  50));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_AMPLI, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,1500));
      }
      break;
    case INTENSITY_TECR4:
      {LOG4CPLUS_INFO (getApplicationLogger(), " (TECR4)");
      std::vector<unsigned short> pattern = PatternTECR4();
      PatternExpand(pattern);
      for(int lasmod=0; lasmod < 8; lasmod++){
	//int the_delay = board_settings_[cfi].bag.module_settings[lasmod].bag.intensity_values[idx];
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_DELAY, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,   0));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_WIDTH, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,  50));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_AMPLI, PatternConvertIntensity(pattern, lasmod));
      }}
      break;
    case INTENSITY_AT:
      {LOG4CPLUS_INFO (getApplicationLogger()," (AT)"); 
      std::vector<unsigned short> pattern = PatternAT();
      PatternExpand(pattern);
      for(int lasmod=0; lasmod < 8; lasmod++){
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_DELAY, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,   0));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_WIDTH, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,  50));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_AMPLI, PatternConvertIntensity(pattern, lasmod));
      }}
      break;
    case INTENSITY_TECR6:
      {LOG4CPLUS_INFO (getApplicationLogger(), " (TECR6)");
      std::vector<unsigned short> pattern = PatternTECR6();
      PatternExpand(pattern);
      for(int lasmod=0; lasmod < 8; lasmod++){
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_DELAY, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,   0));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_WIDTH, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,  50));
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_AMPLI, PatternConvertIntensity(pattern, lasmod));
      }}
      break;
    case INTENSITY_FILE:
      LOG4CPLUS_INFO (getApplicationLogger(), " (From File)");
      TableFileName=cgi["LsBoardInputFile"]->getValue();
      LoadFile(TableFileName, delay, width, ampli);
      for(int lasmod=0; lasmod < 8; lasmod++){
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_DELAY, delay[lasmod]);
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_WIDTH, width[lasmod]);
	lsb->LoadTable(lasmod, LasLsBoardAccess::RAM_AMPLI, ampli[lasmod]);
      }
      break;
    default:
      std::cerr << " (Unknown)" << std::endl;
    }

    if(in && out)this->Default(in,out);		
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }
}

void LasLsBoardSupervisor::LoadFile(const std::string& filename, RAMTable& delay, RAMTable& width, RAMTable& ampli)
{
  //////////////////////////////////////////////////////////////////
  //int lbModule,ram,addr;
  unsigned short lsmod, ram, addr, val;

  delay.resize(8, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,0));
  width.resize(8, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,0));
  ampli.resize(8, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,0));

  std::ifstream fin(filename.c_str());
  if(!fin)throw(std::runtime_error("Could not open file " + filename));

  while(fin >> lsmod >> ram >> addr >> val){
    if(lsmod >= 8 || ram >= 3 || addr >= LasLsBoardAccess::TABLE_SIZE){
      std::cerr << "Invalid line:    lsmod: " << lsmod << " ram: " << ram << " addr: " << addr << " val: " << val << std::endl;
    }
    else{
      switch(ram){
      case LasLsBoardAccess::RAM_DELAY:
	delay[lsmod][addr]=val;
	break;
      case LasLsBoardAccess::RAM_WIDTH:
	width[lsmod][addr]=val;
	break;
      case LasLsBoardAccess::RAM_AMPLI:
	ampli[lsmod][addr]=val;
	break;
      default:
	std::cerr << "Unknown RAM type " << ram << std::endl;
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  return;
}


// Create the AT pattern
std::vector<unsigned short> LasLsBoardSupervisor::PatternAT()
{
  std::vector<unsigned short> pattern;
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(1);
  pattern.push_back(2);
  pattern.push_back(3);
  pattern.push_back(4);
  pattern.push_back(5);
  return pattern;
}

// Create the TECR4 pattern
std::vector<unsigned short> LasLsBoardSupervisor::PatternTECR4()
{
  std::vector<unsigned short> pattern;
  pattern.push_back(1);
  pattern.push_back(2);
  pattern.push_back(3);
  pattern.push_back(4);
  pattern.push_back(5);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  return pattern;
}

// Create the TECR6 pattern
std::vector<unsigned short> LasLsBoardSupervisor::PatternTECR6()
{
  std::vector<unsigned short> pattern;
  pattern.push_back(1);
  pattern.push_back(2);
  pattern.push_back(3);
  pattern.push_back(4);
  pattern.push_back(5);
  return pattern;
}

// Duplicate every entry in the pattern
void LasLsBoardSupervisor::DoublePattern(std::vector<unsigned short>& pattern)
{
  std::vector<unsigned short>::size_type size = pattern.size();
  //std::cout << "Size: " << pattern.size() << " = " << size << std::endl;

  pattern.resize( 2 * size , 0);

  int ctr = 30;  
  for(std::vector<unsigned short>::size_type i = size; i > 0; i--){
    //std::cout << "i: " << i << "   2*i-1: " << (2*i-1) << "   2*i-2: " << (2*i-2) <<std::endl;
    pattern[2*i-1] = pattern[i-1];
    pattern[2*i-2]     = pattern[i-1];
    if(--ctr == 0)break;
  }
}


// Expand a pattern to 128 values
void LasLsBoardSupervisor::PatternExpand(std::vector<unsigned short>& pattern)
{
  if(pattern.empty())throw std::runtime_error("LasLsBoardSupervisor::PatternExpand: Cannot expand empty pattern");
  std::vector<unsigned short> buff=pattern;
  // Repeat base pattern, until size >= 100 is reached
  while(pattern.size() < 100){
    pattern.insert(pattern.begin(), buff.begin(), buff.end());
  }
  pattern.resize(100,0); // Truncate to 100 entries
  pattern.resize(128,0); // Add 28 empty entries
}

// Convert the pattern to intensity values (a table that can be loaded)
std::vector<unsigned short> LasLsBoardSupervisor::PatternConvertIntensity(std::vector<unsigned short>& pattern, int lasmod)
{
  if(selected_board_index < 0) throw std::runtime_error("LasLsBoardSupervisor::PatternConvertIntensity: No valid board in Convert Pattern");
  int cfi=ConfigIndexList[selected_board_index];
  if(cfi < 0) throw std::runtime_error("LasLsBoardSupervisor::PatternConvertIntensity: No valid configuration in Convert Pattern");

  std::vector<unsigned short> conv_patt;

  for(std::vector<unsigned short>::size_type i=0; i<pattern.size(); i++){
    unsigned short idx=pattern[i];
    if(idx >= board_settings_[cfi].bag.module_settings[lasmod].bag.intensity_values.size())conv_patt.push_back(0);
    else conv_patt.push_back(board_settings_[cfi].bag.module_settings[lasmod].bag.intensity_values[idx]);
  }
  return conv_patt;
}


// Convert the pattern to delay values (a table that can be loaded)
std::vector<unsigned short> LasLsBoardSupervisor::PatternConvertDelay(std::vector<unsigned short>& pattern, int lasmod, unsigned short offset)
{
  if(selected_board_index < 0) throw std::runtime_error("LasLsBoardSupervisor::PatternConvertDelay: No valid board in Convert Pattern");
  int cfi=ConfigIndexList[selected_board_index];
  if(cfi < 0) throw std::runtime_error("LasLsBoardSupervisor::PatternConvertDelay: No valid configuration in Convert Pattern");

  std::vector<unsigned short> conv_patt;

  for(std::vector<unsigned short>::size_type i=0; i<pattern.size(); i++){
    unsigned short idx=pattern[i];
    if(idx >= board_settings_[cfi].bag.module_settings[lasmod].bag.sensor_delays.size())
      conv_patt.push_back(0);
    else 
      conv_patt.push_back(board_settings_[cfi].bag.module_settings[lasmod].bag.sensor_delays[idx] + offset);
  }
  return conv_patt;
}


//! Convert the level to a intensity setting
unsigned short LasLsBoardSupervisor::LevelConvert(int boardid, int lasmod, unsigned short level)
{
  int board_idx = GetBoardIdx(boardid);
  if(board_idx < 0) throw std::runtime_error("LasLsBoardSupervisor::LevelConvert: No valid board id");
  int cfi=ConfigIndexList[board_idx];
  if(cfi < 0) throw std::runtime_error("LasLsBoardSupervisor::PatternConvertIntensity: No valid configuration in LevelConvert");

  if(level >= board_settings_[cfi].bag.module_settings[lasmod].bag.intensity_values.size())return 0;
  return board_settings_[cfi].bag.module_settings[lasmod].bag.intensity_values[level];
}

int LasLsBoardSupervisor::GetBoardIdx(int board_id)
{
  for( std::vector<xdata::Integer>::size_type i = 0; i < BoardNrList.size(); i++)
    if (BoardNrList[i] == board_id) return i;

  return -1;
}


//*****//
//SOAP //
//*****//

//! Arm all Boards SOAP interface //
xoap::MessageReference LasLsBoardSupervisor::ArmBoards (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"ArmBoards"); 

  try{ ArmBoards(); }
  catch (const std::exception & e){ XCEPT_RAISE(xoap::exception::Exception, e.what()); }

  return SOAPReply("ArmBoardsOK");
}



//! Set Interactive Mode On //
xoap::MessageReference LasLsBoardSupervisor::SetInteractiveOn (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"SetInteractiveOn"); 
  expert_mode = true;
  return SOAPReply("SetInteractiveOnOK");
}



//! Set Interactive Mode Off //
xoap::MessageReference LasLsBoardSupervisor::SetInteractiveOff (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"SetInteractiveOff"); 
  expert_mode = false;
  return SOAPReply("SetInteractiveOffOK");
}


//! Switch all diodes of a board on or off
xoap::MessageReference LasLsBoardSupervisor::SwitchDiodes (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  // Retrieve State that is to be set
  std::string state = GetSoapAttribute(msg, "state");
  uint32_t diode_sel = (state == "on" ? 0xFF : 0x0);

  // Retrieve BoardId
  std::istringstream thestream( GetSoapAttribute(msg, "boardid") ); 
  int board_id;
  thestream >> board_id;

  LOG4CPLUS_INFO (getApplicationLogger(),"SwitchDiodes " << state << " for Board "  << board_id);

  try{
    LasLsBoardAccess* lsb = 0;
    for( std::vector<xdata::Integer>::size_type i = 0; i < BoardNrList.size(); i++){
      if (BoardNrList[i] == board_id){
	lsb = LsBoardList[i];
      }
    }
    if(!lsb){
      LOG4CPLUS_ERROR (getApplicationLogger(),"Error in SwitchDiodes: could not find board");
      return  SOAPReply("SwitchDiodesFailed");
    }
    lsb->SetLaserOnSel(diode_sel);
  }
  catch (const std::exception & e){ XCEPT_RAISE(xoap::exception::Exception, e.what()); }

  return SOAPReply("SwitchDiodesOK");
}


//! Settings for the Laser Board Scan
xoap::MessageReference LasLsBoardSupervisor::ScanSettings (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  // Retrieve BoardId
  std::istringstream boardstream( GetSoapAttribute(msg, "boardid") ); 
  int board_id;
  boardstream >> board_id;

  // Retrieve Level
  std::istringstream levelstream( GetSoapAttribute(msg, "level") ); 
  int level;
  levelstream >> level;

  // Retrieve first delay
  std::istringstream delay1stream( GetSoapAttribute(msg, "delay_first") ); 
  int delay_first;
  delay1stream >> delay_first;

  // Retrieve last delay
  std::istringstream delay2stream( GetSoapAttribute(msg, "delay_last") ); 
  int delay_last;
  delay2stream >> delay_last;

  LOG4CPLUS_INFO (getApplicationLogger(),"ScanSettings " << " for Board "  << board_id << "  level:" << level << "  delay_first:" << delay_first << "  delay_last:" << delay_last);

  int board_idx = -1;
  for( std::vector<xdata::Integer>::size_type i = 0; i < BoardNrList.size(); i++){
    if (BoardNrList[i] == board_id){
      board_idx = i;
      break;
    }
  }

  if(board_idx < 0){
    LOG4CPLUS_ERROR (getApplicationLogger(),"Error in ScanSettings: could not find board");
    return  SOAPReply("ScanSettingsFailed");
  }

  LasLsBoardAccess* lsb = LsBoardList[board_idx];

  std::vector<unsigned short> intensity(LasLsBoardAccess::TABLE_SIZE,   level);
  PatternExpand(intensity);

  std::vector<unsigned short> delay(LasLsBoardAccess::TABLE_SIZE,   0);
  for(std::vector<unsigned short>::size_type i=0; i < 100; i++) delay[i] = (unsigned short)(delay_first + (float)(delay_last-delay_first)*i/99);
  PatternExpand(delay);

  int selected_board_index_backup = selected_board_index;
  selected_board_index = board_idx;
  for(int lasmod=0; lasmod < 8; lasmod++){
    lsb->LoadTable( lasmod, LasLsBoardAccess::RAM_DELAY, delay );
    lsb->LoadTable( lasmod, LasLsBoardAccess::RAM_WIDTH, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,  50) );
    lsb->LoadTable( lasmod, LasLsBoardAccess::RAM_AMPLI, PatternConvertIntensity(intensity, lasmod) );
  }
  selected_board_index = selected_board_index_backup;

  lsb->SetLaserOnSel(0XFF);


  return SOAPReply("ScanSettingsOK");
}


// Set a board to counter mode
xoap::MessageReference LasLsBoardSupervisor::SetToCounter (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  // Retrieve BoardId
  std::istringstream boardstream( GetSoapAttribute(msg, "boardid") ); 
  int board_id;
  boardstream >> board_id;

  // Retrieve Level
  std::istringstream levelstream( GetSoapAttribute(msg, "level") ); 
  int level;
  levelstream >> level;

  // Retrieve Delay
  std::istringstream delaystream( GetSoapAttribute(msg, "delay") ); 
  int delay;
  delaystream >> delay;

  LOG4CPLUS_INFO (getApplicationLogger(),"SetToCounter " << " for Board "  << board_id << "  level:" << level<< "  delay:" << delay);

  int board_idx = -1;
  for( std::vector<xdata::Integer>::size_type i = 0; i < BoardNrList.size(); i++){
    if (BoardNrList[i] == board_id){
      board_idx = i;
      break;
    }
  }

  if(board_idx < 0){
    LOG4CPLUS_ERROR (getApplicationLogger(),"Error in SetToCounter: could not find board");
    return  SOAPReply("SetToCounterFailed");
  }

  LasLsBoardAccess* lsb = LsBoardList[board_idx];

  int selected_board_index_backup = selected_board_index;
  selected_board_index = board_idx;
  for(int lasmod=0; lasmod < 8; lasmod++){
    lsb->LoadTable( lasmod, LasLsBoardAccess::RAM_DELAY, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,  delay) );
    lsb->LoadTable( lasmod, LasLsBoardAccess::RAM_WIDTH, std::vector<unsigned short>(LasLsBoardAccess::TABLE_SIZE,  50) );
    std::vector<unsigned short> intensity(LasLsBoardAccess::TABLE_SIZE,   0);
    for(std::vector<unsigned short>::size_type i=0; i < 100; i++) intensity[i] = (unsigned short)(((i+1)>>lasmod)%2)*level;
    PatternExpand(intensity);
    lsb->LoadTable( lasmod, LasLsBoardAccess::RAM_AMPLI, PatternConvertIntensity(intensity, lasmod) );
  }
  selected_board_index = selected_board_index_backup;

  lsb->SetLaserOnSel(0XFF);

  return SOAPReply("SetToCounterOK");
}


xoap::MessageReference LasLsBoardSupervisor::SetBoards (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"SetBoards"); 

  try{ SetBoards(); }
  catch (const std::exception & e){ XCEPT_RAISE(xoap::exception::Exception, e.what()); }

  return SOAPReply("SetBoardsOK");
}


xoap::MessageReference LasLsBoardSupervisor::SetRunMode (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  // Retrieve RunMode that is to be set
  std::string run_mode = GetSoapAttribute(msg, "runmode");

  // Retrieve BoardId
  std::istringstream thestream( GetSoapAttribute(msg, "boardid") ); 
  int board_id;
  thestream >> board_id;

  LOG4CPLUS_INFO (getApplicationLogger(),"SetRunMode " << run_mode << " for Board "  << board_id);
  // Do some action here

  return SOAPReply("SetRunModeOK");
}


xoap::MessageReference LasLsBoardSupervisor::SetRunModeOn (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{

   
        LOG4CPLUS_INFO (getApplicationLogger(),"SetRunModeOn ");

        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());

                           // Turn Test Mode On
                        //lasTgBoardAccess.TestModeOn();
  
                       if (selected_board_index!=-1)
                        {
                          LasLsBoardAccess* lsb = LsBoardList[selected_board_index];// Pointer to selected Laser Board
                          lsb->RunModeOn();
             
                        }

                        //LasLsBoardAccess* lsb = LsBoardList[selected_board_index];// Pointer to selected Laser Board
                        //lsb->RunModeOn();

                       // if(runmodeon && !runmodeen)lsb->RunModeOff();
                       //runmodeon = (lsb->GetGenFlags() & 0x08);
 
                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

 
    XCEPT_RAISE(xcept::Exception,"command not found");



}


xoap::MessageReference LasLsBoardSupervisor::SetRunModeOff (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{


   LOG4CPLUS_INFO (getApplicationLogger(),"SetRunModeOff ");
//  std::cout << "Inside LasLsBoardSupervisor::SetRunModeOff " <<  std::endl ;
        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());

        
                       if (selected_board_index!=-1)
                       {
                         LasLsBoardAccess* lsb = LsBoardList[selected_board_index];// Pointer to selected Laser Board
                         lsb->RunModeOff();
                       }
 
                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

 
       XCEPT_RAISE(xcept::Exception,"command not found");



}

xoap::MessageReference LasLsBoardSupervisor::SetStartLoadSequence (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{

 
  LOG4CPLUS_INFO (getApplicationLogger(),"StartLoadSequence ");

        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());

                       if (selected_board_index!=-1)
                       {
                        LasLsBoardAccess* lsb = LsBoardList[selected_board_index];// Pointer to selected Laser Board
                        lsb->StartLdSeq();
                       }

                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

 
    XCEPT_RAISE(xcept::Exception,"command not found");



}


xoap::MessageReference LasLsBoardSupervisor::SetModuleSettings (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{

  LOG4CPLUS_INFO (getApplicationLogger(),"board_index"<< selected_board_index);
  //std::cout << "LasLsBoardSupervisor::SetModuleSettings " <<  std::endl ;

        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                       
                        if (selected_board_index!=-1)
                        {
                          SetModules();
                        }

                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }



   XCEPT_RAISE(xcept::Exception,"command not found");



}


// SelectBoard SOAP interface
xoap::MessageReference LasLsBoardSupervisor::SelectBoard( xoap::MessageReference msg )
        throw (xoap::exception::Exception)
{
  try{
    // Get the body of the message
    xoap::SOAPBody body = msg->getSOAPPart().getEnvelope().getBody();
    
    // Get the child with the relevant information
    xoap::SOAPName name("SelectBoard","xdaq", XDAQ_NS_URI );
    std::vector< xoap::SOAPElement > chel = body.getChildElements(name);
    xoap::SOAPElement& el = chel[0];
    
    // Get the Attribute "value" and convert it to an integer
    xoap::SOAPName attr("value","","");
    std::string val = el.getAttributeValue(attr);
    int bnr = convertTo<int>(val);

    // Try to find this Board Nr in the list of available Boards
    for( std::vector<xdata::Integer>::size_type i = 0; i < BoardNrList.size(); i++){
      if(bnr == BoardNrList[i]){
	selected_board_index = i;
	LOG4CPLUS_INFO (getApplicationLogger(), "SelectBoard " << val ); 
	return SOAPReply("SelectBoardOK");
      }
    }

    // If the Board Nr. was not found do nothing
    LOG4CPLUS_INFO (getApplicationLogger(), "SelectBoard: Could not find Board Number " << val ); 
    return SOAPReply("SelectBoardFailed");
  }
  catch(BadConversion& e){
    LOG4CPLUS_INFO (getApplicationLogger(),
		    "SelectBoard: Could not convert " << e.what() << " to a Board Number"
		    ); 
    return SOAPReply("SelectBoardFailed");
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

xoap::MessageReference LasLsBoardSupervisor::SetThresholdDefault (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{

  LOG4CPLUS_INFO (getApplicationLogger(),"SetThresholdDefault ");
 
        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        thresh_choice = THRESH_DEFAULT;                           
 
                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}



xoap::MessageReference LasLsBoardSupervisor::SetThresholdScan (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
   LOG4CPLUS_INFO (getApplicationLogger(),"SetThresholdScan ");

        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        thresh_choice = THRESH_SCAN; 

                        //std::cout << "selected_board_index : " << selected_board_index  << std::endl;
                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}


xoap::MessageReference LasLsBoardSupervisor::SetBiasDefault (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
   LOG4CPLUS_INFO (getApplicationLogger(),"SetBiasDefault ");

        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        bias_choice = BIAS_DEFAULT;  

                        //std::cout << "selected_board_index : " << selected_board_index  << std::endl;
                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}


xoap::MessageReference LasLsBoardSupervisor::SetBiasAllZero (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
    LOG4CPLUS_INFO (getApplicationLogger(),"SetBiasAllZero ");

        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        bias_choice = BIAS_ZERO;
  
                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}

xoap::MessageReference LasLsBoardSupervisor::SetBiasScan (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
     LOG4CPLUS_INFO (getApplicationLogger(),"SetBiasScan ");
        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        bias_choice = BIAS_SCAN;
                          
                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}

xoap::MessageReference LasLsBoardSupervisor::SetRAMIntensityZero (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
   LOG4CPLUS_INFO (getApplicationLogger(),"SetRAMIntensityZero ");
        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        intensity_choice = INTENSITY_ZERO;  
                        
                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}

xoap::MessageReference LasLsBoardSupervisor::SetRAMRing4 (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
   LOG4CPLUS_INFO (getApplicationLogger(),"SetRAMRing4 ");
        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        intensity_choice = INTENSITY_TECR4;  

                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}

xoap::MessageReference LasLsBoardSupervisor::SetRAMRing6 (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
         LOG4CPLUS_INFO (getApplicationLogger(),"SetRAMRing6 ");
        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        intensity_choice = INTENSITY_TECR6;

                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}

xoap::MessageReference LasLsBoardSupervisor::SetRAMAlignmentTubes (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
    LOG4CPLUS_INFO (getApplicationLogger(),"SetRAMAlignmentTubes ");
        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        intensity_choice = INTENSITY_AT;

                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}

xoap::MessageReference LasLsBoardSupervisor::SetIntensity1500 (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
       LOG4CPLUS_INFO (getApplicationLogger(),"SetIntensity1500 "); 

        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        intensity_choice = INTENSITY_1500;

                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}

xoap::MessageReference LasLsBoardSupervisor::SetRAMCustom (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
        LOG4CPLUS_INFO (getApplicationLogger(),"SetRAMCustom ");
        xoap::SOAPPart part = msg->getSOAPPart();
        xoap::SOAPEnvelope env = part.getEnvelope();
        xoap::SOAPBody body = env.getBody();
        DOMNode* node = body.getDOMNode();
        DOMNodeList* bodyList = node->getChildNodes();
        for (unsigned int i = 0; i < bodyList->getLength(); i++)
        {
                DOMNode* command = bodyList->item(i);

                if (command->getNodeType() == DOMNode::ELEMENT_NODE)
                {

                        std::string commandName = xoap::XMLCh2String (command->getLocalName());
                        intensity_choice = INTENSITY_FILE; 

                        xoap::MessageReference reply = xoap::createMessage();
                        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
                        xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
                        // xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
                        (void) envelope.getBody().addBodyElement ( responseName );
                        return reply;
                }
        }

        XCEPT_RAISE(xcept::Exception,"command not found");

}


////////////////////////////////////////////
// Implementation of ModuleSettings Class //
////////////////////////////////////////////

//! Create the AT pattern
std::vector<unsigned short> ModuleSettings::PatternAT()
{
  std::vector<unsigned short> pattern;
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(1);
  pattern.push_back(2);
  pattern.push_back(3);
  pattern.push_back(4);
  pattern.push_back(5);
  return pattern;
}

//! Create the TECR4 pattern
std::vector<unsigned short> ModuleSettings::PatternTECR4()
{
  std::vector<unsigned short> pattern;
  pattern.push_back(1);
  pattern.push_back(2);
  pattern.push_back(3);
  pattern.push_back(4);
  pattern.push_back(5);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  pattern.push_back(0);
  return pattern;
}

//! Create the TECR6 pattern
std::vector<unsigned short> ModuleSettings::PatternTECR6()
{
  std::vector<unsigned short> pattern;
  pattern.push_back(1);
  pattern.push_back(2);
  pattern.push_back(3);
  pattern.push_back(4);
  pattern.push_back(5);
  return pattern;
}

//! Expand a pattern to 128 values (last 28 values are always 0)
void ModuleSettings::PatternExpand(std::vector<unsigned short>& pattern)
{
  std::vector<unsigned short> buff=pattern;
  // Repeat base pattern, until size >= 100 is reached
  while(pattern.size() < 100){
    pattern.insert(pattern.begin(), buff.begin(), buff.end());
  }
  pattern.resize(100,0); // Truncate to 100 entries
  pattern.resize(128,0); // Add 28 empty entries
}

//! Convert the pattern to intensity values (a table that can be loaded)
std::vector<unsigned short> ModuleSettings::PatternConvertIntensity(std::vector<unsigned short>& pattern)
{
  std::vector<unsigned short> conv_patt;

  for(std::vector<unsigned short>::size_type i=0; i<pattern.size(); i++){
    unsigned short idx=pattern[i];
    if(idx >= intensity_values.size())conv_patt.push_back(0);
    conv_patt.push_back(intensity_values[idx]);
  }
  return conv_patt;
}


// BackDoor SOAP interface (for testing purposes)
xoap::MessageReference LasLsBoardSupervisor::BackDoor (xoap::MessageReference msg)
        throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"BackDoor "); 

  // Toggle the expert mode (this is a preliminary workaround, bacdoor should allow more complex behaviour
  expert_mode = !expert_mode;

  return SOAPReply("Backdoor");
}

// Callback for Timer Listener
void LasLsBoardSupervisor::timeExpired(toolbox::task::TimerEvent& e)
{
  //LOG4CPLUS_INFO (getApplicationLogger(), "timeExpired was called ");
  UpdateBoards();
}
