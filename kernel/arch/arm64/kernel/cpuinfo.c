/*
 * Record and handle CPU attributes.
 *
 * Copyright (C) 2014 ARM Ltd.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <asm/arch_timer.h>
#include <asm/cachetype.h>
#include <asm/cpu.h>
#include <asm/cputype.h>
#include <asm/cpufeature.h>

#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/compat.h>
#include <linux/elf.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/personality.h>
#include <linux/preempt.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/delay.h>
#ifdef CONFIG_AMLOGIC_CPU_INFO
#include <linux/amlogic/cpu_version.h>
#endif


/*
 * In case the boot CPU is hotpluggable, we record its initial state and
 * current state separately. Certain system registers may contain different
 * values depending on configuration at or after reset.
 */
DEFINE_PER_CPU(struct cpuinfo_arm64, cpu_data);
static struct cpuinfo_arm64 boot_cpu_data;

static char *icache_policy_str[] = {
	[ICACHE_POLICY_RESERVED] = "RESERVED/UNKNOWN",
	[ICACHE_POLICY_AIVIVT] = "AIVIVT",
	[ICACHE_POLICY_VIPT] = "VIPT",
	[ICACHE_POLICY_PIPT] = "PIPT",
};

unsigned long __icache_flags;

static const char *const hwcap_str[] = {
	"fp",
	"asimd",
	"evtstrm",
	"aes",
	"pmull",
	"sha1",
	"sha2",
	"crc32",
	"atomics",
	"fphp",
	"asimdhp",
	NULL
};

struct hw_part {
	u16	id;
	char	*name;
};

static const struct hw_part arm_hw_part[] = {
	{ ARM_CPU_PART_AEM_V8,		"AEMv8 Model" },
	{ ARM_CPU_PART_FOUNDATION,	"Foundation Model" },
	{ ARM_CPU_PART_CORTEX_A57,	"Cortex A57" },
	{ ARM_CPU_PART_CORTEX_A53,	"Cortex A53" },
	{ ARM_CPU_PART_CORTEX_A73,	"Cortex A73" },
	{ (-1), "unknown" }		/* Potenza == 0, unfortunately */
};

static const struct hw_part apm_hw_part[] = {
	{ APM_CPU_PART_POTENZA,		"Potenza" },
	{ (-1), "unknown" }		/* Potenza == 0, unfortunately */
};

static const struct hw_part brcm_hw_part[] = {
	{ BRCM_CPU_PART_VULCAN,		"Vulcan" },
	{ (-1), "unknown" }		/* Potenza == 0, unfortunately */
};

static const struct hw_part cavium_hw_part[] = {
	{ CAVIUM_CPU_PART_THUNDERX,	 "ThunderX" },
	{ CAVIUM_CPU_PART_THUNDERX_81XX, "ThunderX 81XX" },
	{ (-1), "unknown" }		/* Potenza == 0, unfortunately */
};

static const struct hw_part unknown_hw_part[] = {
	{ (-1), "unknown" }		/* Potenza == 0, unfortunately */
};

struct hw_impl {
	u8			id;
	const struct hw_part	*parts;
	char			*name;
};

static const struct hw_impl hw_implementer[] = {
	{ ARM_CPU_IMP_ARM,	arm_hw_part,	"ARM Ltd." },
	{ ARM_CPU_IMP_APM,	apm_hw_part,	"Applied Micro" },
	{ ARM_CPU_IMP_CAVIUM,	cavium_hw_part,	"Cavium" },
	{ ARM_CPU_IMP_BRCM,	brcm_hw_part,	"Broadcom" },
	{ 0, unknown_hw_part, "unknown" }
};

#ifdef CONFIG_COMPAT
static const char *const compat_hwcap_str[] = {
	"swp",
	"half",
	"thumb",
	"26bit",
	"fastmult",
	"fpa",
	"vfp",
	"edsp",
	"java",
	"iwmmxt",
	"crunch",
	"thumbee",
	"neon",
	"vfpv3",
	"vfpv3d16",
	"tls",
	"vfpv4",
	"idiva",
	"idivt",
	"vfpd32",
	"lpae",
	"evtstrm",
	NULL
};

