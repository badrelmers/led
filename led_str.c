/***************************************************************************
 Copyright (C) 2024 - Olivier ROUITS <olivier.rouits@free.fr>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 USA
 ***************************************************************************/

#include "led.h"

//-----------------------------------------------
// LED str functions
//-----------------------------------------------

led_str_t* led_str_init(led_str_t* lstr, char* buf, size_t size) {
    lstr->str = buf;
    if (!lstr->str) {
        lstr->len = 0;
        lstr->size = 0;
    }
    else {
        lstr->len = strlen(buf);
        lstr->size = size > 0 ? size : lstr->len + 1;
    }
    return lstr;
}

pcre2_code* LED_REGEX_ALL_LINE = NULL;
pcre2_code* LED_REGEX_BLANK_LINE = NULL;
pcre2_code* LED_REGEX_INTEGER = NULL;
pcre2_code* LED_REGEX_REGISTER = NULL;
pcre2_code* LED_REGEX_FUNC = NULL;
pcre2_code* LED_REGEX_FUNC2 = NULL;

void led_regex_init() {
    if (LED_REGEX_ALL_LINE == NULL) LED_REGEX_ALL_LINE = led_regex_compile("^.*$");
    if (LED_REGEX_BLANK_LINE == NULL) LED_REGEX_BLANK_LINE = led_regex_compile("^[ \t]*$");
    if (LED_REGEX_INTEGER == NULL) LED_REGEX_INTEGER = led_regex_compile("^[0-9]+$");
    if (LED_REGEX_REGISTER == NULL) LED_REGEX_REGISTER = led_regex_compile("\\$R[0-9]?");
    if (LED_REGEX_FUNC == NULL) LED_REGEX_FUNC = led_regex_compile("^[a-z0-9_]+/");
    if (LED_REGEX_FUNC2 == NULL) LED_REGEX_FUNC2 = led_regex_compile("^[a-z0-9_]+:");
}

void led_regex_free() {
    if (LED_REGEX_ALL_LINE != NULL) { pcre2_code_free(LED_REGEX_ALL_LINE); LED_REGEX_ALL_LINE = NULL; }
    if (LED_REGEX_BLANK_LINE != NULL) { pcre2_code_free(LED_REGEX_BLANK_LINE); LED_REGEX_BLANK_LINE = NULL; }
    if (LED_REGEX_INTEGER != NULL) { pcre2_code_free(LED_REGEX_INTEGER); LED_REGEX_INTEGER = NULL; }
    if (LED_REGEX_REGISTER != NULL) { pcre2_code_free(LED_REGEX_REGISTER); LED_REGEX_REGISTER = NULL; }
    if (LED_REGEX_FUNC != NULL) { pcre2_code_free(LED_REGEX_FUNC); LED_REGEX_FUNC = NULL; }
    if (LED_REGEX_FUNC2 != NULL) { pcre2_code_free(LED_REGEX_FUNC2); LED_REGEX_FUNC2 = NULL; }
}

pcre2_code* led_regex_compile(const char* pattern) {
    int pcre_err;
    PCRE2_SIZE pcre_erroff;
    PCRE2_UCHAR pcre_errbuf[256];
    led_assert(pattern != NULL, LED_ERR_ARG, "Missing regex");
    pcre2_code* regex = pcre2_compile(
        (PCRE2_SPTR)pattern,
        PCRE2_ZERO_TERMINATED,
        PCRE2_UTF,
        &pcre_err,
        &pcre_erroff,
        NULL);
    pcre2_get_error_message(pcre_err, pcre_errbuf, sizeof(pcre_errbuf));
    led_assert(regex != NULL, LED_ERR_PCRE, "Regex error \"%s\" offset %d: %s", pattern, pcre_erroff, pcre_errbuf);
    return regex;
}

bool led_str_match(led_str_t* lstr, pcre2_code* regex) {
    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(regex, NULL);
    int rc = pcre2_match(regex, (PCRE2_SPTR)lstr->str, lstr->len, 0, 0, match_data, NULL);
    pcre2_match_data_free(match_data);
    return rc > 0;
}

bool led_str_match_offset(led_str_t* lstr, pcre2_code* regex, size_t* pzone_start, size_t* pzone_stop) {
    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(regex, NULL);
    int rc = pcre2_match(regex, (PCRE2_SPTR)lstr->str, lstr->len, 0, 0, match_data, NULL);
    led_debug("led_str_match_offset: rc=%d ", rc);
    if( rc > 0) {
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
        int iv = (rc - 1) * 2;
        *pzone_start = ovector[iv];
        *pzone_stop = ovector[iv + 1];
        led_debug("led_str_match_offset: offset start=%d char=%c stop=%d char=%c", *pzone_start, lstr->str[*pzone_start], *pzone_stop, lstr->str[*pzone_stop]);
    }
    pcre2_match_data_free(match_data);
    return rc > 0;
}

