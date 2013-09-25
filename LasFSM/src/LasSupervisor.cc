// $Id: LasSupervisor.cc,v 1.8 2011/07/04 13:33:50 wittmer Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "LasSupervisor.h"

// XDAQ includes
#include "xdaq/NamespaceURI.h"

// Toolbox includes
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/TimerFactory.h"

// XGI and CGICC includes
#include "xgi/WSM.h"
#include "xgi/Utils.h"
#include "xgi/Method.h"

// XOAP includes
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/domutils.h"
#include "xoap/Method.h"

//
// provides factory method for instantion of SOAPStateMachine application
//
XDAQ_INSTANTIATOR_IMPL(LasSupervisor)

LasSupervisor::LasSupervisor(xdaq::ApplicationStub * s) : 
  xdaq::Application(s),
  FireInterval(300),
  TimerActive(false),
  laserBoardFailed(false),
  triggerBoardFailed(false)
{

  // Bind XGI Callbacks
  xgi::bind(this, &LasSupervisor::Default,    "Default" );
  xgi::bind(this, &LasSupervisor::dispatch,   "dispatch" );
  xgi::bind(this, &LasSupervisor::firelasers, "firelasers" );  

  // Bind SOAP callbacks
  xoap::bind(this, &LasSupervisor::fireEvent,       "Configure", XDAQ_NS_URI );
  xoap::bind(this, &LasSupervisor::fireEvent,       "Enable", XDAQ_NS_URI );
  xoap::bind(this, &LasSupervisor::fireEvent,       "Suspend", XDAQ_NS_URI );
  xoap::bind(this, &LasSupervisor::fireEvent,       "Resume", XDAQ_NS_URI );
  xoap::bind(this, &LasSupervisor::fireEvent,       "Halt", XDAQ_NS_URI );
  xoap::bind(this, &LasSupervisor::fireEvent,       "Stop", XDAQ_NS_URI );
  xoap::bind(this, &LasSupervisor::reset,           "Reset", XDAQ_NS_URI );  

  // This is for external firing of lasers with a SOAP message  
  xoap::bind(this, &LasSupervisor::fireLasersFromSOAP,  "FireLasersFromSOAP", XDAQ_NS_URI );    
  xoap::bind(this, &LasSupervisor::failedTriggerBoard,  "failedTriggerBoard", XDAQ_NS_URI );  
  xoap::bind(this, &LasSupervisor::failedLaserBoard,    "failedLaserBoard", XDAQ_NS_URI );

  //
  // Define FSM
  //
  fsm_.addState('H', "Halted",    this, &LasSupervisor::stateChanged);
  fsm_.addState('R', "Ready",     this, &LasSupervisor::stateChanged);
  fsm_.addState('E', "Enabled",   this, &LasSupervisor::stateChanged);
  fsm_.addState('S', "Suspended", this, &LasSupervisor::stateChanged);
  //        fsm_.addState('K', "Stopped", this, &LasSupervisor::stateChanged);
    

  fsm_.addStateTransition('H', 'R', "Configure", this, &LasSupervisor::ConfigureAction);
  fsm_.addStateTransition('R', 'E', "Enable",    this, &LasSupervisor::EnableAction);
  fsm_.addStateTransition('E', 'S', "Suspend",   this, &LasSupervisor::SuspendAction);
  fsm_.addStateTransition('S', 'E', "Resume",    this, &LasSupervisor::ResumeAction);
  fsm_.addStateTransition('S', 'R', "Stop",      this, &LasSupervisor::StopAction);
  fsm_.addStateTransition('E', 'R', "Stop",      this, &LasSupervisor::StopAction);

  fsm_.addStateTransition('H', 'H', "Halt", this, &LasSupervisor::HaltAction);
  fsm_.addStateTransition('R', 'H', "Halt", this, &LasSupervisor::HaltAction);
  fsm_.addStateTransition('E', 'H', "Halt", this, &LasSupervisor::HaltAction);
  fsm_.addStateTransition('S', 'H', "Halt", this, &LasSupervisor::HaltAction);

  
  // Failure state setting
  fsm_.setFailedStateTransitionAction ( this, &LasSupervisor::failedTransition );
  fsm_.setFailedStateTransitionChanged( this, &LasSupervisor::stateChanged );
 
  fsm_.setInitialState('H');
  fsm_.setStateName('F', "Failed"); // give a name to the 'F' state
  fsm_.reset();

  state_ = fsm_.getStateName (fsm_.getCurrentState()); // Update the state variable
  
  // Export variables that should be visible to the outside
  getApplicationInfoSpace()->fireItemAvailable("stateName",&state_);
  getApplicationInfoSpace()->fireItemAvailable("TimerActive",&TimerActive);   
  getApplicationInfoSpace()->fireItemAvailable("FireInterval",&FireInterval);   

  // Install a timer for automatic firing of the lasers
  toolbox::task::TimerFactory::getInstance()->createTimer("LAS_Main_Timer");
}


