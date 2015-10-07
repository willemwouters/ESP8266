#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"


/////////////// CREDITS TO https://github.com/cesanta/smart.js  ////////////////


#define EXCCAUSE_ILLEGAL		0	/* Illegal Instruction */
#define EXCCAUSE_SYSCALL		1	/* System Call (SYSCALL instruction) */
#define EXCCAUSE_INSTR_ERROR		2	/* Instruction Fetch Error */
# define EXCCAUSE_IFETCHERROR		2	/* (backward compatibility macro, deprecated, avoid) */
#define EXCCAUSE_LOAD_STORE_ERROR	3	/* Load Store Error */
# define EXCCAUSE_LOADSTOREERROR	3	/* (backward compatibility macro, deprecated, avoid) */
#define EXCCAUSE_LEVEL1_INTERRUPT	4	/* Level 1 Interrupt */
# define EXCCAUSE_LEVEL1INTERRUPT	4	/* (backward compatibility macro, deprecated, avoid) */
#define EXCCAUSE_ALLOCA			5	/* Stack Extension Assist (MOVSP instruction) for alloca */
#define EXCCAUSE_DIVIDE_BY_ZERO		6	/* Integer Divide by Zero */
#define EXCCAUSE_SPECULATION		7	/* Use of Failed Speculative Access (not implemented) */
#define EXCCAUSE_PRIVILEGED		8	/* Privileged Instruction */
#define EXCCAUSE_UNALIGNED		9	/* Unaligned Load or Store */
/* Reserved				10..11 */
#define EXCCAUSE_INSTR_DATA_ERROR	12	/* PIF Data Error on Instruction Fetch (RB-200x and later) */
#define EXCCAUSE_LOAD_STORE_DATA_ERROR	13	/* PIF Data Error on Load or Store (RB-200x and later) */
#define EXCCAUSE_INSTR_ADDR_ERROR	14	/* PIF Address Error on Instruction Fetch (RB-200x and later) */
#define EXCCAUSE_LOAD_STORE_ADDR_ERROR	15	/* PIF Address Error on Load or Store (RB-200x and later) */
#define EXCCAUSE_ITLB_MISS		16	/* ITLB Miss (no ITLB entry matches, hw refill also missed) */
#define EXCCAUSE_ITLB_MULTIHIT		17	/* ITLB Multihit (multiple ITLB entries match) */
#define EXCCAUSE_INSTR_RING		18	/* Ring Privilege Violation on Instruction Fetch */
/* Reserved				19 */	/* Size Restriction on IFetch (not implemented) */
#define EXCCAUSE_INSTR_PROHIBITED	20	/* Cache Attribute does not allow Instruction Fetch */
/* Reserved				21..23 */
#define EXCCAUSE_DTLB_MISS		24	/* DTLB Miss (no DTLB entry matches, hw refill also missed) */
#define EXCCAUSE_DTLB_MULTIHIT		25	/* DTLB Multihit (multiple DTLB entries match) */
#define EXCCAUSE_LOAD_STORE_RING	26	/* Ring Privilege Violation on Load or Store */
/* Reserved				27 */	/* Size Restriction on Load/Store (not implemented) */
#define EXCCAUSE_LOAD_PROHIBITED	28	/* Cache Attribute does not allow Load */
#define EXCCAUSE_STORE_PROHIBITED	29	/* Cache Attribute does not allow Store */
/* Reserved				30..31 */
#define EXCCAUSE_CP_DISABLED(n)		(32+(n))	/* Access to Coprocessor 'n' when disabled */
#define EXCCAUSE_CP0_DISABLED		32	/* Access to Coprocessor 0 when disabled */
#define EXCCAUSE_CP1_DISABLED		33	/* Access to Coprocessor 1 when disabled */
#define EXCCAUSE_CP2_DISABLED		34	/* Access to Coprocessor 2 when disabled */
#define EXCCAUSE_CP3_DISABLED		35	/* Access to Coprocessor 3 when disabled */
#define EXCCAUSE_CP4_DISABLED		36	/* Access to Coprocessor 4 when disabled */
#define EXCCAUSE_CP5_DISABLED		37	/* Access to Coprocessor 5 when disabled */
#define EXCCAUSE_CP6_DISABLED		38	/* Access to Coprocessor 6 when disabled */
#define EXCCAUSE_CP7_DISABLED		39	/* Access to Coprocessor 7 when disabled */
/*#define EXCCAUSE_FLOATING_POINT	40*/	/* Floating Point Exception (not implemented) */
/* Reserved				40..63 */


char causes[] = {EXCCAUSE_ILLEGAL,          EXCCAUSE_INSTR_ERROR,
                   EXCCAUSE_LOAD_STORE_ERROR, EXCCAUSE_DIVIDE_BY_ZERO,
                   EXCCAUSE_UNALIGNED,        EXCCAUSE_INSTR_PROHIBITED,
                   EXCCAUSE_LOAD_PROHIBITED,  EXCCAUSE_STORE_PROHIBITED};


