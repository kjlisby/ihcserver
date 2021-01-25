
#ifndef DOMOTICZSWITCH_H
#define DOMOTICZSWITCH_H

class DomoticzSwitch {
public:
	virtual ~DomoticzSwitch(){};
	std::string getName() { return m_name; };
	int getIDX() { return m_idx; };
	int getModuleNumber() { return m_moduleNumber; };
	int getIONumber() { return m_ioNumber; };
	bool isOutput() { return m_isOutput; };
	
	friend bool operator==(const DomoticzSwitch &lhs, const DomoticzSwitch &rhs){
		return (lhs.m_isOutput == rhs.m_isOutput &&
		        lhs.m_idx == rhs.m_idx &&
				lhs.m_moduleNumber == rhs.m_moduleNumber &&
				lhs.m_ioNumber == rhs.m_ioNumber);
	}

protected:
	DomoticzSwitch(bool isOutput, std::string name, int idx, int moduleNumber, int ioNumber) :
		m_isOutput(isOutput),
		m_name(name),
		m_idx(idx),
		m_moduleNumber(moduleNumber),
		m_ioNumber(ioNumber)
	{};


private:
	bool m_isOutput;
	std::string m_name;
    int m_idx;
	int m_moduleNumber;
	int m_ioNumber;
	
	friend class DomoticzInterface;
};

#endif /* DOMOTICZSWITCH_H */
