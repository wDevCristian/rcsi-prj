/**
 * @file logger.c
 * @author Cristian Gurduza
 * @date 07.11.2023
 * @copyright public use.
 */

#include "logger.h"
#include <stdlib.h>
#include <time.h>

static int __fd = -1;

char *get_current_datetime()
{
   char *currentTime = (char *)malloc(sizeof(char) * 20);
   time_t now;
   struct tm *local;

   now = time(NULL);
   local = localtime(&now);

   strftime(currentTime, 20, "%d.%m.%Y %H:%M:%S", local);

   return currentTime;
}

static int create_file_descriptor()
{
   int file_descriptor = open("src/logs/"__lf, O_WRONLY | O_APPEND | O_CREAT, 0644);

   if (file_descriptor < 0)
   {
      perror("Logger file descriptor hasn't been created.\n");
   }

   return file_descriptor;
}

int get_file_descriptor()
{
   if (__fd < 0)
   {
      __fd = create_file_descriptor();
   }

   return __fd;
}