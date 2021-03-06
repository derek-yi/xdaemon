
#ifndef _DEVM_COMMON_H_
#define _DEVM_COMMON_H_

int devm_import_fru_info(void);

int daemon_store_script(char *file_name);

int cli_show_fru_info(int argc, char **argv);
int cli_fru_set_mac(int argc, char **argv);
int cli_fru_set_uuid(int argc, char **argv);
int cli_fru_set_skuid(int argc, char **argv);
int cli_fru_set_rf_calibration(int argc, char **argv);
int cli_fru_load_json(int argc, char **argv);
int cli_fru_set_sn(int argc, char **argv);

int devm_init(char *file_name);

int devm_exit();

#endif
