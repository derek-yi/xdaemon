#ifndef _UPCFG_MAIN_H_
#define _UPCFG_MAIN_H_

typedef struct _FIELD_INFO
{
    char *field_name;
    uint32 reg_addr;
    uint32 start_bit;
    uint32 mask;
    char *field_desc;
    char *value_desc;
}FIELD_INFO;


int upcfg_cli_init();

int upcfg_init(char *cfg_file);



#endif
