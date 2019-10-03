/*
 * This code is in the public domain; do with it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> in Jul 2019
 */
#ifndef UTIL_LINUX_PTY_SESSION_H
#define UTIL_LINUX_PTY_SESSION_H

#include <pty.h>
#include <termios.h>
#include <signal.h>
#include <sys/time.h>

#include <sys/signalfd.h>

/*
 * Callbacks -- the first argument is always callback data, see
 * ul_pty_set_callback_data().
 */
struct ul_pty_callbacks {
	/*
	 * Executed on SIGCHLD when ssi_code is EXITED, KILLED or DUMPED
	 * @
	 */
	void (*child_wait)(void *);

	/*
	 * Executed on SIGCHLD when ssi_status is SIGSTOP
	 */
	void (*child_sigstop)(void *);

	/*
	 * Executed in master loop before ul_pty enter poll() and in time set by
	 * ul_pty_set_mainloop_time(). The callback is no used when time is not set.
	 */
	int (*mainloop)(void *);

	/*
	 * Executed on master or stdin activity, arguments:
	 *   2nd - file descriptor
	 *   3rd - buffer with data
	 *   4th - size of the data
	 */
	int (*log_stream_activity)(void *, int, char *, size_t);

	/*
	 * Executed on signal, arguments:
	 *   2nd - signal info
	 *   3rd - NULL or signal specific data (e.g. struct winsize on SIGWINCH
	 */
	int (*log_signal)(void *, struct signalfd_siginfo *, void *);
};

struct ul_pty {
	struct termios	stdin_attrs;	/* stdin and slave terminal runtime attributes */
	int		master;		/* parent side */
	int		slave;		/* child side */
	int		sigfd;		/* signalfd() */
	int		poll_timeout;
	struct winsize	win;		/* terminal window size */
	sigset_t	orgsig;		/* original signal mask */

	int		delivered_signal;

	struct ul_pty_callbacks	callbacks;
	void			*callback_data;

	pid_t		child;

	struct timeval	next_callback_time;

	unsigned int isterm:1;		/* is stdin terminal? */
};

void ul_pty_init_debug(int mask);
struct ul_pty *ul_new_pty(int is_stdin_tty);
void ul_free_pty(struct ul_pty *pty);

int ul_pty_get_delivered_signal(struct ul_pty *pty);

void ul_pty_set_callback_data(struct ul_pty *pty, void *data);
void ul_pty_set_child(struct ul_pty *pty, pid_t child);

struct ul_pty_callbacks *ul_pty_get_callbacks(struct ul_pty *pty);
int ul_pty_is_running(struct ul_pty *pty);
int ul_pty_setup(struct ul_pty *pty);
void ul_pty_cleanup(struct ul_pty *pty);
void ul_pty_init_slave(struct ul_pty *pty);
int ul_pty_proxy_master(struct ul_pty *pty);

void ul_pty_set_mainloop_time(struct ul_pty *pty, struct timeval *tv);
int ul_pty_get_childfd(struct ul_pty *pty);

#endif /* UTIL_LINUX_PTY_H */
