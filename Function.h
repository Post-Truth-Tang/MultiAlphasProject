//
// Created by tqz_trader on 2022/11/14.
//

#ifndef ALPHAPROJECT_FUNCTION_H
#define ALPHAPROJECT_FUNCTION_H

#include <string>
#include <cmath>
#include <map>
#include "types.h"
extern "C" {
#include "buffer.h"
}
using namespace std;

namespace funcinfo {
	typedef TI_REAL(*pFunc) (const TI_REAL&);
	typedef TI_REAL(*pFunc2Var) (const TI_REAL&, const TI_REAL&);
	typedef TI_REAL(*pFuncAlpha) (const Signal &);
	typedef TI_REAL(*pFuncAlphaFlex) (Signal &, TI_REAL const*, TI_REAL*);

	enum PFuncType {
		ERROR_TYPE = -1,
		TICK_ITEM_TYPE = 0,
		ONE_VAR_TYPE = 1,
		TWO_VAR_TYPE = 2,
		ALPHA_TYPE = 3,
		AlPHA_FUNC_TYPE = 4,
		NUMBER = 5,
		//ALPHA_PERIOD_TYPE = 7,
	};

	struct alpha_func_info {
		const char *name;
		pFuncAlphaFlex update;
		pFuncAlphaFlex prev;
		PFuncType type;
		int options, caches;
		const char *option_names[10]; //options第一个都要是period
	};


	// simple part.
    inline TI_REAL square(const TI_REAL& value1) { return value1 * value1; }
	inline TI_REAL abs(const TI_REAL& value) { return (value > 0) ? value : (-1 * value); }
    inline TI_REAL todeg(const TI_REAL& value) { return value * (180 / M_PI); }
    inline TI_REAL torad(const TI_REAL& value) { return value * (M_PI / 180); }
    inline TI_REAL trunc(const TI_REAL& value) { return static_cast<int>(value); }
    inline TI_REAL acos(const TI_REAL& value) { return ::acos(value); }
    inline TI_REAL asin(const TI_REAL& value) { return ::asin(value); }
    inline TI_REAL atan(const TI_REAL& value) { return ::atan(value); }
    inline TI_REAL ceil(const TI_REAL& value) { return ::ceil(value); }
    inline TI_REAL floor(const TI_REAL& value) { return ::floor(value); }
    inline TI_REAL cos(const TI_REAL& value) { return ::cos(value); }
    inline TI_REAL cosh(const TI_REAL& value) { return ::cosh(value); }
    inline TI_REAL exp(const TI_REAL& value) { return ::exp(value); }
    inline TI_REAL sqrt(const TI_REAL& value) { return ::sqrt(value); }
    inline TI_REAL ln(const TI_REAL& value) { return ::log(value); }
    inline TI_REAL log10(const TI_REAL& value) { return ::log10(value); }
    inline TI_REAL round(const TI_REAL& value) { return ::round(value); }
    inline TI_REAL sin(const TI_REAL& value) { return ::sin(value); }
    inline TI_REAL sinh(const TI_REAL& value) { return ::sinh(value); }
    inline TI_REAL tan(const TI_REAL& value) { return ::tan(value); }
    inline TI_REAL tanh(const TI_REAL& value) { return ::tanh(value); }
    inline TI_REAL add(const TI_REAL& value1, const TI_REAL& value2) { return value1 + value2; }
    inline TI_REAL mul(const TI_REAL& value1, const TI_REAL& value2) { return value1 * value2; }
    inline TI_REAL div(const TI_REAL& value1, const TI_REAL& value2) { return value1 / value2; }
    inline TI_REAL sub(const TI_REAL& value1, const TI_REAL& value2) { return value1 - value2; }
    inline TI_REAL pow(const TI_REAL& value1, const TI_REAL& value2) { return ::pow(value1, value2); }


    //基础因子
    TI_REAL OIR(const Signal &signals_);
    TI_REAL VOI(const Signal &signals_);

