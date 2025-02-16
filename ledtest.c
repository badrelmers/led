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
#pragma GCC diagnostic ignored "-Wmultichar"

#define test(NAME) printf("-- %s...\n", #NAME);NAME();printf("OK\n")

void test_led_uchar() {
    char buf[5] = "";
    led_uchar_to_str(buf, 'â');
    led_uchar_t uchar = led_uchar_of_str(buf);
    led_debug("native char: x=%X s=%s lc=%lc, ls=%lu, luc=%lu", 'â', "â", L'â', led_uchar_size_str("â"), led_uchar_size('â'));
    led_debug("led_uchar:   x=%X s=%s lc=%lc, ls=%lu, luc=%lu", uchar, buf, uchar, led_uchar_size_str(buf), led_uchar_size(uchar));
}

void test_led_uchar_isin() {
    led_assert(led_uchar_isin('t', "a testAâ"), LED_ERR_INTERNAL, "led_uchar_isin");
    led_assert(led_uchar_isin('â', "a testAâ"), LED_ERR_INTERNAL, "led_uchar_isin");
    led_assert(!led_uchar_isin('b', "a testAâ"), LED_ERR_INTERNAL, "led_uchar_isin");
}

void test_led_str_app() {
    led_str_decl(test, 16);
    led_str_app_str(&test,"a test");
    led_str_app_uchar(&test, 'A');
    led_str_app_uchar(&test, 'â');
    led_debug("%s",led_str_str(&test));
    led_assert(led_str_equal_str(&test, "a testAâ"), LED_ERR_INTERNAL, "test_led_str_app");
}

void test_led_str_uchar_last() {
    led_str_decl(test, 16);
    led_str_app_str(&test,"test=à");
    led_uchar_t c = led_str_uchar_last(&test);
    led_str_app_uchar(&test, c);
    led_debug("%s",led_str_str(&test));
    led_assert(led_str_equal_str(&test, "test=àà"), LED_ERR_INTERNAL, "test_led_str_uchar_last");
}

void test_led_str_trunk_uchar() {
    led_str_decl(test, 16);
    led_str_app_str(&test,"test=àa");
    led_str_trunk_uchar(&test, 'à');
    led_debug("%s",led_str_str(&test));
    led_str_trunk_uchar(&test, 'a');
    led_debug("%s",led_str_str(&test));
    led_str_trunk_uchar(&test, 'à');
    led_debug("%s",led_str_str(&test));
    led_assert(led_str_equal_str(&test, "test="), LED_ERR_INTERNAL, "test_led_str_trunk_uchar");
}

void test_led_str_foreach_uchar() {
    led_str_decl(test, 16);
    led_str_app_str(&test,"ÂBCDÊF");
    led_debug("%s", test.str);
    size_t i = 0;
    led_str_foreach_uchar(&test) {
        led_str_decl(schar, 5);
        led_str_app_uchar(&schar, foreach.uc);
        i = foreach.uc_count;
        led_debug("%lu %lu %lu %s",foreach.i, foreach.i_next, foreach.uc_count, led_str_str(&schar));
    }
    led_assert(i == 6, LED_ERR_INTERNAL, "test_led_str_foreach_uchar_r: error in count");
    led_debug("with zone");
    led_str_foreach_uchar_zone(&test,3,6) {
        led_str_decl(schar, 5);
        led_str_app_uchar(&schar, foreach.uc);
        i = foreach.uc_count;
        led_debug("%lu %lu %lu %s",foreach.i, foreach.i_next, foreach.uc_count, led_str_str(&schar));
    }
    led_assert(i == 3, LED_ERR_INTERNAL, "test_led_str_foreach_uchar_r: error in count");
}

void test_led_str_foreach_uchar_r() {
    led_str_decl_str(test, "ÂBCDÊF");
    led_debug("%s last is %lu", test.str, led_str_pos_uchar_prev(&test, led_str_len(&test)));

    size_t i = 0;
    led_str_foreach_uchar_r(&test) {
        led_debug("%lu %lu %lu %c",foreach.i, foreach.i_next, foreach.uc_count, foreach.uc);
        i = foreach.uc_count;
    }
    led_assert(i == 6, LED_ERR_INTERNAL, "test_led_str_foreach_uchar_r: error in count");
    led_debug("with zone");
    led_str_foreach_uchar_zone_r(&test, 2, 5) {
        led_debug("%lu %lu %lu %c",foreach.i, foreach.i_next, foreach.uc_count, foreach.uc);
        i = foreach.uc_count;
    }
    led_assert(i == 3, LED_ERR_INTERNAL, "test_led_str_foreach_uchar_r: error in count");
    //led_assert(led_str_equal_str(&test, "test="), LED_ERR_INTERNAL, "test_led_str_foreach_uchar_zone");
}

