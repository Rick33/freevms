#ifndef DEVICETEST_H
#define DEVICETEST_H

int call_sys$getdviw1(unsigned short int code, void *value, unsigned short *length);

int call_sys$getdviw2(unsigned short int code1, void *value1, unsigned short *length1, unsigned short int code2, void *value2,
        unsigned short *length2);

void run_device_test(void);

#endif /* DEVICETEST_H */