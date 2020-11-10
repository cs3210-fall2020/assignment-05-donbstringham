/**
 * File: sys-cmd.c
 * ----------------
 * Implements our own system function (which I call so that it doesn't compete with
 * the built-in system function).
 */

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "string.h"
#include "exit-utils.h"

static const int kExecFailed = 1;
static int sysCmd(const char *command) {
  pid_t pid = fork();
  if (pid == 0) {
    char *arguments[] = {"/bin/sh", "-c", (char *) command, NULL};
    // char *arguments[] = {"ls", "-alF", NULL};
    execvp(arguments[0], arguments);
    exitIf(true, kExecFailed, stderr, "[CHILD] execvp failed to invoke this: %s.\n", command);
  }

  int status;
  waitpid(pid, &status, 0);

  if (WIFEXITED(status))
    return WEXITSTATUS(status);
  else
    return -WTERMSIG(status);
}

static const size_t kMaxLine = 2048;
int main(int argc, char *argv[]) {
  char command[kMaxLine];

  while (true) {
    printf("> ");
    if (fgets(command, kMaxLine, stdin) == NULL) break;
    if (feof(stdin)) break;
    command[strlen(command) - 1] = '\0'; // overwrite '\n'
    printf("[PARENT] retcode=%d\n", sysCmd(command));
  }

  printf("\n");
  return 0;
}