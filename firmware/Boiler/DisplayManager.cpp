#include "DisplayManager.h"
#include "BoilerConstants.h"

extern BoilerConfig BoilerCfg; //######################################################################

extern uint8_t current_temp;            // текущая температура для отображения и ПИД
extern uint32_t t_newPage;
extern uint8_t user_boiler_mode;
extern uint8_t user_error;
extern bool connected_to_server;
extern uint8_t radio_connected;

DisplayManager::DisplayManager() {
  this->redraw_display = false;            
  this->menu_item = 0;              
  this->t_page_save_settings = 0;        
  this->t_newPage = 0;                 
  this->last_time_redraw = 0;    
  this->display_init();
}

void DisplayManager::display_init() {
  this->u8g2.begin();
  this->u8g2.setContrast(70);
  this->u8g2.enableUTF8Print();
  this->u8g2.setFontDirection(0);
  this->page = pageTemp;
  this->redraw_display = true;
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  t_newPage = millis();                         //TODO: откуда?
}

void DisplayManager::display_on() {
  // включаем дисплей и подсветку. Переходим на основную страницу.
  this->page = pageTemp;
  this->menu_item = 0;
  this->redraw_display = true;
  this->u8g2.setPowerSave(false);
  digitalWrite(LED_PIN, HIGH);
  t_newPage = millis();                         //TODO: откуда?
}

void DisplayManager::display_off() {
  // выключаем дисплей и подсветку
  this->u8g2.setPowerSave(true);
  digitalWrite(LED_PIN, LOW);
}

void DisplayManager::display_lightning() {
  // плавно выключаем подсветку при отсутствии активности
  static uint8_t brightness = 255;
  if (t_newPage != 0) {                         
    //TODO: вся функция из extern
    
    // если недавно была активность то ждем время до выключения
    if (millis() - t_newPage >= TIMEOUT_LIGHTNING) {                         //TODO: откуда?
      ledcWrite(0, brightness--);
      if (brightness == 0) {
        ledcWrite(0, 0);
        t_newPage = 0;
      }
    } else {
      // если время не подходит, то обновляем данные.
      brightness = 255;
      ledcWrite(0, 255);
    }
  }
}

void DisplayManager::check_page() {
  // проверяем время неактивности. Если в течении 5с не было активности возвращаемся на начальный экран.
  // время экрана "сохранено" - 1с

  if (this->page != pageTemp && this->page != pageError) {

    if (this->page == this->t_page_save_settings || this->page == this->t_page_save_settings) {
      if (millis() - this->t_page_save_settings >= SAVE_TIMEOUT) {
        this->page = pageTemp; //TODO: повторяющийся код
        this->menu_item = 0;
        this->redraw_display = true;
        return;
      }
    } else if (millis() - t_newPage >= CANCEL_TIMEOUT) { //TODO: t_newPage
      this->page = pageTemp;
      this->menu_item = 0;
      this->redraw_display = true;
    }
  }
}