static const char *const compat_hwcap2_str[] = {
	"aes",
	"pmull",
	"sha1",
	"sha2",
	"crc32",
	NULL
};
#endif /* CONFIG_COMPAT */

static int c_show(struct seq_file *m, void *v)
{
	int i, j;
	bool compat = is_compat_task() ||
		 personality(current->personality) == PER_LINUX32;
	u8 impl;
	u16 part;
	const struct hw_part *parts;

#ifdef CONFIG_AMLOGIC_CPU_INFO
	unsigned char chipid[CHIPID_LEN];
#endif

	for_each_online_cpu(i) {
		struct cpuinfo_arm64 *cpuinfo = &per_cpu(cpu_data, i);
		u32 midr = cpuinfo->reg_midr;

		/*
		 * glibc reads /proc/cpuinfo to determine the number of
		 * online processors, looking for lines beginning with
		 * "processor".  Give glibc what it expects.
		 */
		seq_printf(m, "processor\t: %d\n", i);
		if (compat)
			seq_printf(m, "model name\t: ARMv8 Processor rev %d (%s)\n",
				   MIDR_REVISION(midr), COMPAT_ELF_PLATFORM);

		impl = (u8) MIDR_IMPLEMENTOR(midr);
		for (j = 0; hw_implementer[j].id != 0; j++) {
			if (hw_implementer[j].id == impl) {
				seq_printf(m, "CPU name\t: %s ",
					   hw_implementer[j].name);
				parts = hw_implementer[j].parts;
				break;
			}
		}
		if (hw_implementer[j].id == 0) {
			seq_printf(m, "CPU name\t: %s ",
				   hw_implementer[j].name);
			parts = hw_implementer[j].parts;
		}

		part = (u16) MIDR_PARTNUM(midr);
		for (j = 0; parts[j].id != (-1); j++) {
			if (parts[j].id == part) {
				seq_printf(m, "%s\n", parts[j].name);
				break;
			}
		}
		if (parts[j].id == (-1))
			seq_printf(m, "%s", parts[j].name);
		seq_puts(m, "\n");

		seq_printf(m, "BogoMIPS\t: %lu.%02lu\n",
			   loops_per_jiffy / (500000UL/HZ),
			   loops_per_jiffy / (5000UL/HZ) % 100);

		/*
		 * Dump out the common processor features in a single line.
		 * Userspace should read the hwcaps with getauxval(AT_HWCAP)
		 * rather than attempting to parse this, but there's a body of
		 * software which does already (at least for 32-bit).
		 */
		seq_puts(m, "Features\t:");
		if (compat) {
#ifdef CONFIG_COMPAT
			for (j = 0; compat_hwcap_str[j]; j++)
				if (compat_elf_hwcap & (1 << j))
					seq_printf(m, " %s", compat_hwcap_str[j]);

			for (j = 0; compat_hwcap2_str[j]; j++)
				if (compat_elf_hwcap2 & (1 << j))
					seq_printf(m, " %s", compat_hwcap2_str[j]);
#endif /* CONFIG_COMPAT */
		} else {
			for (j = 0; hwcap_str[j]; j++)
				if (elf_hwcap & (1 << j))
					seq_printf(m, " %s", hwcap_str[j]);
		}
		seq_puts(m, "\n");

		seq_printf(m, "CPU implementer\t: 0x%02x\n",
			   MIDR_IMPLEMENTOR(midr));
		seq_printf(m, "CPU architecture: 8\n");
		seq_printf(m, "CPU variant\t: 0x%x\n", MIDR_VARIANT(midr));
		seq_printf(m, "CPU part\t: 0x%03x\n", MIDR_PARTNUM(midr));
		seq_printf(m, "CPU revision\t: %d\n\n", MIDR_REVISION(midr));
	}
#ifdef CONFIG_AMLOGIC_CPU_INFO
	cpuinfo_get_chipid(chipid, CHIPID_LEN);
	seq_puts(m, "Serial\t\t: ");
	for (i = 0; i < 16; i++)
		seq_printf(m, "%02x", chipid[i]);
	seq_puts(m, "\n");
#endif

	seq_printf(m, "Hardware\t: %s\n\n", "Amlogic");
	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations cpuinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_show
};


