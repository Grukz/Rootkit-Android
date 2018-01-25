Android r00tkit
================

End of studies' [project](https://mastercsi.labri.fr/wp-content/uploads/2017/12/PER18.pdf) based on "Android platform based linux kernel rootkit".


Environment Specs
=================
* Ubuntu 12.04
* [Android SDK](https://developer.android.com/studio/index.html#command-tools)
* Kernel tree from [here](https://android.googlesource.com/kernel/goldfish)
* Using Android NDK / Google toolchain


Flash the Kernel
================

There are two version presented to flash the kernel:

Nillyr Version
--------------
* Create a device with avdmanager from android sdk (e.g: Nexus S with Android 2.3)
* Run the emulator: `emulator @Nexus_S -show-kernel`
* Extract the config from the emulated device `adb pull /proc/config.gz .`
* Extract the config file
* Edit the file for modules support. Change '# CONFIG_MODULES is not set' to CONFIG_MODULES=y
* Compile Android kernel with modules support and your new .config
* Run the emulator with the new zImage: `emulator @Nexus_S -kernel path/to/zImage -show-kernel -verbose`

Note: when compiling you may have to '[N/y/?]'.
```bash
Forced module loading (MODULE_FORCE_LOAD) [N/y/?] (NEW) y
Module unloading (MODULE_UNLOAD) [N/y/?] (NEW) y
Forced module unloading (MODULE_FORCE_UNLOAD) [N/y/?] (NEW) y
Module versioning support (MODVERSIONS) [N/y/?] (NEW) y
Source checksum for all modules (MODULE_SRCVERSION_ALL) [N/y/?] (NEW) y
```
Everything else is 'N'