void DisplayManager::paint() {
  // отрисовка дисплея

  if (millis() - this->last_time_redraw >= REDRAW_TIMEOUT) {
    this->last_time_redraw = millis();
    this->redraw_display = true;
  }

  check_page(); //TODO: check_page()
  //display_lightning();

  if (this->redraw_display){   // если пора, то...
    this->last_time_redraw = millis();
    this->u8g2.clearBuffer();

    switch (this->page) { //TODO: остановился тут
      case this->page_name.pageTemp:
        // основной экран. Отображение текущей и уставной температуры.

        // если котел подключен к WiFi, то показываем значок вайфай
        if (this->display_data_config.is_wifi_connect){
          this->u8g2.drawXBM(0, 0, 12, 9, wifi);
        }

        //TODO: старый код в комментах
        // если у нас сейчас "работает" нагрев, то показываем это.
        if (this->display_data_config.is_heating_on){ //digitalRead(SSR1_OUT_PIN) вот он)
          this->u8g2.drawXBM(14, 0, 9, 9, heat);
        }

        // если есть связь с сервером - то показываем это
        if (this->display_data_config.is_connected_to_server){
          this->u8g2.drawXBM(26, 0, 8, 9, inet);
        }

        
        // рисование текущего датчика температуры
        if (this->display_data_config.is_external_sensor){ //user_boiler_mode == MODE_AIR || user_boiler_mode == MODE_PROFILE
          // если выбран внешний датчик температуры или термопрофиль
          this->u8g2.drawXBM(65, 20, 16, 16, pict_air);
        } else if (this->display_data_config.is_internal_sensor){ //user_boiler_mode == MODE_WATER
          // иначе если выбран датчик температуры теплоносителя
          this->u8g2.drawXBM(65, 20, 16, 16, pict_water);
        }

        // если есть радиосвязь с датчиком, то рисуем термометр
        if (this->display_data_config.is_radio_connected){ //radio_connected == RADIO_ON
          this->u8g2.drawXBM(50, 20, 9, 16, pict_therm); //TODO: pict_therm
        }

        // текущая температура
        //u8g2.setFont(u8g2_font_inb30_mr);
        this->u8g2.setFont(u8g2_font_inb30_mn);
        this->u8g2.setCursor(0, 40);
        this->u8g2.print(this->display_data_config.current_temperature);

        // установленная температура
        this->u8g2.setFont(u8g2_font_luBS12_tr);
        this->u8g2.setCursor(43, 12);
        this->u8g2.print(this->display_data_config.target_temperature); // get_target_temp()
        this->u8g2.setFont(u8g_font_5x8);
        this->u8g2.setCursor(67, 5);
        this->u8g2.print("o");
        this->u8g2.setFont(u8g2_font_ncenR12_tf);
        this->u8g2.setCursor(71, 12);
        this->u8g2.print("C");

        // отрисовка текущей даты и времени
        this->u8g2.setFont(u8g_font_5x8);
        this->u8g2.setCursor(0, 48);
        this->u8g2.print(this->display_data_config.current_day); //clock_get_time("d/m/Y")

        this->u8g2.setFont(u8g2_font_t0_12b_mr);
        this->u8g2.drawStr(54, 48, this->display_data_config.current_time); // clock_get_time("H:i")

        break;

      case this->page_name.pageTempSet:
        // страница настройки установленной температуры

        this->u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        this->u8g2.setCursor(0, 6);
        this->u8g2.print("Требуемая");
        this->u8g2.setCursor(0, 13);
        this->u8g2.print("температура:");

        this->u8g2.setCursor(54, 30);
        this->u8g2.print("o");

        this->u8g2.setFont(u8g2_font_luBS12_tr);
        this->u8g2.setCursor(58, 38);
        this->u8g2.print("C");

        this->u8g2.setFont(u8g2_font_luBS12_tr);
        this->u8g2.setCursor(29, 38);
        this->u8g2.print(this->temporary_target_temp);
        break;

      case this->page_name.pageSaveSettings:
        // страница сохранения новой установленной температуры

        this->u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        this->u8g2.setCursor(18, 22);
        this->u8g2.print("Сохранено!");  //Страницы на экране
        break;

      case this->page_name.pageResetSettings:
        // страница сохранения новой установленной температуры

        this->u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        this->u8g2.setCursor(10, 8);
        this->u8g2.print("Настройки");
        this->u8g2.setCursor(10, 20);
        this->u8g2.print("возвращены к");
        this->u8g2.setCursor(10, 32);
        this->u8g2.print("заводским");
        this->u8g2.setCursor(10, 44);
        this->u8g2.print("установкам !");

        break;

      case this->page_name.pageSettings:
        // страница настроек котла.
        
        this->u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        this->u8g2.setCursor(5, 8);
        this->u8g2.print("Режим работы");
        this->u8g2.setCursor(5, 20);
        this->u8g2.print("Сброс настроек");
        

        if (this->menu_item == 0) {
          // без выбора
        } else if (this->menu_item == 1) {
          this->u8g2.drawRFrame(0,  1, 78, 10, 4);
        } else if (this->menu_item == 2) {
          this->u8g2.drawRFrame(0, 12, 78, 10, 4);
        }
        break;

      case this->page_name.pageSetMode:
        // страница выбора режима работы по уставке теплоносителя, воздуха или по термопрофилю

        this->u8g2.setFont(u8g2_font_5x8_t_cyrillic);
        this->u8g2.setCursor(5, 8);
        this->u8g2.print("термопрофиль");
        this->u8g2.setCursor(5, 20);
        this->u8g2.print("темп. вода");
        this->u8g2.setCursor(5, 32);
        this->u8g2.print("темп. воздух");

        if (this->menu_item == 1) {
          this->u8g2.drawRFrame(0, 1, 78, 10, 4);
        } else if (this->menu_item == 2) {
          this->u8g2.drawRFrame(0, 12, 78, 10, 4);
        } else if (this->menu_item == 3){
          this->u8g2.drawRFrame(0, 23, 78, 10, 4);
        }
        break;

      case this->page_name.pageError:
        // страница отображения критических ошибок

        this->u8g2.setFont(u8g2_font_6x12_t_cyrillic);
        this->u8g2.setCursor(18, 12);
        this->u8g2.print("АВАРИЯ!");

        //TODO: ещё комменты
        if (this->display_data_config.is_overheat) { // user_error == OVERHEAT || user_error == WATEROVERHEAT
          // перегрев
          this->u8g2.setCursor(10, 32);
          this->u8g2.print("Перегрев!");
        } else if (this->display_data_config.is_pumpbroken) { // user_error == PUMPBROKEN
          // датчик протока сигнализирует об отсутствии потока
          this->u8g2.setCursor(10, 32);
          this->u8g2.print("Нет");
          this->u8g2.setCursor(10, 44);
          this->u8g2.print("протока!");
        } else if (this->display_data_config.is_ssrbroken) { // user_error == SSRBROKEN
          // твердотельные реле неисправны
          this->u8g2.setCursor(10, 32);
          this->u8g2.print("Перегрев");
          this->u8g2.setCursor(10, 44);
          this->u8g2.print("ТТ реле!");
        } else if (this->display_data_config.is_tempsensbroken) { // user_error == TEMPSENSBROKEN
          // датчик температуры теплоносителя неисправен
          this->u8g2.setCursor(10, 32);
          this->u8g2.print("Датчик темп.");
          this->u8g2.setCursor(10, 44);
          this->u8g2.print("неисправен!");
        } else if (this->display_data_config.is_nopower){ // user_error == NOPOWER
          // нет напряжения после ТТР
          this->u8g2.setCursor(10, 32);
          this->u8g2.print("Нет");
          this->u8g2.setCursor(10, 44);
          this->u8g2.print("нагрева!");
        }
        break;

      default:
        break;
    }

    this->u8g2.sendBuffer();
    this->redraw_display = false;
  }
}

