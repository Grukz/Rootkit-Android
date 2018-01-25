# Android Rootkit with Phrack issues 68-6

## [ What Bowen has done ]

--------------------------------------------------------------------------------

### Build Android Emulator

In the paper of Phrack issues 68-6, the nernel tested are 2.6.29-omap1 kernel and 2.6.32.9 kernel. So I chose Android 2.2 with 2.6.29-00261-g0097074-dirty digit@digit #20 as kernel version for my Android emulator.

#### Step by step

##### Android SDK Tools

As the latest Android SDK Tools which downloaded by brew on macOS does not work, I choose to use the Android SDK Tools Rev. 16 for initialisation and updated it to the latest (Rev. 25.2.5).

At the path of `android-sdk/tools`, launch the Android SDK Manager with `./android`.

##### Packages

For basic use, I installed:

- Android SDK Platform-tools (remember to modify PATH, otherwise with brew on macOS)
- Android SDK Build-tools Rev. 19.1 (the oldest avaliable, otherwise Rev. 8 is enough)
- Android 2.2 (API 8)
- Android Support Repository

##### Create Android Virtual Device(AVD)

**Method 1 : Command line**

At the path of `android-sdk/tools`, create the AVD with `./android create avd -n <avd_name> -t 1`.

**Method 2 : Graphic software**

At the path of `android-sdk/tools`, launch AVD Manager with `./android avd`.

And click `Create...` for a new AVD.

##### Configuration

**Method 1 : Command line**

Edit the file `~/.android/avd/n00d1e5.avd/config.ini`.

For example, `hw.ramSize=512` set 512MB as the size of RAM.

**Method 2 : Graphic software**

With AVD Manager, choose `<avd_name>` and click `Edit...`.

For example, if I want to use the keyboard of PC in the place of vitual keyboard, click `New...` in the Hardware section, and add `Keyboard support`.

##### Launching

**Method 1 : Command line**

At the path of `android-sdk/tools`, launch the AVD with `./emulator -avd <avd_name> &`.

**Method 2 : Graphic software**

With AVD Manager, choose `<avd_name>` and click `Start...`.

--------------------------------------------------------------------------------

### Chmod

By default, the AVD is on mode read-only, we could modify with:

```
adb shell
#su
#mount -o rw,remount rootfs /
#chmod 777 /mnt/sdcard
#exit
```

We could test the mode by send a file to AVD with:

```
adb push <file_name> /
```

--------------------------------------------------------------------------------

### Test Rootkit Existed

With several key words, I found [Android-Rootkit](https://github.com/hiteshd/) on GitHub.

#### Build module

I tried to have the same environment as the Makefile.

[android-ndk-r7-crystax-4-linux-x86.tar.bz2](https://www.crystax.net/en/download)

[android_kernel_htc_qsd8k-jellybean](https://github.com/Evervolv/android_kernel_htc_qsd8k/tree/jellybean)

And the Makefile modified to:

```
obj-m += sys_call_table.o

INC_PATH = ~/Downloads/android-ndk-r7-crystax-4/platforms/android-4/arch-arm/usr/include
LIB_PATH = ~/Downloads/android-ndk-r7-crystax-4/platforms/android-4/arch-arm/usr/lib
GCC = ~/Downloads/android-ndk-r7-crystax-4/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-andr$
LD_FLAGS = -Wl,-dynamic-linker,/system/bin/linker,-rpath-link=$(LIB_PATH) -L$(LIB_PATH) -nostdlib -lc -lm -lstdc++
PRE_LINK = $(LIB_PATH)/crtbegin_dynamic.o
POST_LINK = $(LIB_PATH)/crtend_android.o
CROSS_COMPILE=~/Downloads/android-ndk-r7-crystax-4/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-$
KERNEL_DIR=~/Downloads/android_kernel_htc_qsd8k-jellybean/

VERSION = v1.1

all:
        make -C $(KERNEL_DIR) M=$(PWD) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) CFLAGS_MODULE=-fno-pic modules

clean:
        make -C $(KERNEL_DIR) M=$(PWD) clean
        rm -f sys_call_table_inst

%.o:%.c
        $(GCC) -w -I$(INC_PATH) -c $< -o $@
```

Build fail

```
make -C ~/Downloads/android_kernel_htc_qsd8k-jellybean/ M=/Users/Bowen/Downloads/Android-Rootkit-master ARCH=arm CROSS_COMPILE=~/Downloads/android-ndk-r7-crystax-4/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi- CFLAGS_MODULE=-fno-pic modules
/bin/sh: /Users/Bowen/Downloads/android-ndk-r7-crystax-4/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc: cannot execute binary file

  ERROR: Kernel configuration is invalid.
         include/generated/autoconf.h or include/config/auto.conf are missing.
         Run 'make oldconfig && make prepare' on kernel src to fix it.

/bin/sh: /bin/false: No such file or directory

  WARNING: Symbol version dump /Users/Bowen/Downloads/android_kernel_htc_qsd8k-jellybean/Module.symvers
           is missing; modules will have no dependencies and modversions.

  CC [M]  /Users/Bowen/Downloads/Android-Rootkit-master/sys_call_table.o
/bin/sh: /Users/Bowen/Downloads/android-ndk-r7-crystax-4/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc: cannot execute binary file
make[2]: *** [/Users/Bowen/Downloads/Android-Rootkit-master/sys_call_table.o] Error 126
make[1]: *** [_module_/Users/Bowen/Downloads/Android-Rootkit-master] Error 2
make: *** [all] Error 2
```

## [ What Bowen gonna do ]

- Verifie what's `/system/bin/linker` in Makefile
- Change version of `android-ndk-r7` to get the missing files
- Change version of `android_kernel_htc_qsd8k` to get the missing files
- Update the version of Android chosen, the rootkit existed was tested with kernel 2.6.38.8.