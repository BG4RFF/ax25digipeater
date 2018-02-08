// Generic Libraries

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <error.h>

// Socket Libraries

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef __GLIBC__
  #include <net/if.h>
  #include <net/ethernet.h>
#else
  #include <linux/if_ether.h>
  #include <linux/if.h>
#endif
#include <netinet/in.h>

// AX.25 Libraries
#include <netax25/ax25.h>
#include <netax25/axconfig.h>

#define ax_address_length 7
#define end_address_bit 0x01
#define has_been_repeated_bit 0x80
#define max_ax25_ports 16
#define buffer_size 1500

int main(int argc, char *argv[]) {
  int socket_file_descriptor;
  struct sockaddr socket_address;
  int address_size = sizeof(socket_address);
  int packet_size;
  struct ifreq interface_request;
  int proto = ETH_P_AX25; // ETH_P_AX25 is defined on netax25/ax25.h
  char *port = NULL, *device = NULL;
  unsigned char buffer[buffer_size];

  if (ax25_config_load_ports() == 0)
    printf("axdigi: no AX.25 port data configured\n");
  if (port != NULL) {
    if ((device = ax25_config_get_dev(port)) == NULL) {
      printf("axdigi: invalid port name - %s\n", port);
      return 1;
    }
  }

  if ((socket_file_descriptor = socket(AF_PACKET, SOCK_PACKET, htons(proto))) == -1) {
    perror("socket");
    return 1;
  }
  // printf("Socket has been created\n");

  while(1) {
    address_size = sizeof(socket_address);
    packet_size = recvfrom(socket_file_descriptor, buffer, buffer_size, 0, &socket_address, &address_size);

    if (packet_size == -1) {
      perror("recv");
      return 1;
    }

    if (device != NULL && strcmp(device, socket_address.sa_data) != 0)
      continue;

    strcpy(interface_request.ifr_name, socket_address.sa_data);
    if (ioctl(socket_file_descriptor, SIOCGIFHWADDR, &interface_request) < 0)
      perror("GIFHWADDR");

    if (interface_request.ifr_hwaddr.sa_family == AF_AX25) {
      printf("Got a packet\n");
      continue;
    }
  }
  close(socket_file_descriptor);
  // printf("Socket has been closed\n");
}
