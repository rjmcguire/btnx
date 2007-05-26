
 /* 
  * Copyright (C) 2007  Olli Salonen (www.ollisalonen.com)
  * see btnx.c for detailed license information
  */
  

#include "device.h"


/*const int devices[][2] = 
{ //	VendorID	ProductID
	{	0x046D,		0xC51A	},	// Logitech MX Revolution
	{	0x046D,		0xC518	},	// Logitech VX Revolution
	{	0xFFFF,		0xFFFF	}	// Last Field MUST be 0xFFFF, 0xFFFF
};*/

const device devices[] =
{ //	Name			VendorID	ProductID
	{	"mxrevo",		0x046D,		0xC51A	},	// Logitech MX Revolution
	{	"vxrevo",		0x046D,		0xC518	},	// Logitech VX Revolution
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
