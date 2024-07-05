//
// Created by tqz_trader on 2022/11/16.
//

#include "LRPred.h"

#include <iostream>
#include <fstream>
#include "Function.h"

using namespace funcinfo;

namespace{
    //自定义round函数
    double myRound(double d,int n)
    {
        d *= pow(10,n);
        d += 0.5;
        d = (long)d;
        d /= pow(10,n);
        return d;
    }
    bool isNumber(const string& str)
    {
        return str.find_first_not_of("0123456789.") == string::npos;
    }
    bool getNodeType(const string& node_str, PFuncType& out_type, int& index) {
        bool b_find = false;
        if (isNumber(node_str)) {
            b_find = true;
            out_type = NUMBER;
            index = 0;
        }
        if (!b_find) {
            for (int j = 0; j < sizeof(func1VarArr) / sizeof(pFunc); j++) {
                if (func1VarStrArr[j] == node_str) {
                    b_find = true;
                    out_type = ONE_VAR_TYPE;
                    index = j;
                    break;
                }
            }
        }
        if (!b_find) {
            for (int j = 0; j < sizeof(func2VarArr) / sizeof(pFunc2Var); j++) {
                if (func2VarStrArr[j] == node_str) {
                    b_find = true;
                    out_type = TWO_VAR_TYPE;
                    index = j;
                    break;
                }
            }
        }
        if (!b_find) {
            for (int j = 0; j < sizeof(varStrArr) / sizeof(string); j++) {
                if (varStrArr[j] == node_str) {
                    b_find = true;
                    out_type = TICK_ITEM_TYPE;
                    index = j;
                    break;
                }
            }
        }
        //基础因子
        if (!b_find) {
            for (int j = 0; j < sizeof(funcAlphaStrArr) / sizeof(string); j++) {
                if (funcAlphaStrArr[j] == node_str) {
                    b_find = true;
                    out_type = ALPHA_TYPE;
                    index = j;
                    break;
                }
            }
        }
        //基础因子高级函数
        if (!b_find) {
            for (int j = 0; j < sizeof(funcAlphaFlex) / sizeof(alpha_func_info); j++) {
                if (funcAlphaFlex[j].name == node_str) {
                    b_find = true;
                    out_type = AlPHA_FUNC_TYPE;
                    index = j;
                    break;
                }
            }
        }
        if (!b_find) {
            cout << "node string wrong:" << node_str << endl;
        }
        return b_find;
    }
    string getWholeExpressionFlex(alpha_func_info func_info, string line, int& index, double* options, int& alpha_num, int* alpha_indexs) {
        string res = func_info.name;
        res += '(';
        string nodeStr = "";
        PFuncType type;
        int alpha_index;
        int nodeIndex = 0;
        alpha_num = 0;
        for (; index < line.size(); index++) {
            if (line[index] == '(')
                continue;
            if (line[index] == ')') {
                if (getNodeType(nodeStr, type, alpha_index)) {
                    if (type == NUMBER) {
                        options[nodeIndex] = atof(nodeStr.c_str());
                    } else if (type == ALPHA_TYPE) {
                        options[nodeIndex] = (double)alpha_index;
                        alpha_indexs[alpha_num] = alpha_index;
                        alpha_num++;
                    }
                }
                res += nodeStr + ')';
                nodeStr = "";
                nodeIndex++;
                break;
            }

            if (line[index] == ',') {
                if (getNodeType(nodeStr, type, alpha_index)) {
                    if (type == NUMBER) {
                        options[nodeIndex] = atof(nodeStr.c_str());
                    } else if (type == ALPHA_TYPE) {
                        options[nodeIndex] = (double)alpha_index;
                    }
                }
                res += nodeStr + ',';
                nodeStr = "";
                nodeIndex++;
            } else {
                nodeStr += line[index];
            }
        }
        return res;
    }
}

LRPred::LRPred(const char* target) : m_target(target), m_alphaNum(0) {

}

