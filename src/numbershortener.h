#ifndef _NUMBERSHORTENER_H_
#define _NUMBERSHORTENER_H_

#include "filter.h"

class ShortenNumberModifier : public AmiMessageModifier
{
private:
    std::vector<std::string> long_prefixes;
    std::vector<std::string> short_prefixes;
public:
    explicit ShortenNumberModifier(const std::string &ShorteningRules);
    bool CreateShorteningRules(const std::string &);
    void handleEvent(const AmiMessage &);
    void shorten(std::string &);
};

#endif
