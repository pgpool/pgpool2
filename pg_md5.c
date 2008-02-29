#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <termio.h>

#include "md5.h"


/* Maximum number of characters allowed for input. */
#define MAX_INPUT_SIZE	32

void	print_usage(const char prog[], int exit_code);
void	set_tio_attr(int enable);

int
main(int argc, char *argv[])
{

#define PRINT_USAGE(exit_code)	print_usage(argv[0], exit_code)
#define COMPARE_ARG(arg)		(!strcmp(argv[1], arg))
	
	if (argc != 2)
		PRINT_USAGE(EXIT_FAILURE);
	else if (COMPARE_ARG("--help") || COMPARE_ARG("-h"))
		PRINT_USAGE(EXIT_SUCCESS);

	/* Prompt for password. */
	else if (COMPARE_ARG("--prompt") || COMPARE_ARG("-p"))
	{
	   	char	 md5[MD5_PASSWD_LEN+1];
		char	 buf[MAX_INPUT_SIZE+1];
		int		 len;

		set_tio_attr(1);
		printf("password: ");
		if (!fgets(buf, (MAX_INPUT_SIZE+1), stdin))
		{
			int eno = errno;

			fprintf(stderr, "Couldn't read input from stdin. (fgets(): %s)",
					strerror(eno));
			
			exit(EXIT_FAILURE);
		}
		set_tio_attr(0);

		/* Remove LF at the end of line, if there is any. */
		len = strlen(buf);
		if (len > 0 && buf[len-1] == '\n')
		{
			buf[len-1] = '\0';
			len--;
		}

		pool_md5_hash(buf, len, md5);
		printf("\n%s\n", md5);
	}

	/* Read password from argv[1]. */
	else
	{
		char	md5[MD5_PASSWD_LEN+1];
		int		len = strlen(argv[1]);

		if (len > MAX_INPUT_SIZE)
		{
			fprintf(stderr, "Error: Input exceeds maximum password length!\n\n");
			PRINT_USAGE(EXIT_FAILURE);
		}

		pool_md5_hash(argv[1], len, md5);
		printf("%s\n", md5);
	}

	return EXIT_SUCCESS;
}


void
print_usage(const char prog[], int exit_code)
{
	fprintf(((exit_code == EXIT_SUCCESS) ? stdout : stderr),
			"Usage:\n\
\n\
  %s [OPTIONS]\n\
  %s <PASSWORD>\n\
\n\
  --prompt, -p    Prompt password using standard input.\n\
  --help, -h      This help menu.\n\
\n\
Warning: At most %d characters are allowed for input.\n\
Warning: Plain password argument is deprecated for security concerns\n\
         and kept for compatibility. Please prefer using password\n\
         prompt.\n",
			prog, prog, MAX_INPUT_SIZE);

	exit(exit_code);
}


void
set_tio_attr(int set)
{
	struct termios tio;
	static struct termios tio_save;


	if (!isatty(0))
	{
		fprintf(stderr, "stdin is not tty\n");
		exit(EXIT_FAILURE);
	}

	if (set)
	{
		if (tcgetattr(0, &tio) < 0)
		{
			fprintf(stderr, "set_tio_attr(set): tcgetattr failed\n");
			exit(EXIT_FAILURE);
		}

		tio_save = tio;

		tio.c_iflag &= ~(BRKINT|ISTRIP|IXON);
		tio.c_lflag &= ~(ICANON|IEXTEN|ECHO|ECHOE|ECHOK|ECHONL);
		tio.c_cc[VMIN] = 1;
		tio.c_cc[VTIME] = 0;

		if (tcsetattr(0, TCSANOW, &tio) < 0)
		{
			fprintf(stderr, "(set_tio_attr(set): tcsetattr failed\n");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		if (tcsetattr(0, TCSANOW, &tio_save) < 0)
		{
			fprintf(stderr, "set_tio_attr(reset): tcsetattr failed\n");
			exit(EXIT_FAILURE);
		}
	}
}
