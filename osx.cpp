// Copyright (C) 1999-2005 Open Source Telecom Corporation.
// Copyright (C) 2006-2014 David Sugar, Tycho Softworks.
// Copyright (C) 2015 Cherokees of Idaho.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.
//
// This exception applies only to the code released under the name GNU
// ccAudio.  If you copy code from other releases into a copy of GNU
// ccAudio, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
//
// If you write modifications of your own for GNU ccAudio, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.
//

#include <ucommon/ucommon.h>
#include <ccaudio2-config.h>

extern int _osx_ccaudio_dummy;
int _osx_ccaudio_dummy = 0;

#if defined(OSX_AUDIO)

#include <CoreAudio/AudioHardware.h>
#include <ucommon/export.h>
#include <ccaudio2.h>

static unsigned devcount = 0;
static AudioDeviceID *devlist = NULL, *devinput = NULL, *devoutput = NULL;

static bool scanDevices(unsigned index)
{
    OSStatus err = noErr;
    UInt32 size;
    AudioDeviceID *id;
    unsigned i;

    if(!devlist) {
        if(AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &size, NULL) != noErr)
            return false;

        devcount = size / sizeof(AudioDeviceID);
        devlist = (AudioDeviceID *)malloc(size);

        if(AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &size, (void *) devlist) != noErr) {
            free(devlist);
            return false;
        }

        size = sizeof(AudioDeviceID);
        if(AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &size, &id) == noErr) {
            for(i = 0; i < devcount; ++i)
            {
                if(!memcmp(&devlist[i], &id, sizeof(AudioDeviceID))) {
                    devinput = &devlist[i];
                    break;
                }
            }
        }

        size = sizeof(AudioDeviceID);
        if(AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &id) == noErr) {
            for(i = 0; i < devcount; ++i)
            {
                if(!memcmp(&devlist[i], &id, sizeof(AudioDeviceID))) {
                    devoutput = &devlist[i];
                    break;
                }
            }
        }
    }
    if(!devcount)
        return false;

    if(index > devcount)
        return false;

    return true;
}

namespace ucommon {

bool Audio::is_available(unsigned index)
{
    return scanDevices(index);
}

AudioDevice *Audio::getDevice(unsigned index, DeviceMode mode)
{
    if(!scanDevices(index))
        return NULL;

    return NULL;
}

} // namespace ucommon


#endif
