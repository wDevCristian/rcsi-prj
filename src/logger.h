/**
 * @file logger.h
 * @author Cristian Gurduza
 * @date 07.11.2023
 * @copyright public use.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define __lf "logger.txt" // filename of the logger file, which will be found on the 'logs/'

#ifndef __lf
#define __lf "logger.txt" // in case logger file doesn't have a filename, define a default 'logger.txt'
#endif

#define __log_console // print logs in console
#define __log_file    // print logs in file found of path 'logs/(__lf)'

static char __msg[256];
#if (defined(__log_console) && (defined(__log_file) && defined(__lf))) // print in cli and file
#define INFO_f "INFO"
#define ERROR_f "ERROR"
#define WARNING_f "WARNING"
#define DEBUG_f "DEBUG"
#define INFO_c "\033[1;49;97mINFO\033[0m"
#define ERROR_c "\033[1;49;91mERROR\033[0m"
#define WARNING_c "\033[1;38;5;202mWARNING\033[0m"
#define DEBUG_c "\033[1;49;32mDEBUG\033[0m"
#define ILOG(fmt, ...)                                                                                                   \
   fprintf(stderr, "[" INFO_c "]    [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__); \
   sprintf(__msg, "[" INFO_f "]    [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__);  \
   write(get_file_descriptor(), __msg, strlen(__msg))

#define ELOG(fmt, ...)                                                                                                   \
   fprintf(stderr, "[" ERROR_c "]   [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__); \
   sprintf(__msg, "[" ERROR_f "]   [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__);  \
   write(get_file_descriptor(), __msg, strlen(__msg))

#define WLOG(fmt, ...)                                                                                                   \
   fprintf(stderr, "[" WARNING_c "] [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__); \
   sprintf(__msg, "[" WARNING_f "] [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__);  \
   write(get_file_descriptor(), __msg, strlen(__msg))

#define DLOG(fmt, ...)                                                                                                   \
   fprintf(stderr, "[" DEBUG_c "]   [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__); \
   sprintf(__msg, "[" DEBUG_f "]   [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__);  \
   write(get_file_descriptor(), __msg, strlen(__msg))
#elif (defined(__log_console)) // print in cli only
#define INFO_c "\033[1;49;97mINFO\033[0m"
#define ERROR_c "\033[1;49;91mERROR\033[0m"
#define WARNING_c "\033[1;38;5;202mWARNING\033[0m"
#define DEBUG_c "\033[1;49;32mDEBUG\033[0m"
#define ILOG(fmt, ...) fprintf(stderr, "[" INFO_c "]    [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__)
#define ELOG(fmt, ...) fprintf(stderr, "[" ERROR_c "]   [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__)
#define WLOG(fmt, ...) fprintf(stderr, "[" WARNING_c "] [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__)
#define DLOG(fmt, ...) fprintf(stderr, "[" DEBUG_c "]   [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__)
#elif (defined(__log_file) && defined(__lf)) // print in file only
#define INFO_f "INFO"
#define ERROR_f "ERROR"
#define WARNING_f "WARNING"
#define DEBUG_f "DEBUG"
#define ILOG(fmt, ...)                                                                                                  \
   sprintf(__msg, "[" INFO_f "]    [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__); \
   write(get_file_descriptor(), __msg, strlen(__msg))
#define ELOG(fmt, ...)                                                                                                  \
   sprintf(__msg, "[" ERROR_f "]   [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__); \
   write(get_file_descriptor(), __msg, strlen(__msg))
#define WLOG(fmt, ...)                                                                                                  \
   sprintf(__msg, "[" WARNING_f "] [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__); \
   write(get_file_descriptor(), __msg, strlen(__msg))
#define DLOG(fmt, ...)                                                                                                  \
   sprintf(__msg, "[" DEBUG_f "]   [%s] %s:%d - " fmt "\n", get_current_datetime(), __FILE__, __LINE__, ##__VA_ARGS__); \
   write(get_file_descriptor(), __msg, strlen(__msg))
#endif

/* External functions from logger.c */

char *get_current_datetime();
int get_file_descriptor();

#endif