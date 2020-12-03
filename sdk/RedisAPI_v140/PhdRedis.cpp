#include "PhdRedis.h"



PhdRedis::~PhdRedis()
{
	if (m_pRedisContext)
	{
		redisFree(m_pRedisContext);
		m_pRedisContext = nullptr;
	}
	if (m_pRedisReply)
	{
		freeReplyObject(m_pRedisReply);
		m_pRedisReply = nullptr;
	}
}

bool PhdRedis::ConnectRedis(const std::string& strIP /*= "127.0.0.1"*/, 
	int nPort /*= 6379*/, const std::string& strPwd /*= ""*/)
{
	if (m_pRedisContext)
		return true;

	//连接redis
	m_pRedisContext = redisConnect(strIP.c_str(), nPort);
	if (m_pRedisContext->err)
	{
		redisFree(m_pRedisContext);
		m_pRedisContext = nullptr;
		return false;
	}

	if (strPwd != "")
	{
		//输入redis密码
		RedisReplyToNull();
		m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "AUTH %s", strPwd.c_str());
		if (!m_pRedisReply)
			return false;
		if (!(m_pRedisReply->type == REDIS_REPLY_STATUS && _stricmp(m_pRedisReply->str, "OK") == 0))
			return false;
	}

	return true;
}

bool PhdRedis::SwitchDb(int nDbName)
{
	if (!m_pRedisContext)
		return false;

	RedisReplyToNull();
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "select %d", nDbName);
	if (!m_pRedisReply)
		return false;
	if (!(m_pRedisReply->type == REDIS_REPLY_STATUS && _stricmp(m_pRedisReply->str, "OK") == 0))
		return false;

	return true;
}

bool PhdRedis::SetKeyValue(const std::string& strKey, const std::string& strValue)
{
	if (!m_pRedisContext)
		return false;

	//判断该key是否存在
	RedisReplyToNull();
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "get %s", strKey.c_str());
	if (!m_pRedisReply)
		return false;
	if (m_pRedisReply->type == REDIS_REPLY_NIL)
	{//不存在
		RedisReplyToNull();
		m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "set %s %s", strKey.c_str(),strValue.c_str());
		if (!m_pRedisReply)
			return false;
		if (!(m_pRedisReply->type == REDIS_REPLY_STATUS && _stricmp(m_pRedisReply->str, "OK") == 0))
			return false;
	}
	else
	{//存在
		RedisReplyToNull();
		m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "append %s %s", strKey.c_str(), strValue.c_str());
		if (!m_pRedisReply)
			return false;
		if (!(m_pRedisReply->type == REDIS_REPLY_INTEGER && m_pRedisReply->integer > 0))
			return false;
	}

	return true;
}

bool PhdRedis::GetKeyValue(const std::string& strKey, std::string& strValue) 
{
	if (!m_pRedisContext)
		return false;

	RedisReplyToNull();
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "get %s", strKey.c_str());
	if (!m_pRedisReply)
		return false;
	//是否查询到
	if (m_pRedisReply->type == REDIS_REPLY_NIL)
		return false;
	//value是否是字符串
	if (m_pRedisReply->type != REDIS_REPLY_STRING)
		return false;
	
	strValue = m_pRedisReply->str;
	return true;
}

bool PhdRedis::GetAllKeys(const std::string& strFindKey, std::vector<std::string>& vecKeys)
{
	if (!m_pRedisContext)
		return false;

	RedisReplyToNull();
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "keys %s", strFindKey.c_str());
	if (!m_pRedisReply)
		return false;
	if (m_pRedisReply->type == REDIS_REPLY_NIL)
		return false;
	if (m_pRedisReply->type != REDIS_REPLY_ARRAY)
		return false;
	std::size_t nCount = m_pRedisReply->elements;
	for (std::size_t i = 0; i < nCount; i++)
	{
		redisReply* pReply = m_pRedisReply->element[i];
		std::string strKey = pReply->str;
		if (strKey == "")
			continue;
		vecKeys.push_back(strKey);
	}

	return true;
}

bool PhdRedis::ExecuteCommand(const std::string& strCommand)
{
	if (!m_pRedisContext)
		return false;

	RedisReplyToNull();
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "%s", strCommand.c_str());
	if (!m_pRedisReply)
		return false;

	return true;
}

bool PhdRedis::SetKeyExpire(const std::string& strKey, int nExpire)
{
	if (!m_pRedisContext)
		return false;

	RedisReplyToNull();
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "expire %s %d", strKey.c_str(),nExpire);
	if (!m_pRedisReply)
		return false;
	if (!(m_pRedisReply->type = REDIS_REPLY_INTEGER && m_pRedisReply->integer == 1))
		return false;
	
	return true;
}

bool PhdRedis::GetKeyValueLength(const std::string& strKey, int& nLength)
{
	if (!m_pRedisContext)
		return false;

	RedisReplyToNull();
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "strlen %s", strKey.c_str());
	if (!m_pRedisReply)
		return false;
	//是否查询到
	if (m_pRedisReply->type == REDIS_REPLY_NIL)
		return false;
	//返回的是否是整型
	if (m_pRedisReply->type != REDIS_REPLY_INTEGER)
		return false;

	nLength = m_pRedisReply->integer;
	return true;
}

bool PhdRedis::GetKeyTll(const std::string& strKey, int& nTll)
{
	if (!m_pRedisContext)
		return false;

	RedisReplyToNull();
	m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, "ttl %s", strKey.c_str());
	if (!m_pRedisReply)
		return false;
	//是否查询到
	if (m_pRedisReply->type == REDIS_REPLY_NIL)
		return false;
	//返回的是否是整型
	if (m_pRedisReply->type != REDIS_REPLY_INTEGER)
		return false;

	nTll = m_pRedisReply->integer;
	return true;
}

void PhdRedis::RedisReplyToNull()
{
	if (m_pRedisReply)
	{
		freeReplyObject(m_pRedisReply);
		m_pRedisReply = nullptr;
	}
}
