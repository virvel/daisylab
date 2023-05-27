
class Knot {

    public:
        void Init();
        float Process1(float);
        float Process2(float);
        float Process3(float);
        void setFreq(float);
        void setQ(float q) {q_ = q;}

    private:
        float frequency_;
        float m_;
        float n_;
        float h_;
        float p_;
        float q_;
        float t_;
};
