#ifndef __TWEET_H
#define __TWEET_H

typedef enum {
#define uri(const, str) const,
#include "api_uri.h"
#undef uri
NUM_OF_APIS
} api_enum;

typedef struct {
		char const *c_key;
		char const *c_sec;
		char const *t_key;
		char const *t_sec;
} oauth_keys;

oauth_keys register_keys(oauth_keys);
int check_keys(void);
oauth_keys current_keys(void);

enum {
	GET,
	POST,
};

int bear_init(oauth_keys);
int bear_cleanup(void);

typedef int64_t tweet_id_t;
typedef struct {
	tweet_id_t const max_id;
	tweet_id_t const since_id;
} max_and_since;
typedef int64_t user_id_t;
typedef struct {
	user_id_t const user_id;
	char const *screen_name;
} twitter_id;
typedef int64_t cursor_t;

typedef enum {
	NONE,
	LEFT,
	RIGHT,
	CENTER,
} align;

typedef struct {
		double const latitude;
		double const longitude;
		int const radius;
		char const *unit;
} geocode;

#define MI "mi"
#define KM "km"
#define JA "ja"

enum {
	MIXED = 1,
	RECENT = 2,
	POPULAR = 4,
};

#define SMS "sms"
#define NONE "none"

int get_statuses_mentions_timeline (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const const max_and_since, //optional. if not 0, add it to argument.
	int const trim_user, //optional. if not -1, add it to argument.
	int const contributor_details, //optional. if not -1, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_statuses_user_timeline (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	int const count, //optional. if not 0, add it to argument.
	max_and_since const const max_and_since, //optional. if not 0, add it to argument.
	int const trim_user, //optional. if not -1, add it to argument.
	int const exclude_replies, //optional. if not -1, add it to argument.
	int const contributor_details, //optional. if not -1, add it to argument.
	int const include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_statuses_home_timeline (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const const max_and_since, //optional. if not 0, add it to argument.
	int const trim_user, //optional. if not -1, add it to argument.
	int const exclude_replies, //optional. if not -1, add it to argument.
	int const contributor_details, //optional. if not -1, add it to argument.
	int const include_entities //optional. if not -1, add it to argument.
	);

int get_statuses_retweets_of_me (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const trim_user, //optional. if not -1, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const include_user_entities //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_statuses_retweets_by_id (
	tweet_id_t const id, //required
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	int const trim_user //optional. if not -1, add it to argument.
	);

int get_statuses_show_by_id (
	tweet_id_t const id, //required
	char **res, //response
	int const trim_user, //optional. if not -1, add it to argument.
	int const include_my_retweet, //optional. if not -1, add it to argument.
	int const include_entities //optional. if not -1, add it to argument.
	);

int post_statuses_destroy_by_id (
	tweet_id_t const id, //required
	char **res, //response
	int const trim_user //optional. if not -1, add it to argument.
	);

int post_statuses_update(
	char const *update, //required
	char **res, // response
	tweet_id_t const in_reply_to_status_id, //optional. if not 0, add it to argument.
	int const do_add_l_l, //add it. whether add l_l to argument.
	geocode l_l, //optional. if it is valid figure, add it to argument.
	char const *place_id, //optional. if not 0, add it to argument.
	int const display_coordinates, //optional. if not -1, add it to argument.
	int const trim_user //optional. if not -1, add it to argument.
	);

int post_statuses_retweet_by_id (
	tweet_id_t const id, //required
	char **res, //response
	int const trim_user //optional. if not -1, add it to argument.
	);

//POST statuses/update_with_media is too difficult to implement

int get_statuses_oembed (
	tweet_id_t const id, //required. It is not necessary to include both.
	char const *url, //required. It is not necessary to include both.
	char **res, //response
	int const maxwidth, //optional? It must be between 250 and 550.
	int const hide_media, //optional? If not -1, add it to argument.
	int const hide_thread, //optional? If not -1, add it to argument.
	int const omit_script, //optional? If not -1, add it to argument.
	align align, //optional? If not NONE, add it to argument.
	char const *related, //optional? If it is valid, add it to argument.
	char const *lang //optional? If it is valid, add it to argument.
	);

int get_statuses_retweeters_ids (
	tweet_id_t const id, //required
	char **res, //response
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids //optional. if not -1, add it to argument.
	);

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
	);

