//---------------------------
// Include Files
//---------------------------
#include "Subject.h"

#include "Observer.h"


//---------------------------
// Member functions
//---------------------------

void Subject::AddObserver(Observer* observer)
{
	m_Observers.push_back(observer);
}

void Subject::RemoveObserver(Observer* observer)
{
	std::erase(m_Observers, observer);
}

void Subject::NotifyObservers(const Event& event, dae::GameObject* actor)
{
	for (auto* observer : m_Observers)
	{
		if (observer)
		observer->Notify(event, actor);
	}
}