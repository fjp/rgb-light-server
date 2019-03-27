/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common/cs_dbg.h"

#include "mgos.h"
#include "mongoose.h"
#include "mgos_http_server.h"

#include "mgos_neopixel.h"
#include "mgos_timers.h"

#include <stdio.h> // Included already -> could be omitted

#define PIN 4
#define NUM_PIXELS 1
#define PIXEL 0
#define ORDER MGOS_NEOPIXEL_ORDER_RGB

struct mg_connection *server = NULL; 

struct mgos_neopixel *s_strip = NULL;

static void set_pixel(int r, int g, int b) {
  int pixel = 0;
  mgos_neopixel_clear(s_strip);
  mgos_neopixel_set(s_strip, pixel, r, g, b);
  mgos_neopixel_show(s_strip);
  LOG(LL_INFO, ("%3d %3d %3d %3d", pixel, r, g, b));
}

static void light_handler(struct mg_connection *server, int ev, void *p,
                        void *user_data) {
  (void) p;
  LOG(LL_INFO, ("Light requested"));
  LOG(LL_INFO, ("ev=%i", ev));

  struct http_message *hm = (struct http_message *) p;
  LOG(LL_INFO, ("[%.*s] requested", (int) hm->uri.len, hm->uri.p));
  
  struct mg_str strQuery = hm->query_string;
  LOG(LL_INFO, ("[%.*s] query_string", (int) hm->query_string.len, strQuery.p));

  if (ev != MG_EV_HTTP_REQUEST) return;
  LOG(LL_INFO, ("Light requested"));

  char *pcRed = "r"; 
  char *pcGreen = "g";
  char *pcBlue = "b";
  const size_t nLen = 5;
  char acRed[nLen];
  char acGreen[nLen];
  char acBlue[nLen];
  int nLenRed = mg_get_http_var(&strQuery, pcRed, acRed, nLen);
  int nLenGreen = mg_get_http_var(&strQuery, pcGreen, acGreen, nLen);
  int nLenBlue = mg_get_http_var(&strQuery, pcBlue, acBlue, nLen);

  LOG(LL_INFO, ("Parameter lengths: (r:%i), (g:%i), (b:%i)", nLenRed, nLenGreen, nLenBlue));
  LOG(LL_INFO, ("Parameters (r:%.*s), (g:%.*s), (b:%.*s)", (int) nLenRed, acRed, (int) nLenGreen, acGreen, (int) nLenBlue, acBlue));
  int nRed, nGreen, nBlue;
  sscanf(acRed, "%d", &nRed);
  sscanf(acGreen, "%d", &nGreen);
  sscanf(acBlue, "%d", &nBlue);
  LOG(LL_INFO, ("Parameter converted to int (r:%d), (g:%d), (b:%d)", nRed, nGreen, nBlue));

  set_pixel(nRed, nGreen, nBlue);

  mg_send_response_line(server, 200,
                        "Content-Type: text/html\r\n");
  mg_printf(server, "%s\r\n", "Fooooo");
  //server->flags |= (MG_F_SEND_AND_CLOSE | MGOS_F_RELOAD_CONFIG);
  server->flags |= (MG_F_SEND_AND_CLOSE);
  (void) user_data;
}

enum mgos_app_init_result mgos_app_init(void) {
  LOG(LL_INFO, ("Hi there"));

  /// Setting up http server
  LOG(LL_INFO, ("Setting up http server"));
  server = mgos_get_sys_http_server();

  /// Set endpoints
  mgos_register_http_endpoint("/light", light_handler, NULL);

  /// Initialize neopixel
  LOG(LL_INFO, ("Initialize neopixel"));
  s_strip = mgos_neopixel_create(PIN, NUM_PIXELS, ORDER);
  if (s_strip)
  {
      LOG(LL_INFO, ("Neopixel initialized, setting color 255, 255, 0"));
      int r = 0;
      int g = 255;
      int b = 255;
      set_pixel(r, g, b);
  } 
  else
  {
      LOG(LL_INFO, ("Neopixel not initialized"));
  }

  return MGOS_APP_INIT_SUCCESS;
}
