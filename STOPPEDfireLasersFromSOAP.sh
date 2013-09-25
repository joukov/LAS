#!/bin/sh

curl --stderr /dev/null \
-H "SOAPAction: urn:xdaq-application:class=LasSupervisor,instance=0 " \
-d "<SOAP-ENV:Envelope SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" 
  xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" 
  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" 
  xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\">
<SOAP-ENV:Header>
</SOAP-ENV:Header>
<SOAP-ENV:Body>
	<xdaq:$1 xmlns:xdaq=\"urn:xdaq-soap:3.0\">
        </xdaq:$1>
</SOAP-ENV:Body>
</SOAP-ENV:Envelope>"  http://trackerlas-s5f01-20.cms:40000

echo ""
