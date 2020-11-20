// Copyright (C) 1999-2005 Open Source Telecom Corporation.
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
#ifdef  HAVE_ENDIAN_H
#include <endian.h>
#endif
#include <ucommon/export.h>
#include <ccaudio2.h>

#if defined(__GNUC__)
#define _PACKED
#elif !defined(__hpux) && !defined(_AIX)
#define _PACKED
#endif

#if !defined(__BIG_ENDIAN)
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN    4321
#define __PDP_ENDIAN    3412
#define __BYTE_ORDER    __LITTLE_ENDIAN
#endif

namespace ucommon {

#ifdef  _PACKED
#pragma pack(1)
#endif

typedef struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char mp_sync1 : 8;
    unsigned char mp_crc   : 1;
    unsigned char mp_layer : 2;
    unsigned char mp_ver   : 2;
    unsigned char mp_sync2 : 3;

    unsigned char mp_priv  : 1;
    unsigned char mp_pad   : 1;
    unsigned char mp_srate : 2;
    unsigned char mp_brate : 4;

    unsigned char mp_emp   : 2;
    unsigned char mp_original : 1;
    unsigned char mp_copyright: 1;
    unsigned char mp_extend   : 2;
    unsigned char mp_channels : 2;

#else
    unsigned char mp_sync1 : 8;

    unsigned char mp_sync2 : 3;
    unsigned char mp_ver   : 2;
    unsigned char mp_layer : 2;
    unsigned char mp_crc   : 1;

    unsigned char mp_brate : 4;
    unsigned char mp_srate : 2;
    unsigned char mp_pad   : 1;
    unsigned char mp_priv  : 1;

    unsigned char mp_channels : 2;
    unsigned char mp_extend   : 2;
    unsigned char mp_copyright : 1;
    unsigned char mp_original : 1;
    unsigned char mp_emp : 2;
#endif
}   mpeg_audio;

typedef struct {
    char tag_id[3];
    char tag_title[30];
    char tag_artist[30];
    char tag_album[30];
    char tag_year[4];
    char tag_note[30];
    unsigned char genre;
}   mpeg_tagv1;

#ifdef  _PACKED
#pragma pack()
#endif

static const char * const ErrorStrs[] = {
    "errSuccess",
    "errReadLast",
    "errNotOpened",
    "errEndOfFile",
    "errStartOfFile",
    "errRateInvalid",
    "errEncodingInvalid",
    "errReadInterrupt",
    "errWriteInterrupt",
    "errReadFailure",
    "errWriteFailure",
    "errReadIncomplete",
    "errWriteIncomplete",
    "errRequestInvalid",
    "errTOCFailed",
    "errStatFailed",
    "errInvalidTrack",
    "errPlaybackFailed",
    "errNotPlaying"
};

static void mp3info(Audio::Info& info, mpeg_audio *mp3)
{
    info.headersize = 4;
    info.padding = 0;

    if(mp3->mp_pad)
        info.padding = 1;

    switch(mp3->mp_layer) {
    case 0x03:
        if(mp3->mp_pad)
            info.padding = 4;
        info.encoding = Audio::mp1Audio;
        break;
    case 0x02:
        info.encoding = Audio::mp2Audio;
        break;
    case 0x01:
        info.encoding = Audio::mp3Audio;
        break;
    }

    switch(mp3->mp_ver) {
    case 0x03:
        info.bitrate = 32000;
        switch(mp3->mp_srate) {
        case 00:
            info.rate = 44100;
            break;
        case 01:
            info.rate = 48000;
            break;
        case 02:
            info.rate = 32000;
        }
        switch(mp3->mp_layer) {
        case 0x03:
            info.bitrate = 32000 * mp3->mp_brate;
            break;
        case 0x02:
            if(mp3->mp_brate < 8)
                info.bitrate = 16000 * (mp3->mp_brate + 1);
            else
                info.bitrate = 32000 * (mp3->mp_brate - 4);
            break;
        case 0x01:
            switch(mp3->mp_brate) {
            case 0x02:
                info.bitrate = 40000;
                break;
            case 0x03:
                info.bitrate = 48000;
                break;
            case 0x04:
                info.bitrate = 56000;
                break;
            case 0x05:
                info.bitrate = 64000;
                break;
            case 0x06:
                info.bitrate = 80000;
                break;
            case 0x07:
                info.bitrate = 96000;
                break;
            case 0x08:
                info.bitrate = 112000;
                break;
            case 0x09:
                info.bitrate = 128000;
                break;
            case 0x0a:
                info.bitrate = 160000;
                break;
            case 0x0b:
                info.bitrate = 192000;
                break;
            case 0x0c:
                info.bitrate = 224000;
                break;
            case 0x0d:
                info.bitrate = 256000;
                break;
            case 0x0e:
                info.bitrate = 320000;
                break;
            }
            break;
        }
        break;
    case 0x00:
        switch(mp3->mp_srate) {
        case 00:
            info.rate = 11025;
            break;
        case 01:
            info.rate = 12000;
            break;
        case 02:
            info.rate = 8000;
            break;
        }
    case 0x02:
        if(mp3->mp_ver == 0x02) {
            switch(mp3->mp_srate) {
            case 00:
                info.rate = 22050;
                break;
            case 01:
                info.rate = 24000;
                break;
            case 02:
                info.rate = 16000;
                break;
            }
        }
        switch(mp3->mp_layer) {
        case 0x03:
            if(mp3->mp_brate < 0x0d)
                info.bitrate = 16000 * (mp3->mp_brate + 1);
            else if(mp3->mp_brate == 0x0d)
                info.bitrate = 224000;
            else
                info.bitrate = 256000;
            break;
        case 0x02:
        case 0x01:
            if(mp3->mp_brate < 9)
                info.bitrate = 8000 * mp3->mp_brate;
            else
                info.bitrate = 16000 * (mp3->mp_brate - 4);
        }
    }

    if(mp3->mp_crc)
        info.headersize = 6;

    info.set();
}



