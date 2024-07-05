//
// Created by tqz_trader on 2022/11/16.
//

#ifndef ALPHAPROJECT_LRPRED_H
#define ALPHAPROJECT_LRPRED_H

#include "types.h"
#include <string>
#include <vector>
#include <map>
#include "Stack.h"

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
using namespace boost::interprocess;

#define ALPHA_FUNC_MAX_PARAMS_ADD1 (ALPHA_FUNC_MAX_PARAMS + 1)

using namespace std;

class LRPred {
public:
    explicit LRPred(const char* target);
    ~LRPred();

    LRPred(const LRPred &) = delete;
    LRPred& operator=(const LRPred &) = delete;

public:
    bool InitPred();
    void SetCoreCounts(unsigned int cores);
    void UpdateTick(const Snapshot &tick);
    void UpdateAlphas();
    bool Update(const Snapshot &tick);
	void UpdatePrevData();//update前置操作

private:
    bool InitAlphaFuncs(const char* target);
    void UpdateTickItemMap(const Snapshot &tick);

	//基础因子
	double m_plainAlphas[PLAIN_ALPHA_NUM];
	bool m_plainAlphasNeeded[PLAIN_ALPHA_NUM] { false };
	//所有基础因子高级函数
	int m_alphaFuncFlexCount = 0;
	double m_alphaFuncFlexValue[PLAIN_ALPHA_FUNC_NUM];
	double m_alphaFuncFlexParam[PLAIN_ALPHA_FUNC_NUM * ALPHA_FUNC_MAX_PARAMS_ADD1];
	double **m_alphaFuncFlexCache;

private:
    const char* m_target;

    vector<vector<int>> m_alphaFuncs;

    //tick信息放数组里
    const double* m_tick[20];

    //测试数组的速度
    int m_alphaNum;

    static double ParserAlphaFuncResult(const int alphaFunc[], int size, TaskData* taskData);

	//信号值
	Signal signals_;
	//合约信息
	Param params_;


private: // shared_memory part
    char ** m_funcs;
    TaskData** m_taskDatas;
    unsigned int m_cores;
    pthread_t* m_threads;

    shared_memory_object **m_sharedMemoryObjs;
    mapped_region **m_read_only_sharedMemories;

    void static UpdateSharedMemories(void* pData);

    mapped_region **m_read_write_sharedMemories;
    int **m_alphaFuncs2;
    int *m_sizes2;

};

#endif //ALPHAPROJECT_LRPRED_H
