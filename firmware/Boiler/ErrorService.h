/*
 * Сервис хранения и отслеживания ошибок бойлера
 */

#ifndef _ERROR_SERVICE_H_
#define _ERROR_SERVICE_H_

#include <Arduino.h>
#include <Vector.h>
#include "BoilerConstants.h"
//TODO: include
//#include "display.h"
//#include "pump.h"

#define CRASH_OUT_PIN   33     // выход на расцепитель

#define SSR_DELAY       19    // ms

class ErrorService
{
  private:
    Vector<uint8_t> errors_list;
    uint8_t user_error;
    
    void _remove_error();
    void _clear_errors();
    bool _type_error_validate(uint8_t error_type);
  public:
    ErrorService();
    void check_failure();
    // Возвращает текущие ошибки
    void get_errors_list(uint8_t *result_errors_list);
    // Возвращает все существующие ошибки
    void get_all_errors();
    bool is_set_error(uint8_t error_name);
    void add_error(uint8_t new_error);
    void enable_crash_out_pin();
    void init_error_actions();
};

#endif
