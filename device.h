
 /* 
  * Copyright (C) 2007  Olli Salonen <oasalonen@gmail.com>
  * see btnx.c for detailed license information
  */
  
#ifndef DEVICE_H_
#define DEVICE_H_

int device_get_vendor_id(void);
int device_get_product_id(void);
void device_set_vendor_id(int id);
void device_set_product_id(int id);

#endif /*DEVICE_H_*/
