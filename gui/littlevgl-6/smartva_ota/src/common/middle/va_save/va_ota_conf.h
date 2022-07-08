void va_ota_param_init(char *ota_config_path);
void va_ota_param_uninit(void);
int va_ota_read_string_type_param(char *major, char *minor, char *str, int len);
int va_ota_read_int_type_param(char *major, char *minor, int *param);
