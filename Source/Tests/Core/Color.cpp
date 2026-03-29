#include <assert.h>

#include "Color.h"

int main()
{
    Color32 color;

    // Red color
    color = Color32(255, 0, 0, 255);

    assert(color.data == Color32::red.data);

    return 0;
}