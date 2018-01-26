# Android r00tkit

End of studies' [project](https://mastercsi.labri.fr/wp-content/uploads/2017/12/PER18.pdf) based on "Android platform based linux kernel rootkit".

# Environment Specs

- Ubuntu 12.04
- [Android SDK](https://developer.android.com/studio/index.html#command-tools)
- Kernel tree from [here](https://android.googlesource.com/kernel/goldfish)
- Using Android NDK / [Google toolchain](https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6)

# Flash the Kernel

There are two version presented to flash the kernel:

## N00d1e5 Version

- Create a device with avdmanager `./android create avd -n <avd_name> -t 1`
- Clone the Google Git of [Goldfish](http://android.googlesource.com/kernel/goldfish)
- Add Android NDK to the PATH `export PATH=$PATH:<NDK_PATH>/android-ndk-r8-crystax/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64/bin`
- Modify Makefile of Goldfish with `ARCH := arm` and `CROSS_COMPILE := arm-linux-androideabi-`
- Run `make goldfish_defconfig` for Android 2.0 or `make goldfish_armv7_defconfig` for Android 4.0.
- Then `make menuconfig`
- Enable the `loadable module support`, also `Forced module loading`, `Module unloading` and `Module versioning support` in it.
- Run the emulator `./emulator -avd <avd_name> -kernel <zImage_path>/zImage &`

## Nillyr Version

- Create a device with avdmanager from android sdk (e.g: Nexus S with Android 2.3)
- Run the emulator: `emulator @Nexus_S -show-kernel`
- Extract the config from the emulated device `adb pull /proc/config.gz .`
- Extract the config file
- Edit the file for modules support. Change '# CONFIG_MODULES is not set' to CONFIG_MODULES=y
- Compile Android kernel with modules support and your new .config
- Run the emulator with the new zImage: `emulator @Nexus_S -kernel path/to/zImage -show-kernel -verbose`

Note: when compiling you may have to '[N/y/?]'.

```bash
Forced module loading (MODULE_FORCE_LOAD) [N/y/?] (NEW) y
Module unloading (MODULE_UNLOAD) [N/y/?] (NEW) y
Forced module unloading (MODULE_FORCE_UNLOAD) [N/y/?] (NEW) y
Module versioning support (MODVERSIONS) [N/y/?] (NEW) y
Source checksum for all modules (MODULE_SRCVERSION_ALL) [N/y/?] (NEW) y
```

Everything else is 'N'.

## The Read-Only Issue

Usually, the device is read-only. Here is a quick tip.

```bash
$ adb shell
# mount -o rw,remount rootfs /
# chmod 777 /mnt/sdcard
```
