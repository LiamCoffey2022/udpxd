/*
    This file is part of udpxd.

    Copyright (C) 2015-2016 T.v.Dein.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    You can contact me by mail: <tom AT vondein DOT org>.
*/

#include "host.h"

/* fill a generic struct depending of what we already know,
   which is easier to pass between functions,
   maybe v4 or v6, filled from existing structs or from strings,
   which create the sockaddr* structs */
host_t *get_host(char *ip, int port, struct sockaddr_in *v4, struct sockaddr_in6 *v6) {
  host_t *host = malloc(sizeof(host_t));
  memset(host, 0, sizeof(host_t));

  if(ip != NULL) {
    if(is_v6(ip)) {
      struct sockaddr_in6 *tmp =(struct sockaddr_in6 *) &host->ss;
      inet_pton(AF_INET6, ip, (struct in6_addr*)&tmp->sin6_addr);
      
      tmp->sin6_family = AF_INET6;
      tmp->sin6_port = htons( port );

      unsigned int scope = get_v6_scope(ip);
      if (is_linklocal((struct in6_addr*)&tmp->sin6_addr))
        tmp->sin6_scope_id = scope;
      else
        tmp->sin6_scope_id = 0;

    }
    else {
      struct sockaddr_in *tmp = (struct sockaddr_in *)&host->ss;
      memset(tmp, 0, sizeof(struct sockaddr_in));
      tmp->sin_family = AF_INET;
      tmp->sin_addr.s_addr = inet_addr( ip );
      tmp->sin_port = htons( port );

      memcpy(&host->ss, tmp, sizeof(struct sockaddr_in));
    }
  }
  else if(v4 != NULL) {
    memcpy(&host->ss, v4, sizeof(struct sockaddr_in));
  }
  else if(v6 != NULL) {
    memcpy(&host->ss, v6, sizeof(struct sockaddr_in6));
  }
  else {
    fprintf(stderr, "call invalid!\n");
    exit(1); /* shall not happen */
  }

  return host;
}

int is_v6(char *ip) {
  char *IS = strchr(ip, ':');
  if(IS == NULL)
    return 0;
  else
    return 1;
}

/* via http://stackoverflow.com/questions/13504934/binding-sockets-to-ipv6-addresses
   return the interface index (aka scope) of an ipv6 address, which is required
   in order to bind to it.
*/
unsigned get_v6_scope(const char *ip){
  struct ifaddrs *addrs, *addr;
    char ipAddress[NI_MAXHOST];
    uint32_t scope=0;
    int i;
    
    // walk over the list of all interface addresses
    getifaddrs(&addrs);
    for(addr=addrs;addr;addr=addr->ifa_next){
        if (addr->ifa_addr && addr->ifa_addr->sa_family==AF_INET6){ // only interested in ipv6 ones
            getnameinfo(addr->ifa_addr,sizeof(struct sockaddr_in6),ipAddress,sizeof(ipAddress),NULL,0,NI_NUMERICHOST);
            // result actually contains the interface name, so strip it
            for(i=0;ipAddress[i];i++){
                if(ipAddress[i]=='%'){
                    ipAddress[i]='\0';
                    break;
                }
            }
            // if the ip matches, convert the interface name to a scope index
            if(strcmp(ipAddress,ip)==0){
                scope=if_nametoindex(addr->ifa_name);
                break;
            }
        }
    }
    freeifaddrs(addrs);
    return scope;
}

/* this is the contents of the makro IN6_IS_ADDR_LINKLOCAL,
   which doesn't compile, when used directly, for whatever reasons */
int is_linklocal(struct in6_addr *a) {
  return ((a->s6_addr[0] == 0xfe) && ((a->s6_addr[1] & 0xc0) == 0x80));
}

char *host_ip(host_t *host) {
  static char ip[INET6_ADDRSTRLEN];
  if(host->ss.ss_family == AF_INET ){
     struct sockaddr_in *t =(struct sockaddr_in *) &host->ss;
     return inet_ntoa(t->sin_addr); 
  } else if(host->ss.ss_family == AF_INET6 ){
     struct sockaddr_in6 *t =(struct sockaddr_in6 *) &host->ss;
     inet_ntop(AF_INET6, &t->sin6_addr, ip, INET6_ADDRSTRLEN);
     return ip;
  }
  return "unknow family";
}

int host_port(host_t *host) {
  if(host->ss.ss_family == AF_INET ){
     struct sockaddr_in *t =(struct sockaddr_in *) &host->ss;
     return ntohs(t->sin_port);
  } else if(host->ss.ss_family == AF_INET6 ){
     struct sockaddr_in6 *t =(struct sockaddr_in6 *) &host->ss;
     return ntohs(t->sin6_port);
  }
  return 0;
}

void host_dump(host_t *host) {
  fprintf(stderr, "host -   ip: %s\n", host_ip(host));
  fprintf(stderr, "       port: %d\n", host_port(host));
}

void host_clean(host_t *host) {
  free(host);
}
