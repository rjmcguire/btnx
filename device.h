
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */
  
#ifndef DEVICE_H_
#define DEVICE_H_

typedef struct device
{
	char name[20];
	int vendor;
	int product;
} device;


void device_set(int index);

const char *device_get_name(int index);
int device_get_vendor(int index);
int device_get_product(int index);

#endif /*DEVICE_H_*/
