#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xdd8f8694, "module_layout" },
	{ 0x37669270, "single_release" },
	{ 0x40970142, "seq_read" },
	{ 0x1be63b22, "seq_lseek" },
	{ 0x9cfebac4, "remove_proc_entry" },
	{ 0xc5850110, "printk" },
	{ 0x24ff48a4, "proc_create" },
	{ 0x4d60c256, "__task_pid_nr_ns" },
	{ 0xaab69fcf, "init_pid_ns" },
	{ 0x375cb97e, "seq_printf" },
	{ 0xebd39bd4, "current_task" },
	{ 0x1d8f45ba, "single_open" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "DBC76F19CF8E56D9E01C6EC");
