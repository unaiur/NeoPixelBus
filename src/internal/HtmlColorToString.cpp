#include "HtmlColor.h"
#include "NamedColor.h"

static inline char hexdigit(int v)
{
  return v + (v < 10 ? '0' : 'a' - 10);
}

HtmlColor::operator String() const
{
    const NamedColor *nc = namedColors;
    while (nc->Name != 0 && nc->Color != Color)
        ++nc;

    //Serial.printf("Color %x at index %u\n", Color, nc - namedColors);
    String result;
    if (nc->Name)
    {
        result = String(FPSTR(nc->Name));
    }
    else
    {
        char res[] = {
            '#',
            hexdigit((Color >> 20) & 0xF),
            hexdigit((Color >> 16) & 0xF),
            hexdigit((Color >> 12) & 0xF),
            hexdigit((Color >>  8) & 0xF),
            hexdigit((Color >>  4) & 0xF),
            hexdigit( Color        & 0xF),
            0
        };
        result = res;
    }
    return result;
}