LRPred::~LRPred() {
    if (m_alphaNum > 0) {
        for (int i = 0; i < m_alphaNum; i++) {
            delete[] m_alphaFuncs2[i];
            shared_memory_object::remove(m_funcs[i]);
            delete[] m_funcs[i];
            delete m_sharedMemoryObjs[i];
            delete m_read_only_sharedMemories[i];
            delete m_read_write_sharedMemories[i];
        }

        delete[] m_sharedMemoryObjs;
        delete[] m_read_only_sharedMemories;
        delete[] m_read_write_sharedMemories;

        delete[] m_alphaFuncs2;
        delete[] m_funcs;
        delete[] m_sizes2;

        for (int i = 0; i < m_alphaFuncFlexCount; i++) {
            delete[] m_alphaFuncFlexCache[i];
        }
        delete[] m_alphaFuncFlexCache;

        if (m_taskDatas) {
            for (int i = 0; i < m_cores; i++)
                delete m_taskDatas[i];

            delete[] m_taskDatas;
            m_taskDatas = nullptr;
        }

        if (m_threads) {
            delete[] m_threads;
            m_threads = nullptr;
        }

        m_alphaNum = 0;
    }
}

bool LRPred::InitPred() {
    signals_.last_acc_volume = 0;
    signals_.last_acc_turnover = 0;

    //初始化所需要用到的存储历史数据的队列
    signals_.ask1_queue.resize(7200);
    signals_.bid1_queue.resize(7200);
    signals_.az1_queue.resize(7200);
    signals_.bz1_queue.resize(7200);
    signals_.volume_queue.resize(15100);
    signals_.vwap_queue.resize(15100);
    for (auto & ii : signals_.predefined_alpha_queue){
        ii.resize(4000);
    }

    bool initRet = InitAlphaFuncs(this->m_target);
    if (!initRet) {
        cout << "InitAlphaFuncs fail." << endl;
        return initRet;
    }

    return initRet;
}


void LRPred::SetCoreCounts(unsigned int cores) {
    this->m_cores = cores;
    if (!m_taskDatas) {
        m_taskDatas = new TaskData *[cores];

        unsigned int lastEndIndex = 0;
        unsigned int step = this->m_alphaNum / cores;
        for (int i = 0; i < cores; ++i) {
            auto* taskData = new TaskData();
            taskData->coreNumber = i;

            taskData->startIndex = lastEndIndex;
            lastEndIndex += step;
            taskData->endIndex = lastEndIndex;

            taskData->read_write_sharedMemories = m_read_write_sharedMemories;
            taskData->alphaFuncs2 = m_alphaFuncs2;
            taskData->sizes2 = m_sizes2;

            taskData->ticks = const_cast<double **>(m_tick);

            taskData->plainAlphas = m_plainAlphas;
            taskData->alphaFuncFlexValue = m_alphaFuncFlexValue;

            m_taskDatas[i] = taskData;
        }
        m_taskDatas[cores-1]->endIndex = this->m_alphaNum;
    }

    if (!m_threads) {
        m_threads = new pthread_t[cores];
    }
}

