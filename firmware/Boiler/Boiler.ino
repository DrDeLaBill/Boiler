#include "BoilerController.h"

#define DEBUG_ON    1

BoilerController *boiler_controller;

void setup(){
  Serial.begin(115200);
  Serial.println(F("\n######################################################"));
  Serial.println(F("Initialization started."));
  boiler_controller = new BoilerController();
  
//  // Инициализируем файловую систему SPIFFS:
//  if(!SPIFFS.begin(true)){
//    Serial.println(F("An Error has occurred while mounting SPIFFS"));
//               //  "При монтировании SPIFFS произошла ошибка"
//    ESP.restart();
//  }
//  
//  display_init();
//  Serial.println("display init");
////  paint();    // нарисуем экран
//
////  init_diagnostics();
////  Serial.println("diagnostics system init");
//
//  cfg_init();
//  Serial.println(F("cfg init"));
//  network_init();
//  Serial.println(F("network init"));
//  encoder_init();
//  Serial.println(F("encoder init"));
//  clock_init();
//  Serial.println(F("clock init"));
//  ssr_temp_init();
//  Serial.println(F("ssr temp init"));
//  temp_init();
//  Serial.println(F("temp init"));
//  pid_init();
//  Serial.println(F("PID init"));
//  pump_init();
//  Serial.println(F("pump init"));
//
//
//  // проверяем, надо ли включаться или нет.
//  if (BoilerCfg.standby_flag == WORK) {
//    p_mode = MODE_WORK;
//    Serial.println(F("WORK MODE"));
//    pump_on();
//  } else {
//    p_mode = MODE_STANDBY;
//    Serial.println(F("STANDBY MODE"));
//    display_off();
//  }

}
//------------------------------------------------------------------------------------------

void loop(){
  /*
   * Загрузка значений по термопрофилю. Получение текущего времени.
   * Измерение температуры. Теплоноситель и внешний датчик.
   * Измерение температуры ТТ реле и включение вентилятора.
   * Общение с сервером. Надо поговорить об этом с Ромой.
   * Проверка исправности системы. Отображение ошибок.
   * Опрос энкодера и кнопки.
   * Рисование на дисплее.
   * 
   */

  // Проверка наличия команд через Serial порт
  //command_manager.check_commands();
  
  // всегда проверяем исправность ТТР
  //check_ssr_failure(); //сейчас функция пуста

  // проверка обновлений
  //server.handleClient();

//  if (p_mode == MODE_WORK){
//    // режим работы
//
//    // проверим аварийные ситуации
//    check_failure();
//
//    // проверим нагрев
//    if (user_error == NOERROR){
//      pid_regulating();
//    } else {
//      pid_off();
//    }
//    
//    // нарисуем экран
//    paint();
//  
//    // измерим температуру
//    check_temp();
//  
//    // проверим температуру ТТ реле.
//    // check_ssr_temp();
//  
//    // проверим энкодер
//    if (check_encoder(p_mode) == BUTTON_HOLDED){
//      // если было долгое нажатие кнопки - переходим в режим ожидания
//      Serial.println(F("STANDBY MODE"));
//      display_off();
//      set_settings_standby(STANDBY);
//      p_mode = MODE_STANDBY;
//    }
//
//    check_new_settings();
//  }
//
//  if (p_mode == MODE_STANDBY){
//    // режим ожидания
//    if (check_encoder(p_mode) == BUTTON_HOLDED){
//      // если было долгое нажатие кнопки - переходим в режим работы
//      Serial.println(F("WORK MODE"));
//      pump_on();
//      display_on();
//      set_settings_standby(WORK);
//      p_mode = MODE_WORK;
//      return;
//    }
//    
//    pump_off();
//  }
// 
}
