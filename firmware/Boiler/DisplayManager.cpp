#include "DisplayManager.h"

DisplayPage DisplayManager::page_name = pageTemp;
uint8_t DisplayManager::brightness = 255;
const char* DisplayManager::presets[NUM_PRESETS] = {
  S_WEEKDAYS, 
  S_WEEKEND, 
  S_CUSTOM, 
  S_NOTFREEZE
};
uint8_t DisplayManager::menu_item = 0;              
uint32_t DisplayManager::t_page_save_settings = 0;        
uint32_t DisplayManager::t_newPage = 0;
uint8_t DisplayManager::temporary_target_temp = 0;
DisplayDataConfig DisplayManager::display_data_config;
U8G2_PCD8544_84X48_F_4W_HW_SPI DisplayManager::u8g2(U8G2_R0, /* cs=*/ 25, /* dc=*/ 26, /* reset=*/ U8X8_PIN_NONE);

DisplayManager::DisplayManager() {
  pinMode(LED_PIN, OUTPUT);
  DisplayManager::t_newPage = millis();
  
  if (DisplayManager::u8g2.begin()) {
    Serial.println(F("Display manager begin ok"));
  } else {
    Serial.println(F("Display manager begin error"));
  }
  DisplayManager::u8g2.enableUTF8Print();

  if (BoilerProfile::boiler_configuration.standby_flag == MODE_STANDBY) {
    DisplayManager::display_off();
  } else {
    DisplayManager::display_on();
  }
}

void DisplayManager::display_on() {
  // включаем дисплей и подсветку. Переходим на основную страницу.
  DisplayManager::page_name = pageTemp;
  DisplayManager::menu_item = 0;
  DisplayManager::u8g2.setPowerSave(false);
  digitalWrite(LED_PIN, HIGH);
  DisplayManager::t_newPage = millis();
  BoilerProfile::boiler_configuration.standby_flag = MODE_WORK;
  BoilerProfile::save_configuration();
  Serial.println(F("Display on"));
}

void DisplayManager::display_off() {
  // выключаем дисплей и подсветку
  DisplayManager::u8g2.setPowerSave(true);
  digitalWrite(LED_PIN, LOW);
  BoilerProfile::boiler_configuration.standby_flag = MODE_STANDBY;
  BoilerProfile::save_configuration();
  Serial.println(F("Display off"));
}

void DisplayManager::display_lightning() {
  // плавно выключаем подсветку при отсутствии активности
  if (DisplayManager::t_newPage != 0) {
    // если недавно была активность то ждем время до выключения
    if (millis() - DisplayManager::t_newPage >= TIMEOUT_LIGHTNING) {
      ledcWrite(0, brightness--);
      if (DisplayManager::brightness == 0) {
        ledcWrite(0, 0);
        DisplayManager::t_newPage = 0;
      }
    } else {
      // если время не подходит, то обновляем данные.
      DisplayManager::brightness = 255;
      ledcWrite(0, 255);
    }
  }
}

void DisplayManager::check_page() {
  // проверяем время неактивности. Если в течении 5с не было активности возвращаемся на начальный экран.
  // время экрана "сохранено" - 1с
  if (DisplayManager::page_name != pageTemp && DisplayManager::page_name != pageError) {
    if (DisplayManager::page_name == pageSaveSettings) {
      if (millis() - DisplayManager::t_newPage >= SAVE_TIMEOUT) {
        DisplayManager::set_temp_page();
        return;
      }
    } else if (millis() - DisplayManager::t_newPage >= CANCEL_TIMEOUT) {
      DisplayManager::set_temp_page();
    }
  } else if (DisplayManager::page_name == pageError && ErrorService::if_single_error(ERROR_RADIOSENSOR)) {
    if (millis() - DisplayManager::t_newPage >= CANCEL_TIMEOUT) {
      DisplayManager::set_temp_page();
    }
  }
}

void DisplayManager::set_temp_page() {
  DisplayManager::page_name = pageTemp;
  DisplayManager::menu_item = 0;
}