bool LRPred::InitAlphaFuncs(const char* target) {
    ifstream file(target);
    if (!file) {
        cout << target << " can't open!" << endl;
        return false;
    }

    // init m_alphaFuncs.
    string sourceLine;
    //存储avg(10,VOI)
    map<string, int> alpha_func_flex_map;

    while(getline(file, sourceLine)) {
        string subNodeStr;
        PFuncType subNode1; //类别序号
        int subNode2; //此类中的序号

        vector<int> subNodeVector;
        for (int i = 0; i < sourceLine.size(); ++i) {
            // 过滤行头 & 行尾
            if (sourceLine[i] == '"') {
                if (sourceLine[i + 1] == ',')
                    break;
                continue;
            }

            // split var & func into vector
            if (sourceLine[i] == '(' || sourceLine[i] == ')' || sourceLine[i] == ',') {
                if (subNodeStr.empty())
                    continue;
                subNode1 = ERROR_TYPE;
                subNode2 = 0;
                //cout << subNodeStr << endl;
                //判断subNodeStr的位置
                bool b_find = getNodeType(subNodeStr, subNode1, subNode2);
                if (subNode1 == ALPHA_TYPE) {
                    m_plainAlphasNeeded[subNode2] = true;
                } else if (subNode1 == AlPHA_FUNC_TYPE) {
                    //记录到map中
                    int func_index = subNode2;
                    double options[ALPHA_FUNC_MAX_PARAMS];
                    int alpha_num;
                    int alpha_indexs[PLAIN_ALPHA_FUNC_NUM];
                    string expression = getWholeExpressionFlex(funcAlphaFlex[func_index], sourceLine, i, options, alpha_num, alpha_indexs);

                    if (alpha_func_flex_map.count(expression)) {
                        subNode2 = alpha_func_flex_map[expression];
                    } else {
                        alpha_func_flex_map[expression] = m_alphaFuncFlexCount;
                        subNode2 = m_alphaFuncFlexCount;
                        for (int k = 0; k < alpha_num; k++) {
                            m_plainAlphasNeeded[alpha_indexs[k]] = true;
                        }
                        m_alphaFuncFlexParam[m_alphaFuncFlexCount * ALPHA_FUNC_MAX_PARAMS_ADD1] = func_index;
                        for (int k = 0; k < ALPHA_FUNC_MAX_PARAMS; k++) {
                            m_alphaFuncFlexParam[m_alphaFuncFlexCount * ALPHA_FUNC_MAX_PARAMS_ADD1 + k + 1] = options[k];
                        }

                        m_alphaFuncFlexCount++;
                    }
                }

                if (b_find) {
                    subNodeVector.push_back(subNode1);
                    subNodeVector.push_back(subNode2);
                } else {
                    cout << "not known symbol " << subNodeStr << endl;
                }
                //subNodeVector.push_back(new_node_name);
                subNodeStr = "";
                continue;
            }

            subNodeStr += sourceLine[i];
        }
        m_alphaFuncs.push_back(subNodeVector);
    }
    file.close();

    //把vector变成数组
    m_alphaNum = m_alphaFuncs.size();
    m_alphaFuncs2 = new int*[m_alphaNum];
    m_sizes2 = new int[m_alphaNum];

    // 各个func的共享内存名
    m_funcs = new char*[m_alphaNum];
    int i = 0;
    for (auto& v: m_alphaFuncs) {
        string funcStr;
        for (auto& value: v) {
            funcStr += to_string(value);
        }
        size_t funcSize = funcStr.size() + 1;
        m_funcs[i] = new char[funcSize];
        memcpy(m_funcs[i], funcStr.c_str(), funcSize);
        i++;
    }


    // 各个func结果的共享内存地址
    m_sharedMemoryObjs = new shared_memory_object*[m_alphaNum];
    m_read_write_sharedMemories = new mapped_region*[m_alphaNum];
    m_read_only_sharedMemories = new mapped_region*[m_alphaNum];
    for (i = 0; i < m_alphaNum; ++i) {
        m_sharedMemoryObjs[i] = new shared_memory_object(open_or_create, m_funcs[i], read_write);
        m_sharedMemoryObjs[i]->truncate(sizeof(double));

        m_read_write_sharedMemories[i] = new mapped_region(*m_sharedMemoryObjs[i], read_write);
        m_read_only_sharedMemories[i] = new mapped_region(*m_sharedMemoryObjs[i], read_only);
    }
    i = 0;

    //读取每一行的算式
    for (auto& alphaFunc : m_alphaFuncs) {
        int new_size = (alphaFunc).size();
        m_alphaFuncs2[i] = new int[new_size];
        int j = 0;
        for (auto iterator = alphaFunc.rbegin(); iterator != alphaFunc.rend(); iterator++) {
            //两个为一组倒序
            int index = *iterator;
            iterator++;
            int type_name = *iterator;
            m_alphaFuncs2[i][j] = type_name;
            m_alphaFuncs2[i][j+1] = index;
            j += 2;
        }
        m_sizes2[i] = new_size;
        i++;
    }

	for (i = 0; i < m_alphaNum; i++) {
		for (int j = 0; j < m_sizes2[i]; j++) {
//			cout << m_alphaFuncs2[i][j] << endl;
		}
	}

    //把signal里不用的predefined_alpha_queue大小设置为0
    for (i = 0; i < PLAIN_ALPHA_NUM; i++) {
        if (!m_plainAlphasNeeded[i]) {
            signals_.predefined_alpha_queue[i].resize(0);
        }
    }
    //基础因子高级函数的cache初始化
    m_alphaFuncFlexCache = new double*[m_alphaFuncFlexCount];
    for (i = 0; i < m_alphaFuncFlexCount; i++) {
        int num = funcAlphaFlex[(int)m_alphaFuncFlexParam[i * ALPHA_FUNC_MAX_PARAMS_ADD1]].caches;
        m_alphaFuncFlexCache[i] = new double[funcAlphaFlex[(int)m_alphaFuncFlexParam[i * ALPHA_FUNC_MAX_PARAMS_ADD1]].caches];
		for (int j = 0; j < funcAlphaFlex[(int)m_alphaFuncFlexParam[i * ALPHA_FUNC_MAX_PARAMS_ADD1]].caches; j++) {
			m_alphaFuncFlexCache[i][j] = 0;
		}
    }

    return true;
}

