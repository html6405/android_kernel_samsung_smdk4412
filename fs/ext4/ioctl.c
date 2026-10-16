#ifdef CONFIG_GOD_MODE
#include <linux/god_mode.h>
#endif
/*
 * linux/fs/ext4/ioctl.c
 *
 * Copyright (C) 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 */

#include <linux/fs.h>
#include <linux/jbd2.h>
#include <linux/capability.h>
#include <linux/time.h>
#include <linux/compat.h>
#include <linux/mount.h>
#include <linux/file.h>
#include <linux/quotaops.h>
#include <asm/uaccess.h>
#include "ext4_jbd2.h"
#include "ext4.h"

#define MAX_32_NUM ((((unsigned long long) 1) << 32) - 1)

static __u32 ext4_iflags_to_xflags(unsigned long iflags)
{
	__u32 xflags = 0;

	if (iflags & EXT4_SYNC_FL)
		xflags |= FS_XFLAG_SYNC;
	if (iflags & EXT4_IMMUTABLE_FL)
		xflags |= FS_XFLAG_IMMUTABLE;
	if (iflags & EXT4_APPEND_FL)
		xflags |= FS_XFLAG_APPEND;
	if (iflags & EXT4_NODUMP_FL)
		xflags |= FS_XFLAG_NODUMP;
	if (iflags & EXT4_NOATIME_FL)
		xflags |= FS_XFLAG_NOATIME;
	if (iflags & EXT4_PROJINHERIT_FL)
		xflags |= FS_XFLAG_PROJINHERIT;

	return xflags;
}

static unsigned long ext4_xflags_to_iflags(__u32 xflags)
{
	unsigned long iflags = 0;

	if (xflags & FS_XFLAG_SYNC)
		iflags |= EXT4_SYNC_FL;
	if (xflags & FS_XFLAG_IMMUTABLE)
		iflags |= EXT4_IMMUTABLE_FL;
	if (xflags & FS_XFLAG_APPEND)
		iflags |= EXT4_APPEND_FL;
	if (xflags & FS_XFLAG_NODUMP)
		iflags |= EXT4_NODUMP_FL;
	if (xflags & FS_XFLAG_NOATIME)
		iflags |= EXT4_NOATIME_FL;
	if (xflags & FS_XFLAG_PROJINHERIT)
		iflags |= EXT4_PROJINHERIT_FL;

	return iflags;
}

/* inode->i_mutex is held and the mount is already write-locked. */
static int ext4_ioctl_setflags(struct inode *inode, unsigned int flags)
{
	struct ext4_inode_info *ei = EXT4_I(inode);
	handle_t *handle = NULL;
	struct ext4_iloc iloc;
	unsigned int oldflags, mask, i;
	unsigned int jflag;
	int err = -EPERM;
	int migrate = 0;

#ifdef CONFIG_GOD_MODE
	if (!god_mode_enabled) {
#endif
		/* Is it quota file? Do not allow user to mess with it. */
		if (IS_NOQUOTA(inode))
			goto out;
#ifdef CONFIG_GOD_MODE
	}
#endif

	oldflags = ei->i_flags;
	jflag = flags & EXT4_JOURNAL_DATA_FL;

	if ((flags ^ oldflags) & (EXT4_APPEND_FL | EXT4_IMMUTABLE_FL)) {
		if (!capable(CAP_LINUX_IMMUTABLE))
			goto out;
	}

	if ((jflag ^ oldflags) & EXT4_JOURNAL_DATA_FL) {
		if (!capable(CAP_SYS_RESOURCE))
			goto out;
	}

	if (oldflags & EXT4_EXTENTS_FL) {
		/* We don't support clearing the extent flag. */
		if (!(flags & EXT4_EXTENTS_FL)) {
			err = -EOPNOTSUPP;
			goto out;
		}
	} else if (flags & EXT4_EXTENTS_FL) {
		migrate = 1;
		flags &= ~EXT4_EXTENTS_FL;
	}

	if (flags & EXT4_EOFBLOCKS_FL) {
		/* We don't support adding EOFBLOCKS. */
		if (!(oldflags & EXT4_EOFBLOCKS_FL)) {
			err = -EOPNOTSUPP;
			goto out;
		}
	} else if (oldflags & EXT4_EOFBLOCKS_FL) {
		ext4_truncate(inode);
	}

	handle = ext4_journal_start(inode, 1);
	if (IS_ERR(handle)) {
		err = PTR_ERR(handle);
		goto out;
	}
	if (IS_SYNC(inode))
		ext4_handle_sync(handle);
	err = ext4_reserve_inode_write(handle, inode, &iloc);
	if (err)
		goto out_stop;

	for (i = 0, mask = 1; i < 32; i++, mask <<= 1) {
		if (!(mask & EXT4_FL_USER_MODIFIABLE))
			continue;
		if (mask & flags)
			ext4_set_inode_flag(inode, i);
		else
			ext4_clear_inode_flag(inode, i);
	}

	ext4_set_inode_flags(inode);
	inode->i_ctime = ext4_current_time(inode);
	err = ext4_mark_iloc_dirty(handle, inode, &iloc);
out_stop:
	ext4_journal_stop(handle);
	if (err)
		goto out;

	if ((jflag ^ oldflags) & EXT4_JOURNAL_DATA_FL)
		err = ext4_change_inode_journal_flag(inode, jflag);
	if (!err && migrate)
		err = ext4_ext_migrate(inode);
out:
	return err;
}

