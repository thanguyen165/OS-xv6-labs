#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char * argv[]) {
  const int BUFFER_SIZE = 255;
  char buffer[BUFFER_SIZE + 1];

  int pipe_parent[2]; // pipe from parent to child
  pipe(pipe_parent);

  int pipe_child[2]; // pipe from child to parent
  pipe(pipe_child);

  int pid = fork();
  if (pid > 0) { // in parent process
    write(pipe_parent[1], "ping", strlen("ping"));
    close(pipe_parent[1]); // close write pipe

    wait(0); // wait child process pong

    // read data from pipe
    int read_length = read(pipe_child[0], buffer, BUFFER_SIZE);
    buffer[read_length] = 0; // NULL terminate of string

    printf("%d: received %s\n", getpid(), buffer);
    close(pipe_child[0]); // close read pipe
  }
  else if (pid == 0) { // in child process
    // read data from pipe
    int read_length = read(pipe_parent[0], buffer, BUFFER_SIZE);
    buffer[read_length] = 0; // NULL terminate of string

    printf("%d: received %s\n", getpid(), buffer);
    close(pipe_parent[0]); // close read pipe

    write(pipe_child[1], "pong\n", strlen("pong"));
    close(pipe_child[1]);
    exit(0);
  }
  else {
    printf("fork failed\n");
    exit(1);
  }

  exit(0);
}