#include "net_interface.h"
#include "net_interface_ip.h"
#include "net_interface_tcp.h"
#include "net_interface_hardware.h"
#include "net_interface_software.h"
#include "net_interface_intermediary.h"
#include "net_interface_medium.h"
#include "net_interface_ip_address.h"

#include "net_interface_helper.h"

#include "../../escape.h"

/*
  The best way to think about how this packetizer works is thinking of
  this as an intermediate between the information coming in and going
  directly to whatever the underlying software device (socket) is.

  TCP is pretty weird, since a lot of the cool things it does are offloaded
  to the software device directly (i.e. a socket device is created that does
  a lot of the fancy stuff for us), so it just passes the information directly
  down as one giant vector.

  UDP segments the information into the highest possible MTU (defined as the
  lowest of either the software_dev MTU and the hardware_dev MTU), including
  a numbering scheme for making sure all of the information needed arrives in
  order. A vector of recently sent packets should be kept around so we can
  retransmit the information needed (TODO: allow for the packetizer to
  send information about itself to and from, so we can actually get some decent
  performance out of UDP, since UDP doesn't have the sort of retransmission
  latency that radio does).

  This is where packet radio starts to get pretty hairy, since this would be
  where software modems and modulation schemes start to come into play

  TODO: actuall bother with proper encapsulation at some time, for now we
  merge the two together (this isn't packet radio, we can do that now), and
  just escape and unescape vectors
*/

#define NET_INTERFACE_PACKET_TCP_ESCAPE 0xAB

INTERFACE_PACKETIZE(ip, tcp){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_SW_PTR(software_dev_id);

	const std::vector<uint8_t> retval =
		escape_vector(
			*packet,
			NET_INTERFACE_PACKET_TCP_ESCAPE);
	packet->clear();
	print("packetized protocol datagram of length " + std::to_string(retval.size()), P_WARN);
	return std::vector<std::vector<uint8_t> >({retval});;
}

INTERFACE_DEPACKETIZE(ip, tcp){
	INTERFACE_SET_HW_PTR(hardware_dev_id);
	INTERFACE_SET_SW_PTR(software_dev_id);
	const std::vector<uint8_t> real_packet =
		convert::vector::collapse_2d_vector(
			*packet);
	const std::pair<std::vector<uint8_t>, std::vector<uint8_t> > tmp =
		unescape_vector(
			real_packet,
			NET_INTERFACE_PACKET_TCP_ESCAPE);
	packet->clear();
	packet->push_back(tmp.second);
	if(tmp.first.size() > 0){
		print("depacketized protocol datagram of length " + std::to_string(tmp.first.size()), P_WARN);
	}
	return tmp.first;
}
