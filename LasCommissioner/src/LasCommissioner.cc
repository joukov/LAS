#include "LasCommissioner.h"

#include <xdata/InfoSpaceFactory.h>

#include "xdaq/NamespaceURI.h"

#include "xgi/Method.h"
#include "cgicc/HTMLClasses.h"

#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"

#include "LasCommon.h"
using namespace LaserAlignment;

XDAQ_INSTANTIATOR_IMPL(LasCommissioner);


LasCommissioner::LasCommissioner(xdaq::ApplicationStub * s) : 
  xdaq::Application(s),
  automatic_refresh(false),
  tgboarddelay_available(false),
  lsboardlasersarmed_available(false),
  waiting_for_lsboard_completion(false),
  running_tgboard_delay_scan(false),
  tgboard_delay_first(30),
  tgboard_delay_last(46),
  tgboard_delay_current(0),
  tgboard_level(43),
  running_lsboard_scan(false),
  lsboard_tgboard_delay(28),
  lsboard_delay_first(0),
  lsboard_delay_last(99),
  lsboard_level_first(1),
  lsboard_level_last(67),
  lsboard_level_current(0),
  lsboard_id_first(1),
  lsboard_id_last(5),
  lsboard_id_current(0),
  lsboard_level_ctr(45),
  lsboard_delay_ctr(50),
  scan_table(std::vector<std::vector<int> >(5,std::vector<int>(5,OFF)))
{

  ////////////////////////
  // Bind XGI callbacks //
  ////////////////////////
  xgi::bind(this,&LasCommissioner::Default, "Default");  
  xgi::bind(this,&LasCommissioner::StartTriggerBoardDelayScan, "StartTriggerBoardDelayScan");
  xgi::bind(this,&LasCommissioner::StopTriggerBoardDelayScan, "StopTriggerBoardDelayScan");
  xgi::bind(this,&LasCommissioner::StartLaserBoardScan, "StartLaserBoardScan");
  xgi::bind(this,&LasCommissioner::StopLaserBoardScan, "StopLaserBoardScan");

  // Initialize the pointer to the InfoSpace and start listening to the publishing of Items

  try{
    is = xdata::InfoSpaceFactory::getInstance()->get("TrackerLaserAlignmentInfoSpace");
  }
  catch(xdata::exception::Exception){
    is = xdata::InfoSpaceFactory::getInstance()->create("TrackerLaserAlignmentInfoSpace");
  }

  //is = xdata::InfoSpaceFactory::getInstance()->get("TrackerLaserAlignmentInfoSpace");
  is->addItemAvailableListener(this);

  // Initialize Scan Table for Laser Board Scan
  // This must become better (put it into configuration or get it from web interface)
  scan_table[0][0] = SCAN;
  scan_table[0][1] = CTR;
  scan_table[0][2] = OFF;
  scan_table[0][3] = CTR;
  scan_table[0][4] = CTR;

  scan_table[1][0] = CTR;
  scan_table[1][1] = SCAN;
  scan_table[1][2] = OFF;
  scan_table[1][3] = CTR;
  scan_table[1][4] = CTR;

  scan_table[2][0] = OFF;
  scan_table[2][1] = CTR;
  scan_table[2][2] = SCAN;
  scan_table[2][3] = OFF;
  scan_table[2][4] = CTR;

  scan_table[3][0] = CTR;
  scan_table[3][1] = CTR;
  scan_table[3][2] = OFF;
  scan_table[3][3] = SCAN;
  scan_table[3][4] = CTR;

  scan_table[4][0] = CTR;
  scan_table[4][1] = CTR;
  scan_table[4][2] = OFF;
  scan_table[4][3] = CTR;
  scan_table[4][4] = SCAN;
}



