#include "Engine/Core/EventSytem.hpp"
#include "Engine/Core/EngineCommon.hpp"  
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedProperties.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
EventSystem* g_theEventSystem = nullptr;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
EventSystem::EventSystem(EventSystemConfig const& config)
:m_config(config)
{

}
 
//--------------------------------------------------------------------------------------------------------------------------------------------------------
EventSystem::~EventSystem()
{
  
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::Startup()
{
        
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::Shutdown()
{
  
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::BeginFrame()
{
  
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::EndFrame()
{
  
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr)
{
	EventFuncSubscription* newSubscription = new EventFuncSubscription(functionPtr);
	std::map<std::string, SubscriptionList>::iterator iter = m_subscriptionListsByEventName.find(eventName);
	
	if (iter == m_subscriptionListsByEventName.end())
	{
		SubscriptionList eventSubList;
		eventSubList.emplace_back(newSubscription);
		m_subscriptionListsByEventName[eventName] = eventSubList;
	}
	
	else
	{
		SubscriptionList& eventSubList = iter->second;
		for (int subIndex = 0; subIndex < eventSubList.size(); subIndex++)
		{
			if (eventSubList[subIndex]->IsSameFunction(&functionPtr)) 
			{
				ERROR_AND_DIE("THERE WAS AN ATTEMPT TO DOUBLE SUBSCRIBE A FUNCTION TO AN EVENT");
			}
		}

		eventSubList.emplace_back(newSubscription);
	}
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr)
{
	std::map<std::string, SubscriptionList>::iterator iter = m_subscriptionListsByEventName.find(eventName);
	if (iter == m_subscriptionListsByEventName.end())
	{
		return;
	}

	else 
	{
		SubscriptionList& eventSubList = iter->second;
		for (int subIndex = 0; subIndex < eventSubList.size(); subIndex++) 
		{
			if (eventSubList[subIndex]->IsSameFunction(&functionPtr))
			{
				delete eventSubList[subIndex];
				eventSubList.erase(eventSubList.begin() + subIndex);
				return;
			}
		}
	}
	
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeFromAllEvents(EventCallBackFunction functionPtr)
{
	for (auto& eventNameSubListPair : m_subscriptionListsByEventName)
	{
		SubscriptionList& subs = eventNameSubListPair.second;
		for (auto iter = subs.begin(); iter != subs.end(); )
		{
			if ((*iter)->IsSameFunction(static_cast<void*>(functionPtr)))
			{
				delete* iter;
				iter = subs.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::FireEvent(std::string const& eventName, EventArgs& args)
{
	auto findIter = m_subscriptionListsByEventName.find(eventName);
	if (findIter != m_subscriptionListsByEventName.end())
	{
		for (EventSubscription* subscription : findIter->second)
		{
			if (subscription->Execute(args))
				break; 
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::GetNamesOfAllEvents(std::vector<std::string>& eventNames)
{
	eventNames.reserve(m_subscriptionListsByEventName.size());
	for (std::map<std::string, SubscriptionList>::const_iterator iter = m_subscriptionListsByEventName.begin(); iter != m_subscriptionListsByEventName.end(); iter++)
	{
		SubscriptionList const& eventSubList = iter->second;
		if (eventSubList.size() > 0)
		{
			eventNames.push_back(iter->first);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void EventSystem::FireEvent(std::string const& eventName)
{
	EventArgs args;
	FireEvent(eventName, args);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------





//-------------------------------------------------------------------------------------------------------------------------------------------------------- 
// //Static Functions------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void SubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->SubscribeEventCallbackFunction(eventName, functionPtr);
	}
 
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->UnsubscribeEventCallbackFunction(eventName, functionPtr);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void UnsubscribeFromAllEvents(EventCallBackFunction functionPtr)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->UnsubscribeFromAllEvents(functionPtr);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void FireEvent(std::string const& eventName, EventArgs& args)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->FireEvent(eventName, args);
	}
 
}
  
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void FireEvent(std::string const& eventName)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->FireEvent(eventName);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void GetNamesOfAllEvents(std::vector<std::string>& eventNames)
{
	if (g_theEventSystem)
	{
		g_theEventSystem->GetNamesOfAllEvents(eventNames);
	}
}

