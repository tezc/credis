#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "credis.h"
#include "sc_sock.h"
#include "sc_uri.h"

int credis_create(credis **c, struct credis_config *config)
{
    struct credis *client = calloc(1, sizeof(*client));
    if (!client) {
        return -1;
    }

    client->timeout_millis = config->timeout_millis;
    client->url = sc_uri_create(config->url);
    client->on_connect = config->on_connect;
    client->on_disconnect = config->on_disconnect;
    client->on_response = config->on_response;
    client->eventloop_add = config->eventloop_add;
    client->eventloop_del = config->eventloop_del;

    sc_sock_init(&client->sock, 0, false, SC_SOCK_INET);

    int rc = sc_sock_connect(&client->sock, client->url->host, client->url->port, NULL, NULL);
    if (rc < 0) {
        if (errno == EAGAIN) {
            client->connecting = true;
            config->eventloop_add(client, false, true);
        } else {
            return -1;
        }
    }

    *c = client;
    return 0;
}


int credis_on_read(credis *c)
{
    char buf[1024];

    int ret = sc_sock_recv(&c->sock, buf, 1024, 0);
    if (ret <= 0) {
        if (errno == EAGAIN) {
            return 0;
        }

        sc_sock_term(&c->sock);
        printf("shutdonw \n");
        return -1;
    }

    c->on_response(c, buf);
    return 0;
}

int credis_on_write(credis *c)
{
    int ret;

    if (c->connecting) {
        c->eventloop_del(c, false, true);
        c->eventloop_add(c, true, false);
        c->connecting = false;
        c->on_connect(c);
        return 0;
    }

 retry:
    ret = sc_sock_send(&c->sock, c->buf, c->buf_count, 0);
    if (ret < 0) {
        if (errno == EAGAIN) {
            c->eventloop_add(c, false, true);
            return 0;
        }

        sc_sock_term(&c->sock);
        printf("shutdonw \n");
        return -1;
    }

    c->buf_count -= ret;
    if (c->buf_count) {
        goto retry;
    }

    return 0;
}

int credis_send(credis *c, const char *command)
{
    int ret;

    const char *s = "*2\r\n$3\r\nGET\r\n$1\r\nx\r\n";

    memcpy(c->buf, "*2\r\n$3\r\nGET\r\n$1\r\nx\r\n", strlen(s));
    c->buf_count = strlen(s);

retry:
    ret = sc_sock_send(&c->sock, c->buf, c->buf_count, 0);
    if (ret < 0) {
        if (errno == EAGAIN) {
            c->eventloop_add(c, false, true);
            return 0;
        }

        sc_sock_term(&c->sock);
        printf("shutdonw \n");
        return -1;
    }

    c->buf_count -= ret;
    if (c->buf_count) {
        goto retry;
    }

    return 0;
}