AudioCodec *AudioFile::getCodec(void)
{
    Encoding e = getEncoding();
    switch(e) {
    case alawAudio:
        return AudioCodec::get(e, "g.711");
    case mulawAudio:
        return AudioCodec::get(e, "g.711");
    case g721ADPCM:
    case okiADPCM:
    case voxADPCM:
        return AudioCodec::get(e, "g.721");
    case g722_7bit:
    case g722_6bit:
        return AudioCodec::get(e, "g.722");
    case g723_3bit:
    case g723_5bit:
        return AudioCodec::get(e, "g.723");
    default:
        return NULL;
    }
}

void AudioFile::setShort(unsigned char *data, unsigned short val)
{
    if(info.order == __BIG_ENDIAN) {
        data[0] = val / 256;
        data[1] = val % 256;
    }
    else {
        data[1] = val / 256;
        data[0] = val % 256;
    }
}

unsigned short AudioFile::getShort(unsigned char *data) const
{
    if(info.order == __BIG_ENDIAN)
        return data[0] * 256 + data[1];
    else
        return data[1] * 256 + data[0];
}

void AudioFile::setLong(unsigned char *data, unsigned long val)
{
    int i = 4;

    while(i-- > 0) {
        if(info.order == __BIG_ENDIAN)
            data[i] = (unsigned char)(val & 0xff);
        else
            data[3 - i] = (unsigned char)(val & 0xff);
        val /= 256;
    }
}

unsigned long AudioFile::getLong(unsigned char * data) const
{
    int i = 4;
    unsigned long val =0;

    while(i-- > 0) {
        if(info.order == __BIG_ENDIAN)
            val = (val << 8) | data[3 - i];
        else
            val = (val << 8) | data[i];
    }
    return val;
}

AudioFile::AudioFile(const char *name, unsigned long sample) :
AudioBase()
{
    pathname = NULL;
    initialize();
    AudioFile::open(name);
    if(!is_open())
        return;
    setPosition(sample);
}

AudioFile::AudioFile(const char *name, Info *inf, unsigned long samples) :
AudioBase(inf)
{
    pathname = NULL;
    initialize();
    AudioFile::create(name, inf);
    if(!is_open())
        return;
    setMinimum(samples);
}

AudioFile::~AudioFile()
{
    AudioFile::close();
    AudioFile::clear();
}

