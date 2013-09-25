#ifndef __LASCOMON_H__
#define __LASCOMON_H__

#include <string>

#include "xoap/MessageReference.h"
#include "xdaq/Application.h"


namespace LaserAlignment {

  // SOAP utilities 
  typedef std::vector<std::pair<std::string, std::string> > attribute_list;
  void SendSoapMessage (std::string whichSupervisor, std::string soapMessageToSend, std::string attribute, std::string attr_value, xdaq::Application& sender);
  void SendSoapMessage (std::string whichSupervisor, std::string soapMessageToSend, attribute_list& att_list, xdaq::Application& sender);
  void SendSoapMessage (const std::string& soapMessageToSend, const std::string& whichSupervisor, xdaq::Application& sender);

  xoap::MessageReference SOAPReply(const std::string& message);

  std::string GetSoapCommand (xoap::MessageReference msg);
  std::string GetSoapAttribute(xoap::MessageReference msg, const std::string& attribute);
}

#endif
