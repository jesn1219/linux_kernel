#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>

#include <linux/list.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>


#define BILLION 1000000000
#define GLOBAL_INDEX_MAX 100000
#define NUMJOBS 4


spinlock_t global_spinlock;
DEFINE_MUTEX(global_mutex);
struct rw_semaphore global_rwsem;



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




struct sample_node {
	struct list_head entry;
	int value;
};

int global_index;
struct list_head sample_list[3];





int GLOBAL_PROTECT_METHOD;

void protect_on(void) {
	if (GLOBAL_PROTECT_METHOD == 1) {
		mutex_lock(&global_mutex);
	} else if (GLOBAL_PROTECT_METHOD == 2) {
		spin_lock(&global_spinlock);
	} else if (GLOBAL_PROTECT_METHOD == 3) {
		down_write(&global_rwsem);
	} else {
	       printk("ERROR\n");
	}	       
}

void protect_off(void) {
	if (GLOBAL_PROTECT_METHOD == 1) {
		mutex_unlock(&global_mutex);
	} else if (GLOBAL_PROTECT_METHOD == 2) {
		spin_unlock(&global_spinlock);
	} else if (GLOBAL_PROTECT_METHOD == 3) {
		up_write(&global_rwsem);
	} else {
		printk("ERROR\n");
	}
}



int insert_thread(void* _arg) {
	int* arg = (int*)_arg;
    int exit = 0;
	while (true) {
		protect_on();
		if (global_index < GLOBAL_INDEX_MAX) {	
			struct sample_node *new = kmalloc(sizeof(struct sample_node),GFP_KERNEL);
			new->value = global_index;
			list_add(&new->entry, &sample_list[GLOBAL_PROTECT_METHOD-1]);
		} else {
            exit = 1;
		}
		__sync_fetch_and_add(&global_index,1);
		protect_off();
        if (exit == 1) {
            break;
        }
	}

	return 0;
}

int search_thread(void* _arg) {
	struct sample_node *current_node = NULL;
    int exit = 0;
	while(true) {
		protect_on();
		if (global_index < GLOBAL_INDEX_MAX) {
			list_for_each_entry(current_node,&sample_list[GLOBAL_PROTECT_METHOD-1],entry) {
			}
		} else {
            exit = 1;
		}
		__sync_fetch_and_add(&global_index,1);
		protect_off();
        if (exit == 1) {
            break;
        }
	}
}	


int delete_thread(void* _arg) {
	struct sample_node *current_node = NULL;
	struct sample_node *tmp_node = NULL;
    int exit = 0;
	while(true) {
		protect_on();
	 	if (global_index < GLOBAL_INDEX_MAX) {
			list_for_each_entry_safe(current_node, tmp_node, &sample_list[GLOBAL_PROTECT_METHOD-1],entry) {
				list_del(&current_node->entry);
				kfree(current_node);
			}
		} else {
            exit = 1;
		}
		__sync_fetch_and_add(&global_index,1);
		protect_off();
        if (exit == 1) {
            break;
        }
	}	
}






void create_and_test_thread(void) {
	int i;
	unsigned long long insert_time;
	unsigned long long search_time;
	unsigned long long delete_time;
	struct timespec tmp_time[2];
	getrawmonotonic(&tmp_time[0]);
	global_index = 0;
	for ( i = 0; i < NUMJOBS; i++) {
		kthread_run(&insert_thread,&i,"insert_thread");
	}
	getrawmonotonic(&tmp_time[1]);
	insert_time = cal_clock(tmp_time);

	getrawmonotonic(&tmp_time[0]);
	global_index = 0;
	for ( i = 0; i < NUMJOBS; i++) {
		kthread_run(&search_thread,&i,"serach_thread");
	}
	getrawmonotonic(&tmp_time[1]);
	search_time = cal_clock(tmp_time);
	
	getrawmonotonic(&tmp_time[0]);
	global_index = 0;
	for ( i = 0; i < NUMJOBS; i++) {
		kthread_run(&delete_thread,&i,"delete_thread");
	}
	getrawmonotonic(&tmp_time[1]);
	delete_time = cal_clock(tmp_time);
	
	
	printk("%s linked list insert_time : %llu ns\n", GLOBAL_PROTECT_METHOD == 1 ? "mutex" : GLOBAL_PROTECT_METHOD == 2 ? "spin_lock" : "RW semaphore", insert_time); 

	printk("%s linked list serach_time : %llu ns\n", GLOBAL_PROTECT_METHOD == 1 ? "mutex" : GLOBAL_PROTECT_METHOD == 2 ? "spin_lock" : "RW semaphore", search_time); 

	printk("%s linked list delete_time : %llu ns\n", GLOBAL_PROTECT_METHOD == 1 ? "mutex" : GLOBAL_PROTECT_METHOD == 2 ? "spin_lock" : "RW semaphore", delete_time); 
}


void test_program(void) {
	spin_lock_init(&global_spinlock);
	init_rwsem(&global_rwsem);


	INIT_LIST_HEAD(&sample_list[0]);
	INIT_LIST_HEAD(&sample_list[1]);
	INIT_LIST_HEAD(&sample_list[2]);
	GLOBAL_PROTECT_METHOD = 1;
	create_and_test_thread();

	GLOBAL_PROTECT_METHOD = 2;
	create_and_test_thread();

	GLOBAL_PROTECT_METHOD = 3;
	create_and_test_thread();
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
