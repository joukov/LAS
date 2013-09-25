#include "LasTgBoardSupervisor.h"
#include "LasCommon.h"

using namespace LaserAlignment;

#include "xdaq/NamespaceURI.h"
#include "xdaq/exception/ConfigurationError.h"

#include "xdata/XMLDOM.h"
#include <InfoSpaceFactory.h>

#include "toolbox/task/TimerFactory.h"

#include "xgi/Method.h"
#include "cgicc/HTMLClasses.h"

#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/domutils.h"
#include "xoap/Method.h"

#include <sstream>

XDAQ_INSTANTIATOR_IMPL(LasTgBoardSupervisor);

LasTgBoardSupervisor::LasTgBoardSupervisor(xdaq::ApplicationStub * s) : 
  xdaq::Application(s),
  AutoInit_(-1),
  address_table_loaded(false),
  board_initialized(false),
  interactive_mode(true),
  lasTgBoardAccess(),
  BaseAddress_(0xD0000000),
  AddressTableFileName_("LASTgBoardAddressTable.txt"),
  LASTgBoardDelay_ (0),
  repeat_trg(100),
  repeat_trg_ctr(0)
{
  /////////////////////////
  // bind SOAP Callbacks //
  /////////////////////////
  xoap::bind(this, &LasTgBoardSupervisor::SetTestModeOn,  "SetTestModeOn",  XDAQ_NS_URI );
  xoap::bind(this, &LasTgBoardSupervisor::SetTestModeOff, "SetTestModeOff", XDAQ_NS_URI );
  xoap::bind(this, &LasTgBoardSupervisor::SetInteractiveOn,  "SetInteractiveOn",  XDAQ_NS_URI );
  xoap::bind(this, &LasTgBoardSupervisor::SetInteractiveOff, "SetInteractiveOff", XDAQ_NS_URI );
  xoap::bind(this, &LasTgBoardSupervisor::SetDelay, "SetDelay", XDAQ_NS_URI );

  ////////////////////////
  // Bind XGI callbacks //
  ////////////////////////
  xgi::bind(this,&LasTgBoardSupervisor::Default, "Default");  
  xgi::bind(this,&LasTgBoardSupervisor::TgBoardInit, "TgBoardInit");
  xgi::bind(this,&LasTgBoardSupervisor::SetDelay, "SetDelay");
  xgi::bind(this,&LasTgBoardSupervisor::TestModeOn, "TestModeOn");
  xgi::bind(this,&LasTgBoardSupervisor::TestModeOff, "TestModeOff");
  xgi::bind(this,&LasTgBoardSupervisor::TriggerA, "TriggerA");
  xgi::bind(this,&LasTgBoardSupervisor::TriggerB, "TriggerB");
  xgi::bind(this,&LasTgBoardSupervisor::TriggerC, "TriggerC");
  xgi::bind(this,&LasTgBoardSupervisor::TriggerD, "TriggerD");
  xgi::bind(this,&LasTgBoardSupervisor::TriggerRep, "TriggerRep");

  // Initialize VME Address Table Name
  char *basic = getenv ("ENV_CMS_TK_LASTGBOARD") ;
  if (basic != NULL) {
    std::ostringstream vmeFileOff ;
    vmeFileOff << basic;
    // << "./LasTgBoard/LASTgBoardAddressTable.txt" ;
    //vmeFileOff << basic << "/LasTgBoard/AddressTableTest.txt" ;
    AddressTableFileName_ = vmeFileOff.str() ;
  }

  // Export Variables that should be visible
  getApplicationInfoSpace()->fireItemAvailable("LocalConfigFile", &LocalConfigFile_);
  getApplicationInfoSpace()->fireItemAvailable("Delay", &LASTgBoardDelay_);
  getApplicationInfoSpace()->fireItemAvailable("BaseAddress", &BaseAddress_);
  getApplicationInfoSpace()->fireItemAvailable("AddressTableFileName", &AddressTableFileName_);
  getApplicationInfoSpace()->fireItemAvailable("AutoInit", &AutoInit_);
  getApplicationInfoSpace()->fireItemAvailable("TgBoardInteractive", &interactive_mode, this);

  // Set up the Action Listener
  getApplicationInfoSpace()->addListener(this, "urn:xdaq-event:setDefaultValues");
  getApplicationInfoSpace()->addItemChangedListener("Delay",this);

  // Publish to InfoSpace

  try{
    is = xdata::InfoSpaceFactory::getInstance()->get("TrackerLaserAlignmentInfoSpace");
  }
  catch(xdata::exception::Exception){
    is = xdata::InfoSpaceFactory::getInstance()->create("TrackerLaserAlignmentInfoSpace");
  }
  is->fireItemAvailable("TgBoardDelay", &LASTgBoardDelay_, this);
  is->fireItemAvailable("TgBoardInteractive", &interactive_mode, this);


  //is = xdata::InfoSpaceFactory::getInstance()->get("TrackerLaserAlignmentInfoSpace");
  //is->fireItemAvailable("TgBoardDelay", &LASTgBoardDelay_, this);
  //is->fireItemAvailable("TgBoardInteractive", &interactive_mode, this);

  // Install a timer
  timer = toolbox::task::TimerFactory::getInstance()->createTimer("LasTgBoardEvents");
  //toolbox::TimeVal start(0,0);
  //toolbox::TimeInterval period(3,0);
  //timer->scheduleAtFixedRate ( start, this, period, 0, "EvtTrg");
  //timer->scheduleAtFixedRate ( start, this, period , 0, "EvtTrg");
  //timer->stop();
}

