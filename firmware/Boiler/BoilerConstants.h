/*
 * Константы для всего проекта
 */


// Ошибки
#define ERRORS_COUNT            7   // Изменить, если количество поменяется

#define ERROR_NOERROR           0   // нет ошибок
#define ERROR_OVERHEAT          1   // перегрев (аварийный датчик), которого нет
#define ERROR_PUMPBROKEN        2   // насос неисправен
#define ERROR_SSRBROKEN         3   // твердотельные реле не выключаются
#define ERROR_TEMPSENSBROKEN    4   // датчик температуры теплоносителя не работает
#define ERROR_WATEROVERHEAT     5   // перегрев теплоносителя (обычный датчик)
#define ERROR_NOPOWER           6   // нет нагрева

//Рисунки на дисплее
const unsigned char pict_air[] = {
  0x0, 0x0, 0x0, 0x1e, 0x0, 0x33, 0x0, 0x61, 0x0, 0x40, 0xfe, 0x7f, 0x0, 0x0, 0xfe, 0x1f,
  0x0, 0x30, 0x3e, 0x60, 0x60, 0x40, 0x40, 0x60, 0x48, 0x32, 0x78, 0x1e, 0x0, 0x0, 0x0, 0x0
};

const unsigned char pict_water[] = {
  0x0, 0x0, 0x0, 0x0, 0xc, 0x43, 0x92, 0x24, 0x61, 0x18, 0xc, 0x43, 0x92, 0x24, 0x61, 0x18,
  0xc, 0x43, 0x92, 0x24, 0x61, 0x18, 0xc, 0x43, 0x92, 0x24, 0x61, 0x18, 0x0, 0x0, 0x0, 0x0
};

const unsigned char pict_therm[] = {
  0xc, 0x0, 0xec, 0x1, 0xc, 0x0, 0xec, 0x0, 0xc, 0x0, 0x6c, 0x0, 0xc, 0x0, 0x2c, 0x0,
  0xc, 0x0, 0xc, 0x0, 0xc, 0x0, 0x1e, 0x0, 0x21, 0x0, 0x21, 0x0, 0x33, 0x0, 0x1e, 0x0
};

const unsigned char wifi[] = {
  0xe0, 0x0, 0x18, 0x3, 0x4, 0x4, 0xf2, 0x9, 0x8, 0x2, 0xe4,
  0x4, 0x10, 0x1, 0x0, 0x0, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0
};

const unsigned char heat[] = {
  0xff, 0x1, 0xaa, 0x0, 0x0, 0x0, 0xaa, 0x0, 0xaa,
  0x0, 0xaa, 0x0, 0xaa, 0x0, 0xff, 0x1, 0xaa, 0x0
};

const unsigned char inet[] = {
  0x28, 0x68, 0xe8, 0x28, 0x28, 0x28, 0x2e, 0x2c, 0x28
};
