// vim: set foldmethod=syntax :
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <oauth.h>
#include <curl/curl.h>

#include "tweet.h"
#include "../utils/utils.h"

oauth_keys user_keys = {NULL, NULL, NULL, NULL};
oauth_keys register_keys(oauth_keys keys) {
	return user_keys = keys;
}

int check_keys(void) {
	return user_keys.c_key&&user_keys.c_sec&&user_keys.t_key&&user_keys.t_sec;
}

oauth_keys current_keys(void) {
	return user_keys;
}

int bear_init(oauth_keys keys) {
	register_keys(keys);
	return curl_global_init(CURL_GLOBAL_DEFAULT);
}

int bear_cleanup(void) {
	curl_global_cleanup();
	return 0;
}

static size_t write_data(char *buffer, size_t const size, size_t const nmemb, void *rep) {
	*(buffer + size * nmemb) = '\0';
	alloc_strcat((char**)rep, buffer);
	return size * nmemb;
}

static int http_request(char const *u, int const p, char **rep) {
	dbg_printf("\n");

	if (rep && *rep) {
		memset(*rep, 0, strlen(*rep));
	}
	CURL *curl = curl_easy_init();
	CURLcode ret;
	if (!curl) {
		fprintf(stderr, "failed to initialize curl\n");
	}
	char *request = NULL;
	char *post = NULL;
		request = oauth_sign_url2(u, p?&post:NULL, OA_HMAC, NULL, user_keys.c_key, user_keys.c_sec, user_keys.t_key, user_keys.t_sec);
	if (p) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *) post);
	}
	curl_easy_setopt(curl, CURLOPT_URL, request);
	//is it good? i dont know.
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	if (rep) {
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) rep);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	}

	ret = curl_easy_perform (curl);
	if (ret != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror (ret));
	}
	free_and_assign(request);
	free_and_assign(post);
	curl_easy_cleanup(curl);

	return ret;
}

char const *api_uri_1_1 = "https://api.twitter.com/1.1/";

char const * api_uri[] = {
	#define uri(cst, str) [cst] = #str,
	#include "api_uri.h"
	#undef uri
};

inline static char **add_que_or_amp(api_enum const api, char **uri) {
	alloc_strcat(uri, strlen(*uri)==(strlen(api_uri_1_1) + strlen(api_uri[api]))?"?":"&");
	return uri;
}

#define add_args(type, arg) static char **add_##arg(api_enum const api, char **uri, type arg)

add_args(int const, count) {
	if (!count) {
		return uri;
	}
	char cnt[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "count=");
	snprintf(cnt, sizeof(cnt), "%d", count<201?count:200);
	alloc_strcat(uri, cnt);

	return uri;
}

add_args(tweet_id_t const, id) {
	if (!id) {
		return uri;
	}
	char i[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "id=");
	snprintf(i, sizeof(i), "%" PRId64, id);
	alloc_strcat(uri, i);

	return uri;
}

add_args(tweet_id_t const, since_id) {
	if (!since_id) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "since_id=");
	snprintf(id, sizeof(id), "%" PRId64, since_id);
	alloc_strcat(uri, id);

	return uri;
}

add_args(tweet_id_t const, max_id) {
	if (!max_id) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "max_id=");
	snprintf(id, sizeof(id), "%" PRId64, max_id);
	alloc_strcat(uri, id);

	return uri;
}

add_args(max_and_since const, max_and_since) {
	add_max_id(api, uri, max_and_since.max_id);
	add_since_id(api, uri, max_and_since.since_id);
	return uri;
}

add_args(int const, trim_user) {
	if (trim_user == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "trim_user=");
	snprintf(boolean, sizeof(boolean), "%d", !!trim_user);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, contributor_details) {
	if (contributor_details == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "contributor_details=");
	snprintf(boolean, sizeof(boolean), "%d", !!contributor_details);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, include_entities) {
	if (include_entities == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "include_entities=");
	snprintf(boolean, sizeof(boolean), "%d", !!include_entities);
	alloc_strcat(uri, boolean);

	return uri;
}

static char **add_include_rts(api_enum const api, char **uri, int const include_rts, int const count) {
	if (!count && include_rts == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "include_rts=");
	snprintf(boolean, sizeof(boolean), "%d", count || (include_rts != -1));
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(user_id_t const, user_id) {
	if (!user_id) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "user_id=");
	snprintf(id, sizeof(id), "%ld", user_id);
	alloc_strcat(uri, id);

	return uri;
}

add_args(char const * const, screen_name) {
	if (!(screen_name && *screen_name)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "screen_name=");
	alloc_strcat(uri, screen_name);

	return uri;
}

add_args(twitter_id const, twitter_id) {
	add_user_id(api, uri, twitter_id.user_id);
	add_screen_name(api, uri, twitter_id.screen_name);
	return uri;
}

add_args(int const, exclude_replies) {
	if (exclude_replies == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "exclude_replies=");
	snprintf(boolean, sizeof(boolean), "%d", !!exclude_replies);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, include_user_entities) {
	if (include_user_entities == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "include_user_entities=");
	snprintf(boolean, sizeof(boolean), "%d", !!include_user_entities);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, include_my_retweet) {
	if (include_my_retweet == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "include_my_retweet=");
	snprintf(boolean, sizeof(boolean), "%d", !!include_my_retweet);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(char const * const, status) {
	if (!(status && *status)) {
		return uri;
	}
	char *status_140 = NULL;
	alloc_strcat(&status_140, status);
	if (utf8_strlen(status_140) > 140) {
		*(utf8_offset_to_pointer(status_140, 140)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(status_140);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "status=");
	alloc_strcat(uri, escaped_msg);
	free_and_assign(status_140);
	free_and_assign(escaped_msg);

	return uri;
}

add_args(tweet_id_t const, in_reply_to_status_id) {
	if (!in_reply_to_status_id) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "in_reply_to_status_id=");
	snprintf(id, sizeof(id), "%" PRId64, in_reply_to_status_id);
	alloc_strcat(uri, id);

	return uri;
}

add_args(geocode const, coods) {
	if (!((int)(fabs(coods.latitude)) < 90 && (int)(fabs(coods.longitude)) < 180)) {
		return uri;
	}
	char latitude[32];
	char longitude[32];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "latitude=");
	snprintf(latitude, sizeof(latitude), "%2.12f", coods.latitude);
	alloc_strcat(uri, latitude);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "longitude=");
	snprintf(longitude, sizeof(longitude), "%2.12f", coods.longitude);
	alloc_strcat(uri, longitude);

	return uri;
}

add_args(char const * const, place_id) {
	if (!(place_id && *place_id)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "place_id=");
	alloc_strcat(uri, place_id);

	return uri;
}

