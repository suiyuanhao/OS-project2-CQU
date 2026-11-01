// SPDX-License-Identifier: GPL-2.0
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "rust_lab_cmd_counter"
#define LOCAL_BUF_SIZE 128
#define OUTPUT_BUF_SIZE 512

extern ssize_t rust_cmd_counter_render(char *buf, size_t size);
extern ssize_t rust_cmd_counter_store(const char *buf, size_t len);

static struct proc_dir_entry *proc_entry;

static ssize_t proc_counter_read(struct file *file, char __user *ubuf,
                                 size_t count, loff_t *ppos)
{
    char buf[OUTPUT_BUF_SIZE];
    ssize_t len;

    len = rust_cmd_counter_render(buf, sizeof(buf));
    if (len < 0)
        return len;

    return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static ssize_t proc_counter_write(struct file *file, const char __user *ubuf,
                                  size_t count, loff_t *ppos)
{
    char buf[LOCAL_BUF_SIZE];
    size_t len;
    ssize_t ret;

    len = min(count, sizeof(buf) - 1);

    if (copy_from_user(buf, ubuf, len))
        return -EFAULT;

    buf[len] = '\0';

    ret = rust_cmd_counter_store(buf, len);
    if (ret < 0)
        return ret;

    return count;
}

static const struct proc_ops proc_counter_ops = {
    .proc_read = proc_counter_read,
    .proc_write = proc_counter_write,
};

int rust_cmd_proc_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_counter_ops);
    if (!proc_entry)
        return -ENOMEM;

    return 0;
}

void rust_cmd_proc_exit(void)
{
    if (proc_entry)
        proc_remove(proc_entry);

    proc_entry = NULL;
}