void AudioFile::create(const char *name, Info *myinfo, bool exclusive, timeout_t framing)
{
    int pcm = 0;
    unsigned char aufile[24];
    unsigned char riffhdr[40];
    const char *ext = strrchr(name, '/');

    if(!ext)
        ext = strrchr(name, '\\');

    if(!ext)
        ext = strrchr(name, ':');

    if(!ext)
        ext = strrchr(name, '.');
    else
        ext = strrchr(ext, '.');

    if(!ext)
        ext = ".none";

    mode = modeWrite;

    if(!afCreate(name, exclusive))
        return;

    memset(riffhdr, 0, sizeof(riffhdr));
    memcpy(&info, myinfo, sizeof(info));
    info.annotation = NULL;
    pathname = new char[strlen(name) + 1];
    strcpy(pathname, name);
    if(myinfo->annotation) {
        info.annotation = new char[strlen(myinfo->annotation) + 1];
        strcpy(info.annotation, myinfo->annotation);
    }

    if(!stricmp(ext, ".raw") || !stricmp(ext, ".bin")) {
        info.format = raw;
        if(info.encoding == unknownEncoding)
            info.encoding = pcm16Mono;
    }
    else if(!stricmp(ext, ".au") || !stricmp(ext, ".snd"))
    {
        info.order = 0;
        info.format = snd;
    }
    else if(!stricmp(ext, ".wav") || !stricmp(ext, ".wave"))
    {
        info.order = 0;
        info.format = wave;
    }
    else if(!stricmp(ext, ".ul") || !stricmp(ext, ".ulaw") || !stricmp(ext, ".mulaw"))
    {
        info.encoding = mulawAudio;
        info.format = raw;
        info.order = 0;
        info.rate = 8000;
    }
    else if(!stricmp(ext, ".al") || !stricmp(ext, ".alaw"))
    {
        info.encoding = alawAudio;
        info.format = raw;
        info.order = 0;
        info.rate = 8000;
    }
    else if(!stricmp(ext, ".sw") || !stricmp(ext, ".pcm"))
    {
        info.encoding = pcm16Mono;
        info.format = raw;
        info.order = 0;
    }
    else if(!stricmp(ext, ".vox"))
    {
        info.encoding = voxADPCM;
        info.format = raw;
        info.order = 0;
        info.rate = 6000;
    }
    else if(!stricmp(ext, ".gsm"))
    {
        info.encoding = gsmVoice;
        info.format = raw;
        info.order = 0;
        info.rate = 8000;
        info.framecount = 160;
        info.framesize = 33;
        info.bitrate = 13200;
    }
    else if(!stricmp(ext, ".adpcm") || !stricmp(ext, ".a32"))
    {
        info.encoding = g721ADPCM;
        info.format = raw;
        info.order = 0;
        info.rate = 8000;
    }
    else if(!stricmp(ext, ".a24"))
    {
        info.encoding = g723_3bit;
        info.format = raw;
        info.order = 0;
        info.rate = 8000;
    }
    else if(!stricmp(ext, ".a16"))
    {
        info.encoding = g723_2bit;
        info.format = raw;
        info.order = 0;
        info.order = 8000;
    }
    else if(!stricmp(ext, ".sx"))
    {
        info.encoding = sx96Voice;
        info.format = raw;
        info.order = 0;
        info.rate = 8000;
    }
    else if(!stricmp(ext, ".a40"))
    {
        info.encoding = g723_5bit;
        info.format = raw;
        info.order = 0;
        info.rate = 8000;
    }
    else if(!stricmp(ext, ".cda"))
    {
        info.encoding = cdaStereo;
        info.format = raw;
        info.order = __LITTLE_ENDIAN;
        info.rate = 44100;
    }

    switch(info.format) {
    case wave:
    case riff:
        /*
         * RIFF header
         *
         * 12 bytes
         *
         * 0-3: RIFF magic "RIFF" (0x52 49 46 46)
         *
         * 4-7: RIFF header chunk size
         *      (4 + (8 + subchunk 1 size) + (8 + subchunk 2 size))
         *
         * 8-11: WAVE RIFF type magic "WAVE" (0x57 41 56 45)
         */
        if(!info.order)
            info.order = __LITTLE_ENDIAN;
        if(info.order == __LITTLE_ENDIAN)
            strncpy((char *)riffhdr, "RIFF", 4);
        else
            strncpy((char *)riffhdr, "RIFX", 4);
        if(!info.rate)
            info.rate = Audio::getRate(info.encoding);
        if(!info.rate)
            info.rate = rate8khz;

        header = 0;

        memset(riffhdr + 4, 0xff, 4);
        strncpy((char *)riffhdr + 8, "WAVE", 4);
        if(afWrite(riffhdr, 12) != 12) {
            AudioFile::close();
            return;
        }

        /*
         * Subchunk 1: WAVE metadata
         *
         * Length: 24+
         *
         * (offset from start of subchunk 1) startoffset-endoffset
         *
         * (0) 12-15: WAVE metadata magic "fmt " (0x 66 6d 74 20)
         *
         * (4) 16-19: subchunk 1 size minus 8.  0x10 for PCM.
         *
         * (8) 20-21: Audio format code.  0x01 for linear PCM.
         * More codes here:
         * http://www.goice.co.jp/member/mo/formats/wav.html
         *
         * (10) 22-23: Number of channels.  Mono = 0x01,
         * Stereo = 0x02.
         *
         * (12) 24-27: Sample rate in samples per second. (8000,
         * 44100, etc)
         *
         * (16) 28-31: Bytes per second = SampleRate *
         * NumChannels * (BitsPerSample / 8)
         *
         * (20) 32-33: Block align (the number of bytes for
         * one multi-channel sample) = Numchannels *
         * (BitsPerSample / 8)
         *
         * (22) 34-35: Bits per single-channel sample.  8 bits
         * per channel sample = 0x8, 16 bits per channel
         * sample = 0x10
         *
         * (24) 36-37: Size of extra PCM parameters for
         * non-PCM formats.  If a PCM format code is
         * specified, this doesn't exist.
         *
         * Subchunk 3: Optional 'fact' subchunk for non-PCM formats
         * (26) 38-41: WAVE metadata magic 'fact' (0x 66 61 63 74)
         *
         * (30) 42-45: Length of 'fact' subchunk, not
         * including this field and the fact
         * identification field (usually 4)
         *
         * (34) 46-49: ??? sndrec32.exe outputs 0x 00 00 00 00
         * here.  See
         * http://www.health.uottawa.ca/biomech/csb/ARCHIVES/riff-for.txt
         *
         */

        memset(riffhdr, 0, sizeof(riffhdr));
        strncpy((char *)riffhdr, "fmt ", 4);
        // FIXME A bunch of the following only works for PCM
        // and mulaw/alaw, so you'll probably have to fix it
        // if you want to use one of the other formats.
        if(info.encoding < cdaStereo)
            setLong(riffhdr + 4, 18);
        else
            setLong(riffhdr + 4, 16);

        setShort(riffhdr + 8, 0x01); // default in case invalid encoding specified
        if(is_mono(info.encoding))
            setShort(riffhdr + 10, 1);
        else
            setShort(riffhdr + 10, 2);
        setLong(riffhdr + 12, info.rate);
        setLong(riffhdr + 16, (unsigned long)toBytes(info, info.rate));
        setShort(riffhdr + 20, (snd16_t)toBytes(info, 1));
        setShort(riffhdr + 22, 0);

        switch(info.encoding) {
        case pcm8Mono:
        case pcm8Stereo:
            setShort(riffhdr + 22, 8);
            pcm = 1;
            break;
        case pcm16Mono:
        case pcm16Stereo:
        case cdaMono:
        case cdaStereo:
            setShort(riffhdr + 22, 16);
            pcm = 1;
            break;
        case pcm32Mono:
        case pcm32Stereo:
            setShort(riffhdr + 22, 32);
            pcm = 1;
            break;
        case alawAudio:
            setShort(riffhdr + 8, 6);
            setShort(riffhdr + 22, 8);
            break;
        case mulawAudio:
            setShort(riffhdr + 8, 7);
            setShort(riffhdr + 22, 8);
            break;

            // FIXME I'm pretty sure these are supposed to
            // be writing to offset 22 instead of 24...
        case okiADPCM:
            setShort(riffhdr + 8, 0x10);
            setShort(riffhdr + 24, 4);
            break;
        case voxADPCM:
            setShort(riffhdr + 8, 0x17);
            setShort(riffhdr + 24, 4);
            break;
        case g721ADPCM:
            setShort(riffhdr + 8, 0x40);
            setShort(riffhdr + 24, 4);
            break;
        case g722Audio:
            setShort(riffhdr + 8, 0x64);
            setShort(riffhdr + 24, 8);
            break;
        case gsmVoice:
        case msgsmVoice:
            setShort(riffhdr + 8, 0x31);
            setShort(riffhdr + 24, 260);
            break;
        case g723_3bit:
            setShort(riffhdr + 8, 0x14);
            setShort(riffhdr + 24, 3);
            break;
        case g723_5bit:
            setShort(riffhdr + 8, 0x14);
            setShort(riffhdr + 24, 5);
        case unknownEncoding:
        default:
            break;
        }

        if(pcm == 0) {
            setShort(riffhdr + 24, 0);
            strncpy((char *)(riffhdr + 26), "fact", 4);
            setLong(riffhdr + 30, 4);
            setLong(riffhdr + 34, 0);
            if(afWrite(riffhdr, 38) != 38) {
                AudioFile::close();
                return;
            }
        }
        else {
            if(afWrite(riffhdr, 24) != 24) {
                AudioFile::close();
                return;
            }
        }

        /*
         * Subchunk 2: data subchunk
         *
         * Length: 8+
         *
         * (0) 36-39: data subchunk magic "data" (0x 64 61 74 61)
         *
         * (4) 40-43: subchunk 2 size =
         * NumSamples * NumChannels * (BitsPerSample / 8)
         * Note that this does not include the size of this
         * field and the previous one.
         *
         * (8) 44+: Samples
         */

        memset(riffhdr, 0, sizeof(riffhdr));
        strncpy((char *)riffhdr, "data", 4);
        memset(riffhdr + 4, 0xff, 4);
        if(afWrite(riffhdr, 8) != 8) {
            AudioFile::close();
            return;
        }

        header = getAbsolutePosition();
        length = getAbsolutePosition();
        break;

    case snd:
//      if(!info.order)
            info.order = __BIG_ENDIAN;

        if(!info.rate)
            info.rate = Audio::getRate(info.encoding);
        if(!info.rate)
            info.rate = rate8khz;

        strncpy((char *)aufile, ".snd", 4);
        if(info.annotation)
            setLong(aufile + 4, 24 + (unsigned long)strlen(info.annotation) + 1);
        else
            setLong(aufile + 4, 24);
        header = getLong(aufile + 4);
        setLong(aufile + 8, ~0l);
        switch(info.encoding) {
        case pcm8Stereo:
        case pcm8Mono:
            setLong(aufile + 12, 2);
            break;
        case pcm16Stereo:
        case pcm16Mono:
        case cdaStereo:
        case cdaMono:
            setLong(aufile + 12, 3);
            break;
        case pcm32Stereo:
        case pcm32Mono:
            setLong(aufile + 12, 5);
            break;
        case g721ADPCM:
            setLong(aufile + 12, 23);
            break;
        case g722Audio:
        case g722_7bit:
        case g722_6bit:
            setLong(aufile + 12, 24);
            break;
        case g723_3bit:
            setLong(aufile + 12, 25);
            break;
        case g723_5bit:
            setLong(aufile + 12, 26);
            break;
        case gsmVoice:
            setLong(aufile + 12, 28);
            break;
        case alawAudio:
            setLong(aufile + 12, 27);
            break;
        default:
            setLong(aufile + 12, 1);
        }
        setLong(aufile + 16, info.rate);
        if(is_mono(info.encoding))
            setLong(aufile + 20, 1);
        else
            setLong(aufile + 20, 2);
        if(afWrite(aufile, 24) != 24) {
            AudioFile::close();
            return;
        }
        if(info.annotation)
            afWrite((unsigned char *)info.annotation, (unsigned long)strlen(info.annotation) + 1);
        header = getAbsolutePosition();
        length = getAbsolutePosition();
        break;
    case mpeg:
        framing = 0;
        info.headersize = 4;    // no crc...
    case raw:
        break;
    }
    if(framing)
        info.setFraming(framing);
    else
        info.set();
}

