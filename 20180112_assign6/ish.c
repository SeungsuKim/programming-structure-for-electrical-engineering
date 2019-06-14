/*--------------------------------------------------------------------*/
/* ish.c                                                              */
/* Author: Seungsu Kim, Youngmoo Kim                                  */
/*--------------------------------------------------------------------*/


#include "dynarray.h"
#include "lexAnalyzer.h"
#include "synAnalyzer.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>

pid_t pid;

char *pcPgmName;
int isAlarmSet;

enum { MAX_LINE_SIZE = 1024 };
enum { FALSE, TRUE };

static void sigQuitHandler(int iSignal) {
    if (isAlarmSet) {
        exit(0);
    } else {
        printf("\n");
        printf("Type Ctrl-\\ again within 5 seconds to exit.\n");
        alarm(5);
        isAlarmSet = TRUE;
    }   
}

static void sigAlarmHandler(int iSignal) {
    isAlarmSet = FALSE;
}

static int command(Command_T pCommand, void **args, int input, int first, int last) {
    int fds[2], inFd, outFd;
    char **ppcArgs;
    void (*pfRet)(int);

    assert(pCommand != NULL);
    assert(args != NULL);

    if (pipe(fds) == -1) {
        perror(args[0]);
        exit(EXIT_FAILURE);
    }

    ppcArgs = (char **)args;

    fflush(NULL);
    pid = fork();
    if (pid == 0) {
        pfRet = signal(SIGINT, SIG_DFL);
        if (pfRet == SIG_ERR) {
            perror(ppcArgs[0]);
            exit(EXIT_FAILURE);
        }
        pfRet = signal(SIGQUIT, SIG_DFL);
        if (pfRet == SIG_ERR) {
            perror(ppcArgs[0]);
            exit(EXIT_FAILURE);
        }

        if (first && !last && !input) {
            dup2(fds[1], 1);
        } else if (!first && !last && input) {
            dup2(input, 0);
            dup2(fds[1], 1);
        } else {
            dup2(input, 0);
        }

        if (pCommand->input != NULL) {
            inFd = open(pCommand->input, O_RDONLY);
            if (inFd < 0) {
                fprintf(stderr, "Failed to open %s for input redirection.\n", pCommand->input);
                exit(EXIT_FAILURE);
            }
            dup2(inFd, 0);
            close(inFd);
        }
        if (pCommand->output != NULL) {
            outFd = creat(pCommand->output, 0600);
            if (outFd < 0) {
                fprintf(stderr, "Failed to create %s for output recirection.\n", pCommand->output);
                return(EXIT_FAILURE);
            }
            dup2(outFd, 1);
            close(outFd);
        }

        execvp(ppcArgs[0], ppcArgs);
        perror(ppcArgs[0]);
        exit(EXIT_FAILURE);
    } else {
        waitpid(pid, 0, 0);
    }

    if (last) close(fds[0]);
    if (input) close(input);
    close(fds[1]);
    return fds[0];
}

