#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "DomoticzInterface.h"
#include "DomoticzSwitch.h"
#include "Configuration.h"
#include "utils/printutil.h"
#include "GetRequest.h"
#include <curl/curl.h>
#include "3rdparty/cajun-2.0.2/json/reader.h"
#include "3rdparty/cajun-2.0.2/json/writer.h"
#include "3rdparty/cajun-2.0.2/json/elements.h"


/**********************************************************************************************
 *
 *********************************************************************************************/
DomoticzInterface* DomoticzInterface::m_instance = NULL;
pthread_mutex_t DomoticzInterface::m_instanceMutex = PTHREAD_MUTEX_INITIALIZER;
DomoticzInterface* DomoticzInterface::getInstance() {
	pthread_mutex_lock(&m_instanceMutex);
	if(m_instance == NULL) {
		m_instance = new DomoticzInterface();
	}
	pthread_mutex_unlock(&m_instanceMutex);
	return m_instance;
};

/**********************************************************************************************
 *
 *********************************************************************************************/
DomoticzInterface::DomoticzInterface(void) {
	m_ihcServer    = IHCServer::getInstance();
	m_ihcInterface = m_ihcServer->getIHCInterface();
	m_configuration = Configuration::getInstance();
	m_domoticzServer = m_configuration->getDomoticzServer();
	if (m_domoticzServer.length() > 0) {
		printf("Configured for Domoticz: \"%s\"",m_domoticzServer.c_str());
		InitListeners();
		InitDomoticzSwitches();
		StartDomoticzSynchronizer();
	} else {
		printf("Not configured for Domoticz (empty Domoticz server name");
	}
}

/**********************************************************************************************
 *
 *********************************************************************************************/
void DomoticzInterface::update(Subject* sub, void* obj) {
	IHCIO* io;
	if(dynamic_cast<IHCOutput*>(sub) != 0) {
		io = (IHCOutput*)sub;
		//printTimeStamp();
		//printf("DomoticzInterface OUTPUT %d.%d changed to %s\n",io->getModuleNumber(),io->getIONumber(),io->getState()?"ON":"OFF");
		UpdateDomoticz(m_domoticzOutputs[io->getModuleNumber()][io->getIONumber()]->getIDX(), io->getState());
	} else if(dynamic_cast<IHCInput*>(sub) != 0) {
		io = (IHCInput*)sub;
		//printTimeStamp();
		//printf("DomoticzInterface INPUT %d.%d changed to %s\n",io->getModuleNumber(),io->getIONumber(),io->getState()?"ON":"OFF");
		UpdateDomoticz(m_domoticzInputs[io->getModuleNumber()][io->getIONumber()]->getIDX(), io->getState());
	}
}

/**********************************************************************************************
 *
 *********************************************************************************************/
void DomoticzInterface::InitListeners() {
	// Attach to all IHC I/Os
	std::list<IHCInput*> inputs = m_ihcInterface->getAllInputs();
	std::list<IHCInput*>::iterator input = inputs.begin();
	for(; input != inputs.end(); ++input) {
		(*input)->attach(this);
	}

	std::list<IHCOutput*> outputs = m_ihcInterface->getAllOutputs();
	std::list<IHCOutput*>::iterator output = outputs.begin();
	for(; output != outputs.end(); ++output) {
		(*output)->attach(this);
	}
}

/**********************************************************************************************
 * Get the idx of all virtual switches in Domoticz that correspond to IHC inputs and outputs.
 *********************************************************************************************/
