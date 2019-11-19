#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

void msleep(int length) { //XXX this is not good
  for (int i = 0; i<length*50000; i++) {
    asm volatile("nop");
  }
}

void note(int length, int frq) {
  beep(frq);
  msleep(length);
  beep(0);
}

/* Stolen from the Linux tool */
#define DEFAULT_FREQ  440.0
#define DEFAULT_LEN   200
#define DEFAULT_DELAY 100

static int repetitions = 1;
static float frequency = DEFAULT_FREQ;
static int length      = DEFAULT_LEN;
static int delay       = DEFAULT_DELAY;
static int beep_after  = 0;

void snd(void) {
  for (int i = 0; i < repetitions; ++i) {
    note(length, frequency);
    if (delay && ((i != repetitions - 1) || beep_after)) {
      msleep(delay);
    }
  }
}

int main(int argc, char** argv)
{
  int opt;
 
  while ((opt = getopt(argc, argv, "?hr:f:l:d:D:n")) != -1) {
    switch (opt) {
    case 'h':
    case '?':
      fprintf(stderr, "usage: %s BEEP...\n"
              "Where BEEP consists of:\n"
              "  -r  REPS  \033[3mNumber of repetitions.\033[0m\n"
              "  -f  FREQ  \033[3mFrequency in Hz. 440 is A4. Supports fractional values.\033[0m\n"
              "  -l  TIME  \033[3mDuration in milliseconds.\033[0m\n"
              "  -d  TIME  \033[3mDelay between repetitions in milliseconds.\033[0m\n"
              "  -D  TIME  \033[3mDelay between, and after, repetitions.\033[0m\n"
              "  -n        \033[3mStart a new beep.\033[0m\n"
              "\n"
              "The default values are:\n"
              "   -r 1 -l %d -f %.2f -d %d\n"
              "\n"
              "A length of -1 will start a sustained beep without blocking.\n"
              "A length of 0 will stop a currently playing sustained beep.\n",
              argv[0], DEFAULT_LEN, DEFAULT_FREQ, DEFAULT_DELAY);
      return 1;
    case 'r':
      repetitions = atoi(optarg);
      break;
      break;
    case 'l':
      length = atoi(optarg);
      break;
    case 'f':
      frequency = atof(optarg);
      break;
    case 'd':
      delay = atoi(optarg);
      beep_after = 0;
      break;
    case 'D':
      delay = atoi(optarg);
      beep_after = 1;
      break;
    case 'n':
      snd();
      repetitions = 1;
      frequency = DEFAULT_FREQ;
      length = DEFAULT_LEN;
      delay = DEFAULT_DELAY;
      beep_after = 0;
      break;
    }
  }

  snd();
  
  return 0;
}
