#ifndef POOL_TIMESTAMP_H
#define POOL_TIMESTAMP_H
#include "pool.h"
#include "pool_proto_modules.h"
#include "parser/nodes.h"

char *rewrite_timestamp(POOL_CONNECTION_POOL *backend, Node *node, bool rewrite_to_params, Portal *portal);
char *bind_rewrite_timestamp(POOL_CONNECTION_POOL *backend, Portal *portal, const char *orig_msg, int *len);

#endif /* POOL_TIMESTAMP_H */
