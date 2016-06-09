#ifndef __LOG_H_
#define __LOG_H_

#ifdef VERBOSE
#define Log(args...) fprintf(stderr, args)
#else
#define Log(args...)
#endif

#endif  // __LOG_H_