void AudioFile::getWaveFormat(int request)
{
    unsigned char filehdr[24];
    int bitsize;
    int channels;

    if(request > 24)
        request = 24;

    if(!afPeek(filehdr, request)) {
        AudioFile::close();
        return;
    }
    channels = getShort(filehdr + 2);
    info.rate = getLong(filehdr + 4);

    switch(getShort(filehdr)) {
    case 1:
        bitsize = getShort(filehdr + 14);
        switch(bitsize) {
        case 8:
            if(channels > 1)
                info.encoding = pcm8Stereo;
            else
                info.encoding = pcm8Mono;
            break;
        case 16:
            if(info.rate == 44100) {
                if(channels > 1)
                    info.encoding = cdaStereo;
                else
                    info.encoding = cdaMono;
                break;
            }
            if(channels > 1)
                    info.encoding = pcm16Stereo;
            else
                info.encoding = pcm16Mono;
            break;
        case 32:
            if(channels > 1)
                info.encoding = pcm32Stereo;
            else
                info.encoding = pcm32Mono;
            break;
        default:
            info.encoding = unknownEncoding;
        }
        break;
    case 6:
        info.encoding = alawAudio;
        break;
    case 7:
        info.encoding = mulawAudio;
        break;
    case 0x10:
        info.encoding = okiADPCM;
        break;
    case 0x17:
        info.encoding = voxADPCM;
        break;
    case 0x40:
        info.encoding = g721ADPCM;
        break;
    case 0x65:
        info.encoding = g722Audio;
        break;
    case 0x31:
        info.encoding = msgsmVoice;
        break;
    case 0x14:
        bitsize = getLong(filehdr + 8) * 8 / info.rate;
        if(bitsize == 3)
            info.encoding = g723_3bit;
        else
            info.encoding = g723_5bit;
        break;
    default:
        info.encoding = unknownEncoding;
    }
}