// Default Callback of XDAQ Application
void LasTgBoardSupervisor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  // HTML header stuff
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("LAS Trigger Board Supervisor") << std::endl;
  //  *out << cgicc::a("Visit our Web site").set("href","http://accms04.physik.rwth-aachen.de/~cms/Tracker/Alignment-Hardware/") << endl;


  if(!interactive_mode){
    // Automatic Reloading of HTML page every 5 seconds
    // Added string manipulation to remove xgi commands
    *out << "<script>";
    *out << "window.onload=function(){";
    *out << "var expression = window.location.host + \"/.*/\";";
    *out << "var thematch = window.location.href.match(\".*\" + expression);";
    *out << "var thecommand = 'window.location.replace(\"' + thematch + '\")';";
    //*out << "window.alert(thecommand);";
    *out << "window.setTimeout(thecommand,5000)";
    *out << "};";
    *out << "</script>";
  }


  *out << cgicc::h1("LAS Trigger Board Supervisor") << std::endl;
  *out << cgicc::hr() << std::endl;

  // Strings containing the methods for the forms
  std::string init_method = toolbox::toString("/%s/TgBoardInit",getApplicationDescriptor()->getURN().c_str());
  std::string delay_method = toolbox::toString("/%s/SetDelay",getApplicationDescriptor()->getURN().c_str());
  std::string testmodeon_method = toolbox::toString("/%s/TestModeOn",getApplicationDescriptor()->getURN().c_str());
  std::string testmodeoff_method = toolbox::toString("/%s/TestModeOff",getApplicationDescriptor()->getURN().c_str());
  std::string TriggerA_method = toolbox::toString("/%s/TriggerA",getApplicationDescriptor()->getURN().c_str());
  std::string TriggerB_method = toolbox::toString("/%s/TriggerB",getApplicationDescriptor()->getURN().c_str());
  std::string TriggerC_method = toolbox::toString("/%s/TriggerC",getApplicationDescriptor()->getURN().c_str());
  std::string TriggerD_method = toolbox::toString("/%s/TriggerD",getApplicationDescriptor()->getURN().c_str());
  std::string TriggerRep_method = toolbox::toString("/%s/TriggerRep",getApplicationDescriptor()->getURN().c_str());

  // Toggle enabling/disabling of fields	
  bool TTC_Trigger = !lasTgBoardAccess.GetStatus();
  std::string init_disabled = (interactive_mode && !board_initialized) ? "" : "disabled";
  std::string ctrl_disabled = (interactive_mode &&  board_initialized) ? "" : "disabled";
  std::string TTC_disabled = (interactive_mode &&  board_initialized && !TTC_Trigger) ? "" : "disabled";
  std::string Internal_disabled = (interactive_mode &&  board_initialized && TTC_Trigger) ? "" : "disabled";

  // Main Layout

  *out << "<table border=0 cellpadding=0 cellspacing=0>"; // Top Level Table
  *out << "<tr>"; // Row for Board Initialization
  // Form for the Initialization
  //*out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  //*out << std::endl;
  //*out << cgicc::legend("Initialize the Board") << cgicc::p(); // << std::endl;
  *out << cgicc::form().set("method","GET").set("action", init_method); // << std::endl;
  *out << "Address Table File Name ";
  *out << cgicc::input().set("type","text").set(init_disabled,init_disabled).set("name","TgBoardAddressTableFile").set("value", AddressTableFileName_.toString()).set("size","60");
  //*out << cgicc::p();
  std::ostringstream hex_base_address;
  hex_base_address << "0x" << std::hex << BaseAddress_;
  *out << "Base Address ";
  //  *out << cgicc::input().set("type","text").set("name","BaseAddress").set("value", BaseAddress_.toString());
  *out << cgicc::input().set("type","text").set(init_disabled,init_disabled).set("name","BaseAddress").set("value", hex_base_address.str());
  //*out << std::endl;
  *out << cgicc::input().set("type","submit").set(init_disabled,init_disabled).set("value","Apply")  << std::endl;
  *out << cgicc::form(); // << std::endl;
  //*out << cgicc::fieldset();

  *out << "</tr>"; // End Row for Board Initialization

  *out << "<tr>"; // Row for Tg Board Controls
  *out << "<td>"; // Column for Trigger Source Selection


  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << std::endl;
  *out << cgicc::legend("Trigger Source Selection") << cgicc::p() << std::endl;

  *out << "<table border=0 cellpadding=8 cellspacing=0>"; // Table for Trigger Source Selection


  *out << (TTC_Trigger ? "<tr bgcolor=\"#00FF00\">" : "<tr bgcolor=\"#FF0000\">");  // Row for TTC

  *out << "<td> TTC </td>";

  *out << "<td>";
  // Form for selecting TTC as Trigger Source 
  *out << cgicc::form().set("method","GET").set("action", testmodeoff_method) << std::endl;
  *out << cgicc::input().set("type","submit").set(TTC_disabled,TTC_disabled).set("value","Select TTC")  << std::endl;
  *out << cgicc::form() << std::endl;
  *out << "</td>";

  *out << (TTC_Trigger ? "<td> --> </td>" : "<td> </td>");

  *out << "</tr>"; // End Row for TTC

  *out << (!TTC_Trigger ? "<tr bgcolor=\"#00FF00\">" : "<tr bgcolor=\"#FF0000\">");  // Row for Internal Triggers

  *out << "<td>";
  // Forms for simulated Triggers
  *out << "<table border=0 cellpadding=0 cellspacing=0>"; // Table for Partition Triggering
  *out << "<tr><td>";
  *out << cgicc::form().set("method","GET").set("action", TriggerA_method) << std::endl;
  *out << cgicc::input().set("type","submit").set(TTC_disabled,TTC_disabled).set("value","Trigger A")  << std::endl;
  *out << cgicc::form() << std::endl;
  *out << "</td><td>";
  *out << cgicc::form().set("method","GET").set("action", TriggerB_method) << std::endl;
  *out << cgicc::input().set("type","submit").set(TTC_disabled,TTC_disabled).set("value","Trigger B")  << std::endl;
  *out << cgicc::form() << std::endl;
  *out << "</td><td>";
  *out << cgicc::form().set("method","GET").set("action", TriggerC_method) << std::endl;
  *out << cgicc::input().set("type","submit").set(TTC_disabled,TTC_disabled).set("value","Trigger C")  << std::endl;
  *out << cgicc::form() << std::endl;
  *out << "</td><td>";
  *out << cgicc::form().set("method","GET").set("action", TriggerD_method) << std::endl;
  *out << cgicc::input().set("type","submit").set(TTC_disabled,TTC_disabled).set("value","Trigger D")  << std::endl;
  *out << cgicc::form() << std::endl;
  *out << "</td></tr><tr><td colspan=4>";
  *out << cgicc::form().set("method","GET").set("action", TriggerRep_method) << std::endl;
  *out << cgicc::input().set("type","submit").set(TTC_disabled,TTC_disabled).set("value","Repetitive Trigger")  << std::endl;
  *out << cgicc::input().set("type","text").set(TTC_disabled,TTC_disabled).set("name","TriggerRep").set("value", repeat_trg.toString());
  *out << cgicc::form() << std::endl;
  *out << "</td></tr></table>";


  *out << "</td>";

  *out << "<td>";
  // Form for Testmode On
  *out << cgicc::form().set("method","GET").set("action", testmodeon_method) << std::endl;
  *out << cgicc::input().set("type","submit").set(Internal_disabled,Internal_disabled).set("value","Select internal")  << std::endl;
  *out << cgicc::form(); // << std::endl;
  //*out << "Status: " << lasTgBoardAccess.GetStatus();
  *out << "</td>";

  *out << (!TTC_Trigger ? "<td> --> </td>" : "<td> </td>");

  *out << "</tr>"; // End Row for Internal Triggers

  *out << "</table>";

  *out << cgicc::fieldset();

  *out << "</td>"; // End Column for Trigger Source Selection

  *out << "<td>"; // Column Delay settings
  // Form for Setting the Delay
  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;");
  *out << std::endl;
  *out << cgicc::legend("Set the value of Delay") << cgicc::p() << std::endl;
  *out << cgicc::form().set("method","GET").set("action", delay_method) << std::endl;
  *out << "Delay ";
  *out << cgicc::input().set("type","text").set(ctrl_disabled,ctrl_disabled).set("name","Delay").set("value", LASTgBoardDelay_.toString());
  *out << " Read Delay: " << lasTgBoardAccess.GetDelay() << "   ";
  *out << cgicc::input().set("type","submit").set(ctrl_disabled,ctrl_disabled).set("value","Apply")  << std::endl;
  *out << cgicc::form() << std::endl;
  *out << cgicc::fieldset();

  *out << "</td>"; // End Column Delay settings

  *out << "</tr>"; // End Row for Tg Board Controls

  *out << "</table>"; // End Top Level Table

}


