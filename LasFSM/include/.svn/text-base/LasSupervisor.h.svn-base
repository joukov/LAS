// $Id: LasSupervisor.h,v 1.2 2010/02/02 13:51:03 wittmer Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _LasSupervisor_h_
#define _LasSupervisor_h_

#include "xdaq/WebApplication.h"
#include "xdata/String.h"
#include "xdata/Boolean.h"

#include "toolbox/task/Timer.h"
#include "toolbox/fsm/FiniteStateMachine.h"

class LasSupervisor: public xdaq::Application, public xdata::ActionListener, public toolbox::task::TimerListener
{	
 public:
	
  XDAQ_INSTANTIATOR();
	
  LasSupervisor(xdaq::ApplicationStub * s);
		
  //
  // SOAP Callbacks
  //
  xoap::MessageReference fireEvent          ( xoap::MessageReference msg ) throw (xoap::exception::Exception);
  xoap::MessageReference reset              ( xoap::MessageReference msg ) throw (xoap::exception::Exception); // Reset the state machine
  xoap::MessageReference fireLasersFromSOAP ( xoap::MessageReference msg ) throw (xoap::exception::Exception);
  xoap::MessageReference failedTriggerBoard ( xoap::MessageReference msg ) throw (xoap::exception::Exception);
  xoap::MessageReference failedLaserBoard   ( xoap::MessageReference msg ) throw (xoap::exception::Exception);

  void SendSOAPmessage (std::string soapMessageToSend, std::string whichSupervisor);
  void SendSOAPmessage (std::string soapMessageToSend, std::string whichSupervisor, std::string attribute, std::string attr_value);

  //
  // Finite State Machine action callbacks
  //
  void ConfigureAction  (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
  void EnableAction     (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
  void StopAction       (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
  void SuspendAction    (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
  void ResumeAction     (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
  void HaltAction       (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
  void failedTransition (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
  // Finite State Machine callback for entring state
  void stateChanged (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);

  //
  // XGI Callbacks
  //
  void dispatch  (xgi::Input *in, xgi::Output *out ) throw (xgi::exception::Exception); 
  void firelasers(xgi::Input *in, xgi::Output *out ) throw (xgi::exception::Exception);
  void Default  (xgi::Input *in, xgi::Output *out ) throw (xgi::exception::Exception);

  //
  // Listener Callbacks //
  //
  void actionPerformed (xdata::Event& e) ;        // Callback for Action Listener
  void timeExpired(toolbox::task::TimerEvent& e); // Callback for Timer Listener

 private:
  void switchOnAllBoards();
  void switchOffAllBoards(); 

  toolbox::fsm::FiniteStateMachine fsm_; // the actual state machine
  xdata::String state_;                  // used to reflect the current state to the outside world	

  xdata::UnsignedInteger FireInterval;   // Time Interval in minutes between automatic firing of Lasers
  xdata::Boolean TimerActive;            // Enable/Disable automatic firing of Lasers

  bool  laserBoardFailed;
  bool  triggerBoardFailed; 
};

#endif
