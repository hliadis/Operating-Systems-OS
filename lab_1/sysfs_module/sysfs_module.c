#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kobject.h>
#include <linux/sysfs.h> 
#include <linux/init.h> 
#include <linux/fs.h> 
#include <linux/string.h>
#include  <linux/kernel.h> 
#include <linux/sched.h>

#define GNU_SOURCE
#define INIT_PID 1

static struct kobject *find_roots;

static ssize_t foo_show( struct kobject *kobj , struct kobj_attribute *attr , char *buf ) {
    	struct task_struct *current_task = current;
	struct task_struct *task;
	char comm[TASK_COMM_LEN];
	int my_pid = current_task->pid;

	printk("find_roots sysfs opened by process %d\n", current_task->pid);

	for(task = current_task; task->pid != INIT_PID; task = task->real_parent){

		get_task_comm(comm, task);
		
		printk(" id:%d, name:%s\n", task->pid, comm);
	}

	get_task_comm(comm, task);
	printk("id:%d, name:%s\n", task->pid, comm);

	return(sprintf(buf, "id:%d\n", my_pid));
}

struct kobj_attribute foo_attribute = __ATTR(find_roots, 0660, foo_show , NULL);

static int __init mymodule_init (void) {
    int error = 0;
    printk("Module initialized successfully \n");

    find_roots = kobject_create_and_add ("team2523_2780_2806" ,
    kernel_kobj ) ;
    if (! find_roots )
	 return (- ENOMEM);

        error = sysfs_create_file(find_roots, &foo_attribute.attr);
   
    if (error) {
        printk("failed to create the foo file in /sys/kernel/team2523_2780_2806\n") ;
    }
    return error ;
}

static void __exit mymodule_exit (void) {
    printk("Module un initialized successfully \n");
    kobject_put ( find_roots ) ;
}

module_init ( mymodule_init ) ;
module_exit ( mymodule_exit ) ;
MODULE_LICENSE( "GPL" ) ;