void execute(DynArray_T oCommands) {
    int i, numCommands, numPipes, numArgs, input, first, iSuccesful;
    char *cmd;
    void **args;
    Command_T pCommand;

    assert(oCommands != NULL);

    numCommands = DynArray_getLength(oCommands);
    numPipes = numCommands-1;

    if (numCommands > 0) {
        pCommand = (Command_T)DynArray_get(oCommands, 0);
        cmd = (char *)DynArray_get(pCommand->oArgs, 0);
        if (strcmp(cmd, "exit") == 0) {
            exit(0);
        }
        else if (strcmp(cmd, "cd") == 0) {
            char *homedir, *dir;
            homedir = getenv("HOME");
            dir = DynArray_get(pCommand->oArgs, 1);
            if (dir == NULL) {
                chdir(homedir);
            } else if ((strcmp(dir, "~")==0) || (strcmp(dir, "~/")==0)) {
                chdir(homedir);
            } else if (chdir(dir) < 0) {
                fprintf(stderr, "%s: No such file or directory.\n", pcPgmName);
            }
        }
        else if (strcmp(cmd, "setenv") == 0) {
            numArgs = DynArray_getLength(pCommand->oArgs);
            if (numArgs != 4) {
                fprintf(stderr, "%s: Setenv need 2 parameters.\n", pcPgmName);
            } else {
                char *name = (char *)DynArray_get(pCommand->oArgs, 1);
                char *value = (char *)DynArray_get(pCommand->oArgs, 2);
                iSuccesful = setenv(name, value, 0);
                if (iSuccesful == -1) {
                    fprintf(stderr, "%s: Setenv failed.\n", pcPgmName);
                }
            }
        }
        else if (strcmp(cmd, "unsetenv") == 0) {
            numArgs = DynArray_getLength(pCommand->oArgs);
            if (numArgs != 3) {
                fprintf(stderr, "%s: Unsetenv need 1 parameter.\n", pcPgmName);
            } else {
                char *name = (char *)DynArray_get(pCommand->oArgs, 1);
                iSuccesful = unsetenv(name);
                if (iSuccesful == -1) {
                    fprintf(stderr, "%s: Unsetenv failed.\n", pcPgmName);
                }
            }
        } else {
            for (i = 0, input = 0, first = TRUE; i < numPipes; i++) {
                pCommand = (Command_T) DynArray_get(oCommands, i);
                numArgs = DynArray_getLength(pCommand->oArgs);
                args = calloc((size_t) numArgs, sizeof(char *));
                if (args == NULL) {
                    fprintf(stderr, "%s: Can't allocate a memory for args\n", pcPgmName);
                } else {
                    DynArray_toArray(pCommand->oArgs, args);
                    input = command(pCommand, args, input, first, FALSE);
                    free(args);
                }
                first = FALSE;
            }
            pCommand = (Command_T) DynArray_get(oCommands, numPipes);
            numArgs = DynArray_getLength(pCommand->oArgs);
            args = calloc((size_t) numArgs, sizeof(char *));
            if (args == NULL) {
                fprintf(stderr, "%s: Can't allocate a memory for args\n", pcPgmName);
            } else {
                DynArray_toArray(pCommand->oArgs, args);
                command(pCommand, args, input, first, TRUE);
                free(args);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char acLine[MAX_LINE_SIZE] = {0};
    DynArray_T oTokens, oCommands;
    int iSuccesful;
    void (*pfRet)(int);
    char *homedir, *ishrcdir;
    FILE *ishrc;

    pcPgmName = argv[0];
    isAlarmSet = FALSE;

    pfRet = signal(SIGINT, SIG_IGN);
    if (pfRet == SIG_ERR) {
        perror(pcPgmName);
        exit(EXIT_FAILURE);
    }
    pfRet = signal(SIGQUIT, sigQuitHandler);
    if (pfRet == SIG_ERR) {
        perror(pcPgmName);
        exit(EXIT_FAILURE);
    }
    pfRet = signal(SIGALRM, sigAlarmHandler);
    if (pfRet == SIG_ERR) {
        perror(pcPgmName);
        exit(EXIT_FAILURE);
    }

    homedir = getenv("HOME");
    ishrcdir = (char *)calloc((size_t)MAX_LINE_SIZE, sizeof(char));
    ishrcdir = strdup(homedir);
    strcat(ishrcdir, "/ishrc");
    ishrc = fopen(ishrcdir, "r");
    if (ishrc == NULL) {
        fprintf(stderr, "No ishrc in home directory.\n");
    } else {

        while(fgets(acLine, MAX_LINE_SIZE, ishrc) != NULL) {
            printf("%% %s", acLine);
            oTokens = DynArray_new(0);
            if (oTokens == NULL) {
                fprintf(stderr, "%s: Can't allocate a memory for oTokens.\n", pcPgmName);
                exit(EXIT_FAILURE);
            }
            iSuccesful = lexLine(acLine, oTokens, pcPgmName);
            if (iSuccesful) {
                iSuccesful = synTokens(oTokens, pcPgmName);
                if (iSuccesful) {
                    oCommands = DynArray_new(0);
                    if (oCommands == NULL) {
                        fprintf(stderr, "%s: Can't allocate a memory for oCommands.\n", pcPgmName);
                        exit(EXIT_FAILURE);
                    }
                    iSuccesful = genCommands(oTokens, oCommands, pcPgmName);
                    if (iSuccesful) {
                        execute(oCommands);
                    } 
                    DynArray_map(oCommands, freeCommand, NULL);
                    DynArray_free(oCommands);
                }
            }
            DynArray_map(oTokens, freeToken, NULL); 
            DynArray_free(oTokens);

            memset(acLine, 0, sizeof(acLine));

        }
    }

    while(printf("%% "), fgets(acLine, MAX_LINE_SIZE, stdin) != NULL) {

        oTokens = DynArray_new(0);
        if (oTokens == NULL) {
            fprintf(stderr, "%s: Can't allocate a memory for oTokens.\n", pcPgmName);
            exit(EXIT_FAILURE);
        }
        iSuccesful = lexLine(acLine, oTokens, pcPgmName);
        if (iSuccesful) {
            iSuccesful = synTokens(oTokens, pcPgmName);
            if (iSuccesful) {
                oCommands = DynArray_new(0);
                if (oCommands == NULL) {
                    fprintf(stderr, "%s: Can't allocate a memory for oCommands.\n", pcPgmName);
                    exit(EXIT_FAILURE);
                }
                iSuccesful = genCommands(oTokens, oCommands, pcPgmName);
                if (iSuccesful) {
                    execute(oCommands);
                } 
                DynArray_map(oCommands, freeCommand, NULL);
                DynArray_free(oCommands);
            }
        }
        DynArray_map(oTokens, freeToken, NULL); 
        DynArray_free(oTokens);

        memset(acLine, 0, sizeof(acLine));
    }

    return 0;
}

