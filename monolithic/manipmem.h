#ifndef MANIPMEM_H
#define MANIPMEM_H

#include <sys/types.h>

#define LIMIT  0x0fffffff
#define P_SIZE 0x1000             /* size of memory page */

#define ulong unsigned long

typedef int (*fun_cond)(char *buffer, void *param); 

typedef struct condition_t{
  fun_cond cond;
  ulong marge;
  void *param;
} condition;

int write_mem(int fd, ulong p_addr, u_int32_t *buff, ulong size);
ulong get_page(int fd, char *buff, ulong addr);

u_int32_t scan_mem(int fd, ulong start, condition cond,
		   ulong num, char full, ulong limit);
u_int32_t find_lower_near(int fd, u_int32_t addr, condition cond, char full);

#endif