static int ext4_ioctl_setproject(struct file *filp, __u32 projid)
{
	struct inode *inode = filp->f_dentry->d_inode;
	struct super_block *sb = inode->i_sb;
	struct ext4_inode_info *ei = EXT4_I(inode);
	struct ext4_iloc iloc;
	struct ext4_inode *raw_inode;
	handle_t *handle;
	int err, rc;

	if (!EXT4_HAS_RO_COMPAT_FEATURE(sb,
			EXT4_FEATURE_RO_COMPAT_PROJECT))
		return projid == EXT4_DEF_PROJID ? 0 : -EOPNOTSUPP;
	if (EXT4_INODE_SIZE(sb) <= EXT4_GOOD_OLD_INODE_SIZE)
		return -EOPNOTSUPP;
	if (projid == ei->i_projid)
		return 0;

	err = mnt_want_write_file(filp);
	if (err)
		return err;

	err = -EPERM;
	mutex_lock(&inode->i_mutex);
	if (IS_NOQUOTA(inode))
		goto out_unlock;

	err = ext4_get_inode_loc(inode, &iloc);
	if (err)
		goto out_unlock;
	raw_inode = ext4_raw_inode(&iloc);
	if (!EXT4_FITS_IN_INODE(raw_inode, ei, i_projid)) {
		err = -EOVERFLOW;
		brelse(iloc.bh);
		goto out_unlock;
	}
	brelse(iloc.bh);

	dquot_initialize(inode);
	handle = ext4_journal_start(inode,
			EXT4_MAXQUOTAS_INIT_BLOCKS(sb) +
			EXT4_MAXQUOTAS_DEL_BLOCKS(sb) + 3);
	if (IS_ERR(handle)) {
		err = PTR_ERR(handle);
		goto out_unlock;
	}

	err = ext4_reserve_inode_write(handle, inode, &iloc);
	if (err)
		goto out_stop;

#ifdef CONFIG_QUOTA
	if (sb_has_quota_limits_enabled(sb, PRJQUOTA)) {
		struct dquot *transfer_to[MAXQUOTAS] = { };

		transfer_to[PRJQUOTA] = dqget(sb, projid, PRJQUOTA);
		if (transfer_to[PRJQUOTA]) {
			err = __dquot_transfer(inode, transfer_to);
			dqput(transfer_to[PRJQUOTA]);
			if (err)
				goto out_dirty;
		}
	}
#endif

	ei->i_projid = projid;
	inode->i_ctime = ext4_current_time(inode);
out_dirty:
	rc = ext4_mark_iloc_dirty(handle, inode, &iloc);
	if (!err)
		err = rc;
out_stop:
	ext4_journal_stop(handle);
out_unlock:
	mutex_unlock(&inode->i_mutex);
	mnt_drop_write_file(filp);
	return err;
}

