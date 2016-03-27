#include "apue.h"
#include <errno.h>
#include <stdarg.h>
#include <sys/resource.h>
//#include <bits/fcntl-linux.h>
#include <fcntl.h>
#include <sys/syslog.h>

static void err_doit(int, int, const char *, va_list);

void err_ret(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

void err_sys(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

void err_cont(int error, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

void err_exit(int error, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

void err_dump(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, 0, fmt, ap);
	va_end(ap);
	//abord();
	exit(1);
}

void err_msg(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, 0, fmt, ap);
	va_end(ap);
}

void err_quit(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(1, 0, fmt, ap);
	va_end(ap);
	exit(1);
}

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char buf[MAXLINE];
	vsnprintf(buf, MAXLINE-1, fmt, ap);
	if(errnoflag)
		snprintf(buf + strlen(buf), MAXLINE - strlen(buf)-1, "; %s",
			strerror(error));
	strcat(buf, "\n");
	fflush(stdout);
	fputs(buf,stderr);
	fflush(NULL);
}


/**
 * 守护进程
 */
void daemonize(const char *cmd) {
	int 				i, fd0, fd1, fd2;
	pid_t 				pid;
	struct rlimit		rl;
	struct sigaction	sa;

	/**
	 * Clear file creation mask.
	 */
	umask(0);

	/**
	 * Get maximum number of file descriptors.
	 */
	if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
		err_quit("%s: can't get file limit", cmd);

	/**
	 * Become a session leader to lose controlling TTY
	 */
	if((pid = fork()) < 0)
		err_quit("%s: can't fork.");
	else if(pid != 0)/*parent*/
		exit(0);
	setsid();

	/*
	 * Ensure future opens won't allocate controlling TTY.
	 */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if(sigaction(SIGHUP, &sa, NULL) < 0)
		err_quit("%s: can't ignore SIGHUP", cmd);
	if((pid = fork()) < 0)
		err_quit("%s: can't fork", cmd);
	else if(pid != 0) /* parent */
		exit(0);

	/*
	 * Change the current working directory to the root so
	 * we won't prevent file systems from being ummounted.
	 */
	if(chdir("/") < 0)
		err_quit("%s: can't change directory to /", cmd);

	/**
	 * Close all open file descriptors.
	 */
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; ++i) {
		close(i);
	}

	/**
	 * Attach file descriptors 0, 1 and 2 to /dev/null
	 */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	/*
	 * Initialize the log fiile.
	 */
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2)
	{
		syslog(LOG_ERR, "unexpeccted file descriptors %d %d %d", fd0, fd1, fd2);
		exit(1);
	}
}

/*
 * 设置执行时关闭
 */
int set_cloexec(int fd)
{
	int 			val;

	if ((val = fcntl(fd, F_GETFD, 0)) < 0)
		return (-1);

	val |= FD_CLOEXEC;	/* enabel close-on-exec */

	return (fcntl(fd, F_SETFD, val));
}