struct regfile {
  uint32_t a[16];
  uint32_t pc;
  uint32_t sar;
  uint32_t litbase;
  uint32_t sr176;
  uint32_t sr208;
  uint32_t ps;
};





#define NUM_UPPERCASES ('Z' - 'A' + 1)
#define NUM_LETTERS (NUM_UPPERCASES * 2)
#define NUM_DIGITS ('9' - '0' + 1)

typedef void (*cs_base64_putc_t)(char, void *);

struct cs_base64_ctx {
  /* cannot call it putc because it's a macro on some environments */
  cs_base64_putc_t b64_putc;
  unsigned char chunk[3];
  int chunk_size;
  void *user_data;
};

#define UART_BASE(i) (0x60000000 + (i) *0xf00)
#define UART_INTR_STATUS(i) (UART_BASE(i) + 0x8)
#define UART_FORMAT_ERROR (BIT(3))
#define UART_RXBUF_FULL (BIT(0))
#define UART_RX_NEW (BIT(8))
#define UART_TXBUF_EMPTY (BIT(1))
#define UART_CTRL_INTR(i) (UART_BASE(i) + 0xC)
#define UART_CLEAR_INTR(i) (UART_BASE(i) + 0x10)
#define UART_DATA_STATUS(i) (UART_BASE(i) + 0x1C)
#define UART_BUF(i) UART_BASE(i)


void uart_tx_char(unsigned uartno, char ch) {
  while (1) {
    uint32 fifo_cnt =
        (READ_PERI_REG(UART_DATA_STATUS(uartno)) & 0x00FF0000) >> 16;
    if (fifo_cnt < 126) {
      break;
    }
  }
  WRITE_PERI_REG(UART_BUF(uartno), ch);
}

void uart_putchar(char ch) {

  if (ch == '\n') uart_tx_char(0, '\r');
  uart_tx_char(0, ch);
}

/* output an unsigned decimal integer */
static void uart_putdec(unsigned int n) {
  unsigned int tmp;
  unsigned long long p = 1;

  for (tmp = n; tmp > 0; tmp /= 10) {
    p *= 10;
  }
  p /= 10;
  if (p == 0) {
    p = 1;
  }

  for (; p > 0; p /= 10) {
    uart_putchar('0' + (unsigned int) (n / p) % 10);
  }
}

static int core_dump_emit_char_fd = 0;
static void core_dump_emit_char(char c, void *user_data) {
  int *col_counter = (int *) user_data;
#ifdef RTOS_SDK
  system_soft_wdt_feed();
#endif
  (*col_counter)++;
  uart_putchar(c);
  if (*col_counter >= 160) {
    uart_putchar('\n');
    (*col_counter) = 0;
  }
}

/*
 * Emit a base64 code char.
 *
 * Doesn't use memory, thus it's safe to use to safely dump memory in crashdumps
 */
static void cs_base64_emit_code(struct cs_base64_ctx *ctx, int v) {
  if (v < NUM_UPPERCASES) {
    ctx->b64_putc(v + 'A', ctx->user_data);
  } else if (v < (NUM_LETTERS)) {
    ctx->b64_putc(v - NUM_UPPERCASES + 'a', ctx->user_data);
  } else if (v < (NUM_LETTERS + NUM_DIGITS)) {
    ctx->b64_putc(v - NUM_LETTERS + '0', ctx->user_data);
  } else {
    ctx->b64_putc(v - NUM_LETTERS - NUM_DIGITS == 0 ? '+' : '/',
                  ctx->user_data);
  }
}

static void cs_base64_emit_chunk(struct cs_base64_ctx *ctx) {
  int a, b, c;

  a = ctx->chunk[0];
  b = ctx->chunk[1];
  c = ctx->chunk[2];

  cs_base64_emit_code(ctx, a >> 2);
  cs_base64_emit_code(ctx, ((a & 3) << 4) | (b >> 4));
  if (ctx->chunk_size > 1) {
    cs_base64_emit_code(ctx, (b & 15) << 2 | (c >> 6));
  }
  if (ctx->chunk_size > 2) {
    cs_base64_emit_code(ctx, c & 63);
  }
}

void cs_base64_init(struct cs_base64_ctx *ctx, cs_base64_putc_t b64_putc,
                    void *user_data) {
  ctx->chunk_size = 0;
  ctx->b64_putc = b64_putc;
  ctx->user_data = user_data;
}

void cs_base64_update(struct cs_base64_ctx *ctx, const char *str, size_t len) {
  const unsigned char *src = (const unsigned char *) str;
  size_t i;
  for (i = 0; i < len; i++) {
    ctx->chunk[ctx->chunk_size++] = src[i];
    if (ctx->chunk_size == 3) {
      cs_base64_emit_chunk(ctx);
      ctx->chunk_size = 0;
    }
  }
}

