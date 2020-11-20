// Copyright (C) 2005 David Sugar, Tycho Softworks.
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
#include <ctype.h>
#include <ucommon/export.h>
#include <ccaudio2.h>

namespace ucommon {

TelTone::TelTone(tonekey_t *k, Level l, timeout_t duration) :
AudioTone(duration)
{
    tone = k;

    if(!tone) {
        complete = true;
        return;
    }

    framing = duration;
    def = tone->first;
    complete = false;
    remaining = silent = count = 0;
    level = l;
}

TelTone::~TelTone()
{
    AudioTone::cleanup();
}

bool TelTone::is_complete(void)
{
    return complete;
}

Audio::Linear TelTone::getFrame(void)
{
    if(complete)
        return NULL;

    if(count >= def->count && !remaining && !silent) {
        def = def->next;
        count = 0;
        if(!def) {
            complete = true;
            return NULL;
        }
    }

    if(!remaining && !silent) {
        if(count && !def->duration)
            return AudioTone::getFrame();

        if(def->f2)
            dual(def->f1, def->f2, level, level);
        else
            single(def->f1, level);
        ++count;
        remaining = def->duration / framing;
        if(def->silence)
            silent = (def->duration + def->silence) / framing - remaining;
        else
            silent = 0;
    }

    if(!remaining && m1 && silent)
        reset();

    if(remaining)
        --remaining;
    else if(silent)
        --silent;

    return AudioTone::getFrame();
}

} // namespace ucommon