xoap::MessageReference LasSupervisor::fireEvent (xoap::MessageReference msg)
	throw (xoap::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), "fireEvent" );
  
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
	  
	  
	  try
	    {
	      toolbox::Event::Reference e(new toolbox::Event(commandName,this));
	      fsm_.fireEvent(e);
	    }
	  catch (toolbox::fsm::exception::Exception & e)
	    {
	      XCEPT_RETHROW(xoap::exception::Exception, "invalid command", e);
	    }
	  
	  catch(...){
	    LOG4CPLUS_INFO (getApplicationLogger(), "Unspecified Exception caught when firing Event");
	  }
	  xoap::MessageReference reply = xoap::createMessage();
	  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
	  xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
	  (void) envelope.getBody().addBodyElement ( responseName );
	  return reply;
	}
    }
  
  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}


xoap::MessageReference LasSupervisor::reset (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
     LOG4CPLUS_INFO (getApplicationLogger(), fsm_.getStateName (fsm_.getCurrentState()) );

     fsm_.reset();
     state_ = fsm_.getStateName (fsm_.getCurrentState());

	xoap::MessageReference reply = xoap::createMessage();
	xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
	xoap::SOAPName responseName = envelope.createName("ResetResponse", "xdaq", XDAQ_NS_URI);
	// xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
	(void) envelope.getBody().addBodyElement ( responseName );

	return reply;
}

xoap::MessageReference LasSupervisor::fireLasersFromSOAP (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
 
  xdata::String currentState = fsm_.getStateName(fsm_.getCurrentState());
               
  if (currentState=="Enabled")
  {
    switchOnAllBoards();
  }


   LOG4CPLUS_INFO (getApplicationLogger(), "Firing lasers from cron job every x minutes .." );
 

   xoap::MessageReference reply = xoap::createMessage();
   xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
   xoap::SOAPName responseName = envelope.createName("ResetResponse", "xdaq", XDAQ_NS_URI);
   (void) envelope.getBody().addBodyElement ( responseName );

   return reply;

}


xoap::MessageReference LasSupervisor::failedTriggerBoard (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
   LOG4CPLUS_INFO (getApplicationLogger(), "Error:  Trigger Board failed" );
   triggerBoardFailed=true; 
   try
    {
     toolbox::Event::Reference e(new toolbox::Event("Stop",this));
     fsm_.fireEvent(e);
    }
    catch (toolbox::fsm::exception::Exception & e)
    {
     XCEPT_RETHROW(xoap::exception::Exception, "invalid command", e);
    }


   LOG4CPLUS_INFO (getApplicationLogger(), "Error:  Trigger Board failed" );


   xoap::MessageReference reply = xoap::createMessage();
   xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
   xoap::SOAPName responseName = envelope.createName("ResetResponse", "xdaq", XDAQ_NS_URI);
   (void) envelope.getBody().addBodyElement ( responseName );

   return reply;

}


xoap::MessageReference LasSupervisor::failedLaserBoard (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
   LOG4CPLUS_INFO (getApplicationLogger(), "Error:  Laser Board failed" );
   laserBoardFailed=true;

   xdata::String currentState = fsm_.getStateName(fsm_.getCurrentState());

   try
    {
      if (currentState=="Enabled")
      {     
        toolbox::Event::Reference e(new toolbox::Event("Stop",this));
        fsm_.fireEvent(e);
      }
    }
    catch (toolbox::fsm::exception::Exception & e)
    {
     XCEPT_RETHROW(xoap::exception::Exception, "invalid command", e);
    }


   LOG4CPLUS_INFO (getApplicationLogger(), "Error:  Laser Board failed" );


   xoap::MessageReference reply = xoap::createMessage();
   xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
   xoap::SOAPName responseName = envelope.createName("ResetResponse", "xdaq", XDAQ_NS_URI);
   (void) envelope.getBody().addBodyElement ( responseName );

   return reply;

}



