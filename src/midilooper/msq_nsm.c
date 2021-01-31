#include "msq_nsm_inc.h"
#include <sys/types.h>
#include <unistd.h>

static msq_nsm_client_t *g_msq_nsm_client = NULL;

void msq_nsm_free(void)
{
  lo_server_thread_free(g_msq_nsm_client->srv_th);
  lo_address_free(g_msq_nsm_client->nsm_srv_addr);
  g_msq_nsm_client = NULL;
}

void msq_nsm_quit_signal_handler(int signum)
{
  sigaction(SIGTERM, &(g_msq_nsm_client->old_action), NULL);
  msq_nsm_free();
  g_msq_nsm_client->quit_cb(g_msq_nsm_client->cb_arg);
  /* msq_logabort("NSM quit callback is not quitting"); */
}

void msq_nsm_error(int num, const char *msg, const char *where)
{
  pbt_logmsg("liblo server error %d in path %s: %s\n", num, where, msg);
  exit(1);
}

int msq_nsm_generic_handler(const char *path,
                            const char *types,
                            lo_arg **argv,
                            int argc,
                            void *data,
                            void *unused)
{
  int i;

  pbt_logmsg("\n*** DEBUG *** %s", path);
  for (i = 0; i < argc; i++) {
    pbt_logmsg(" %c:", types[i]);
    lo_arg_pp((lo_type)types[i], argv[i]);
  }
  pbt_logmsg("\n");

  /* tmp */
  /* abort(); */

  return 1;
}

#define MSQ_NSM_ERR_GENERAL -1
#define MSQ_NSM_ERR_INCOMPATIBLE_API -2
#define MSQ_NSM_ERR_BLACKLISTED -3
#define MSQ_NSM_ERR_LAUNCH_FAILED -4
#define MSQ_NSM_ERR_NO_SUCH_FILE -5
#define MSQ_NSM_ERR_NO_SESSION_OPEN -6
#define MSQ_NSM_ERR_UNSAVED_CHANGES -7
#define MSQ_NSM_ERR_NOT_NOW -8
#define MSQ_NSM_ERR_BAD_PROJECT -9
#define MSQ_NSM_ERR_CREATE_FAILED -10

#include <string.h>

int msq_nsm_error_handler(const char *path,
                          const char *types,
                          lo_arg **argv,
                          int argc,
                          void *data,
                          void *unused)
{
  /* /error s:"/nsm/server/announce" i:error_code s:error_message */
  if (strcmp("/nsm/server/announce", &(argv[0]->s)) == 0)
    {
      pbt_logmsg("NSM announce error code %d: %s",
                 argv[1]->i,
                 &(argv[2]->s));
      g_msq_nsm_client->state = MSQ_NSM_ANNOUNCE_ERROR;
    }
  return 0;
}

int msq_nsm_reply_handler(const char *path,
                          const char *types,
                          lo_arg **argv,
                          int argc,
                          void *data,
                          void *unused)
{
  /* /reply s:"/nsm/server/announce" s:message s:name_of_session_manager s:capabilities */
  if (strcmp("/nsm/server/announce", &(argv[0]->s)) == 0)
    {
      pbt_logmsg("Receive a msg from %s (capabilities:%s)\n%s\n",
                 &(argv[2]->s),
                 &(argv[3]->s),
                 &(argv[1]->s));
      g_msq_nsm_client->manager_name = strdup(&(argv[2]->s));
      g_msq_nsm_client->state = MSQ_NSM_CONNECTED;
    }
  else
    pbt_logmsg("Unhandled path");
  return 0;
}

int msq_nsm_open_handler(const char *path,
                          const char *types,
                          lo_arg **argv,
                          int argc,
                          void *data,
                          void *unused)
{
  /* /nsm/client/open s:path_to_instance_specific_project s:display_name s:client_id */
  g_msq_nsm_client->project_path = strdup(&(argv[0]->s));
  g_msq_nsm_client->display_name = strdup(&(argv[1]->s));
  g_msq_nsm_client->clien_id = strdup(&(argv[2]->s));

  if (g_msq_nsm_client->open_cb(&(argv[0]->s),
                                &(argv[2]->s),
                                g_msq_nsm_client->cb_arg) == MSQ_TRUE)
    /* /reply "/nsm/client/open" s:message */
    lo_send_from(g_msq_nsm_client->nsm_srv_addr,
                 g_msq_nsm_client->srv,
                 LO_TT_IMMEDIATE,
                 "/reply",
                 "ss",
                 "/nsm/client/open",
                 "File opened successfully");
  else
    /* /error "/nsm/client/open" i:error_code s:message */
    lo_send_from(g_msq_nsm_client->nsm_srv_addr,
                 g_msq_nsm_client->srv,
                 LO_TT_IMMEDIATE,
                 "/error",
                 "sis",
                 "/nsm/client/open",
                 MSQ_NSM_ERR_GENERAL,
                 "Error while opening file");
  return 0;
}

