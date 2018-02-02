/* manipmem.c */
#include "manipmem.h"

#include <unistd.h>

#define S_ADDR sizeof (u_int32_t) /* taille d'un void* */

#define B_SIZE P_SIZE/S_ADDR
#define P_MASK (u_int32_t) ~(P_SIZE - 1)

ulong
get_page(int fd, char *buff, ulong addr)
{
  addr &= P_MASK;
  int ret = lseek(fd, addr, SEEK_SET);
  if (ret != addr)
    return 0;
  ret = read(fd, buff, P_SIZE);
  if (ret <= 0)
    return 0;
  return ret;
}

u_int32_t
scan_mem(int fd, ulong start, condition cond, ulong num, char full, ulong limit)
{
  if (!num)
    return 0;
  if (full != 0 && full != 1)
    return 0;
  start &= P_MASK;
  char buff[P_SIZE];
  ulong len;
  u_int32_t addr = 0;
  unsigned char inc;
  u_int32_t b_addr = (u_int32_t) buff;
  if (full)
    inc = 1;
  else
    inc = 4;
      
  for (ulong i = start; i <= limit && num; i += P_SIZE)
  {
    len = get_page(fd, buff, i);
    if (!len)
      continue;
    for (u_int32_t j = 0; j < len - cond.marge && num; j += inc)
	if (cond.cond(buff + j, cond.param))
	  if (!(--num))
	    addr = i + j;
  }
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
	return tmp;
    }
    return 0;
  }
  else
  {
    ulong cp = 1;
    u_int32_t tmp2 = tmp;
    while (tmp2 != addr)
    {
      tmp = tmp2;
      tmp2 = scan_mem(fd, start, cond, cp++,full, addr);
    }
    return tmp;
  }
}

int
write_mem(int fd, ulong p_addr, u_int32_t *buff, ulong size)
{
  ulong ret = lseek(fd, p_addr, SEEK_SET);
  if (ret != p_addr)
    return -2;
  int wr = write(fd, buff, size);
  if (wr != size)
    return -1;
  return 0;
}
