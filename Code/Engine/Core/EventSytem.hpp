#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include <mutex>
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class NamedProperties;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
typedef NamedProperties EventArgs;
typedef bool (*EventCallBackFunction)(EventArgs& eventArgs);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class EventSubscription
{
public:
	EventSubscription() = default;
	virtual ~EventSubscription() = default;

	virtual bool IsSameFunction(void* otherFunction) const = 0;
	virtual bool Execute(EventArgs&) const = 0;
	virtual bool BelongsToObject(void* object) const { object; return false; }
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class EventFuncSubscription : public EventSubscription
{
public:
	EventFuncSubscription(EventCallBackFunction callbackFunction) : m_callbackFunction(callbackFunction) {}

	virtual bool Execute(EventArgs& eventArgs) const override 
	{
		if (m_callbackFunction) 
		{
			return m_callbackFunction(eventArgs);
		}
		return false; // Return false if the function pointer was null
	}

	virtual bool IsSameFunction(void* otherFunction) const override
	{
		EventCallBackFunction otherFunc = reinterpret_cast<EventCallBackFunction>(otherFunction);
		return m_callbackFunction == otherFunc;
	}

	EventCallBackFunction m_callbackFunction = nullptr;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename T_ObjectType>
class EventMethodSubscription : public EventSubscription 
{
	friend class EventSystem;
	typedef bool(T_ObjectType::* MethodType)(EventArgs& args);
public:
	
	EventMethodSubscription(T_ObjectType* objectInstance, MethodType subscribedMethod) :
		m_objectInstance(objectInstance),
		m_method(subscribedMethod) {}

	virtual bool Execute(EventArgs& eventArgs) const override 
	{
		return ((*m_objectInstance).*m_method)(eventArgs);
	}

	virtual bool IsSameFunction(void* otherFunction) const override 
	{
		MethodType* otherFuncAsMethodType = reinterpret_cast<MethodType*>(otherFunction);
		return (m_method) == *otherFuncAsMethodType;
	}

	virtual bool BelongsToObject(void* object) const override
	{
		return m_objectInstance == object;
	}


private:
	T_ObjectType* m_objectInstance = nullptr;
	MethodType m_method = nullptr;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct EventSystemConfig
{

};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
extern EventSystem* g_theEventSystem;
typedef std::vector<EventSubscription*> SubscriptionList;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class EventSystem
{
public:
    EventSystem(EventSystemConfig const& config);
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr);
	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr);
    void UnsubscribeFromAllEvents(EventCallBackFunction functionPtr);
    void FireEvent(std::string const& eventName, EventArgs& args);
	void FireEvent(std::string const& eventName);
 
    void GetNamesOfAllEvents(std::vector<std::string>& eventNames);

	template<typename T_ObjectType> 
	inline void UnsubscribeAllEventCallbackFunctions(T_ObjectType* objectInstance);

	template<typename T_ObjectType, typename MethodType>
	inline void SubscribeEventCallbackFunction(std::string const& eventName, T_ObjectType* objectInstance, MethodType functionPtr);

	template<typename T_ObjectType, typename MethodType>
	inline void UnsubscribeEventCallbackFunction(std::string const& eventName, T_ObjectType* objectInstance, MethodType functionPtr);

	template<typename T_ObjectType, typename MethodType> // Unsubscribe specific object's methods from all events
	inline void UnsubscribeAllEventCallbackFunctions(T_ObjectType* objectInstance, MethodType functionPtr);

protected:
	EventSystemConfig						m_config;
	std::map<std::string, SubscriptionList> m_subscriptionListsByEventName;
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename MethodType>
void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, T_ObjectType* objectInstance, MethodType functionPtr)
{
	EventMethodSubscription<T_ObjectType>* newSubscription = new EventMethodSubscription<T_ObjectType>(objectInstance, functionPtr);


	std::map<std::string, SubscriptionList>::iterator iter = m_subscriptionListsByEventName.find(eventName);
	if (iter == m_subscriptionListsByEventName.end()) 
	{
		SubscriptionList eventSubList;
		eventSubList.push_back(newSubscription);
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
		eventSubList.push_back(newSubscription);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename T_ObjectType>
void EventSystem::UnsubscribeAllEventCallbackFunctions(T_ObjectType* objectInstance)
{
	for (auto it = m_subscriptionListsByEventName.begin(); it != m_subscriptionListsByEventName.end(); it++) 
	{
		SubscriptionList& subList = it->second;
		if (subList.size() == 0) 
			continue;

		for (auto subListIt = subList.begin(); subListIt != subList.end();) 
		{
			EventSubscription*& eventSub = *subListIt;
			if (eventSub->BelongsToObject(objectInstance))
			{
				subListIt = subList.erase(subListIt);
			}
			else 
			{
				subListIt++;
			}
		}
	}
}

template<typename T_ObjectType, typename MethodType>
void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, T_ObjectType* objectInstance, MethodType functionPtr)
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
			EventSubscription*& eventSub = eventSubList[subIndex];
			if (eventSub->BelongsToObject(objectInstance)) 
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
}

template<typename T_ObjectType, typename MethodType>
void EventSystem::UnsubscribeAllEventCallbackFunctions(T_ObjectType* objectInstance, MethodType functionPtr)
{
	for (auto it = m_subscriptionListsByEventName.begin(); it != m_subscriptionListsByEventName.end(); it++)
	{
		SubscriptionList& subList = it->second;
		if (subList.size() == 0) 
			continue;

		for (auto subListIt = subList.begin(); subListIt != subList.end();) 
		{
			EventSubscription*& eventSub = *subListIt;
			
			if (eventSub->BelongsToObject(objectInstance)) 
			{
				if (eventSub->IsSameFunction(&functionPtr)) 
				{
					subListIt = subList.erase(subListIt);
				}
				else
				{
					subListIt++;
				}
			}
			
			else
			{
				subListIt++;
			}
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------





template<typename T_ObjectType, typename MethodType>
void SubscribeEventCallbackFunction(std::string const& eventName, T_ObjectType* objectInstance, MethodType functionPtr)
{
	if (g_theEventSystem) 
	{
		g_theEventSystem->SubscribeEventCallbackFunction(eventName, objectInstance, functionPtr);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename MethodType>
void UnsubscribeEventCallbackFunction(std::string const& eventName, T_ObjectType* objectInstance, MethodType functionPtr)
{
	if (g_theEventSystem) 
	{
		g_theEventSystem->UnsubscribeEventCallbackFunction(eventName, objectInstance, functionPtr);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename T_ObjectType, typename MethodType>
void UnsubscribeAllEventCallbackFunctions(T_ObjectType* objectInstance, MethodType functionPtr)
{
	if (g_theEventSystem) 
	{
		g_theEventSystem->UnsubscribeAllEventCallbackFunctions(objectInstance, functionPtr);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
template<typename T_ObjectType>
void UnsubscribeAllEventCallbackFunctions(T_ObjectType* objectInstance)
{
	if (g_theEventSystem) 
	{
		g_theEventSystem->UnsubscribeAllEventCallbackFunctions(objectInstance);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------

//standalone global-namespace helper functions, these forward to "the" event system, if it exists
void SubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr);
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr);
void UnsubscribeFromAllEvents(EventCallBackFunction functionPtr);
void FireEvent(std::string const& eventName, EventArgs& args);
void FireEvent(std::string const& eventName);
void GetNamesOfAllEvents(std::vector<std::string>& eventNames);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
class EventRecipient
{
public:
	virtual ~EventRecipient()
	{
		UnsubscribeAllEventCallbackFunctions(this);
	}
};
//--------------------------------------------------------------------------------------------------------------------------------------------------------