#ifndef __UTILS_H__
#define __UTILS_H__

#define HAS_FLAG(MASK, VAL) VAL & MASK
#define FLAG_ON(MASK, VAL) VAL = VAL | MASK
#define FLAG_OFF(MASK, VAL) VAL = VAL & ~MASK

#endif