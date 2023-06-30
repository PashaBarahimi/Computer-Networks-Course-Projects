#ifndef NEW_RENO_HPP_INCLUDE
#define NEW_RENO_HPP_INCLUDE

#include "reno.hpp"

class NewReno : public Reno {
public:
    using Reno::Reno;

    void sendData() override;
};

#endif // NEW_RENO_HPP_INCLUDE