add_args(int const, display_coordinates) {
	if (display_coordinates == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "display_coordinates=");
	snprintf(boolean, sizeof(boolean), "%d", !!display_coordinates);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(char const * const, url) {
	if (!(url && *url)) {
		return uri;
	}
	char *escaped_msg = oauth_url_escape(url);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "url=");
	alloc_strcat(uri, escaped_msg);
	free_and_assign(escaped_msg);

	return uri;
}

add_args(int const, maxwidth) {
	if (!(249 < maxwidth && maxwidth < 551)) {
		return uri;
	}
	char cnt[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "maxwidth=");
	snprintf(cnt, sizeof(cnt), "%d", maxwidth);
	alloc_strcat(uri, cnt);

	return uri;
}

add_args(int const, hide_media) {
	if (hide_media == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "hide_media=");
	snprintf(boolean, sizeof(boolean), "%d", !!hide_media);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, hide_thread) {
	if (hide_thread == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "hide_thread=");
	snprintf(boolean, sizeof(boolean), "%d", !!hide_thread);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, omit_script) {
	if (omit_script == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "omit_script=");
	snprintf(boolean, sizeof(boolean), "%d", !!omit_script);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, align) {
	if (align > (CENTER + 1) || align == -1) {
		return uri;
	}
	char const *algn[] = {"none", "left", "right", "center"};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "align=");
	alloc_strcat(uri, algn[align]);

	return uri;
}

add_args(char const * const, related) {
	if (!(related && *related)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "related=");
	alloc_strcat(uri, related);

	return uri;
}

add_args(char const * const, lang) {
	if (!(lang && *lang)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "lang=");
	alloc_strcat(uri, lang);

	return uri;
}

add_args(cursor_t const, cursor) {
	if (!cursor) {
		return uri;
	}
	char cur[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "cursor=");
	snprintf(cur, sizeof(cur), "%" PRId64, cursor);
	alloc_strcat(uri, cur);

	return uri;
}

add_args(int const, stringify_ids) {
	if (stringify_ids == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "stringify_ids=");
	snprintf(boolean, sizeof(boolean), "%d", !!stringify_ids);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(char const * const, q) {
	if(!(q && *q)) {
		return uri;
	}
	char *escaped_msg = oauth_url_escape(q);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "q=");
	alloc_strcat(uri, escaped_msg);
	free_and_assign(escaped_msg);

	return uri;
}

add_args(geocode const, geocode) {
	if (!((int)(fabs(geocode.latitude)) < 90 && (int)(fabs(geocode.longitude)) < 180 && geocode.radius != 0 && geocode.unit && *geocode.unit)) {
		return uri;
	}
	char latitude[32];
	char longitude[32];
	char rad[8];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "geocode=");
	snprintf(latitude, sizeof(latitude), "%2.12f,", geocode.latitude);
	alloc_strcat(uri, latitude);
	snprintf(longitude, sizeof(longitude), "%2.12f,", geocode.longitude);
	alloc_strcat(uri, longitude);
	snprintf(rad, sizeof(rad), "%d", geocode.radius);
	alloc_strcat(uri, rad);
	alloc_strcat(uri, geocode.unit);

	return uri;
}

add_args(char const * const, locale) {
	if (!(locale && *locale)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "locale=");
	alloc_strcat(uri, locale);

	return uri;
}

add_args(int const, result_type) {
	if (!result_type) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "result_type=");
	if (result_type & MIXED) {
		alloc_strcat(uri, "mixed");
	}
	if (result_type & RECENT) {
		if (result_type & MIXED) {
			alloc_strcat(uri, ",");
		}
		alloc_strcat(uri, "recent");
	}
	if (result_type & POPULAR) {
		if (result_type & (MIXED | RECENT)) {
			alloc_strcat(uri, ",");
		}
		alloc_strcat(uri, "popular");
	}

	return uri;
}

add_args(char const * const, until) {
	if (!(until && *until)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "until=");
	alloc_strcat(uri, until);

	return uri;
}

add_args(char const * const, callback) {
	if (!(callback && *callback)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "callback=");
	alloc_strcat(uri, callback);

	return uri;
}

add_args(int const, skip_status) {
	if (skip_status == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "skip_status=");
	snprintf(boolean, sizeof(boolean), "%d", !!skip_status);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, pages) {
	if (!pages) {
		return uri;
	}
	char pg[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "pages=");
	snprintf(pg, sizeof(pg), "%d", pages);
	alloc_strcat(uri, pg);

	return uri;
}

add_args(char const * const, text) {
	if (!(text && *text)) {
		return uri;
	}
	char *escaped_msg = oauth_url_escape(text);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "text=");
	alloc_strcat(uri, escaped_msg);
	free_and_assign(escaped_msg);

	return uri;
}

add_args(int const, count_upto_5000) {
	if (!count_upto_5000) {
		return uri;
	}
	char cnt[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "count=");
	snprintf(cnt, sizeof(cnt), "%d", count_upto_5000<5001?count_upto_5000:5000);
	alloc_strcat(uri, cnt);

	return uri;
}

add_args(char const * const, user_id_str) {
	if (!(user_id_str && *user_id_str)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "user_id=");
	alloc_strcat(uri, user_id_str);

	return uri;
}

add_args(int const, follow) {
	if (follow == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "follow=");
	snprintf(boolean, sizeof(boolean), "%d", !!follow);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, device) {
	if (device == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "device=");
	snprintf(boolean, sizeof(boolean), "%d", !!device);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, retweets) {
	if (retweets == -1) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "retweets=");
	snprintf(boolean, sizeof(boolean), "%d", !!retweets);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(user_id_t const, source_id) {
	if (!source_id) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "source_id=");
	snprintf(id, sizeof(id), "%" PRId64, source_id);
	alloc_strcat(uri, id);

	return uri;
}

add_args(char const * const, source_screen_name) {
	if (!(source_screen_name && *source_screen_name)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "source_screen_name=");
	alloc_strcat(uri, source_screen_name);

	return uri;
}

add_args(twitter_id const, source) {
	add_source_id(api, uri, source.user_id);
	add_source_screen_name(api, uri, source.screen_name);
	return uri;
}

add_args(user_id_t const, target_id) {
	if (!target_id) {
		return uri;
	}
	char id[32] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "target_id=");
	snprintf(id, sizeof(id), "%" PRId64, target_id);
	alloc_strcat(uri, id);

	return uri;
}

add_args(char const * const, target_screen_name) {
	if (!(target_screen_name && *target_screen_name)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "target_screen_name=");
	alloc_strcat(uri, target_screen_name);

	return uri;
}

add_args(twitter_id const, target) {
	add_target_id(api, uri, target.user_id);
	add_target_screen_name(api, uri, target.screen_name);
	return uri;
}

