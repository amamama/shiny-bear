// vim: set fenc=utf-8 :

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <oauth.h>
#include <curl/curl.h>
#include <ncurses.h>

#include "include/tweet.h"
#include "keys.h"

int init(void) {
	bear_init(&(union KEYS){.keys_array = {c_key, c_sec, t_key, t_sec}});

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

int main(void)
{
	init();
	box(stdscr, 0, 0);
	getch();
	fini();
	return 0;
}

