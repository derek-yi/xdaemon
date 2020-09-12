



#ifndef _TINY_CLI_H_
#define _TINY_CLI_H_


#define CMD_OK                  0x00
#define CMD_ERR                 0x01
#define CMD_ERR_PARAM           0x02
#define CMD_ERR_NOT_MATCH       0x03
#define CMD_ERR_AMBIGUOUS       0x04

typedef int (* CMD_FUNC)(int argc, char **argv);

int cli_cmd_reg(char *cmd, char *help, CMD_FUNC func);

void cli_main_task(void);


#endif