void AudioFile::open(const char *name, Mode m, timeout_t framing)
{
    unsigned char filehdr[24];
    unsigned int count;
    char *ext;
    unsigned channels;
    mpeg_audio *mp3 = (mpeg_audio *)&filehdr;
    mode = m;


    while(!afOpen(name, m)) {
        if(mode == modeReadAny || mode == modeReadOne)
            name = getContinuation();
        else
            name = NULL;

        if(!name)
            return;
    }

    pathname = new char[strlen(name) + 1];
    strcpy(pathname, name);
    header = 0l;

    info.framesize = 0;
    info.framecount = 0;
    info.encoding = mulawAudio;
    info.format = raw;
    info.order = 0;
    ext = strrchr(pathname, '.');
    if(!ext)
        goto done;

    info.encoding = Audio::getEncoding(ext);
    switch(info.encoding) {
    case cdaStereo:
        info.order = __LITTLE_ENDIAN;
        break;
    case unknownEncoding:
        info.encoding = mulawAudio;
    default:
        break;
    }

    strcpy((char *)filehdr, ".xxx");

    if(!afPeek(filehdr, 24)) {
        AudioFile::close();
        return;
    }

    if(!strncmp((char *)filehdr, "RIFF", 4)) {
        info.format = riff;
        info.order = __LITTLE_ENDIAN;
    }

    if(!strncmp((char *)filehdr, "RIFX", 4)) {
        info.order = __BIG_ENDIAN;
        info.format = riff;
    }

    if(!strncmp((char *)filehdr + 8, "WAVE", 4) && info.format == riff) {
        header = 12;
        for(;;)
        {
            if(!afSeek(header)) {
                AudioFile::close();
                return;
            }
            if(!afPeek(filehdr, 8)) {
                AudioFile::close();
                return;
            }
            header += 8;
            if(!strncmp((char *)filehdr, "data", 4))
                break;

            count = getLong(filehdr + 4);
            header += count;
            if(!strncmp((char *)filehdr, "fmt ", 4))
                getWaveFormat(count);

        }
        afSeek(header);
        goto done;
    }
    if(!strncmp((char *)filehdr, ".snd", 4)) {
        info.format = snd;
        info.order = __BIG_ENDIAN;
        header = getLong(filehdr + 4);
        info.rate = getLong(filehdr + 16);
        channels = getLong(filehdr + 20);

        switch(getLong(filehdr + 12)) {
        case 2:
            if(channels > 1)
                info.encoding = pcm8Stereo;
            else
                info.encoding = pcm8Mono;
            break;
        case 3:
            if(info.rate == 44100) {
                if(channels > 1)
                    info.encoding = cdaStereo;
                else
                    info.encoding = cdaMono;
                break;
            }
            if(channels > 1)
                info.encoding = pcm16Stereo;
            else
                info.encoding = pcm16Mono;
            break;
        case 5:
            if(channels > 1)
                info.encoding = pcm32Stereo;
            else
                info.encoding = pcm32Mono;
            break;
        case 23:
            info.encoding = g721ADPCM;
            break;
        case 24:
            info.encoding = g722Audio;
            break;
        case 25:
            info.encoding = g723_3bit;
            break;
        case 26:
            info.encoding = g723_5bit;
            break;
        case 27:
            info.encoding = alawAudio;
            break;
        case 28:
            info.encoding = gsmVoice;
            break;
        case 1:
            info.encoding = mulawAudio;
            break;
        default:
            info.encoding = unknownEncoding;
        }
        if(header > 24) {
            info.annotation = new char[header - 24];
            afSeek(24);
            afRead((unsigned char *)info.annotation, header - 24);
        }
        goto done;
    }

    if(!strnicmp((char *)filehdr, "ID3", 3)) {
        afSeek(10);
        info.order = __BIG_ENDIAN;
        header = 10;
        if(filehdr[5] & 0x10)
            header += 10;   // footer

        header += (filehdr[9] & 0x7f);
        header += (filehdr[8] & 0x7f) * 128;
        header += (filehdr[7] & 0x7f) * 128 * 128;
        header += (filehdr[6] & 0x7f) * 128 * 128 * 128;
        afSeek(header);
        afRead(filehdr, 4);
        goto mp3;
    }

    if(mp3->mp_sync1 == 0xff && mp3->mp_sync2 == 0x07)
        goto mp3;
    else
        afSeek(0);

    goto done;

mp3:
    //framing = 0;
    afSeek(header);

    info.order = __BIG_ENDIAN;
    info.format = mpeg;
    mp3info(info, mp3);
    return;

done:
    info.headersize = 0;
    if(framing)
        info.setFraming(framing);
    else
        info.set();

    if(mode == modeFeed) {
        setPosition();
        iolimit = (unsigned long)toBytes(info, getPosition());
        setPosition(0);
    }
}

