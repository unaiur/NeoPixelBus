#include "HtmlColor.h"
#include "NamedColor.h"

static inline int hexvalue(char c)
{
    return c - (c < 'a' ? '0' : 'a' - 10);
}

bool HtmlColor::Parse(String name)
{
    name.toLowerCase();
    if (name[0] == '#')
    {
        if (name.length() > 7)
            return false;
        u32 color = 0;
        for (int i = 1; i < name.length(); ++i)
        {
            if (!isxdigit(name[i]))
                return false;
            color = (color << 4) | hexvalue(name[i]);
        }
        Color = color;
        return true;
    }

    Serial.printf("Searching for %s\n", name.c_str());
    const NamedColor *nc = namedColors;
    while (nc->Name != 0)
    {
        if (name == FPSTR(nc->Name))
        {
            Color = nc->Color;
            return true;
        }
	++nc;
    }
    return false;
}

