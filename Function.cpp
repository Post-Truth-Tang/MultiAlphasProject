//
// Created by tqz_trader on 2022/12/6.
//

#include "Function.h"
#include <algorithm>
#define M_PI 3.14159265358979323846
#include <cmath>

namespace funcinfo {
    //基础因子
    TI_REAL OIR(const Signal &signals_) {
        return (signals_.bz1 * 1.0 - signals_.az1 * 1.0) / (signals_.bz1 * 1.0 + signals_.az1 * 1.0);
    }
    TI_REAL VOI(const Signal &signals_) {
        TI_REAL bid1 = signals_.bid1;
        TI_REAL ask1 = signals_.ask1;
        int bz1 = signals_.bz1;
        int az1 = signals_.az1;
        TI_REAL beta_bz1 = 0;
        TI_REAL beta_az1 = 0;
        TI_REAL beta_bid1 = bid1 - signals_.last_bid1;
        TI_REAL beta_ask1 = ask1 - signals_.last_ask1;

        if (beta_bid1 > 0) {
            beta_bz1 = bz1;
        }
        else if (beta_bid1 < 0) {
            beta_bz1 = -signals_.last_bz1;
        }
        else {
            beta_bz1 = bz1 - signals_.last_bz1;
        }

        if (beta_ask1 < 0) {
            beta_az1 = az1;
        }
        else if (beta_ask1 > 0) {
            beta_az1 = -signals_.last_az1;
        }
        else {
            beta_az1 = az1 - signals_.last_az1;
        }

        return beta_bz1 - beta_az1;
    }

    //tqz 基础因子的高级函数
    // 判断当前是否为 金叉或死叉, 需要两个序列
    TI_REAL crossanyPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int long_period = (int)options[0];
        if (signals_.counter < long_period) {
            long_period = signals_.counter;
        }

        int short_period = (int)options[1];
        if (long_period <= short_period) {
            short_period = long_period - 1;
        }

        int alpha_index = (int)options[2];
        TI_REAL shortSum = 0;
        for (int i = 0; i < short_period - 1; i++)
            shortSum += signals_.predefined_alpha_queue[alpha_index].lag(i + 1);
        cache[0] = shortSum / short_period;
        cache[1] = (signals_.predefined_alpha_queue[alpha_index].lag(short_period) / short_period) + cache[0];

        TI_REAL longSum = 0;
        for (int i = 0; i < long_period - 1; i++)
            longSum += signals_.predefined_alpha_queue[alpha_index].lag(i + 1);
        cache[2] = longSum / long_period;
        cache[3] = (signals_.predefined_alpha_queue[alpha_index].lag(long_period) / long_period) + cache[2];