Audio::Info::Info()
{
    clear();
}

Audio::Info::Info(const Info& copy)
{
    format = copy.format;
    encoding = copy.encoding;
    rate = copy.rate;
    bitrate = copy.bitrate;
    order = copy.order;
    framesize = copy.framesize;
    framecount = copy.framecount;
    headersize = copy.headersize;
    padding = copy.padding;
    framing = copy.framing;
    annotation = copy.annotation;
}

void Audio::Info::clear(void)
{
    memset(this, 0, sizeof(Info));
}

void Audio::Info::setRate(Rate r)
{
    rate = getRate(encoding, r);
    set();
}

void Audio::Info::setFraming(timeout_t timeout)
{
    set();

    framing = getFraming(encoding);

    if(!timeout)
        return;

    if(framing) {
        timeout = (timeout / framing);
        if(!timeout)
            timeout = framing;
        else
            timeout = timeout * framing;
    }

    switch(timeout) {
    case 10:
    case 15:
    case 20:
    case 30:
    case 40:
        break;
    default:
        timeout = 20;
    }

    framing = timeout;
    framecount = (rate * framing) / 1000l;
    framesize = (unsigned)toBytes(encoding, framecount);
}

void Audio::Info::set(void)
{

    switch(encoding) {
    case mp1Audio:
        framecount = 384;
        framesize = (12 * bitrate / rate) * 4 + headersize + padding;
        return;
    case mp2Audio:
    case mp3Audio:
        framecount = 1152;
        framesize = (144 * bitrate / rate) + headersize + padding;
        return;
    default:
        break;
    }
    if(!framesize)
        framesize = getFrame(encoding);

    if(!framecount)
        framecount = getCount(encoding);

    if(!rate)
        rate = getRate(encoding);

    if(!bitrate && rate && framesize && framecount)
        bitrate = ((long)framesize * 8l * rate) / (long)framecount;
}


