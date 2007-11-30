
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */
  

#include "device.h"

/* Static variables */
static int product_id=0;
static int vendor_id=0;


int device_get_vendor_id(void)
{
	return vendor_id;
}

int device_get_product_id(void)
{
	return product_id;
}

void device_set_vendor_id(int id)
{
	vendor_id = id;
}

void device_set_product_id(int id)
{
	product_id = id;
}
