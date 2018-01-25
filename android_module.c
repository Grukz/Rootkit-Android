/*
  Hadrien Amrouche, Nicolas Grellety, Bowen Liu, Roland Mounier
  
  (c) 2018

  End of studies' project based on "Android platform based linux kernel rootkit".
*/

#include <linux/kernel.h>
#include <linux/module.h>

static int __init
root_start(void) 
{
  printk(KERN_INFO "Hello word!\n");
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

