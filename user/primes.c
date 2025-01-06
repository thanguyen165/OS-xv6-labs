#include "kernel/types.h"
#include "user/user.h"

int
sieve(int in_fd, int first_prime);

int
main(int argc, char * argv[]) {
  int THRESHOLD = 280; // default value
  if (argc == 2) {
    THRESHOLD = atoi(argv[1]);
  }

  int p[2];
  pipe(p);

  // child process to push initial numbers into pipe
  // since pipe can contain only about 120 integers
  if (fork() == 0) {
    for (int i = 2; i < THRESHOLD; ++i) {
      write(p[1], & i, sizeof(int));
    }

    close(p[1]);
    exit(0);
  }
  close(p[1]);

  int first_prime;
  while (read(p[0], &first_prime, sizeof(int))) {
    printf("prime %d\n", first_prime);
    p[0] = sieve(p[0], first_prime);
  }

  exit(0);
}

int
sieve(int in_fd, int first_prime) {
  int out_pipe[2];
  pipe(out_pipe);

  if (fork() == 0) { // create child process
    int num;
    while (read(in_fd, & num, sizeof(int))) {
      if (num % first_prime) {
        write(out_pipe[1], & num, sizeof(int));
      }
    }

    close(in_fd);
    close(out_pipe[1]);
    exit(0);
  }

  close(in_fd);
  close(out_pipe[1]);

  return out_pipe[0];
}