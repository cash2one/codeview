
#ifndef __BCL_SWITCH_SEQUENCE_H__
#define __BCL_SWITCH_SEQUENCE_H__

//当有数据时，通知
template<typename ConditionVarType> 
class TSwitchSequenceNoticfy
{
public:
	//当有数据进队
	void NotifyOne()
	{
		m_ReadConditionVar.notify_one(); //通知有数据可读
	}

	//当没有数据可读
	template<typename MutexType> 
	bool OnNotData(MutexType & mutex)
	{
		m_ReadConditionVar.wait(mutex);
		return true;
	}
private:
	ConditionVarType  m_ReadConditionVar;  //可读条件变量
};


template<typename SequenceType,typename MutexType,typename EventNotifyType> 
class TSwitchSequence
{
public:

	typedef typename SequenceType::value_type  value_type;

	TSwitchSequence(EventNotifyType * pEventNotifyType=new EventNotifyType(),bool bDelete=true)
	{
		m_pInSequence  = &m_SequenceFirst;
		m_pOutSequence = &m_SequenceSecond;
		m_bStop = false;
		m_pEventNotify = pEventNotifyType;
		m_bDelete = bDelete;
	}

	~TSwitchSequence()
	{
		if(m_bDelete && m_pEventNotify)
		{
			delete m_pEventNotify;
			m_pEventNotify = 0;
		}
	}

	bool Put(value_type & value)
	{
	 	if(m_bStop) return false;	
		bool bEmpty = false;
		m_mutex.lock();
		bEmpty = m_pInSequence->empty();
		m_pInSequence->push_back(value);	
		m_mutex.unlock();
		if(bEmpty && m_pEventNotify)
		{
			m_pEventNotify->NotifyOne(); //通知有数据可读
		}
		return true;
	}

	bool Get(value_type & value)
	{
		if(m_bStop) return false;	


		if(m_pOutSequence->empty())
		{
			SequenceType *pList = m_pInSequence;

			if(m_pEventNotify==0)
			{
				m_mutex.lock();
				if(m_pInSequence->empty())
				{
					m_mutex.unlock();
					return false;
				}

				m_pInSequence = m_pOutSequence;

				m_mutex.unlock();
			}
			else
			{
				m_mutex.lock();

				while(m_bStop==false && m_pInSequence->empty())
				{
					if(m_pEventNotify->OnNotData(m_mutex)==false)
					{
						m_mutex.unlock();
						return false;
					}
				}			

				m_pInSequence = m_pOutSequence;

				m_mutex.unlock();
			}



			m_pOutSequence = pList;

			if(m_bStop) return false;		

			value = m_pOutSequence->front();

			m_pOutSequence->pop_front();
		}
		else
		{
			size_t count =  m_pOutSequence->size();

			value = m_pOutSequence->front();

			m_pOutSequence->pop_front();
		}

		return true;
	}

	//遍历所有,注意:该函数不是线程安全的，必须在竞争消除后再调用。
	bool Next(value_type & value)
	{
		if(m_bStop==false)
		{
			return false;
		}

		if(m_pOutSequence->empty())
		{		
			if(m_pInSequence->empty())
			{
				return false;
			}
			SequenceType *pList = m_pInSequence;
			m_pInSequence = m_pOutSequence;
			m_pOutSequence = pList;		
		}		

		value = m_pOutSequence->front();

		m_pOutSequence->pop_front();

		return true;
	}

	void Stop()
	{
		m_bStop = true;
		m_pEventNotify->NotifyOne(); //通知有数据可读
	}

	bool IsStop()
	{
		return m_bStop;
	}

private:
	MutexType m_mutex; //锁

	EventNotifyType * m_pEventNotify;  //事件通知

	SequenceType m_SequenceFirst;
	SequenceType m_SequenceSecond;

	SequenceType * m_pInSequence;
	SequenceType * m_pOutSequence;
	bool m_bStop;
	bool m_bDelete;

};

#endif

