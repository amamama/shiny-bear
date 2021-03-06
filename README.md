shiny-bear
==========

multi-platform twitter API library written in C.
Based on [twicpps](http://www.soramimi.jp/twicpps/index.html).

add your keys
---

You must add your OAuth keys to your source.

```c
	char const *c_key = "your consumer key";
	char const *c_sec = "your consuer secret";
	char const *t_key = "your token key";
	char const *t_sec = "your token secret";
	bear_init(c_key, c_sec, t_key, t_sec);
```
c\_key/sec is consumer\_key/secret, t\_key/sec is token\_key/secret.

compile
---

Before compilation, you need gcc, make, liboauth and libcurl.

```sh
$ make
```

That's all.

APIs
---

Currently this library has wrappers for following twitter REST APIs:

**Timelines**
- [statuses/mentions_timeline](https://dev.twitter.com/docs/api/1.1/get/statuses/mentions_timeline)
- [statuses/user_timeline](https://dev.twitter.com/docs/api/1.1/get/statuses/user_timeline)
- [statuses/home_timeline](https://dev.twitter.com/docs/api/1.1/get/statuses/home_timeline)
- [statuses/retweets_of_me](https://dev.twitter.com/docs/api/1.1/get/statuses/retweets_of_me)

**Tweets**
- [statuses/retweets/:id](https://dev.twitter.com/docs/api/1.1/get/statuses/retweets/%3Aid)
- [statuses/show/:id](https://dev.twitter.com/docs/api/1.1/get/statuses/show/%3Aid)
- [statuses/destroy/:id](https://dev.twitter.com/docs/api/1.1/post/statuses/destroy/%3Aid)
- [statuses/update](https://dev.twitter.com/docs/api/1.1/post/statuses/update)
- [statuses/retweet/:id](statuses/retweet/:id)
- ~~[statuses/update_with_media](https://dev.twitter.com/docs/api/1.1/post/statuses/update_with_media)~~
- [statuses/oembed](https://dev.twitter.com/docs/api/1.1/get/statuses/oembed)
- [statuses/retweeters/ids](statuses/retweeters/ids)

**Search**
- [search/tweets](https://dev.twitter.com/docs/api/1.1/get/search/tweets)

**Streaming**

**Direct Messages**
- [direct_messages](https://dev.twitter.com/docs/api/1.1/get/direct_messages)
- [direct_messages/sent](https://dev.twitter.com/docs/api/1.1/get/direct_messages/sent)
- [direct_messages/show](https://dev.twitter.com/docs/api/1.1/get/direct_messages/show)
- [direct_messages/destroy](https://dev.twitter.com/docs/api/1.1/post/direct_messages/destroy)
- [direct_messages/new](https://dev.twitter.com/docs/api/1.1/post/direct_messages/new)

**Friends & Followers**
- [friendships/no_retweets/ids](https://dev.twitter.com/docs/api/1.1/get/friendships/no_retweets/ids)
- [friends/ids](https://dev.twitter.com/docs/api/1.1/get/friends/ids)
- [followers/ids](https://dev.twitter.com/docs/api/1.1/get/followers/ids)
- [friendships/incoming](https://dev.twitter.com/docs/api/1.1/get/friendships/incoming)
- [friendships/outgoing](https://dev.twitter.com/docs/api/1.1/get/friendships/outgoing)
- [friendships/create](https://dev.twitter.com/docs/api/1.1/post/friendships/create)
- [friendships/destroy](https://dev.twitter.com/docs/api/1.1/post/friendships/destroy)
- [friendships/update](https://dev.twitter.com/docs/api/1.1/post/friendships/update)
- [friendships/show](https://dev.twitter.com/docs/api/1.1/get/friendships/show)
- [friends/list](https://dev.twitter.com/docs/api/1.1/get/friends/list)
- [followers/list](https://dev.twitter.com/docs/api/1.1/get/followers/list)
- [friendships/lookup](https://dev.twitter.com/docs/api/1.1/get/friendships/lookup)

**Users**
- [account/settings](https://dev.twitter.com/docs/api/1.1/get/account/settings)
- [account/verify_credentials](https://dev.twitter.com/docs/api/1.1/get/account/verify_credentials)
- [account/settings](https://dev.twitter.com/docs/api/1.1/post/account/settings)
- [account/update_delivery_device](https://dev.twitter.com/docs/api/1.1/post/account/update_delivery_device)
- [account/update_profile](https://dev.twitter.com/docs/api/1.1/post/account/update_profile)
- ~~[account/update_profile_background_image](https://dev.twitter.com/docs/api/1.1/post/account/update_profile_background_image)~~
- [account/update_profile_colors](https://dev.twitter.com/docs/api/1.1/post/account/update_profile_colors)
- ~~[account/update_profile_image](https://dev.twitter.com/docs/api/1.1/post/account/update_profile_image)~~
- [blocks/list](https://dev.twitter.com/docs/api/1.1/get/blocks/list)
- [blocks/ids](https://dev.twitter.com/docs/api/1.1/get/blocks/ids)
- [blocks/create](https://dev.twitter.com/docs/api/1.1/post/blocks/create)
- [blocks/destroy](https://dev.twitter.com/docs/api/1.1/post/blocks/destroy)
- [users/lookup](https://dev.twitter.com/docs/api/1.1/get/users/lookup)
- [users/show](https://dev.twitter.com/docs/api/1.1/get/users/show)
- [users/search](https://dev.twitter.com/docs/api/1.1/get/users/search)
- [users/contributees](https://dev.twitter.com/docs/api/1.1/get/users/contributees)
- [users/contributors](https://dev.twitter.com/docs/api/1.1/get/users/contributors)
- [account/remove_profile_banner](https://dev.twitter.com/docs/api/1.1/post/account/remove_profile_banner)
- ~~[account/update_profile_banner](https://dev.twitter.com/docs/api/1.1/post/account/update_profile_banner)~~
- [users/profile_banner](https://dev.twitter.com/docs/api/1.1/get/users/profile_banner)