void LasSupervisor::ConfigureAction (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{
	LOG4CPLUS_INFO (getApplicationLogger(), e->type());
}

void LasSupervisor::EnableAction (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());
  if(TimerActive){
    try{
      toolbox::task::Timer* timer = toolbox::task::TimerFactory::getInstance()->getTimer("LAS_Main_Timer");
      if(! timer->isActive())
	timer->start();
      toolbox::TimeVal start(0,0);
      //toolbox::TimeInterval period( FireInterval * 60, 0 );
      toolbox::TimeInterval period( FireInterval, 0 );
      timer->scheduleAtFixedRate ( start, this, period , 0, "FireLasers");
      LOG4CPLUS_INFO (getApplicationLogger(), "timer->isActive(): " << (timer->isActive()?"true":"false"));
      LOG4CPLUS_INFO (getApplicationLogger(), "timer->getScheduledTasks().size(): " << timer->getScheduledTasks().size());
    }
    catch(toolbox::task::exception::Exception& ex){
      LOG4CPLUS_INFO (getApplicationLogger(), "Exception caught when trying to start LAS_Main_Timer, message: " << ex.what());
    }
  }
}

void LasSupervisor::StopAction (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());

  try{
    // Stop the Timer
    toolbox::task::Timer* timer = toolbox::task::TimerFactory::getInstance()->getTimer("LAS_Main_Timer");
    if(timer->isActive()) timer->stop();

    // Switch off the Laser boards
    switchOffAllBoards();
    return;
  }
  catch(xcept::Exception & ex){ // Base class of all XDAQ exceptions
    LOG4CPLUS_INFO (getApplicationLogger(),
		    "\nException '" << ex.name() << "' when trying to Stop "
		    << "\nexception error message: " << ex.message()
		    << "\nline in which the exception occurred: " << ex.line()
		    << "\nfunction in which the exception occurred: " << ex.function()
		    << "\nmodule in which the exception occurred: " << ex.module());
  }
  catch(std::exception & ex){
    LOG4CPLUS_INFO (getApplicationLogger(),
		    "Exception 'std::exception' when trying to Stop: " << ex.what());
    XCEPT_RAISE(toolbox::fsm::exception::Exception, ex.what());
  }
  catch(...){
    XCEPT_RAISE(toolbox::fsm::exception::Exception, "Undefined Exception when trying to Stop");
  }
}



