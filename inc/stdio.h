#ifndef JOS_INC_STDIO_H
#define JOS_INC_STDIO_H

#include <inc/types.h>
#include <stdarg.h>

#ifndef NULL
#define NULL	((void *) 0)
#endif /* !NULL */

// lib/stdio.c
void	cputchar(int c);
int	getchar(void);
int	iscons(int fd);

// lib/sprintf.c
int	snprintf(char *str, size_t size, const char *fmt, ...);
int	vsnprintf(char *str, size_t size, const char *fmt, va_list);

// lib/printfmt.c
int sprintf(char *buf, const char *fmt, ...);

// lib/fprintf.c
int	printf(const char *fmt, ...);
int	fprintf(int fd, const char *fmt, ...);
int	vfprintf(int fd, const char *fmt, va_list);

// lib/readline.c
char*	readline(const char *prompt);

// kern/mmap.c

void *tmalloc(size_t size);
int tfree(void *start);

/* lib/printfmt.c */
void	printfmt(void (*putch)(int, void*), void *putdat,
	    const char *fmt, ...)
	    __attribute__((__format__ (__printf__, 3, 4)));
void	vprintfmt(void (*putch)(int, void*), void *putdat,
	    const char *fmt, va_list)
	    __attribute__((__format__ (__printf__, 3, 0)));

const char *e2s(int err);
const char *syscall2s(int sys);



/* lib/printf.c */
extern struct sem print_sem;
int debug_printf(const char *fmt, ...)
		__attribute__((__format__ (__printf__, 1, 2)));
int lock_cprintf(const char *fmt, ...)
		__attribute__((__format__ (__printf__, 1, 2)));
int	cprintf(const char *fmt, ...)
	    __attribute__((__format__ (__printf__, 1, 2)));
void	cflush(void);
int	vcprintf(const char *fmt, va_list)
	    __attribute__((__format__ (__printf__, 1, 0)));

#endif /* !JOS_INC_STDIO_H */
