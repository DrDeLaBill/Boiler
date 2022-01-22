#include "BoilerController.h"

#define DEBUG_ON    1

BoilerController *boiler_controller;

void setup(){
  Serial.begin(115200);
  Serial.println(F("\n######################################################"));
  Serial.println(F("Initialization started."));
  boiler_controller = new BoilerController();
}

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
}
