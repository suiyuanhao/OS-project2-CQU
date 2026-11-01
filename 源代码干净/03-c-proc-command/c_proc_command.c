#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/string.h>

#define PROC_NAME "os_lab_cmd_counter"
#define BUF_SIZE 256
#define INPUT_SIZE 64

static long counter;
static unsigned long read_count;
static unsigned long write_count;
static unsigned long command_count;
static unsigned long invalid_count;
static unsigned long load_jiffies;

static DEFINE_MUTEX(counter_lock);
static struct proc_dir_entry *proc_entry;

static void strip_newline(char *s)
{
    size_t len = strlen(s);

    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

static ssize_t cmd_counter_read(struct file *file, char __user *buf,
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
                    "command_count = %lu\n"
                    "invalid_count = %lu\n"
                    "load_jiffies = %lu\n"
                    "current_jiffies = %lu\n"
                    "commands: inc, dec, reset, or a number\n",
                    counter, read_count, write_count, command_count,
                    invalid_count, load_jiffies, jiffies);

    mutex_unlock(&counter_lock);

    return simple_read_from_buffer(buf, count, ppos, kbuf, len);
}

static ssize_t cmd_counter_write(struct file *file, const char __user *buf,
                                 size_t count, loff_t *ppos)
{
    char kbuf[INPUT_SIZE];
    long value;
    int ret = 0;

    if (count == 0 || count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    kbuf[count] = '\0';
    strip_newline(kbuf);

    mutex_lock(&counter_lock);

    write_count++;

    if (strcmp(kbuf, "inc") == 0) {
        counter++;
        command_count++;
        pr_info("c_proc_command: inc, counter=%ld\n", counter);
    } else if (strcmp(kbuf, "dec") == 0) {
        counter--;
        command_count++;
        pr_info("c_proc_command: dec, counter=%ld\n", counter);
    } else if (strcmp(kbuf, "reset") == 0) {
        counter = 0;
        command_count++;
        pr_info("c_proc_command: reset, counter=%ld\n", counter);
    } else {
        ret = kstrtol(kbuf, 10, &value);
        if (ret == 0) {
            counter = value;
            command_count++;
            pr_info("c_proc_command: set, counter=%ld\n", counter);
        } else {
            invalid_count++;
            pr_info("c_proc_command: invalid input: %s\n", kbuf);
            mutex_unlock(&counter_lock);
            return -EINVAL;
        }
    }

    mutex_unlock(&counter_lock);
    return count;
}

static const struct proc_ops cmd_counter_proc_ops = {
    .proc_read = cmd_counter_read,
    .proc_write = cmd_counter_write,
};

static int __init c_proc_command_init(void)
{
    load_jiffies = jiffies;

    proc_entry = proc_create(PROC_NAME, 0666, NULL, &cmd_counter_proc_ops);
    if (!proc_entry)
        return -ENOMEM;

    pr_info("c_proc_command: module loaded, /proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit c_proc_command_exit(void)
{
    proc_remove(proc_entry);
    pr_info("c_proc_command: module unloaded, /proc/%s removed\n", PROC_NAME);
}

module_init(c_proc_command_init);
module_exit(c_proc_command_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("student");
MODULE_DESCRIPTION("A command-based /proc counter module for openEuler");