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
// GNU ccAudio2 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with GNU ccAudio2.  If not, see <http://www.gnu.org/licenses/>.

#include <ucommon/ucommon.h>
#include <ccaudio2-config.h>
#include <math.h>
#include <ucommon/export.h>
#include <ccaudio2.h>

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

namespace ucommon {

AudioTone::AudioTone(timeout_t duration, Rate r)
{
    rate = r;
    df1 = df2 = 0;
    samples = (duration *(long)rate) / 1000;
    frame = new Sample[samples];
    silencer = true;

    reset();
}

AudioTone::AudioTone(unsigned freq, Level l, timeout_t duration, Rate r)
{
    rate = r;
    df1 = (freq * M_PI * 2) / (long)rate;
    df2 = (freq * M_PI * 2) / (long)rate;
    p1 = 0, p2 = 0;
    samples = (duration * (long)rate) / 1000;
    m1 = l / 2;
    m2 = l / 2;
    silencer = false;

    frame = new Sample[samples];
}

AudioTone::AudioTone(unsigned f1, unsigned f2, Level l1, Level l2, timeout_t duration, Rate r)
{
    rate = r;
    df1 = (f1 * M_PI * 2) / (long)r;
    df2 = (f2 * M_PI * 2) / (long)r;
    p1 = 0, p2 = 0;
    samples = (duration * (long)r) / 1000;
    m1 = l1 / 2;
    m2 = l2 / 2;
    silencer = false;

    frame = new Sample[samples];
}

unsigned AudioTone::getFrames(Linear buffer, unsigned pages)
{
    unsigned count = 0;
    Linear save = frame;

    while(count < pages) {
        frame = buffer;
        buffer += samples;
        if(!getFrame())
            break;
        ++count;
    }

    if(count && count < pages)
        memset(buffer, 0, samples * (pages - count) * 2);

    frame = save;
    return count;
}


bool AudioTone::is_complete(void)
{
    return false;
}

void AudioTone::silence(void)
{
    silencer = true;
}

void AudioTone::reset(void)
{
    m1 = m2 = 0;
    p1 = p2 = 0;
}

void AudioTone::single(unsigned freq, Level l)
{
    df1 = (freq * M_PI * 2) / (long)rate;
    df2 = (freq * M_PI * 2) / (long)rate;
    m1 = l / 2;
    m2 = l / 2;
    silencer = false;
}

void AudioTone::dual(unsigned f1, unsigned f2, Level l1, Level l2)
{
    df1 = (f1 * M_PI * 2) / (long)rate;
    df2 = (f2 * M_PI * 2) / (long)rate;
    m1 = l1 / 2;
    m2 = l2 / 2;
    silencer = false;
}

Audio::Linear AudioTone::getFrame(void)
{
    unsigned count = samples;
    Linear data = frame;

    if(is_silent() && !p1 && !p2) {
        if(!p1 && !p2) {
            memset(frame, 0, samples * 2);
            return frame;
        }
    }
    else if(silencer)
    {
        while(count--) {
            if(p1 <= 0 && df1 >= p1) {
                p1 = 0;
                df1 = 0;
                m1 = 0;
            }
            if(p1 >= 0 && -df1 >= p1) {
                p1 = 0;
                df1 = 0;
                m1 = 0;
            }
            if(p2 <= 0 && df2 >= p1) {
                p2 = 0;
                df2 = 0;
                m2 = 0;
            }
            if(p2 >= 0 && -df2 >= p1) {
                p2 = 0;
                df2 = 0;
                m2 = 0;
            }

            if(!m1 && !m2) {
                *(data++) = 0;
                continue;
            }

                    *(data++) = (Level)(sin(p1) * (double)m1) +
                            (Level)(sin(p2) * (double)m2);

            p1 += df1;
            p2 += df2;
        }
    }
    else {
        while(count--) {
                    *(data++) = (Level)(sin(p1) * (double)m1) +
                (Level)(sin(p2) * (double)m2);

                    p1 += df1;
                    p2 += df2;
        }
    }

    return frame;
}

AudioTone::~AudioTone()
{
    cleanup();
}

void AudioTone::cleanup(void)
{
    if(frame) {
        delete[] frame;
        frame = NULL;
    }
}

bool AudioTone::is_silent(void)
{
    if(!m1 && !m2)
        return true;

    return false;
}

} // namespace ucommon
