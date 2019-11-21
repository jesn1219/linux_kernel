#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>

#include <linux/list.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/mutex.h>


#define BILLION 1000000000
#define GLOBAL_INDEX_MAX 100000
#define NUMJOBS 4


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


DEFINE_MUTEX(global_mutex);


struct sample_node {
	struct list_head entry;
	int value;
};

int global_index;
struct list_head sample_list[3];





int GLOBAL_PROTECT_NUM;

void protect_on(void) {
	if (GLOBAL_PROTECT_NUM == 1) {
		mutex_lock(&global_mutex);
	} 
}

void protect_off(void) {
	if (GLOBAL_PROTECT_NUM == 1) {
		mutex_unlock(&global_mutex);
	}
}



int insert_thread(void* _arg) {
	int* arg = (int*)_arg;
	printk("mutex_insert_thread is created\n");

	while (true) {
		protect_on();
		if (global_index < GLOBAL_INDEX_MAX) {	
			struct sample_node *new = kmalloc(sizeof(struct sample_node),GFP_KERNEL);
			new->value = global_index;
			list_add(&new->entry, &sample_list[0]);
		} else {
			protect_off();
			break;
		}
		__sync_fetch_and_add(&global_index,1);
		protect_off();
	}

	printk("mutex_insert_thread is finished\n");
	return 0;
}

int search_thread(void* _arg) {
	struct sample_node *current_node = NULL;
	while(true) {
		protect_on();
		if (global_index < GLOBAL_INDEX_MAX) {
			list_for_each_entry(current_node,&sample_list[0],entry) {
			}
		} else {
			protect_off();
			break;
		}
		__sync_fetch_and_add(&global_index,1);
		protect_off();
	}
}	


int delete_thread(void* _arg) {
	struct sample_node *current_node = NULL;
	struct sample_node *tmp_node = NULL;
	while(true) {
		protect_on();
	 	if (global_index < GLOBAL_INDEX_MAX) {
			list_for_each_entry_safe(current_node, tmp_node, &sample_list[0],entry) {
				list_del(&current_node->entry);
				kfree(current_node);
			}
		} else {
			protect_off();
			break;
		}
		__sync_fetch_and_add(&global_index,1);
		protect_off();
	}	
}





void create_and_test_thread(void) {
	int i;
	unsigned long long insert_time;
	unsigned long long search_time;
	unsigned long long delete_time;
	struct timespec tmp_time[2];
	getrawmonotonic(&tmp_time[0]);
	printk("checkpoint a\n");
	global_index = 0;
	for ( i = 0; i < NUMJOBS; i++) {
		kthread_run(&insert_thread,&i,"insert_thread");
	}
	getrawmonotonic(&tmp_time[1]);
	insert_time = cal_clock(tmp_time);

	getrawmonotonic(&tmp_time[0]);
	printk("checkpoint b\n");
	global_index = 0;
	for ( i = 0; i < NUMJOBS; i++) {
		kthread_run(&search_thread,&i,"serach_thread");
	}
	getrawmonotonic(&tmp_time[1]);
	search_time = cal_clock(tmp_time);
	
	getrawmonotonic(&tmp_time[0]);
	printk("check[oint c\n");
	global_index = 0;
	for ( i = 0; i < NUMJOBS; i++) {
		kthread_run(&delete_thread,&i,"delete_thread");
	}
	getrawmonotonic(&tmp_time[1]);
	delete_time = cal_clock(tmp_time);
	
	
	printk("%s linked list insert_time : %llu ns\n", GLOBAL_PROTECT_NUM == 1 ? "mutex" : GLOBAL_PROTECT_NUM == 2 ? "spin_lock" : "RW semaphore", insert_time); 

	printk("%s linked list serach_time : %llu ns\n", GLOBAL_PROTECT_NUM == 1 ? "mutex" : GLOBAL_PROTECT_NUM == 2 ? "spin_lock" : "RW semaphore", insert_time); 

	printk("%s linked list delete_time : %llu ns\n", GLOBAL_PROTECT_NUM == 1 ? "mutex" : GLOBAL_PROTECT_NUM == 2 ? "spin_lock" : "RW semaphore", insert_time); 
}



void test_program(void) {

	printk("check point 2\n");
	INIT_LIST_HEAD(&sample_list[0]);
	INIT_LIST_HEAD(&sample_list[1]);
	INIT_LIST_HEAD(&sample_list[2]);
	global_index = 0;
	printk("check point 3\n");
	GLOBAL_PROTECT_NUM = 1;
	create_and_test_thread();
	printk("check point 4\n");

}



int __init jesnm9_module_init(void) {
	printk("Hello, this is jesn_m9 Module\n");
	test_program();
	return 0;
}






void __exit jesnm9_module_cleanup(void) {
	printk("Bye, this is jesn_m9 Module\n");
	
	return;
}


module_init(jesnm9_module_init);
module_exit(jesnm9_module_cleanup);


MODULE_LICENSE("GPL");