led_str_t* led_str_cut_next(led_str_t* lstr, led_uchar_t uchar, led_str_t* stok) {
    led_str_clone(stok, lstr);
    led_str_foreach_uchar(lstr) {
        // led_debug("led_str_cut_next - i=%u c=%x l=%u", i, c, l);
        if ( foreach.c == uchar ) {
            stok->str[foreach.i] = '\0';
            stok->len = foreach.i;
            stok->size = foreach.n;
            lstr->str += foreach.n;
            lstr->len -= foreach.n;
            lstr->size -= foreach.n;
            // led_debug("led_str_cut_next - lstr=%s tok=%s", lstr->str, stok->str);
            return lstr;
        }
    }
    lstr->str = lstr->str + lstr->len;
    lstr->len = 0;
    lstr->size = 0;
    return lstr;
}

//-----------------------------------------------
// LED utf8 functions
//-----------------------------------------------

size_t const led_uchar_size_table[] = {
    1,1,1,1,1,1,1,1,0,0,0,0,2,2,3,4
};

// bool led_uchar_isvalid(led_uchar_t c)
// {
//   if (c <= 0x7F) return true;

//   if (0xC280 <= c && c <= 0xDFBF)
//      return ((c & 0xE0C0) == 0xC080);

//   if (0xEDA080 <= c && c <= 0xEDBFBF)
//      return 0; // Reject UTF-16 surrogates

//   if (0xE0A080 <= c && c <= 0xEFBFBF)
//      return ((c & 0xF0C0C0) == 0xE08080);

//   if (0xF0908080 <= c && c <= 0xF48FBFBF)
//      return ((c & 0xF8C0C0C0) == 0xF0808080);

//   return false;
// }

// led_uchar_t led_uchar_encode(uint32_t code) {
//     led_uchar_t uc = code;
//     if (code > 0x7F) {
//         uc =  (code & 0x000003F)
//         | (code & 0x0000FC0) << 2
//         | (code & 0x003F000) << 4
//         | (code & 0x01C0000) << 6;

//         if      (code < 0x0000800) uc |= 0x0000C080;
//         else if (code < 0x0010000) uc |= 0x00E08080;
//         else                       uc |= 0xF0808080;
//     }
//     return uc;
// }

// uint32_t led_uchar_decode(led_uchar_t c) {
//   uint32_t mask;
//   if (c > 0x7F) {
//     mask = (c <= 0x00EFBFBF) ? 0x000F0000 : 0x003F0000 ;
//     c = ((c & 0x07000000) >> 6) |
//         ((c & mask )      >> 4) |
//         ((c & 0x00003F00) >> 2) |
//          (c & 0x0000003F);
//   }
//   return c;
// }

size_t led_uchar_from_str(char* str, led_uchar_t* uchar) {
    size_t l = led_uchar_size_str(str);
    // led_debug("led_uchar_from_str - len=%lu", l);
    led_uchar_t c = 0;
    for (size_t i = 0; i < l && str[i]; i++)
        c = (c << 8) | ((uint8_t*)str)[i];
    // led_debug("led_uchar_from_str - c=%x", c);
    *uchar = c;
    return l;
}

size_t led_uchar_from_rstr(char* str, size_t len, led_uchar_t* uchar) {
    size_t uchar_len = 0;
    led_uchar_t c = 0;
    while ( len > 0 ) {
        len--;
        c = c | (((uint8_t*)str)[len] << (uchar_len*8));
        uchar_len++;
        // led_debug("led_uchar_from_rstr - len=%lu c=%x", uchar_len, c);
        if ( !led_uchar_iscont(str[len]) ) {
            *uchar = c;
            return uchar_len;
        }
    }
    return 0;
}

size_t led_uchar_to_str(char* str, led_uchar_t uchar) {
    uint32_t mask = 0xFF000000;
    size_t l = 0;
    for (size_t i = 0; i < 4; i++) {
        uint8_t c = (uchar & mask) >> ((3-i)*8);
        // led_debug("led_uchar_to_str - uchar&mask=%x shift=%x", (uchar & mask), c);
        if (c) {
            *((uint8_t*)str++) = c;
            l++;
        }
        mask >>= 8;
    }
    // led_debug("led_uchar_to_str - %x => %x %x %x %x", uchar, (uint8_t)str[0], (uint8_t)str[1], (uint8_t)str[2], (uint8_t)str[3] );
    return l;
}
