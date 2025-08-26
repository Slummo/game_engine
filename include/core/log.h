#pragma once

#include <iostream>
#include <string>

#define LOG(x) std::cout << x << std::endl;
#define ERR(x) std::cerr << x << std::endl;

class Printable {
public:
    virtual ~Printable() = default;

    friend std::ostream& operator<<(std::ostream& os, const Printable& p) {
        return p.print(os);
    }

protected:
    virtual std::ostream& print(std::ostream& os) const = 0;
};