int get_direct_messages (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_dm_sent (
	char **res, //response
	int const count, //optional. if not 0, add it to argument.
	max_and_since const max_and_since, //optional. if not 0, add it to argument.
	int const pages, //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	int const include_entities //optional. if not -1, add it to argument.
	);

int get_dm_show (
	tweet_id_t const id, //required
	char **res //response
	);

int post_dm_destroy (
	tweet_id_t const id, //required
	char **res, //response
	int const include_entities //optional. if not -1, add it to argument.
	);

int post_dm_new (
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	char const *text, //required.
	char **res //response
	);

int get_fs_no_retweets_ids (
	char **res, //response
	int const stringify_ids //optional. if not -1, add it to argument.
	);

int get_friends_ids (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids, //optional. if not -1, add it to argument.
	int const count //optional. if not 0, add it to argument.
	);

int get_followers_ids (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids, //optional. if not -1, add it to argument.
	int const count //optional. if not 0, add it to argument.
	);

int get_fs_lookup (
	char **res, //response
	char const *screen_name, //optional. if not 0, add it to argument.
	char const *user_id //optional. if not 0, add it to argument.
	);

int get_fs_incoming (
	char **res, //response
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids //optional. if not -1, add it to argument.
	);

int get_fs_outgoing (
	char **res, //response
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const stringify_ids //optional. if not -1, add it to argument.
	);

int post_fs_create (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	int const follow //optional. if not -1, add it to argument.
	);

int post_fs_destroy (
	char **res, //response
	twitter_id const twitter_id //Always specify either an user_id or screen_name when requesting a user timeline.
	);

int post_fs_update (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	int const device, //optional. if not -1, add it to argument.
	int const retweets //optional. if not -1, add it to argument.
	);

int get_fs_show (
	char **res, //response
	twitter_id const source, //Always specify either an user_id or screen_name when requesting a user timeline.
	twitter_id const target //Always specify either an user_id or screen_name when requesting a user timeline.
	);

int get_friends_list (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const count, //optional. if not 0, add it to argument.
	int const skip_status, //optional. if not -1, add it to argument.
	int const include_user_entities //optional. if not -1, add it to argument.
	);

int get_followers_list (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	cursor_t const cursor, //optional. if not 0, add it to argument.
	int const count, //optional. if not 0, add it to argument.
	int const skip_status, //optional. if not -1, add it to argument.
	int const include_user_entities //optional. if not -1, add it to argument.
	);

int get_account_settings (
	char **res //response
	);

int get_account_verify_credentials (
	char **res, //response
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	);

int post_account_settings (
	char **res, //response
	int const trend_location_woeid, //optional. if not 0, add it to argument.
	int const sleep_time_enabled, //optional. if not -1, add it to argument.
	int const start_sleep_time, //optional. if not -1, add it to argument.
	int const end_sleep_time, //optional. if not -1, add it to argument.
	char const *time_zone, //optional. if it is valid, add it to argument.
	char const *lang //optional. if it is valid, add it to argument.
	);

int post_account_update_delivery_device (
	char const *device, //required.
	char **res, //response
	int const include_entities //optional. if not -1, add it to argument.
	);

int post_account_update_profile (
	char **res, //response
	char const *name, //optional. if not 0, add it to argument.
	char const *url, //optional. if not 0, add it to argument.
	char const *location, //optional. if not 0, add it to argument.
	char const *description, //optional. if not 0, add it to argument.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	);

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
	);

// POST account/update_profile_image is too difficult to implement

int get_blocks_list (
	char **res, //response
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status, //optional. if not -1, add it to argument.
	cursor_t const cursor //optional. if not 0, add it to argument.
	);

int get_blocks_ids (
	char **res, //response
	int const stringify_ids, //optional. if not -1, add it to argument.
	cursor_t const cursor //optional. if not 0, add it to argument.
	);

int post_blocks_create (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	);

int post_blocks_destroy (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	);

int get_users_lookup (
	char **res, //response
	char const *screen_name, //optional. if not 0, add it to argument.
	char const *user_id, //optional. if not 0, add it to argument.
	int const include_entities //optional. if not -1, add it to argument.
	);

int get_users_show (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	int const include_entities //optional. if not -1, add it to argument.
	);

int get_users_search (
	char const *q, //required.
	char **res, //response
	int const page, //optional. if not 0, add it to argument.
	int const count, //optional. if not 0, add it to argument.
	int const include_entities //optional. if not -1, add it to argument.
	);

int get_users_contributees (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	);

int get_users_contributors (
	char **res, //response
	twitter_id const twitter_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	int const include_entities, //optional. if not -1, add it to argument.
	int const skip_status //optional. if not -1, add it to argument.
	);

int post_account_remove_profile_banner (
	char **res //response
	);

// POST account/update_profile_banner is too difficult to implement

int get_users_profile_banner (
	char **res, //response
	twitter_id const twitter_id //Always specify either an user_id or screen_name when requesting a user timeline.
	);


/*--- Streaming API ---*/

#endif
