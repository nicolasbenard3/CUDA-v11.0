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

#define MAP_HASH_SIZE   197
#define MAP_PAGE_COUNT  255
#define MAP_PAGE_SIZE (sizeof(void *[MAP_PAGE_COUNT]))
#define MAP_PAGE_FIX (MAP_PAGE_SIZE / MAP_PAGE_COUNT)

namespace ucommon {

static unsigned char *page = NULL;
static unsigned used = MAP_PAGE_SIZE;
static TelTone::tonekey_t *hash[MAP_HASH_SIZE];

static unsigned key(const char *id)
{
    unsigned val = 0;
    while(*id)
        val = (val << 1) ^ (*(id++) & 0x1f);

    return val % MAP_HASH_SIZE;
}

static void *map(unsigned len)
{
    unsigned char *pos;
    unsigned fix = len % MAP_PAGE_FIX;


    if(fix)
        len += MAP_PAGE_FIX - fix;

    if(used + len > MAP_PAGE_SIZE) {
        page = (unsigned char *)(new void *[MAP_PAGE_COUNT]);
        used = 0;
    }

    pos = page + used;
    used += len;
    return pos;
}

TelTone::tonekey_t *TelTone::find(const char *id, const char *locale)
{
    unsigned k;
    tonekey_t *tk;
    char namebuf[65];
    char env[32];
    char *cp, *ep;

    if(locale == NULL) {
        locale = getenv("LANG");
        if(!locale)
            locale="us";

        snprintf(env, sizeof(env), "%s", locale);
        ep = strchr(env, '.');
        if(ep)
            *ep = 0;
        cp = strchr(env, '_');
        if(cp)
            locale = ++cp;
        else
            locale = env;
    }

    snprintf(namebuf, sizeof(namebuf), "%s.%s", locale, id);
    k = key(namebuf);
    tk = hash[k];

    while(tk) {
        if(!stricmp(namebuf, tk->id))
            break;
        tk = tk->next;
    }
    return tk;
}


bool TelTone::load(const char *path, const char *l)
{
    char buffer[256];
    char locale[256];
    char *loclist[128], *cp, *ep, *name;
    char *lists[64];
    char **field, *freq, *fdur, *fcount;
    tonedef_t *def, *first, *again, *last, *final = NULL;
    tonekey_t *tk;
    unsigned count, i, k;
    unsigned lcount = 0;
    FILE *fp;
    char namebuf[65];
    bool loaded = false;

    fp = fopen(path, "r");
    if(!fp)
        return false;

    memset(&hash, 0, sizeof(hash));

    for(;;)
    {
        if(!fgets(buffer, sizeof(buffer) - 1, fp) || feof(fp))
            break;
        cp = buffer;
        while(isspace(*cp))
            ++cp;

        if(*cp == '[') {
            if(loaded)
                break;
            strcpy(locale, buffer);
            cp = locale;
            lcount = 0;
            cp = strtok(cp, "[]|\r\n");
            while(cp) {
                if(*cp && !l)
                    loclist[lcount++] = cp;
                else if(*cp && !stricmp(cp, l))
                {
                    loclist[lcount++] = cp;
                    loaded = true;
                }
                cp = strtok(NULL, "[]|\r\n");
            }
            continue;
        }

        if(!isalpha(*cp) || !lcount)
            continue;

        ep = strchr(cp, '=');
        if(!ep)
            continue;
        *(ep++) = 0;
        name = strtok(cp, " \t");
        cp = strchr(ep, ';');
        if(cp)
            *cp = 0;
        cp = strchr(ep, '#');
        if(cp)
            *cp = 0;
        cp = strtok(ep, ",");
        count = 0;
        while(cp) {
            while(isspace(*cp))
                ++cp;

            lists[count++] = cp;
            cp = strtok(NULL, ",");
        }
        if(!count)
            continue;

        field = &lists[0];
        first = last = again = NULL;
        while(count--) {
            freq = strtok(*field, " \t\r\n");
            fdur = strtok(NULL, " \t\r\n");
            fcount = strtok(NULL, " \t\r\n");

            if(!freq)
                goto skip;

            freq = strtok(freq, " \r\r\n");

            if(isalpha(*freq)) {
                tk = find(freq, loclist[0]);
                if(tk) {
                    if(!first)
                        first = tk->first;
                    else {
                        final = tk->last;
                        again = tk->first;
                    }
                }
                break;
            }

            def = (tonedef_t *)map(sizeof(tonedef_t));
            memset(def, 0, sizeof(tonedef_t));
            if(!first)
                first = def;
            else
                last->next = def;

            last = final = def;

            def->next = NULL;

            if(!fdur || !atol(fdur)) {
                again = def;
                count = 0;
            }
            else if((!fcount || !atoi(fcount)) && !count)
                again = first;

            cp = strtok(freq, " \t\r\n");
            ep = cp;
            while(isdigit(*ep))
                ++ep;
            def->f1 = atoi(cp);
            if(*ep)
                def->f2 = atoi(++ep);
            else
                def->f2 = 0;

            if(!fcount)
                fcount = (char *)"1";

            def->count = atoi(fcount);
            if(!def->count)
                ++def->count;

            if(!fdur)
                goto skip;

            cp = strtok(fdur, " \t\r\n");
            ep = cp;
            while(isdigit(*ep))
                ++ep;
            def->duration = atol(cp);
            if(*ep)
                def->silence = atol(++ep);
            else
                def->silence = 0;

skip:
            ++field;
        }
        if(last)
            last->next = again;
        field = &loclist[0];
        i = lcount;
        while(i--) {
            snprintf(namebuf, sizeof(namebuf), "%s.%s",
                *(field++), name);
            tk = (tonekey_t *)map((unsigned)(sizeof(tonekey_t) + strlen(namebuf)));
            strcpy(tk->id, namebuf);
            tk->first = first;
            tk->last = final;
            k = key(namebuf);
            tk->next = hash[k];
            hash[k] = tk;
        }
    }

    fclose(fp);
    if(page)
        return true;
    return false;
}

} // namespace ucommon
