/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
  // perror looks at global errno variable and prints a error message
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  // get terminal attributes
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcsetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  // When BRKINT is turned on, a break condition will cause a SIGINT signal to be sent to the program, like pressing Ctrl-C
  // Disable ICRNL stops carriage return translation to 10 (ctrl + m)
  // INPCK enables parity checking, which doesn’t seem to apply to modern terminal emulators.
  // ISTRIP causes the 8th bit of each input byte to be stripped, meaning it will set it to 0. This is probably already turned off
  // IXON Disable ctrl + s and ctrl + q
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  // CS8 is not a flag, it is a bit mask with multiple bits, which we set using the bitwise-OR (|) operator unlike all the flags we are turning off. It sets the character size (CS) to 8 bits per byte
  raw.c_cflag |= (CS8);

  // stop output processing (translating \n to \r\n)
  raw.c_oflag &= ~(OPOST);

  // c_lflag = local flag
  // ECHO is a bitflag
  // IEXTEN is ctrv + v
  // ISIG is SIGINT and SIGTSTP
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // TCSAFLUSH specifies when to apply changes (Wait for all pending output to be written)

  // sets the minimum number of bytes of input needed before read() can return
  raw.c_cc[VMIN] = 0;
  // sets the maximum amount of time to wait before read() returns in tenths of a second (1/10 or 100ms in this case)
  raw.c_cc[VTIME] = 1;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

/*** init ***/

int main()
{
  enableRawMode();
  char c;
  while (1) {
    c = '\0';
    if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if(iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      // %d - format as a decimal number
      // %c - write the byte as a character
      printf("%d ('%c')\r\n", c, c);
    }

    if(c == 'q') break;
  };

  return 0;
}