	//基础因子的高级函数
    TI_REAL avgPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL avg(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL diffPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL diff(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL lagPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL lag(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL maxPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL max(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL minPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL min(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL emaPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL ema(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL bbandsUpPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL bbandsUp(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL bbandsDownPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL bbandsDown(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL demaPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL dema(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL hmaPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL hma(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL kamaPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL kama(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL linregPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL linreg(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	//indicator
	TI_REAL nviPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL nvi(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL pviPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL pvi(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL rocPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL roc(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL rocrPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL rocr(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL rsiPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
	TI_REAL rsi(Signal &signals_, TI_REAL const* options, TI_REAL* cache);

	// math part.
//    TI_REAL crossanyPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
//    TI_REAL crossany(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
//    TI_REAL crossoverPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
//    TI_REAL crossover(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL decayPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL decay(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL edecayPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL edecay(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL mdPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL md(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL variancePrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL variance(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL stddevPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL stddev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL _stderrPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache); // stderr 与 系统重名;
    TI_REAL _stderr(Signal &signals_, TI_REAL const* options, TI_REAL* cache); // stderr 与 系统重名;
    TI_REAL sumPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL sum(Signal &signals_, TI_REAL const* options, TI_REAL* cache);

    // tqz extern
    TI_REAL apoPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL apo(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL cmoPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL cmo(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL dpoPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL dpo(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL macdPrev(Signal &signals_, TI_REAL const* options, TI_REAL* cache);
    TI_REAL macd(Signal &signals_, TI_REAL const* options, TI_REAL* cache);


    //所有变量的数组
	const static string varStrArr[] = { "bid1", "ask1" };
	const static string funcAlphaStrArr[] = { "OIR", "VOI" };
	const static pFuncAlpha funcAlphaArr[] = { OIR, VOI };

	//所有函数的数组
	const static string func1VarStrArr[] = {
		"square", "abs","todeg", "torad", "trunc", "acos", "asin",
		"atan", "ceil", "floor", "cos", "cosh", "exp", "sqrt",
		"ln", "log10", "round", "sin", "sinh", "tan", "tanh"
	};
	const static pFunc func1VarArr[] = {
		square, abs, todeg, torad, trunc, acos, asin,
		atan, ceil, floor, cos, cosh, exp, sqrt,
		ln, log10, round, sin, sinh, tan, tanh
	};
	const static string func2VarStrArr[] = { "add", "mul", "div", "sub", "pow" };
	const static pFunc2Var func2VarArr[] = { add, mul, div, sub, pow };

	//不定数量参数的高级函数
	//mark:需要review的：linereg,
	const static alpha_func_info funcAlphaFlex[] = {
		//Overlay
		{"avg", avg, avgPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"diff", diff, diffPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"bbandsup", bbandsUp, bbandsUpPrev, AlPHA_FUNC_TYPE, 3, 2, {"period", "rate", "alpha_index"}},
		{"bbandsdown", bbandsDown, bbandsDownPrev, AlPHA_FUNC_TYPE, 3, 2, {"period", "rate", "alpha_index"}},
		{"dema", dema, demaPrev, AlPHA_FUNC_TYPE, 5, 2, {"ema long period", "ema rate", "dema short period", "dema rate", "alpha_index"}},
		{"ema", ema, emaPrev, AlPHA_FUNC_TYPE, 3, 1, {"period", "rate", "alpha_index"}},
		{"hma", hma, hmaPrev, AlPHA_FUNC_TYPE, 2, 3, {"period", "alpha_index"}},//需要period+sqrt(period)长度之后才能计算出结果
		{"kama", kama, kamaPrev, AlPHA_FUNC_TYPE, 2, 2, {"period", "alpha_index"}},
		{"linreg", linreg, linregPrev, AlPHA_FUNC_TYPE, 2, 6, {"period", "alpha_index"}},
		//Indicator
		//bc
		{"nvi", nvi, nviPrev, AlPHA_FUNC_TYPE, 1, 1, {"period"}},
		{"pvi", pvi, pviPrev, AlPHA_FUNC_TYPE, 1, 1, {"period"}},
		{"roc", roc, rocPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"rocr", rocr, rocrPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"rsi", rsi, rsiPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		//Math
		//{"crossany", crossany, crossanyPrev, AlPHA_FUNC_TYPE, 3, 4, {"long_period", "short_period", "alpha_index"}},
		//{"crossover", crossover, crossoverPrev, AlPHA_FUNC_TYPE, 3, 4, {"long_period", "short_period", "alpha_index"}},
		{"decay", decay, decayPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"edecay", edecay, edecayPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"lag", lag, lagPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"max", max, maxPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"md", md, mdPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"min", min, minPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"stddev", stddev, stddevPrev, AlPHA_FUNC_TYPE, 2, 2, {"period", "alpha_index"}},
		{"stderr", _stderr, _stderrPrev, AlPHA_FUNC_TYPE, 2, 2, {"period", "alpha_index"}},
		{"sum", sum, sumPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
		{"variance", variance, variancePrev, AlPHA_FUNC_TYPE, 2, 2, {"period", "alpha_index"}},

        // tqz extern.
        {"apo", apo, apoPrev, AlPHA_FUNC_TYPE, 3, 2, {"short_period", "long_period", "alpha_index"}},
        {"cmo", cmo, cmoPrev, AlPHA_FUNC_TYPE, 2, 2, {"period", "alpha_index"}},
        {"dpo", dpo, dpoPrev, AlPHA_FUNC_TYPE, 2, 1, {"period", "alpha_index"}},
        {"macd", macd, macdPrev, AlPHA_FUNC_TYPE, 3, 2, {"short_period", "long_period", "alpha_index"}},

	};
}

#endif //ALPHAPROJECT_FUNCTION_H