void LRPred::UpdateTick(const Snapshot &tick) {
    this->UpdateTickItemMap(const_cast<Snapshot&>(tick));

    //计算plainAlpha
    for (int i = 0; i < PLAIN_ALPHA_NUM; ++i) {
        if (m_plainAlphasNeeded[i]) {
            m_plainAlphas[i] = funcAlphaArr[i](signals_);
            signals_.plainAlphas[i] = m_plainAlphas[i];
//            cout << funcAlphaStrArr[i] << ":" << m_plainAlphas[i] << endl;
        }
    }

    //计算plainAlpha的func,第二跳才开始计算
//    if (signals_.counter <= 1) return true;

    for (int i = 0; i < m_alphaFuncFlexCount; ++i) {
        m_alphaFuncFlexValue[i] = funcAlphaFlex[(int)m_alphaFuncFlexParam[i * ALPHA_FUNC_MAX_PARAMS_ADD1]].update(
            signals_,
            &m_alphaFuncFlexParam[i * ALPHA_FUNC_MAX_PARAMS_ADD1 + 1],
            m_alphaFuncFlexCache[i]
        );
    }
}


void LRPred::UpdateAlphas() {
    for (int i = 0; i < m_cores; ++i) {
        pthread_create(&m_threads[i], nullptr, reinterpret_cast<void *(*)(void *)>(LRPred::UpdateSharedMemories), (void *)m_taskDatas[i]);
    }

    for (int i = 0; i < m_cores; ++i) {
        pthread_join(m_threads[i], nullptr);
    }
}


int set_cpu(int i)
{
#ifdef _WIN32

#elif __linux__

#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

    cpu_set_t mask;
    CPU_ZERO(&mask);

    CPU_SET(i, &mask);

    if(-1 == pthread_setaffinity_np(pthread_self() ,sizeof(mask),&mask)) {
        return -1;
    }
#endif

    return 0;
}

void LRPred::UpdateSharedMemories(void* pData) {
    auto *taskData = (TaskData *) pData;

    set_cpu(taskData->coreNumber);

    for (size_t i = taskData->startIndex; i < taskData->endIndex; ++i) { // m_funcs[i], m_alphaFuncs2[i], m_sizes2[i]; 三者绑定 func
        auto *alphaValue = static_cast<double*>(taskData->read_write_sharedMemories[i]->get_address());

        // 将计算好的因子结果放到共享内存中
        *alphaValue = ParserAlphaFuncResult(taskData->alphaFuncs2[i], taskData->sizes2[i], taskData);
    }
}

