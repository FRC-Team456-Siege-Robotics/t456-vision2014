
#include <stdio.h>
#include <string.h>
#include "mongoose.h"

extern char target_message[100];
extern int  target_message_length;

// This function will be called by mongoose on every new request.
static int begin_request_handler(struct mg_connection *conn) {

  // Send HTTP reply to the client
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %d\r\n"        // Always set Content-Length
            "\r\n"
            "%s",
            target_message_length, target_message);

  // Returning non-zero tells mongoose that our function has replied to
  // the client, and mongoose should not send client any more data.
  return 1;
}

struct mg_context *ctx;

void T456_start_server(void) {
  struct mg_callbacks callbacks;

  target_message_length =
     snprintf(target_message, sizeof(target_message),
     "00,000000,000000,0000,000000");

  // List of options. Last element must be NULL.
  const char *options[] = {"listening_ports", "8080", NULL};

  // Prepare callbacks structure. We have only one callback, the rest are NULL.
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.begin_request = begin_request_handler;

  // Start the web server.
  ctx = mg_start(&callbacks, NULL, options);

  return;
}

void T456_stop_server(void) {

  // Stop the server.
  mg_stop(ctx);

  return;
}
