#ifndef LOGGER_H_
#define LOGGER_H_

#define ERROR   2
#define WARN    1
#define INFO    0

/***************************************************************************
 * name: log_print
 * parameter:
 *      level: Log level
 *      filename: File name needs to be print
 *      line : Line number needs to be print
 *      fmt: standard fmt for the supporting the print fn
 * function: Print the customized log information in the specified log file
 * **************************************************************************/
void log_print(uint8_t level, char* filename, uint32_t line, char *fmt,...);

#define LOG_PRINT(...) log_print(INFO, __FILE__, __LINE__, __VA_ARGS__ )
#define LOG_ERROR(...) log_print(ERROR, __FILE__, __LINE__, __VA_ARGS__ )
#define LOG_WARN(...) log_print(WARN, __FILE__, __LINE__, __VA_ARGS__ )

#endif //LOGGER_H