// Default Callback of XDAQ Application
void LasCommissioner::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  // HTML header stuff
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("LAS Commissioner") << std::endl;

  *out << cgicc::h1("LAS Commissioner") << std::endl;

  if(automatic_refresh){
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

  *out << cgicc::hr() << std::endl;

  // Form for Start/Stop of Trigger Board Delay Run
  std::string button_method = toolbox::toString(
						running_tgboard_delay_scan ? "/%s/StopTriggerBoardDelayScan" : "/%s/StartTriggerBoardDelayScan"
						, getApplicationDescriptor()->getURN().c_str()
						);
  std::string disabled_at_tgdelscan(running_tgboard_delay_scan ? "disabled" : "");
  std::string disabled_at_lsbrdscan(running_lsboard_scan ? "disabled" : "");

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << cgicc::legend("Trigger Board Delay Scan") << cgicc::p() << std::endl;
  *out << "<table border=0 cellpadding=0 cellspacing=0>"; // Table for TgBoard Delay Scan
  *out << "<tr>";
  *out << cgicc::form().set("method","GET").set("action", button_method);

  *out << cgicc::input().set("type","submit").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("value", running_tgboard_delay_scan ? "Stop Run" : "Start Run");
  *out << "Start Delay ";
  *out << cgicc::input().set("type","text").set(disabled_at_tgdelscan,disabled_at_tgdelscan).set("name","Delay_start").set("value", tgboard_delay_first.toString());
  *out << "End Delay ";
  *out << cgicc::input().set("type","text").set(disabled_at_tgdelscan,disabled_at_tgdelscan).set("name","Delay_end").set("value", tgboard_delay_last.toString());

  *out << cgicc::form();
  if(running_tgboard_delay_scan) *out << "Current Delay: " << tgboard_delay_current.toString();
  *out << "</tr><tr>";
  *out << "Laser Board Level ";
  *out << cgicc::input().set("type","text").set(disabled_at_tgdelscan,disabled_at_tgdelscan).set("name","Level").set("value", tgboard_level.toString());
  *out << "</tr>";
  *out << "</table>";
  *out << cgicc::fieldset();

  // Form for Start/Stop of Laser Board Scan
  std::string button_method_2 = toolbox::toString(
						running_lsboard_scan ? "/%s/StopLaserBoardScan" : "/%s/StartLaserBoardScan"
						, getApplicationDescriptor()->getURN().c_str()
						);
  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << cgicc::legend("Laser Board Scan") << cgicc::p();
  *out << cgicc::form().set("method","GET").set("action", button_method_2);

  *out << "<table border=0 cellpadding=0 cellspacing=20>"; // Table for LsBoard Scan
  *out << "<tr><td>";
  *out << cgicc::input().set("type","submit").set(disabled_at_tgdelscan,disabled_at_tgdelscan).set("value", running_lsboard_scan ? "Stop Run" : "Start Run");
  *out << "</td><td>";

  *out << "<table border=0 cellpadding=0 cellspacing=10>"; // Table for LsBoard Scan
  *out << "<tr>";
  *out << "<td></td>";
  *out << "<td>Start</td>";
  *out << "<td>End</td>";
  if(running_lsboard_scan)
    *out << "<td>Current</td>";
  *out << "</tr><tr>";
  *out << "<td>Board</td>";
  *out << "<td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Id_first").set("value", lsboard_id_first.toString()) << "</td>";
  *out << "<td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Id_last").set("value", lsboard_id_last.toString()) << "</td>";
  if(running_lsboard_scan)
    *out << "<td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Id_current").set("value", lsboard_id_current.toString()) << "</td>";

  *out << "</tr><tr>";
  *out << "<td>Level</td>";
  *out << "<td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Level_first").set("value", lsboard_level_first.toString()) << "</td>";
  *out << "<td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Level_last").set("value", lsboard_level_last.toString()) << "</td>";
  if(running_lsboard_scan)
    *out << "<td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Level_current").set("value", lsboard_level_current.toString()) << "</td>";

  *out << "</tr><tr>";
  *out << "<td>Delay</td>";
  *out << "<td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Delay_start").set("value", lsboard_delay_first.toString()) << "</td>";
  *out << "<td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Delay_end").set("value", lsboard_delay_last.toString()) << "</td>";
  if(running_lsboard_scan)
    *out << "<td></td>";
  *out << "</table>";
  *out << "</td>";
  *out << "<td>";

  *out << "<table border=0 cellpadding=0 cellspacing=10>"; // Table for LsBoard Scan
  *out << "<tr>";
  *out << "<td> Trigger Board Delay </td><td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","TgBoard_Delay").set("value", lsboard_tgboard_delay.toString()) << "</td>";
  *out << "</tr>";
  *out << "<tr>";
  *out << "<td> Level for Counter </td><td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Level_ctr").set("value", lsboard_level_ctr.toString()) << "</td>";
  *out << "</tr>";
  *out << "<tr>";
  *out << "<td> Delay for Counter </td><td>" << cgicc::input().set("type","text").set(disabled_at_lsbrdscan,disabled_at_lsbrdscan).set("name","Delay_ctr").set("value", lsboard_delay_ctr.toString()) << "</td>";
  *out << "</tr>";
  *out << "</table>";

  *out << "</td>";
  *out << "</tr>";
  *out << "</table>";
  *out << cgicc::form();
  *out << cgicc::fieldset();


  *out << cgicc::form().set("method","GET").set("action", toolbox::toString("/%s/Default", getApplicationDescriptor()->getURN().c_str()));
  *out << cgicc::input().set("type","submit").set("value", "Refresh");
  *out << cgicc::form();
}


