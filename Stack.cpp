//
// Created by tqz_trader on 2022/11/18.
//

#include "Stack.h"

#include <iostream>
using namespace std;

Stack::Stack(size_t capacity): m_size(0), m_capacity(capacity) {
    this->m_data = new double[this->m_capacity];
    for(size_t index = 0; index < this->m_capacity; index++) {
        this->m_data[index] = 0;
    }
}

Stack::~Stack() {
    if (this->m_data) {
        delete[] this->m_data;
        this->m_data = nullptr;
    }
}

void Stack::push(const double& value) {
//    if (this->m_size == this->m_capacity)
//        this->reset(this->m_capacity+10);

    this->m_data[this->m_size] = value;
    this->m_size++;
}

double& Stack::pop() {
    this->m_size--;

    return this->m_data[this->m_size];
//    if (!this->empty()) {
//        this->m_size--;
//        return this->m_data[this->m_size];
//    }
//
//    return 0;
}

double *Stack::top() {
//    if (!this->m_size)
//        return nullptr;

    return m_data + this->m_size - 1;
}

size_t Stack::size() const {
    return this->m_size;
}

bool Stack::empty() const {
    return (0 == this->m_size);
}


void Stack::reset(size_t capacity){
    this->m_capacity = capacity;

    auto* data = new double[this->m_capacity];
    for (size_t index = 0; index < this->m_capacity; index++) {
        if (index < this->m_size) {
            data[index] = this->m_data[index];
        } else {
            data[index] = 0;
        }
    }
    delete[] this->m_data;

    this->m_data = data;
}