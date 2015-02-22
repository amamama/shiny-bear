#ifndef __TWEET_H
#define __TWEET_H

typedef unsigned long long int tweet_id_t;
typedef long long int cursor_t;

enum {
	GET,
	POST,
};

typedef enum {
#define uri(const, str) const,
#include "api_uri.h"
NUM_OF_APIS
} api_enum;

typedef struct {
		char const *c_key;
		char const *c_sec;
		char const *t_key;
		char const *t_sec;
} oauth_keys;

oauth_keys register_keys (oauth_keys);
int check_keys(void);
oauth_keys current_keys(void);

int bear_init(char const *, char const *, char const *, char const *);
int bear_cleanup(void);

typedef enum ALIGN {
	NONE,
	LEFT,
	RIGHT,
	CENTER,
} align;

typedef struct GEOCODE {
		double latitude;
		double longitude;
		int radius;
		char *unit;
} geocode;

#define MI "mi"
#define KM "km"
#define JA "ja"

enum {
	MIXED = 1,
	RECENT = 2,
	POPULAR = 4,
}

#define SMS "sms"
#define NONE "none"

int get_statuses_mentions_timeline (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_statuses_user_timeline (
	char **res, //response
	tweet_id_t user_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	char *screen_name, //Always specify either an user_id or screen_name when requesting a user timeline.
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int exclude_replies, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_statuses_home_timeline (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int exclude_replies, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	);

int get_statuses_retweets_of_me (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int include_user_entities //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_statuses_retweets_by_id (
	tweet_id_t id, //required
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	);

int get_statuses_show_by_id (
	tweet_id_t id, //required
	char **res, //response
	int trim_user, //optional. if not -1, add it to argument.
	int include_my_retweet, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	);

int post_statuses_destroy_by_id (
	tweet_id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	);

int post_statuses_update(
	char *update, //required
	char **res, // response
	tweet_id_t in_reply_to_status_id, //optional. if not 0, add it to argument.
	int do_add_l_l, //add it. whether add l_l to argument.
	struct GEOCODE l_l, //optional. if it is valid figure, add it to argument.
	tweet_id_t place_id, //optional. if not 0, add it to argument.
	int display_coordinates, //optional. if not -1, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	);

int post_statuses_retweet_by_id (
	tweet_id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	);

//POST statuses/update_with_media is too difficult to implement

int get_statuses_oembed (
	tweet_id_t id, //required. It is not necessary to include both.
	char *url, //required. It is not necessary to include both.
	char **res, //response
	int maxwidth, //optional? It must be between 250 and 550.
	int hide_media, //optional? If not -1, add it to argument.
	int hide_thread, //optional? If not -1, add it to argument.
	int omit_script, //optional? If not -1, add it to argument.
	enum ALIGN align, //optional? If not NONE, add it to argument.
	char *related, //optional? If it is valid, add it to argument.
	char *lang //optional? If it is valid, add it to argument.
	);

int get_statuses_retweeters_ids (
	tweet_id_t id, //required
	char **res, //response
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids //optional. if not -1, add it to argument.
	);

int get_search_tweets (
	char *q, //required
	char **res, //response
	struct GEOCODE geocode, //optional. If it is valid, add it to argument.
	char *lang, //optional. If not 0, add it to argument.
	char *locale, //optional. If not 0, add it to argument. Only ja is currently effective
	int result_type, //optional. If not 0, add it to argument. 1 = "mixed",2="recent",4="popular"
	int count, //optional. If not 0, add it to argument.
	char *until, //optional. If not 0, add it to argument.
	tweet_id_t since_id, //optional. If not 0, add it to argument.
	tweet_id_t max_id, //optional. If not 0, add it to argument.
	int include_entities, //optional. If not -1, add it to argument.
	char *callback //optional. If not 0, add it to argument.
	);

int get_direct_messages (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_dm_sent (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	tweet_id_t since_id, //optional. if not 0, add it to argument.
	tweet_id_t max_id, //optional. if not 0, add it to argument.
	int pages, //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	int include_entities //optional. if not -1, add it to argument.
	);

int get_dm_show (
	tweet_id_t id, //required
	char **res //response
	);

int post_dm_destroy (
	tweet_id_t id, //required
	char **res, //response
	int include_entities //optional. if not -1, add it to argument.
	);

int post_dm_new (
	tweet_id_t user_id, //One of user_id or screen_name are required.
	char *screen_name, //One of user_id or screen_name are required.
	char *text, //required.
	char **res //response
	);

int get_fs_no_retweets_ids (
	char **res, //response
	int stringify_ids //optional. if not -1, add it to argument.
	);

int get_friends_ids (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids, //optional. if not -1, add it to argument.
	int count //optional. if not 0, add it to argument.
	);

int get_followers_ids (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids, //optional. if not -1, add it to argument.
	int count //optional. if not 0, add it to argument.
	);

int get_fs_lookup (
	char **res, //response
	char *screen_name, //optional. if not 0, add it to argument.
	char *user_id //optional. if not 0, add it to argument.
	);

int get_fs_incoming (
	char **res, //response
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids //optional. if not -1, add it to argument.
	);

int get_fs_outgoing (
	char **res, //response
	cursor_t cursor, //optional. if not 0, add it to argument.
	int stringify_ids //optional. if not -1, add it to argument.
	);

int post_fs_create (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int follow //optional. if not -1, add it to argument.
	);

int post_fs_destroy (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name //optional. if not 0, add it to argument.
	);

int post_fs_update (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int device, //optional. if not -1, add it to argument.
	int retweets //optional. if not -1, add it to argument.
	);

int get_fs_show (
	char **res, //response
	tweet_id_t source_id, //optional. if not 0, add it to argument.
	char *source_screen_name, //optional. if not 0, add it to argument.
	tweet_id_t target_id, //optional. if not 0, add it to argument.
	char *target_screen_name //optional. if not 0, add it to argument.
	);

int get_friends_list (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	cursor_t cursor, //optional. if not 0, add it to argument.
	int count, //optional. if not 0, add it to argument.
	int skip_status, //optional. if not -1, add it to argument.
	int include_user_entities //optional. if not -1, add it to argument.
	);

int get_followers_list (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	cursor_t cursor, //optional. if not 0, add it to argument.
	int count, //optional. if not 0, add it to argument.
	int skip_status, //optional. if not -1, add it to argument.
	int include_user_entities //optional. if not -1, add it to argument.
	);

int get_account_settings (
	char **res //response
	);

int get_account_verify_credentials (
	char **res, //response
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	);

int post_account_settings (
	char **res, //response
	int trend_location_woeid, //optional. if not 0, add it to argument.
	int sleep_time_enabled, //optional. if not -1, add it to argument.
	int start_sleep_time, //optional. if not -1, add it to argument.
	int end_sleep_time, //optional. if not -1, add it to argument.
	char *time_zone, //optional. if it is valid, add it to argument.
	char *lang //optional. if it is valid, add it to argument.
	);

int post_account_update_delivery_device (
	char *device, //required.
	char **res, //response
	int include_entities //optional. if not -1, add it to argument.
	);

int post_account_update_profile (
	char **res, //response
	char *name, //optional. if not 0, add it to argument.
	char *url, //optional. if not 0, add it to argument.
	char *location, //optional. if not 0, add it to argument.
	char *description, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	);

int post_account_update_profile_colors (
	char **res, //response
	long profile_background_color, //optional. if not -1, add it to argument.
	long profile_link_color, //optional. if not -1, add it to argument.
	long profile_sidebar_border_color, //optional. if not -1, add it to argument.
	long profile_sidebar_fill_color, //optional. if not -1, add it to argument.
	long profile_text_color, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	);

int get_blocks_list (
	char **res, //response
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status, //optional. if not -1, add it to argument.
	cursor_t cursor //optional. if not 0, add it to argument.
	);

int get_blocks_ids (
	char **res, //response
	int stringify_ids, //optional. if not -1, add it to argument.
	cursor_t cursor //optional. if not 0, add it to argument.
	);

int post_blocks_create (
	char **res, //response
	char *screen_name, //optional. if not 0, add it to argument.
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	);

int post_blocks_destroy (
	char **res, //response
	char *screen_name, //optional. if not 0, add it to argument.
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	);

int get_users_lookup (
	char **res, //response
	char *screen_name, //optional. if not 0, add it to argument.
	char *user_id, //optional. if not 0, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	);

int get_users_show (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	);

int get_users_search (
		char *q, //required.
	char **res, //response
	int page, //optional. if not 0, add it to argument.
	int count, //optional. if not 0, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	);

int get_users_contributees (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	);

int get_users_contributors (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument.
	);

int post_account_remove_profile_banner (
	char **res //response
	);

int get_users_profile_banner (
	char **res, //response
	tweet_id_t user_id, //optional. if not 0, add it to argument.
	char *screen_name //optional. if not 0, add it to argument.
	);


/*--- Streaming API ---*/

#endif
