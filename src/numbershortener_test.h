#include <cxxtest/TestSuite.h>

#include "amimessage.h"
#define _OBSERVER_H_
class IObservable{
    public: void Notify(const AmiMessage &m){};
            IObservable(){};
            ~IObservable(){};
};
class IObserver
{
public:
    IObserver(){};
    virtual void handleEvent(const AmiMessage &) = 0;
};

#include "numbershortener.h"
#include "numbershortener.cpp"

void AmiMessageModifier::handleEvent(const AmiMessage &m){};

class TestNumberShortener : public CxxTest::TestSuite
{
public:
    void test_shorten1()
    {
        ShortenNumberModifier s("/83832/2/,/83833/3/");
        std::string n;

        n = "83832123456";
        s.shorten(n);
        TS_ASSERT_EQUALS(n, "2123456");

        n = "83833123456";
        s.shorten(n);
        TS_ASSERT_EQUALS(n, "3123456");

        n = "83834123456";
        s.shorten(n);
        TS_ASSERT_EQUALS(n, "83834123456");
    }

    void test_shorten2()
    {
        ShortenNumberModifier s("/+7/8/");
        std::string n = "+79131234567";
        s.shorten(n);
        TS_ASSERT_EQUALS(n, "89131234567");
    }

    void test_shorten3()
    {
        ShortenNumberModifier s("/1//");
        std::string n = "11";
        s.shorten(n);
        TS_ASSERT_EQUALS(n, "1");
        s.shorten(n);
        TS_ASSERT_EQUALS(n, "");
    }

    void test_shorten4()
    {
        ShortenNumberModifier s("//22");
        std::string n = "11";
        s.shorten(n);
        TS_ASSERT_EQUALS(n, "11");
    }
};
