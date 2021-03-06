#include "support/access.h"

namespace webdev
  {

  using std::string;
  using std::vector;
  using redox::Redox;

  namespace
    {
    vector<shout> shouts_get(Redox & redis, vector<string> const & ids)
      {
      auto shouts = vector<shout>{};

      for_each(ids.cbegin(), ids.cend(), [&](auto const & id){
        auto shout = shout_get(redis, id);

        if(shout.text().size() && shout.user().name().size())
          {
          shouts.push_back(shout);
          }
        });

      return shouts;
      }

    }

  bool user_exists(Redox & redis, user const & user)
    {
    return redis.commandSync<int>({"SISMEMBER", "users", user.hash()}).reply();
    }

  bool user_exists(Redox & redis, string const & id)
    {
    return redis.commandSync<int>({"SISMEMBER", "users", id}).reply();
    }

  bool user_create(Redox & redis, user const & user)
    {
    return redis.commandSync<string>({"HMSET", "user:" + user.hash(), "name", user.name()}).ok() &&
           redis.commandSync<int>({"SADD", "users", user.hash()}).ok();
    }

  user user_get_by_id(Redox & redis, std::string const & id)
    {
    return user{redis.commandSync<string>({"HGET", "user:" + id, "name"}).reply()};
    }

  vector<string> users_getall(Redox & redis)
    {
    auto & result = redis.commandSync<vector<string>>({"SMEMBERS", "users"});
    return result.ok() ? result.reply() : vector<string>{};
    }

  bool session_exists(Redox & redis, string const & sessionId)
    {
    return redis.commandSync<int>({"SISMEMBER", "sessions", sessionId}).reply();
    }

  bool session_store(Redox & redis, string const & sessionId)
    {
    return redis.commandSync<int>({"SADD", "sessions", sessionId}).reply();
    }

  bool session_remove(Redox & redis, string const & sessionId)
    {
    return redis.commandSync<int>({"SREM", "sessions", sessionId}).reply();
    }

  bool shout_create(Redox & redis, shout const & shout)
    {
    return redis.commandSync<string>({"HMSET", "shout:" + shout.hash(), "text", shout.text(), "user", shout.user().hash()}).ok() &&
           redis.commandSync<int>({"LPUSH", "shouts:" + shout.user().hash(), shout.hash()}).ok() &&
           redis.commandSync<int>({"LPUSH", "shouts", shout.hash()}).ok();
    }

  shout shout_get(Redox & redis, string const & id)
    {
    auto & shoutResult = redis.commandSync<vector<string>>({"HMGET", "shout:" + id, "text", "user"});

    if(shoutResult.ok())
      {
      auto text = shoutResult.reply()[0];
      auto userId = shoutResult.reply()[1];

      return {text, user_get_by_id(redis, userId)};
      }

    return {"", user{string{}}};
    }

  vector<shout> shouts_getall_for_user(Redox & redis, string const & id)
    {
    auto & result = redis.commandSync<vector<string>>({"LRANGE", "shouts:" + id, "0", "-1"});

    if(result.ok())
      {
      return shouts_get(redis, result.reply());
      }

    return {};
    }

  vector<shout> shouts_getall(Redox & redis)
    {
    auto & result = redis.commandSync<vector<string>>({"LRANGE", "shouts", "0", "-1"});

    if(result.ok())
      {
      return shouts_get(redis, result.reply());
      }

    return {};
    }

  }

