cmd_/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/bcmsrom.o := /opt/toolchains/uclibc-crosstools-gcc-4.2.3-3/usr/bin/mips-linux-uclibc-gcc -Wp,-MD,/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/.bcmsrom.o.d  -nostdinc -isystem /opt/toolchains/uclibc-crosstools-gcc-4.2.3-3/usr/bin/../lib/gcc/mips-linux-uclibc/4.2.3/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -O2  -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding  -march=mips32 -Wa,-mips32 -Wa,--trap -Iinclude/asm-mips/mach-bcm963xx -Iinclude/asm-mips/mach-generic -fomit-frame-pointer  -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign -DDSLCPE_WLAN_VERSION=\"cpe4.402.0\" -DDSLCPE_DELAY -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/wl/sys -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/emf/emf -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/emf/igs -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/router/shared -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/router/emf/emf -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/router/emf/igs -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/emf/emf -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/emf/igs -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/opensource/include/bcm963xx -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/include/bcm963xx -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/shared/opensource/include/bcm963xx -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/shared/broadcom/include/bcm963xx -I/home/eric/U12H154_Optus/Motive/EVG2000-53115/shared/opensource/boardparms/bcm963xx -DIL_BIGENDIAN -DBCMDRIVER -DDSLCPE_DGASP -DDSLCPE_WOMBO -DDSLCPE -DDSLCPE_SCBLIST -DD11CONF=0x00011b00 -DACONF=0 -DGCONF=0x00000080 -DNCONF=0x00000005f -DLPCONF=0 -DWLC_LOW -DWLC_HIGH -DAP -DMBSS -DWME_PER_AC_TX_PARAMS -DWME_PER_AC_TUNING -DRXCHAIN_PWRSAVE -DWMF -DWLLED -DWME -DWLAFTERBURNER -DCRAM -DWL11N -DWL11H -DWL11D -DWLCNT -DWLCHANIM -DDELTASTATS -DWLCNTSCB -DWLCOEX -DBCMWPA2 -DBCMCCMP -DBCMDMA64 -DWLAMSDU -DWLAMSDU_SWDEAGG -DWLAMPDU -DWLTINYDUMP -DWLTEST -DDSLCPE_BLOG  -DMODULE -mlong-calls -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(bcmsrom)"  -D"KBUILD_MODNAME=KBUILD_STR(wl)" -c -o /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/bcmsrom.o /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/bcmsrom.c

