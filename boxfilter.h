#ifndef BOXFILTER_H
#define BOXFILTER_H

struct BoxFilter{
private:
    int SIZE;
public:
    double box[5][5];
    void set_size(int size){ if (size > 0 && size <= 5) SIZE = size; }
    int size(){ return SIZE; }
};

#endif // BOXFILTER_H
