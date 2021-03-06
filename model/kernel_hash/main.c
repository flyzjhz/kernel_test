#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/kprobes.h>
#include <linux/limits.h> 
#include "debug.h"
#include "nf.h"
#include "chunk.h"
#include "sha.h"
#include "hash_table.h"
#include "bitmap.h"
#include "slab_cache.h"
#include "alloc_file.h"
	
unsigned long RM = 1;
unsigned long zero_value = 1;
int zero_num = 6;
unsigned long Q = 1;
unsigned long R = 1048583;
int chunk_num = 32;  //控制最小值
static int kprobe_in_reged = 0;
DECLARE_PER_CPU(unsigned long *, bitmap); //percpu-BITMAP
struct workqueue_struct *skb_wq;
unsigned long long used_mem = 0ULL;

void init_hash_parameters(void)
{
	int i;
	
	// precalculate
	for (i = 0; i < 60; ++i)
		Q = (2 * Q);

	for (i = 1; i <= chunk_num - 1; i++)
		RM = (R * RM) % Q;

	for (i = 0; i < zero_num; ++i)
        zero_value = (2 * zero_value);
    zero_value = zero_value - 1;
}

int init_some_parameters(void)
{	
	int cpu;

	atomic64_set(&save_num, 0L);
	atomic64_set(&sum_num, 0L);
	atomic64_set(&skb_num, 0L);
	atomic64_set(&rdl, 0L);
	atomic64_set(&rdf, 0L);
	
	skb_wq = create_workqueue("kread_queue");
	if (!skb_wq)
		return -1;
	
	for_each_online_cpu(cpu) {
		INIT_LIST_HEAD(&per_cpu(skb_list, cpu));
	}
	
	return 0;
}

static int minit(void)
{
	int err = 0;

	init_hash_parameters();

	if (0 > (err = init_some_parameters()))
		goto out;

	if (0 > (err = alloc_percpu_file()))
		goto err_alloc_file;

	if (0 > (err = alloc_slab()))
		goto err_alloc_slab;

	if (0 > (err = alloc_bitmap()))
		goto err_bitmap;

	if (0 > (err = initial_hash_table_cache()))
		goto err_hash_table_cache;

	printk(KERN_INFO "Start %s.", THIS_MODULE->name);

	if (0 > (err = nf_register_hook(&nf_out_ops))) {
		printk(KERN_ERR "Failed to register nf_out %s.\n", THIS_MODULE->name);
		goto err_nf_reg_out;
	}

	if (0 > (err = nf_register_hook(&nf_in_ops))) {
		printk(KERN_ERR "Failed to register nf_in %s.\n", THIS_MODULE->name);
		goto err_nf_reg_in;
	}    
	
	if (tcp_alloc_sha1sig_pool() == NULL) { 
		printk(KERN_ERR "Failed to alloc sha1 pool %s.\n", THIS_MODULE->name);
		goto err_sha1siq_pool;
	}   

	err = register_jprobe(&jps_netif_receive_skb);
    if (err < 0) {
        printk(KERN_ERR "Failed to register jprobe netif_receive_skb %s.\n", THIS_MODULE->name);
        goto out;
    }
    kprobe_in_reged = 1; 

	goto out;

err_sha1siq_pool:
	tcp_free_sha1sig_pool();
err_nf_reg_in:
	nf_unregister_hook(&nf_in_ops);
err_nf_reg_out:
	nf_unregister_hook(&nf_out_ops);
err_hash_table_cache:
	release_hash_table_cache();
err_bitmap:
	free_bitmap();
err_alloc_slab:
	free_slab();
err_alloc_file:
	free_percpu_file();
out:
	return err;    
}

static void mexit(void)
{
	/* free the hash table contents */
	long tmp_save, tmp_sum;
	
	nf_unregister_hook(&nf_in_ops);
	nf_unregister_hook(&nf_out_ops);
	
	if (kprobe_in_reged)
        unregister_jprobe(&jps_netif_receive_skb);
	
	flush_workqueue(skb_wq);
	clear_remainder_skb();
	destroy_workqueue(skb_wq);

	release_hash_table_cache();
	free_percpu_file();
	free_slab();
	free_bitmap();	
	tcp_free_sha1sig_pool();
	
	tmp_save = atomic64_read(&save_num);
	tmp_sum =  atomic64_read(&sum_num);

	if (tmp_sum > 0)
		printk(KERN_INFO "Cache ratio is:%ld%%", (tmp_save*100)/tmp_sum);
	
	printk(KERN_INFO "savenum is:%ld; sumnum is:%ld,%ld(Mb);\nExit %s.", tmp_save, tmp_sum, (tmp_sum /1024 /1024 *8), THIS_MODULE->name);
	
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lix");
#ifdef DEBUG
MODULE_VERSION("0.0.1.debug");
#else
MODULE_VERSION("0.0.1");
#endif
