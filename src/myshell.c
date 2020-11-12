#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "exit-utils.h"

#define CMD_BUF_SIZE 1024
#define CMD_ARGS_SIZE 32
#define CMD_BUILTIN_NUMBER 7
#define CMD_EXIT_STR "exit"
#define CMD_HELP_STR "help"
#define CMD_LS_STR "ls"
#define CMD_CP_STR "cp"
#define CMD_RM_STR "rm"
#define CMD_CAT_STR "cat"
#define CMD_HIST_STR "hist"
#define HISTORY_FILENAME ".history"
#define STR_EMPTY ""
#define STR_SPACE " "

static const int kExecFailed = 1;
static int sysCmd(char command[CMD_BUF_SIZE])
{
  printf(">>>> command=%s\n", command);
  pid_t pid = fork();
  if (pid == 0)
  {
    char *arguments[] = {"/bin/sh", "-c", (char *)command, NULL};
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

void init()
{
  printf("+======================+\n");
  printf("|       04 - mysh      |\n");
  printf("+======================+\n");
  printf("User: %s\n\n", getenv("USER"));
  read_history(HISTORY_FILENAME);
}

/**
 * Built-in Command: QUIT
 */
void quit()
{
  char ch;

  printf(">>>> Are you sure (y/n)? ");
  ch = fgetc(stdin);

  if (ch == 'Y' || ch == 'y')
  {
    write_history(HISTORY_FILENAME);
    exit(0);
  }
}
/**
 * Built-in Command: HELP 
 */
void showHelp()
{
  printf("+======================+\n");
  printf("|       mysh help      |\n");
  printf("+======================+");
  puts(
      "\nList of supported commands:"
      "\n> cat"
      "\n> cd"
      "\n> ls"
      "\n> rm"
      "\n> hist"
      "\n> help"
      "\n> exit");
  return;
}
/**
 * Built-in Command: CAT <filename>
 */
void cmdCat(char *filename)
{
  char ch;
  FILE *fp;

  printf(">>>>\n");

  fp = fopen(filename, "r");

  if (fp == NULL)
  {
    printf(">>>> [ERROR] cannot open %s", filename);
    return;
  }

  while ((ch = fgetc(fp)) != EOF)
  {
    putchar(ch);
  }
  fclose(fp);
}
/**
 * Built-in Command: CP <source> <destination>
 */
void cmdCp(char *src, char *dest)
{
  FILE *fpin;
  FILE *fpout;
  char ch;

  fpin = fopen(src, "rb");
  fpout = fopen(dest, "wb");

  while ((ch = fgetc(fpin)) != EOF)
  {
    fputc(ch, fpout);
  }

  fclose(fpout);
  fclose(fpin);
}
/**
 * Built-in Command: LS <dir>
 */
void cmdLs(char *dir)
{
  DIR *ds;

  ds = opendir(dir);
  if (ds == NULL)
  {
    printf(">>>> [ERROR] cannot open %s", dir);
    return;
  }

  struct dirent *file_in_dir;

  printf(">>>>\n");
  while ((file_in_dir = readdir(ds)) != NULL)
  {
    printf(">>>> %s\n", file_in_dir->d_name);
  }
  closedir(ds);
}
/**
 * Built-in Command: RM <filename>
 */
void cmdRm(char *filename)
{
  char ch;
  int retCode;

  printf(">>>> Remove %s (y/n)? ", filename);
  ch = fgetc(stdin);

  if (ch == 'N' || ch == 'n')
  {
    return;
  }

  retCode = remove(filename);

  if (retCode == 0)
  {
    printf(">>>> %s removed", filename);
  }
  else
  {
    printf(">>>> [ERROR] %s NOT removed", filename);
  }
}
/**
 * Built-in Command: HIST
 */
void cmdHist(void)
{
  HISTORY_STATE *histState = history_get_history_state();

  printf(">>>> %d commands in history\n", histState->length);

  for (int i = 0; i < histState->length; i++)
  {
    printf(">>>> %s\n", (char *)histState->entries[i]->line);
  }
}
/**
 * Command Handler
 */
int cmdHandler(char args[CMD_ARGS_SIZE][CMD_BUF_SIZE])
{
  char builtInCommands[CMD_BUILTIN_NUMBER][CMD_BUF_SIZE];
  char command[CMD_BUF_SIZE];
  int switchCmdNum = 0;
  int i = 1;

  // Load the built-in command array
  strcpy(builtInCommands[0], CMD_EXIT_STR);
  strcpy(builtInCommands[1], CMD_HELP_STR);
  strcpy(builtInCommands[2], CMD_CAT_STR);
  strcpy(builtInCommands[3], CMD_CP_STR);
  strcpy(builtInCommands[4], CMD_LS_STR);
  strcpy(builtInCommands[5], CMD_RM_STR);
  strcpy(builtInCommands[6], CMD_HIST_STR);

  // Convert to integer for faster case processing
  for (int i = 0; i < CMD_BUILTIN_NUMBER; i++)
  {
    if (strcmp(args[0], builtInCommands[i]) == 0)
    {
      switchCmdNum = i + 1;
      break;
    }
  }

  switch (switchCmdNum)
  {
  case 1:
    quit();
    break;
  case 2:
    showHelp();
    break;
  case 3:
    cmdCat(args[1]);
    break;
  case 4:
    cmdCp(args[1], args[2]);
    break;
  case 5:
    cmdLs(args[1]);
    break;
  case 6:
    cmdRm(args[1]);
    break;
  case 7:
    cmdHist();
    break;
  default:
    i = 1; // Just being extra SAFE!
    strcpy(command, args[0]);
    while (strcmp(args[i], STR_EMPTY) != 0)
    {
      strcat(command, STR_SPACE);
      strcat(command, args[i]);
      i = i + 1;
    }
    printf(">>>> Linux RetCode=%d\n", sysCmd(command));
    break;
  }

  return 0;
}

int readStdin(char *str)
{
  char *buf = malloc(CMD_BUF_SIZE);

  buf = readline("\n>>>> ");
  add_history(buf);
  strcpy(str, buf);
  free(buf);

  if (strlen(str) != 0)
  {
    return 0;
  }

  return 1;
}
/**
 *  parsed_args[0] = Linux system command (ex. ls, ping, cat, ...)
 *  parsed_args[1] = first argument
 *  parsed_args[2] = second argument
 *  parsed_args[3] = third argument
 *  ... and so on ...
 */
void parseStdin(char cmd[CMD_BUF_SIZE], char parsed_args[CMD_ARGS_SIZE][CMD_BUF_SIZE])
{
  int i = 0;

  for (char *t = strtok(cmd, " "); t != NULL; t = strtok(NULL, " "))
  {
    strcpy(parsed_args[i++], t);
  }
}

void printDir()
{
  char cwd[CMD_BUF_SIZE];
  getcwd(cwd, sizeof(cwd));
  printf("\ndir> %s", cwd);
}

void resetBuffers(char cmd[CMD_BUF_SIZE], char parsed_args[CMD_ARGS_SIZE][CMD_BUF_SIZE])
{
  strcpy(cmd, STR_SPACE);
  for (int i = 0; i < CMD_ARGS_SIZE; i++)
  {
    strcpy(parsed_args[i], STR_EMPTY);
  }
}

/**
 * main(): applications entry point
 */
int main()
{
  char cmd[CMD_BUF_SIZE];
  char parsedArgs[CMD_ARGS_SIZE][CMD_BUF_SIZE];

  resetBuffers(cmd, parsedArgs);

  init();

  while (1)
  {
    printDir();
    if (readStdin(cmd))
    {
      continue;
    }
    // Needed for fork() and execvp()
    // Parse the user's input
    parseStdin(cmd, parsedArgs);
    // Handle all commands
    cmdHandler(parsedArgs);
    // Reset all the buffers
    resetBuffers(cmd, parsedArgs);
  }
}
