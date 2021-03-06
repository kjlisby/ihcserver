#include "Userlevel.h"
#include "Configuration.h"
#include <openssl/sha.h>
#include <list>
#include <cstdio>
#include "IHCServerDefs.h"

class Userlevel::UserlevelToken {
public:
	UserlevelToken(){};
	virtual ~UserlevelToken(){};
};

class Level {
public:
	Level(std::string code_sha1, Userlevel::Levels level) :
		m_code_sha1(code_sha1),
		m_token(),
		m_level(level)
	{};

	std::string m_code_sha1;
	Userlevel::UserlevelToken m_token;
	Userlevel::Levels m_level;
};

#include <sstream>
#include <iostream>
#include <iomanip>
std::list<Level*> levels;

Level* basic = 0;

void Userlevel::init() {
	if(levels.empty()) {
		std::string adminpw_sha = Configuration::getInstance()->getValue(IHCServerDefs::ADMIN_SHA1_CONFKEY);
		if(adminpw_sha == "") {
			unsigned char* md = new unsigned char[SHA_DIGEST_LENGTH];
			unsigned char* pw = (unsigned char*)"12345678";
			SHA1(pw,8,md);
			std::stringstream ist;
			for(unsigned int k=0; k<SHA_DIGEST_LENGTH; k++) {
				ist << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) md[k];
			}
			adminpw_sha = ist.str();
			delete[] md;
		}
		std::string superuserpw_sha = Configuration::getInstance()->getValue(IHCServerDefs::SUPERUSER_SHA1_CONFKEY);
		if(superuserpw_sha == "") {
			unsigned char* md = new unsigned char[SHA_DIGEST_LENGTH];
			unsigned char* pw = (unsigned char*)"1234";
			SHA1(pw,4,md);
			std::stringstream ist;
			for(unsigned int k=0; k<SHA_DIGEST_LENGTH; k++) {
				ist << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) md[k];
			}
			superuserpw_sha = ist.str();
			delete[] md;
		}

		basic = new Level("",BASIC);
		levels.push_back(new Level(superuserpw_sha,SUPERUSER));
		levels.push_back(new Level(adminpw_sha,ADMIN));
	}
}

void Userlevel::setCode(enum Userlevel::Levels level, std::string code) {
	std::string var = "";
	if(level == Userlevel::ADMIN) {
		var = IHCServerDefs::ADMIN_SHA1_CONFKEY;
	} else if(level == Userlevel::SUPERUSER) {
		var = IHCServerDefs::SUPERUSER_SHA1_CONFKEY;
	}
	if(var == "") {
		return;
	}
        unsigned char* md = new unsigned char[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)code.c_str(),code.size(),md);
        std::stringstream ist;
        for(unsigned int k=0; k<SHA_DIGEST_LENGTH; k++) {
                ist << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) md[k];
        }
        std::string code_sha1 = ist.str();
        delete[] md;

        std::list<Level*>::const_iterator it = levels.begin();
	for(;it != levels.end(); it++) {
		if(level == (*it)->m_level) {
			(*it)->m_code_sha1 = code_sha1;
			Configuration::getInstance()->setValue(var,code_sha1);
			Configuration::getInstance()->save();
		}
	}
}

void Userlevel::setCodeSHA(Userlevel::Levels level, std::string codeSHA)
{
	std::string var = "";
	if(level == Userlevel::ADMIN) {
		var = IHCServerDefs::ADMIN_SHA1_CONFKEY;
	} else if(level == Userlevel::SUPERUSER) {
		var = IHCServerDefs::SUPERUSER_SHA1_CONFKEY;
	}
	if(var == "") {
		return;
	}
	std::list<Level*>::const_iterator it = levels.begin();
	for(;it != levels.end(); it++) {
		if(level == (*it)->m_level) {
			(*it)->m_code_sha1 = codeSHA;
			Configuration::getInstance()->setValue(var,codeSHA);
			Configuration::getInstance()->save();
		}
	}
}

void Userlevel::login(Userlevel::UserlevelToken *&token, std::string code) {
	unsigned char* md = new unsigned char[SHA_DIGEST_LENGTH];
	SHA1((unsigned char*)code.c_str(),code.size(),md);
	std::stringstream ist;
	for(unsigned int k=0; k<SHA_DIGEST_LENGTH; k++) {
		ist << std::hex << std::setw(2) << std::setfill('0') << (unsigned int) md[k];
	}
	std::string code_sha1 = ist.str();
	delete[] md;
	loginSHA(token,code_sha1);
}

void Userlevel::loginSHA(Userlevel::UserlevelToken*& token, std::string codeSHA)
{
	std::list<Level*>::const_iterator it = levels.begin();
	for(;it != levels.end(); it++) {
		if(codeSHA == (*it)->m_code_sha1) {
			token = &(*it)->m_token;
			break;
		}
	}
	if(it == levels.end()) {
		token = &basic->m_token;
	}
}

enum Userlevel::Levels Userlevel::getUserlevel(Userlevel::UserlevelToken* &token) {
	if(token == NULL) return Userlevel::BASIC;

	std::list<Level*>::iterator it = levels.begin();

	for(;it != levels.end(); it++) {
		if(token == &(*it)->m_token) {
			return (*it)->m_level;
		}
	}
	return Userlevel::BASIC;
}

std::string Userlevel::tokenToString(Userlevel::UserlevelToken*& token)
{
	std::string levelString = "BASIC";
	if(token == NULL) return levelString;
	std::list<Level*>::const_iterator it = levels.begin();
	for(;it != levels.end(); it++) {
		if(token == &(*it)->m_token) {
			switch((*it)->m_level) {
				case ADMIN:
					levelString = "ADMIN";
				break;
				case SUPERUSER:
					levelString = "SUPERUSER";
				break;
				case BASIC:
				default:
					levelString = "BASIC";
			}
			break;
		}
	}
	return levelString;
}
