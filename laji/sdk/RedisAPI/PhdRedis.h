#pragma once
#include <hiredis.h>
#include <string>

//�������ӿ�
#pragma comment (lib,"hiredis.lib")
#pragma comment (lib,"Win32_Interop.lib")

class PhdRedis
{
public:
	PhdRedis():m_pRedisContext(nullptr),m_pRedisReply(nullptr)
	{}
	~PhdRedis();

	//************************************
	// Summary:  ����redis���ݿ�
	// Parameter:
	//      strIP -    �������ݿ�ip��ַ
	//      nPort -    �������ݿ�˿ں�
	//      strPwd -    �������ݿ�����
	// Update Time: 2020-11-16 18:10:55
	//************************************
	bool ConnectRedis(const std::string& strIP = "127.0.0.1",int nPort = 6379,const std::string& strPwd = "");

	//************************************
	// Summary:  �л�redis���ݿ�
	// Parameter:
	//	  nDbName -	�������ݿ������
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 12:50:40
	//************************************
	bool SwitchDb(int nDbName);

	//************************************
	// Summary:  ����key��value
	// Parameter:
	//	  strKey -	����key
	//	  strValue -	����value
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 12:55:40
	//************************************
	bool SetKeyValue(const std::string& strKey, const std::string& strValue);

	//************************************
	// Summary:  �õ�key��value
	// Parameter:
	//	  strKey -	����key
	//	  strValue -	���value
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:06:23
	//************************************
	bool GetKeyValue(const std::string& strKey, std::string& strValue);

	//************************************
	// Summary:  ִ���������
	// Parameter:
	//	  strCommand -	
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:19:07
	//************************************
	bool ExecuteCommand(const std::string& strCommand);

	//************************************
	// Summary:  �õ���ǰredis�ظ�ָ��
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
	// Summary:  ����key����������
	// Parameter:
	//	  strKey -	����key
	//	  nExpire -	������������ʱ�䣨�룩
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 18:55:58
	//************************************
	bool SetKeyExpire(const std::string& strKey, int nExpire);

	//************************************
	// Summary:  �õ�key��value����
	// Parameter:
	//	  strKey -	����key
	//	  nLength -	���value�ĳ���
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:12:34
	//************************************
	bool GetKeyValueLength(const std::string& strKey, int& nLength);

	//************************************
	// Summary:  �õ�key��ʣ��ʱ�䣨�룩
	// Parameter:
	//	  strKey -	����key
	//	  nTll -	���key��ʣ��ʱ��
	// Return:	 
	// Notice:   
	// Update Time: 2020-11-21 19:16:19
	//************************************
	bool GetKeyTll(const std::string& strKey, int& nTll);

private:
	//��m_pRedisReply��Ϊnull
	void RedisReplyToNull();

private:
	redisContext* m_pRedisContext;		//redis������ʾ��
	redisReply* m_pRedisReply;			//redis��ʾ��
};