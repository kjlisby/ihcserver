/**
 * This class models an input in the IHC system
 *
 * December 2012, Martin Hejnfelt (martin@hejnfelt.com)
 */
#ifndef IHCINPUT_H
#define IHCINPUT_H
#include "IHCIO.h"

class IHCInput : public IHCIO {
public:
	int getInputNumber() { return getIONumber(); };
protected:
	IHCInput(int moduleNumber, int inputNumber) :
		IHCIO(moduleNumber, inputNumber)
	{};
	friend class IHCInterface;
};

#endif /* IHCINPUT_H */
