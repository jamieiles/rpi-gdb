#include "debug.h"
#include "io.h"
#include "kernel.h"
#include "regs.h"
#include "printk.h"
#include "string.h"
#include "types.h"
#include "uart.h"

#define L1_CACHE_LINE_SZ	32

#define CMD_BUF_LEN	512
static char cmd_buf[CMD_BUF_LEN];
static bool running = false;

/*
 * The breakpoint instruction that we're using.  A normal breakpoint just
 * triggers an exception but won't transfer control to the monitor vectors.
 * Instead we use an SMC call - the application shouldn't be making monitor
 * calls.
 */
#define SMC_BKPT	0xe161097a

struct bkpt {
	uint32_t *addr;
	uint32_t saved;
};

#define NR_BREAKPOINTS	64

static struct bkpt breakpoints[NR_BREAKPOINTS];

static inline void dcache_flush_range(unsigned long start, unsigned long end)
{
	asm volatile("mcrr p15, 0, %0, %1, c12"
		     :: "r"(end), "r"(start) : "memory");
}

static inline void icache_inval_range(unsigned long start, unsigned long end)
{
	asm volatile("mcrr p15, 0, %0, %1, c5"
		     :: "r"(end), "r"(start) : "memory");
}

static inline void prefetch_flush(void)
{
	asm volatile("mcr p15, 0, %0, c7, c5, 4" :: "r"(0) : "memory");
}

static inline void btc_flush(void)
{
	asm volatile("mcr p15, 0, %0, c7, c5, 6" :: "r"(0) : "memory");
}

static inline void icache_sync(unsigned long addr, size_t len)
{
	unsigned long start = addr, end = addr + len + L1_CACHE_LINE_SZ;

	start &= ~(L1_CACHE_LINE_SZ - 1);
	end &= ~(L1_CACHE_LINE_SZ - 1);

	prefetch_flush();
	btc_flush();
	dcache_flush_range(start, end);
	icache_inval_range(start, end);
}

void gdbstub_io_handler(struct arm_regs *regs);

static void barrier(void)
{
	asm volatile("mcr p15, 0, %0, c7, c10, 4" :: "r"(0) : "memory");
}

static struct bkpt *alloc_bkpt(void)
{
	unsigned int m;

	for (m = 0; m < ARRAY_SIZE(breakpoints); ++m)
		if (!breakpoints[m].addr)
			return &breakpoints[m];

	return NULL;
}

static struct bkpt *find_bkpt(uint32_t *addr)
{
	unsigned int m;

	for (m = 0; m < ARRAY_SIZE(breakpoints); ++m)
		if (breakpoints[m].addr == addr)
			return &breakpoints[m];

	return NULL;
}

static void free_bkpt(struct bkpt *b)
{
	b->addr = NULL;
}

static void activate_bkpt(struct bkpt *b)
{
	uint32_t inst = SMC_BKPT;

	if (b->saved == inst)
		return;

	memcpy(&b->saved, b->addr, sizeof(b->saved));
	memcpy(b->addr, &inst, sizeof(inst));
	icache_sync((unsigned long)b->addr, sizeof(inst));
}

static void deactivate_bkpt(struct bkpt *b)
{
	memcpy(b->addr, &b->saved, sizeof(b->saved));
	icache_sync((unsigned long)b->addr, sizeof(b->saved));
}

#define IRQC_BASE			0x2000B000
#define INT_PENDING_REG_OFFS		0x200
#define INT_PENDING1_REG_OFFS		0x204
#define INT_PENDING2_REG_OFFS		0x208
#define INT_FIQ_CONTROL_REG_OFFS	0x20c
#define		FIQ_ENABLE		(1 << 7)
#define INT_ENABLE1_REG_OFFS		0x210
#define INT_ENABLE2_REG_OFFS		0x214
#define INT_BASIC_EN_REG_OFFS		0x218
#define INT_DISABLE1_REG_OFFS		0x21c
#define INT_DISABLE2_REG_OFFS		0x220
#define INT_DISABLE_BASIC_REG_OFFS	0x224

void mon_panic(void)
{
	BUG("monitor paniced!");
}

static inline uint32_t bswap32(uint32_t v)
{
	return __builtin_bswap32(v);
}

static inline uint16_t bswap16(uint16_t v)
{
	return ((v & 0xff) << 8) | ((v & 0xff00) >> 8);
}

