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
    led_str_foreach_uchar(&test) {
        led_str_decl(schar, 5);
        led_str_app_uchar(&schar, foreach.c);
        led_debug("%d %d %s",foreach.i, foreach.n, led_str_str(&schar));
    }
    //led_assert(led_str_equal_str(&test, "test="), LED_ERR_INTERNAL, "test_led_str_foreach_uchar");
}

void test_led_str_foreach_uchar_zone() {
    led_str_decl(test, 16);
    led_str_app_str(&test,"ÂBCDÊF");
    led_debug("%s", test.str);
    led_str_foreach_uchar_zone(&test,3,6) {
        led_str_decl(schar, 5);
        led_str_app_uchar(&schar, foreach.c);
        led_debug("%d %d %s",foreach.i, foreach.n, led_str_str(&schar));
    }
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
//-----------------------------------------------
// LEDTEST main
//-----------------------------------------------

int main(int , char* []) {
    led.opt.verbose = true;
    test(test_led_str_app);
    test(test_led_str_uchar_last);
    test(test_led_str_trunk_uchar);
    test(test_led_str_foreach_uchar);
    test(test_led_str_foreach_uchar_zone);
    test(test_led_str_cut_next);
    return 0;
}
