/**
 * @file client.c
 * @author Gurduza Cristian
 * @date 03.12.2023
 * @brief client side source code
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../logger.h"

/* DEFINE zone */

typedef enum status_code
{
   STATUS_FAILURE = -1,
   STATUS_SUCCESS
} st_code;

#define PORT "3940"
#define BLOCK_SIZE 1024 // bytes

/* FUNCTION declaration zone */
char *get_http_req_header()
{
   char *path = "resources/http-header.txt";
   char *http_header = NULL;

   FILE *f = fopen(path, "r");

   if (f == NULL)
   {
      ELOG("File '%s' can't be opened.", path);
      exit(EXIT_FAILURE);
   }

   fseek(f, 0, SEEK_END);
   long length = ftell(f);
   fseek(f, 0, SEEK_SET);

   if (length < BLOCK_SIZE)
   {
      http_header = (char *)malloc(sizeof(char) * BLOCK_SIZE);
      fread(http_header, 1, length, f);
   }

   fclose(f);

   return http_header;
}

int main(int argc, char *argv[])
{
   struct addrinfo hints, *res, *iter_ai;
   int32_t status, nr_bytes;
   int32_t sock_desc;
   char ip_addr[INET6_ADDRSTRLEN * 2];
   char buff[BLOCK_SIZE];
   char *http_header = get_http_req_header();

   // set to (int)0 all the bytes in hints struct
   memset(&hints, 0, sizeof(hints));

   // set hints for getaddrinfo(), which will complete entirely all the addrinfo struct.
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   // complete addrinfo struct by calling getaddrinf()
   status = getaddrinfo("127.0.1.1", PORT, &hints, &res);

   // check if failure
   if (status != STATUS_SUCCESS)
   {
      ELOG("%s", gai_strerror(status));
      exit(status);
   }

   // iterate through linked list of addrinfo and connect to the first one that is working
   for (iter_ai = res; iter_ai != NULL; iter_ai = iter_ai->ai_next)
   {
      void *addr;
      char ipstr[INET_ADDRSTRLEN];

      // get the pointer to the address itself,
      if (iter_ai->ai_family == AF_INET)
      { // IPv4
         struct sockaddr_in *ipv4 = (struct sockaddr_in *)iter_ai->ai_addr;
         addr = &(ipv4->sin_addr);
      }
      // convert the IP to a string and print it:
      inet_ntop(iter_ai->ai_family, addr, ipstr, sizeof ipstr);
      // sprintf(ip_addr, "%s", ipstr);
      strncpy(ip_addr, ipstr, INET6_ADDRSTRLEN);

      sock_desc = socket(iter_ai->ai_family, iter_ai->ai_socktype, iter_ai->ai_protocol);
      if (sock_desc == STATUS_FAILURE)
      {
         WLOG("%s", strerror(errno));
         continue;
      }

      status = connect(sock_desc, iter_ai->ai_addr, iter_ai->ai_addrlen);
      if (status == STATUS_FAILURE)
      {
         WLOG("%s", strerror(errno));
         continue;
      }

      break;
   }

   freeaddrinfo(res);

   // if not connected after iteration, then exit
   if (iter_ai == NULL)
   {
      ELOG("failed to connect");
      exit(EXIT_FAILURE);
   }

   // convert internet address from binary to presentation form
   // inet_ntop(iter_ai->ai_family, get_addr((struct sockaddr *)iter_ai->ai_addr), ip_addr, sizeof(ip_addr));
   ILOG("connected to %s", ip_addr);

   // if ((nr_bytes = recv(sock_desc, buff, BLOCK_SIZE - 1, 0)) == -1)
   // {
   //    ELOG("%s", strerror(errno));
   //    exit(1);
   // }

   // buff[nr_bytes] = '\0';
   // printf("client: received '%s'\n", buff);

   if (nr_bytes = send(sock_desc, http_header, BLOCK_SIZE, 0) == -1)
   {
      ELOG("%s", strerror(errno));
   }

   if (nr_bytes = recv(sock_desc, buff, BLOCK_SIZE, 0) == -1)
   {
      ELOG("%s", strerror(errno));
   }

   buff[nr_bytes] = '\0';
   printf("HTTP response:\n%s", buff);

   free(http_header);
   http_header = NULL;

   close(sock_desc);

   return 0;
}
