#ifndef FS4412_LED_H
#define FS4412_LED_H

#define LED_MAGIC 'L'
/*
 * need arg = 1/2 
 */

#define LED_ON	_IOW(LED_MAGIC, 0, int)
#define LED_OFF	_IOW(LED_MAGIC, 1, int)

#endif
