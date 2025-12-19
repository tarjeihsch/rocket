// Right-hand rule

#define PI 3.14
#define RAD(x) x * (PI / 180.0)
#define DEG(x) x * (180.0 / PI)

#ifdef MATH_REAL_32
    using Real = float;
#endif

#ifdef MATH_REAL_64
    using Real = double;
#endif
