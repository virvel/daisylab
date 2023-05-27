#include "knot.h"

void Knot::Init() {
    frequency_ = 0.5f;
    m_ = 0.5f;
    n_ = 0.5f;
    h_ = 1.f;
    p_ = 2.f;
    q_ = -3.f;
    t_ = 4.f;
}


float Knot::Process1(float theta) {
    return m_ * cos(p_ * theta) + n_ * cos(q_ * theta);;
}

float Knot::Process2(float theta) {
    return m_ * sin(p_ * theta) + n_ * cos(q_ * theta);
}

float Knot::Process3(float theta) {
    return h_ * sin(t_ * theta); 
}
