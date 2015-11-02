// vim: set fenc=utf-8 :

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#include <oauth.h>
#include <curl/curl.h>
#include <ncurses.h>
#include <jansson.h>

#include "include/tweet.h"
#include "mykey.h"

oauth_keys mykeys = {NULL, NULL, NULL, NULL};

int init(void) {
	mykeys = (oauth_keys){c_key, c_sec, t_key, t_sec};
	bear_init(mykeys);

	initscr();
	cbreak();
	noecho();

	return 0;
}

int fini(void) {
	bear_cleanup();

	endwin();

	return 0;
}

int main(void) {

	setlocale(LC_CTYPE,"");
	init();
	border(0x7c, 0x7c, 0x2d ,0x2d, 0x2b, 0x2b ,0x2b, 0x2b);
	char *res = NULL;
	int ret = get_statuses_user_timeline(&res, 0, "amama_bot", 0, 0, 0, -1, -1, -1, -1);
	mvaddch(1,1,ret + 0x30);

	json_error_t json_err;
	json_t *result = json_loads(res, 0, &json_err);

	json_t *tweet;
	int i = 0;
	json_array_foreach(result, i, tweet) {
		char const *key;
		json_t *value;
		json_object_foreach(tweet, key, value) {
			switch(json_typeof(value)) {
				case JSON_NULL:
				case JSON_FALSE:
				case JSON_TRUE:
				break;

				case JSON_STRING: {
					char const *str = json_string_value(value);
					printw("%s:%s\n", key, str);
					break;
				}

				default:
				printw("%s:%s\n",key,((char const*[JSON_NULL + 1]){"json_object", "json_array", "json_string", "json_integer", "json_real", "json_true", "json_false", "json_null"})[json_typeof(value)]);
				//break;
			}
		}
		break;
	}

	getch();
	json_decref(result);
	fini();
	return 0;
}

