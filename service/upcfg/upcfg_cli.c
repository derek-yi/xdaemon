

#include "daemon_pub.h"

#include "upcfg_main.h"


int cli_set_work_state(int argc, char **argv)
{
    vos_print("todo: cli_set_work_state\r\n");
    return VOS_OK;
}

int cli_dpd_proc(int argc, char **argv)
{
    vos_print("todo: cli_dpd_proc\r\n");
    return VOS_OK;
}

int cli_set_tx_att(int argc, char **argv)
{
    vos_print("todo: cli_set_tx_att\r\n");
    return VOS_OK;
}

int cli_get_tx_att(int argc, char **argv)
{
    vos_print("todo: cli_get_tx_att\r\n");
    return VOS_OK;
}

int cli_set_rx_gain(int argc, char **argv)
{
    vos_print("todo: cli_set_rx_gain\r\n");
    return VOS_OK;
}

int cli_check_frrx(int argc, char **argv)
{
    vos_print("todo: cli_check_frrx\r\n");
    return VOS_OK;
}

int cli_en_chan(int argc, char **argv)
{
    vos_print("todo: cli_en_chan\r\n");
    return VOS_OK;
}

int upcfg_cli_init()
{
    cli_cmd_reg("set_work_state",       "set rru work state",           &cli_set_work_state);
    cli_cmd_reg("dpdproc",              "dpd operation",                &cli_dpd_proc);
    cli_cmd_reg("set_tx_att",           "set tx attenuation",           &cli_set_tx_att);
    cli_cmd_reg("get_tx_att",           "get tx attenuation",           &cli_get_tx_att);
    cli_cmd_reg("set_rx_gain",          "set rx gain",                  &cli_set_rx_gain);
    cli_cmd_reg("check_frrx",           "get rx gain",                  &cli_check_frrx);
    cli_cmd_reg("en_chan",              "enable channel",               &cli_en_chan);

    return VOS_OK;
}

