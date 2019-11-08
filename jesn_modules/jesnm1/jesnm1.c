#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/time.h>
#include <linux/slab.h>
#define BILLION 1000000000



int i;
struct my_data_struct {
	int value;
	struct list_head entry;
};



unsigned long long cal_clock(struct timespec* localtime) {
	unsigned long long total_time = 0, time_delay = 0, tmp, tmp_n;
	if (localtime[1].tv_nsec >= localtime[0].tv_nsec) {
		tmp = localtime[1].tv_sec - localtime[0].tv_sec;
		tmp_n = localtime[1].tv_nsec - localtime[0].tv_nsec;
	} else {
		tmp = localtime[1].tv_sec - localtime[0].tv_sec - 1;
		tmp_n = BILLION + localtime[1].tv_nsec - localtime[0].tv_nsec;
	}
	time_delay = BILLION * tmp + tmp_n;
	__sync_fetch_and_add(&total_time, time_delay);
	return total_time;
}



unsigned long long insert_ret[3];
unsigned long long search_ret[3];
unsigned long long delete_ret[3];
int count_idx;
int size_idx;

void get_insert_nodes_benchmark(void) {
	struct list_head lists[3];
	INIT_LIST_HEAD(&lists[0]);
	INIT_LIST_HEAD(&lists[1]);
	INIT_LIST_HEAD(&lists[2]);

	int size_1 = 1000;
	int size_2 = 10000;
	int size_3 = 100000;	
	int size[3] = {size_1,size_2,size_3};	
	struct timespec tmp_time[2];		
 	printk("ADD nodes Benchmark\n");	
	for (size_idx = 0; size_idx < 3; size_idx++) {
		int insert_node_size = size[size_idx];
		getrawmonotonic(&tmp_time[0]);
		for (count_idx = 0; count_idx < insert_node_size; count_idx ++) {
			struct my_data_struct* new = (struct my_data_struct*)
				kmalloc(sizeof(struct my_data_struct),GFP_KERNEL);
			new->value = count_idx;
			list_add(&new->entry, &lists[size_idx]);

		}
		getrawmonotonic(&tmp_time[1]);
		insert_ret[size_idx] = cal_clock(tmp_time);	
		printk("insert %d size : time : %llu\n",size[size_idx],insert_ret[size_idx]);
	}
	printk("Search Benchmark\n");
	struct my_data_struct* cur_node = NULL;
	for ( i = 0; i < 3; i ++) {
		getrawmonotonic(&tmp_time[0]);
		list_for_each_entry(cur_node, &lists[i],entry) {
		}
		getrawmonotonic(&tmp_time[1]);
		search_ret[i] = cal_clock(tmp_time);	
		printk("Search %d size : time : %llu\n",size[i],search_ret[i]);
	}

	printk("Delete node Benchmark\n");
	struct my_data_struct *delete_ptr;
	for (i = 0; i < 3 ; i++) {
		getrawmonotonic(&tmp_time[0]);
		list_for_each_entry_safe(cur_node, delete_ptr, &lists[i],entry) {
			list_del(&cur_node->entry);
			kfree(cur_node);
		}
		getrawmonotonic(&tmp_time[1]);
		delete_ret[i] = cal_clock(tmp_time);
		printk("Delete %d size : time : %llu\n",size[i],delete_ret[i]);
	}

	printk("Time profiling is done\n");
}













int __init jesnm1_module_init(void) {
	printk("Hello, this is jesnm1 Module\n");
	int count;
	struct my_data_struct *obj;


	get_insert_nodes_benchmark();
	//get_search_nodes_benchmark();	
	
	
	return 0;
}







void __exit jesnm1_module_cleanup(void) {
	printk("Bye, this is jesnm1 Module\n");
	
	return;
}
module_init(jesnm1_module_init);
module_exit(jesnm1_module_cleanup);


MODULE_LICENSE("GPL");
