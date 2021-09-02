/*
 * Copyright (c) 2016-2020, Immotor
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-27     Allen      first version
 */

#ifdef CONFIG_CMDLINE

#ifndef  _CMDLINE_DEF_H_
#define  _CMDLINE_DEF_H_

#ifdef __cplusplus
extern "C"{
#endif

	typedef long (*syscall_func)(void);

	/* system call table */
	struct finsh_syscall
	{
		const char* name;       /* the name of system call */
#if defined(FINSH_USING_DESCRIPTION) && defined(FINSH_USING_SYMTAB)
		const char* desc;       /* description of system call */
#endif
		syscall_func func;      /* the function address of system call */
	};
	extern struct finsh_syscall* _syscall_table_begin, * _syscall_table_end;

	/* find out system call, which should be implemented in user program */
	struct finsh_syscall* finsh_syscall_lookup(const char* name);

////////////////////////////////////////////////////////

#define FINSH_FUNCTION_EXPORT_CMD(name, cmd, desc)                      \
                const char __fsym_##cmd##_name[] SECTION(".rodata.name") = #cmd;    \
                const char __fsym_##cmd##_desc[] SECTION(".rodata.name") = #desc;   \
                RT_USED const struct finsh_syscall __fsym_##cmd SECTION("FSymTab")= \
                {                           \
                    __fsym_##cmd##_name,    \
                    __fsym_##cmd##_desc,    \
                    (syscall_func)&name     \
                };

#define MSH_CMD_EXPORT(command, desc)   \
    FINSH_FUNCTION_EXPORT_CMD(command, __cmd_##command, desc)



#ifdef __cplusplus
}
#endif

#endif 

#endif
