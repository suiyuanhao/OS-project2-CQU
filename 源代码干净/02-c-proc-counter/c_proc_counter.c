#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/fs.h>

#define PROC_NAME "os_lab_counter"
#define BUF_SIZE 256

static long counter;
static unsigned long read_count;
static unsigned long write_count;
static unsigned long load_jiffies;

static DEFINE_MUTEX(counter_lock);
static struct proc_dir_entry *proc_entry;

static ssize_t counter_read(struct file *file, char __user *buf,
                            size_t count, loff_t *ppos)
{
    char kbuf[BUF_SIZE];
    int len;

    if (*ppos > 0)
        return 0;

    mutex_lock(&counter_lock);
    read_count++;

    len = scnprintf(kbuf, sizeof(kbuf),
                    "counter = %ld\n"
                    "read_count = %lu\n"
                    "write_count = %lu\n"
                    "load_jiffies = %lu\n"
                    "current_jiffies = %lu\n",
                    counter, read_count, write_count,
                    load_jiffies, jiffies);

    mutex_unlock(&counter_lock);

    return simple_read_from_buffer(buf, count, ppos, kbuf, len);
}

static ssize_t counter_write(struct file *file, const char __user *buf,
                             size_t count, loff_t *ppos)
{
    char kbuf[32];
    long value;
    int ret;

    if (count == 0 || count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    kbuf[count] = '\0';

    ret = kstrtol(kbuf, 10, &value);
    if (ret)
        return ret;

    mutex_lock(&counter_lock);
    counter = value;
    write_count++;
    mutex_unlock(&counter_lock);

    pr_info("c_proc_counter: counter set to %ld\n", value);

    return count;
}

static const struct proc_ops counter_proc_ops = {
    .proc_read = counter_read,
    .proc_write = counter_write,
};

static int __init c_proc_counter_init(void)
{
    load_jiffies = jiffies;

    proc_entry = proc_create(PROC_NAME, 0666, NULL, &counter_proc_ops);
    if (!proc_entry)
        return -ENOMEM;

    pr_info("c_proc_counter: module loaded, /proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit c_proc_counter_exit(void)
{
    proc_remove(proc_entry);
    pr_info("c_proc_counter: module unloaded, /proc/%s removed\n", PROC_NAME);
}

module_init(c_proc_counter_init);
module_exit(c_proc_counter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("student");
MODULE_DESCRIPTION("A simple /proc counter module for openEuler");