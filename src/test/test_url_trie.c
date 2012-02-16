
#include "hpdaemon/url_trie.h"
#include "include/hpd_error.h"
#include "utest.h"

int handler_run = 0;

size_t 
reqHandl ( char* buffer, size_t max_buffer_size, int *http_ret_code, int argc, 
           char **argv, char *req_body, void *data_ptr) 
{
  handler_run = 1;
}


int
main (void)
{
  UrlTrieElement* head;
  int* data_ptr = (int*)malloc(sizeof(int));
  *data_ptr = 5;

  int test_results = 0;

  run_test("Creating head: ", (head = create_url_trie_element (NULL, NULL)) != NULL);

  run_test("Destroy head: ", destroy_url_trie_element(head) == 0);

  run_test("Creating head: ", (head = create_url_trie_element (NULL, NULL)) != NULL);

  int i;
  run_test("Regestering Url: ", (i = register_url ( head, "/dev/asdf", NULL, &reqHandl, NULL, (void*)4, data_ptr)) == 0);

  RequestContainer* reqCon;

  run_test("Creating Request Container: ", ((reqCon = create_request_container()) != NULL));
  run_test("Destroy Request Container: ", ((destroy_request_container(reqCon)) == 0));
  run_test("Creating Request Container: ", ((reqCon = create_request_container()) != NULL));

  run_test("Lookup registered URL: ", (i = lookup_url( head, "/dev/asdf", "PUT", reqCon)) == 0);

  run_test("Correct handler: ", reqCon->req_handler == &reqHandl);
  run_test("Correct data_ptr", reqCon->data_ptr == data_ptr);

  reqCon->req_handler(NULL, 1, NULL, 0, NULL, NULL, NULL);
  run_test("Handler run: ", (handler_run == 1));

  run_test("Lookup registered URL unknown method: ", (i = lookup_url( head, "/dev/asdf", "NEW_METHOD", reqCon)) == HPD_E_URL_METHOD_NOT_IMPLEMENTED);
  run_test("Getting unregistered URL: ", (i = lookup_url( head, "/dev/fisk", "PUT", reqCon)) != 0);


  run_end();

  return test_results;
}

