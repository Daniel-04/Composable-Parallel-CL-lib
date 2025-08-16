#ifndef SCAN_H_
#define SCAN_H_

#include "cl_utils.h"

extern const char *_scan_fmt;
char *get_scan (const char *dtype, const char *op1);
unsigned long long scan (const char *op1, array A, cl_event *event);
#define _SCAN_ONE(op1, A) scan (op1, A, NULL);
#define _SCAN_TWO(op1, A, event) scan (op1, A, event)
#define SCAN(...) _GETM_THREE (__VA_ARGS__, _SCAN_TWO, _SCAN_ONE) (__VA_ARGS__)

#endif // SCAN_H_