void LasTgBoardSupervisor::failurePage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("LAS Trigger Board Supervisor") << std::endl;
  *out << cgicc::hr() << std::endl;
  *out << cgicc::h1("LAS Trigger Board Supervisor: Hardware failure ") << std::endl;
  *out << cgicc::h2("The Trigger Board could not be initialized. Check if it is ON/operating properly ") << std::endl; 
  *out << cgicc::hr() << std::endl;
}

// Initialize the LAS Trigger Board Acces
void LasTgBoardSupervisor::TgBoardInit(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  //std::cout << "Calling TgBoardInit()" << std::endl;
  try{
    // Gather all input in one CGI object
    cgicc::Cgicc cgi(in);

    // Get the name of the file with the address table
    //xdata::String TgBoardAddressTableFile_ = cgi["TgBoardAddressTableFile"]->getValue();
    AddressTableFileName_ = cgi["TgBoardAddressTableFile"]->getValue();
    std::istringstream inp(cgi["BaseAddress"]->getValue());
    // Get the base address
    unsigned long ba;
    inp >> std::hex >> ba;
    BaseAddress_=ba;

    // Initialize the Trigger Board
    //lasTgBoardAccess.Initialize(TgBoardAddressTableFile_, BaseAddress_);
    lasTgBoardAccess.Initialize(AddressTableFileName_, BaseAddress_);
    board_initialized=true;

    LOG4CPLUS_INFO (getApplicationLogger(), "Trigger Board creation done on address " << std::hex << BaseAddress_  << "status is " << std::hex << lasTgBoardAccess.GetStatus() );

    // Call again the Default callback
    this->Default(in,out);		
  }
  catch (const std::exception & e){
    //LOG4CPLUS_INFO (getApplicationLogger(), "Trigger Board could not be initialized. Check if it is On/Operating properly" );
    LOG4CPLUS_INFO (getApplicationLogger(), e.what() );
   //SendSoapMessage("failedTriggerBoard",    "LasSupervisor", *this);  // Careful, this function may throw an exception
   failurePage(in,out);
    //XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	

}

// Set the LAS Trigger Board Delay
void LasTgBoardSupervisor::SetDelay(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Get the Delay Value 
    LASTgBoardDelay_ = cgi["Delay"]->getIntegerValue();

    // Set the Delay
    lasTgBoardAccess.SetDelay(LASTgBoardDelay_);
    is->fireItemValueChanged("TgBoardDelay", 0);

    // Call again the Default callback
    this->Default(in,out);		
  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Turn Test Mode On
void LasTgBoardSupervisor::TestModeOn(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Turn Test Mode On
    lasTgBoardAccess.TestModeOn();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Turn Test Mode Off
void LasTgBoardSupervisor::TestModeOff(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Turn Test Mode Off
    lasTgBoardAccess.TestModeOff();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Simulate a Trigger on Partition A
void LasTgBoardSupervisor::TriggerA(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Trigger Partition A
    lasTgBoardAccess.TriggerA();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Simulate a Trigger on Partition B
void LasTgBoardSupervisor::TriggerB(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Trigger Partition B
    lasTgBoardAccess.TriggerB();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Simulate a Trigger on Partition C
void LasTgBoardSupervisor::TriggerC(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Trigger Partition C
    lasTgBoardAccess.TriggerC();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Simulate a Trigger on Partition D
void LasTgBoardSupervisor::TriggerD(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Trigger Partition D
    lasTgBoardAccess.TriggerD();

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}

// Send repetitive Software Triggers on Partition A
void LasTgBoardSupervisor::TriggerRep(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  try{
    // Gather all input of the form in one CGI object
    cgicc::Cgicc cgi(in);

    // Get the Repeat Value 
    repeat_trg = cgi["TriggerRep"]->getIntegerValue();


    repeat_trg_ctr = 0;
    toolbox::TimeInterval interval(0,100000);
    toolbox::TimeVal start;
    start = toolbox::TimeVal::gettimeofday();
    if(! timer->isActive())timer->start();
    timer->scheduleAtFixedRate( start, this, interval, 0, "" );

//     for(unsigned short i=0; i < repeat_trg; i++){
//       // Trigger Partition A
//       lasTgBoardAccess.TriggerA();
//       log4cplus::helpers::sleepmillis(2); // wait 2 ms
//     }

    // Call again the Default callback
    this->Default(in,out);		

  }
  catch (const std::exception & e){
    XCEPT_RAISE(xgi::exception::Exception, e.what());
  }	
  
}


///////////
//SOAP
///////////

//! Test Mode ON SOAP interface
xoap::MessageReference LasTgBoardSupervisor::SetTestModeOn (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"SetTestModeOn"); 
  try{
    if (board_initialized) lasTgBoardAccess.TestModeOn();    // Turn Test Mode On
    else return SOAPReply("SetTestModeOnFailed");
  }
  catch (const std::exception & e){ XCEPT_RAISE(xoap::exception::Exception, e.what()); }

  return SOAPReply("SetTestModeOnOK");
}


//! Test Mode OFF SOAP interface
xoap::MessageReference LasTgBoardSupervisor::SetTestModeOff (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"SetTestModeOff"); 
  try{
    if (board_initialized) lasTgBoardAccess.TestModeOff();    // Turn Test Mode Off
    else return SOAPReply("SetTestModeOffFailed");
  }
  catch (const std::exception & e){ XCEPT_RAISE(xoap::exception::Exception, e.what()); }

  return SOAPReply("SetTestModeOffOK");
}


xoap::MessageReference LasTgBoardSupervisor::SetInteractiveOn (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"SetInteractiveOn"); 
  interactive_mode = true;
  return SOAPReply("SetInteractiveOnOK");
}


xoap::MessageReference LasTgBoardSupervisor::SetInteractiveOff (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(),"SetInteractiveOff"); 
  interactive_mode = false;
  return SOAPReply("SetInteractiveOffOK");
}

xoap::MessageReference LasTgBoardSupervisor::SetDelay (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  try{
    // Retrieve the Delay
    std::istringstream delaystream( GetSoapAttribute(msg, "delay") );
    int delay;
    delaystream >> delay;
    LASTgBoardDelay_ = delay;
    
    LOG4CPLUS_INFO (getApplicationLogger(),"Set Delay to " << LASTgBoardDelay_);
    
    lasTgBoardAccess.SetDelay(LASTgBoardDelay_);  // Set the Delay
    is->fireItemValueChanged("TgBoardDelay", 0);
  }
  catch (const std::exception & e){ XCEPT_RAISE(xoap::exception::Exception, e.what()); }

  return SOAPReply("SetDelayOK");
}

//! Callback for Action Listener
void LasTgBoardSupervisor::actionPerformed (xdata::Event& e) 
{
  static bool local_config_loaded = false; // This variable avoids infinite recursive calls when local configuration is applied
  //std::cout << "e.type: " << e.type() << std::endl;

  if ( e.type() == "ItemChangedEvent" ){
    // Set the Delay
    lasTgBoardAccess.SetDelay(LASTgBoardDelay_);
  }
  if ( e.type() == "urn:xdaq-event:setDefaultValues" ){
//     LOG4CPLUS_INFO (getApplicationLogger(), 
// 		    "Checking if local config needs to be loaded " 
// 		    << "  AutoInit_: " << AutoInit_ 
// 		    << "  local_config_loaded: " << (local_config_loaded ? "true" : "false"));
    if(!local_config_loaded){
      try{
	LOG4CPLUS_INFO( getApplicationLogger(), 
			"Default values have been set for LasTgBoardSupervisor by XDAQ server, now loading local xml configuration  " << std::string(LocalConfigFile_)
			);
	//xdata::XMLDOMLoader loader(AbstractDOMParser::Val_Never);
	xdata::XMLDOMLoader loader;
	DOMDocument * document = loader.load(LocalConfigFile_);
	local_config_loaded = true; // Loading local configuration should be tried only once, since there is a recursive call (see below)
	setDefaultValues(document->getDocumentElement()); // WARNING: This line will generate a recursive call of this function
      }
      catch(xdaq::exception::ParameterSetFailed & e){
	LOG4CPLUS_INFO (getApplicationLogger(), 
			"\nException 'xdaq::exception::ParameterSetFailed' when trying to initialize LasTgBoardSupervisor with file " << std::string(LocalConfigFile_)
			<< "\nexception error message: " << e.message()
			<< "\nline in which the exception occurred: " << e.line()
			<< "\nfunction in which the exception occurred: " << e.function()
			<< "\nmodule in which the exception occurred: " << e.module()
			<< "\n Exception reporting: " << e.what() << "\n");
      }
      catch(xdaq::exception::ConfigurationError & e){
	LOG4CPLUS_INFO (getApplicationLogger(), 
			"\nException 'xdaq::exception::ConfigurationError' when trying to initialize LasTgBoardSupervisor with file " << std::string(LocalConfigFile_)
			<< "\nexception error message: " << e.message()
			<< "\nline in which the exception occurred: " << e.line()
			<< "\nfunction in which the exception occurred: " << e.function()
			<< "\nmodule in which the exception occurred: " << e.module()
			<< "\n Exception reporting: " << e.what() << "\n");
      }
      catch(xdaq::exception::Exception & e){
	LOG4CPLUS_INFO (getApplicationLogger(),
			"\nException 'xdaq::exception::Exception' when trying to initialize LasTgBoardSupervisor with file " << std::string(LocalConfigFile_)
			<< "\nexception error message: " << e.message()
			<< "\nline in which the exception occurred: " << e.line()
			<< "\nfunction in which the exception occurred: " << e.function()
			<< "\nmodule in which the exception occurred: " << e.module()
			<< "\n Exception reporting: " << e.what() << "\n");
      }
      catch(std::exception & e){
	LOG4CPLUS_INFO (getApplicationLogger(),
			"\nException 'std::exception' when trying to initialize LasTgBoardSupervisor with file " << std::string(LocalConfigFile_)
			<< "\n Exception reporting: " << e.what() << "\n");
      }
      
    }
    

//     LOG4CPLUS_INFO (getApplicationLogger(), 
// 		    "Checking if boards need to be initialized " 
// 		    << "  AutoInit_: " << AutoInit_ 
// 		    << "  boards_initialized: " << (board_initialized ? "true" : "false"));
    if(AutoInit_ == 1 && !board_initialized){
      // Initialize the Trigger Board
      
      try{
        lasTgBoardAccess.Initialize(AddressTableFileName_, BaseAddress_);
        board_initialized=true;
	LOG4CPLUS_INFO (getApplicationLogger(),
			"Board creation done on address " << std::hex << BaseAddress_
			<< ", status is " << std::hex << lasTgBoardAccess.GetStatus() << std::dec );
        // Set the Delay
        lasTgBoardAccess.SetDelay(LASTgBoardDelay_);
        // Turn Test Mode On to prevent triggers from propagating
        lasTgBoardAccess.TestModeOn();
      }
      catch (const HAL::HardwareAccessException& e){
        //SendSoapMessage("failedTriggerBoard",    "LasSupervisor", *this); // Careful, this function may throw an exception
        LOG4CPLUS_INFO (getApplicationLogger(), 
			"\nTrigger Board could not be initialized. Check if it is On/Operating properly"
			<< "\n The HAL Library reports a Hardware Access Exception"
			<< "\n The error message is: " << e.what());
      }
      catch (const std::exception & e){ 
        //SendSoapMessage("failedTriggerBoard",    "LasSupervisor", *this); // Careful, this function may throw an exception
        LOG4CPLUS_INFO (getApplicationLogger(), 
			"\nTrigger Board could not be initialized. Check if it is On/Operating properly"
			<< "\nThe error message is: " << e.what());
      }
    }
  }
}

// Callback for Timer Listener
void LasTgBoardSupervisor::timeExpired(toolbox::task::TimerEvent& e)
{
  //LOG4CPLUS_INFO (getApplicationLogger(), "timeExpired was called ");
  if(repeat_trg_ctr >= repeat_trg && timer->isActive()) timer->stop();
  else{
    repeat_trg_ctr = repeat_trg_ctr + 1;
    // Trigger Partition A
    lasTgBoardAccess.TriggerA();
  }
}
