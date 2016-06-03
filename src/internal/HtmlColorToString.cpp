/*-------------------------------------------------------------------------
HtmlColor::ToString() implementation

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

static inline char hexdigit(char v)
{
    return v + (v < 10 ? '0' : 'a' - 10);
}

void HtmlColor::ToString(char name[MAX_HTML_COLOR_NAME_LEN]) const
{
    for (const HtmlColorName* cn = ColorNames; cn->Name; ++cn)
    {
        if (cn->Color == Color)
        {
            strcpy_P(name, cn->Name);
            return;
        }
    }

    name[0] = '#';
    name[1] = hexdigit((Color >> 20) & 0xF);
    name[2] = hexdigit((Color >> 16) & 0xF);
    name[3] = hexdigit((Color >> 12) & 0xF);
    name[4] = hexdigit((Color >>  8) & 0xF);
    name[5] = hexdigit((Color >>  4) & 0xF);
    name[6] = hexdigit( Color        & 0xF);
    name[7] = 0;
}
