#ifndef _AMIMESSAGE_H_
#define _AMIMESSAGE_H_

#include <map>

/************************************************************************/
/* Comparator for case-insensitive comparison in STL assos. containers  */
/************************************************************************/
struct ci_less : std::binary_function<std::string, std::string, bool>
{
  // case-independent (ci) compare_less binary function
  struct nocase_compare : public std::binary_function<unsigned char,unsigned char,bool> 
  {
    bool operator() (const unsigned char& c1, const unsigned char& c2) const {
        return tolower (c1) < tolower (c2); 
    }
  };
  bool operator() (const std::string & s1, const std::string & s2) const {
    return std::lexicographical_compare 
      (s1.begin (), s1.end (),   // source range
      s2.begin (), s2.end (),   // dest range
      nocase_compare ());  // comparison
  }
};

typedef std::map<std::string, std::string, ci_less> AmiMessageMap;
class AmiMessage
{
private:
	AmiMessageMap map;
public:
	std::string &operator[](std::string &key){return map[key];};
	std::string &operator[](const char *key){return map[key];};
	const std::string operator[](std::string &key) const {try{return map.at(key);}catch (std::out_of_range){return "";}};
	const std::string operator[](const char *key) const {try{return map.at(key);}catch (std::out_of_range){return "";}};
	const std::string at(const std::string &key) const {try{return map.at(key);}catch (std::out_of_range){return "";}};
	AmiMessageMap getMap() const {return map;};
	void clear(){map.clear();};

	typedef AmiMessageMap::iterator iterator;
	typedef AmiMessageMap::const_iterator const_iterator;
	iterator begin() { return map.begin(); }
	const_iterator begin() const { return map.begin(); }
	iterator end() { return map.end(); }
	const_iterator end() const { return map.end(); }
};

#endif