void DisplayManager::paint() {
  DisplayManager::check_page();
  // display_lightning();

  DisplayManager::u8g2.clearBuffer();

  switch (DisplayManager::page_name) {
    case pageTemp:
      // основной экран. Отображение текущей и уставной температуры.
      
      // если котел подключен к WiFi, то показываем значок вайфай
      if (DisplayManager::display_data_config.is_wifi_connect){
        DisplayManager::u8g2.drawXBM(0, 0, 12, 9, wifi);
      }

      // если у нас сейчас "работает" нагрев, то показываем это.
      if (DisplayManager::display_data_config.is_heating_on){
        DisplayManager::u8g2.drawXBM(14, 0, 9, 9, heat);
      }

      // если есть связь с сервером - то показываем это
      if (DisplayManager::display_data_config.is_connected_to_server){
        DisplayManager::u8g2.drawXBM(26, 0, 8, 9, inet);
      }

      
      // рисование текущего датчика температуры
      if (DisplayManager::display_data_config.is_external_sensor){
        // если выбран внешний датчик температуры или термопрофиль
        DisplayManager::u8g2.drawXBM(65, 20, 16, 16, pict_air);
      } else if (DisplayManager::display_data_config.is_internal_sensor){
        // иначе если выбран датчик температуры теплоносителя
        DisplayManager::u8g2.drawXBM(65, 20, 16, 16, pict_water);
      }

      // если есть радиосвязь с датчиком, то рисуем термометр
      if (DisplayManager::display_data_config.is_radio_connected){
        DisplayManager::u8g2.drawXBM(50, 20, 9, 16, pict_therm);
      }

      // текущая температура
      //u8g2.setFont(u8g2_font_inb30_mr);
      DisplayManager::u8g2.setFont(u8g2_font_inb30_mn);
      DisplayManager::u8g2.setCursor(0, 40);
      DisplayManager::u8g2.print(DisplayManager::display_data_config.current_temperature);

      // установленная температура
      DisplayManager::u8g2.setFont(u8g2_font_luBS12_tr);
      DisplayManager::u8g2.setCursor(43, 12);
      DisplayManager::u8g2.print(DisplayManager::display_data_config.target_temperature);
      DisplayManager::u8g2.setFont(u8g_font_5x8);
      DisplayManager::u8g2.setCursor(67, 5);
      DisplayManager::u8g2.print(F("o"));
      DisplayManager::u8g2.setFont(u8g2_font_ncenR12_tf);
      DisplayManager::u8g2.setCursor(71, 12);
      DisplayManager::u8g2.print(F("C"));

      // отрисовка текущей даты и времени
      DisplayManager::u8g2.setFont(u8g_font_5x8);
      DisplayManager::u8g2.setCursor(0, 48);
      DisplayManager::u8g2.print(DisplayManager::display_data_config.current_day); //clock_get_time("d/m/Y")

      DisplayManager::u8g2.setFont(u8g2_font_t0_12b_mr);
      DisplayManager::u8g2.drawStr(54, 48, DisplayManager::display_data_config.current_time); // clock_get_time("H:i")

      break;

    case pageTempSet:
      // страница настройки установленной температуры
      
      DisplayManager::u8g2.setFont(u8g2_font_5x8_t_cyrillic);
      DisplayManager::u8g2.setCursor(0, 6);
      DisplayManager::u8g2.print(F("Необходимая"));
      DisplayManager::u8g2.setCursor(0, 13);
      DisplayManager::u8g2.print(F("температура:"));

      DisplayManager::u8g2.setCursor(54, 30);
      DisplayManager::u8g2.print(F("o"));

      DisplayManager::u8g2.setFont(u8g2_font_luBS12_tr);
      DisplayManager::u8g2.setCursor(58, 38);
      DisplayManager::u8g2.print(F("C"));

      DisplayManager::u8g2.setFont(u8g2_font_luBS12_tr);
      DisplayManager::u8g2.setCursor(29, 38);
      DisplayManager::u8g2.print(DisplayManager::temporary_target_temp);
      
      break;

    case pageSaveSettings:
      // страница сохранения новой установленной температуры

      DisplayManager::u8g2.setFont(u8g2_font_5x8_t_cyrillic);
      DisplayManager::u8g2.setCursor(18, 22);
      DisplayManager::u8g2.print(F("Сохранено!"));  //Страницы на экране
      break;

    case pageResetSettings:
      // страница сохранения новой установленной температуры

      DisplayManager::u8g2.setFont(u8g2_font_5x8_t_cyrillic);
      DisplayManager::u8g2.setCursor(10, 8);
      DisplayManager::u8g2.print(F("Настройки"));
      DisplayManager::u8g2.setCursor(10, 20);
      DisplayManager::u8g2.print(F("возвращены к"));
      DisplayManager::u8g2.setCursor(10, 32);
      DisplayManager::u8g2.print(F("заводским"));
      DisplayManager::u8g2.setCursor(10, 44);
      DisplayManager::u8g2.print(F("установкам !"));

      break;

    case pageSettings:
//       страница настроек котла.
      DisplayManager::u8g2.setFont(u8g2_font_5x8_t_cyrillic);
      DisplayManager::u8g2.setCursor(5, 8);
      DisplayManager::u8g2.print(F("Режим работы"));
      DisplayManager::u8g2.setCursor(5, 20);
      DisplayManager::u8g2.print(F("Сброс настроек"));
      

      if (DisplayManager::menu_item == 0) {
        // без выбора
      } else if (DisplayManager::menu_item == 1) {
        DisplayManager::u8g2.drawRFrame(0,  1, 78, 10, 4);
      } else if (DisplayManager::menu_item == 2) {
        DisplayManager::u8g2.drawRFrame(0, 12, 78, 10, 4);
      }
      break;

    case pageSetMode:
      // страница выбора режима работы по уставке теплоносителя, воздуха или по термопрофилю

      DisplayManager::u8g2.setFont(u8g2_font_5x8_t_cyrillic);
      DisplayManager::u8g2.setCursor(5, 8);
      DisplayManager::u8g2.print(F("термопрофиль"));
      DisplayManager::u8g2.setCursor(5, 20);
      DisplayManager::u8g2.print(F("темп. вода"));
      DisplayManager::u8g2.setCursor(5, 32);
      DisplayManager::u8g2.print(F("темп. воздух"));

      if (DisplayManager::menu_item == 1) {
        DisplayManager::u8g2.drawRFrame(0, 1, 78, 10, 4);
      } else if (DisplayManager::menu_item == 2) {
        DisplayManager::u8g2.drawRFrame(0, 12, 78, 10, 4);
      } else if (DisplayManager::menu_item == 3){
        DisplayManager::u8g2.drawRFrame(0, 23, 78, 10, 4);
      }
      break;

    case pageError:
      // страница отображения критических ошибок

      DisplayManager::u8g2.setFont(u8g2_font_6x12_t_cyrillic);
      DisplayManager::u8g2.setCursor(18, 12);
      DisplayManager::u8g2.print(F("АВАРИЯ!"));

      //TODO: ещё комменты
      if (DisplayManager::display_data_config.is_overheat) { // user_error == OVERHEAT || user_error == WATEROVERHEAT
        // перегрев
        DisplayManager::u8g2.setCursor(10, 32);
        DisplayManager::u8g2.print(F("Перегрев!"));
      } else if (DisplayManager::display_data_config.is_pumpbroken) { // user_error == PUMPBROKEN
        // датчик протока сигнализирует об отсутствии потока
        DisplayManager::u8g2.setCursor(10, 32);
        DisplayManager::u8g2.print(F("Нет"));
        DisplayManager::u8g2.setCursor(10, 44);
        DisplayManager::u8g2.print(F("протока!"));
      } else if (DisplayManager::display_data_config.is_ssrbroken) { // user_error == SSRBROKEN
        // твердотельные реле неисправны
        DisplayManager::u8g2.setCursor(10, 32);
        DisplayManager::u8g2.print(F("Перегрев"));
        DisplayManager::u8g2.setCursor(10, 44);
        DisplayManager::u8g2.print(F("ТТ реле!"));
      } else if (DisplayManager::display_data_config.is_tempsensbroken) { // user_error == TEMPSENSBROKEN
        // датчик температуры теплоносителя неисправен
        DisplayManager::u8g2.setCursor(10, 32);
        DisplayManager::u8g2.print(F("Датчик темп."));
        DisplayManager::u8g2.setCursor(10, 44);
        DisplayManager::u8g2.print(F("неисправен!"));
      } else if (DisplayManager::display_data_config.is_nopower){ // user_error == NOPOWER
        // нет напряжения после ТТР
        DisplayManager::u8g2.setCursor(10, 32);
        DisplayManager::u8g2.print(F("Нет"));
        DisplayManager::u8g2.setCursor(10, 44);
        DisplayManager::u8g2.print(F("нагрева!"));
      }
      break;

    default:
      break;
  }

  DisplayManager::u8g2.sendBuffer();
}

