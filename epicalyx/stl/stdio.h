#ifndef _STDIO_H_
#define _STDIO_H_

#define NULL ((void*)0)
#define EOF (-1)

typedef struct FILE FILE;
extern FILE *stdout, *stderr;

int ungetc(int c, FILE *stream);
int	printf(const char *, ...);
int	putc(int, FILE *);
int	putchar(int);
int snprintf(char *, long long, const char *, ...);
int fprintf(FILE *stream, const char *format, ...);
int fgetc(FILE *stream);
int fputs(const char *s, FILE *stream);
int fputc(int c, FILE *stream);
FILE *fopen(const char *, const char *);
int *fclose(FILE *);

#endif  // _STDIO_H_