bool LRPred::Update(const Snapshot &tick) {
    /* --- strategy part --- */

    //数组方式更新
    // 直接根据共享内存的标识字符串来读取值
    for (int i = 0; i < m_alphaNum; ++i) {
        auto *alphaValue = static_cast<double*>(m_read_only_sharedMemories[i]->get_address());
        cout << i << "-ret:" << *alphaValue << endl;
    }

    return true;
}

void LRPred::UpdatePrevData()
{
    //push是耗时操作
    signals_.ask1_queue.push(signals_.ask_lis[0]);
    signals_.bid1_queue.push(signals_.bid_lis[0]);
    signals_.az1_queue.push(signals_.az_lis[0]);
    signals_.bz1_queue.push(signals_.bz_lis[0]);
    signals_.volume_queue.push(signals_.volume);
    signals_.vwap_queue.push(signals_.vwap);
    signals_.mid_queue.push(signals_.mid_price);

    signals_.last_vwap = signals_.vwap;
    signals_.last_acc_volume = signals_.acc_volume;
    signals_.last_acc_turnover = signals_.acc_turnover;

    //int tick_count=signals_.counter+ 1;
    signals_.counter += 1;

    for (int ii = 0; ii < PLAIN_ALPHA_NUM; ii++) {
        if (m_plainAlphasNeeded[ii])
            signals_.predefined_alpha_queue[ii].push(m_plainAlphas[ii]);
    }

    //计算plainAlpha的func
    for (int i = 0; i < m_alphaFuncFlexCount; i++) {
        funcAlphaFlex[(int)m_alphaFuncFlexParam[i * ALPHA_FUNC_MAX_PARAMS_ADD1]].prev(
                signals_,
                &m_alphaFuncFlexParam[i * ALPHA_FUNC_MAX_PARAMS_ADD1 + 1],
                m_alphaFuncFlexCache[i]
        );
    }
}