void AudioFile::close(void)
{
#ifndef _MSWINDOWS_
    struct stat ino;
#endif

    unsigned char buf[58];
    if(! is_open())
        return;

    if(mode != modeWrite) {
        afClose();
        clear();
        return;
    }

    if(! afSeek(0)) {
        afClose();
        clear();
        return;
    }

    if(-1 == afRead(buf, 58)) {
        afClose();
        clear();
        return;
    }

    afSeek(0);

    switch(info.format) {
    case riff:
    case wave:
#ifndef _MSWINDOWS_
        fstat(file.fd, &ino);
        length = ino.st_size;
#endif
        // RIFF header
        setLong(buf + 4, length - 8);

        // If it's a non-PCM datatype, the offsets are a bit
        // different for subchunk 2.
        switch(info.encoding) {
        case cdaStereo:
        case cdaMono:
        case pcm8Stereo:
        case pcm8Mono:
        case pcm16Stereo:
        case pcm16Mono:
        case pcm32Stereo:
        case pcm32Mono:
            setLong(buf + 40, length - header);
            break;
        default:
            setLong(buf + 54, length - header);
        }

        afWrite(buf, 58);
        break;
    case snd:
#ifndef _MSWINDOWS_
        fstat(file.fd, &ino);
        length = ino.st_size;
#endif
        setLong(buf + 8, length - header);
        afWrite(buf, 12);
        break;
    default:
        break;
    }
    afClose();
    clear();
}

void AudioFile::clear(void)
{
    if(pathname) {
        if(pathname)
            delete[] pathname;
        pathname = NULL;
    }
    if(info.annotation) {
        if(info.annotation)
            delete[] info.annotation;
        info.annotation = NULL;
    }
    minimum = 0;
    iolimit = 0;
}

Audio::Error AudioFile::getInfo(Info *infobuf)
{
    if(!is_open())
        return setError(errNotOpened);

    if(!infobuf)
        return setError(errRequestInvalid);

    memcpy(infobuf, &info, sizeof(info));
    return errSuccess;
}

Audio::Error AudioFile::setError(Error err)
{
    if(err)
        error = err;
    return err;
}

const char * AudioFile::getErrorStr(Error err) const
{
    return ErrorStrs[err];
}

Audio::Error AudioFile::setMinimum(unsigned long samples)
{
    if(!is_open())
        return setError(errNotOpened);
    minimum = samples;
    return errSuccess;
}

unsigned AudioFile::getLinear(Linear addr, unsigned samples)
{
    unsigned rts = 0;
    int count;
    AudioCodec *codec;

    if(!samples)
        samples = info.framecount;

    if(getEncoding() == pcm16Mono) {
        count = getNative((Encoded)addr, samples * 2);
        if(count < 0)
            return 0;
        return count / 2;
    }

    codec = getCodec();
    if(!codec)
        return 0;

    count = getCount(info.encoding);
    samples = (samples / count) * count;
    count = (int)toBytes(info, samples);

    Encoded buffer = new unsigned char[count];
    count = getBuffer(buffer, count);
    if(count < 1) {
        delete[] buffer;
        return 0;
    }

    samples = toSamples(info, count);
    rts = codec->decode(addr, buffer, samples);
    delete[] buffer;
    return rts;
}

unsigned AudioFile::putLinear(Linear addr, unsigned samples)
{
    int count;
    AudioCodec *codec;

    if(!samples)
        samples = info.framecount;

    if(getEncoding() == pcm16Mono) {
        count = putNative((Encoded)addr, samples * 2);
        if(count < 0)
            return 0;
        return count / 2;
    }
    codec = getCodec();
    if(!codec)
        return 0;

    count = getCount(info.encoding);
    samples = samples / count * count;
    count = (int)toBytes(info, samples);

    Encoded buffer = new unsigned char[count];

    samples = codec->encode(addr, buffer, samples);
    if(samples < 1) {
        delete[] buffer;
        return 0;
    }
    count = (int)toBytes(info, samples);
    count = putBuffer(buffer, count);
    delete[] buffer;
    if(count < 0)
        return 0;
    return toSamples(info, count);
}