void DomoticzInterface::InitDomoticzSwitches() {
    GetRequest* m_httpRequest;
    std::string URL = "http://"+m_domoticzServer+"/json.htm?type=command&param=getlightswitches";

    m_httpRequest = new GetRequest(&URL[0]);
    std::string domoticzLightSwitchDefinitions = m_httpRequest->getResponse();
    std::istringstream domLSwDefStream;
    domLSwDefStream.str(domoticzLightSwitchDefinitions);
//    cout << "domoticzLightSwitchDefinitions:" << endl << domoticzLightSwitchDefinitions << endl;
    json::UnknownElement obj;
    json::Reader::Read(obj,domLSwDefStream);
    json::Object conf = (json::Object)obj;
    json::Array result = conf["result"];
    json::Array::const_iterator it;
    int next_output_index = 0;
    int next_input_index = 0;
    for(it = result.Begin(); it != result.End(); it++) {
		try {
			json::Object lightSwitch = json::Object(*it);
			std::string name = json::String(lightSwitch["Name"]).Value();
			std::string idxstr = json::String(lightSwitch["idx"]).Value();
			int idx = atoi(idxstr.c_str());
			//std::cout << "Examining Domoticz switch. Name: " << name << " idx: " << idx << std::endl;
			//std::cout << "   Name compare: " << name.compare(0,4,"IHC_") << std::endl;
			if (name.compare(0,4,"IHC_")==0) {
				bool isOutput = name.compare(3,3,"_O_")==0;
				int delimiterPos = name.find_last_of('_');
				std::string moduleNumberString = name.substr(6,delimiterPos-6);
				int moduleNumber = atoi(moduleNumberString.c_str());
				std::string ioNumberString = name.substr(delimiterPos+1,2);
				int ioNumber = atoi(ioNumberString.c_str());
				DomoticzSwitch* ds = new DomoticzSwitch(isOutput, name, idx, moduleNumber, ioNumber);
				if (isOutput) {
					//std::cout << "   adding Domoticz output switch " << moduleNumber << " " << ioNumber << " with idx " << idx << std::endl;
					m_domoticzOutputs[moduleNumber][ioNumber] = ds;
				}else {
					//std::cout << "   adding Domoticz input switch " << moduleNumber << " " << ioNumber << " with idx " << idx << std::endl;
					m_domoticzInputs[moduleNumber][ioNumber] = ds;
				}
			}
		} catch (...) {
			// ignore any parsing error - by that we will only ignore mis-named switches in Domoticz
		}
    }
    delete m_httpRequest;
}

/**********************************************************************************************
 * Call the Domoticz JSON interface to update a virtual switch with known idx.
 * Note that Domoticz will always execute whatever script is associated with the virtual switch,
 * which typiclly would mean that it switches the IHC input/output on/off as it already is.
 *********************************************************************************************/
void DomoticzInterface::UpdateDomoticz(int idx, bool status) {
	//printf("DomoticzInterface::UpdateDomoticz %d %s\n",idx,status?"ON":"OFF");
    GetRequest* m_httpRequest;
    char URL[200];
    sprintf(URL,"http://%s/json.htm?type=command&param=udevice&idx=%i%s",m_domoticzServer.c_str(),idx,status?"&nvalue=1&svalue=On":"&nvalue=0&svalue=Off");
    std::cout << URL << std::endl;
    m_httpRequest = new GetRequest(URL);
    std::string response = m_httpRequest->getResponse();
}

/**********************************************************************************************
 * Update Domoticz with correct status for all outputs. Do so every 30 minutes.
 *********************************************************************************************/
void* DomoticzInterface::DomoticzSynchronizer(void* domo_if) {
	DomoticzInterface *domoticzInterfaceObject = (DomoticzInterface *)domo_if;
	while (true) {
		sleep(30*60);
		printf("Updating state of outputs in Domoticz\n");
		for (std::map<int,std::map<int,DomoticzSwitch*> >::iterator mmodule_it=domoticzInterfaceObject->GetDomoticzOutputs()->begin(); mmodule_it!=domoticzInterfaceObject->GetDomoticzOutputs()->end(); ++mmodule_it) {
			//std::cout << mmodule_it->first << std::endl;
			for (std::map<int,DomoticzSwitch*>::iterator ioNumber_it=mmodule_it->second.begin(); ioNumber_it!=mmodule_it->second.end(); ++ioNumber_it) {
				//std::cout << "  " << ioNumber_it->first << std::endl;
				int module   = ioNumber_it->second->getModuleNumber();
				int ioNumber = ioNumber_it->second->getIONumber();
				int idx = ioNumber_it->second->getIDX();
				bool status  = domoticzInterfaceObject->m_ihcInterface->getOutput(module,ioNumber)->getState();
				//std::cout << module << " " << ioNumber << " " << idx << " " << status << std::endl;
				domoticzInterfaceObject->UpdateDomoticz(idx, status);
			}
		}
	}
}
void DomoticzInterface::StartDomoticzSynchronizer() {
	pthread_t thread;
	int rc;
	rc = pthread_create(&thread, NULL, DomoticzInterface::DomoticzSynchronizer, this);
	if (rc) {
		std::cout << "Error:unable to create thread," << rc << std::endl;
		exit(-1);
	}
}
