#include <linux/kfifo.h>
#include <linux/percpu_counter.h>
#include "hash_table.h"

#define CHUNKSTEP 32
#define SHALEN 20
#define KFIFOLEN (1024*sizeof(int))
#define MEMLIMIT (2*100*1024*1024)  /* Memory limit is 200M. */

extern unsigned long RM;
extern unsigned long zero_value;
extern int zero_num;
extern unsigned long Q;
extern unsigned long R;
//extern int R = 10;
extern int chunk_num;  //控制最小值
extern atomic64_t sum_num;
extern atomic64_t save_num;
extern atomic64_t skb_num;
extern atomic64_t rdl;
extern atomic64_t rdf;
extern struct workqueue_struct *writeread_wq; 
extern unsigned long long used_mem;

typedef struct {
	struct work_struct wr_work;
	unsigned long      index;
} w_work_t;

struct reject_skb {
	struct sk_buff *skb;
	struct list_head list;
};

struct read_skb {
	struct hashinfo_item *item;
	struct list_head list;
};

extern struct workqueue_struct *skb_wq;
DECLARE_PER_CPU(struct list_head, skb_list);

/* 
struct tcp_chunk {
	uint8_t *sha;                      
    int id;                           
    UT_hash_handle hh;                
};
*/
void clear_remainder_skb(void);
void calculate_partition(char *playload, int playload_len, struct kfifo *fifo);
