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




struct my_type {
		struct rb_node node;
			int key;
				int value;
};



struct my_type *rb_search(struct rb_root *root, int key)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct my_type *data = container_of(node, struct my_type, node);

		if(data->key > key)	
			node = node->rb_left;
		else if(data->key < key)	
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}



int rb_insert(struct rb_root *root, struct my_type *data)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out "where" to put new node */
	while (*new) {
		struct my_type *this = container_of(*new, struct my_type, node);
		parent = *new;
		if (this->key > data->key)
			new = &((*new)->rb_left);
		else if (this->key < data->key)
			new = &((*new)->rb_right);
		else
			return 0;
	}

	rb_link_node(&data->node, parent, new); 	/*relinking*/
	rb_insert_color(&data->node, root);	 /*recoloring & rebalancing*/

	return 1;
}


int rb_delete(struct rb_root *mytree, int key)
{
	struct my_type *data = rb_search(mytree, key);

	if (data) {
		rb_erase(&data->node, mytree);
		kfree(data);
	}
}


void struct_example(void)
{
	struct rb_root my_tree[3] = {RB_ROOT, RB_ROOT, RB_ROOT};
	int index[3] = {1000,10000,100000};
	unsigned long long insert_time[3];
	unsigned long long search_time[3];
	unsigned long long del_time[3];
	struct timespec temp[2];
	int i = 0,j= 0, ret;


	/* rb_node create and insert */	
	for(i =0; i< 3; i++){
		getrawmonotonic(&temp[0]);
		for(j = 0; j <index[i]; j++){
			struct my_type *new = kmalloc(sizeof(struct my_type),GFP_KERNEL);

			if(!new){
				return NULL;
			}
			new->value = j;
			new->key =j;

			ret = rb_insert(&my_tree[i],new);
		}
		getrawmonotonic(&temp[1]);
		insert_time[i] = cal_clock(temp);
	}


	/* rb_tree find node */
	for(i = 0; i<3; i++){
		getrawmonotonic(&temp[0]);
		struct my_type *find_node = rb_search(&my_tree[i],8);
		getrawmonotonic(&temp[1]);
		search_time[i] = cal_clock(temp);
	}

	/* rb_tree delete node */
	for(i = 0; i<3; i++){
		getrawmonotonic(&temp[0]);
		rb_delete(&my_tree[i],0);
		getrawmonotonic(&temp[1]);
		del_time[i] = cal_clock(temp);
	}

	printk("#############################\n");
	printk("jesnk RB tree time profiling \n",index[i]); 
	for(i = 0; i<3;i++){
		printk("Insert  %d : %lld\n",index[i],insert_time[i]);
	}

	for(i = 0; i<3;i++){
		printk("Search %d : %lld\n",index[i],search_time[i]);
	}
	
	for(i = 0; i<3;i++){
		printk("Delete %d : %lld\n",index[i],del_time[i]);
	}
}





int __init jesnm2_module_init(void) {
	printk("Hello, this is jesnm2 Module\n");
	struct_example();
	return 0;
}







void __exit jesnm2_module_cleanup(void) {
	printk("Bye, this is jesnm2 Module\n");
	
	return;
}
module_init(jesnm2_module_init);
module_exit(jesnm2_module_cleanup);


MODULE_LICENSE("GPL");
