//
// Created by tqz_trader on 2022/11/18.
//

#ifndef ALPHAPROJECT_STACK_H
#define ALPHAPROJECT_STACK_H

#define StoreTickNum 7200
#include <iostream>

//template<class T>
class Stack {

public:
    explicit Stack(size_t capacity);
    ~Stack();

    void push(const double& value);
    double& pop();

    double *top();

    size_t size() const;
    bool empty() const;

private:
    void reset(size_t capacity);

private:
    double* m_data;

    size_t m_capacity; // stack 容量
    size_t m_size; // stack 元素个数

};

//快速队列
class Queue {
public:
    Queue() {
        _begin = 0;
        _end = 0;
        _data = NULL;
        _capacity = StoreTickNum;  // \u961f\u5217\u957f\u5ea6
        _innersize = StoreTickNum + 1000;
        _updated = false;
        _data = new double[_innersize];
        for (int jj = 0; jj < _innersize; jj++) {
            _data[jj] = 0;
        }
    }
    ~Queue() {
        // \u91ca\u653e\u52a8\u6001\u521b\u5efa\u7684\u6570\u7ec4
        if (_data) delete[] _data;
    }
    void resize(int capacity) {
        _begin = 0;
        _end = 0;
        delete[] _data;
        _data = NULL;
        _capacity = capacity;  // \u961f\u5217\u957f\u5ea6
        if (_capacity > 0) {
            _innersize = capacity + 1000;
            _updated = false;
            _data = new double[_innersize];
            for (int ii = 0; ii < _innersize; ii++)
                _data[ii] = 0;
        }
    }
    void push(double e) {
        if (this->size() == _capacity) this->pop();
        _data[_end] = e;  // \u5c06end\u4f4d\u7f6e\u7f6e\u4e3ae
        _end = _end + 1;  // \u5faa\u73af\u540e\u79fb
        _updated = true;
    }
    double pop() {
        if (!this->empty()) {  // \u4e0d\u7a7a\u624d\u51fa\u961f
            double tmp = _data[_begin];
            _begin = _begin + 1;  // \u5faa\u73af\u540e\u79fb
            this->reset_pos();
            return tmp;
        }
    }
    double *head() {
        return _data + _begin;  // \u961f\u5934\u4f4d\u7f6e
    }

    // double tail_value(int n=0) {
    //     return _data[_end -n-1 ];  // \u53d6\u961f\u5c3e
    // }

    double *tail(int n = 1) {
        return _data + _end - n;  // \u53d6\u961f\u5c3e
    }

    double lag(int n = 1) {
        return *(_data + _end - n);
    }

    int size() const {
        return _end - _begin;  // \u961f\u5217\u5927\u5c0f
    }
    bool empty() const {
        return _begin == _end;  // \u5224\u7a7a
    }

    void reset_pos() {
        if (_begin > _innersize - _capacity - 10) {
            // cout<<_begin<<endl;
            for (int ii = 0; ii < this->size(); ii++) {
                _data[ii] = _data[ii + _begin];
            }
            _end = _end - _begin;
            _begin = 0;
        }
    }

    bool is_updated() {
        return _updated;
    }

    void reset_updated() {
        _updated = false;
    }

private:
    double* _data;  // \u6570\u7ec4\u540d
    int _begin;  // begin
    int _end;  // end
    int _capacity;  // \u961f\u5217\u957f\u5ea6
    int _innersize;
    bool _updated;
};

#endif //ALPHAPROJECT_STACK_H