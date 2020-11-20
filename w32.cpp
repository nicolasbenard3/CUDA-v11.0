// Copyright (C) 2004-2005 Open Source Telecom Corporation.
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
#ifdef  HAVE_ENDIAN_H
#include <endian.h>
#endif
#define MAX_DEVICES 1

#if defined(_MSWINDOWS_) && !defined(__BIG_ENDIAN)
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN    4321
#define __PDP_ENDIAN    3412
#define __BYTE_ORDER    __LITTLE_ENDIAN
#endif

extern int _w32_ccaudio_dummy;
int _w32_ccaudio_dummy = 0;

#ifdef  _MSWINDOWS_

#include "mmsystem.h"
#include <ucommon/export.h>
#include <ccaudio2.h>

namespace ucommon {

extern  LONG buffer_framing;
extern  LONG buffer_count;

class __LOCAL W32Semaphore
{
private:
    HANDLE sem;

public:
    W32Semaphore(unsigned count);
    ~W32Semaphore();
    void wait(void);
    void post(void);
};

W32Semaphore::W32Semaphore(unsigned count)
{
    sem = CreateSemaphore((LPSECURITY_ATTRIBUTES)NULL, (LONG)count - 2, (LONG)count - 1, (LPCSTR)NULL);
}

W32Semaphore::~W32Semaphore()
{
    CloseHandle(sem);
}

void W32Semaphore::wait(void)
{
    WaitForSingleObject(sem, INFINITE);
}

void W32Semaphore::post(void)
{
    ReleaseSemaphore(sem, 1, (LPLONG)NULL);
}

class __LOCAL W32AudioDevice : public AudioDevice
{
private:
    HWAVEOUT hPlay;
    WAVEFORMATEX wfx;
    WAVEHDR *headers;
    char *buffers;
    W32Semaphore *sem;
    unsigned bufsize, bufcount;

    volatile DWORD index, counter;

    unsigned channels;
    unsigned devid;

    static W32AudioDevice *devices[MAX_DEVICES + 1];

public:
    W32AudioDevice(unsigned index);
    ~W32AudioDevice();

    bool setAudio(Rate rate, bool stereo, timeout_t timeout);

    unsigned putSamples(Linear buffer, unsigned count);
    unsigned getSamples(Linear buffer, unsigned count);

    friend void CALLBACK playProc(HWAVEOUT hPlay, UINT msg, DWORD instance, DWORD p1, DWORD p2);

    void sync(void);
    void flush(void);
};

W32AudioDevice *W32AudioDevice::devices[MAX_DEVICES + 1];

void CALLBACK playProc(HWAVEOUT hPlay, UINT msg, DWORD instance, DWORD p1, DWORD p2)
{
    W32AudioDevice *dev = W32AudioDevice::devices[instance];
    if(msg != WOM_DONE)
        return;

    if(!dev->sem)
        return;

    --dev->counter;
    dev->sem->post();
}

W32AudioDevice::W32AudioDevice(unsigned index)
{
    headers = NULL;
    buffers = NULL;

    enabled = false;
    sem = NULL;

    devid = index;
    devices[index] = this;
}

W32AudioDevice::~W32AudioDevice()
{
    if(enabled)
        waveOutClose(hPlay);

    if(sem)
        delete sem;

    if(headers)
        delete[] headers;

    if(buffers)
        delete[] buffers;
}

void W32AudioDevice::sync(void)
{
    Sleep(counter * info.framing);
}

void W32AudioDevice::flush(void)
{
    unsigned i;

    if(!enabled)
        return;

    waveOutReset(hPlay);
    waveOutClose(hPlay);
    enabled = false;

    for(i = 0; i < bufcount; i++)
    {
        if(headers[i].dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hPlay, &headers[i], sizeof(WAVEHDR));
    }
    if(sem)
        delete sem;

    sem = NULL;
    counter = 0;
}

bool W32AudioDevice::setAudio(Rate rate, bool stereo, timeout_t framing)
{
    unsigned i = 0;

    flush();

    wfx.nSamplesPerSec = rate;
    wfx.wBitsPerSample = 16;

    if(stereo) {
        info.encoding = pcm16Stereo;
        channels = 2;
    }
    else {
        info.encoding = pcm16Mono;
        channels = 1;
    }

    counter = 0;
    index = 0;
    bufsize = 2 * channels * rate * buffer_framing / 1000;
    bufcount = buffer_count;

    if(headers)
            delete[] headers;

    if(buffers)
            delete[] buffers;

    headers = new WAVEHDR[bufcount];
    buffers = new char[bufsize * bufcount];

    memset(headers, 0, sizeof(WAVEHDR) * bufcount);
    while(i < bufcount) {
        headers[i].dwBufferLength = bufsize;
        headers[i].lpData = &buffers[i * bufsize];
        ++i;
    }

    info.rate = rate;
    info.bitrate = rate * 16 * channels;
    info.order = __BYTE_ORDER;
    info.format = raw;
    info.annotation = (char *)"Microsoft Windows";
    info.framecount = bufsize / 2 / channels;
    info.framesize = bufsize;
    info.framing = (info.framecount * 1000l) / info.rate;

    wfx.nChannels = channels;
    wfx.cbSize = 0;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = 2 * channels;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * rate;

    if(enabled) {
        waveOutClose(hPlay);
        enabled = false;
    }

    index = 0;

    if(sem)
        delete sem;

    sem = new W32Semaphore(bufcount);

    if(waveOutOpen(&hPlay, WAVE_MAPPER, &wfx, (DWORD_PTR)&playProc, (DWORD)devid, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
        return false;

    enabled = true;
    return true;
}

unsigned W32AudioDevice::getSamples(Linear samples, unsigned count)
{
    return 0;
}

unsigned W32AudioDevice::putSamples(Linear samples, unsigned count)
{
    size_t size = count * channels * 2;
    unsigned char *data = (unsigned char *)samples;
    unsigned fill;
    unsigned total = 0;
    unsigned scale = 2 * channels;
    WAVEHDR *header;
    MMRESULT result;

    swapEndian(info, samples, count);

    while(size > 0) {
        header = &headers[index];
        if(header->dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(hPlay, header, sizeof(WAVEHDR));

        if(size < (bufsize - header->dwUser)) {
            memcpy(header->lpData + header->dwUser, data, size);
            header->dwUser += size;
            total += (unsigned)(size / scale);
            break;
        }
        fill = (unsigned)(bufsize - header->dwUser);
        memcpy(header->lpData + header->dwUser, data, fill);
        size -= fill;
        data += fill;
        total += fill / scale;
        header->dwBufferLength = bufsize;
        header->dwUser = 0;
        result = waveOutPrepareHeader(hPlay, header, sizeof(WAVEHDR));

        if(result == MMSYSERR_NOERROR) {
            sem->wait();
            ++counter;
            result = waveOutWrite(hPlay, header, sizeof(WAVEHDR));
        }
        index = (++index) % bufcount;
        headers[index].dwUser = 0;
    }
    return total;
}

bool Audio::is_available(unsigned index)
{
    if(index > 1)
        return false;

    return true;
}

AudioDevice *Audio::getDevice(unsigned index, DeviceMode mode)
{
    AudioDevice *dev;

    if(index > MAX_DEVICES)
        return NULL;

    if(mode != PLAY)
        return NULL;

    dev = new W32AudioDevice(index);
    dev->setAudio(rate8khz, false);
    return dev;
}

} // namespace ucommon

#endif
