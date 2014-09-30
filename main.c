// vim: set fenc=utf-8 :

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <oauth.h>
#include <curl/curl.h>

#include "include/tweet.h"

int main(void)
{
	char *rep = NULL;
	bear_init(&(union KEYS){.keys_array = {c_key, c_sec, t_key, t_sec}});
	printf("%d\n",post_statuses_update("status/update", &rep, 0, 0, (struct GEOCODE){0, 0, 0, ""}, 0, -1, -1));
	if(rep) {puts(rep);}
	//free(rep);
	//rep = NULL;
	printf("%d\n",get_statuses_home_timeline(&rep, 5, 0, 0, -1, -1, -1, -1) );
	if (rep){puts(rep);}
	free(rep);
	rep = NULL;
	bear_cleanup();
	return 0;
}