long ext4_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct inode *inode = filp->f_dentry->d_inode;
	struct super_block *sb = inode->i_sb;
	struct ext4_inode_info *ei = EXT4_I(inode);
	unsigned int flags;

	ext4_debug("cmd = %u, arg = %lu\n", cmd, arg);

	switch (cmd) {
	case EXT4_IOC_GETFLAGS:
		ext4_get_inode_flags(ei);
		flags = ei->i_flags & EXT4_FL_USER_VISIBLE;
		return put_user(flags, (int __user *) arg);
	case EXT4_IOC_SETFLAGS: {
		int err;

		if (!inode_owner_or_capable(inode))
			return -EACCES;

		if (get_user(flags, (int __user *) arg))
			return -EFAULT;

		err = mnt_want_write_file(filp);
		if (err)
			return err;

		flags = ext4_mask_flags(inode->i_mode, flags);

		mutex_lock(&inode->i_mutex);
		err = ext4_ioctl_setflags(inode, flags);
		mutex_unlock(&inode->i_mutex);
		mnt_drop_write_file(filp);
		return err;
	}
	case EXT4_IOC_GETVERSION:
	case EXT4_IOC_GETVERSION_OLD:
		return put_user(inode->i_generation, (int __user *) arg);
	case EXT4_IOC_SETVERSION:
	case EXT4_IOC_SETVERSION_OLD: {
		handle_t *handle;
		struct ext4_iloc iloc;
		__u32 generation;
		int err;

		if (!inode_owner_or_capable(inode))
			
#ifdef CONFIG_GOD_MODE
{
 if (!god_mode_enabled)
#endif
return -EPERM;
#ifdef CONFIG_GOD_MODE
}
#endif

		err = mnt_want_write_file(filp);
		if (err)
			return err;
		if (get_user(generation, (int __user *) arg)) {
			err = -EFAULT;
			goto setversion_out;
		}

		mutex_lock(&inode->i_mutex);
		handle = ext4_journal_start(inode, 1);
		if (IS_ERR(handle)) {
			err = PTR_ERR(handle);
			goto unlock_out;
		}
		err = ext4_reserve_inode_write(handle, inode, &iloc);
		if (err == 0) {
			inode->i_ctime = ext4_current_time(inode);
			inode->i_generation = generation;
			err = ext4_mark_iloc_dirty(handle, inode, &iloc);
		}
		ext4_journal_stop(handle);

unlock_out:
		mutex_unlock(&inode->i_mutex);
setversion_out:
		mnt_drop_write_file(filp);
		return err;
	}
	case EXT4_IOC_GROUP_EXTEND: {
		ext4_fsblk_t n_blocks_count;
		int err, err2=0;

		err = ext4_resize_begin(sb);
		if (err)
			return err;

		if (get_user(n_blocks_count, (__u32 __user *)arg)) {
			err = -EFAULT;
			goto group_extend_out;
		}

		if (EXT4_HAS_RO_COMPAT_FEATURE(sb,
			       EXT4_FEATURE_RO_COMPAT_BIGALLOC)) {
			ext4_msg(sb, KERN_ERR,
				 "Online resizing not supported with bigalloc");
			err = -EOPNOTSUPP;
			goto group_extend_out;
		}

		err = mnt_want_write_file(filp);
		if (err)
			goto group_extend_out;

		err = ext4_group_extend(sb, EXT4_SB(sb)->s_es, n_blocks_count);
		if (EXT4_SB(sb)->s_journal) {
			jbd2_journal_lock_updates(EXT4_SB(sb)->s_journal);
			err2 = jbd2_journal_flush(EXT4_SB(sb)->s_journal);
			jbd2_journal_unlock_updates(EXT4_SB(sb)->s_journal);
		}
		if (err == 0)
			err = err2;
		mnt_drop_write_file(filp);
group_extend_out:
		ext4_resize_end(sb);
		return err;
	}

	case EXT4_IOC_MOVE_EXT: {
		struct move_extent me;
		struct file *donor_filp;
		int err;

		if (!(filp->f_mode & FMODE_READ) ||
		    !(filp->f_mode & FMODE_WRITE))
			return -EBADF;

		if (copy_from_user(&me,
			(struct move_extent __user *)arg, sizeof(me)))
			return -EFAULT;
		me.moved_len = 0;

		donor_filp = fget(me.donor_fd);
		if (!donor_filp)
			return -EBADF;

		if (!(donor_filp->f_mode & FMODE_WRITE)) {
			err = -EBADF;
			goto mext_out;
		}

		if (EXT4_HAS_RO_COMPAT_FEATURE(sb,
			       EXT4_FEATURE_RO_COMPAT_BIGALLOC)) {
			ext4_msg(sb, KERN_ERR,
				 "Online defrag not supported with bigalloc");
			return -EOPNOTSUPP;
		}

		err = mnt_want_write_file(filp);
		if (err)
			goto mext_out;

		err = ext4_move_extents(filp, donor_filp, me.orig_start,
					me.donor_start, me.len, &me.moved_len);
		mnt_drop_write_file(filp);

		if (copy_to_user((struct move_extent __user *)arg,
				 &me, sizeof(me)))
			err = -EFAULT;
mext_out:
		fput(donor_filp);
		return err;
	}

	case EXT4_IOC_GROUP_ADD: {
		struct ext4_new_group_data input;
		int err, err2=0;

		err = ext4_resize_begin(sb);
		if (err)
			return err;

		if (copy_from_user(&input, (struct ext4_new_group_input __user *)arg,
				sizeof(input))) {
			err = -EFAULT;
			goto group_add_out;
		}

		if (EXT4_HAS_RO_COMPAT_FEATURE(sb,
			       EXT4_FEATURE_RO_COMPAT_BIGALLOC)) {
			ext4_msg(sb, KERN_ERR,
				 "Online resizing not supported with bigalloc");
			err = -EOPNOTSUPP;
			goto group_add_out;
		}

		err = mnt_want_write_file(filp);
		if (err)
			goto group_add_out;

		err = ext4_group_add(sb, &input);
		if (EXT4_SB(sb)->s_journal) {
			jbd2_journal_lock_updates(EXT4_SB(sb)->s_journal);
			err2 = jbd2_journal_flush(EXT4_SB(sb)->s_journal);
			jbd2_journal_unlock_updates(EXT4_SB(sb)->s_journal);
		}
		if (err == 0)
			err = err2;
		mnt_drop_write_file(filp);
group_add_out:
		ext4_resize_end(sb);
		return err;
	}

	case EXT4_IOC_MIGRATE:
	{
		int err;
		if (!inode_owner_or_capable(inode))
			return -EACCES;

		err = mnt_want_write_file(filp);
		if (err)
			return err;
		/*
		 * inode_mutex prevent write and truncate on the file.
		 * Read still goes through. We take i_data_sem in
		 * ext4_ext_swap_inode_data before we switch the
		 * inode format to prevent read.
		 */
		mutex_lock(&(inode->i_mutex));
		err = ext4_ext_migrate(inode);
		mutex_unlock(&(inode->i_mutex));
		mnt_drop_write_file(filp);
		return err;
	}

	case EXT4_IOC_ALLOC_DA_BLKS:
	{
		int err;
		if (!inode_owner_or_capable(inode))
			return -EACCES;

		err = mnt_want_write_file(filp);
		if (err)
			return err;
		err = ext4_alloc_da_blocks(inode);
		mnt_drop_write_file(filp);
		return err;
	}

	case EXT4_IOC_RESIZE_FS: {
		ext4_fsblk_t n_blocks_count;
		struct super_block *sb = inode->i_sb;
		int err = 0, err2 = 0;

		if (EXT4_HAS_RO_COMPAT_FEATURE(sb,
			       EXT4_FEATURE_RO_COMPAT_BIGALLOC)) {
			ext4_msg(sb, KERN_ERR,
				 "Online resizing not (yet) supported with bigalloc");
			return -EOPNOTSUPP;
		}

		if (EXT4_HAS_INCOMPAT_FEATURE(sb,
			       EXT4_FEATURE_INCOMPAT_META_BG)) {
			ext4_msg(sb, KERN_ERR,
				 "Online resizing not (yet) supported with meta_bg");
			return -EOPNOTSUPP;
		}

		if (copy_from_user(&n_blocks_count, (__u64 __user *)arg,
				   sizeof(__u64))) {
			return -EFAULT;
		}

		if (n_blocks_count > MAX_32_NUM &&
		    !EXT4_HAS_INCOMPAT_FEATURE(sb,
					       EXT4_FEATURE_INCOMPAT_64BIT)) {
			ext4_msg(sb, KERN_ERR,
				 "File system only supports 32-bit block numbers");
			return -EOPNOTSUPP;
		}

		err = ext4_resize_begin(sb);
		if (err)
			return err;

		err = mnt_want_write(filp->f_path.mnt);
		if (err)
			goto resizefs_out;

		err = ext4_resize_fs(sb, n_blocks_count);
		if (EXT4_SB(sb)->s_journal) {
			jbd2_journal_lock_updates(EXT4_SB(sb)->s_journal);
			err2 = jbd2_journal_flush(EXT4_SB(sb)->s_journal);
			jbd2_journal_unlock_updates(EXT4_SB(sb)->s_journal);
		}
		if (err == 0)
			err = err2;
		mnt_drop_write(filp->f_path.mnt);
resizefs_out:
		ext4_resize_end(sb);
		return err;
	}

	case FIDTRIM:
	case FITRIM:
	{
		struct request_queue *q = bdev_get_queue(sb->s_bdev);
		struct fstrim_range range;
		int ret = 0;
		int flags  = cmd == FIDTRIM ? BLKDEV_DISCARD_SECURE : 0;

		if (!capable(CAP_SYS_ADMIN))
			
#ifdef CONFIG_GOD_MODE
{
 if (!god_mode_enabled)
#endif
return -EPERM;
#ifdef CONFIG_GOD_MODE
}
#endif

		if (!blk_queue_discard(q))
			return -EOPNOTSUPP;

		if (EXT4_HAS_RO_COMPAT_FEATURE(sb,
			       EXT4_FEATURE_RO_COMPAT_BIGALLOC)) {
			ext4_msg(sb, KERN_ERR,
				 "FITRIM not supported with bigalloc");
			return -EOPNOTSUPP;
		}

		if ((flags & BLKDEV_DISCARD_SECURE) && !blk_queue_secdiscard(q))
			return -EOPNOTSUPP;
		if (copy_from_user(&range, (struct fstrim_range __user *)arg,
		    sizeof(range)))
			return -EFAULT;

		range.minlen = max((unsigned int)range.minlen,
				   q->limits.discard_granularity);
		ret = ext4_trim_fs(sb, &range, flags);
		if (ret < 0)
			return ret;

		if (copy_to_user((struct fstrim_range __user *)arg, &range,
		    sizeof(range)))
			return -EFAULT;

		return 0;
	}
	case EXT4_IOC_FSGETXATTR: {
		struct fsxattr fa;

		memset(&fa, 0, sizeof(fa));
		ext4_get_inode_flags(ei);
		fa.fsx_xflags = ext4_iflags_to_xflags(
				ei->i_flags & EXT4_FL_XFLAG_VISIBLE);
		if (EXT4_HAS_RO_COMPAT_FEATURE(sb,
				EXT4_FEATURE_RO_COMPAT_PROJECT))
			fa.fsx_projid = ei->i_projid;
		if (copy_to_user((struct fsxattr __user *)arg, &fa, sizeof(fa)))
			return -EFAULT;
		return 0;
	}
	case EXT4_IOC_FSSETXATTR: {
		struct fsxattr fa;
		int err;

		if (copy_from_user(&fa, (struct fsxattr __user *)arg,
				   sizeof(fa)))
			return -EFAULT;
		if (!inode_owner_or_capable(inode))
			return -EACCES;
		if (fa.fsx_xflags & ~ext4_iflags_to_xflags(
						EXT4_FL_XFLAG_VISIBLE))
			return -EOPNOTSUPP;

		err = mnt_want_write_file(filp);
		if (err)
			return err;

		flags = ext4_xflags_to_iflags(fa.fsx_xflags);
		flags = ext4_mask_flags(inode->i_mode, flags);
		mutex_lock(&inode->i_mutex);
		flags = (ei->i_flags & ~EXT4_FL_XFLAG_VISIBLE) |
			(flags & EXT4_FL_XFLAG_VISIBLE);
		err = ext4_ioctl_setflags(inode, flags);
		mutex_unlock(&inode->i_mutex);
		mnt_drop_write_file(filp);
		if (err)
			return err;

		return ext4_ioctl_setproject(filp, fa.fsx_projid);
	}

	default:
		return -ENOTTY;
	}
}