void test_led_str_cut_next() {
    led_str_decl(test, 256);
    led_str_app_str(&test, "chara/charà/charÂ");
    led_str_t tok;

    led_debug("%s",led_str_str(&test));
    led_str_cut_next(&test, '/', &tok);
    led_debug("%s -> tok=%s",led_str_str(&test), led_str_str(&tok));
    led_assert(led_str_equal_str(&test, "charà/charÂ"), LED_ERR_INTERNAL, "test_led_str_cut_next");
    led_assert(led_str_equal_str(&tok, "chara"), LED_ERR_INTERNAL, "test_led_str_cut_next");

    led_str_cpy_str(&test, "char1àchar2àchar3Â");
    led_str_cut_next(&test, 'à', &tok);
    led_debug("%s -> tok=%s",led_str_str(&test), led_str_str(&tok));
    led_assert(led_str_equal_str(&test, "char2àchar3Â"), LED_ERR_INTERNAL, "test_led_str_cut_next");
    led_assert(led_str_equal_str(&tok, "char1"), LED_ERR_INTERNAL, "test_led_str_cut_next");
}

void test_led_str_startswith() {
    led_str_decl_str(test1, "â short test");
    led_str_decl_str(test2, "â short");
    led_str_decl_str(test3, "â lông");
    led_str_decl_str(test4, "â short test ");
    led_debug("smaller");
    led_assert(led_str_startswith(&test1, &test2), LED_ERR_INTERNAL, "test_led_str_startswith");
    led_debug("smaller different");
    led_assert(!led_str_startswith(&test1, &test3), LED_ERR_INTERNAL, "test_led_str_startswith (not)");
    led_debug("equal");
    led_assert(led_str_startswith(&test1, &test1), LED_ERR_INTERNAL, "test_led_str_startswith (equal)");
    led_debug("longer");
    led_assert(!led_str_startswith(&test1, &test4), LED_ERR_INTERNAL, "test_led_str_startswith (longer)");
}

void test_led_str_startswith_str() {
    led_str_decl_str(test1, "â short test");
    led_debug("smaller");
    led_assert(led_str_startswith_str(&test1, "â short"), LED_ERR_INTERNAL, "test_led_str_startswith_str");
    led_debug("smaller different");
    led_assert(!led_str_startswith_str(&test1, "â lông"), LED_ERR_INTERNAL, "test_led_str_startswith_str (not)");
    led_debug("equal");
    led_assert(led_str_startswith_str(&test1, "â short test"), LED_ERR_INTERNAL, "test_led_str_startswith_str (equzl)");
    led_debug("longer");
    led_assert(!led_str_startswith_str(&test1, "â short test "), LED_ERR_INTERNAL, "test_led_str_startswith_str (longer)");
}


void test_led_str_find_uchar() {
    led_str_decl_str(test1, "â short tëst");
    size_t idx;

    idx = led_str_find_uchar(&test1, 'â');
    led_debug("present %lu", idx);
    led_assert(idx == 0, LED_ERR_INTERNAL, "test_led_str_find_uchar");

    idx = led_str_find_uchar(&test1, 'ë');
    led_debug("present %lu", idx);
    led_assert(idx == 10, LED_ERR_INTERNAL, "test_led_str_find_uchar");

    idx = led_str_find_uchar(&test1, 'k');
    led_debug("absent %lu", idx);
    led_assert(idx == led_str_len(&test1), LED_ERR_INTERNAL, "test_led_str_find_uchar (not)");
}

//-----------------------------------------------
// LEDTEST main
//-----------------------------------------------

int main(int , char* []) {
    setlocale(LC_ALL, "");
    led.opt.verbose = true;

    test(test_led_uchar);
    test(test_led_uchar_isin);
    test(test_led_str_app);
    test(test_led_str_uchar_last);
    test(test_led_str_trunk_uchar);
    test(test_led_str_foreach_uchar);
    test(test_led_str_foreach_uchar_r);
    test(test_led_str_cut_next);
    test(test_led_str_startswith);
    test(test_led_str_startswith_str);
    test(test_led_str_find_uchar);
    return 0;
}
