#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>

static int __init c_hello_init(void)
{
    pr_info("c_hello: module loaded, jiffies=%lu\n", jiffies);
    return 0;
}

static void __exit c_hello_exit(void)
{
    pr_info("c_hello: module unloaded\n");
}

module_init(c_hello_init);
module_exit(c_hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("student");
MODULE_DESCRIPTION("A simple openEuler kernel module written in C");