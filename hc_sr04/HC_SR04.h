#ifndef __HC_SR04_H__
#define __HC_SR04_H__

#include "math.h"
#include "driver.h"
#include "device.h"
#include "i2c-dev.h"

#define HC_SR04_POLLING_MS 200

class HC_SR04 : public Device {
	private:
		enum {
			s_detect, 
			s_request,
			s_read, 
			s_error, 
			s_wait
		} state;
		TickType_t tickcnt, polling_tickcnt;
		
		uint32_t distance = 0;
		
	public:
		// constructor
		HC_SR04(int bus_ch, int dev_addr);
		// override
		void init(void);
		void process(Driver *drv);
		int prop_count(void);
		bool prop_name(int index, char *name);
		bool prop_unit(int index, char *unit);
		bool prop_attr(int index, char *attr);
		bool prop_read(int index, char *value);
		bool prop_write(int index, char *value);
		
		// method
		float get_distance() ;
		
};

#endif