static struct kobj_type cpuregs_kobj_type = {
	.sysfs_ops = &kobj_sysfs_ops,
};

/*
 * The ARM ARM uses the phrase "32-bit register" to describe a register
 * whose upper 32 bits are RES0 (per C5.1.1, ARM DDI 0487A.i), however
 * no statement is made as to whether the upper 32 bits will or will not
 * be made use of in future, and between ARM DDI 0487A.c and ARM DDI
 * 0487A.d CLIDR_EL1 was expanded from 32-bit to 64-bit.
 *
 * Thus, while both MIDR_EL1 and REVIDR_EL1 are described as 32-bit
 * registers, we expose them both as 64 bit values to cater for possible
 * future expansion without an ABI break.
 */
#define kobj_to_cpuinfo(kobj)	container_of(kobj, struct cpuinfo_arm64, kobj)
#define CPUREGS_ATTR_RO(_name, _field)						\
	static ssize_t _name##_show(struct kobject *kobj,			\
			struct kobj_attribute *attr, char *buf)			\
	{									\
		struct cpuinfo_arm64 *info = kobj_to_cpuinfo(kobj);		\
										\
		if (info->reg_midr)						\
			return sprintf(buf, "0x%016x\n", info->reg_##_field);	\
		else								\
			return 0;						\
	}									\
	static struct kobj_attribute cpuregs_attr_##_name = __ATTR_RO(_name)

CPUREGS_ATTR_RO(midr_el1, midr);
CPUREGS_ATTR_RO(revidr_el1, revidr);

static struct attribute *cpuregs_id_attrs[] = {
	&cpuregs_attr_midr_el1.attr,
	&cpuregs_attr_revidr_el1.attr,
	NULL
};

static struct attribute_group cpuregs_attr_group = {
	.attrs = cpuregs_id_attrs,
	.name = "identification"
};

static int cpuid_add_regs(int cpu)
{
	int rc;
	struct device *dev;
	struct cpuinfo_arm64 *info = &per_cpu(cpu_data, cpu);

	dev = get_cpu_device(cpu);
	if (!dev) {
		rc = -ENODEV;
		goto out;
	}
	rc = kobject_add(&info->kobj, &dev->kobj, "regs");
	if (rc)
		goto out;
	rc = sysfs_create_group(&info->kobj, &cpuregs_attr_group);
	if (rc)
		kobject_del(&info->kobj);
out:
	return rc;
}

static int cpuid_remove_regs(int cpu)
{
	struct device *dev;
	struct cpuinfo_arm64 *info = &per_cpu(cpu_data, cpu);

	dev = get_cpu_device(cpu);
	if (!dev)
		return -ENODEV;
	if (info->kobj.parent) {
		sysfs_remove_group(&info->kobj, &cpuregs_attr_group);
		kobject_del(&info->kobj);
	}

	return 0;
}

static int cpuid_callback(struct notifier_block *nb,
			 unsigned long action, void *hcpu)
{
	int rc = 0;
	unsigned long cpu = (unsigned long)hcpu;

	switch (action & ~CPU_TASKS_FROZEN) {
	case CPU_ONLINE:
		rc = cpuid_add_regs(cpu);
		break;
	case CPU_DEAD:
		rc = cpuid_remove_regs(cpu);
		break;
	}

	return notifier_from_errno(rc);
}

static int __init cpuinfo_regs_init(void)
{
	int cpu;

	cpu_notifier_register_begin();

	for_each_possible_cpu(cpu) {
		struct cpuinfo_arm64 *info = &per_cpu(cpu_data, cpu);

		kobject_init(&info->kobj, &cpuregs_kobj_type);
		if (cpu_online(cpu))
			cpuid_add_regs(cpu);
	}
	__hotcpu_notifier(cpuid_callback, 0);

	cpu_notifier_register_done();
	return 0;
}
static void cpuinfo_detect_icache_policy(struct cpuinfo_arm64 *info)
{
	unsigned int cpu = smp_processor_id();
	u32 l1ip = CTR_L1IP(info->reg_ctr);

	if (l1ip != ICACHE_POLICY_PIPT) {
		/*
		 * VIPT caches are non-aliasing if the VA always equals the PA
		 * in all bit positions that are covered by the index. This is
		 * the case if the size of a way (# of sets * line size) does
		 * not exceed PAGE_SIZE.
		 */
		u32 waysize = icache_get_numsets() * icache_get_linesize();

		if (l1ip != ICACHE_POLICY_VIPT || waysize > PAGE_SIZE)
			set_bit(ICACHEF_ALIASING, &__icache_flags);
	}
	if (l1ip == ICACHE_POLICY_AIVIVT)
		set_bit(ICACHEF_AIVIVT, &__icache_flags);

	pr_info("Detected %s I-cache on CPU%d\n", icache_policy_str[l1ip], cpu);
}

static void __cpuinfo_store_cpu(struct cpuinfo_arm64 *info)
{
	info->reg_cntfrq = arch_timer_get_cntfrq();
	info->reg_ctr = read_cpuid_cachetype();
	info->reg_dczid = read_cpuid(DCZID_EL0);
	info->reg_midr = read_cpuid_id();
	info->reg_revidr = read_cpuid(REVIDR_EL1);

	info->reg_id_aa64dfr0 = read_cpuid(ID_AA64DFR0_EL1);
	info->reg_id_aa64dfr1 = read_cpuid(ID_AA64DFR1_EL1);
	info->reg_id_aa64isar0 = read_cpuid(ID_AA64ISAR0_EL1);
	info->reg_id_aa64isar1 = read_cpuid(ID_AA64ISAR1_EL1);
	info->reg_id_aa64mmfr0 = read_cpuid(ID_AA64MMFR0_EL1);
	info->reg_id_aa64mmfr1 = read_cpuid(ID_AA64MMFR1_EL1);
	info->reg_id_aa64mmfr2 = read_cpuid(ID_AA64MMFR2_EL1);
	info->reg_id_aa64pfr0 = read_cpuid(ID_AA64PFR0_EL1);
	info->reg_id_aa64pfr1 = read_cpuid(ID_AA64PFR1_EL1);

	/* Update the 32bit ID registers only if AArch32 is implemented */
	if (id_aa64pfr0_32bit_el0(info->reg_id_aa64pfr0)) {
		info->reg_id_dfr0 = read_cpuid(ID_DFR0_EL1);
		info->reg_id_isar0 = read_cpuid(ID_ISAR0_EL1);
		info->reg_id_isar1 = read_cpuid(ID_ISAR1_EL1);
		info->reg_id_isar2 = read_cpuid(ID_ISAR2_EL1);
		info->reg_id_isar3 = read_cpuid(ID_ISAR3_EL1);
		info->reg_id_isar4 = read_cpuid(ID_ISAR4_EL1);
		info->reg_id_isar5 = read_cpuid(ID_ISAR5_EL1);
		info->reg_id_mmfr0 = read_cpuid(ID_MMFR0_EL1);
		info->reg_id_mmfr1 = read_cpuid(ID_MMFR1_EL1);
		info->reg_id_mmfr2 = read_cpuid(ID_MMFR2_EL1);
		info->reg_id_mmfr3 = read_cpuid(ID_MMFR3_EL1);
		info->reg_id_pfr0 = read_cpuid(ID_PFR0_EL1);
		info->reg_id_pfr1 = read_cpuid(ID_PFR1_EL1);

		info->reg_mvfr0 = read_cpuid(MVFR0_EL1);
		info->reg_mvfr1 = read_cpuid(MVFR1_EL1);
		info->reg_mvfr2 = read_cpuid(MVFR2_EL1);
	}

	cpuinfo_detect_icache_policy(info);
}

void cpuinfo_store_cpu(void)
{
	struct cpuinfo_arm64 *info = this_cpu_ptr(&cpu_data);
	__cpuinfo_store_cpu(info);
	update_cpu_features(smp_processor_id(), info, &boot_cpu_data);
}

void __init cpuinfo_store_boot_cpu(void)
{
	struct cpuinfo_arm64 *info = &per_cpu(cpu_data, 0);
	__cpuinfo_store_cpu(info);

	boot_cpu_data = *info;
	init_cpu_features(&boot_cpu_data);
}

device_initcall(cpuinfo_regs_init);
