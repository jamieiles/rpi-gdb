#ifndef __DEBUG_H__
#define __DEBUG_H__

struct bug_entry {
	const char *filename;
	unsigned int line;
	unsigned long addr;
	const char *msg;
};

#define BUG(__msg) ({ \
	asm volatile("1: .word	0xffffffff\n" \
		     ".pushsection .bugtable, \"a\"\n" \
		     ".word	%c0\n" \
		     ".word	%c1\n" \
		     ".word	1b\n" \
		     ".word	%c2\n" \
		     ".popsection" :: "i"(__FILE__), "i"(__LINE__), \
		     "i"(__msg)); \
})

#define BUG_ON(__cond, __msg) ({ \
	if ((__cond)) \
		BUG((__msg)); \
})

#endif /* __DEBUG_H__ */