ssize_t AudioFile::getBuffer(Encoded addr, size_t bytes)
{
    Info prior;
    char *fname;
    int count;
    unsigned long curpos, xfer = 0;
    mpeg_audio *mp3 = (mpeg_audio *)addr;

    if(!bytes && info.format == mpeg) {
rescan:
        count = afRead(addr, 4);
        if(count < 0)
            return count;
        if(count < 4)
            return 0;
        if(mp3->mp_sync1 != 0xff || mp3->mp_sync2 != 0x07) {
            afSeek(getAbsolutePosition() - 3);
            goto rescan;
        }
        mp3info(info, mp3);
        count = afRead(addr + 4, info.framesize - 4);
        if(count > 0)
            count += 4;
        return count;
    }

    if(!bytes)
        bytes = info.framesize;

    curpos = (unsigned long)toBytes(info, getPosition());
    if(curpos >= iolimit && mode == modeFeed) {
        curpos = 0;
        setPosition(0);
    }
    if (iolimit && ((curpos + bytes) > iolimit))
        bytes = iolimit - curpos;

    getInfo(&prior);

    for(;;)
    {
        count = afRead(addr, (unsigned)bytes);
        if(count < 0) {
            if(!xfer)
                return count;
            return xfer;
        }
        xfer += count;
        if(count == (int)bytes)
            return xfer;
        if(mode == modeFeed) {
            setPosition(0l);
            goto cont;
        }

retry:
        if(mode == modeReadOne)
            fname = NULL;
        else
            fname = getContinuation();

        if(!fname)
            return xfer;
        AudioFile::close();
        AudioFile::open(fname, modeRead, info.framing);
        if(!is_open()) {
            if(mode == modeReadAny)
                goto retry;
            return xfer;
        }

        if(prior.encoding != info.encoding) {
            AudioFile::close();
            return xfer;
        }
cont:
        bytes -= count;
        addr += count;
    }
}

Audio::Error AudioFile::getSamples(void *addr, unsigned request)
{
    char *fname;
    unsigned char *caddr = (unsigned char *)addr;
    int count, bytes;

    if(!request)
        request = info.framecount;

    for(;;)
    {
        bytes = (int)toBytes(info, request);
        if(bytes < 1)
            return setError(errRequestInvalid);
        count = afRead(caddr, bytes);
        if(count == bytes)
            return errSuccess;

        if(count < 0)
            return errReadFailure;

        if(count > 0) {
            caddr += count;
            count = request - toSamples(info.encoding, count);
        }
        else
            count = request;

        if(mode == modeFeed) {
            setPosition(0);
            request = count;
            continue;
        }

retry:
        if(mode == modeReadOne)
            fname = NULL;
        else
            fname = getContinuation();

        if(!fname)
            break;

        AudioFile::close();
        AudioFile::open(fname, modeRead);
        if(!is_open()) {
            if(mode == modeReadAny)
                goto retry;
            break;
        }

        request = count;
    }
    if(count)
        Audio::fill(caddr, count, info.encoding);
    return errReadIncomplete;
}

ssize_t AudioFile::putBuffer(Encoded addr, size_t len)
{
    int count;
    unsigned long curpos;
    mpeg_audio *mp3 = (mpeg_audio *)addr;

    if(!len && info.format == mpeg) {
        mp3info(info, mp3);
        len = info.framesize;
    }

    if(!len)
        len = info.framesize;

    curpos = (unsigned long)toBytes(info, getPosition());
    if(curpos >= iolimit && mode == modeFeed) {
        curpos = 0;
        setPosition(0);
    }

    if (iolimit && ((curpos + len) > iolimit))
        len = iolimit - curpos;

    if (len <= 0)
        return 0;

    count = afWrite((unsigned char *)addr, (unsigned)len);
    if(count == (int)len) {
        length += count;
        return count;
    }
    if(count < 1)
        return count;
    else {
        length += count;
        return count;
    }
}

Audio::Error AudioFile::putSamples(void *addr, unsigned samples)
{
    int count;
    int bytes;

    if(!samples)
        samples = info.framecount;

    bytes = (int)toBytes(info, samples);
    if(bytes < 1)
        return setError(errRequestInvalid);

    count = afWrite((unsigned char *)addr, bytes);
    if(count == bytes) {
        length += count;
        return errSuccess;
    }
    if(count < 1)
        return errWriteFailure;
    else {
        length += count;
        return errWriteIncomplete;
    }
}

Audio::Error AudioFile::skip(long samples)
{
    unsigned long orig = getPosition();
    unsigned long pos = orig + samples;

    setPosition(pos);
    if(orig < getPosition())
        length += (getPosition() - orig);

    return errSuccess;
}

Audio::Error AudioFile::setLimit(unsigned long samples)
{
    if(!is_open())
        return setError(errNotOpened);

    if(!samples) {
        iolimit = 0;
        return errSuccess;
    }

    samples += getPosition();
    iolimit = (unsigned long)toBytes(info, samples);
    return errSuccess;
}

bool AudioFile::is_signed(void) const
{
    switch(info.format) {
    case snd:
        return false;
    default:
        break;
    }

    switch(info.encoding) {
    case pcm8Mono:
    case pcm8Stereo:
    case pcm16Mono:
    case pcm16Stereo:
    case pcm32Mono:
    case pcm32Stereo:
        return true;
    default:
        return false;
    }
}

Audio::Error AudioFile::position(const char *timestamp)
{
    timeout_t pos = toTimeout(timestamp);

    pos /= info.framing;
    return setPosition(pos * info.framecount);
}

void AudioFile::getPosition(char *timestamp, size_t size)
{
    timeout_t pos = getAbsolutePosition();

    pos /= info.framecount;
    toTimestamp(pos * info.framing, timestamp, size);
}

} // namespace ucommon