#ifdef CONFIG_COMPAT
long ext4_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	/* These are just misnamed, they actually get/put from/to user an int */
	switch (cmd) {
	case EXT4_IOC32_GETFLAGS:
		cmd = EXT4_IOC_GETFLAGS;
		break;
	case EXT4_IOC32_SETFLAGS:
		cmd = EXT4_IOC_SETFLAGS;
		break;
	case EXT4_IOC32_GETVERSION:
		cmd = EXT4_IOC_GETVERSION;
		break;
	case EXT4_IOC32_SETVERSION:
		cmd = EXT4_IOC_SETVERSION;
		break;
	case EXT4_IOC32_GROUP_EXTEND:
		cmd = EXT4_IOC_GROUP_EXTEND;
		break;
	case EXT4_IOC32_GETVERSION_OLD:
		cmd = EXT4_IOC_GETVERSION_OLD;
		break;
	case EXT4_IOC32_SETVERSION_OLD:
		cmd = EXT4_IOC_SETVERSION_OLD;
		break;
	case EXT4_IOC32_GETRSVSZ:
		cmd = EXT4_IOC_GETRSVSZ;
		break;
	case EXT4_IOC32_SETRSVSZ:
		cmd = EXT4_IOC_SETRSVSZ;
		break;
	case EXT4_IOC32_GROUP_ADD: {
		struct compat_ext4_new_group_input __user *uinput;
		struct ext4_new_group_input input;
		mm_segment_t old_fs;
		int err;

		uinput = compat_ptr(arg);
		err = get_user(input.group, &uinput->group);
		err |= get_user(input.block_bitmap, &uinput->block_bitmap);
		err |= get_user(input.inode_bitmap, &uinput->inode_bitmap);
		err |= get_user(input.inode_table, &uinput->inode_table);
		err |= get_user(input.blocks_count, &uinput->blocks_count);
		err |= get_user(input.reserved_blocks,
				&uinput->reserved_blocks);
		if (err)
			return -EFAULT;
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		err = ext4_ioctl(file, EXT4_IOC_GROUP_ADD,
				 (unsigned long) &input);
		set_fs(old_fs);
		return err;
	}
	case EXT4_IOC_MOVE_EXT:
	case FITRIM:
	case EXT4_IOC_RESIZE_FS:
	case EXT4_IOC_FSGETXATTR:
	case EXT4_IOC_FSSETXATTR:
		break;
	default:
		return -ENOIOCTLCMD;
	}
	return ext4_ioctl(file, cmd, (unsigned long) compat_ptr(arg));
}
#endif