void cs_base64_finish(struct cs_base64_ctx *ctx) {
  if (ctx->chunk_size > 0) {
    int i;
    memset(&ctx->chunk[ctx->chunk_size], 0, 3 - ctx->chunk_size);
    cs_base64_emit_chunk(ctx);
    for (i = 0; i < (3 - ctx->chunk_size); i++) {
      ctx->b64_putc('=', ctx->user_data);
    }
  }
}




/* address must be aligned to 4 and size must be multiple of 4 */
static void emit_core_dump_section(const char *name, uint32_t addr,
                                   uint32_t size) {
struct cs_base64_ctx ctx;


  int col_counter = 0;

  cs_base64_init(&ctx, core_dump_emit_char, &col_counter);
  uint32_t end = addr + size;
  while (addr < end) {
	//char buff;
	//buff = *((char *) addr);

//    uint32_t buf;
//    buf = *((uint32_t *) addr);
    addr += sizeof(uint32_t);
    system_soft_wdt_feed();
    //cs_base64_update(&ctx, (char *) &buf, sizeof(uint32_t));
  }
  cs_base64_finish(&ctx);
  os_printf("\"}");
}

void esp_dump_core(struct regfile *regs) {


	os_printf("{\"arch\": \"ESP8266\"");
  /* rtos relocates vectors here */
  //emit_core_dump_section("REGS", &regs, sizeof(*regs));
//  emit_core_dump_section("DRAM", 0x3FFE8000, 0x18000);
//  emit_core_dump_section("VEC", 0x40100000, 0x1000);
//  emit_core_dump_section("ROM", 0x40000000, 0x10000);


  os_printf("}\n");

}

struct xtensa_stack_frame {
  uint32_t pc; /* instruction causing the trap */
  uint32_t ps;
  uint32_t sar;
  uint32_t vpri;  /* current xtos virtual priority */
  uint32_t a0;    /* when __XTENSA_CALL0_ABI__ is true */
  uint32_t a[16]; /* a2 - a15 */
};

#define ESP_EXC_SP_OFFSET 0x100
#define EXCCAUSE	232
#define DEBUGCAUSE	233
#define EXCVADDR	238
#define LITBASE		5

#define __stringify_1(x...) #x
#define __stringify(x...) __stringify_1(x)
#define RSR(sr)                                       \
  ({                                                  \
    uint32_t r;                                       \
    asm volatile("rsr %0,"__stringify(sr) : "=a"(r)); \
    r;                                                \
  })

void dump_frame(struct xtensa_stack_frame * frame) {
	int i;
	  os_printf("pc=%p \n", (void *) frame->pc);
	  os_printf(",\"");

		os_printf("REG");
		os_printf("\": {\"addr\": ");
		os_printf("%p", frame);
		os_printf(", \"data\": a0=%p", frame->a0);

	for(i = 0; i < 16; i++) {
		os_printf("a%d=%p ", i, frame->a[i]);
	}
	os_printf("\n");
}
void esp_exception_handler(struct xtensa_stack_frame *frame) {
//  uint32_t cause = RSR(EXCCAUSE);
//  uint32_t vaddr = RSR(EXCVADDR);
//  ets_wdt_disable();
//
//  os_printf("\nTrap %d: pc=%p va=%p\n", cause, (void *) frame->pc, (void *) vaddr);
//  memcpy(&regs.a[2], frame->a, sizeof(frame->a));
//
//  regs.a[0] = frame->a0;
//  regs.a[1] = (uint32_t) frame + ESP_EXC_SP_OFFSET;
//  regs.pc = frame->pc;
//  regs.sar = frame->sar;
//  regs.ps = frame->ps;
//  regs.litbase = RSR(LITBASE);
//
//  esp_dump_core(&regs);
//  _ResetVector();

	 uint32_t cause = RSR(EXCCAUSE);
	  uint32_t vaddr = RSR(EXCVADDR);
	  os_printf("--- BEGIN CORE DUMP ---\n");

	  os_printf("\nTrap %d: \n", cause);

	  dump_frame(frame);

	  struct xtensa_stack_frame *frame2 = (struct xtensa_stack_frame *) frame->a[9];
	  dump_frame(frame2);
//
//	  dump_frame((struct xtensa_stack_frame *) frame2->a[14]);
//	  dump_frame((struct xtensa_stack_frame *) frame2->a[15]);
	  dump_frame((struct xtensa_stack_frame *) frame->a[2]);
	  dump_frame((struct xtensa_stack_frame *) frame->a[13]);
	  //esp_dump_core(&regs);

	  os_printf("---- END CORE DUMP ----\n");
	  while (1) {
	      system_soft_wdt_feed();
	    }
}


void debug_init() {

	int i;
		  for (i = 0; i < (int) sizeof(causes); i++) {
		    _xtos_set_exception_handler(causes[i], esp_exception_handler);
		  }
}