        return 0;
    }
    TI_REAL crossany(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int shortPeriod = (int)options[0];
        int longPeriod = (int)options[1];
        int alpha_index = (int)options[2];

        TI_REAL shortPeriodValue = cache[0] + (signals_.plainAlphas[alpha_index] / shortPeriod);
        TI_REAL longPeriodValue = cache[2] + (signals_.plainAlphas[alpha_index] / longPeriod);

        TI_REAL shortPeriodPreValue = cache[1];
        TI_REAL longPeriodPreValue = cache[3];

        bool longMarket = (shortPeriodValue > longPeriodValue) && (shortPeriodPreValue <= longPeriodPreValue);
        bool shortMarket = (shortPeriodValue < longPeriodValue) && (shortPeriodPreValue >= longPeriodPreValue);

        return shortMarket || longMarket;
    }
    // 判断当前是否金叉
    TI_REAL crossoverPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        crossanyPrev(signals_, options, cache);

        return 0;
    }
    TI_REAL crossover(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int shortPeriod = (int)options[0];
        int longPeriod = (int)options[1];
        int alpha_index = (int)options[2];

        TI_REAL shortPeriodValue = cache[0] + (signals_.plainAlphas[alpha_index] / shortPeriod);
        TI_REAL longPeriodValue = cache[2] + (signals_.plainAlphas[alpha_index] / longPeriod);

        TI_REAL shortPeriodPreValue = cache[1];
        TI_REAL longPeriodPreValue = cache[3];

        bool longMarket = (shortPeriodValue > longPeriodValue) && (shortPeriodPreValue <= longPeriodPreValue);
        return longMarket;
    }

    // 返回当前衰变期的值
    TI_REAL decayPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        if (signals_.counter <= 1) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL scale = 1.0 / period;
        TI_REAL tmp = signals_.predefined_alpha_queue[alpha_index].lag(period);
        for (int i = period - 2; i >= 0; i--) {
            tmp -= scale;
            tmp = signals_.predefined_alpha_queue[alpha_index].lag(i + 1) > tmp ? signals_.predefined_alpha_queue[alpha_index].lag(i + 1) : tmp;
        }
        cache[0] = tmp - scale;
        return 0;
    }
    TI_REAL decay(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        TI_REAL tmp = cache[0];
        return (signals_.plainAlphas[alpha_index] > tmp) ? signals_.plainAlphas[alpha_index] : tmp;
    }
    TI_REAL edecayPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        if (signals_.counter <= 1) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL scale = 1.0 - 1.0 / period;
        TI_REAL tmp = signals_.predefined_alpha_queue[alpha_index].lag(period);
        for (int i = period - 2; i >= 0; i--) {
            tmp *= scale;
            tmp = signals_.predefined_alpha_queue[alpha_index].lag(i + 1) > tmp ? signals_.predefined_alpha_queue[alpha_index].lag(i + 1) : tmp;
        }
        cache[0] = tmp * scale;
        return 0;
    }
    TI_REAL edecay(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        TI_REAL tmp = cache[0];
        return (signals_.plainAlphas[alpha_index] > tmp) ? signals_.plainAlphas[alpha_index] : tmp;
    }
    // mean deviation over period.
    TI_REAL mdPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        sumPrev(signals_, options, cache);
        return 0;
    }
    TI_REAL md(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        TI_REAL mean = (cache[0] + signals_.plainAlphas[alpha_index]) / period;
        TI_REAL ret = 0;
        for (int i = 0; i < period - 1; i++)
            ret += abs(signals_.predefined_alpha_queue[alpha_index].lag(i + 1) - mean);
        ret += abs(signals_.plainAlphas[alpha_index] - mean);
        return ret / period;
    }
    TI_REAL stddevPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        variancePrev(signals_, options, cache);
        return 0;
    }
    TI_REAL stddev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        return sqrt(variance(signals_, options, cache));
    }
    TI_REAL _stderrPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        stddevPrev(signals_, options, cache);
        return 0;
    } // stderr 与 系统重名;
    TI_REAL _stderr(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        const TI_REAL mul = 1.0 / sqrt(period);
        return stddev(signals_, options, cache) * mul;
    } // stderr 与 系统重名;
    TI_REAL sumPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        TI_REAL ret = 0;
        for (int i = 0; i < period - 1; i++)
            ret += signals_.predefined_alpha_queue[alpha_index].lag(i + 1);
        cache[0] = ret;
        return 0;
    }
    TI_REAL sum(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        return cache[0] + signals_.plainAlphas[(int)options[1]];
    }
    TI_REAL variancePrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter <= 2) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int alpha_index = (int)options[1];
        cache[0] = 0;
        cache[1] = 0;
        TI_REAL tmp;
        for (int i = 1; i < period; i++) {
            tmp = signals_.predefined_alpha_queue[alpha_index].lag(i);
            cache[0] += tmp;
            cache[1] += tmp * tmp;
        }
        return 0;
    }
    TI_REAL variance(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int alpha_index = (int)options[1];
        TI_REAL tmp = signals_.plainAlphas[alpha_index];
        cache[0] += tmp;
        cache[1] += tmp * tmp;
        TI_REAL scale = 1. / period;
        TI_REAL middle = cache[0] * scale;
        return cache[1] * scale - (middle) * (middle);
    }


    //bc
    TI_REAL nviPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter <= 2) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL tmp = 0;
        TI_REAL mid_price1, mid_price2;
        for (int i = period - 2; i >= 0; i--) {
            if (signals_.volume_queue.lag(i + 1) < signals_.volume_queue.lag(i + 2)) {
                mid_price1 = signals_.mid_queue.lag(i + 1);
                mid_price2 = signals_.mid_queue.lag(i + 2);
                tmp += (mid_price1 - mid_price2) / mid_price2;
            }
        }
        cache[0] = tmp;
        return 0;
    }
    TI_REAL nvi(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        TI_REAL tmp = 0;
        if (signals_.volume < signals_.volume_queue.lag(1)) {
            TI_REAL mid_price1, mid_price2;
            mid_price1 = signals_.mid_price;
            mid_price2 = signals_.mid_queue.lag(1);
            tmp = (mid_price1 - mid_price2) / mid_price2;
        }
        return cache[0] + tmp;
    }
    TI_REAL pviPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter <= 2) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL tmp = 0;
        TI_REAL mid_price1,mid_price2;
        for (int i = period - 2; i >= 0; i--) {
            if (signals_.volume_queue.lag(i + 1) > signals_.volume_queue.lag(i + 2)) {
                mid_price1 = signals_.mid_queue.lag(i + 1);
                mid_price2 = signals_.mid_queue.lag(i + 2);
                tmp += (mid_price1 - mid_price2) / mid_price2;
            }
        }
        cache[0] = tmp;
        return 0;
    }
    TI_REAL pvi(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        TI_REAL tmp = 0;
        if (signals_.volume > signals_.volume_queue.lag(1)) {
            TI_REAL mid_price1, mid_price2;
            mid_price1 = signals_.mid_price;
            mid_price2 = signals_.mid_queue.lag(1);
            tmp = (mid_price1 - mid_price2) / mid_price2;
        }
        return cache[0] + tmp;
    }
    TI_REAL rocPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        if (signals_.counter <= 2) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL tmp = 0;
        for (int i = period - 2; i >= 0; i--) {
            if (signals_.predefined_alpha_queue[alpha_index].lag(i + 2) == 0) {
                tmp += 0;
            }
            else {
                tmp += (signals_.predefined_alpha_queue[alpha_index].lag(i + 1) - signals_.predefined_alpha_queue[alpha_index].lag(i + 2)) / (signals_.predefined_alpha_queue[alpha_index].lag(i + 2));
            }
        }
        cache[0] = tmp;
        return 0;
    }
    TI_REAL roc(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int alpha_index = (int)options[1];
        if (signals_.counter <= 2) {
            return 0;
        }
        TI_REAL tmp = 0;
        TI_REAL lag_v = signals_.predefined_alpha_queue[alpha_index].lag(1);
        if (lag_v == 0) {
            tmp = 0;
        }
        else {
            tmp = (signals_.plainAlphas[alpha_index] - lag_v) / (lag_v);
        }
        return cache[0] + tmp;
    }
    TI_REAL rocrPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        if (signals_.counter <= 2) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL tmp = 1;
        for (int i = period - 2; i >= 0; i--) {
            if (signals_.predefined_alpha_queue[alpha_index].lag(i + 2) == 0 || signals_.predefined_alpha_queue[alpha_index].lag(i + 1) == 0) {
                tmp = tmp * 1.0;
            }
            else {
                tmp = tmp * (signals_.predefined_alpha_queue[alpha_index].lag(i + 1)) / (signals_.predefined_alpha_queue[alpha_index].lag(i + 2));
            }

        }
        cache[0] = tmp;
        return 0;
    }
    TI_REAL rocr(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int alpha_index = (int)options[1];
        if (signals_.counter <= 2) {
            return 0;
        }
        TI_REAL tmp = 1;
        TI_REAL lag_v = signals_.predefined_alpha_queue[alpha_index].lag(1);
        if (lag_v == 0 || signals_.plainAlphas[alpha_index] == 0) {
            tmp = 1;
        }
        else {
            tmp = signals_.plainAlphas[alpha_index] / lag_v;
        }
        return (cache[0] * tmp);
    }
    TI_REAL rsiPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        if (signals_.counter <= 2) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int upward = 0;
        int downward = 0;
        int upsmooth = 0;
        int downsmooth = 0;
        for (int i = period - 3; i >= 0; i--) {
            upward = signals_.predefined_alpha_queue[alpha_index].lag(i + 1) > signals_.predefined_alpha_queue[alpha_index].lag(i + 2) ? 1 : 0;
            downward = signals_.predefined_alpha_queue[alpha_index].lag(i + 1) < signals_.predefined_alpha_queue[alpha_index].lag(i + 2) ? 1 : 0;
            upsmooth += upward;
            downsmooth += downward;
        }
        cache[0] = upsmooth - downsmooth;
        return 0;
    }
    TI_REAL rsi(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        int alpha_index = (int)options[1];
        if (signals_.counter <= 2) {
            return 0;
        }
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL tmp = 0;
        int upward = 0;
        int downward = 0;
        int upsmooth = 0;
        int downsmooth = 0;
        upward = signals_.plainAlphas[alpha_index] > signals_.predefined_alpha_queue[alpha_index].lag(1) ? 1 : 0;
        downward = signals_.plainAlphas[alpha_index] < signals_.predefined_alpha_queue[alpha_index].lag(1) ? 1 : 0;
        upsmooth += upward;
        downsmooth += downward;
        tmp = upsmooth - downsmooth;
        return (cache[0] + tmp) / period;
    }

    //基础因子的高级函数
    TI_REAL avgPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int alpha_index = (int)options[1];
        if (signals_.counter == 1) {
            return signals_.predefined_alpha_queue[alpha_index].lag();
        }
        TI_REAL tmp = 0;
        for (int i = 0; i < period - 1; i++) {
            tmp += signals_.predefined_alpha_queue[alpha_index].lag(i + 1);
        }
        cache[0] = tmp / period;
        return 0;
    }
    TI_REAL avg(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int alpha_index = (int)options[1];
        return cache[0] + signals_.plainAlphas[alpha_index] / period;
    }
    TI_REAL diffPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        return 0;
    }
    TI_REAL diff(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int alpha_index = (int)options[1];
        return signals_.plainAlphas[alpha_index] - signals_.predefined_alpha_queue[alpha_index].lag(period - 1);
    }
    TI_REAL lagPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        return 0;
    }
    TI_REAL lag(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int alpha_index = (int)options[1];
        return signals_.predefined_alpha_queue[alpha_index].lag(period - 1);
    }
    TI_REAL maxPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int alpha_index = (int)options[1];
        TI_REAL max_num = signals_.predefined_alpha_queue[alpha_index].lag();
        for (int i = 2; i < period; i++) {
            if (signals_.predefined_alpha_queue[alpha_index].lag(i) > max_num) {
                max_num = signals_.predefined_alpha_queue[alpha_index].lag(i);
            }
        }
        cache[0] = max_num;
        return 0;
    }
    TI_REAL max(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        //int period = (int)options[0];
        int alpha_index = (int)options[1];
        if (signals_.plainAlphas[alpha_index] > cache[0]) {
            return signals_.plainAlphas[alpha_index];
        }
        else {
            return cache[0];
        }
    }
    TI_REAL minPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int alpha_index = (int)options[1];
        TI_REAL min_num = signals_.predefined_alpha_queue[alpha_index].lag();
        for (int i = 2; i < period; i++) {
            if (signals_.predefined_alpha_queue[alpha_index].lag(i) < min_num) {
                min_num = signals_.predefined_alpha_queue[alpha_index].lag(i);
            }
        }
        cache[0] = min_num;
        return 0;
    }
    TI_REAL min(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        //int period = (int)options[0];
        int alpha_index = (int)options[1];
        if (signals_.plainAlphas[alpha_index] < cache[0]) {
            return signals_.plainAlphas[alpha_index];
        }
        else {
            return cache[0];
        }
    }
    TI_REAL emaPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL rate = options[1];
        int alpha_index = (int)options[2];
        int n = period - 1;
        TI_REAL tmp = rate * signals_.predefined_alpha_queue[alpha_index].lag(n + 1) + (1 - rate) * signals_.predefined_alpha_queue[alpha_index].lag(n);
        for (int i = n - 1; i > 0; i--) {
            tmp = tmp * rate + (1 - rate)*signals_.predefined_alpha_queue[alpha_index].lag(i);
        }
        cache[0] = tmp;
        return 0;
    }
    TI_REAL ema(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        //int period = (int)options[0];
        TI_REAL rate = options[1];
        int alpha_index = (int)options[2];
        return cache[0] * rate + (1 - rate) * signals_.plainAlphas[alpha_index];
    }
    TI_REAL bbandsUpPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL rate = options[1];
        int alpha_index = (int)options[2];
        cache[0] = 0;
        cache[1] = 0;
        TI_REAL tmp;
        for (int i = 1; i < period; i++) {
            tmp = signals_.predefined_alpha_queue[alpha_index].lag(i);
            cache[0] += tmp;
            cache[1] += tmp * tmp;
        }
        return 0;
    }
    TI_REAL bbandsUp(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL rate = options[1];
        int alpha_index = (int)options[2];
        TI_REAL tmp = signals_.plainAlphas[alpha_index];
        cache[0] += tmp;
        cache[1] += tmp * tmp;
        TI_REAL scale = 1. / period;
        TI_REAL middle = cache[0] * scale;
        TI_REAL std = sqrt(cache[1] * scale - (middle) * (middle));
        return middle + std * rate;
    }
    TI_REAL bbandsDownPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL rate = options[1];
        int alpha_index = (int)options[2];
        cache[0] = 0;
        cache[1] = 0;
        TI_REAL tmp;
        for (int i = 1; i < period; i++) {
            tmp = signals_.predefined_alpha_queue[alpha_index].lag(i);
            cache[0] += tmp;
            cache[1] += tmp * tmp;
        }
        return 0;
    }
    TI_REAL bbandsDown(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        TI_REAL rate = options[1];
        int alpha_index = (int)options[2];
        TI_REAL tmp = signals_.plainAlphas[alpha_index];
        cache[0] += tmp;
        cache[1] += tmp * tmp;
        TI_REAL scale = 1. / period;
        TI_REAL middle = cache[0] * scale;
        TI_REAL std = sqrt(cache[1] * scale - (middle) * (middle));
        return middle - std * rate;
    }
    TI_REAL demaPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int long_period = (int)options[0];
        if (signals_.counter < long_period) {
            long_period = signals_.counter;
        }
        TI_REAL long_rate = options[1];
        int short_period = (int)options[2];
        if (long_period <= short_period) {
            short_period = long_period - 1;
        }
        TI_REAL short_rate = options[3];
        int alpha_index = (int)options[4];
        int n = long_period - 1;
        TI_REAL ema = long_rate * signals_.predefined_alpha_queue[alpha_index].lag(n + 1) + (1 - long_rate) * signals_.predefined_alpha_queue[alpha_index].lag(n);
        TI_REAL dema = ema;
        for (int i = n - 1; i > 0; i--) {
            if (i == short_period) {
                dema = ema;
            }
            ema = ema * long_rate + (1 - long_rate)*signals_.predefined_alpha_queue[alpha_index].lag(i);
            if (i < short_period) {
                dema = dema * short_rate + (1 - short_rate) * ema;
            }
        }
        cache[0] = ema;
        cache[1] = dema;
        return 0;

    }
    TI_REAL dema(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        //int long_period = (int)options[0];
        TI_REAL long_rate = options[1];
        //int short_period = (int)options[2];
        TI_REAL short_rate = options[3];
        int alpha_index = (int)options[4];
        TI_REAL ema = cache[0] * long_rate + (1 - long_rate) * signals_.plainAlphas[alpha_index];
        TI_REAL dema = cache[1] * short_rate + (1 - short_rate) * ema;
        return dema;
    }
    TI_REAL hmaPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        const int period = (int)options[0];
        const int period2 = (int)(period / 2);
        const int periodsqrt = (int)(sqrt(period));
        if (signals_.counter <= period + periodsqrt - 1) {
            return 0;
        }
        const int alpha_index = (int)options[1];
        int i;
        auto *input = new TI_REAL[period];
        for (i = 0; i < period + periodsqrt - 1; ++i) {
            input[i] = signals_.predefined_alpha_queue[alpha_index].lag(period + periodsqrt - 1 - i);
        }
        const TI_REAL weights = period * (period + 1) / 2.0;
        const TI_REAL weights2 = period2 * (period2 + 1) / 2.0;
        const TI_REAL weightssqrt = periodsqrt * (periodsqrt + 1) / 2.0;
        TI_REAL sum = 0;
        TI_REAL weight_sum = 0;
        TI_REAL sum2 = 0;
        TI_REAL weight_sum2 = 0;
        TI_REAL sumsqrt = 0;
        TI_REAL weight_sumsqrt = 0;
        for (i = 0; i < period - 1; ++i) {
            weight_sum += input[i] * (i + 1);
            sum += input[i];
            if (i >= period - period2) {
                weight_sum2 += input[i] * (i + 1 - (period - period2));
                sum2 += input[i];
            }
        }
        //TI_REAL output;
        ti_buffer *buff = ti_buffer_new(periodsqrt);
        for (i = period - 1; i <= (period - 1) + (periodsqrt - 1); ++i) {
            weight_sum += input[i] * period;
            sum += input[i];
            weight_sum2 += input[i] * period2;
            sum2 += input[i];
            const TI_REAL wma = weight_sum / weights;
            const TI_REAL wma2 = weight_sum2 / weights2;
            const TI_REAL diff = 2 * wma2 - wma;
            weight_sumsqrt += diff * periodsqrt;
            sumsqrt += diff;
            ti_buffer_qpush(buff, diff);
            if (i == (period - 1) + (periodsqrt - 1)) {
                //output = weight_sumsqrt / weightssqrt;
                weight_sumsqrt -= sumsqrt;
                sumsqrt -= ti_buffer_get(buff, 1);
            }
            else {
                weight_sumsqrt -= sumsqrt;
            }
            weight_sum -= sum;
            sum -= input[i - period + 1];
            weight_sum2 -= sum2;
            sum2 -= input[i - period2 + 1];
        }
        cache[0] = weight_sum;
        cache[1] = weight_sum2;
        cache[2] = weight_sumsqrt;

        delete[] input;
        return 0;
    }
    TI_REAL hma(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        const int period = (int)options[0];
        const int period2 = (int)(period / 2);
        const int periodsqrt = (int)(sqrt(period));
        if (signals_.counter <= period + periodsqrt - 1) {
            return 0;
        }
        const int alpha_index = (int)options[1];
        TI_REAL weight_sum = cache[0];
        TI_REAL weight_sum2 = cache[1];
        TI_REAL weight_sumsqrt = cache[2];
        const TI_REAL weights = period * (period + 1) / 2.0;
        const TI_REAL weights2 = period2 * (period2 + 1) / 2.0;
        const TI_REAL weightssqrt = periodsqrt * (periodsqrt + 1) / 2.0;

        TI_REAL alpha = signals_.plainAlphas[alpha_index];
        weight_sum += alpha * period;
        weight_sum2 += alpha * period2;
        const TI_REAL wma = weight_sum / weights;
        const TI_REAL wma2 = weight_sum2 / weights2;
        const TI_REAL diff = 2 * wma2 - wma;
        weight_sumsqrt += diff * periodsqrt;
        TI_REAL output = weight_sumsqrt / weightssqrt;
        return output;
    }
    TI_REAL kamaPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        const int alpha_index = (int)options[1];
        if (signals_.counter < period) {
            period = signals_.counter;
            cache[0] = signals_.predefined_alpha_queue[alpha_index].lag(period); //第一个kama数值是原来的值
        }
        int i;
        TI_REAL *input = new TI_REAL[period];
        for (i = 0; i < period; ++i) {
            input[i] = signals_.predefined_alpha_queue[alpha_index].lag(period - i);
        }
        TI_REAL sum = 0;
        for (int i = 1; i < period; ++i) {
            sum += fabs(input[i] - input[i - 1]);
        }
        cache[1] = sum;
        delete[] input;
        return 0;
    }
    TI_REAL kama(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        const int alpha_index = (int)options[1];
        TI_REAL kama = cache[0]; //读取上一个kama数值
        TI_REAL sum = cache[1];
        const TI_REAL short_per = 2 / (2.0 + 1);
        const TI_REAL long_per = 2 / (30.0 + 1);
        sum += fabs(signals_.plainAlphas[alpha_index] - signals_.predefined_alpha_queue[alpha_index].lag(1));
        TI_REAL er, sc;
        if (sum != 0.0) {
            er = fabs(signals_.plainAlphas[alpha_index] - signals_.predefined_alpha_queue[alpha_index].lag(period)) / sum;
        }
        else {
            er = 1.0;
        }
        sc = pow(er * (short_per - long_per) + long_per, 2);
        kama = kama + sc * (signals_.plainAlphas[alpha_index] - kama);
        cache[0] = kama;
        return kama;
    }
    TI_REAL linregPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        const int alpha_index = (int)options[1];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        int i;
        TI_REAL *input = new TI_REAL[period];
        for (i = 0; i < period; ++i) {
            input[i] = signals_.predefined_alpha_queue[alpha_index].lag(period - i);
        }
        TI_REAL x = 0; TI_REAL x2 = 0; TI_REAL y = 0; TI_REAL xy = 0;
        for (i = 0; i < (period)-1; ++i) {
            x += i + 1;
            x2 += (i + 1)*(i + 1);
            xy += (input)[i] * (i + 1);
            y += (input)[i];
        }
        x += (period);
        x2 += (period) * (period);
        const TI_REAL bd = 1.0 / ((period)* x2 - x * x);
        cache[0] = x;
        cache[1] = x2;
        cache[2] = y;
        cache[4] = xy;
        cache[5] = bd;
        delete[] input;
        return 0;
    }
    TI_REAL linreg(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) {
            return 0;
        }
        int period = (int)options[0];
        if (signals_.counter < period) {
            period = signals_.counter;
        }
        const int alpha_index = (int)options[1];
        const TI_REAL p = (1.0 / (period));
        TI_REAL x = cache[0];
        TI_REAL x2 = cache[1];
        TI_REAL y = cache[2];
        TI_REAL xy = cache[4];
        TI_REAL bd = cache[5];
        xy += signals_.plainAlphas[alpha_index] * (period);
        y += signals_.plainAlphas[alpha_index];
        const TI_REAL b = ((period)* xy - x * y) * bd;
        const TI_REAL a = (y - b * x) * p;
        return (a + b * ((period)));
    }

    TI_REAL apoPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) return 0;

        const int longPeriod = (signals_.counter < (int)options[1]) ? signals_.counter : (int)options[1];
        const int shortPeriod = (longPeriod <= (int)options[0]) ? (longPeriod - 1) : (int)options[0];
        const int alphaIndex = (int)options[2];

        const TI_REAL shortPer = 2 / ((TI_REAL)shortPeriod + 1);
        const TI_REAL longPer = 2 / ((TI_REAL)longPeriod + 1);

        TI_REAL shortEma = signals_.predefined_alpha_queue[alphaIndex].lag(longPeriod-1);
        TI_REAL longEma = shortEma;
        for (int i = 2; i < longPeriod; i++) {
            shortEma += (signals_.predefined_alpha_queue[alphaIndex].lag(longPeriod-i) - shortEma) * shortPer;
            longEma += (signals_.predefined_alpha_queue[alphaIndex].lag(longPeriod-i) - longEma) * longPer;
        }
        cache[0] = shortEma;
        cache[1] = longEma;

        return 0;
    }
    TI_REAL apo(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) return 0;

        const int longPeriod = (signals_.counter < (int)options[1]) ? signals_.counter : (int)options[1];
        const int shortPeriod = (longPeriod <= (int)options[0]) ? (longPeriod - 1) : (int)options[0];
        const int alphaIndex = (int)options[2];

        const TI_REAL shortPer = 2 / ((TI_REAL)shortPeriod + 1);
        const TI_REAL longPer = 2 / ((TI_REAL)longPeriod + 1);

        const TI_REAL shortEma = cache[0] + (signals_.plainAlphas[alphaIndex] - cache[0]) * shortPer;
        const TI_REAL longEma = cache[1] + (signals_.plainAlphas[alphaIndex] - cache[1]) * longPer;

        return shortEma - longEma;
    }

    TI_REAL cmoPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) return 0;

        const int period = (signals_.counter < (int)options[0]) ? signals_.counter : (int)options[0];
        const int alphaIndex = (int)options[1];

        TI_REAL upSum = 0;
        TI_REAL downSum = 0;
        for (int i = 1; i < period; i++) {
            auto queue = signals_.predefined_alpha_queue[alphaIndex];
            upSum += (queue.lag(i) > queue.lag(i+1) ? (queue.lag(i) - queue.lag(i+1)) : 0);
            downSum += (queue.lag(i) < queue.lag(i+1) ? (queue.lag(i+1) - queue.lag(i)) : 0);
        }
        cache[0] = upSum;
        cache[1] = downSum;

        return 0;
    }
    TI_REAL cmo(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) return 0;

        TI_REAL upSum = cache[0];
        TI_REAL downSum = cache[1];
        const int alphaIndex = (int)options[1];

        TI_REAL preValue = signals_.predefined_alpha_queue[alphaIndex].lag(2);
        TI_REAL value = signals_.plainAlphas[alphaIndex]; // 与 signals_.predefined_alpha_queue[alphaIndex].lag(1) 等价.
        upSum += (value > preValue ? (value - preValue) : 0);
        downSum += (value < preValue ? (preValue - value) : 0);

        return 100 * (upSum - downSum) / (upSum + downSum);
    }


    TI_REAL dpoPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) return 0;

        const int period = (signals_.counter < (int)options[0]) ? signals_.counter : (int)options[0];
        const int alphaIndex = (int)options[1];

        TI_REAL sum = 0;
        for (int i = 1; i < period; ++i) {
            sum += signals_.predefined_alpha_queue[alphaIndex].lag(i);
        }
        cache[0] = sum;

        return 0;
    }
    TI_REAL dpo(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        if (signals_.counter <= 2) return 0;

        const int period = (signals_.counter < (int)options[0]) ? signals_.counter : (int)options[0];
        const int alphaIndex = (int)options[1];
        const int back = period / 2 + 1;
        const TI_REAL scale = 1.0 / period;

        const TI_REAL sum = cache[0] + signals_.plainAlphas[alphaIndex];
        return signals_.predefined_alpha_queue[alphaIndex].lag(period-1-back-1) - (sum * scale);
    }


    TI_REAL macdPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        const int shortPeriod = (int)options[0];
        const int longPeriod = (int)options[1];
        if (signals_.counter < longPeriod) return 0;

        const int alphaIndex = (int)options[2];

        TI_REAL fastMa = 0;
        TI_REAL slowMa = 0;
        for (int i = 1; i < longPeriod; ++i) {
            if (i < shortPeriod)
                fastMa += signals_.predefined_alpha_queue[alphaIndex].lag(i);

            slowMa += signals_.predefined_alpha_queue[alphaIndex].lag(i);
        }

        cache[0] = fastMa / shortPeriod;
        cache[1] = slowMa / longPeriod;

        return 0;
    }

    TI_REAL macd(Signal &signals_, TI_REAL const* options, TI_REAL* cache) {
        const int shortPeriod = (int)options[0];
        const int longPeriod = (int)options[1];
        if (signals_.counter < longPeriod) return 0;

        const int alphaIndex = (int)options[2];

        TI_REAL fastMa = cache[0] + signals_.plainAlphas[alphaIndex] / shortPeriod;
        TI_REAL slowMa = cache[1] + signals_.plainAlphas[alphaIndex] / longPeriod;

        return fastMa - slowMa;
    }
}