// Start selected TriggerBoardDelayScan
void LasCommissioner::StartTriggerBoardDelayScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Get the Values from the form
    tgboard_delay_first = cgi["Delay_start"]->getIntegerValue();
    tgboard_delay_last = cgi["Delay_end"]->getIntegerValue();
    //tgboard_level = cgi["Level"]->getIntegerValue();

    StartTgBoardDelayRun();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
}

// Stop the ongoing TriggerBoardDelayScan
void LasCommissioner::StopTriggerBoardDelayScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Reset all Flags
    running_tgboard_delay_scan = false;
    automatic_refresh = false;
    waiting_for_lsboard_completion = false;
    // Turn off boards, and switch Supervisors back to interactive mode
    SendSoapMessage ("LasLsBoardSupervisor", "TurnOffBoards", *this);
    SendSoapMessage ("LasTgBoardSupervisor", "SetInteractiveOn", *this);
    SendSoapMessage ("LasLsBoardSupervisor", "SetInteractiveOn", *this);

    LOG4CPLUS_INFO (getApplicationLogger(), "Warning: Trigger Board Delay Scan was stopped manually!!! ");

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}


// Start Laser Board Scan
void LasCommissioner::StartLaserBoardScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Retrieve values from the form 
    lsboard_delay_first = cgi["Delay_start"]->getIntegerValue();
    lsboard_delay_last = cgi["Delay_end"]->getIntegerValue();

    lsboard_id_first = cgi["Id_first"]->getIntegerValue();
    lsboard_id_last = cgi["Id_last"]->getIntegerValue();

    lsboard_level_first = cgi["Level_first"]->getIntegerValue();
    lsboard_level_last = cgi["Level_last"]->getIntegerValue();

    lsboard_level_ctr = cgi["Level_ctr"]->getIntegerValue();
    lsboard_delay_ctr = cgi["Delay_ctr"]->getIntegerValue();

    lsboard_tgboard_delay = cgi["TgBoard_Delay"]->getIntegerValue();

    // Start the Scan
    StartLsBoardRun();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
}

