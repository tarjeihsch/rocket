// Right-hand rule

#include "CoreTypes.h"

#define PI 3.14
#define RAD(x) x * (PI / 180.0)
#define DEG(x) x * (180.0 / PI)

#ifdef MATH_REAL_64
    using Real = double;
#else
    using Real = float;
#endif
