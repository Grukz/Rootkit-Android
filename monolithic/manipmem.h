#ifndef MANIPMEM_H
#define MANIPMEM_H

#include <sys/types.h>

#define LIMIT  0xffffffff
#define P_SIZE 0x1000             /* size of memory page */

#define ulong u_int32_t

typedef int (*fun_cond)(char *buffer, void *param); 

typedef struct condition_t{
  fun_cond cond;
  ulong marge;
  void *param;
} condition;

int write_mem(int fd, ulong p_addr, char *buff, ulong size);

/* fill the buff with the page of addr
   return the size fill, if we have an error this will return 0 */
ulong get_page(int fd, char *buff, ulong addr);

/* return 0 if don't find */
u_int32_t scan_mem(int fd, ulong start, condition cond,
		   ulong num, char full, ulong limit);
u_int32_t find_lower_near(int fd, u_int32_t addr, condition cond, char full);

#endif