int msq_nsm_save_handler(const char *path,
                         const char *types,
                         lo_arg **argv,
                         int argc,
                         void *data,
                         void *unused)
{
  if (g_msq_nsm_client->save_cb(g_msq_nsm_client->cb_arg) == MSQ_TRUE)
    /* /reply "/nsm/client/save" s:message */
    lo_send_from(g_msq_nsm_client->nsm_srv_addr,
                 g_msq_nsm_client->srv,
                 LO_TT_IMMEDIATE,
                 "/reply",
                 "ss",
                 "/nsm/client/save",
                 "File saved successfully");
  else
    /* /error "/nsm/client/save" i:error_code s:message */
    lo_send_from(g_msq_nsm_client->nsm_srv_addr,
                 g_msq_nsm_client->srv,
                 LO_TT_IMMEDIATE,
                 "/error",
                 "sis",
                 "/nsm/client/save",
                 MSQ_NSM_ERR_GENERAL,
                 "Error while saving file");
  return 0;
}

#define MSQ_NSM_API_MAJOR 1
#define MSQ_NSM_API_MINOR 1

msq_bool_t msq_nsm_start_client(msq_nsm_client_t *msq_nsm_client,
                                const char *nsm_srv_url,
                                const char *app_name,
                                const char *exe_name,
                                msq_nsm_open_cb_t open_cb,
                                msq_nsm_quit_cb_t quit_cb,
                                msq_nsm_save_cb_t save_cb,
                                void *cb_arg)
{
  struct sigaction new_action = {};
  lo_address       nsm_srv_addr;

  if (g_msq_nsm_client != NULL)
    {
      pbt_logmsg("An nsm client is already launched.");
      return MSQ_FALSE;
    }

  nsm_srv_addr = lo_address_new_from_url(nsm_srv_url);
  pbt_logmsg("lo_addr: %p", nsm_srv_addr);
  if (nsm_srv_addr == NULL)
    {
      pbt_logmsg("Unable to connect url \"%s\".", nsm_srv_url);
      return MSQ_FALSE;
    }

  g_msq_nsm_client = msq_nsm_client;
  g_msq_nsm_client->nsm_srv_addr = nsm_srv_addr;

  g_msq_nsm_client->srv_th = lo_server_thread_new(NULL, msq_nsm_error);
  g_msq_nsm_client->srv =
    lo_server_thread_get_server(g_msq_nsm_client->srv_th);
  pbt_logmsg("server thread: %p", g_msq_nsm_client->srv_th);

  g_msq_nsm_client->quit_cb = quit_cb;
  g_msq_nsm_client->open_cb = open_cb;
  g_msq_nsm_client->save_cb = save_cb;
  g_msq_nsm_client->cb_arg = cb_arg;

  new_action.sa_handler = msq_nsm_quit_signal_handler;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;
  sigaction(SIGTERM, &new_action, &(g_msq_nsm_client->old_action));

  lo_server_thread_add_method(g_msq_nsm_client->srv_th,
                              NULL,
                              NULL,
                              msq_nsm_generic_handler,
                              NULL);
  lo_server_thread_add_method(g_msq_nsm_client->srv_th,
                              "/error",
                              "sis",
                              msq_nsm_error_handler,
                              NULL);
  lo_server_thread_add_method(g_msq_nsm_client->srv_th,
                              "/reply",
                              "ssss",
                              msq_nsm_reply_handler,
                              NULL);
  lo_server_thread_add_method(g_msq_nsm_client->srv_th,
                              "/nsm/client/open",
                              "sss",
                              msq_nsm_open_handler,
                              NULL);
  lo_server_thread_add_method(g_msq_nsm_client->srv_th,
                              "/nsm/client/save",
                              "",
                              msq_nsm_save_handler,
                              NULL);

  g_msq_nsm_client->pid = getpid();

  g_msq_nsm_client->state = MSQ_NSM_WAIT_CONNECTION;

  if (lo_server_thread_start(g_msq_nsm_client->srv_th) != 0)
    {
      msq_nsm_free();
      return MSQ_FALSE;
    }

  /* /nsm/server/announce s:application_name s:capabilities s:executable_name i:api_version_major i:api_version_minor i:pid */
  if (lo_send_from(g_msq_nsm_client->nsm_srv_addr,
                   g_msq_nsm_client->srv,
                   LO_TT_IMMEDIATE,
                   "/nsm/server/announce",
                   "sssiii",
                   app_name,
                   "",
                   exe_name,
                   MSQ_NSM_API_MAJOR,
                   MSQ_NSM_API_MINOR,
                   g_msq_nsm_client->pid) == -1)
    {
      msq_nsm_free();
      pbt_logmsg("Announce send failed");
      return MSQ_FALSE;
    }
  unsigned int timeout_connection_ms = 2000;
  while (g_msq_nsm_client->state == MSQ_NSM_WAIT_CONNECTION &&
         timeout_connection_ms > 0)
    {
      usleep(1000);
      timeout_connection_ms--;
    }

  if (g_msq_nsm_client->state != MSQ_NSM_CONNECTED)
    {
      msq_nsm_free();
      pbt_logmsg("Timeout: unable to connect nsm server %s", nsm_srv_url);
      return MSQ_FALSE;
    }
  return MSQ_TRUE;
}