void LRPred::UpdateTickItemMap(const Snapshot &tick) {
    m_tick[0] = &tick.bid1;
    m_tick[1] = &tick.ask1;

    //更新tick数据
    for (int i = 0; i < MKTDEPTH && i < tick.level; ++i) {
        if (signals_.counter > 2) {
            signals_.last3_bid_lis[i] = signals_.last2_bid_lis[i];
            signals_.last3_ask_lis[i] = signals_.last2_ask_lis[i];
            signals_.last3_bz_lis[i] = signals_.last2_bz_lis[i];
            signals_.last3_az_lis[i] = signals_.last2_az_lis[i];
        }

        if (signals_.counter > 1) {
            signals_.last2_bid_lis[i] = signals_.last_bid_lis[i];
            signals_.last2_ask_lis[i] = signals_.last_ask_lis[i];
            signals_.last2_bz_lis[i] = signals_.last_bz_lis[i];
            signals_.last2_az_lis[i] = signals_.last_az_lis[i];
        }

        if (signals_.counter > 0) {
            signals_.last_bid_lis[i] = signals_.bid_lis[i];
            signals_.last_ask_lis[i] = signals_.ask_lis[i];
            signals_.last_bz_lis[i] = signals_.bz_lis[i];
            signals_.last_az_lis[i] = signals_.az_lis[i];
        }

        signals_.bid_lis[i] = myRound(*(&tick.bid + i), 5);
        signals_.ask_lis[i] = myRound(*(&tick.ask + i), 5);
        signals_.bz_lis[i] = *(&tick.bz + i);
        signals_.az_lis[i] = *(&tick.az + i);

        if (signals_.counter == 0) {
            signals_.last_bid_lis[i] = signals_.bid_lis[i];
            signals_.last_ask_lis[i] = signals_.ask_lis[i];
            signals_.last_bz_lis[i] = signals_.bz_lis[i];
            signals_.last_az_lis[i] = signals_.az_lis[i];
        }
        if (signals_.counter <= 1) {
            signals_.last2_bid_lis[i] = signals_.last_bid_lis[i];
            signals_.last2_ask_lis[i] = signals_.last_ask_lis[i];
            signals_.last2_bz_lis[i] = signals_.last_bz_lis[i];
            signals_.last2_az_lis[i] = signals_.last_az_lis[i];
        }
        if (signals_.counter <= 2) {
            signals_.last3_bid_lis[i] = signals_.last2_bid_lis[i];
            signals_.last3_ask_lis[i] = signals_.last2_ask_lis[i];
            signals_.last3_bz_lis[i] = signals_.last2_bz_lis[i];
            signals_.last3_az_lis[i] = signals_.last2_az_lis[i];
        }
    }

    signals_.last_price = tick.last;

    signals_.bz1 = signals_.bz_lis[0];
    signals_.bz2 = signals_.bz_lis[1];
    signals_.bz3 = signals_.bz_lis[2];
    signals_.bz4 = signals_.bz_lis[3];
    signals_.bz5 = signals_.bz_lis[4];

    signals_.az1 = signals_.az_lis[0];
    signals_.az2 = signals_.az_lis[1];
    signals_.az3 = signals_.az_lis[2];
    signals_.az4 = signals_.az_lis[3];
    signals_.az5 = signals_.az_lis[4];

    signals_.bid1 = signals_.bid_lis[0];
    signals_.ask1 = signals_.ask_lis[0];
    signals_.last_bid1 = signals_.last_bid_lis[0];
    signals_.last_ask1 = signals_.last_ask_lis[0];
    signals_.last_bz1 = signals_.last_bz_lis[0];
    signals_.last_az1 = signals_.last_az_lis[0];
    signals_.acc_volume = tick.acc_volume;
    signals_.acc_turnover = tick.acc_turnover;
	signals_.mid_price = signals_.bid1 * 0.5 + signals_.ask1 * 0.5;

    int volume = signals_.acc_volume - signals_.last_acc_volume;
    if (signals_.counter == 0)
        volume = 0;
    double turnover = signals_.acc_turnover - signals_.last_acc_turnover;

    if (signals_.counter == 0) {
        signals_.first_mid = signals_.bid_lis[0] / 2 + signals_.ask_lis[0] / 2;
    }

    if (volume > 0) {
        signals_.vwap = turnover / volume / params_.multiplier;
        int geshu = floor((long double)myRound(signals_.vwap / params_.price_tick, 5));
        signals_.price_down = geshu * params_.price_tick;
        signals_.price_down = myRound(signals_.price_down, 5);
        if (myRound(signals_.price_down, 5) == myRound(signals_.vwap, 5)) {
            signals_.price_up = signals_.price_down;
        } else {
            signals_.price_up = signals_.price_down + params_.price_tick;
            signals_.price_up = myRound(signals_.price_up, 5);
        }

    } else {
        signals_.vwap = signals_.bid1 * 0.5 + signals_.ask1 * 0.5;
        signals_.price_up = signals_.vwap;
        signals_.price_down = signals_.vwap;
    }

    signals_.volume = volume;

    signals_.timestamp = tick.timestamp;
}


double LRPred::ParserAlphaFuncResult(const int alphaFunc[], int size, TaskData* taskData) {
    auto valueStack = taskData->valueStack;
    double** ticks = taskData->ticks;
    double* plainAlphas = taskData->plainAlphas;
    double* alphaFuncFlexValue = taskData->alphaFuncFlexValue;

    for (int i = 0; i < size; i+=2) {
        switch (alphaFunc[i]) {
            case TICK_ITEM_TYPE:
                valueStack->push(*ticks[alphaFunc[i+1]]);
                break;
            case ONE_VAR_TYPE:
                valueStack->push(func1VarArr[alphaFunc[i+1]](valueStack->pop()));
                break;
            case TWO_VAR_TYPE: {
                double var1 = valueStack->pop();
                double var2 = valueStack->pop();
                valueStack->push(func2VarArr[alphaFunc[i+1]](var1, var2));
                break;
            }
            case ALPHA_TYPE:
                valueStack->push(plainAlphas[alphaFunc[i+1]]);
                break;
            case AlPHA_FUNC_TYPE:
                valueStack->push(alphaFuncFlexValue[alphaFunc[i+1]]);
                break;
        }
    }

    return valueStack->pop();
}