int read_mem(void *dst, unsigned long addr, size_t len)
{
	switch (len) {
	case 4:
		*(uint32_t *)dst =
			bswap32(*(const volatile uint32_t *)addr);
		break;
	case 2:
		*(uint16_t *)dst =
			bswap16(*(const volatile uint16_t *)addr);
		break;
	case 1:
		*(uint8_t *)dst = *(const volatile uint8_t *)addr;
		break;
	default:
		return -1;
	}

	barrier();

	return 0;
}

int write_mem(const void *src, unsigned long addr, size_t len)
{
	switch (len) {
	case 4:
		*(volatile uint32_t *)addr =
			bswap32(*(const uint32_t *)src);
		break;
	case 2:
		*(volatile uint16_t *)addr =
			bswap16(*(const uint16_t *)src);
		break;
	case 1:
		*(volatile uint8_t *)addr = *(const uint8_t *)src;
		break;
	default:
		return -1;
	}

	icache_sync(addr, len);

	return 0;
}

static ssize_t gdbstub_writev(const struct iovec *vec,
			      size_t count)
{
	const struct iovec *v;
	const char *p;
	ssize_t written = 0;

	for (v = vec; v < vec + count; ++v)
		for (p = v->iov_base; p < v->iov_base + v->iov_len;
		     ++p, ++written)
			uart_putc(*p);

	return written;
}

static int send_response(const char *msg, size_t len)
{
	char pfx = '$';
	char csum[4];
	struct iovec vec[3] = {
		{ .iov_base = &pfx, .iov_len = 1 },
		{ .iov_base = (void *)msg, .iov_len = len },
		{ .iov_base = csum, .iov_len = 3 },
	};
	uint8_t c = 0;
	unsigned int m;

	for (m = 0; m < len; ++m)
		c += msg[m];
	csum[0] = '#';
	put_hex_byte(csum + 1, c);

	return gdbstub_writev(vec, ARRAY_SIZE(vec));
}

static void hit_breakpoint(struct bkpt *bp)
{
	const char resp[] = "S05";

	running = false;

	send_response(resp, strlen(resp));
}

void gdbstub_smc_handler(struct arm_regs *regs)
{
	unsigned int m;

	for (m = 0; m < ARRAY_SIZE(breakpoints); ++m)
		if (breakpoints[m].addr == (uint32_t *)regs->r[PC])
			hit_breakpoint(&breakpoints[m]);
	gdbstub_io_handler(regs);
}

static int gdbstub_recv_byte(char *b, bool wait)
{
	*b = uart_getc();

	return 1;
}

static int gdbstub_break(void)
{
	const char resp[] = "S05";

	running = false;

	return send_response(resp, strlen(resp));
}

static uint8_t hex_nibble_val(uint8_t v)
{
	if (v >= '0' && v <= '9')
		return v - '0';
	return (v - 'a') + 10;
}

static int read_csum(uint8_t *dst)
{
	char b[3] = {};

	b[0] = uart_getc();
	b[1] = uart_getc();

	*dst = hex_nibble_val(b[0]) * 16 + hex_nibble_val(b[1]);

	return 0;
}

static void ack_packet(bool is_good)
{
	uart_putc(is_good ? '+' : '-');
}

static int unsupported_op(const char *msg)
{
	return send_response(NULL, 0);
}

struct cmdhandler {
	const char *pfx;
	int (*handle)(struct arm_regs *regs, const char *msg);
};

static int qsupported(struct arm_regs *regs, const char *msg)
{
	const char resp[] = "qSupported:PacketSize=256;multiprocess+";

	return send_response(resp, strlen(resp));
}

static int stop_status(struct arm_regs *regs, const char *msg)
{
	const char resp[] = "S05";

	return send_response(resp, strlen(resp));
}

static unsigned long read_hex(const char *p, char **next)
{
	unsigned long v = 0;

	while (is_hex(*p)) {
		v *= 16;
		v += hex_nibble_val(*p);
		++p;
	}

	if (next)
		*next = (char *)p;

	return v;
}

static int gdb_read_mem(struct arm_regs *regs, const char *msg)
{
	char *resp = NULL;
	unsigned long addr;
	char *next;

	msg += 1;
	addr = read_hex(msg, &next);
	if (*next != ',') {
		resp = "E01";
	} else {
		unsigned long len = read_hex(next + 1, NULL);
		unsigned int m;
		uint32_t v;

		resp = cmd_buf;
		for (m = 0; m < len; ++m) {
			if (read_mem(&v, addr + m, 1))
				break;
			put_hex_byte(resp + m * 2, v & 0xff);
		}
	}

	return send_response(resp, strlen(resp));
}

