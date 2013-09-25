#include "LasCommon.h"

#include <xdaq/NamespaceURI.h>
#include <xdaq/ApplicationDescriptor.h>

#include <xoap/MessageFactory.h>
#include <xoap/SOAPEnvelope.h>

#include <xoap/domutils.h>

namespace LaserAlignment{

  xoap::MessageReference SOAPReply(const std::string& message)
{
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( message );
  (void) envelope.getBody().addBodyElement ( responseName );
  return reply;
 }



//! send a SOAP message to a specific Application
void SendSoapMessage (const std::string& whichSupervisor, const std::string& soapMessageToSend, xdaq::Application& sender)
{
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName(soapMessageToSend, "xdaq", XDAQ_NS_URI );
  body.addBodyElement(command);
  
  xdaq::ApplicationDescriptor& d = *  sender.getApplicationContext()->getDefaultZone()->getApplicationDescriptor(whichSupervisor, 0);
  xoap::MessageReference reply = sender.getApplicationContext()->postSOAP(msg, * sender.getApplicationDescriptor(), d);

  LOG4CPLUS_INFO (sender.getApplicationLogger(), soapMessageToSend);
}

//! send a SOAP message to a specific Application with an attribute
void SendSoapMessage (std::string whichSupervisor, std::string soapMessageToSend, std::string attribute, std::string attr_value, xdaq::Application& sender) 
{
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName(soapMessageToSend, "xdaq", XDAQ_NS_URI );
  xoap::SOAPName attr(attribute,"","");
  body.addBodyElement(command).addAttribute(attr, attr_value);
  
  xdaq::ApplicationDescriptor& d = * sender.getApplicationContext()->getDefaultZone()->getApplicationDescriptor(whichSupervisor, 0);
  xoap::MessageReference reply = sender.getApplicationContext()->postSOAP(msg, * sender.getApplicationDescriptor(), d);

  LOG4CPLUS_INFO (sender.getApplicationLogger(), soapMessageToSend);
}

//! send a SOAP message to a specific Application with a list of attributes
void SendSoapMessage (std::string whichSupervisor, std::string soapMessageToSend, attribute_list& att_list, xdaq::Application& sender)
{
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName(soapMessageToSend, "xdaq", XDAQ_NS_URI );

  for(attribute_list::size_type i = 0; i < att_list.size(); i++){
    xoap::SOAPName attr(att_list[i].first,"","");
    body.addBodyElement(command).addAttribute(attr, att_list[i].second);
  }

  xdaq::ApplicationDescriptor& d = * sender.getApplicationContext()->getDefaultZone()->getApplicationDescriptor(whichSupervisor, 0);
  xoap::MessageReference reply = sender.getApplicationContext()->postSOAP(msg, * sender.getApplicationDescriptor(), d);

  LOG4CPLUS_INFO (sender.getApplicationLogger(), soapMessageToSend);
}



//! Return the value of one specific Attribute
std::string GetSoapAttribute(xoap::MessageReference msg, const std::string& attribute)
{
  //std::string the_attribute = attribute;
  xoap::SOAPPart part = msg->getSOAPPart();
  xoap::SOAPEnvelope env = part.getEnvelope();
  xoap::SOAPBody body = env.getBody();
  DOMNode* node = body.getDOMNode();
  DOMNodeList* bodyList = node->getChildNodes();
  for (unsigned int i = 0; i < bodyList->getLength(); i++){
    DOMNode* command = bodyList->item(i);
    if (command->getNodeType() == DOMNode::ELEMENT_NODE){
      DOMNamedNodeMap * node_map = command->getAttributes();
      DOMNode * attribute_node = node_map->getNamedItem(XMLString::transcode(attribute.c_str()));
      if(attribute_node){
	//std::cout << "attribute_node NodeName is " << xoap::XMLCh2String (attribute_node->getNodeName()) <<  std::endl ; 
	//std::cout << "attribute_node NodeValue is " << xoap::XMLCh2String (attribute_node->getNodeValue()) <<  std::endl ; 
	return xoap::XMLCh2String ( attribute_node->getNodeValue() );
      }
    }
  }
  XCEPT_RAISE(xoap::exception::Exception,"Failed to find command or attribute node in SOAP message");
}

//! Return the name of the command inside the SOAP message
std::string GetSoapCommand (xoap::MessageReference msg)
{
  xoap::SOAPPart part = msg->getSOAPPart();
  xoap::SOAPEnvelope env = part.getEnvelope();
  xoap::SOAPBody body = env.getBody();
  DOMNode* node = body.getDOMNode();
  DOMNodeList* bodyList = node->getChildNodes();
  for (unsigned int i = 0; i < bodyList->getLength(); i++){
    DOMNode* command = bodyList->item(i);
    if (command->getNodeType() == DOMNode::ELEMENT_NODE){
      return xoap::XMLCh2String (command->getLocalName());
    }
  }
  XCEPT_RAISE(xoap::exception::Exception,"Failed to find a command in the SOAP message");
}

}
