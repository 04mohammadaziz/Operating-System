cmd_/home/20maa2/elec377-tues-pm-60/lab0/lab0mod.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000 -z noexecstack   --build-id  -T ./scripts/module-common.lds -o /home/20maa2/elec377-tues-pm-60/lab0/lab0mod.ko /home/20maa2/elec377-tues-pm-60/lab0/lab0mod.o /home/20maa2/elec377-tues-pm-60/lab0/lab0mod.mod.o;  true