static int read_field(const char **p, uint32_t *dst, char delim)
{
	const char *del = strchr(*p, delim);
	char *next;

	if (!del)
		return -1;

	*dst = read_hex(*p, &next);
	if (next != del)
		return -1;
	*p = next + 1;

	return 0;
}

static int read_addr(const char **p, uint32_t *addr)
{
	return read_field(p, addr, ',');
}

static int read_len(const char **p, uint32_t *len)
{
	return read_field(p, len, ':');
}

static int gdb_write_mem(struct arm_regs *regs, const char *msg)
{
	const char *resp;
	uint32_t addr, len;
	const char *p;
	uint32_t v;

	p = msg + 1;
	if (read_addr(&p, &addr) ||
	    read_len(&p, &len) ||
	    strlen(p) != len * 2) {
		resp = "E01";
		goto out;
	}

	v = read_hex(p, NULL);
	if (write_mem(&v, addr, len)) {
		resp = "E02";
		goto out;
	}

	resp = "OK";

out:
	return send_response(resp, strlen(resp));
}

/* Print a register in little-endian (ARM) byte-order. */
static ssize_t put_reg(char *dst, uint32_t r)
{
	unsigned int m;

	for (m = 0; m < 4; ++m) {
		put_hex_byte(dst, r & 0xff);
		dst += 2;
		r >>= 8;
	}

	return 8;
}

static ssize_t put_invalid_reg(char *dst)
{
	const char *v = "xxxxxxxx";

	memcpy(dst, v, strlen(v) + 1);

	return 8;
}

/*
 * Extra registers that we don't support but GDB expects in the 'g' command.
 * This is the FPA registers normally.
 */
#define EXTRA_REGS 25
static int read_regs(struct arm_regs *regs, const char *msg)
{
	unsigned int m;
	char *p = cmd_buf;

	for (m = R0; m <= PC; ++m)
		p += put_reg(p, regs->r[m]);
	for (m = 0; m < EXTRA_REGS; ++m)
		p += put_invalid_reg(p);
	p += put_reg(p, regs->r[CPSR]);

	return send_response(cmd_buf, (17 + EXTRA_REGS) * 8);
}

static uint32_t reg_from_str(const char *str)
{
	char s[9];

	memcpy(s, str, 8);
	s[8] = '\0';
	return bswap32(read_hex(s, NULL));
}

static int write_regs(struct arm_regs *regs, const char *msg)
{
	const char *resp;
	unsigned int m;

	msg++;
	if (strlen(msg) != (17 + EXTRA_REGS) * 8) {
		resp = "E 01";
		goto out;
	}

	for (m = R0; m <= PC; ++m)
		regs->r[m] = reg_from_str(msg + m * 8);
	regs->r[CPSR] = reg_from_str(msg + (16 + EXTRA_REGS) * 8);

	resp = "OK";

out:
	return send_response(resp, strlen(resp));
}

static int do_continue(struct arm_regs *regs, const char *msg)
{
	const char *p = strchr(msg, ';');

	if (!p)
		return send_response(NULL, 0);

	p++;
	switch (*p) {
	case 'c':
		running = true;
		break;

	case 't':
		running = false;
		break;

	default:
		return send_response(NULL, 0);
	}

	return 1;
}

static int vcont_query(struct arm_regs *regs, const char *msg)
{
	const char resp[] = "vCont;c;C;s;S";

	return send_response(resp, strlen(resp));
}

static int qattached(struct arm_regs *regs, const char *msg)
{
	const char resp[] = "1";

	return send_response(resp, strlen(resp));
}

static int detach(struct arm_regs *regs, const char *msg)
{
	const char resp[] = "OK";

	running = true;

	return send_response(resp, strlen(resp));
}

static int add_bkpt(uint32_t *addr)
{
	struct bkpt *bp = find_bkpt(addr);

	if (bp) {
		activate_bkpt(bp);
		return 0;
	}

	bp = alloc_bkpt();
	bp->saved = 0;
	bp->addr = addr;
	activate_bkpt(bp);

	return 0;
}

