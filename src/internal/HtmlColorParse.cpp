/*-------------------------------------------------------------------------
HtmlColor::Parse() implementation

Written by Unai Uribarri

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/
#include "HtmlColor.h"

bool HtmlColor::Parse(const char* name)
{
    if (name[0] == '#')
    {
        char *err;
        unsigned long color = strtoul(name + 1, &err, 16);
        if (*err || color > 0xFFFFFF)
        {
            return false;
        }
        Color = color;
        return true;
    }

    const HtmlColorName* cn = ColorNames;
    while (cn->Name != 0)
    {
        if (strcasecmp_P(name, cn->Name) == 0)
        {
            Color = cn->Color;
            return true;
        }
        ++cn;
    }
    return false;
}

