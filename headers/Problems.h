#pragma once

enum MODE
{
    TEST, MAIN1, OSC
};

class Problem {
private:
    MODE mode;

public:
    Problem() : mode(TEST) {}
    Problem(MODE _mode) : mode(_mode) {}

    double func(double x) const;
    double deriv(double x) const;
    double deriv2(double x) const;
    
    // Add getMode() method to access the private mode member
    MODE getMode() const { return mode; }
};