static int del_bkpt(uint32_t *addr)
{
	struct bkpt *bp = find_bkpt(addr);

	if (!bp)
		return -1;

	deactivate_bkpt(bp);
	free_bkpt(bp);

	return 0;
}

static int bkpt_op(const char *msg, int (*op)(uint32_t *addr))
{
	uint32_t addr;
	const char *p = msg;
	const char *resp;

	p += 3;
	if (read_field(&p, &addr, ',')) {
		resp = "E01";
		goto out;
	}

	resp = !op((uint32_t *)addr) ? "OK" : "E01";

out:
	return send_response(resp, strlen(resp));
}

static int insert_bkpt(struct arm_regs *regs, const char *msg)
{
	return bkpt_op(msg, add_bkpt);
}

static int remove_bkpt(struct arm_regs *regs, const char *msg)
{
	return bkpt_op(msg, del_bkpt);
}

static const struct cmdhandler handlers[] = {
	{ .pfx = "qSupported:", .handle = qsupported },
	{ .pfx = "?", .handle = stop_status },
	{ .pfx = "g", .handle = read_regs },
	{ .pfx = "G", .handle = write_regs },
	{ .pfx = "m", .handle = gdb_read_mem },
	{ .pfx = "M", .handle = gdb_write_mem },
	{ .pfx = "z1", .handle = remove_bkpt },
	{ .pfx = "Z1", .handle = insert_bkpt },
	{ .pfx = "z0", .handle = remove_bkpt },
	{ .pfx = "Z0", .handle = insert_bkpt },
	{ .pfx = "vCont?", .handle = vcont_query },
	{ .pfx = "vCont", .handle = do_continue },
	{ .pfx = "qAttached", .handle = qattached },
	{ .pfx = "D", .handle = detach },
};

static const struct cmdhandler *lookup_cmdhandler(const char *msg)
{
	unsigned int m;

	for (m = 0; m < ARRAY_SIZE(handlers); ++m)
		if (!strncmp(handlers[m].pfx, msg, strlen(handlers[m].pfx)))
			return &handlers[m];

	return NULL;
}

static void unescape(char *msg)
{
	char *w = msg, *r = msg;

	while (*r) {
		char v = *r++;
		if (v != '}') {
			*w++ = v;
			continue;
		}

		*w++ = *r++ ^ 0x20;
	}
	*w = 0;
}

/*
 * Process a request.  We've already read the '$', but nothing else.
 */
static int gdbstub_handle(struct arm_regs *regs)
{
	char *p = cmd_buf;
	int rc;
	uint8_t recvd_csum, csum = 0;
	const struct cmdhandler *handler;

	for (;;) {
		rc = gdbstub_recv_byte(p, true);
		if (rc <= 0)
			return rc;

		if (*p == '#')
			break;
		csum += *p++;
	}
	*p = '\0';

	if (read_csum(&recvd_csum))
		return -1;

	ack_packet(recvd_csum == csum);

	unescape(cmd_buf);

	handler = lookup_cmdhandler(cmd_buf);

	return handler ? handler->handle(regs, cmd_buf) :
		unsupported_op(cmd_buf);
}

void gdbstub_io_handler(struct arm_regs *regs)
{
	do {
		int c = uart_getc();

		switch (c) {
		case '+': /* ACK. */
			break;

		case 0x03: /* Stop execution. */
			gdbstub_break();
			break;

		case '$': /* Command. */
			gdbstub_handle(regs);
			break;

		case '-':
			BUG("transmission error");

		default: /* Unsupported operation. */
			unsupported_op(NULL);
		}
	} while (!running);
}

#define UART_INT		57

static void fiq_init(void)
{
	writel(IRQC_BASE + INT_DISABLE1_REG_OFFS, ~0LU);
	writel(IRQC_BASE + INT_DISABLE2_REG_OFFS, ~0LU);
	writel(IRQC_BASE + INT_DISABLE_BASIC_REG_OFFS, ~0LU);
	writel(IRQC_BASE + INT_FIQ_CONTROL_REG_OFFS,
	       FIQ_ENABLE | UART_INT);
}

extern void init_monitor(void *stack_top);

#define MON_STACK_WORDS	1024
unsigned long monitor_stack[MON_STACK_WORDS];

void gdbstub_init(void)
{
	init_monitor(monitor_stack + MON_STACK_WORDS - 16);
	fiq_init();
	uart_enable(UART_INT_ENABLE);
	uart_flush_rx();
}
