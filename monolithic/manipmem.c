/* manipmem.c */
#include "manipmem.h"

#include <stdlib.h>
#include <unistd.h>

#define S_ADDR sizeof (u_int32_t) /* taille d'un void* */

#define B_SIZE P_SIZE/S_ADDR
#define P_MASK ~(P_SIZE - 1)

//#define BY_HEAP

ulong
get_page(int fd, char *buff, ulong addr)
{
  addr &= P_MASK;
  int ret = lseek(fd, addr, SEEK_SET);
  if (ret != addr)
    return 0;
  ret = read(fd, buff, P_SIZE);
  return (ret <= 0) ? 0 : ret;
}

u_int32_t
scan_mem(int fd, ulong start, condition cond, ulong num, char full, ulong limit)
{
  if (!num || (full != 0 && full != 1) ||
      (limit < start) || (limit - P_SIZE < 0))
    return 0;
  
#ifdef BY_HEAP
  char *buff = malloc(sizeof(char) * P_SIZE);
  if (buff == NULL)
    return 0;
#else
  char buff[P_SIZE];
#endif
  ulong len;
  u_int32_t addr = 0;
  unsigned char inc = (full ? 1 : 4);
  for (ulong i = start; (i < limit) && num; i += P_SIZE)
  {
    len = get_page(fd, buff, i);
    if (!len || (len - cond.marge < 0))
      continue;
    len -= cond.marge;
    for (u_int32_t j = (i ? 0 : (start & 0xfff)) ; (j < len) && num; j += inc)
      if (cond.cond(buff + j, cond.param))
	  if (!(--num))
	    addr = i + j;
  }
#ifdef BY_HEAP
  free(buff);
#endif
  return addr;
}

u_int32_t
find_lower_near(int fd, u_int32_t addr, condition cond, char full)
{
  long int start = addr & P_MASK;
  u_int32_t tmp = scan_mem(fd, start, cond, 1, full, addr);
  if (tmp == 0 || tmp >= addr)
  {
    while (start >= 0)
    {
      tmp = start;
      start -= P_SIZE;
      tmp = scan_mem(fd, start, cond, 1, full, tmp);
      if (tmp)
        break;
    }
  }
  if (start < 0)
    return 0;
  ulong cp = 1;
  u_int32_t tmp2 = tmp;
  while (tmp2 > 0)
  {
    tmp = tmp2;
    tmp2 = scan_mem(fd, start, cond, cp++, full, addr);
  }
  return tmp;
}

int
write_mem(int fd, ulong p_addr, char *buff, ulong size)
{
  ulong ret = lseek(fd, p_addr, SEEK_SET);
  if (ret != p_addr)
    return -2;
  int wr = write(fd, buff, size);
  return (wr != size) ? -1 : 0;
}
