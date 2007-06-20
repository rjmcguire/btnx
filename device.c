
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */
  

#include "device.h"

/* 
 * The Vendor and Product IDs for supported mice 
 * as included in /proc/bus/input/devices
 */
const device devices[] =
{ //	Name			VendorID	ProductID
	{	"mxrevo",		0x046D,		0xC51A	},	// Logitech MX Revolution
	{	"vxrevo",		0x046D,		0xC518	},	// Logitech VX Revolution
	{	"vxrevo",		0x046D,		0xC521	},	// Logitech VX Revolution *another version*
	{	"g5",			0x046D,		0xC041	},	// Logitech G5
	{	"mx510",		0x046D,		0xC01D	},	// Logitech MX-510
	{	"mx600",		0x046D,		0xC50E	},	// Logitech MX-600 && MX-1000 && Mediaplay (problem)
	{	"msime3",		0x0002,		0x0006	},	// Microsoft IntelliMouse Explorer 3.0 USB (illegal ID values)
	{	"msime3",		0x045e,		0x0047	},	// Microsoft IntelliMouse Explorer 3.0 USB (should be these)
	{	"rx300",		0x046d,		0xC040	},	// Logitech RX300
	{	"mx500",		0x046d,		0xC025	},	// Logitech MX-500
	{	"mx310",		0x046d,		0xC01B	},	// Logitech MX-310
	{	"0",			0xFFFF,		0xFFFF	}	// Last Field MUST be 0xFFFF, 0xFFFF
};

static int detected_device = 0;

void device_set(int index)
{
	detected_device = index;
}

const char *device_get_name(int index)
{
	if (index == -1)
		index = detected_device;
	return devices[index].name;
}

int device_get_vendor(int index)
{
	if (index == -1)
		index = detected_device;
	return devices[index].vendor;
}

int device_get_product(int index)
{
	if (index == -1)
		index = detected_device;
	return devices[index].product;
}
