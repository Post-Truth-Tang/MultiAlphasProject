//
// Created by tqz_trader on 2022/11/16.
//

#ifndef ALPHAPROJECT_TYPES_H
#define ALPHAPROJECT_TYPES_H

#define MKTDEPTH 5
//VOI,OIR这类基础因子个数
#define PLAIN_ALPHA_NUM 500
//avg(5,VOI),avg(10,VOI)这类基础因子高级函数个数
#define PLAIN_ALPHA_FUNC_NUM (PLAIN_ALPHA_NUM * 10)
//高级函数最多的参数数量
#define ALPHA_FUNC_MAX_PARAMS 6

#include "Stack.h"
#include <boost/interprocess/mapped_region.hpp>
using namespace boost::interprocess;

struct TaskData {
    virtual ~TaskData() {
        if (valueStack) {
            delete valueStack;
            valueStack = nullptr;
        }
    }
    TaskData() = default;

    unsigned int startIndex{};
    unsigned int endIndex{};

    unsigned int coreNumber{};

    Stack *valueStack = new Stack(1000);

    mapped_region ** read_write_sharedMemories{};
    int** alphaFuncs2{};
    int* sizes2{};
    double** ticks{};
    double* plainAlphas{};

    double* alphaFuncFlexValue{};

};

struct Snapshot {
    int dsrc;
    int security;
    long timestamp;
    int level;
    double last;
    double bid;
    double bid1;
    double bid2;
    double bid3;
    double bid4;
    double ask;
    double ask1;
    double ask2;
    double ask3;
    double ask4;
    int bz;
    int bz1;
    int bz2;
    int bz3;
    int bz4;
    int az;
    int az1;
    int az2;
    int az3;
    int az4;
    int volume;
    int acc_volume;
    double turnover;
    double acc_turnover;
};

struct Signal {
	Signal() : counter(0) {};
	// counter
	long counter;
	// previous data
	double low_limit;
	double high_limit;

	double bid_lis[MKTDEPTH];
	double ask_lis[MKTDEPTH];
	int bz_lis[MKTDEPTH];
	int az_lis[MKTDEPTH];

	double bid1;
	double ask1;

	int bz1;
	int bz2;
	int bz3;
	int bz4;
	int bz5;

	int az1;
	int az2;
	int az3;
	int az4;
	int az5;

	double last_price;
	double mid_price;

	double last_bid_lis[MKTDEPTH];
	double last_ask_lis[MKTDEPTH];
	int last_bz_lis[MKTDEPTH];
	int last_az_lis[MKTDEPTH];

	int last_bz1;
	int last_az1;
	double last_bid1;
	double last_ask1;

	double last2_bid_lis[MKTDEPTH];
	double last2_ask_lis[MKTDEPTH];
	int last2_bz_lis[MKTDEPTH];
	int last2_az_lis[MKTDEPTH];
	double last3_bid_lis[MKTDEPTH];
	double last3_ask_lis[MKTDEPTH];
	int last3_bz_lis[MKTDEPTH];
	int last3_az_lis[MKTDEPTH];
	
	Queue ask1_queue, bid1_queue, az1_queue, bz1_queue, volume_queue, vwap_queue, mid_queue;
	Queue predefined_alpha_queue[PLAIN_ALPHA_NUM];
	// theo price
	double prev_theo;
	double theo;

	double vwap;
	double price_up;
	double price_down;

	double last_vwap;
	int acc_volume;
	int last_acc_volume;
	double acc_turnover;
	double last_acc_turnover;

	//补上缺失的
	double first_mid;
	long timestamp;
	int volume;
	//当前的因子值
	double plainAlphas[PLAIN_ALPHA_NUM];
};

struct Param{
	double multiplier;
	double price_tick;
};
#endif //ALPHAPROJECT_TYPES_H
