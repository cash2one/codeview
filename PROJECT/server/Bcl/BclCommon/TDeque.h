
#ifndef __BCL_TDEQUE_H__
#define __BCL_TDEQUE_H__

#include "boost/thread.hpp"
#include <vector>

template<typename DequeType,typename HandlerType>
class TActiveDeque
{
public:
	typedef typename DequeType::value_type value_type;

	TActiveDeque() : m_Handler()
   {
   }
 
   ~TActiveDeque()
   {
	   for(int i=0; i< m_vectThread.size();i++)
	   {
		   if( m_vectThread[i])
		   {
			   delete  m_vectThread[i];
		   }
	   }

	    m_vectThread.clear();
   }

    bool Start( HandlerType   Handler ,int ThreadNum=1)
   {
	   m_Handler = Handler;

	   for(int i=0; i<ThreadNum;++i)
		   m_vectThread.push_back(new boost::thread(boost::bind(&TActiveDeque<DequeType,HandlerType>::thread_run,this)));

	   return true;

   }

   bool Put(value_type & value)
   {
	   return m_Deque.Put(value);
   }
   bool Get(value_type & value)
   {
	   return m_Deque.Get(value);
   }

   	//遍历所有,注意:该函数不是线程安全的，必须在竞争消除后再调用。
	bool Next(value_type & value)
	{
		return m_Deque.Next(value);
	}

   void Stop()
   {
	   m_Deque.Stop();
	   for(int i=0; i< m_vectThread.size();i++)
	   {
		   if( m_vectThread[i])
		   {
			   m_vectThread[i]->join();
		   }
	   }
   }

   bool IsStop()
   {
	   return m_Deque.IsStop();
   }

private:
	//线程运行函数
int thread_run()
{
	value_type data;
	while(1)
	{
		if(m_Deque.Get(data))
		{
			m_Handler(data);
		}
		else if(m_Deque.IsStop())
		{
			break;
		}
	}
	return 0;
}

private:
	DequeType m_Deque;
	std::vector<boost::thread *>  m_vectThread; //线程
	HandlerType m_Handler;

};


#endif

