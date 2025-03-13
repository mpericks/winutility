#pragma once
#include <vector>
#include <algorithm>

template <class TSinkType> class NotificationImpl
{
public:
  void AddNotifySink( TSinkType* event_sink_ptr )
  {
    typename std::vector<TSinkType*>::const_iterator finder ;
    finder = std::find( subscribers.begin(), subscribers.end(), event_sink_ptr ) ;
    if ( finder == subscribers.end() || ! subscribers.size() ) 
    {
      subscribers.push_back( event_sink_ptr ) ;
    }   
  }
  void RemoveNotifySink( TSinkType* event_sink_ptr )
  {
     typename std::vector<TSinkType**>::const_iterator new_end = std::remove( subscribers.begin(), subscribers.end(), event_sink_ptr ) ;
     if ( new_end != subscribers.end() )
     {
        subscribers.erase( new_end, subscribers.end() ) ; 
     }
  }
private:
  virtual void Notify( TSinkType* event_sink_ptr, UINT event_id ) = 0 ;
protected:
  void CallNotifyForEachSubscriber( UINT notification_code )
  {
    for ( UINT i = 0; i < subscribers.size() ; i++ )
    {
      Notify( subscribers[i], notification_code ) ;
    }
  }

private:
  typename std::vector<TSinkType*> subscribers ;
};