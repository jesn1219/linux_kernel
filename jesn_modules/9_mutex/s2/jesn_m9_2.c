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





int test_thread(void* _arg) {
    int *arg = (int*)_arg;
    int thread_idx = *arg;
    while (true) {
        if (test_and_set(&global_lock)) {
            printk("th_id : %d will append idx : %d\n", __sync_fetch_and_add(&global_index,1));
            int tmp = __sync_fetch_and_add(&global_index,1);
            __sync_bool_compare_and_swap(&value,tmp,tmp+1);
            global_lock = 0;
        }
        if (global_index > 15) {
            break;
        }
    }
}




void create_and_test_thread(void) {
    global_index = 0;
    global_lock = 0;
    value = 0;
    for ( i=0; i < NUMJOBS; i++) {
        int* arg = i;
        kthread_run(&test_thread, &arg, "test_thread");
    }

}


void test_program(void) {
	create_and_test_thread();
}



int __init jesn_m9_2_module_init(void) {
	printk("Hello, this is jesn_m9_2 Module\n");
	test_program();
	return 0;
}


void __exit jesn_m9_2_module_cleanup(void) {
	printk("Bye, this is jesn_m9_2 Module\n");
	return;
}


module_init(jesn_m9_2_module_init);
module_exit(jesn_m9_2_module_cleanup);


MODULE_LICENSE("GPL");
