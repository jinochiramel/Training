//logger.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "logger.h"

FILE *fp ;
static uint32_t SESSION_TRACKER = 0; //Keeps track of session

// Local function proto type 
static char* print_time( char *buf);
// Local function proto type ends

/***************************************************************************
 * name: print_time
 * parameter:
 *      buf: buffer
 * function: Get the time string in buffer to print on the log file
 * **************************************************************************/
static char* print_time( char *buf)
{
    time_t t;
    uint32_t size = 0;

    if(buf == NULL)
    {
        return NULL;
    }

    t = time(NULL); /* get current calendar time */

    char *timestr = asctime( localtime(&t) );
    size = strlen(timestr)+ 1 + 2; //Additional +2 for square braces
    timestr[strlen(timestr) - 1] = 0;  //Getting rid of \n

    snprintf(buf,size,"[%s]", timestr);

    return buf;
}

/***************************************************************************
 * name: log_print
 * parameter:
 *      level: Log level
 *      filename: File name needs to be print
 *      line : Line number needs to be print
 *      fmt: standard fmt for the supporting the print fn
 * function: Print the customized log information in the specified log file
 * **************************************************************************/
void log_print(uint8_t level, char* filename, uint32_t line, char *fmt,...)
{
    va_list list;
    char *out_mesg = NULL;
    int32_t ret;
    char buf[64] = {0};

    if(SESSION_TRACKER > 0)
    {
        fp = fopen ("./log/gcif_test.log","a+");
    }
    else
    {
        fp = fopen ("./log/gcif_test.log","w");
    }

    fprintf(fp,"%s ",print_time(buf));

    switch(level)
    {
        case INFO:
        {
            fprintf(fp,"[%s][line: %d] ",filename,line);
        }
        break;

        case WARN:
        {
            fprintf(fp,"[%s][line: %d] %s ",filename,line,"Warning:");
        }
        break;

        case ERROR:
        {
            fprintf(fp,"[%s][line: %d] %s ",filename,line, "Error:");
        }
        break;
        default:
        {
            fprintf(fp,"[%s][line: %d] ",filename,line);
        }
        break;
    }

    va_start( list, fmt );
    ret = vasprintf(&out_mesg, fmt, list);
    if (ret > 0)
    {
        fprintf(fp,"%s", out_mesg);
    }

    va_end( list );

    if(out_mesg)
    {
        free(out_mesg);
    }

    fputc( '\n', fp );
    SESSION_TRACKER++;
    fclose(fp);
}
