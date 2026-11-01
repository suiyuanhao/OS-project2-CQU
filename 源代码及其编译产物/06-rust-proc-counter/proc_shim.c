#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "rust_lab_counter"
#define READ_BUF_SIZE 256
#define WRITE_BUF_SIZE 64

extern ssize_t rust_proc_counter_render(char *buf, size_t size);
extern ssize_t rust_proc_counter_store(const char *buf, size_t len);

static struct proc_dir_entry *rust_proc_entry;

static ssize_t rust_counter_read(struct file *file, char __user *buf,
                                 size_t count, loff_t *ppos)
{
    char kbuf[READ_BUF_SIZE];
    ssize_t len;

    if (*ppos > 0)
        return 0;

    len = rust_proc_counter_render(kbuf, sizeof(kbuf));
    if (len < 0)
        return len;

    return simple_read_from_buffer(buf, count, ppos, kbuf, len);
}

static ssize_t rust_counter_write(struct file *file, const char __user *buf,
                                  size_t count, loff_t *ppos)
{
    char kbuf[WRITE_BUF_SIZE];
    ssize_t ret;

    if (count == 0 || count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    kbuf[count] = '\0';

    ret = rust_proc_counter_store(kbuf, count);
    if (ret < 0)
        return ret;

    return count;
}

static const struct proc_ops rust_counter_ops = {
    .proc_read = rust_counter_read,
    .proc_write = rust_counter_write,
};

int rust_proc_counter_register(void)
{
    rust_proc_entry = proc_create(PROC_NAME, 0666, NULL, &rust_counter_ops);
    if (!rust_proc_entry)
        return -ENOMEM;

    return 0;
}

void rust_proc_counter_unregister(void)
{
    proc_remove(rust_proc_entry);
    rust_proc_entry = NULL;
}