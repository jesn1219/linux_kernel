#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>


int test_thread(void *_arg) {
	int* arg = (int*)_arg;
	printk("argument : %d\n", *arg);
	kfree(arg);
	return 0;
}

void thread_create(void) {
	int i;
	/* thread_create*/
	for (i=0; i<10; i++) {
		int* arg = (int*)kmalloc(sizeof(int),GFP_KERNEL);
		*arg = i;
		kthread_run(&test_thread,(void*)arg,"test_thread");
	}
}

int __init jesnm3_module_init(void) {
	printk("Hello, this is jesnm3 Module\n");
	thread_create();
	return 0;
}




void __exit jesnm3_module_cleanup(void) {
	printk("Bye, this is jesnm3 Module\n");
	
	return;
}
module_init(jesnm3_module_init);
module_exit(jesnm3_module_cleanup);


MODULE_LICENSE("GPL");
