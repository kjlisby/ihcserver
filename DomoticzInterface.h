/**
 * This is the main interface to Domoticz, i.e. the class that keep the state of all
 * IHC switches updated in Domoticz.
 * Communication the other way (i.e. Domoticz sending switch commands towards IHCServer)
 * is done via scripts in Domoticz.
 *
 * September 2018, Karl Johan Lisby (kjlisby @ Github)
 */

 #ifndef DOMOTICZINTERFACE_H
#define DOMOTICZINTERFACE_H
#include "utils/Observer.h"
#include "IHCServer.h"
#include "IHCInterface.h"
#include "DomoticzSwitch.h"

class DomoticzInterface : public Observer {
public:
	static DomoticzInterface* getInstance();
	~DomoticzInterface();
	
	// We get notified by IHCInputs and IHCOutputs here
	void update(Subject* sub, void* obj);
	
private:
	static pthread_mutex_t m_instanceMutex;
	static DomoticzInterface* m_instance;
	DomoticzInterface();
	void InitListeners();
	void InitDomoticzSwitches();
	static void* DomoticzSynchronizer(void*);
	void StartDomoticzSynchronizer();
	void UpdateDomoticz(int idx, bool status);
	std::map<int,std::map<int,DomoticzSwitch*> >* GetDomoticzOutputs() {
		return &m_domoticzOutputs;
	}
	
	// The interface to the IHC controller
	IHCServer*    m_ihcServer;
	IHCInterface* m_ihcInterface;

	// The instance of the configuration
	Configuration* m_configuration;
	std::string m_domoticzServer;
	
	// All switches in Domoticz
	std::map<int,std::map<int,DomoticzSwitch*> > m_domoticzInputs;
	std::map<int,std::map<int,DomoticzSwitch*> > m_domoticzOutputs;
};

#endif /* DOMOTICZINTERFACE_H */