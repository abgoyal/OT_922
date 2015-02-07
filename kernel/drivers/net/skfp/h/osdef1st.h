


// HWM (HardWare Module) Definitions
// -----------------------

#include <asm/byteorder.h>

#ifdef __LITTLE_ENDIAN
#define LITTLE_ENDIAN
#else
#define BIG_ENDIAN
#endif

// this is set in the makefile
// #define PCI			/* only PCI adapters supported by this driver */
// #define MEM_MAPPED_IO	/* use memory mapped I/O */


#define USE_CAN_ADDR		/* DA and SA in MAC header are canonical. */

#define MB_OUTSIDE_SMC		/* SMT Mbufs outside of smc struct. */

// -----------------------


// SMT Definitions 
// -----------------------
#define SYNC	       		/* allow synchronous frames */

// #define SBA			/* Synchronous Bandwidth Allocator support */
				/* not available as free source */

#define ESS			/* SBA End Station Support */

#define	SMT_PANIC(smc, nr, msg)	printk(KERN_INFO "SMT PANIC: code: %d, msg: %s\n",nr,msg)


#ifdef DEBUG
#define printf(s,args...) printk(KERN_INFO s, ## args)
#endif

// #define HW_PTR	u_long
// -----------------------



// HWM and OS-specific buffer definitions
// -----------------------

// default number of receive buffers.
#define NUM_RECEIVE_BUFFERS		10

// default number of transmit buffers.
#define NUM_TRANSMIT_BUFFERS		10

// Number of SMT buffers (Mbufs).
#define NUM_SMT_BUF	4

// Number of TXDs for asynchronous transmit queue.
#define HWM_ASYNC_TXD_COUNT	(NUM_TRANSMIT_BUFFERS + NUM_SMT_BUF)

// Number of TXDs for synchronous transmit queue.
#define HWM_SYNC_TXD_COUNT	HWM_ASYNC_TXD_COUNT


// Number of RXDs for receive queue #1.
// Note: Workaround for ASIC Errata #7: One extra RXD is required.
#if (NUM_RECEIVE_BUFFERS > 100)
#define SMT_R1_RXD_COUNT	(1 + 100)
#else
#define SMT_R1_RXD_COUNT	(1 + NUM_RECEIVE_BUFFERS)
#endif

// Number of RXDs for receive queue #2.
#define SMT_R2_RXD_COUNT	0	// Not used.
// -----------------------




struct s_txd_os {	// os-specific part of transmit descriptor
	struct sk_buff *skb;
	dma_addr_t dma_addr;
} ;

struct s_rxd_os {	// os-specific part of receive descriptor
	struct sk_buff *skb;
	dma_addr_t dma_addr;
} ;



#define AIX_REVERSE(x)		((u32)le32_to_cpu((u32)(x)))
#define MDR_REVERSE(x)		((u32)le32_to_cpu((u32)(x)))