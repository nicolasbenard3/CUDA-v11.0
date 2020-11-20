// Copyright (C) 2000-2005 Open Source Telecom Corporation.
// Copyright (C) 2006-2014 David Sugar, Tycho Softworks.
// Copyright (C) 2015 Cherokees of Idaho.
//
// This file is part of GNU ccAudio2.
//
// GNU ccAudio2 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GNU ccAuydio2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with GNU ccAudio2.  If not, see <http://www.gnu.org/licenses/>.

#include <ucommon/ucommon.h>
#include <ccaudio2-config.h>
#include <ucommon/export.h>
#include <ccaudio2.h>

namespace ucommon {

AudioResample::AudioResample(Rate div, Rate mul)
{
    bool common = true;
    while(common) {
        common = false;

        while(!(mul & 0x01) && !(div & 0x01)) {
            mul = (Rate)(mul >> 1);
            div = (Rate)(div >> 1);
            common = true;
        }

        while(!(mul % 3) && !(div % 3)) {
            mul = (Rate)(mul / 3);
            div = (Rate)(div / 3);
            common = true;
        }

        while(!(mul % 5) && !(div %5)) {
            mul = (Rate)(mul / 5);
            div = (Rate)(div / 5);
            common = true;
        }
    }


    mfact = (unsigned)mul;
    dfact = (unsigned)div;

    max = mfact;
    if(dfact > mfact)
        max = dfact;

    ++max;
    buffer = new Sample[max];
    ppos = gpos = 0;
    memset(buffer, 0, max * 2);
    last = 0;
}

AudioResample::~AudioResample()
{
    delete[] buffer;
}

size_t AudioResample::estimate(size_t count)
{
    count *= mfact;
    count += (mfact - 1);
    return (count / dfact) + 1;
}

size_t AudioResample::process(Linear from, Linear dest, size_t count)
{
    size_t saved = 0;
    Sample diff, current;
    unsigned pos;
    unsigned dpos;

    while(count--) {
        current = *(from++);
        diff = (current - last) / mfact;
        pos = mfact;
        while(pos--) {
            last += diff;
            buffer[ppos++] = current;
            if(ppos >= max)
                ppos = 0;

            if(gpos < ppos)
                dpos = ppos - gpos;
            else
                dpos = max - (gpos - ppos);
            if(dpos >= dfact) {
                *(dest++) = buffer[gpos];
                ++saved;
                gpos += dfact;
                if(gpos >= max)
                    gpos = gpos - max;
            }
        }
        last = current;
    }
    return saved;
}

} // namespace ucommon