void DisplayManager::rotary_encoder_action(uint8_t rotary_state) {
  if (rotary_state == BUTTON_ROTARY_RIGHT) {
    DisplayManager::rotary_right();
  } else if (rotary_state == BUTTON_ROTARY_LEFT) {
    DisplayManager::rotary_left();
  }
}

void DisplayManager::rotary_right() {
  // обработаем вращение энкодера вправо
  DisplayManager::t_newPage = millis();

  uint8_t session_boiler_mode = BoilerProfile::get_session_boiler_mode();

  switch (DisplayManager::page_name) {
    case pageTemp:
      // страница с настройками
      DisplayManager::page_name = pageSettings;
      break;

    case pageTempSet:
      // страница настройки устанавливаемой температуры

      if (session_boiler_mode == MODE_WATER){
        if (DisplayManager::temporary_target_temp < WATER_TEMP_MAX) {
          DisplayManager::temporary_target_temp++;
        }
      } else {
        if (DisplayManager::temporary_target_temp < AIR_TEMP_MAX) {
          DisplayManager::temporary_target_temp++;
        }
      }
      break;

    case pageSettings:
      // страница настроек

      if (DisplayManager::menu_item != 0 && DisplayManager::menu_item < 3) {
        DisplayManager::menu_item++;
      }
      break;

    case pageSetMode:
      // страница выбора режима работы

      if (DisplayManager::menu_item < 3) {
        DisplayManager::menu_item++;
      }
      break;

    default:
      break;
  }
}

