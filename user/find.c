#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Function to extract the last file name from a given path
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find the character after the last '/'
  for (p = path + strlen(path); p >= path && *p != '/'; p--);
  p++;

  // If the file name is longer than DIRSIZ, return the name directly
  if (strlen(p) >= DIRSIZ)
    return p;

  // Copy the file name into the buffer and pad with spaces
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}

// Function to search for files with the specified filename in the directory tree
void
find(char *path, char *filename)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // Open the file or directory at the given path
  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // Retrieve information about the file or directory
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // If the current object is a file, compare its name with the target filename
  if (st.type == T_FILE) {
    if (strcmp(fmtname(path), filename) == 0) {
      // Print the full path if the filename matches
      printf("%s\n", path);
    }
  }
  // If the current object is a directory, recursively search within it
  else if (st.type == T_DIR) {
    // Ensure the path isn't too long to store in the buffer
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      fprintf(2, "find: path too long\n");
      close(fd);
      return;
    }
    strcpy(buf, path);  // Copy the current path into the buffer
    p = buf + strlen(buf);
    *p++ = '/';  // Add '/' to navigate deeper into the directory

    // Read each directory entry (file or subdirectory) within the current directory
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      // Skip entries that are empty or invalid
      if (de.inum == 0)
        continue;
      // Copy the name of the current entry to the path
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;  // Null-terminate the string

      // Skip the current directory (.) and parent directory (..)
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      // Compare the current file or directory name with the target filename
      if (strcmp(de.name, filename) == 0) {
        printf("%s\n", buf);  // If it matches, print the full path
      }

      // Recursively search in the subdirectory or file
      find(buf, filename);
    }
  }
  close(fd);  // Close the file descriptor when done
}

int
main(int argc, char *argv[])
{
  // Ensure the correct number of arguments are provided (path and filename)
  if (argc != 3) {
    fprintf(2, "Usage: find <path> <filename>\n");
    exit(1);
  }

  // Start the find operation at the specified path and filename
  find(argv[1], argv[2]);
  exit(0);
}
