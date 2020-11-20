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
#include <ucommon/export.h>
#include <ccaudio2.h>

namespace ucommon {

AudioBuffer::AudioBuffer(Info *i, size_t sz) :
AudioBase(i)
{
    size = sz;
    buf = new char[size];
    start = len = 0;
}

AudioBuffer::~AudioBuffer()
{
    if(buf)
        delete[] buf;
    buf = NULL;
}

ssize_t AudioBuffer::get(Encoded data, size_t amount)
{
    size_t left, copied;

    if(amount == 0)
        return 0;

    mutex.lock();
    if(len == 0) {
        memset(data, 0, amount);
        mutex.release();
        return (ssize_t)amount;
    }

    if(amount > len)
        memset(data + len, 0, amount - len);

    left = ((amount) < (len) ? amount : len);
    if((start + left) > size) {
        copied = size - start;
        memcpy(data, buf + start, copied);
        data += copied;
        left -= copied;
        len -= copied;
        start = 0;
    }

    if(left > 0) {
        memcpy(data, buf + start, left);
        len -= left;
        start = (start + left) % size;
    }
    mutex.release();
    return (ssize_t)amount;
}

ssize_t AudioBuffer::put(Encoded data, size_t amount)
{
    size_t nl, written, removed, offset;

    if(amount == 0)
        return 0;

    mutex.lock();
    nl = len + amount;
    if(len > size) {
        removed = nl - size;
        start = (start + removed) % size;
        len -= removed;
    }

    offset = (start + len) % size;
    if((offset + amount) > size) {
        written = size - offset;
        memcpy(buf + offset, data, written);
        offset = 0;
        data += written;
        amount -= written;
        len += written;
    }

    if(amount > 0) {
        memcpy(buf + offset, data, amount);
        len += amount;
    }
    mutex.release();
    return (ssize_t)amount;
}

} // namespace ucommon


