#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "kidbright32.h"
#include "HC_SR04.h"
#include "esp_log.h"

HC_SR04::HC_SR04(int bus_ch, int dev_addr) {
	channel = bus_ch;
	address = dev_addr;
	polling_ms = HC_SR04_POLLING_MS;
}

void i2c_setClock(uint32_t clock) {
	// Reset speed of I2C
	i2c_config_t conf;

	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = CHAIN_SDA_GPIO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = CHAIN_SCL_GPIO;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = clock;

	i2c_param_config(I2C_NUM_1, &conf);
}

void HC_SR04::init(void) {
	esp_log_level_set("*", ESP_LOG_VERBOSE);
	i2c_setClock(100E3);

	state = s_detect;
}

int HC_SR04::prop_count(void) {
	return 0;
}

bool HC_SR04::prop_name(int index, char *name) {
	// not supported
	return false;
}

bool HC_SR04::prop_unit(int index, char *unit) {
	// not supported
	return false;
}

bool HC_SR04::prop_attr(int index, char *attr) {
	// not supported
	return false;
}

bool HC_SR04::prop_read(int index, char *value) {
	// not supported
	return false;
}

bool HC_SR04::prop_write(int index, char *value) {
	// not supported
	return false;
}

void HC_SR04::process(Driver *drv) {
	I2CDev *i2c = (I2CDev *)drv;
	
	switch (state) {
		case s_detect:
			// detect i2c device
			if (i2c->detect(channel, address) == ESP_OK) {	
				// set initialized flag
				initialized = true;
					
				// clear error flag
				error = false;

				tickcnt = get_tickcnt() + 10000;
				state = s_request;
			} else {
				state = s_error;
			}
			break;

		case s_request: {
			if (!is_tickcnt_elapsed(tickcnt, polling_ms)) {
				break;
			}

			uint8_t data = 0x01;
			if (i2c->write(channel, address, &data, 1) == ESP_OK) {
				tickcnt = get_tickcnt();
				state = s_read;
			} else {
				state = s_error;
			}
		}

		case s_read: {
			if (!is_tickcnt_elapsed(tickcnt, 200)) {
				break;
			}
			tickcnt = get_tickcnt();

			uint8_t reg = 0xAF;
			uint8_t buffer[3];
			if (i2c->read(channel, address, &reg, 1, buffer, 3) == ESP_OK) {
				uint8_t BYTE_H = buffer[0];
				uint8_t BYTE_M = buffer[1];
				uint8_t BYTE_L = buffer[2];
				distance = (((uint32_t)BYTE_H << 16) + ((uint32_t)BYTE_M << 8) + (uint32_t)BYTE_L) / 1000.0;
				distance = distance / 10;
				
				tickcnt = get_tickcnt();
				state = s_request;
			} else {
				state = s_error;
			}
			
			break;
		}

		case s_error:
			// set error flag
			error = true;
			// clear initialized flag
			initialized = false;
			// get current tickcnt
			tickcnt = get_tickcnt();
			// goto wait and retry with detect state
			state = s_wait;
			break;

		case s_wait:
			// wait polling_ms timeout
			if (is_tickcnt_elapsed(tickcnt, polling_ms)) {
				state = s_detect;
			}
			break;
	}
}

float HC_SR04::get_distance() {
	return distance;
}
