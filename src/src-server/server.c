/**
 * @file server.c
 * @author Gurduza Cristian
 * @date 03.12.2023
 * @brief server side source code
 */

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
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
#define BACKLOG 5
#define BLOCK_SIZE 1024
#define RESPONSE_SIZE 1024 * 1024
#define DEFAULT_PAGE "index.html"

/* FUNCTION declaration zone */
static void *get_addr(const struct sockaddr *const sa)
{
   if (sa->sa_family == AF_INET)
   {
      return &(((struct sockaddr_in *)sa)->sin_addr);
   }

   return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

const char *get_file_extension(const char *file_name)
{
   const char *dot = strrchr(file_name, '.');
   if (!dot || dot == file_name)
   {
      return "";
   }
   return dot + 1;
}

const char *get_mime_type(const char *file_ext)
{
   if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0)
   {
      return "text/html";
   }
   else if (strcasecmp(file_ext, "txt") == 0)
   {
      return "text/plain";
   }
   else if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0)
   {
      return "image/jpeg";
   }
   else if (strcasecmp(file_ext, "png") == 0)
   {
      return "image/png";
   }
   else
   {
      return "application/octet-stream";
   }
}

void build_http_response(const char *filename,
                         const char *file_extension,
                         char *response,
                         size_t *response_len)
{

   int fd = open(filename, O_RDONLY);

   // form response for the case if file is not found
   if (fd == -1)
   {
      snprintf(response, RESPONSE_SIZE,
               "HTTP/1.1 404 Not Found\r\n"
               "Content-Type: text/plain\r\n"
               "\r\n"
               "404 Not Found");
      *response_len = strlen(response);
      return;
   }

   // form header of response
   const char *mime_type = get_mime_type(file_extension);
   char *header = (char *)malloc(sizeof(char) * RESPONSE_SIZE);
   snprintf(header, RESPONSE_SIZE,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "\r\n",
            mime_type);

   // get file size
   struct stat file_stat;
   fstat(fd, &file_stat);
   off_t file_size = file_stat.st_size;

   // copy header in response
   *response_len = 0;
   memcpy(response, header, strlen(header));
   *response_len += strlen(header);

   // copy file to response buffer
   ssize_t bytes_read;
   while ((bytes_read = read(fd,
                             response + *response_len,
                             RESPONSE_SIZE - *response_len)) > 0)
   {
      *response_len += bytes_read;
   }
   free(header);
   close(fd);
}

void *handle_client(void *csk_desc)
{
   int32_t client_socket_desc = *(int32_t *)csk_desc;
   char buff[BLOCK_SIZE];

   // receive http request
   ssize_t recv_bytes = recv(client_socket_desc, buff, BLOCK_SIZE * 2, 0);

   if (recv_bytes > 0)
   {
      printf("num_bytes = %ld\n", recv_bytes);
      printf("buff:\n%s\n", buff);
      // check http request syntax, only GET request has got implemented
      regex_t reg_rule;
      regmatch_t matched_subs[2];
      regcomp(&reg_rule, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);

      if (regexec(&reg_rule, buff, 2, matched_subs, 0) == 0)
      {
         ILOG("HTTP request MATCHED the reference pattern.\n");
      }
      else
      {
         ELOG("HTTP request DIDN'T MATCH the reference pattern.\n");
      }

      // extract the url_filename
      buff[matched_subs[1].rm_eo] = '\0';

      char *url_filename = buff + matched_subs[1].rm_so;
      ILOG("url_filename = %s\n", url_filename);

      // route if unspecified file_name
      if (strcmp(url_filename, "") == 0)
      {
         strcpy(url_filename, DEFAULT_PAGE);
      }

      char path[256];

      sprintf(path, "%s%s", "assets/", url_filename);
      ILOG("file path: %s", path);

      // extract the file extension
      char file_ext[32];
      strcpy(file_ext, get_file_extension(url_filename));
      ILOG("file_extension = .%s\n", file_ext);

      // build http response
      size_t response_len;
      char *response = (char *)malloc(sizeof(char) * RESPONSE_SIZE * 20); // 20 MiB
      build_http_response(path, file_ext, response, &response_len);

      ILOG("HTTP response file:\n%s", response);

      send(client_socket_desc, response, response_len, 0);

      free(response);
      regfree(&reg_rule);
   }

   close(client_socket_desc);
   return NULL;
}

int main(int argc, char *argv[])
{
   struct addrinfo hints, *res, *iter_ai;
   struct sockaddr_storage con_addr;
   socklen_t ca_len;
   char ip_addr[INET6_ADDRSTRLEN];
   int32_t status;
   int32_t sock_desc;
   int32_t yes = 1;

   // set to (int)0 all the bytes in hints struct
   memset(&hints, 0, sizeof(hints));

   // set hints for getaddrinfo(), which will complete entirely all the addrinfo struct.
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;

   // complete addrinfo struct by calling getaddrinf()
   status = getaddrinfo(NULL, PORT, &hints, &res);

   // check if failure
   if (status != STATUS_SUCCESS)
   {
      ELOG("%s", gai_strerror(status));
      exit(status);
   }

   // iterate through linked list of addrinfo and connect to the first one that is working
   for (iter_ai = res; iter_ai != NULL; iter_ai = iter_ai->ai_next)
   {
      sock_desc = socket(iter_ai->ai_family, iter_ai->ai_socktype, iter_ai->ai_protocol);
      if (sock_desc == STATUS_FAILURE)
      {
         WLOG("%s", strerror(errno));
         continue;
      }

      status = setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
      if (status == STATUS_FAILURE)
      {
         ELOG("%s", strerror(errno));
         exit(EXIT_FAILURE);
      }

      status = bind(sock_desc, iter_ai->ai_addr, iter_ai->ai_addrlen);
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
      ELOG("failed to bind");
      exit(EXIT_FAILURE);
   }

   status = listen(sock_desc, BACKLOG);
   if (status == STATUS_FAILURE)
   {
      ELOG("%s", strerror(errno));
      exit(EXIT_FAILURE);
   }

   ILOG("waiting for connection");

   // client manager
   while (1)
   {
      // main accept() loop
      int32_t *nsock_desk = (int32_t *)malloc(sizeof(int));
      ca_len = sizeof(con_addr);
      *nsock_desk = accept(sock_desc, (struct sockaddr *)&con_addr, &ca_len);

      if (*nsock_desk == STATUS_FAILURE)
      {
         perror("accept");
         continue;
      }

      inet_ntop(con_addr.ss_family,
                get_addr((struct sockaddr *)&con_addr),
                ip_addr,
                sizeof(ip_addr));

      ILOG("server: got connection from %s", ip_addr);

      pthread_t thread_id;
      pthread_create(&thread_id, NULL, handle_client, (void *)nsock_desk);
      pthread_detach(thread_id);
   }
   return 0;
}
