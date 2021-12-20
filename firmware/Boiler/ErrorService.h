/*
 * Сервис хранения и отслеживания ошибок бойлера
 */

#ifndef _ENCODER_H_
#define _ENCODER_H_

#include <Arduino.h>
#include <Vector.h>
#include "BoilerConstants.h"
#include "display.h"
#include "pump.h"

#define CRASH_OUT_PIN   33     // выход на расцепитель

#define SSR_DELAY       19    // ms

class ErrorService
{
  private:
    Vector<uint8_t> errors_list;
    uint8_t user_error;
    
    void _add_error(uint8_t new_error);
    void _remove_error();
    void _clear_errors();
  public:
    ErrorService();
    void check_failure();
    // Возвращает текущие ошибки
    void get_errors_list(uint8_t *result_errors_list);
    // Возвращает все существующие ошибки
    void get_all_errors();
    bool is_set_error(uint8_t error_name);
};

#endif
