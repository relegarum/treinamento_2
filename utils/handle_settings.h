/* \file handle_settings.h
 *
 * \brief Contem a implementacao de funcoes de validação de settings
 * ativas.
 *
 * "$Id: $"
*/
#ifndef HANDLE_SETTINGS_H
#define HANDLE_SETTINGS_H
#include <stdint.h>

int32_t handle_arguments(int argc,
                         char **argv,
                         char *port,
                         char *path,
                         int32_t* transmission_rate);

int32_t verify_if_valid_port(const char *port);
int32_t verify_if_valid_path(const char *path);

int32_t get_real_path(const char *path, char *real_path);

int32_t treat_transmission_rate(const char *transmission_rate_string, int32_t *transmission_rate);

#endif // HANDLE_SETTINGS_H
