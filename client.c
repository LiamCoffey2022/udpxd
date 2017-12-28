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

#include "client.h"
#include "log.h"

void client_del(client_t *client) {
  HASH_DEL(clients, client);
}

void client_add(client_t *client) {
  HASH_ADD(hh, clients, src, sizeof(host_t), client);
}

client_t *client_find_src(host_t *src) {
  client_t *client= NULL;
  HASH_FIND(hh, clients, src, sizeof(host_t), client);
  return client; /*  maybe NULL! */
}

void client_seen(client_t *client) {
  client->lastseen = (long)time(0);
}

client_t *client_new(int fd, host_t *src, host_t *dst) {
  client_t *client = malloc(sizeof(client_t));
  client->src = *src;
  client->dst = *dst;
  client->fd = fd;
  client_seen(client);
  return client;
}

void client_close(client_t *client) {
  client_del(client);
  close(client->fd);
  free(client);
}

void client_clean(int asap) {
  uint32_t now = (long)time(0);
  uint32_t diff;
  client_t *current;
  client_iter(clients, current) {
    diff = now - current->lastseen;
    if(diff >= MAXAGE || asap) {
      verbose("closing socket %s:%d for client %s:%d (aged out after %d seconds)\n",
              host_ip(&current->src), host_port(&current->src), host_ip2(&current->dst), host_port(&current->dst), MAXAGE);
      client_close(current);
    }
  }
}

