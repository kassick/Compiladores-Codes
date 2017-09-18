/****************************************************************************
 *        Filename: "StackVM/include/vm.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Sep 18 14:06:37 2017"
 *         Updated: "2017-09-18 15:51:18 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <vector>
#include <stack>
#include <unordered_map>

#include "data.H"

struct VM;

struct Instruction {
    virtual VM& run(VM&) const = 0;
};

struct VM {

    std::vector<DataItem> stack;
    std::stack<int> marker_stack;
    std::vector<Instruction*> instructions;
    std::unordered_map<string, int> label_map;
    int pc = 0;

    VM() {
        marker_stack.push(0);
    }

    int size() {
        return stack.size() - marker_stack.top();
    }

    int jump(int target) {
        if (target < instructions.size())
            pc = target;
        return pc;
    }

    template<typename T>
    void push(T v) {
        stack.emplace_back(v);
    }

    DataItem pop() {
        if (stack.size() == marker_stack.top())
            throw logic_error("Popping beyond top mark");

        DataItem item = stack[stack.size() - 1];
        stack.resize(stack.size() - 1);
        return item;
    }

    DataItem get(int i) {
        int logic_size = stack.size() - marker_stack.top();

        if (i < 0 || i >= logic_size)
            throw logic_error("Trying to get item past stack end");

        DataItem item = stack[marker_stack.top() + i];

        return item;
    }

    void dup(int i) {
        DataItem item = get(i);
        stack.push_back(item);
    }

    void dup() {
        dup(stack.size());
    }


    void swap(int top, int other) {
        if (top < 0 || other < 0 || top >= size() || other >= size() )
            throw logic_error("Invalid arguments for swap");

        top += marker_stack.top();
        other += marker_stack.top();

        std::swap(stack[top], stack[other]);
    }

    void swap(int other) {
        int top = size() - 1;
        swap(top, other);
    }

    void swap() {
        int top = size() - 1,
            other = size() - 2;
        swap(top, other);
    }

    void crunch(int bottom, int size) {
        if (bottom < 0 ||
            size < 0 ||
            bottom + size > this->size()) {
            throw logic_error("Crunching beyond stack");
        }

        bottom += marker_stack.top();

        vector<DataItem>::iterator b = stack.begin() + bottom;
        vector<DataItem>::iterator e = stack.begin() + bottom + size;

        stack.erase(b, e);
    }

    void trim(int at) {
        if (at < 0 || at > size())
            throw logic_error("Trimming beyond stack end");

        at += marker_stack.top();

        stack.resize(at);
    }

    void push_mark(int at) {
        if (at < 0 || at > size())
            throw logic_error("Invalid mark");

        at += marker_stack.top();

        marker_stack.push(at);
    }

    void push_mark() {
        push_mark(size());
    }

    int pop_mark() {
        // marker stack always begins with 0 as a top marker
        if (marker_stack.size() == 1)
            throw logic_error("Empty marker stack during pop_mark");

        int mark = marker_stack.top();
        marker_stack.pop();

        mark -= marker_stack.top();

        return mark;
    }
};