add_args(int const, trend_location_woeid) {
	if (!trend_location_woeid) {
		return uri;
	}
	char woeid[12] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "trend_location_woeid=");
	snprintf(woeid, sizeof(woeid), "%d", trend_location_woeid);
	alloc_strcat(uri, woeid);

	return uri;
}

add_args(int const, sleep_time_enabled) {
	if (!(sleep_time_enabled == -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "sleep_time_enabled=");
	snprintf(boolean, sizeof(boolean), "%d", !!sleep_time_enabled);
	alloc_strcat(uri, boolean);

	return uri;
}

add_args(int const, start_sleep_time) {
	if (!start_sleep_time) {
		return uri;
	}
	char time[12] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "start_sleep_time=");
	snprintf(time, sizeof(time), "%d", start_sleep_time);
	alloc_strcat(uri, time);

	return uri;
}

add_args(int const, end_sleep_time) {
	if (!end_sleep_time) {
		return uri;
	}
	char time[12] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "end_sleep_time=");
	snprintf(time, sizeof(time), "%d", end_sleep_time);
	alloc_strcat(uri, time);

	return uri;
}

add_args(char const * const, time_zone) {
	if (!(time_zone && *time_zone)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "time_zone=");
	alloc_strcat(uri, time_zone);

	return uri;
}

add_args(char const * const, device_str) {
	if (!(device_str && *device_str)) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "device=");
	alloc_strcat(uri, device_str);

	return uri;
}

