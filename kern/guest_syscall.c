#include <inc/guest_syscall.h>

inline void *get_guest_buf(int cpuid, uint64_t cr3)
{
	void *ret = NULL;
	return ret;
}



uint64_t tramp_addr = 0;

void register_trampline(uint64_t addr, uint64_t cr3)
{

	tramp_addr = addr;

}

inline uint64_t get_trampline_addr(uint64_t cr3)
{
	uint64_t ret = 0UL;
	return tramp_addr;
}
#define STAT 0
#define POLLFD 1
uint32_t struct_size[100] = {
	0
};
uint64_t guest_rsp[32];	// recorde guest user stack

/*
 * We store the syscall informaton into thread stack
 * 		return address
 * 		parameters number
 * 		address start
 * 		length
 * 		.......
 */
#if 0
void parse_guest_syscall(struct vmcb *vmcb, struct generl_regs *regs, uint64_t procid)
{
	int syscall_num = vmcb->rax;
	void *buf = get_guest_buf(procid, vmcb->cr3);
	/* recorde the offset of next return address */
	uint64_t *p_save = (uint64_t *)(vmcb->rsp);


	/*
	 * System call Register setup
	 * rax	system call number
	 * rdi	arg0
	 * rcx	return address for syscall/sysret
	 * rsi	arg1
	 * rdx	arg2
	 * r10	arg3
	 * r8		arg4
	 * r9		arg5
	 * 
	 */

	switch(syscall_num){
		case 1:	/* sys_write(unsigned int fd, char __user *buf, size_t count) */
			*p_save = regs->rcx;
			regs->rcx = get_shim_addr(vmcb->cr3);
			if(regs->rdx < 4096){
				memcpy(buf, (void *)phy_to_machine(regs->rsi, 0,
							vmcb, 0, __LINE__), regs->rdx);
				regs->rsi = (uint64_t)buf;
				/* force every syscall to return into a shim we set */
			}
			break;
		case 2:	/* sys_open(const char __user *filename, int flags, umode_t mode) */
			*p_save = regs->rcx;
			strcpy(buf, (void *)phy_to_machine(regs->rdi, 0,
						vmcb, 0, __LINE__));
			regs->rdi = (uint64_t)buf;
			regs->rcx = get_shim_addr(vmcb->cr3);
			break;
		case 4:	/* sys_newstat(const char __user *filename, struct stat __user *statbuf); */
			// copy dev stat into statbuf
			*p_save = regs->rcx;
			regs->rcx = get_shim_addr(vmcb->cr3);
			strcpy(buf, (void *)phy_to_machine(regs->rdi, 0,
						vmcb, 0, __LINE__));
			regs->rdi = (uint64_t)buf;
			// save the address start and length
			p_save ++;
			*p_save = 1;
			p_save ++;
			*p_save = regs->rsi;
			p_save ++;
			*p_save = struct_size[STAT];
			break;
		case 6:	/* sys_newlstat(const char __user *filename, strcut stat __user *statbuf) */
			*p_save = regs->rcx;
			regs->rcx = get_shim_addr(vmcb->cr3);
			strcpy(buf, (void *)phy_to_machine(regs->rdi, 0,
						vmcb, 0, __LINE__));
			regs->rdi = (uint64_t)buf;
			// save the address start and length
			p_save ++;
			*p_save = 1;
			p_save ++;
			*p_save = regs->rsi;
			p_save ++;
			*p_save = struct_size[STAT];
			break;
		case 7:	/* sys_poll(struct pollfd __user *ufds, unsigned int nfds, int timeout) */
			*p_save = regs->rcx;
			regs->rcx = get_shim_addr(vmcb->cr3);
			p_save ++;
			*p_save = 1;
			p_save ++;
			*p_save = regs->rdi;
			p_save ++;
			*p_save = struct_size[POLLFD];
			break;
		case 18:	/* sys_pwrite64(unsigned int fd, char __user *buf, size_t count, loff_t pos) */
			*p_save = regs->rcx;
			regs->rcx = get_shim_addr(vmcb->cr3);
			if(regs->rdx<4096){
				memcpy(buf, (void*)phy_to_machine(regs->rsi, 0,
							vmcb, 0, __LINE__), regs->rdx);
				regs->rsi = (uint64_t)buf;
			}
			break;
		case 26:	/* sys_msync(unsigned long start, size_t len, int flags) */
			break;
		case 35: /* sys_nanosleep(struct timespec __user *rqtp, sturct timespec __user *rmtp) */
			break;
		case 38: /* sys_setitimer(int which, struct itimerval __user *value,
											sturct itimerval __user *ovalue) */
			break;
		case 42:	/* sys_connect(int, struct sockaddr __user*, int) */
			break;
		case 44:	/* sys_sendto(int, void __user *, size_t , unsigned, struct sockaddr __user *, int) */
			break;
		case 46:	/* sys_sendmsg(int fd, struct msghdr __user *msg, unsigned flags) */
			break;
		case 49:	/* sys_bind(int, struct sockaddr __user *, int) */
			break;
		case 53:	/* sys_socketpair(int, int, int, int __user *) */
			break;
		case 54:	/* sys_setsockopt(int fd, int level, int optname, char __user *optval, int optlen) */
			break;
		case 56:	/*	sys_clone(unsigned long, unsigned long, int __user *, int __user *, int) */
			break;
		case 59:	/* sys_execve(const char __user *filename, const char __user *const __user * argv,
										const char __user *const __user *envp); */
			break;
		case 63:	/*	sys_newuname(struct new_utsname __user *name) */
			break;
		case 65:	/* sys_semop(int semid, struct sembuf __user *sops, unsigned nsops) */
			break;
		case 67:	/* sys_shmdt(char __user *shmaddr) */
			break;
		case 69:	/* sys_msgsnd(int msqid, struct msgbuf __user *msgp,size_t msgz, int msgflag) */
			break;
		case 71:	/* sys_msgctl(int msqid, int cmd, struct msqid_ds __user *buf) */
			break;
		case 76:	/* sys_truncate(const char __user *path, long length) */
			break;
		case 80:	/* sys_chdir(const char __user *filename) */
			break;
		case 82:	/* sys_rename(const char __user *oldname, const char __user *newname) */
			break;
		case 83:	/* sys_mkdir(const char __user *pathname, unode_t mode) */
			break;
		case 84:	/* sys_fmdir (const char __user *pathname) */
			break;
		case 85:	/* sys_create(const char __user *pathname, umode_t mode) */
			break;
		case 86:	/* sys_link(const char __user *oldname, const char __user *newname) */
			break;
		case 87: /* sys_unlink(const char __user *pathname) */
			break;
		case 88:	/* sys_symlink(const char __user *old, const char __user *new) */
			break;
		case 89:	/* sys_readlink(const char __user *paht, char __user *buf, int bufsize) */
			break;
		case 90:	/* sys_chmod(const char __user *filename, umode_t mode) */
			break;
		case 91:	/* sys_fchmod(unsigned int fd, umode_t mode) */
			break;
		case 92:	/* sys_chown(const char __user *filename, uid_t user, gid_t group) */
			break;
		case 93:	/* sys_fchown(unsigned int fd, uid_t user, gid_t group) */
			break;
		case 94:	/* sys_lchown(const char __user *filename, uid_t user, gid_t group ) */
			break;
		case 116:	/* sys_setgroups(int gidsetsize, gid_t __user *grouplist) */
			break;
		case 126:	/* sys_capset(cap_user_header_t header, cap_user_data_t data) */
			break;
		case 130:	/* sys__rt_sigsuspend(sigset_t __user *unewset, size_t sigset size) */
			break;
		case 131:	/* sys_sigaltstack(const struct sigaltstack __user *uss, struct sigaltstack __user *uoss) */
			break;
		case 132:	/* sys_utime(char __user *filename, struct utimbuf __user *times) */
			break;
		case 133:	/* sys_mknod(const char __user *filename, umode_t mode, unsigned dev) */
			break;
		case 142:	/* sys_sched_setparam(pid_t pid, struct sched_param __user *parm) */
			break;
		case 144:	/* sys_sched_setscheduler(pid_t pid, int policy, struct sched_param __user *parm) */
			break;
		case 154:	/* sys_modify_ldt(int, void __user *, unsigned long) */
			break;
		case 156:	/* sys_sysctl(struct __sysctl_args __user *args) */
			break;
		case 157:	/* sys_prctl(int option, unsigned long arg2, unsigned long arg3,
										unsigned long arg4, unsigned long arg5) */
			break;
		case 160:	/* sys_setrlimit(unsigned int resource,
										struct rlimit __user *rlim) */
			break;
		case 161:	/* sys_chroot(const char __user *filename) */
			break;
		case 163:	/* sys_acct(const char __user *name) */
			break;
		case 164:	/* sys_settimeofday(struct timeval __user *tv,
									struct timezone __user *tz) */
			break;
		case 165:	/* sys_mount(char __user *dev_name, char __user *dir_name,
									char __user *type, unsigned long flags,
									void __user *data) */
			break;
		case 166:	/* sys_umount(char __user *name, int flags) */
			break;
		case 167:	/* sys_swapon(const char __user *specialfile, int swap_flags) */
			break;
		case 168:	/* sys_swapoff(const char __user *specialfile) */
			break;
		case 169:	/* sys_reboot(int magic1, int magic2, unsigned int cmd,
									void __user *arg) */
			break;
		case 170:	/* sys_sethostname(char __user *name, int len) */
			break;
		case 171:	/* sys_setdomainname(char __user *name, int len) */
			break;
		case 175:	/* sys_init_module(void __user *umod, unsigned long len,
									const char __user *uargs) */
			break;
		case 176:	/* sys_delete_module(const char __user *name_user,
									unsigned int flags) */
			break;
		case 179:	/* sys_quotactl(unsigned int cmd, const char __user *special,
									qid_t id, void __user *addr) */
			break;
		case 203:	/* sys_sched_setaffinity(pid_t pid, unsigned int len, unsigned long __user *user_mask_ptr) */
			break;
		case 209:	/* sys_io_submit(aio_context_t, long, struct iocb __user * __user *) */
			break;
		case 220:	/* sys_semtimedop(int semid, struct sembuf __user *sops, unsigned nsops, const struct timespec __user *timeout) */
			break;
		case 222:	/* sys_timer_create(clockid_t which_clock, struct sigevent __user *timer_event_spec, timer_t __user * created_timer_id) */
			break;
		case 223:	/* sys_timer_settime(timer_t timer_id, int flags, const struct itimerspec __user *new_setting, struct itimrespec __user *old_setting) */
			break;
		case 227:	/* sys_clock_settime(clockid_t which_clock, const struct timespec __user *tp) */
			break;
		case 230:	/* sys_clock_nanosleep(clockid_t which_clock, int flags, const struct timespec __user *rqtp, struct timespec __user *rmtp) */
			break;
		case 237:	/* sys_mbind(unsigned long start, unsigned long len, unsigned long mode, unsigned long __user *nmask, unsigned long maxnode, unsigned flags) */
			break;
		case 240:	/* sys_mq_open(const char __user *name, int oflag, umode_t mode, struct mq_attr __user *attr) */
			break;
		case 241:	/* sys_mq_unlink(const char __user *name) */
			break;
		case 242:	/*sys_mq_timedsend(mqd_t mqdes, const char __user *msg_ptr, size_t msg_len, unsigned int msg_prio, const struct timespec __user *abs_timeout) */
			break;
		case 243:	/* sys_mq_timedreceive(mqd_t mqdes, char __user *msg_ptr, size_t msg_len, unsigned int __user *msg_prio, const struct timespec __user *abs_timeout) */
			break;
		case 244:	/* sys_mq_notify(mqd_t mqdes, const struct sigevent __user *notification) */
			break;
		case 248:	/* sys_add_key(const char __user *_type, const char __user *_desdription, const void __user *_payload, size_t plen, key_serial_t destringid) */
			break;
		case 254:	/* sys_inotify_add_watch(int fd, const char __user *path, u32 mask) */
			break;
		case 256:	/* sys_migrate_pages(pid_t pid, unsigned long maxnode,
							const unsigned long __user *from, const unsigned long __user *to) */
			break;
		case 257:	/* sys_openat(int dfd, const char __user *filename, int flags, umode_t mode) */
			break;
		case 258:	/* sys_mkdirat(int dfd, const char __user * pathname, umode_t mode) */
			break;
		case 259:	/* sys_mknodat(int dfd, const char __user * filename, umode_t mode, unsigned dev) */
			break;
		case 260:	/* sys_fchownat(int dfd, const char __user *filename, uid_t user, gid_t group, int flag) */
			break;
		case 261:	/* sys_futimesat(int dfd, const char __user *filename, struct timeval __user *utimes) */
			break;
		case 262:	/* sys_newfstatat(int dfd, const char __user *filename, struct stat __user *statbuf, int flag) */
			break;
		case 263:	/* sys_unlinkat(int dfd, const char __user * pathname, int flag) */
			break;
		case 264:	/* sys_renameat(int olddfd, const char __user * oldname, int newdfd, const char __user * newname) */
			break;
		case 265:	/* sys_linkat(int olddfd, const char __user *oldname,
							int newdfd, const char __user *newname, int flags) */
			break;
		case 266:	/* sys_symlinkat(const char __user * oldname, int newdfd, const char __user * newname) */
			break;
		case 267:	/* sys_readlinkat(int dfd, const char __user *path, char __user *buf, int bufsiz) */
			break;
		case 268:	/* sys_fchmodat(int dfd, const char __user * filename, umode_t mode) */
			break;
		case 270:	/* sys_pselect6(int, fd_set __user *, fd_set __user *,
							fd_set __user *, struct timespec __user *, void __user *) */
			break;
		case 271:	/* sys_ppoll(struct pollfd __user *, unsigned int,
							struct timespec __user *, const sigset_t __user *, size_t) */
			break;
		case 280:	/* sys_utimensat(int dfd, const char __user *filename, struct timespec __user *utimes, int flags) */
			break;
		case 281:	/* sys_epoll_pwait(int epfd, struct epoll_event __user *events,
							int maxevents, int timeout, const sigset_t __user *sigmask, size_t sigsetsize) */
			break;
		case 282:	/* sys_signalfd(int ufd, sigset_t __user *user_mask, size_t sizemask) */
			break;
		case 286:	/* sys_timerfd_settime(int ufd, int flags, const struct itimerspec __user *utmr,
struct itimerspec __user *otmr )*/
			break;
		case 288:	/* sys_accept4(int, struct sockaddr __user *, int __user *, int) */
			break;
		case 289:	/* sys_signalfd4(int ufd, sigset_t __user *user_mask, size_t sizemask, int flags) */
			break;
		case 293:	/* sys_pipe2(int __user *fildes, int flags) */
			break;
		case 296:	/* sys_pwritev(unsigned long fd, const struct iovec __user *vec,
							unsigned long vlen, unsigned long pos_l, unsigned long pos_h) */
			break;
		case 297:	/* sys_rt_tgsigqueueinfo(pid_t tgid, pid_t  pid, int sig, siginfo_t __user *uinfo) */
			break;
		case 298:	/* sys_perf_event_open(struct perf_event_attr __user *attr_uptr,
							pid_t pid, int cpu, int group_fd, unsigned long flags) */
			break;
		case 301:	/* sys_fanotify_mark(int fanotify_fd, unsigned int flags, u64 mask, int fd,
							const char  __user *pathname) */
			break;
		case 303:	/* sys_name_to_handle_at(int dfd, const char __user *name,
							struct file_handle __user *handle, int __user *mnt_id, int flag) */
			break;
		case 304:	/* sys_open_by_handle_at(int mountdirfd, struct file_handle __user *handle, int flags) */
			break;
		case 305:	/* sys_clock_adjtime(clockid_t which_clock, struct timex __user *tx) */
			break;
		case 311:	/* sys_process_vm_writev(pid_t pid, const struct iovec __user *lvec, unsigned long liovcnt,
							const struct iovec __user *rvec, unsigned long riovcnt,  unsigned long flags) */
			break;
		case 313:	/* sys_finit_module(int fd, const char __user *uargs, int flags) */
			break;




		case 0:	/* sys_read(unsigned int fd, char __user *buf, size_t count) */
			*p_save = regs->rcx;
			regs->rcx = get_shim_addr(vmcb->cr3);
			p_save ++;
			*p_save = regs->rsi;
			p_save ++;
			*p_save = regs->rdx;
			break;
		case 5:	/* sys_newfstat(unsigned int fd, struct stat __user *statbuf) */
			*p_save = regs->rcx;
			regs->rcx = get_shim_addr(vmcb->cr3);
			p_save ++;
			*p_save = regs->rsi;
			p_save ++;
			*p_save = struct_size[STAT];
			break;
		case 13:	/* sys_rt_sigaction(int, const struct sigaction __user *,
												struct sigaction __user*, size_t); */
			break;
		case 14:	/* sys_rt_sigprocmask(int how, sigset_t __user *set, sigset_t __user *oset,
												size_t sigsetsize) */
			break;
		case 17:	/* sys_pread64(usigned int fd, char __user *buf, size_t count, loff_t pos) */
			break;
		case 19:	/* sys_readv(unsigned long fd, const struct iovec __user *vec, unsigned long vlen) */
			break;
		case 20:	/* sys_writev(unsigned long fd, const struct iovec __user *vec, unsigned long vlen ) */
			break;
		case 21:	/* sys_access(const char __user *filename, int mode) */
			break;
		case 22:	/* sys_pipe(int __user *fildes) */
			break;
		case 23:	/* sys_select(int n, fd_set __user *inp, fd_set __user *outp,
										fd_set __user *exp, struct timeval __user *tvp) */
			break;
		case 36:	/* sys_getitimer(int which, struce itimerval __user *value) */
			break;
		case 40:	/* sys_sendfile64(int out_fd, int in_fd, loff_t __user *offset, size_t count) */
			break;
		case 43:	/* sys_accept(int, struct sockaddr __user *, int __user *) */
			break;
		case 45:	/* sys_recvfrom(int, void __user *, size_t, unsigned) */
			break;
		case 47:	/* sys_recvmsg(int fd, struct msghdr __user *msg, unsigned flags) */
			break;
		case 51:	/* sys_getsockname(int. struct sockaddr __user *, int __user*) */
			break;
		case 52:	/* sys_getpeername(int, struct sockaddr __user *, int __user *) */
			break;
		case 55:	/* sys_getsockopt(int fd, int leve, int optname, char __user *optval, int __user *optlen) */
			break;
		case 61:	/* sys_wait4(pid_t pid, int __user *stat_addr, int options, struct rusage __user *ru) */
			break;
		case 70:	/* sys_msgrcv(int msqid, struct msgbuf __user *msgp,size_t msgz, long msgtyp, int msgflag) */
			break;
		case 78:	/* sys_getdents(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count) */
			break;
		case 79:	/* sys_getcwd(char __user *buf, unsigned long size) */
			break;
		case 96:	/* sys_gettimeofdat(struct timeval __user *ty, struct timezone __user *tz) */
			break;
		case 97:	/* sys_getrlimit(unsigned int resource, struct rlimit __user *rlim) */
			break;
		case 98:	/* sys_getrusage(int who, struct rusage __user *ru) */
			break;
		case 99:	/* sys_sysinfo(struct sysinfo __user *info) */
			break;
		case 100:	/* sys_times(struct tms __user *tbuf) */
			break;
		case 103:	/* sys_syslog(int type, char __user *buf, int len) */
			break;
		case 115:	/* sys_getgroups(int gidsetsize, gid_t __user *grouplist) */
			break;
		case 118:	/* sys_getresuid(uid_t __user *ruid, uid_t __user *euid, uid_t __user *suid) */
			break;
		case 120:	/* sys_getresgid(gid_t __user *rgid, gid_t __user *egid, gid_t __user *sgid) */
			break;
		case 125:	/* sys_capget(cap_user_header_t header, cap_user_data_t dataptr) */
			break;
		case 127:	/* sys_rt_sigpending(sigset_t __user *set, size_t sigsetsize) */
			break;
		case 128:	/* sys_rt_sigtimedwait(const sigset_t __user *uthese, siginfo_t __user *uinfo,
											const struct timespec __user *uts, size_t sigsetsize) */
			break;
		case 129:	/* sys_rt_sigqueueinfo(int pid, int sig, siginfo_t __user *uinfo) */
			break;
		case 136:	/* sys_ustat(unsigned dev, struct ustat __user *buf) */
			break;
		case 137:	/* sys_statfs(const char __user *path, struct statfs __user *buf) */
			break;
		case 138:	/* sys_fstatfs(unsigned int fd, struct statfs __user *buf) */
			break;
		case 139:	/* sys_sysfs(int option, unsigned long arg1, unsigned long arg2) */
			break;
		case 143:	/* sys_sched_getparam(pid_t pid, struct sched_param __user *parm) */
			break;
		case 148:	/* sys_sched_rr_get_interval(pid_t pid, struct timespec __user *interval) */
			break;
		case 155:	/* sys_pivot_root(const char __user *new_root,
										const char __user *put_old); */
			break;
		case 201:	/* sys_time(time_t __user *tloc) */
			break;
		case 202:	/* sys_futex(u32 __user *uaddr, int op, u32 val, struct timespec __user *utime, u32 __user *uaddr2, u32 val3) */
			break;
		case 204:	/* sys_sched_getaffinity(pid_t pid, unsigned int len, unsigned long __user *user_mask_ptr) */
			break;
		case 206:	/* sys_io_setup(unsign nr_reqs, aio_context_t __user *ctx) */
			break;
		case 208:	/* sys_io_getevents(aio_context_t ctx_id, long min_nr,
							long nr, struct io_event __user *events, struct timespec __user *timeout) */
			break;
		case 210:	/* sys_io_cancel(aio_context_t ctx_io, struct iocb __user *iocb, struct io_event __user *result) */
			break;
		case 212:	/* sys_lookup_dcookie(u64 cookie64, char __user *buf, size_t len) */
			break;
		case 217:	/* sys_getdents64(unsigned int fd, srtuct linux_dirent64 __user *dirent, unsigned int count) */
			break;
		case 224:	/* sys_timer_gettime(timer_t timer_id, struct itimerspec __user *setting) */
			break;
		case 228:	/* sys_clock_gettime(clockid_t which_clock, struct timespec __user *tp) */
			break;
		case 229:	/* sys_clock_getres(clockid_t which_clock, struct timespec __user *tp) */
			break;
		case 232:	/* sys_epoll_wait(int epfd, struct epoll_event __user *events, int maxevents, int timeout) */
			break;
		case 233:	/* sys_epoll_ctl(int epfd, int op, int fd, struct epoll_event __user *event) */
			break;
		case 235:	/* sys_utimes(char __user *filename, struct timeval __user *utimes) */
			break;
		case 245:	/* sys_mq_getsetattr(mqd_t mqdes,const struct mq_attr __user *mqstat, struct mq_attr __user *omqstat) */
			break;
		case 247:	/* sys_waitid(int which, pid_t pid, struct siginfo __user *infop, int options, struct rusage __user *ru) */
			break;
		case 249:	/* sys_request_key(const char __user *_type, const char __user *_description, const char __user *_callout_info, key_serial_t desringid) */
			break;
		case 275:	/* sys_splice(int fd_in, loff_t __user *off_in, int fd_out, loff_t __user *off_out,
							size_t len, unsigned int flags) */
			break;
		case 278:	/* sys_vmsplice(int fd, const struct iovec __user *iov, unsigned long nr_segs, unsigned int flags) */
			break;
		case 279:	/* sys_move_pages(pid_t pid, unsigned long nr_pages, const void __user * __user *pages,
							const int __user *nodes, int __user *status, int flags) */
			break;
		case 287:	/* sys_timerfd_gettime(int ufd, struct itimerspec __user *otmr) */
			break;
		case 295:	/* sys_preadv(unsigned long fd, const struct iovec __user *vec,
							unsigned long vlen, unsigned long pos_l, unsigned long pos_h) */
			break;
		case 299:	/* sys_recvmmsg(int fd, struct mmsghdr __user *msg,
							unsigned int vlen, unsigned flags, struct timespec __user *timeout) */
			break;
		case 302:	/* sys_prlimit64(pid_t pid, unsigned int resource, const struct rlimit64 __user *new_rlim,
							struct rlimit64 __user *old_rlim) */
			break;
		case 307:	/* sys_sendmmsg(int fd, struct mmsghdr __user *msg, unsigned int vlen, unsigned flags) */
			break;
		case 309:	/* sys_getcpu(unsigned __user *cpu, unsigned __user *node, struct getcpu_cache __user *cache) */
			break;
		case 310:	/* sys_process_vm_readv(pid_t pid, const struct iovec __user *lvec, unsigned long liovcnt,
							const struct iovec __user *rvec, unsigned long riovcnt, unsigned long flags) */
			break;







		case 3:	/* sys_close(unsigned int fd); */
			break;
		case 8:	/* sys_lseek(unsigned int fd, off_t offset, unsigned int whence) */
			break;
		case 9:	/* sys_mmap(unsigned long, unsigned long, unsigned long, unsigned long,
									unsigned long, unsigned long) */
			break;
		case 10:	/* sys_mprotect(unsigned long start, size_t len, unsigned long prot) */
			break;
		case 11:	/* sys_munmap(unsigned long addr, size_t len) */
			break;
		case 12:	/* sys_brk(unsigned long brk) */
			break;
		case 15:	/* sys_rt_sigreturn(void) */
			break;
		case 16:	/* sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg) */
			break;
		case 24:	/* sys_sched_yield(void) */
			break;
		case 25:	/* sys_mremap(unsigned long addr, unsigned long old_len, unsigned long new_len,
										unsigned long flags, unsigned long new_addr) */
			break;
		case 27:	/* sys_mincore(unsigned long start, size_t len, unsignec char __user *vec) */
			break;
		case 28:	/* sys_madvise(unsigned long start, size_t len, int behavior) */
			break;
		case 29:	/* sys_shmget(key_t key, size_t size, int flag) */
			break;
		case 30: /* sys_shmat(int shmid, char __user *shmaddr, int shmflag) */
			break;
		case 31:	/* sys_shmctl(int shmid, int cmd, struct shmid_ds ___user *buf) */
			break;
		case 32:	/* sys_dup(unsigned int fildes) */
			break;
		case 33:	/* sys_dup2(unsigned int oldfd, unsigned int newfd) */
			break;
		case 34:	/* sys_pause(void) */
			break;
		case 37:	/* sys_alarm(unsigned int seconds) */
			break;
		case 39:	/* sys_getpid(void) */
			break;
		case 41:	/* sys_socket(int, int, int) */
			break;
		case 48: /* sys_shutdown(int, int) */
			break;
		case 50:	/* sys_listen(int, int) */
			break;
		case 57:	/* sys_fork(void) */
			break;
		case 58:	/* sys_vfrok(void) */
			break;
		case 60:	/* sys_exit(int error_code) */
			break;
		case 62:	/* sys_kill(int pid, int sig) */
			break;
		case 64:	/* sys_semget(key_t key, int nsems, int semflag) */
			break;
		case 66:	/* sys_semctl(int semid, int semnum, int cmd, unsigned long arg) */
			break;
		case 68:	/* sys_msgget(key_t key, int msgflag) */
			break;
		case 72:	/* sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg) */
			break;
		case 73:	/* sys_flock(unsigned int fd, unsigned int cmd) */
			break;
		case 74:	/* sys_fsync(unsigned int fd) */
			break;
		case 75:	/* sys_fdatasync(unsigned int fd) */
			break;
		case 77:	/* sys_ftruncate(unsigned int fd, unsigned long length) */
			break;
		case 81:	/* sys_fchdir(unsigned int fd) */
			break;
		case 95:	/* sys_umask(int mask) */
			break;
		case 101:	/* sys_ptrace(long request, long pid, unsigned long addr, unsigned long data) */
			break;
		case 102:	/* sys_getuid(void) */
			break;
		case 104:	/* sys_getgid(void) */
			break;
		case 105:	/* sys_setuid(uid_t udi) */
			break;
		case 106:	/* sys_setgid(gid_t gid) */
			break;
		case 107:	/* sys_geteuid(void) */
			break;
		case 108:	/* sys_getegid(void) */
			break;
		case 109:	/* sys_setpgid(pid_t pid, pid_t pgid) */
			break;
		case 110:	/* sys_getpgid(pid_t pid) */
			break;
		case 111:	/* sys_getpgrp(void) */
			break;
		case 112:	/* sys_setsid(void) */
			break;
		case 113:	/* sys_setreuid(uid_t ruid, uid_t euid) */
			break;
		case 114:	/* sys_setregid(gid_t rgid, gid_t egid) */
			break;
		case 117:	/* sys_setresuid(uid_t ruid, uid_t euid, uid_t suid) */
			break;
		case 119:	/* sys_setresgid(gid_t rgid, gid_t egid, gid_t sgid) */
			break;
		case 121:	/* sys_getpgid(pid_t pid) */
			break;
		case 122:	/* sys_setfsuid(uid_t uid) */
			break;
		case 123:	/* sys_setfsgid(gid_t gid) */
			break;
		case 124:	/* sys_getsid(pid_t pid) */
			break;
		case 134:	/* ~~~~~~~ */
			break;
		case 135:	/* sys_personality(unsigned int personality) */
			break;
		case 140:	/* sys_getpriority(int which, int who) */
			break;
		case 141:	/* sys_setpriority(int which, int who, int niceval) */
			break;
		case 145:	/* sys_sched_getscheduler(pid_t pid) */
			break;
		case 146:	/* sys_sched_get_priority_max(int policy) */
			break;
		case 147:	/* sys_sched_get_priority_mix(int policy) */
			break;
		case 149:	/* sys_mlock(unsigned long start, size_t len); */
			break;
		case 150:	/* sys_munlock(unsigned long start, size_t len) */
			break;
		case 151:	/* sys_mlockall(int flags)  */
			break;
		case 152:	/* sys_munlockall(void) */
			break;
		case 153:	/* sys_vhangup(void) */
			break;
		case 158:	/* sys_arch_prctl(int, unsigned long) */
			break;
		case 159:	/* sys_arch_prctl(int, unsigned long) */
			break;
		case 162:	/* sys_sync(void) */
			break;
		case 172:	/* sys_ni_syscall(void) */
			break;
		case 173:	/* sys_ioperm(unsigned long from, unsigned long num, int on) */
			break;
		case 174:	/* ~~~~~~~~*/
			break;
		case 177:	/* ~~~~~~~~*/
			break;
		case 178:	/* ~~~~~~~ */
			break;
		case 180:	/* ~~~~~~~~~*/
			break;
		case 181:	/* ~~~~~~~~~*/
			break;
		case 182:	/* ~~~~~~~~~*/
			break;
		case 183:	/* ~~~~~~~~~*/
			break;
		case 184:	/* ~~~~~~~~~*/
			break;
		case 185:	/* ~~~~~~~~~*/
			break;
		case 186:	/* sys_gettid(void) */
			break;
		case 187:	/* sys_readahead(int fd, loff_t offset, size_t count) */
			break;
		case 188:	/* sys_setxattr(const char __user *path, const char __user *name,
									const void __user *value, size_t size, int flags) */
			break;
		case 189:	/* sys_lsetxattr(const char __user *path, const char __user *name,
									const void __user *value, size_t size, int flags) */
			break;
		case 190:	/* sys_fsetxattr(int fd, const char __user *name,
									const void __user *value, size_t size, int flags) */
			break;
		case 191:	/* sys_getxattr(const char __user *path, const char __user *name,
									void __user *value, size_t size) */
			break;
		case 192:	/* sys_lgetxattr(const char __user *path, const char __user *name,
									void __user *value, size_t size)*/
			break;
		case 193:	/* sys_fgetxattr(int fd, const char __user *name,
									void __user *value, size_t size) */
			break;
		case 194:	/* sys_listxattr(const char __user *path, char __user *list,
									size_t size) */
			break;
		case 195:	/* sys_llistxattr(const char __user *path, char __user *list, size_t size) */
			break;
		case 196:	/* sys_flistxattr(int fd, char __user *list, size_t size) */
			break;
		case 197:	/* sys_removexattr(const char __user *path, const char __user *name) */
			break;
		case 198:	/* sys_lremovexattr(const char __user *path, const char __user *name) */
			break;
		case 199:	/* sys_fremovexattr(int fd, const char __user *name) */
			break;
		case 200:	/* sys_tkill(int pid, int sig) */
			break;
		case 205:	/* ~~~ */
			break;
		case 207:	/* sys_io_destroy(aio_context_t ctx) */
			break;
		case 211:	/* ~~~ */
			break;
		case 213:	/* sys_epoll_create(int size) */
			break;
		case 214:	/* ~~~ */
			break;
		case 215:	/* ~~~ */
			break;
		case 216:	/* sys_remap_file_pages(unsigned long start, unsigned long size,unsigned long prot, unsigned long pgoff, unsigned long flags) */
			break;
		case 218:	/* sys_set_tid_address(int __user *tidptr) */
			break;
		case 219:	/* sys_restart_syscall(void) */
			break;
		case 221:	/* sys_fadvise64(int fd, loff_t offset, size_t len, int advice) */
			break;
		case 225:	/* sys_timer_getoverrun(timer_t timer_id) */
			break;
		case 226:	/* sys_timer_delete(timer_t timer_id) */
			break;
		case 231:	/* sys_exit_group(int error_code) */
			break;
		case 234:	/* sys_tgkill(int tgid, int pid, int sig) */
			break;
		case 236:	/* ~~~ */
			break;
		case 238:	/* sys_set_mempolicy(int mode, unsigned long __user *nmask, unsigned long maxnode) */
			break;
		case 239:	/* sys_get_mempolicy(int __user *policy, unsigned long __user *nmask, unsigned long maxnode, unsigned long addr, unsigned long flags) */
			break;
		case 246:	/* sys_kexec_load(unsigned long entry, unsigned long nr__segments, struct kexec_segment __user *segments, unsigned long flags) */
			break;
		case 250:	/* sys_keyctl(int cmd, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5) */
			break;
		case 251:	/* sys_ioprio_set(int which, int who, int ioprio) */
			break;
		case 252:	/* sys_ioprio_get(int which, int who) */
			break;
		case 253:	/* sys_inotify_init(void) */
			break;
		case 255:	/* sys_inotify_rm_watch(int fd, __s32 wd) */
			break;
		case 272:	/* sys_unshare(unsigned long unshare_flags) */
			break;
		case 273:	/* sys_set_robust_list(struct robust_list_head __user *head, size_t len) */
			break;
		case 274:	/* sys_get_robust_list(int pid, struct robust_list_head __user * __user *head_ptr,
							size_t __user *len_ptr) */
			break;
		case 276:	/* sys_tee(int fdin, int fdout, size_t len, unsigned int flags) */
			break;
		case 277:	/* sys_sync_file_range(int fd, loff_t offset, loff_t nbytes, unsigned int flags) */
			break;
		case 283:	/* sys_timerfd_create(int clockid, int flags) */
			break;
		case 284:	/* sys_eventfd(unsigned int count) */
			break;
		case 285:	/* sys_fallocate(int fd, int mode, loff_t offset, loff_t len) */
			break;
		case 290:	/* sys_eventfd2(unsigned int count, int flags) */
			break;
		case 291:	/* sys_epoll_create1(int flags) */
			break;
		case 292:	/* sys_dup3(unsigned int oldfd, unsigned int newfd, int flags) */
			break;
		case 294:	/* sys_inotify_init1(int flags) */
			break;
		case 300:	/* sys_fanotify_init(unsigned int flags, unsigned int event_f_flags) */
			break;
		case 306:	/* sys_syncfs(int fd) */
			break;
		case 308:	/* sys_setns(int fd, int nstype) */
			break;
		case 312:	/* sys_kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1, unsigned long idx2) */
			break;
		default:
			while(1)
				lock_cprintf("Unknown syscall number:%d\n", syscall_num);
	}
}
#endif

struct guest_brk_ops brk_record[32];
struct guest_unmap_ops unmap_record[32];
extern void unmask_usr_mem_range(struct vmcb* vmcb, uint64_t addr_start, uint64_t addr_end);
extern void process_exit_handle(struct vmcb *vmcb);
extern uint64_t sys_ret_addr;
extern int test_flag;
extern uint64_t kernel_npt, user_npt;
extern uint64_t npt_travel(uint64_t addr, uint64_t cr3 );



void parse_guest_syscall(struct vmcb *vmcb, struct generl_regs *regs, int flag)
{
}