// Какая красота
void DisplayManager::set_display_data_config(DisplayDataConfig *display_data_config) {
  this->display_data_config.is_wifi_connect = display_data_config.is_wifi_connect;
  this->display_data_config.is_heating_on = display_data_config.is_heating_on;
  this->display_data_config.is_connected_to_server = display_data_config.is_connected_to_server;
  this->display_data_config.is_external_sensor = display_data_config.is_external_sensor;
  this->display_data_config.is_internal_sensor = display_data_config.is_internal_sensor;
  this->display_data_config.is_radio_connected = display_data_config.is_radio_connected;
  this->display_data_config.is_overheat = display_data_config.is_overheat;
  this->display_data_config.is_pumpbroken = display_data_config.is_pumpbroken;
  this->display_data_config.is_ssrbroken = display_data_config.is_ssrbroken;
  this->display_data_config.is_tempsensbroken = display_data_config.is_tempsensbroken;
  this->display_data_config.is_nopower = display_data_config.is_nopower;
  this->display_data_config.current_temperature = display_data_config.current_temperature;
  this->display_data_config.target_temperature = display_data_config.target_temperature;
  strncpy(this->display_data_config.current_day, display_data_config.current_day, sizeof(display_data_config.current_day));
  strncpy(this->display_data_config.current_time, display_data_config.current_time, sizeof(display_data_config.current_time));
}
