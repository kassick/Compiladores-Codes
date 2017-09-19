/****************************************************************************
 *        Filename: "StackVM/include/vm.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Sep 18 14:06:37 2017"
 *         Updated: "2017-09-19 16:02:29 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <vector>
#include <stack>
#include <unordered_map>
#include <memory>
#include <string>
#include <ostream>
#include <istream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>

#include "data.H"

struct VM;

struct Instruction {
    typedef shared_ptr<Instruction> pointer;

    virtual VM& run(VM&) const = 0;

    virtual string to_string() const = 0;
};

struct VM {

    std::vector<DataItem> stack;
    std::stack<int> marker_stack;

    std::vector<Instruction::pointer> instructions;
    std::unordered_map<string, int> label_map;
    int pc = 0;
    int last_pc = -1;
    bool ok_to_go; // if the machine was correctly loaded

    istream& in;
    ostream& out;

    VM(istream& in_, ostream& out_) :
            in(in_), out(out_), ok_to_go(true)
    {
        marker_stack.push(0);
    }

    string to_string() const {
        stringstream ss;
        ss << "Stack: " << endl;

        for (int i = stack.size() - 1; i >= 0; i--)
        {
            ss << setw(5) << i - marker_stack.top()
               << " : " << stack[i].to_string()
               << endl;
        }

        ss << endl << "Code: (* is currenct pc, > is last pc)" << endl;

        for(int i = 0; i < instructions.size(); i++)
        {
            ss << (pc == i? "*" : (last_pc == i? ">" : " "))
               << setw(5) << i
               << " : " << instructions[i]->to_string()
               << endl;
        }

        return ss.str();
    }

    int set_pc_to(const string label) {
        const auto it = label_map.find(label);
        if (it == label_map.cend()) {
            pc = -1;
        } else {
            pc = it->second;
        }

        return pc;
    }

    int set_pc_to(const int target) {
        pc = target;
        return pc;
    }

    void run() {
        while(pc >= 0 && pc < instructions.size()) {
            try {
                last_pc = pc;
                Instruction::pointer instr = instructions[pc++];
                instr->run(*this);

                #ifndef NDEBUG
                cout << "--------------------"
                     << endl
                     << this->to_string()
                     << "--------------------"
                     << endl;
                std::this_thread::sleep_for (std::chrono::seconds(1));
                #endif

            } catch (out_of_range & e) {
                out << "Tried to access invalid pc " << pc << endl;
                pc = -1;
            } catch (logic_error & e) {
                out << e.what() << endl;
                pc = -1;
            }
        }
    }

    int size() const {
        return stack.size() - marker_stack.top();
    }

    int jump(int target) {
        if (target < instructions.size())
            pc = target;
        return pc;
    }

    template<typename T>
    void push(T&& v) {
        stack.emplace_back(std::forward<T>(v));
    }

    DataItem pop() {
        if (stack.size() == marker_stack.top())
            throw logic_error("Popping beyond top mark");

        DataItem item = std::move(stack[stack.size() - 1]);
        stack.resize(stack.size() - 1);
        return item;
    }

    DataItem get(int i) const {
        if (i < 0 || i >= size())
            throw logic_error("Trying to get item past stack end");

        DataItem item = stack[marker_stack.top() + i];

        return item;
    }

    void dup(int i) {
        DataItem item = get(i);
        stack.push_back(item);
    }

    void dup() {
        dup(size() - 1);
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
