#ifndef VERSION_H_
#define VERSION_H_

#define VER_MAIN        0
#define VER_MINOR       1
#define VER_REV         0
#define VER_BUILD       0

#define VER_COMBO       (VER_MAIN << 24 & VER_MINOR << 16 & VER_REV << 8 & VER_BUILD)

#endif /*VERSION_H_*/