#pragma once
#include <hiredis.h>
#include <string>

//加载连接库
#pragma comment (lib,"hiredis.lib")
#pragma comment (lib,"Win32_Interop.lib")

class PhdRedis
{
public:
	PhdRedis():m_pRedisContext(nullptr),m_pRedisReply(nullptr)
	{}
	~PhdRedis();

	//************************************
	// Summary:  连接redis数据库
	// Parameter:
	//      strIP -    输入数据库ip地址
	//      nPort -    输入数据库端口号
	//      strPwd -    输入数据库密码
	// Update Time: 2020-11-16 18:10:55
	//************************************
	bool ConnectRedis(const std::string& strIP = "127.0.0.1",int nPort = 6379,const std::string& strPwd = "");

	//************************************
	// Summary:  切换redis数据库
	// Parameter:
	//	  nDbName -	输入数据库的名字
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 12:50:40
	//************************************
	bool SwitchDb(int nDbName);

	//************************************
	// Summary:  设置key的value
	// Parameter:
	//	  strKey -	输入key
	//	  strValue -	输入value
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 12:55:40
	//************************************
	bool SetKeyValue(const std::string& strKey, const std::string& strValue);

	//************************************
	// Summary:  得到key的value
	// Parameter:
	//	  strKey -	输入key
	//	  strValue -	输出value
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:06:23
	//************************************
	bool GetKeyValue(const std::string& strKey, std::string& strValue);

	//************************************
	// Summary:  执行命令语句
	// Parameter:
	//	  strCommand -	
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:19:07
	//************************************
	bool ExecuteCommand(const std::string& strCommand);

	//************************************
	// Summary:  得到当前redis回复指针
	// Parameter:
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:21:24
	//************************************
	inline redisReply* GetCurReplyPtr() const {
		return m_pRedisReply;
	}

public:

	//************************************
	// Summary:  设置key的生命周期
	// Parameter:
	//	  strKey -	输入key
	//	  nExpire -	输入生命周期时间（秒）
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 18:55:58
	//************************************
	bool SetKeyExpire(const std::string& strKey, int nExpire);

	//************************************
	// Summary:  得到key的value长度
	// Parameter:
	//	  strKey -	输入key
	//	  nLength -	输出value的长度
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:12:34
	//************************************
	bool GetKeyValueLength(const std::string& strKey, int& nLength);

	//************************************
	// Summary:  得到key的剩余时间（秒）
	// Parameter:
	//	  strKey -	输入key
	//	  nTll -	输出key的剩余时间
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:16:19
	//************************************
	bool GetKeyTll(const std::string& strKey, int& nTll);

private:
	//将m_pRedisReply置为null
	void RedisReplyToNull();

private:
	redisContext* m_pRedisContext;		//redis上下文示例
	redisReply* m_pRedisReply;			//redis答复示例
};