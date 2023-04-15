#ifndef CREDIS_CREDIS_H
#define CREDIS_CREDIS_H

#include <stdint.h>
#include <stdbool.h>
#include "sc_sock.h"


typedef struct credis credis;

struct credis {
    bool connecting;
    struct sc_uri *url;
    uint32_t timeout_millis;

    struct sc_sock sock;

    char buf[1024];
    int buf_count;

    void (*on_connect) (credis *c);
    void (*on_disconnect) (credis *c);
    void (*on_response) (credis *c, const char *str);

    int (*eventloop_add) (credis *c, bool read, bool write);
    int (*eventloop_del) (credis *c, bool read, bool write);
};


struct credis_config {
    const char *url;
    uint32_t timeout_millis;


    void (*on_connect) (credis *c);
    void (*on_disconnect) (credis *c);
    void (*on_response) (credis *c, const char *str);

    int (*eventloop_add) (credis *c, bool read, bool write);
    int (*eventloop_del) (credis *c, bool read, bool write);
};

int credis_create(credis **c, struct credis_config *config);

int credis_on_read(credis *c);
int credis_on_write(credis *c);

int credis_send(credis *c, const char *command);

#endif
