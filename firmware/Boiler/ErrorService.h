/*
 * Сервис хранения и отслеживания ошибок бойлера
 */

#ifndef _ERROR_SERVICE_H_
#define _ERROR_SERVICE_H_

#include <Arduino.h>
#include <Vector.h>

#include "BoilerConstants.h"
#include "PumpManager.h"

// выход на расцепитель
#define CRASH_OUT_PIN   33
// ms
#define SSR_DELAY       19

class ErrorService
{
  public:
    static Vector<uint8_t> errors_list;
    
    ErrorService();
    static bool is_set_error(uint8_t error_name);
    static void add_error(uint8_t new_error);
    static bool type_error_validate(uint8_t error_type);
    static void clear_errors();
    static void check_failure();
    // Возвращает текущие ошибки
    static void get_errors_list(uint8_t *result_errors_list);
    // Возвращает все существующие ошибки
    static void get_all_errors();
    static void enable_crash_out_pin();
    static void init_error_actions();
    static void disable_crash_out_pin();
};

#endif
