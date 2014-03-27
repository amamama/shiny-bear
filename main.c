// vim: set fenc=utf-8 :

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <oauth.h>
#include <curl/curl.h>

#include "tweet.h"

int main(void)
{
	char *rep = NULL;
	bear_init(&(union KEYS){.keys_array = {"xf4aQHHSuCSq3uLZPtGdA", "WE4QvfbK0gaoFGATUScyDaHC0PmS8uACb7GyjigD4", "400441143-sP6sFupmd9tlYrXHtaVsuXBykWvgoVyyHogBJ3Ph", "UpEDheB2CPRPv8osP0AIcs2wlWxDdUzetuoaelnFXDLWq"}});
	char str[400] = {0};
	time_t t = time(NULL);
	int day = 60*60*24;
	snprintf(str, sizeof(str), "センターまであと%ld日（多分）\n国立大学前期試験まであと%ld日（多分）", (1421420400 - t)/day/*2015/1/17 00:00:00*/, (1424790000 - t)/day/*2015/2/25 00:00:00*/);
	curl_global_init(CURL_GLOBAL_DEFAULT);
	post_statuses_update(str, &rep, 0, 0, (struct GEOCODE){0, 0, 0, ""}, 0, -1, -1);
	free(rep);rep = NULL;
	bear_cleanup();
	return 0;
}