deps_/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/bcmsrom.o := \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/bcmsrom.c \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/typedefs.h \
  include/linux/version.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbd.h) \
    $(wildcard include/config/lsf.h) \
    $(wildcard include/config/resources/64bit.h) \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/enable/must/check.h) \
  include/linux/compiler-gcc4.h \
    $(wildcard include/config/forced/inlining.h) \
  include/linux/compiler-gcc.h \
  include/asm/posix_types.h \
  include/asm/sgidefs.h \
  include/asm/types.h \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/64bit/phys/addr.h) \
    $(wildcard include/config/64bit.h) \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmdefs.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/osl.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/linux_osl.h \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/numa.h) \
  /opt/toolchains/uclibc-crosstools-gcc-4.2.3-3/usr/bin/../lib/gcc/mips-linux-uclibc/4.2.3/include/stdarg.h \
  include/linux/linkage.h \
  include/asm/linkage.h \
  include/linux/bitops.h \
  include/asm/bitops.h \
    $(wildcard include/config/cpu/mipsr2.h) \
    $(wildcard include/config/cpu/mips32.h) \
    $(wildcard include/config/cpu/mips64.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/x86.h) \
  include/asm/irqflags.h \
    $(wildcard include/config/mips/mt/smtc.h) \
    $(wildcard include/config/irq/cpu.h) \
    $(wildcard include/config/mips/mt/smtc/instant/replay.h) \
  include/asm/hazards.h \
    $(wildcard include/config/cpu/r10000.h) \
    $(wildcard include/config/cpu/rm9000.h) \
    $(wildcard include/config/cpu/sb1.h) \
  include/asm/barrier.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/cpu/has/wb.h) \
    $(wildcard include/config/weak/ordering.h) \
    $(wildcard include/config/smp.h) \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  include/asm/break.h \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/debug/bugverbose.h) \
  include/asm/byteorder.h \
    $(wildcard include/config/cpu/mips64/r2.h) \
  include/linux/byteorder/big_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm/cpu-features.h \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/cpu/mipsr2/irq/vi.h) \
    $(wildcard include/config/cpu/mipsr2/irq/ei.h) \
  include/asm/cpu.h \
    $(wildcard include/config/mips/brcm.h) \
  include/asm/cpu-info.h \
    $(wildcard include/config/sgi/ip27.h) \
    $(wildcard include/config/mips/mt.h) \
  include/asm/cache.h \
    $(wildcard include/config/mips/l1/cache/shift.h) \
  include/asm-mips/mach-generic/kmalloc.h \
    $(wildcard include/config/dma/coherent.h) \
  include/asm-mips/mach-bcm963xx/cpu-feature-overrides.h \
    $(wildcard include/config/bcm96358.h) \
    $(wildcard include/config/bcm96368.h) \
    $(wildcard include/config/bcm96816.h) \
  include/asm/war.h \
    $(wildcard include/config/sgi/ip22.h) \
    $(wildcard include/config/sni/rm.h) \
    $(wildcard include/config/cpu/r5432.h) \
    $(wildcard include/config/sb1/pass/1/workarounds.h) \
    $(wildcard include/config/sb1/pass/2/workarounds.h) \
    $(wildcard include/config/mips/malta.h) \
    $(wildcard include/config/mips/atlas.h) \
    $(wildcard include/config/mips/sead.h) \
    $(wildcard include/config/cpu/tx49xx.h) \
    $(wildcard include/config/momenco/jaguar/atx.h) \
    $(wildcard include/config/pmc/yosemite.h) \
    $(wildcard include/config/basler/excite.h) \
    $(wildcard include/config/momenco/ocelot/3.h) \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ffz.h \
  include/asm-generic/bitops/find.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  include/asm-generic/bitops/ext2-atomic.h \
  include/asm-generic/bitops/minix.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/string.h \
  include/asm/string.h \
    $(wildcard include/config/cpu/r3000.h) \
  include/asm/addrspace.h \
    $(wildcard include/config/cpu/r4300.h) \
    $(wildcard include/config/cpu/r4x00.h) \
    $(wildcard include/config/cpu/r5000.h) \
    $(wildcard include/config/cpu/rm7000.h) \
    $(wildcard include/config/cpu/nevada.h) \
    $(wildcard include/config/cpu/r8000.h) \
    $(wildcard include/config/cpu/sb1a.h) \
  include/asm-mips/mach-generic/spaces.h \
    $(wildcard include/config/dma/noncoherent.h) \
  include/asm/paccess.h \
  include/linux/errno.h \
  include/asm/errno.h \
  include/asm-generic/errno-base.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/linuxver.h \
    $(wildcard include/config/net/radio.h) \
    $(wildcard include/config/wireless/ext.h) \
    $(wildcard include/config/pcmcia.h) \
  include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/sysfs.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/thread_info.h \
  include/asm/thread_info.h \
    $(wildcard include/config/page/size/4kb.h) \
    $(wildcard include/config/page/size/8kb.h) \
    $(wildcard include/config/page/size/16kb.h) \
    $(wildcard include/config/page/size/64kb.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/asm/processor.h \
    $(wildcard include/config/mips/mt/fpaff.h) \
    $(wildcard include/config/cpu/has/prefetch.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/hotplug/cpu.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/bitmap.h \
  include/asm/cachectl.h \
  include/asm/mipsregs.h \
    $(wildcard include/config/cpu/vr41xx.h) \
  include/asm/prefetch.h \
  include/asm/system.h \
  include/asm/dsp.h \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/generic/hardirqs.h) \
    $(wildcard include/config/prove/locking.h) \
  include/linux/spinlock_types_up.h \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  include/asm/atomic.h \
  include/asm-generic/atomic.h \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
  include/linux/prefetch.h \
  include/linux/stat.h \
  include/asm/stat.h \
  include/linux/time.h \
  include/linux/seqlock.h \
  include/linux/cache.h \
  include/linux/kmod.h \
    $(wildcard include/config/kmod.h) \
  include/linux/elf.h \
  include/linux/auxvec.h \
  include/asm/auxvec.h \
  include/linux/elf-em.h \
  include/asm/elf.h \
    $(wildcard include/config/mips32/n32.h) \
    $(wildcard include/config/mips32/o32.h) \
    $(wildcard include/config/mips32/compat.h) \
  include/linux/kobject.h \
    $(wildcard include/config/hotplug.h) \
  include/linux/sysfs.h \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/kref.h \
  include/linux/wait.h \
  include/asm/current.h \
  include/linux/moduleparam.h \
  include/linux/init.h \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/acpi/hotplug/memory.h) \
  include/asm/local.h \
  include/linux/percpu.h \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/slab.h) \
    $(wildcard include/config/klob.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/gfp.h \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/nodemask.h \
  include/asm/page.h \
    $(wildcard include/config/build/elf64.h) \
    $(wildcard include/config/limited/dma.h) \
  include/linux/pfn.h \
  include/asm/io.h \
  include/asm-generic/iomap.h \
  include/asm/pgtable-bits.h \
    $(wildcard include/config/cpu/mips32/r1.h) \
    $(wildcard include/config/cpu/tx39xx.h) \
    $(wildcard include/config/mips/uncached.h) \
  include/asm-mips/mach-generic/ioremap.h \
  include/asm-mips/mach-generic/mangle-port.h \
    $(wildcard include/config/swap/io/space.h) \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/out/of/line/pfn/to/page.h) \
  include/asm-generic/page.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  include/linux/smp.h \
  include/asm/topology.h \
  include/asm-mips/mach-generic/topology.h \
  include/asm-generic/topology.h \
  include/linux/slab_def.h \
    $(wildcard include/config/kmalloc/accounting.h) \
  include/asm/percpu.h \
  include/asm-generic/percpu.h \
  include/asm/module.h \
    $(wildcard include/config/cpu/mips32/r2.h) \
    $(wildcard include/config/cpu/mips64/r1.h) \
    $(wildcard include/config/cpu/r6000.h) \
  include/asm/uaccess.h \
  include/asm-generic/uaccess.h \
  include/linux/mm.h \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/pagealloc.h) \
  include/linux/sched.h \
    $(wildcard include/config/preempt/softirqs.h) \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/cc/stackprotector.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/compat.h) \
    $(wildcard include/config/fault/injection.h) \
  include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/capability.h \
  include/linux/timex.h \
    $(wildcard include/config/time/interpolation.h) \
    $(wildcard include/config/no/hz.h) \
  include/asm/timex.h \
  include/asm-mips/mach-generic/timex.h \
  include/linux/jiffies.h \
  include/linux/calc64.h \
  include/asm/div64.h \
  include/asm/compiler.h \
  include/linux/rbtree.h \
  include/asm/semaphore.h \
  include/asm/ptrace.h \
    $(wildcard include/config/cpu/has/smartmips.h) \
  include/asm/isadep.h \
  include/asm/mmu.h \
  include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/sem.h \
  include/linux/ipc.h \
    $(wildcard include/config/ipc/ns.h) \
  include/asm/ipcbuf.h \
  include/asm/sembuf.h \
  include/linux/signal.h \
  include/asm/signal.h \
    $(wildcard include/config/trad/signals.h) \
    $(wildcard include/config/binfmt/irix.h) \
  include/asm-generic/signal.h \
  include/asm/sigcontext.h \
  include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/securebits.h \
  include/linux/fs_struct.h \
  include/linux/completion.h \
  include/linux/pid.h \
  include/linux/rcupdate.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/futex.h \
    $(wildcard include/config/futex.h) \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/param.h \
  include/linux/resource.h \
  include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  include/linux/aio.h \
  include/linux/workqueue.h \
  include/linux/aio_abi.h \
  include/linux/uio.h \
  include/linux/sysdev.h \
  include/linux/pm.h \
    $(wildcard include/config/software/suspend.h) \
    $(wildcard include/config/pm.h) \
  include/linux/prio_tree.h \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/security.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
  include/linux/limits.h \
  include/linux/ioctl.h \
  include/asm/ioctl.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
    $(wildcard include/config/profiling.h) \
  include/linux/namei.h \
  include/linux/radix-tree.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/fcntl.h \
  include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
  include/linux/err.h \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  include/linux/backing-dev.h \
  include/linux/mm_types.h \
  include/asm/pgtable.h \
  include/asm/pgtable-32.h \
  include/asm/fixmap.h \
  include/asm-generic/pgtable-nopmd.h \
  include/asm-generic/pgtable-nopud.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/s390.h) \
    $(wildcard include/config/swap.h) \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
  include/linux/pci.h \
    $(wildcard include/config/pci/msi.h) \
    $(wildcard include/config/pci.h) \
    $(wildcard include/config/ht/irq.h) \
    $(wildcard include/config/pci/domains.h) \
  include/linux/pci_regs.h \
  include/linux/mod_devicetable.h \
  include/linux/ioport.h \
  include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
  include/linux/klist.h \
  include/asm/device.h \
  include/asm-generic/device.h \
  include/linux/pci_ids.h \
  include/linux/dmapool.h \
  include/asm/scatterlist.h \
  include/asm/pci.h \
    $(wildcard include/config/dma/need/pci/map/state.h) \
  include/asm-generic/pci-dma-compat.h \
  include/linux/dma-mapping.h \
  include/asm/dma-mapping.h \
  include/linux/interrupt.h \
    $(wildcard include/config/preempt/hardirqs.h) \
    $(wildcard include/config/preempt/rt.h) \
    $(wildcard include/config/generic/irq/probe.h) \
  include/linux/irqreturn.h \
  include/linux/hardirq.h \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/virt/cpu/accounting.h) \
  include/linux/smp_lock.h \
    $(wildcard include/config/lock/kernel.h) \
  include/asm/hardirq.h \
  include/linux/irq.h \
    $(wildcard include/config/irq/per/cpu.h) \
    $(wildcard include/config/irq/release/method.h) \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/irqbalance.h) \
    $(wildcard include/config/auto/irq/affinity.h) \
    $(wildcard include/config/generic/hardirqs/no//do/irq.h) \
  include/asm/irq.h \
    $(wildcard include/config/i8259.h) \
  include/asm/mipsmtregs.h \
  include/asm-mips/mach-generic/irq.h \
    $(wildcard include/config/irq/cpu/rm7k.h) \
    $(wildcard include/config/irq/cpu/rm9k.h) \
  include/asm/irq_regs.h \
  include/asm/hw_irq.h \
  include/linux/profile.h \
  include/linux/irq_cpustat.h \
  include/linux/netdevice.h \
    $(wildcard include/config/ax25.h) \
    $(wildcard include/config/tr.h) \
    $(wildcard include/config/net/ipip.h) \
    $(wildcard include/config/net/ipgre.h) \
    $(wildcard include/config/ipv6/sit.h) \
    $(wildcard include/config/ipv6/tunnel.h) \
    $(wildcard include/config/netpoll.h) \
    $(wildcard include/config/net/poll/controller.h) \
    $(wildcard include/config/netpoll/trap.h) \
    $(wildcard include/config/net/dma.h) \
  include/linux/if.h \
  include/linux/socket.h \
  include/asm/socket.h \
  include/asm/sockios.h \
  include/linux/sockios.h \
  include/linux/hdlc/ioctl.h \
  include/linux/if_ether.h \
  include/linux/skbuff.h \
    $(wildcard include/config/blog.h) \
    $(wildcard include/config/netfilter.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/vlan/8021q.h) \
    $(wildcard include/config/nf/conntrack.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/net/cls/act.h) \
    $(wildcard include/config/network/secmark.h) \
  include/linux/net.h \
  include/linux/random.h \
  include/linux/sysctl.h \
  include/linux/textsearch.h \
  include/net/checksum.h \
  include/asm/checksum.h \
  include/linux/in6.h \
  include/linux/dmaengine.h \
    $(wildcard include/config/dma/engine.h) \
  include/linux/if_packet.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/linux_osl_dslcpe.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmutils.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/opensource/include/bcm963xx/bcmnet.h \
    $(wildcard include/config/igmp/snooping//.h) \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/hndsoc.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/sbconfig.h \
    $(wildcard include/config/h.h) \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/aidmp.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/sbchipc.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmdevs.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmendian.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/sbpcmcia.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/pcicfg.h \
    $(wildcard include/config/addr.h) \
    $(wildcard include/config/bus.h) \
    $(wildcard include/config/slot.h) \
    $(wildcard include/config/fun.h) \
    $(wildcard include/config/off.h) \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/siutils.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmsrom_fmt.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmsrom.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmsrom_fmt.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmsrom_tbl.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/sbpcmcia.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/wlioctl.h \
    $(wildcard include/config/item.h) \
    $(wildcard include/config/usbrndis/retail.h) \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/proto/ethernet.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/proto/bcmeth.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/proto/bcmevent.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/proto/802.11.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/proto/wpa.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmwifi.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmnvram.h \
  /home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/include/bcmotp.h \

/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/bcmsrom.o: $(deps_/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/bcmsrom.o)

$(deps_/home/eric/U12H154_Optus/Motive/EVG2000-53115/bcmdrivers/broadcom/net/wl/bcm96368/shared/bcmsrom.o):