void LasSupervisor::SuspendAction (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{
	LOG4CPLUS_INFO (getApplicationLogger(), e->type());

	// a failure is forced here
	//XCEPT_RAISE(toolbox::fsm::exception::Exception, "error in suspend");
}

void LasSupervisor::ResumeAction (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{
	LOG4CPLUS_INFO (getApplicationLogger(), e->type());

}

void LasSupervisor::HaltAction (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{
  LOG4CPLUS_INFO (getApplicationLogger(), e->type());

  try{
    // Stop the Timer
    toolbox::task::Timer* timer = toolbox::task::TimerFactory::getInstance()->getTimer("LAS_Main_Timer");
    if(timer->isActive()) timer->stop();
    
    // Switch off the Laser boards
    switchOffAllBoards();
    return;
  }
  catch(toolbox::task::exception::Exception& ex){
    LOG4CPLUS_INFO (getApplicationLogger(), "toolbox::task Exception caught when trying to Stop the Timer: " << ex.what());
  }
  catch(xoap::exception::Exception& ex){
    LOG4CPLUS_INFO (getApplicationLogger(), "SOAP Exception caught when trying to Halt: " << ex.what());
  }
  catch(xcept::Exception & ex){
    LOG4CPLUS_INFO (getApplicationLogger(),
		    "\nException 'xcept::Exception' when trying to Halt "
		    << "\nexception error message: " << ex.message()
		    << "\nline in which the exception occurred: " << ex.line()
		    << "\nfunction in which the exception occurred: " << ex.function()
		    << "\nmodule in which the exception occurred: " << ex.module()
		    << "\n Exception reporting: " << ex.what() << "\n");
  }
  catch(std::exception & ex){
    LOG4CPLUS_INFO (getApplicationLogger(),
		    "Exception 'std::exception' when trying to Stop: " << ex.what());
    XCEPT_RAISE(toolbox::fsm::exception::Exception, ex.what());
  }
  catch(...){
    XCEPT_RAISE(toolbox::fsm::exception::Exception, "Undefined Exception when trying to Stop");
  }
  
  return;
}



void LasSupervisor::stateChanged (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{
	// Reflect the new state
	state_ = fsm_.getStateName (fsm_.getCurrentState());
}

void LasSupervisor::failedTransition (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{ 
	toolbox::fsm::FailedEvent & fe = dynamic_cast<toolbox::fsm::FailedEvent&>(*e);
	LOG4CPLUS_INFO (getApplicationLogger(), "Failure occurred when performing transition from: "  <<
			fe.getFromState() <<  " to: " << fe.getToState() << " exception: " << fe.getException().what() );
}


 void LasSupervisor::dispatch(xgi::Input *in, xgi::Output *out ) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  // const cgicc::CgiEnvironment& env =cgi.getEnvironment();
  cgicc::const_form_iterator stateInputElement=cgi.getElement("StateInput");
  std::string stateInput=(*stateInputElement).getValue();
  state_ = (*stateInputElement).getValue();
  
  // display FSM
  std::set<std::string> possibleInputs = fsm_.getInputs(fsm_.getCurrentState());
  
  int isPossibleInput=0;
  
  std::set<std::string>::iterator i;
  for ( i = possibleInputs.begin(); i != possibleInputs.end(); i++){
    if ((*i)==stateInput) isPossibleInput=1; 
  }
  
  
  if(isPossibleInput){
    toolbox::Event::Reference e(new toolbox::Event(stateInput,this));
    fsm_.fireEvent(e); // This method can throw toolbox::fsm::exception::Exception !!!!!!!
  }
  this->Default(in,out);
}


 void LasSupervisor::Default(xgi::Input *in, xgi::Output *out ) throw (xgi::exception::Exception)
{
                out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
		*out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
		*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
   

		xgi::Utils::getPageHeader(*out, "",fsm_.getStateName(fsm_.getCurrentState()));

		std::string url = "/";
		url += getApplicationDescriptor()->getURN();
		url += "/dispatch";

		// display FSM
  		std::set<std::string> possibleInputs = fsm_.getInputs(fsm_.getCurrentState());
        	std::set<std::string> allInputs = fsm_.getInputs();

               // Refresh webpage
               //*out << "<script>";
               //*out << "window.onload=function(){";
               //*out << "window.setTimeout('window.location.reload()',10000)";
               //*out << "};";
               //*out << "</script>" << std::endl;



        	*out << cgicc::h3("LAS Supervisor").set("style", "font-family: arial") << std::endl;
        	*out << "<table border cellpadding=10 cellspacing=0>" << std::endl;
        	*out << "<tr>" << std::endl;
        	*out << "<th>" << fsm_.getStateName(fsm_.getCurrentState()) << "</th>" << std::endl;
        	*out << "</tr>" << std::endl;
        	*out << "<tr>" << std::endl;
       		std::set<std::string>::iterator i;
        	for ( i = allInputs.begin(); i != allInputs.end(); i++)
        	{
                	*out << "<td>";
			*out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;

                	if ( possibleInputs.find(*i) != possibleInputs.end() )
                	{      
                              if (  ( (fsm_.getStateName(fsm_.getCurrentState())=="Halted") ||
                                      (fsm_.getStateName(fsm_.getCurrentState())=="Ready" )   
                                    ) && 
                                    ((*i)=="Stop") )

                                
                              {  *out << cgicc::input() .set("type", "submit").set("name", "StateInput").set("value", (*i) ).set("disabled", "true"); }
                              else
                              { 
                        	*out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", (*i) );
                              }
                	}
                	else
                	{
                       	  *out << cgicc::input() .set("type", "submit").set("name", "StateInput").set("value", (*i) ).set("disabled", "true");
                	}

                	*out << cgicc::form();
                	*out << "</td>" << std::endl;
        	}
                *out << "</tr>" << std::endl;
                //*out << "</table>" << std::endl;


                // Fire lasers button enabled only when FSM enabled
                std::string urlfirelasers = "/";
                urlfirelasers += getApplicationDescriptor()->getURN();
                urlfirelasers += "/firelasers";

                //*out << "<tr>" << std::endl;
                //*out << "<td>"; *out << "</td>" << std::endl;
                //*out << "</tr>" << std::endl; 
               
                *out << "<tr>" << std::endl;
                //*out << "<td>"; *out << "</td>" << std::endl;
                *out << "</tr>" << std::endl;

                *out << "<tr>" << std::endl;
                //*out << "<td>"; *out << "</td>" << std::endl;
                *out << "<tr>" << std::endl;
                *out << "<td>";
  
             
                xdata::String currentState = fsm_.getStateName(fsm_.getCurrentState());
              
                if (currentState=="Enabled")
                {
                   
                   *out << cgicc::form().set("method","get").set("action", urlfirelasers).set("enctype","multipart/form-data")
                        << std::endl;
                   *out << cgicc::input().set("type", "submit").set("name", "FireButton").set("value", "Fire Lasers" );
                   *out << cgicc::form();
                }
                else
                { 
                 *out << cgicc::form().set("method","get").set("action", urlfirelasers).set("enctype","multipart/form-data")
                      << std::endl;
                 *out << cgicc::input().set("type", "submit").set("name", "FireButton").set("value", "Fire Lasers" ).set("disabled", "true");
                 *out << cgicc::form();

                }

                *out << "</td>" << std::endl;
                *out << "</tr>" << std::endl;
                *out << "</table>" << std::endl;
 
		//
		xgi::Utils::getPageFooter(*out);


 }


 void LasSupervisor::firelasers(xgi::Input *in, xgi::Output *out ) throw (xgi::exception::Exception)
{
   switchOnAllBoards();

   LOG4CPLUS_INFO (getApplicationLogger(), "Firing lasers from web interface. No person should be close to tracker!" );
   this->Default(in,out);
}


void LasSupervisor::switchOnAllBoards()
{
  SendSOAPmessage("SetTestModeOn",        "LasTgBoardSupervisor"); // Inhibit Triggers
  
  SendSOAPmessage("ArmBoards",            "LasLsBoardSupervisor");
  
  SendSOAPmessage("SetTestModeOff",       "LasTgBoardSupervisor"); // Enable Triggers
}


void LasSupervisor::switchOffAllBoards()
{

  if (!(triggerBoardFailed || laserBoardFailed))
    {
      SendSOAPmessage("TurnOffBoards",  "LasLsBoardSupervisor");
    }
}


void LasSupervisor::SendSOAPmessage (std::string soapMessageToSend, std::string whichSupervisor) 
{
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName(soapMessageToSend, "xdaq", XDAQ_NS_URI );
  body.addBodyElement(command);

  xdaq::ApplicationDescriptor& d = * getApplicationContext()->getDefaultZone()->getApplicationDescriptor(whichSupervisor, 0);
  xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *getApplicationDescriptor(), d);

  LOG4CPLUS_INFO (getApplicationLogger(), soapMessageToSend);
}


void LasSupervisor::SendSOAPmessage (std::string soapMessageToSend, std::string whichSupervisor, std::string attribute, std::string attr_value) 
{
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName(soapMessageToSend, "xdaq", XDAQ_NS_URI );
  xoap::SOAPName attr(attribute,"","");
  body.addBodyElement(command).addAttribute(attr, attr_value);
  
  xdaq::ApplicationDescriptor& d = * getApplicationContext()->getDefaultZone()->getApplicationDescriptor(whichSupervisor, 0);
  xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *getApplicationDescriptor(), d);

  LOG4CPLUS_INFO (getApplicationLogger(), soapMessageToSend);
}


// Callback for Action Listener
void LasSupervisor::actionPerformed (xdata::Event& e) 
{
}

// Callback for Timer Listener
void LasSupervisor::timeExpired(toolbox::task::TimerEvent& e)
{
  LOG4CPLUS_INFO (getApplicationLogger(), "timeExpired was called for Task: " << e.getTimerTask()->name);
  if(e.getTimerTask()->name == "FireLasers"){
    try{
      switchOnAllBoards();
    }
    catch(...){
      LOG4CPLUS_INFO (getApplicationLogger(), "Undefined Exception when trying to fire Lasers automatically" );
      //XCEPT_RAISE(toolbox::fsm::exception::Exception, "Undefined Exception when trying to fire Lasers automatically");
    }
  }
}
