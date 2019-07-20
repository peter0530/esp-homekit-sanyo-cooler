#include <FreeRTOS.h>
#include <esp8266.h>
#include <stdio.h>

#include "homekit_callback.h"
#include "homekit_config.h"

ac_state_t AC = {.active = false,
                 .rotationSpeed = 33,
                 .swing = false,
                 .targetTemperature = DEFAULT_COOLER_TEMPERATURE,
                 .lastTargetTempChange = 0};
fan_state_t FAN = {.active = false, .rotationSpeed = 33};

bool homekit_initialized = false;

homekit_characteristic_t current_humidity =
  HOMEKIT_CHARACTERISTIC_(CURRENT_RELATIVE_HUMIDITY, 0);
homekit_characteristic_t current_temperature =
  HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0);
homekit_characteristic_t target_temperature = HOMEKIT_CHARACTERISTIC_(
  COOLING_THRESHOLD_TEMPERATURE, DEFAULT_COOLER_TEMPERATURE,
  .min_value = (float[]){MIN_COOLER_TEMPERATURE},
  .max_value = (float[]){MAX_COOLER_TEMPERATURE}, .min_step = (float[]){1},
  .getter = target_temperature_get, .setter = target_temperature_set);
homekit_characteristic_t units =
  HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS, 0);
homekit_characteristic_t current_heater_cooler_state =
  HOMEKIT_CHARACTERISTIC_(CURRENT_HEATER_COOLER_STATE, 0);
/* INACTIVE = 0; IDLE = 1; HEATING = 2; COOLING = 3; */

homekit_characteristic_t target_heater_cooler_state =
  HOMEKIT_CHARACTERISTIC_(TARGET_HEATER_COOLER_STATE, 2,
                          .valid_values = /* AUTO = 0; HEAT = 1; COOL = 2; */
                          {
                            .count = 1,
                            .values = (uint8_t[]){2},
                          });

homekit_characteristic_t ac_active = HOMEKIT_CHARACTERISTIC_(
  ACTIVE, 0, .getter = ac_active_get, .setter = ac_active_set);
homekit_characteristic_t ac_rotation_speed =
  HOMEKIT_CHARACTERISTIC_(ROTATION_SPEED, 0);
homekit_characteristic_t ac_swing_mode = HOMEKIT_CHARACTERISTIC_(SWING_MODE, 0);

homekit_characteristic_t fan_active = HOMEKIT_CHARACTERISTIC_(
  ACTIVE, 0, .getter = fan_active_get, .setter = fan_active_set);
homekit_characteristic_t fan_rotation_speed =
  HOMEKIT_CHARACTERISTIC_(ROTATION_SPEED, 0);
homekit_characteristic_t fan_swing_mode =
  HOMEKIT_CHARACTERISTIC_(SWING_MODE, 0);

homekit_accessory_t *homekit_accessories[] = {
  HOMEKIT_ACCESSORY(
      .id = 1, .category = homekit_accessory_category_air_conditioner,
      .services =
        (homekit_service_t *[]){
          HOMEKIT_SERVICE(
            ACCESSORY_INFORMATION,
            .characteristics =
              (homekit_characteristic_t *[]){
                HOMEKIT_CHARACTERISTIC(NAME, "冷氣"),
                HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Swift"),
                HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "20190721"),
                HOMEKIT_CHARACTERISTIC(MODEL, "SWF-08C"),
                HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.0.11"),
                HOMEKIT_CHARACTERISTIC(IDENTIFY, ac_identify), NULL}),
          HOMEKIT_SERVICE(HEATER_COOLER, .primary = true,
                          .characteristics =
                            (homekit_characteristic_t *[]){
                              HOMEKIT_CHARACTERISTIC(NAME, "冷氣"), &ac_active,
                              &current_temperature, &target_temperature,
                              &current_heater_cooler_state,
                              &target_heater_cooler_state, &units,
                              &ac_rotation_speed, &ac_swing_mode, NULL}),
          HOMEKIT_SERVICE(
            HUMIDITY_SENSOR,
            .characteristics =
              (homekit_characteristic_t *[]){
                HOMEKIT_CHARACTERISTIC(NAME, "濕度"), &current_humidity,
                HOMEKIT_CHARACTERISTIC(ACTIVE, 1), NULL}),
          NULL}),
  HOMEKIT_ACCESSORY(
      .id = 2, .category = homekit_accessory_category_fan,
      .services =
        (homekit_service_t *[]){
          HOMEKIT_SERVICE(
            ACCESSORY_INFORMATION,
            .characteristics =
              (homekit_characteristic_t *[]){
                HOMEKIT_CHARACTERISTIC(NAME, "電風扇"),
                HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Y.S. TECH"),
                HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "20190721"),
                HOMEKIT_CHARACTERISTIC(MODEL, "YS-9166SFR"),
                HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.0.1"),
                HOMEKIT_CHARACTERISTIC(IDENTIFY, ac_identify), NULL}),
          HOMEKIT_SERVICE(FAN2, .primary = true,
                          .characteristics =
                            (homekit_characteristic_t *[]){
                              HOMEKIT_CHARACTERISTIC(NAME, "電風扇"),
                              &fan_active, &fan_rotation_speed, &fan_swing_mode,
                              NULL}),
          NULL}),
  NULL};
;
homekit_server_config_t homekit_config = {.accessories = homekit_accessories,
                                          .password = HOMEKIT_PASSWORD};