add_args(char const * const, name) {
	if (!(name && *name)) {
		return uri;
	}
	char *name_20 = NULL;
	alloc_strcat(&name_20, name);
	if (utf8_strlen(name_20) > 20) {
		*(utf8_offset_to_pointer(name_20, 20)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(name_20);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "name=");
	alloc_strcat(uri, escaped_msg);
	free_and_assign(name_20);
	free_and_assign(escaped_msg);

	return uri;
}

add_args(char const * const, url_upto_100) {
	if (!(url_upto_100 && *url_upto_100)) {
		return uri;
	}
	char *url_100 = NULL;
	alloc_strcat(&url_100, url_upto_100);
	if (utf8_strlen(url_100) > 100) {
		*(utf8_offset_to_pointer(url_100, 100)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(url_100);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "url=");
	alloc_strcat(uri, escaped_msg);
	free_and_assign(url_100);
	free_and_assign(escaped_msg);

	return uri;
}

add_args(char const * const, location) {
	if (!(location && *location)) {
		return uri;
	}
	char *location_30 = NULL;
	alloc_strcat(&location_30, location);
	if (utf8_strlen(location_30) > 30) {
		*(utf8_offset_to_pointer(location_30, 30)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(location_30);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "location=");
	alloc_strcat(uri, escaped_msg);
	free_and_assign(location_30);
	free_and_assign(escaped_msg);

	return uri;
}

add_args(char const * const, description) {
	if (!(description && *description)) {
		return uri;
	}
	char *description_160 = NULL;
	alloc_strcat(&description_160, description);
	if (utf8_strlen(description_160) > 160) {
		*(utf8_offset_to_pointer(description_160, 160)) = '\0';
	}
	char *escaped_msg = oauth_url_escape(description_160);
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "description=");
	alloc_strcat(uri, escaped_msg);
	free_and_assign(description_160);
	free_and_assign(escaped_msg);

	return uri;
}

static inline char **add_color(char **uri, uint32_t color, int digit){
	char hex[7] = {0};
	snprintf(hex, sizeof(hex), "%0*" PRIx32, digit?digit:6, color);
	alloc_strcat(uri, hex);
	return uri;
}

add_args(int32_t const, profile_background_color) {
	if (profile_background_color < 0) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_background_color=");
	add_color(uri, profile_background_color & 0x00ffffff, (profile_background_color & 0x0f000000) >> 24);

	return uri;
}

add_args(int32_t const, profile_link_color) {
	if (profile_link_color < 0) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_link_color=");
	add_color(uri, profile_link_color & 0x00ffffff, (profile_link_color & 0x0f000000) >> 24);

	return uri;
}

add_args(int32_t const, profile_sidebar_border_color) {
	if (profile_sidebar_border_color < 0) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_sidebar_border_color=");
	add_color(uri, profile_sidebar_border_color & 0x00ffffff, (profile_sidebar_border_color & 0x0f000000) >> 24);

	return uri;
}

add_args(int32_t const, profile_sidebar_fill_color) {
	if (profile_sidebar_fill_color < 0) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_sidebar_fill_color=");
	add_color(uri, profile_sidebar_fill_color & 0x00ffffff, (profile_sidebar_fill_color & 0x0f000000) >> 24);

	return uri;
}

add_args(int32_t const, profile_text_color) {
	if (profile_text_color < 0) {
		return uri;
	}
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "profile_text_color=");
	add_color(uri, profile_text_color & 0x00ffffff, (profile_text_color & 0x0f000000) >> 24);

	return uri;
}

add_args(int const, page) {
	if (!page) {
		return uri;
	}
	char pg[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "page=");
	snprintf(pg, sizeof(pg), "%d", page);
	alloc_strcat(uri, pg);

	return uri;
}

add_args(int const, count_upto_20) {
	if (!count_upto_20) {
		return uri;
	}
	char cnt[8] = {0};
	add_que_or_amp(api, uri);
	alloc_strcat(uri, "count=");
	snprintf(cnt, sizeof(cnt), "%d", count_upto_20<21?count_upto_20:20);
	alloc_strcat(uri, cnt);

	return uri;
}


#undef add_args

/*--- Streaming API ---*/

/*
static char **add_delimited (stream_enum stream, char **uri, int delimited) {
	if (!(delimited)) {
		return uri;
	}
	add_que_or_amp_stream(stream, uri);
	alloc_strcat(uri, "delimited=");
	alloc_strcat(uri, "length");

	return uri;
}

static char **add_stall_warnings (stream_enum stream, char **uri, int stall_warnings) {
	if (!(stall_warnings != -1)) {
		return uri;
	}
	char boolean[2];
	add_que_or_amp_stream(stream, uri);
	alloc_strcat(uri, "stall_warnings=");
	snprintf(boolean, sizeof(boolean), "%d", !!stall_warnings);
	alloc_strcat(uri, boolean);

	return uri;
}
*/

/*--- REST API ---*/
#define add_args(arg) add_##arg(api, &uri, arg)

static inline bool is_valid_id(twitter_id const id) {
	return id.user_id || (id.screen_name && id.screen_name[0]);
}

int get_statuses_mentions_timeline (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const trim_user, //optional. if not -1, add it to argument.
	int const contributor_details, //optional. if not -1, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/mentions_timeline.json
Parameters
count optional

Specifies the number of tweets to try and retrieve, up to a maximum of 200. The value of count is best thought of as a limit to the number of tweets to return because suspended or deleted content is removed after the count has been applied. We include retweets in the count, even if include_rts is not supplied. It is recommended you always send include_rts=1 when using this API method.

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_MENTIONS_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(count);
	add_args(max_and_since);
	add_args(trim_user);
	add_args(contributor_details);
	add_args(include_entities);
	add_include_rts(api, &uri, include_rts, count);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_statuses_user_timeline (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const trim_user, //optional. if not -1, add it to argument.
	int const exclude_replies, //optional. if not -1, add it to argument.
	int const contributor_details, //optional. if not -1, add it to argument.
	int const include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/user_timeline.json
Parameters

Always specify either an user_id or screen_name.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

count optional

Specifies the number of tweets to try and retrieve, up to a maximum of 200 per distinct request. The value of count is best thought of as a limit to the number of tweets to return because suspended or deleted content is removed after the count has been applied. We include retweets in the count, even if include_rts is not supplied. It is recommended you always send include_rts=1 when using this API method.

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

exclude_replies optional

This parameter will prevent replies from appearing in the returned timeline. Using exclude_replies with the count parameter will mean you will receive up-to count tweets — this is because the count parameter retrieves that many tweets before filtering out retweets and replies. This parameter is only supported for JSON and XML responses.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_rts optional

When set to false, the timeline will strip any native retweets (though they will still count toward both the maximal length of the timeline and the slice selected by the count parameter). Note: If you're using the trim_user parameter in conjunction with include_rts, the retweets will still contain a full user object.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_USER_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(count);
	add_args(max_and_since);
	add_args(trim_user);
	add_args(exclude_replies);
	add_args(contributor_details);
	add_include_rts(api, &uri, include_rts, count);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_statuses_home_timeline (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const trim_user, //optional. if not -1, add it to argument.
	int const exclude_replies, //optional. if not -1, add it to argument.
	int const contributor_details, //optional. if not -1, add it to argument.
	int const include_entities //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/home_timeline.json
Parameters
count optional

Specifies the number of records to retrieve. Must be less than or equal to 200. Defaults to 20.

Example Values: 5

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

exclude_replies optional

This parameter will prevent replies from appearing in the returned timeline. Using exclude_replies with the count parameter will mean you will receive up-to count tweets — this is because the count parameter retrieves that many tweets before filtering out retweets and replies.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_HOME_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(count);
	add_args(max_and_since);
	add_args(trim_user);
	add_args(exclude_replies);
	add_args(contributor_details);
	add_args(include_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_statuses_retweets_of_me (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const trim_user, //optional. if not -1, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const include_user_entities //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweets_of_me.json
Parameters
count optional

Specifies the number of records to retrieve. Must be less than or equal to 100. If omitted, 20 will be assumed.

Example Values: 5

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

include_entities optional

The tweet entities node will be disincluded when set to false.

Example Values: false

include_user_entities optional

The user entities node will be disincluded when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_RETWEETS_OF_ME;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(count);
	add_args(max_and_since);
	add_args(trim_user);
	add_args(include_entities);
	add_args(include_user_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_statuses_retweets_by_id (
	tweet_id_t const id, //required
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	int const trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweets/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

count optional

Specifies the number of records to retrieve. Must be less than or equal to 100.

Example Values: 5

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	if (100 < count) {
		fprintf(stderr, "count must be <= 100\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_RETWEETS_BY_ID;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%" PRId64 ".json", id);
	alloc_strcat(&uri, i);

	add_args(count);
	add_args(trim_user);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_statuses_show_by_id (
	tweet_id_t const id, //required
	char **res, //response
	int const trim_user, //optional. if not -1, add it to argument.
	int const include_my_retweet, //optional. if not -1, add it to argument.
	int const include_entities //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/show.json
Parameters
id required

The numerical ID of the desired Tweet.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

include_my_retweet optional

When set to either true, t or 1, any Tweets returned that have been retweeted by the authenticating user will include an additional current_user_retweet node, containing the ID of the source status for the retweet.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_SHOW_BY_ID;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(id);
	add_args(trim_user);
	add_args(include_my_retweet);
	add_args(include_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int post_statuses_destroy_by_id (
	tweet_id_t const id, //required
	char **res, //response
	int const trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/destroy/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_DESTROY_BY_ID;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%" PRId64 ".json", id);
	alloc_strcat(&uri, i);

	add_args(trim_user);


	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int post_statuses_update(
	char const *status, //required
	char **res, // response
	tweet_id_t const in_reply_to_status_id, //optional. if not 0, add it to argument.
	int const do_add_l_l, //add it. whether add l_l to argument.
	geocode const l_l, //optional. if it is valid figure, add it to argument.
	char const const *place_id, //optional. if not NULL, add it to argument.
	int const display_coordinates, //optional. if not -1, add it to argument.
	int const trim_user //optional. if not -1, add it to argument.
	)
{
/*

Resource URL

https://api.twitter.com/1.1/statuses/update.json
Parameters
status required

The text of your status update, typically up to 140 characters. URL encode as necessary. t.co link wrapping may effect character counts.

There are some special commands in this field to be aware of. For instance, preceding a message with "D " or "M " and following it with a screen name can create a direct message to that user if the relationship allows for it.

in_reply_to_status_id optional

The ID of an existing status that the update is in reply to.

Note:: This parameter will be ignored unless the author of the tweet this parameter references is mentioned within the status text. Therefore, you must include @username, where username is the author of the referenced tweet, within the update.

lat optional

The latitude of the location this tweet refers to. This parameter will be ignored unless it is inside the range -90.0 to +90.0 (North is positive) inclusive. It will also be ignored if there isn't a corresponding long parameter.

Example Values: 37.7821120598956

long optional

The longitude of the location this tweet refers to. The valid ranges for longitude is -180.0 to +180.0 (East is positive) inclusive. This parameter will be ignored if outside that range, if it is not a number, if geo_enabled is disabled, or if there not a corresponding lat parameter.

Example Values: -122.400612831116

place_id optional

A place in the world. These IDs can be retrieved from GET geo/reverse_geocode.

Example Values: df51dec6f4ee2b2c

display_coordinates optional

Whether or not to put a pin on the exact coordinates a tweet has been sent from.

Example Values: true

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/

	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!status[0]) {
		fprintf(stderr, "need status text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_UPDATE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(status);
	add_args(in_reply_to_status_id);
	if (do_add_l_l) {
		add_coods(api, &uri, l_l);
	}
	add_args(place_id);
	add_args(display_coordinates);
	add_args(trim_user);


	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int post_statuses_retweet_by_id (
	tweet_id_t const id, //required
	char **res, //response
	int const trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweet/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_RETWEET_BY_ID;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%" PRId64 ".json", id);
	alloc_strcat(&uri, i);

	add_args(trim_user);


	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

//POST statuses/update_with_media is too difficult to implement

int get_statuses_oembed (
	tweet_id_t const id, //required. It is not necessary to include both.
	char const *url, //required. It is not necessary to include both.
	char **res, //response
	int const maxwidth, //optional? It must be between 250 and 550.
	int const hide_media, //optional? If not -1, add it to argument.
	int const hide_thread, //optional? If not -1, add it to argument.
	int const omit_script, //optional? If not -1, add it to argument.
	align const align, //optional? If not NONE, add it to argument.
	char const *related, //optional? If it is valid, add it to argument.
	char const *lang //optional? If it is valid, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/oembed.json
Parameters

Either the id or url parameters must be specified in a request. It is not necessary to include both.
id required

The Tweet/status ID to return embed code for.

Example Values: 99530515043983360

url required

The URL of the Tweet/status to be embedded.

Example Values:

To embed the Tweet at https://twitter.com/#!/twitter/status/99530515043983360, use:

https%3A%2F%2Ftwitter.com%2F%23!%2Ftwitter%2Fstatus%2F99530515043983360

To embed the Tweet at https://twitter.com/twitter/status/99530515043983360, use:

https%3A%2F%2Ftwitter.com%2Ftwitter%2Fstatus%2F99530515043983360

maxwidth

The maximum width in pixels that the embed should be rendered at. This value is constrained to be between 250 and 550 pixels.

Note that Twitter does not support the oEmbed maxheight parameter. Tweets are fundamentally text, and are therefore of unpredictable height that cannot be scaled like an image or video. Relatedly, the oEmbed response will not provide a value for height. Implementations that need consistent heights for Tweets should refer to the hide_thread and hide_media parameters below.

Example Values: 325

hide_media

Specifies whether the embedded Tweet should automatically expand images which were uploaded via POST statuses/update_with_media. When set to either true, t or 1 images will not be expanded. Defaults to false.

Example Values: true

hide_thread

Specifies whether the embedded Tweet should automatically show the original message in the case that the embedded Tweet is a reply. When set to either true, t or 1 the original Tweet will not be shown. Defaults to false.

Example Values: true

omit_script

Specifies whether the embedded Tweet HTML should include a <script> element pointing to widgets.js. In cases where a page already includes widgets.js, setting this value to true will prevent a redundant script element from being included. When set to either true, t or 1 the <script> element will not be included in the embed HTML, meaning that pages must include a reference to widgets.js manually. Defaults to false.

Example Values: true

align

Specifies whether the embedded Tweet should be left aligned, right aligned, or centered in the page. Valid values are left, right, center, and none. Defaults to none, meaning no alignment styles are specified for the Tweet.

Example Values: center

related

A value for the TWT related parameter, as described in Web Intents. This value will be forwarded to all Web Intents calls.

Example Values:

twitterapi,twittermedia,twitter

lang

Language code for the rendered embed. This will affect the text and localization of the rendered HTML.

Example Values: fr

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id && !url && !(*url)) {
		fprintf(stderr, "need id number or url text.\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_OEMBED;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(id);
	add_args(url);
	add_args(maxwidth);
	add_args(hide_media);
	add_args(hide_thread);
	add_args(omit_script);
	add_args(align);
	add_args(related);
	add_args(lang);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_statuses_retweeters_ids (
	tweet_id_t const id, //required
	char **res, //response
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweeters/ids.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 327473909412814850

cursor semi-optional

Causes the list of IDs to be broken into pages of no more than 100 IDs at a time. The number of IDs returned is not guaranteed to be 100 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

While this method supports the cursor parameter, the entire result set can be returned in a single cursored collection. Using the count parameter with this method will not provide segmented cursors for use with this parameter.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our ids due to their size. Provide this option to have ids returned as strings instead. Read more about Twitter IDs, JSON and Snowflake.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = STATUSES_RETWEETERS_IDS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(id);
	add_args(cursor);
	add_args(stringify_ids);


	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_search_tweets (
	char const *q, //required
	char **res, //response
	geocode const geocode, //optional. If it is valid, add it to argument.
	char const *lang, //optional. If not 0, add it to argument.
	char const *locale, //optional. If not 0, add it to argument. Only ja is currently effective
	int const result_type, //optional. If not 0, add it to argument. 1 = "mixed",2="recent",4="popular"
	int const count, //optional. If not 0, add it to argument.
	char const *until, //optional. If not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const include_entities, //optional. If not -1, add it to argument.
	char const *callback //optional. If not 0, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/search/tweets.json
Parameters
q required

A UTF-8, URL-encoded search query of 1,000 characters maximum, including operators. Queries may additionally be limited by complexity.

Example Values: @noradio

geocode optional

Returns tweets by users located within a given radius of the given latitude/longitude. The location is preferentially taking from the Geotagging API, but will fall back to their Twitter profile. The parameter value is specified by "latitude,longitude,radius", where radius units must be specified as either "mi" (miles) or "km" (kilometers). Note that you cannot use the near operator via the API to geocode arbitrary locations; however you can use this geocode parameter to search near geocodes directly. A maximum of 1,000 distinct "sub-regions" will be considered when using the radius modifier.

Example Values: 37.781157,-122.398720,1mi

lang optional

Restricts tweets to the given language, given by an ISO 639-1 code. Language detection is best-effort.

Example Values: eu

locale optional

Specify the language of the query you are sending (only ja is currently effective). This is intended for language-specific consumers and the default should work in the majority of cases.

Example Values: ja

result_type optional

Optional. Specifies what type of search results you would prefer to receive. The current default is "mixed." Valid values include:
  * mixed: Include both popular and real time results in the response.
  * recent: return only the most recent results in the response
  * popular: return only the most popular results in the response.

Example Values: mixed, recent, popular

count optional

The number of tweets to return per page, up to a maximum of 100. Defaults to 15. This was formerly the "rpp" parameter in the old Search API.

Example Values: 100

until optional

Returns tweets generated before the given date. Date should be formatted as YYYY-MM-DD. Keep in mind that the search index may not go back as far as the date you specify here.

Example Values: 2012-09-01

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

callback optional

If supplied, the response will use the JSONP format with a callback of the given name. The usefulness of this parameter is somewhat diminished by the requirement of authentication for requests to this endpoint.

Example Values: processTweets

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!q || !(*q)) {
		fprintf(stderr, "need q text\n");
		return 0;
	}

	if (strlen(q) > 1000) {
		fprintf(stderr, "too long q text\n");
		return 0;
	}
	char *uri = NULL;
	api_enum api = SEARCH_TWEETS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(q);
	add_args(geocode);
	add_args(lang);
	add_args(locale);
	add_args(result_type);
	add_args(count);
	add_args(until);
	add_args(max_and_since);
	add_args(include_entities);
	add_args(callback);


	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_direct_messages (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/direct_messages.json
Parameters
since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

count optional

Specifies the number of direct messages to try and retrieve, up to a maximum of 200. The value of count is best thought of as a limit to the number of Tweets to return because suspended or deleted content is removed after the count has been applied.

Example Values: 5

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = DIRECT_MESSAGES;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(count);
	add_args(max_and_since);
	add_args(include_entities);
	add_args(skip_status);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_dm_sent (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const pages, //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	int const include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/direct_messages/sent.json
Parameters
since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

count optional

Specifies the number of records to retrieve. Must be less than or equal to 200.

Example Values: 5

page optional

Specifies the page of results to retrieve.

Example Values: 3

include_entities optional

The entities node will not be included when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = DM_SENT;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(count);
	add_args(max_and_since);
	add_pages(api, &uri,pages);
	add_args(include_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_dm_show (
	tweet_id_t const id, //required
	char **res //response
	) {
/*
Resource URL
https://api.twitter.com/1.1/direct_messages/show.json
Parameters
id required

The ID of the direct message.

Example Values: 587424932

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = DM_SHOW;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(id);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int post_dm_destroy (
	tweet_id_t const id, //required
	char **res, //response
	int const include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/direct_messages/destroy.json
Parameters
id required

The ID of the direct message to delete.

Example Values: 1270516771

include_entities optional

The entities node will not be included when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = DM_DESTROY;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(id);
	add_args(include_entities);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int post_dm_new (
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	char const *text, //required.
	char **res //response
	) {
/*
Resource URL
https://api.twitter.com/1.1/direct_messages/new.json
Parameters

One of user_id or screen_name are required.

user_id optional

The ID of the user who should receive the direct message. Helpful for disambiguating when a valid user ID is also a valid screen name.

Example Values: 12345

screen_name optional

The screen name of the user who should receive the direct message. Helpful for disambiguating when a valid screen name is also a user ID.

Example Values: noradio
text required

The text of your direct message. Be sure to URL encode as necessary, and keep the message under 140 characters.

Example Values: Meet me behind the cafeteria after school

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = DM_NEW;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(text);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int get_fs_no_retweets_ids (
	char **res, //response
	int const stringify_ids //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/no_retweets/ids.json
Parameters
stringify_ids optional

Many programming environments will not consume our ids due to their size. Provide this option to have ids returned as strings instead. Read more about Twitter IDs, JSON and Snowflake. This parameter is especially important to use in Javascript environments.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FS_NO_RETWEETS_IDS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(stringify_ids);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_friends_ids (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids, //optional. if not -1, add it to argument.
	int const count //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friends/ids.json
Parameters

Either a screen_name or a user_id must be provided.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

cursor semi-optional

Causes the list of connections to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our Tweet ids due to their size. Provide this option to have ids returned as strings instead. More about Twitter IDs, JSON and Snowflake.

Example Values: true

count optional

Specifies the number of IDs attempt retrieval of, up to a maximum of 5,000 per distinct request. The value of count is best thought of as a limit to the number of results to return. When using the count parameter with this method, it is wise to use a consistent count value across all requests to the same user's collection. Usage of this parameter is encouraged in environments where all 5,000 IDs constitutes too large of a response.

Example Values: 2048

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FRIENDS_IDS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(cursor);
	add_args(stringify_ids);
	add_count_upto_5000(api, &uri, count);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_followers_ids (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids, //optional. if not -1, add it to argument.
	int const count //optional. if not 0, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/followers/ids.json

Parameters

Either a screen_name or a user_id must be provided.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

cursor semi-optional

Causes the list of connections to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our Tweet ids due to their size. Provide this option to have ids returned as strings instead. More about Twitter IDs, JSON and Snowflake.

Example Values: true

count optional

Specifies the number of IDs attempt retrieval of, up to a maximum of 5,000 per distinct request. The value of count is best thought of as a limit to the number of results to return. When using the count parameter with this method, it is wise to use a consistent count value across all requests to the same user's collection. Usage of this parameter is encouraged in environments where all 5,000 IDs constitutes too large of a response.

Example Values: 2048

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FOLLOWERS_IDS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(cursor);
	add_args(stringify_ids);
	add_count_upto_5000(api, &uri, count);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_fs_lookup (
	char **res, //response
	char const *screen_name, //optional. if not 0, add it to argument.
	char const *user_id //optional. if not 0, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/friendships/lookup.json
Parameters
screen_name optional

A comma separated list of screen names, up to 100 are allowed in a single request.

Example Values: twitterapi,twitter

user_id optional

A comma separated list of user IDs, up to 100 are allowed in a single request.

Example Values: 783214,6253282

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FS_LOOKUP;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(screen_name);
	add_user_id_str(api, &uri, user_id);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_fs_incoming (
	char **res, //response
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/friendships/incoming.json
Parameters
cursor semi-optional

Causes the list of connections to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our Tweet ids due to their size. Provide this option to have ids returned as strings instead. More about Twitter IDs, JSON and Snowflake.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FS_INCOMING;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(cursor);
	add_args(stringify_ids);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_fs_outgoing (
	char **res, //response
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/outgoing.format
Parameters
cursor semi-optional

Causes the list of connections to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our Tweet ids due to their size. Provide this option to have ids returned as strings instead. More about Twitter IDs, JSON and Snowflake.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FS_OUTGOING;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(cursor);
	add_args(stringify_ids);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int post_fs_create (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	int const follow //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/friendships/create.json
Parameters

Providing either screen_name or user_id is required.
screen_name optional

The screen name of the user for whom to befriend.

Example Values: noradio

user_id optional

The ID of the user for whom to befriend.

Example Values: 12345

follow optional

Enable notifications for the target user.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FS_CREATE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(follow);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int post_fs_destroy (
	char **res, //response
	twitter_id const twitter_id //Always specify either an user_id or screen_name.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/destroy.json
Parameters

Providing either screen_name or user_id is required.

screen_name optional

The screen name of the user for whom to unfollow.

Example Values: noradio

user_id optional

The ID of the user for whom to unfollow.

Example Values: 12345
*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FS_DESTROY;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int post_fs_update (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	int const device, //optional. if not -1, add it to argument.
	int const retweets //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/update.json
Parameters

Providing either screen_name or user_id is required.

screen_name optional

The screen name of the user for whom to befriend.

Example Values: noradio

user_id optional

The ID of the user for whom to befriend.

Example Values: 12345

device optional

Enable/disable device notifications from the target user.

Example Values: true, false

retweets optional

Enable/disable retweets from the target user.

Example Values: true, false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FS_UPDATE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(device);
	add_args(retweets);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int get_fs_show (
	char **res, //response
	twitter_id const source, //Always specify either an user_id or screen_name.
	twitter_id const target //Always specify either an user_id or screen_name.
	) {
/*
Resource URL
https://api.twitter.com/1.1/friendships/show.json
Parameters

At least one source and one target, whether specified by IDs or screen_names, should be provided to this method.

source_id optional

The user_id of the subject user.

Example Values: 3191321

source_screen_name optional

The screen_name of the subject user.

Example Values: raffi

target_id optional

The user_id of the target user.

Example Values: 20

target_screen_name optional

The screen_name of the target user.

Example Values: noradio

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if (!(is_valid_id(source)) && !(is_valid_id(target))) {
		fprintf(stderr, "At least one source and one target, whether specified by IDs or screen_names");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FS_SHOW;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(source);
	add_args(target);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_friends_list (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const count, //optional. if not 0, add it to argument.
	int const skip_status, //optional. if not -1, add it to argument.
	int const include_user_entities //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/friends/list.json
Parameters

Either a screen_name or a user_id should be provided.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

cursor semi-optional

Causes the results to be broken into pages. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

countoptional

The number of users to return per page, up to a maximum of 200. Defaults to 20.

Example Values: 42

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

Example Values: false

include_user_entities optional

The user object entities node will be disincluded when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FRIENDS_LIST;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(cursor);
	add_args(count);
	add_args(skip_status);
	add_args(include_user_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_followers_list (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const count, //optional. if not 0, add it to argument.
	int const skip_status, //optional. if not -1, add it to argument.
	int const include_user_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/followers/list.json
Parameters

Either a screen_name or a user_id should be provided.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

cursor semi-optional

Causes the results to be broken into pages. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

count optional

The number of users to return per page, up to a maximum of 200. Defaults to 20.

Example Values: 42

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

Example Values: false

include_user_entities optional

The user object entities node will be disincluded when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = FOLLOWERS_LIST;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(cursor);
	add_args(count);
	add_args(skip_status);
	add_args(include_user_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_account_settings (
	char **res //response
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/settings.json

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = ACCOUNT_SETTINGS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_account_verify_credentials (
	char **res, //response
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/verify_credentials.json
Parameters

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = ACCOUNT_VERIFY_CREDEBTIALS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(include_entities);
	add_args(skip_status);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int post_account_settings (
	char **res, //response
	int const trend_location_woeid, //optional. if not 0, add it to argument.
	int const sleep_time_enabled, //optional. if not -1, add it to argument.
	int const start_sleep_time, //optional. if not -1, add it to argument.
	int const end_sleep_time, //optional. if not -1, add it to argument.
	char const *time_zone, //optional. if it is valid, add it to argument.
	char const *lang //optional. if it is valid, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/account/settings.json
Parameters

While all parameters for this method are optional, at least one or more should be provided when executing this request.

trend_location_woeid optional

The Yahoo! Where On Earth ID to use as the user's default trend location. Global information is available by using 1 as the WOEID. The woeid must be one of the locations returned by GET trends/available.

Example Values: 1

sleep_time_enabled optional

When set to true, t or 1, will enable sleep time for the user. Sleep time is the time when push or SMS notifications should not be sent to the user.

Example Values: true

start_sleep_time optional

The hour that sleep time should begin if it is enabled. The value for this parameter should be provided in ISO8601 format (i.e. 00-23). The time is considered to be in the same timezone as the user's time_zone setting.

Example Values: 13

end_sleep_time optional

The hour that sleep time should end if it is enabled. The value for this parameter should be provided in ISO8601 format (i.e. 00-23). The time is considered to be in the same timezone as the user's time_zone setting.

Example Values: 13

time_zone optional

The timezone dates and times should be displayed in for the user. The timezone must be one of the Rails TimeZone names.

Example Values: Europe/Copenhagen, Pacific/Tongatapu

lang optional

The language which Twitter should render in for this user. The language must be specified by the appropriate two letter ISO 639-1 representation. Currently supported languages are provided by GET help/languages.

Example Values: it, en, es

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = ACCOUNT_SETTINGS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(trend_location_woeid);
	add_args(sleep_time_enabled);
	add_args(start_sleep_time);
	add_args(end_sleep_time);
	add_args(time_zone);
	add_args(lang);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int post_account_update_delivery_device (
	char const *device, //required.
	char **res, //response
	int const include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/update_delivery_device.json
Parameters
device required

Must be one of: sms, none.

Example Values: sms

include_entities optional

When set to either true, t or 1, each tweet will include a node called "entities,". This node offers a variety of metadata about the tweet in a discreet structure, including: user_mentions, urls, and hashtags. While entities are opt-in on timelines at present, they will be made a default component of output in the future. See Tweet Entities for more detail on entities.

Example Values: true

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = ACCOUNT_UPDATE_DELIVERY_DEVICE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_device_str(api, &uri, device);
	add_args(include_entities);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int post_account_update_profile (
	char **res, //response
	char const *name, //optional. if not 0, add it to argument.
	char const *url, //optional. if not 0, add it to argument.
	char const *location, //optional. if not 0, add it to argument.
	char const *description, //optional. if not 0, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/update_profile.json
Parameters

While no specific parameter is required, at least one of these parameters should be provided when executing this method.

name optional

Full name associated with the profile. Maximum of 20 characters.

Example Values: Marcel Molina

url optional

URL associated with the profile. Will be prepended with "http://" if not present. Maximum of 100 characters.

Example Values: http://project.ioni.st

location optional

The city or country describing where the user of the account is located. The contents are not normalized or geocoded in any way. Maximum of 30 characters.

Example Values: San Francisco, CA

description optional

A description of the user owning the account. Maximum of 160 characters.

Example Values: Flipped my wig at age 22 and it never grew back. Also: I work at Twitter.

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.
*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = ACCOUNT_UPDATE_PROFILE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(name);
	add_url_upto_100(api, &uri, url);
	add_args(location);
	add_args(description);
	add_args(include_entities);
	add_args(skip_status);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

// POST account/update_profile_background_image is too difficult to implement

int post_account_update_profile_colors (
	char **res, //response
	int32_t const profile_background_color, //optional. if not -1, add it to argument.
	int32_t const profile_link_color, //optional. if not -1, add it to argument.
	int32_t const profile_sidebar_border_color, //optional. if not -1, add it to argument.
	int32_t const profile_sidebar_fill_color, //optional. if not -1, add it to argument.
	int32_t const profile_text_color, //optional. if not -1, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/update_profile_colors.json
Parameters
profile_background_color optional

Profile background color.

Example Values: 3D3D3D

profile_link_color optional

Profile link color.

Example Values: 0000FF

profile_sidebar_border_color optional

Profile sidebar's border color.

Example Values: 0F0F0F

profile_sidebar_fill_color optional

Profile sidebar's background color.

Example Values: 00FF00

profile_text_color optional

Profile text color.

Example Values: 000000

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = ACCOUNT_UPDATE_PROFILE_COLORS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(profile_background_color);
	add_args(profile_link_color);
	add_args(profile_sidebar_border_color);
	add_args(profile_sidebar_fill_color);
	add_args(profile_text_color);
	add_args(include_entities);
	add_args(skip_status);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

// POST account/update_profile_image is too difficult to implement

int get_blocks_list (
	char **res, //response
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status, //optional. if not -1, add it to argument.
	cursor_t const cursor //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/blocks/list.json
Parameters
include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

cursor semi-optional

Causes the list of blocked users to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = BLOCKS_LIST;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(include_entities);
	add_args(skip_status);
	add_args(cursor);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_blocks_ids (
	char **res, //response
	int const stringify_ids, //optional. if not -1, add it to argument.
	cursor_t const cursor //optional. if not 0, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/blocks/ids.json
Parameters
stringify_ids optional

Many programming environments will not consume our ids due to their size. Provide this option to have ids returned as strings instead. Read more about Twitter IDs, JSON and Snowflake.

Example Values: true

cursor semi-optional

Causes the list of IDs to be broken into pages of no more than 5000 IDs at a time. The number of IDs returned is not guaranteed to be 5000 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

Example Values: 12893764510938

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = BLOCKS_IDS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(stringify_ids);
	add_args(cursor);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int post_blocks_create (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/blocks/create.json
Parameters

Either screen_name or user_id must be provided.
screen_name optional

The screen name of the potentially blocked user. Helpful for disambiguating when a valid screen name is also a user ID.

Example Values: noradio
user_id optional

The ID of the potentially blocked user. Helpful for disambiguating when a valid user ID is also a valid screen name.

Example Values: 12345
include_entities optional

The entities node will not be included when set to false.

Example Values: false
skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = BLOCKS_CREATE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(include_entities);
	add_args(skip_status);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int post_blocks_destroy (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/blocks/destroy.json
Parameters

One of screen_name or id must be provided.

screen_name optional

The screen name of the potentially blocked user. Helpful for disambiguating when a valid screen name is also a user ID.

Example Values: noradio

user_id optional

The ID of the potentially blocked user. Helpful for disambiguating when a valid user ID is also a valid screen name.

Example Values: 12345

include_entities optional

The entities node will not be included when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = BLOCKS_DESTROY;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(include_entities);
	add_args(skip_status);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

int get_users_lookup (
	char **res, //response
	char const *screen_name, //optional. if not 0, add it to argument.
	char const *user_id, //optional. if not 0, add it to argument.
	int const include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/lookup.json
Parameters
screen_name optional

A comma separated list of screen names, up to 100 are allowed in a single request. You are strongly encouraged to use a POST for larger (up to 100 screen names) requests.

Example Values: twitterapi,twitter

user_id optional

A comma separated list of user IDs, up to 100 are allowed in a single request. You are strongly encouraged to use a POST for larger requests.

Example Values: 783214,6253282

include_entities optional

The entities node that may appear within embedded statuses will be disincluded when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = USERS_LOOKUP;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(screen_name);
	add_user_id_str(api, &uri, user_id);
	add_args(include_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_users_show (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	int const include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/show.json
Parameters

user_id required

The ID of the user for whom to return results for. Either an id or screen_name is required for this method.

Example Values: 12345

screen_name required

The screen name of the user for whom to return results for. Either a id or screen_name is required for this method.

Example Values: noradio

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = USERS_SHOW;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(include_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_users_search (
	char const *q, //required.
	char **res, //response
	int const page, //optional. if not 0, add it to argument.
	int const count, //optional. if not 0, add it to argument.
	int const include_entities //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/search.json
Parameters
q required

The search query to run against people search.

Example Values: Twitter%20API

page optional

Specifies the page of results to retrieve.

Example Values: 3

count optional

The number of potential user results to retrieve per page. This value has a maximum of 20.

Example Values: 5

include_entities optional

The entities node will be disincluded from embedded tweet objects when set to false.

Example Values: false

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = USERS_SEARCH;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(q);
	add_args(page);
	add_count_upto_20(api, &uri, count);
	add_args(include_entities);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_users_contributees (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/contributees.json
Parameters

A user_id or screen_name is required.
user_id optional

The ID of the user for whom to return results for. Helpful for disambiguating when a valid user ID is also a valid screen name.
screen_name optional

The screen name of the user for whom to return results for.
include_entities optional

The entities node will be disincluded when set to false.

Example Values: false
skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = USERS_CONTRIBUTEES;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(include_entities);
	add_args(skip_status);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int get_users_contributors (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/contributors.json
Parameters

A user_id or screen_name is required.

user_id optional

The ID of the user for whom to return results for.

screen_name optional

The screen name of the user for whom to return results for.

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

skip_status optional

When set to either true, t or 1 statuses will not be included in the returned user objects.

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = USERS_CONTRIBUTORS;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);
	add_args(include_entities);
	add_args(skip_status);

	int ret = http_request(uri, GET, res);

	free_and_assign(uri);

	return ret;
}

int post_account_remove_profile_banner (
	char **res //response
	) {
/*
Resource URL
https://api.twitter.com/1.1/account/remove_profile_banner.json
*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = ACCOUNT_REMOVE_PROFILE_BANNER;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

// POST account/update_profile_banner is too difficult to implement

int get_users_profile_banner (
	char **res, //response
	twitter_id const twitter_id //Always specify either an user_id or screen_name.
	) {
/*
Resource URL
https://api.twitter.com/1.1/users/profile_banner.json
Parameters

Always specify either an user_id or screen_name when requesting this method.

user_id optional

The ID of the user for whom to return results for. Helpful for disambiguating when a valid user ID is also a valid screen name.

Example Values: 12345

Note:: Specifies the ID of the user to befriend. Helpful for disambiguating when a valid user ID is also a valid screen name.

screen_name optional

The screen name of the user for whom to return results for. Helpful for disambiguating when a valid screen name is also a user ID.

Example Values: noradio

*/
	dbg_printf("\n");

	if (!check_keys()) {
		fprintf(stderr, "need register_keys\n");
		return 0;
	}

	if(!is_valid_id(twitter_id)) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}

	char *uri = NULL;
	api_enum api = USERS_PROFILE_BANNER;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);

	add_args(twitter_id);

	int ret = http_request(uri, POST, res);

	free_and_assign(uri);

	return ret;
}

/*--- Streaming API ---*/