void DisplayManager::rotary_left() {
  // обработаем вращение энкодера влево
  DisplayManager::t_newPage = millis();

  uint8_t session_boiler_mode = BoilerProfile::get_session_boiler_mode();

  switch (DisplayManager::page_name) {
    case pageTempSet:
      // страница настройки устанавливаемой температуры

      if (session_boiler_mode == MODE_WATER){
        if (DisplayManager::temporary_target_temp > WATER_TEMP_MIN) {
          DisplayManager::temporary_target_temp--;
        }
      } else {
        if (DisplayManager::temporary_target_temp > AIR_TEMP_MIN) {
          DisplayManager::temporary_target_temp--;
        }
      }
      break;

    case pageSettings:
      // страница настроек

      if (DisplayManager::menu_item == 0) {
        DisplayManager::page_name = pageTemp;
      } else {
        if (DisplayManager::menu_item > 1) {
          DisplayManager::menu_item--;
        }
      }
      break;

    case pageSetMode:
      // страница выбора режима работы

      if (DisplayManager::menu_item > 1) {
        DisplayManager::menu_item--;
      }
      break;

    default:
      break;
  }
}

void DisplayManager::fill_display_configuration() {
  DisplayManager::display_data_config.is_wifi_connect        = NetworkManager::is_wifi_connected();
  DisplayManager::display_data_config.is_heating_on          = RelayTemperature::is_heating_on();
  DisplayManager::display_data_config.is_connected_to_server = ExternalServer::get_connected_to_server();
  DisplayManager::display_data_config.is_external_sensor     = (BoilerProfile::is_set_session_boiler_mode(MODE_AIR) || BoilerProfile::is_set_session_boiler_mode(MODE_PROFILE));
  DisplayManager::display_data_config.is_internal_sensor     = BoilerProfile::is_set_session_boiler_mode(MODE_WATER);
  DisplayManager::display_data_config.is_radio_connected     = RadioSensor::is_sensor_online() || BoilerProfile::is_set_session_boiler_mode(MODE_WATER) && !ErrorService::is_set_error(ERROR_TEMPSENSBROKEN);
  DisplayManager::display_data_config.is_overheat            = ErrorService::is_set_error(ERROR_OVERHEAT) || ErrorService::is_set_error(ERROR_WATEROVERHEAT);
  DisplayManager::display_data_config.is_pumpbroken          = ErrorService::is_set_error(ERROR_PUMPBROKEN);
  DisplayManager::display_data_config.is_ssrbroken           = ErrorService::is_set_error(ERROR_SSRBROKEN);
  DisplayManager::display_data_config.is_tempsensbroken      = ErrorService::is_set_error(ERROR_TEMPSENSBROKEN) || ErrorService::is_set_error(ERROR_RADIOSENSOR);
  DisplayManager::display_data_config.is_nopower             = ErrorService::is_set_error(ERROR_NOPOWER);
  DisplayManager::display_data_config.current_temperature    = TemperatureSensor::get_current_temperature();
  DisplayManager::display_data_config.target_temperature     = BoilerProfile::get_target_temp();
  strncpy(DisplayManager::display_data_config.current_day, BoilerProfile::get_current_time("d/m/Y"), DISPLAY_CONF_STR_LENGTH);
  strncpy(DisplayManager::display_data_config.current_time, BoilerProfile::get_current_time("H:i"), DISPLAY_CONF_STR_LENGTH);
}

void DisplayManager::set_t_newPage(int value) {
  DisplayManager::t_newPage = value;
}

DisplayPage DisplayManager::get_page_name() {
  return DisplayManager::page_name;
}

// TODO: проверка на существование
void DisplayManager::set_page_name(DisplayPage page_name) {
  DisplayManager::page_name = page_name;
}

uint8_t DisplayManager::get_menu_item() {
  return DisplayManager::menu_item;
}

// TODO: проверка на существование
void DisplayManager::set_menu_item(uint8_t menu_item) {
  DisplayManager::menu_item = menu_item;
}

void DisplayManager::set_temporary_target_temp(uint8_t temporary_target_temp) {
  DisplayManager::temporary_target_temp = temporary_target_temp;
}

uint8_t DisplayManager::get_temporary_target_temp() {
  return DisplayManager::temporary_target_temp;
}