// Stop the ongoing Laser Board Scan
void LasCommissioner::StopLaserBoardScan(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{

    LOG4CPLUS_INFO (getApplicationLogger(), "Warning: Laser Board Scan was stopped manually!!! ");
    FinishLsBoardRun();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}


//! Switch Alldiodes on or off for a specific board
void LasCommissioner::SwitchDiodes(int board_id, bool state)
{
  attribute_list att_list;
  att_list.push_back(std::pair<std::string, std::string>( "state", (state ? "on" : "off") ));
  att_list.push_back(std::pair<std::string, std::string>( "boardid",stringify(board_id) ));
  SendSoapMessage("LasLsBoardSupervisor", "SwitchDiodes", att_list, *this);
}


//! Send settings for Laser Board Scan to a specific board
void LasCommissioner::ScanSettings(int boardid, int level, int delay_first, int delay_last)
{
  attribute_list att_list;
  att_list.push_back(std::pair<std::string, std::string>( "boardid",stringify(boardid) ));
  att_list.push_back(std::pair<std::string, std::string>( "level", stringify(level) ));
  att_list.push_back(std::pair<std::string, std::string>( "delay_first", stringify(delay_first) ));
  att_list.push_back(std::pair<std::string, std::string>( "delay_last", stringify(delay_last) ));
  SendSoapMessage("LasLsBoardSupervisor", "ScanSettings", att_list, *this);
}


//! Set this board to counting mode
void LasCommissioner::SetToCounter(int boardid, int level, int delay)
{
  attribute_list att_list;
  att_list.push_back(std::pair<std::string, std::string>( "boardid",stringify(boardid) ));
  att_list.push_back(std::pair<std::string, std::string>( "level", stringify(level) ));
  att_list.push_back(std::pair<std::string, std::string>( "delay", stringify(delay) ));
  SendSoapMessage("LasLsBoardSupervisor", "SetToCounter", att_list, *this);
}

//! Callback for Action Listener
void LasCommissioner::actionPerformed (xdata::Event& received) 
{
  //std::cout << "received.type: " << received.type() << std::endl;

  if ( received.type() == "ItemAvailableEvent" ){
    xdata::ItemEvent& e = dynamic_cast<xdata::ItemEvent&>(received);
    if( e.itemName() == "TgBoardDelay"){
      LOG4CPLUS_INFO (getApplicationLogger(),  "Adding ItemChangedListener for TgBoardDelay");
      tgboarddelay_available = true;
      is->addItemChangedListener("TgBoardDelay", this);
    }
    if( e.itemName() == "LsBoardLasersArmed"){
      LOG4CPLUS_INFO (getApplicationLogger(),  "Adding ItemChangedListener for LsBoardLasersArmed");
      lsboardlasersarmed_available = true;
      is->addItemChangedListener("LsBoardLasersArmed", this);
    }
  }
  if ( received.type() == "ItemChangedEvent" ){
    xdata::ItemEvent& e = dynamic_cast<xdata::ItemEvent&>(received);
    if( e.itemName() == "LsBoardLasersArmed" && waiting_for_lsboard_completion){
      // It would be better to check also if the state is really false
      if(running_tgboard_delay_scan) FinishTgBoardDelayStep();
      if(running_lsboard_scan) FinishLsBoardStep();
    }
  }
}

////////////////////////////////////////////////////////////////////
// Methods Implementing the Loop for the Trigger Board Delay Scan //
////////////////////////////////////////////////////////////////////

void LasCommissioner::StartTgBoardDelayRun()
{
  LOG4CPLUS_INFO (getApplicationLogger(), "Starting Trigger Board Delay Scan <Start Delay " << tgboard_delay_first.toString() << "> <End Delay " << tgboard_delay_last.toString() << ">");
  // Set the flags
  waiting_for_lsboard_completion = false;
  running_tgboard_delay_scan = true;
  automatic_refresh = true;

  SendSoapMessage ("LasTgBoardSupervisor", "SetInteractiveOff", *this);
  SendSoapMessage ("LasLsBoardSupervisor", "SetInteractiveOff", *this);

  //SendSoapMessage ("LasLsBoardSupervisor", "SetBoards", *this);
  //for(int i = 01; i <= 5; i++)  ScanSettings(i, tgboard_level, 0, 0);

  tgboard_delay_current = tgboard_delay_first;
  StartTgBoardDelayStep();
}

void  LasCommissioner::FinishTgBoardDelayRun()
{
  LOG4CPLUS_INFO (getApplicationLogger(), "Completed Trigger Board Delay Scan");
  SendSoapMessage ("LasLsBoardSupervisor", "TurnOffBoards", *this);
  SendSoapMessage ("LasTgBoardSupervisor", "SetInteractiveOn", *this);
  SendSoapMessage ("LasLsBoardSupervisor", "SetInteractiveOn", *this);
  running_tgboard_delay_scan = false;
  automatic_refresh = false;
}

void  LasCommissioner::StartTgBoardDelayStep()
{
  LOG4CPLUS_INFO (getApplicationLogger(), "Starting Trigger Board Delay Scan Step <Current Delay " << tgboard_delay_current.toString() << ">");
  SendSoapMessage ("LasTgBoardSupervisor", "SetTestModeOn", *this);
  SendSoapMessage ("LasTgBoardSupervisor", "SetDelay", "delay", tgboard_delay_current.toString(), *this);
  SendSoapMessage ("LasLsBoardSupervisor", "ArmBoards", *this);
  SendSoapMessage ("LasTgBoardSupervisor", "SetTestModeOff", *this);
  waiting_for_lsboard_completion = true;
}

void  LasCommissioner::FinishTgBoardDelayStep()
{
  LOG4CPLUS_INFO (getApplicationLogger(), "Completed Trigger Board Delay Scan Step");
  waiting_for_lsboard_completion = false;
  if(tgboard_delay_current < tgboard_delay_last){
    tgboard_delay_current = tgboard_delay_current + 1;
    StartTgBoardDelayStep();
  }
  else FinishTgBoardDelayRun();
}

////////////////////////////////////////////////////////////
// Methods Implementing the Loop for the Laser Board Scan //
////////////////////////////////////////////////////////////

void LasCommissioner::StartLsBoardRun()
{
  LOG4CPLUS_INFO (getApplicationLogger(), "Starting Laser Board Scan <Start Delay " << lsboard_delay_first.toString() << "> <End Delay " << lsboard_delay_last.toString() << ">");
  // Set the flags
  waiting_for_lsboard_completion = false;
  running_lsboard_scan = true;
  automatic_refresh = true;

  SendSoapMessage ("LasTgBoardSupervisor", "SetInteractiveOff", *this);
  SendSoapMessage ("LasLsBoardSupervisor", "SetInteractiveOff", *this);

  SendSoapMessage ("LasTgBoardSupervisor", "SetDelay", "delay", lsboard_tgboard_delay.toString(), *this);

  lsboard_id_current = lsboard_id_first;
  lsboard_level_current = lsboard_level_first;
  StartLsBoardStep();
}

void  LasCommissioner::FinishLsBoardRun()
{
  LOG4CPLUS_INFO (getApplicationLogger(), "Completed Laser Board Scan");

  // Reset all Flags
  running_lsboard_scan = false;
  automatic_refresh = false;
  waiting_for_lsboard_completion = false;

  // Turn off boards, and switch Supervisors back to interactive mode
  SendSoapMessage ("LasLsBoardSupervisor", "TurnOffBoards", *this);
  SendSoapMessage ("LasTgBoardSupervisor", "SetInteractiveOn", *this);
  SendSoapMessage ("LasLsBoardSupervisor", "SetInteractiveOn", *this);
}

void  LasCommissioner::StartLsBoardStep()
{
  LOG4CPLUS_INFO (getApplicationLogger(), "Starting Laser Board Scan Step <Current BoardId " << lsboard_id_current.toString() << "> <Current Level " << lsboard_level_current.toString() << ">");

  SendSoapMessage ("LasTgBoardSupervisor", "SetTestModeOn", *this);

  // Set Board levels (counters, off and scanned board)
  for(std::vector<int>::size_type i = 0; i < scan_table[lsboard_id_current-1].size(); i++){
    //std::cout << "scan_table entry " << lsboard_id_current-1 << "  configure board " << i+1 << "  to " << scan_table[lsboard_id_current-1][i] << std::endl;
    switch(scan_table[lsboard_id_current-1][i]){
    case OFF:
      // switch board i+1 off
      SwitchDiodes(i+1, false);
      break;
    case SCAN:
      // set board i+1 to scan (lsboard_level_current, lsboard_delay_first, lsboard_delay_last)
      ScanSettings(i+1, lsboard_level_current, lsboard_delay_first, lsboard_delay_last);
      break;
    case CTR:
      // set board i+1 to counter mode
      SetToCounter(i+1, lsboard_level_ctr, lsboard_delay_ctr);
      break;
    default:
      LOG4CPLUS_ERROR (getApplicationLogger(), "Unrecognized board state in board scan");
    }
  }

  SendSoapMessage ("LasLsBoardSupervisor", "ArmBoards", *this);
  SendSoapMessage ("LasTgBoardSupervisor", "SetTestModeOff", *this);

  waiting_for_lsboard_completion = true;
}

void  LasCommissioner::FinishLsBoardStep()
{
  LOG4CPLUS_INFO (getApplicationLogger(), "Completed Laser Board Scan Step");
  waiting_for_lsboard_completion = false;
  if(lsboard_level_current < lsboard_level_last){
    lsboard_level_current = lsboard_level_current + 1;
    StartLsBoardStep();
    return;
  }
  if(lsboard_id_current < lsboard_id_last){
    lsboard_level_current = lsboard_level_first;
    lsboard_id_current = lsboard_id_current + 1;
    StartLsBoardStep();
    return;
  }

  FinishLsBoardRun();
}
