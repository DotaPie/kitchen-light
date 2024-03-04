#ifndef MACROS_H
#define MACROS_H

#define RGB888_TO_RGB565(R, G, B){(uint16_t)((((R >> 3) & 0x1f) << 11) | (((G >> 2) & 0x3f) << 5) | ((B >> 3) & 0x1f))}

#endif