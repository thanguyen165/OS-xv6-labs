#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

void
xargs(int argc, char ** argv);

int
main(int argc, char * argv[]) {
  if (argc < 2) {
    fprintf(2, "Usage: xargs COMMAND\n");
    exit(1);
  }

  // Check if the number of arguments exceeds the maximum allowed.
  // We use '>' instead of '>=' because if the user provides no addition arguments,
  // 'argc - 1' could be MAXARG, which is valid. 
  if (argc - 1 > MAXARG) {
    fprintf(2, "Error: Too much arguments\n");
    exit(1);
  }

  // Remove "xargs" from argc and argv, and execute command
  xargs(argc - 1, argv + 1);
  exit(0);
}

int
is_space(char c);

void
xargs(int argc, char ** argv) {
  // Prepare the command to be executed.
  // `args` will be fed into `exec` function
  char * args[MAXARG];
  int args_index = 0;
  for (args_index = 0; args_index < argc; ++args_index) {
    args[args_index] = argv[args_index];
  }

  const int ARRAY_MAX_LENGTH = MAXARG * MAXARG;

  // Read args from pipe and append to (cmd + cmd_index) position
  char cmd[ARRAY_MAX_LENGTH + 1];
  int cmd_index = 0;

  args[args_index++] = & cmd[cmd_index];

  // Buffer to read from pipe.
  // Iterate in array is faster than read byte by byte from pipe.
  char read_buffer[ARRAY_MAX_LENGTH + 1];
  int bytes_read;
  while ((bytes_read = read(0, read_buffer, ARRAY_MAX_LENGTH)) > 0) {
    for (int i = 0; i < bytes_read; ++i) {
      // meet space -> mark new argument, then skip all spaces
      if (is_space(read_buffer[i])) {
        cmd[cmd_index++] = 0;
        args[args_index++] = & cmd[cmd_index];

        if (cmd_index >= ARRAY_MAX_LENGTH) {
          fprintf(2, "Error: Command too long\n");
          exit(1);
        }
        if (args_index >= MAXARG) {
          fprintf(2, "Error: Too much arguments\n");
          exit(1);
        }

        // skip all spaces
        while (i < bytes_read && is_space(read_buffer[i])) {
          ++i;
        }
      }

      // if not the end of command
      if (read_buffer[i] != '\n') {
        cmd[cmd_index++] = read_buffer[i];

        if (cmd_index >= ARRAY_MAX_LENGTH) {
          fprintf(2, "error: command too long\n");
          exit(1);
        }
        continue;
      }

      cmd[cmd_index++] = 0; // Null-terminate the command string

      // Create a new process to execute command
      if (fork() == 0) {
        exec(args[0], args);
        exit(0);
      } else {
        wait(0);
      }

      // return to postion to write new command
      cmd_index = 0;
      args_index = argc + 1;
    }
  }

  exit(0);
}

int
is_space(char c) {
  return (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f');
}