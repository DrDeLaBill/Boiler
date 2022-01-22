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
    uint8_t user_error;
    
    void _remove_error();
    void _clear_errors();
  public:
    static Vector<uint8_t> errors_list;
    static bool is_set_error(uint8_t error_name);
    static void add_error(uint8_t new_error);
    static bool type_error_validate(uint8_t error_type);
    
    ErrorService();
    void check_failure();
    // Возвращает текущие ошибки
    void get_errors_list(uint8_t *result_errors_list);
    // Возвращает все существующие ошибки
    void get_all_errors();
    void enable_crash_out_pin();
    void init_error_actions();
};

#endif
