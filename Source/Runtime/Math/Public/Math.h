#define PI 3.14
#define RAD(x) x * (PI / 180.0)
#define DEG(x) x * (180.0 / PI)

#ifdef MATH_SP
    using TP = float;
    #define TValue = TP;
#endif

#ifdef MATH_DP
    using TP = double;
    #define TValue = TP;
#endif
