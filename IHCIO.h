#ifndef IHCIO_H
#define IHCIO_H
#include "utils/Subject.h"

class IHCIO : public Subject {
public:
	virtual ~IHCIO(){};
	int getModuleNumber() { return m_moduleNumber; };
	int getIONumber() { return m_ioNumber; };
	bool getState() { return m_state; };

protected:
	IHCIO(int moduleNumber, int ioNumber) :
		m_moduleNumber(moduleNumber),
		m_ioNumber(ioNumber),
		m_state(false)
	{};

private:
        void setState(bool newState) {
                bool oldState = m_state;
                m_state = newState;
                if(m_state != oldState) {
                        notify();
                };
        };

	int m_moduleNumber;
	int m_ioNumber;
	bool m_state;

	friend class IHCServer;
};

#endif /* IHCIO_H */
