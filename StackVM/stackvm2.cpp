/****************************************************************************
 *        Filename: "stackvm.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 13 11:00:28 2017"
 *         Updated: "2017-09-13 16:06:50 kassick"
 *
 *          Author:
 *
 *                    Copyright (C) 2017,
 ****************************************************************************/

#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

enum DataType {
    UNINITIALIZED = -1,
    Char = 0,
    Int,
    Float,
    Double,
};

const char * dataTypeNameStr(DataType t) {
    switch (t) {
        case Char: return "char";
        case Int: return "int";
        case Float: return "float";
        case Double: return "double";
        default: return "UNINITIALIZED";
    }
}

struct DataItem {
    DataType type;
    vector<char> data;

    DataItem() {
        type = UNINITIALIZED;
    }

    template <typename T>
    DataItem(T v, DataType _type ) :
            type(_type),
            data(&v, &v + sizeof(T))
    {}

    DataItem(const char c) : DataItem(c, Char) {}

    DataItem(const int i) : DataItem(i, Int) {}

    DataItem(const float d) : DataItem(d, Float) {}

    DataItem(const double d) : DataItem(d, Double) {}

    template <typename T>
    T as() const {
        if (sizeof(T) != data.size())
            throw logic_error("Trying to convert something of the wrong size");

        return *(T*)data.data();
    }

    template <typename T>
    T cast() const {
        switch(type) {
            case Char: return (T)this->as<char>();
            case Int: return (T)this->as<int>();
            case Float: return (T)this->as<float>();
            case Double: return (T)this->as<double>();
            default:
                throw logic_error("Invalid type in cast");
        }

        throw logic_error("Invalid type in cast");
    }

    template <typename T>
    static DataItem createWithType(T i, DataType t) {
        switch (t) {
            case Char: return DataItem((char)i);
            case Int: return DataItem((int)i);
            case Float: return DataItem((float)i);
            case Double: return DataItem((double)i);
            default:
                throw logic_error("Invalid type in create_and_cast");
        }
    }

    DataItem dataCastAs(DataType d) {
        if (d == this->type)
            return *this;

        switch(this->type) {
            case Char: return createWithType(this->as<char>(), d);
            case Int: return createWithType(this->as<int>(), d);
            case Float: return createWithType(this->as<float>(), d);
            case Double: return createWithType(this->as<double>(), d);
            default:
                throw logic_error("Invalid target type to dataCastAs");
        }
    }

    string to_string() const {
        stringstream ss;

        ss << dataTypeNameStr(this->type)
           << " : ";
        switch (this->type) {
            case Char: ss << "'" << as<char>() << "'";
            case Int: ss << as<int>();
            case Float: ss << as<float>();
            case Double: ss << as<double>();
            default: ss << "null";
        }

        return ss.str();
    }

    string value_to_string() const {
        stringstream ss;

        switch (this->type) {
            case Char:   return std::to_string( as<char>() );
            case Int:    return std::to_string( as<int>() );
            case Float:  return std::to_string( as<float>() );
            case Double: return std::to_string( as<double>() );
            default:     return "null";
        }

        return ss.str();
    }
};
};

struct VM {

    std::vector<DataItem> stack;

    template<typename T>
    void push(T v) {
        stack.emplace_back(v);
    }

    DataItem pop() {
        if (stack.size() == 0)
            throw logic_error("Popping empty stack");

        DataItem item = stack[stack.size() - 1];
        stack.resize(stack.size() - 1);
        return item;
    }

    DataItem get(int i) {
        if (i > stack.size())
            throw logic_error("Trying to get item past stack end");

        DataItem item = stack[stack.size() - i - 1];

        return item;
    }

    void dup(int i = 0) {
        DataItem item = get(i);
        stack.push_back(item);
    }


    void swap(int top, int other) {
        if (top < 0 || other < 0 || top > stack.size() || other > stack.size())
            throw logic_error("Invalid arguments for swap");

        std::swap(stack[top], stack[other]);
    }

    void swap(int other) {
        int top = stack.size() - 1;
        swap(top, other);
    }

    void swap() {
        int top = stack.size() - 1,
            other = stack.size() - 2;
        swap(top, other);
    }

    void crunch(int bottom, int size) {
        if (bottom + size > stack.size()) {
            throw logic_error("Crunching beyond stack");
        }

        vector<DataItem>::iterator b = stack.begin() + bottom;
        vector<DataItem>::iterator e = stack.begin() + bottom + size;

        stack.erase(b, e);
    }

    void trim(int at) {
        if (at < 0 || at >= stack.size())
            throw logic_error("Trimming beyond stack end");

        stack.resize(at);
    }
};

struct Instruction {
    virtual VM& run(VM&) = 0;
};

template <typename T>
struct PushInstr : public Instruction {

    DataItem d;

    PushInstr(T val) : d(val) {}

    virtual VM& run(VM& vm) {
        vm.push(d);
        return vm;
    }
};

template <>
struct PushInstr<string> : public Instruction {

    string val;

    PushInstr(string _val) : val(_val) {}

    virtual VM& run(VM& vm) {
        for (char c : val)
            vm.push(c);
        vm.push(val.length());
        return vm;
    }
};

// Store the top of the stack in some specific position of the stack
struct StoreInstr : public Instruction {
    int target;

    StoreInstr(int target_) : target(target_) {}

    virtual VM& run(VM& vm) {
        // swap top with target
        vm.swap(target);
        vm.pop();
        return vm;
    }
};

// Load (dup) a given position of the stack on the top
struct LoadInstr : public Instruction {
    int target;

    LoadInstr(int target_) : target(target_) {}

    virtual VM& run(VM& vm) {
        vm.dup(target);
        return vm;
    }
};

struct SwapInstr : public Instruction {
    int i, j;

    // Swap two positions
    SwapInstr(int _i, int _j) : i(_i), j(_j) {}

    // Swap top with other
    SwapInstr(int _j) : SwapInstr(-1, _j) {}

    // Swap top with next
    SwapInstr() : SwapInstr(-1, -1) {}

    virtual VM& run(VM& vm) {
        if (i < 0 && j < 0)
            vm.swap();
        else if (i < 0)
            vm.swap(j);
        else
            vm.swap(i, j);

        return vm;
    }
};

struct CrunchInstr : public Instruction {
    int base, size;

    CrunchInstr(int base_, int size_) :
            base(base_),
            size(size_)
    {}

    virtual VM& run(VM& vm) {
        vm.crunch(base, size);
        return vm;
    }
};

struct TrimInstr : public Instruction {
    int base;

    TrimInstr(int base_) : base(base_) {}

    virtual VM& run(VM& vm) {
        vm.trim(base);
        return vm;
    }
};

struct DupInstr : public Instruction {
    virtual VM& run(VM& vm) {
        vm.dup();
        return vm;
    }
};

struct PopInstr : public Instruction {

    virtual VM& run(VM & vm) {
        vm.pop();
        return vm;
    }
};

// I/O
template <typename T>
struct ReadInstr : public Instruction {
    virtual VM& run(VM & vm) {
        T val;
        val << cin;

        vm.push(val);
    }
};

struct PrintInstr : public Instruction {
    virtual VM& run(VM & vm) {
        DataItem val = vm.pop();
        cout << val.value_to_string();
    }
};

struct PrintStrInstr : public Instruction {
    virtual VM& run(VM & vm) {
        DataItem len = vm.pop();
        if (len->type != Int)
            throw logic_error("Printing something that ain't string on PrintStr");

        int l = len.as<int>();

        if (l < 0) throw logic_error("Invalid string len");

        while (l--)
        {
            // TODO
        }
    }
};

// Binary Operators
template <typename T>
struct AddFunctor {
    template <typename L, typename R>
    T operator()(L lhs, R rhs) {
        return lhs + rhs;
    }
};

template <typename T>
struct SubFunctor {
    template <typename L, typename R>
    T operator()(L lhs, R rhs) {
        return lhs - rhs;
    }
};

template <typename T>
struct MulFunctor {
    template <typename L, typename R>
    T operator()(L lhs, R rhs) {
        return lhs * rhs;
    }
};

template <typename T>
struct DivFunctor {
    template <typename L, typename R>
    T operator()(L lhs, R rhs) {
        return lhs / rhs;
    }
};


struct AddInstr : public Instruction {

    virtual VM& run(VM & vm) {
        DataItem lhs = vm.pop();
        DataItem rhs = vm.pop();

        if (lhs.type == Double || rhs.type == Double)
        {
            vm.push(AddFunctor<double>()(lhs.cast<double>(), rhs.cast<double>()));
        } else if (lhs.type == Float || rhs.type == Float)
        {
            vm.push(AddFunctor<float>()(lhs.cast<float>(), rhs.cast<float>()));
        } else if (lhs.type == Int || rhs.type == Int)
        {
            vm.push(AddFunctor<int>()(lhs.cast<int>(), rhs.cast<int>()));
        } else if (lhs.type == Char || rhs.type == Char)
        {
            vm.push(AddFunctor<char>()(lhs.cast<char>(), rhs.cast<char>()));
        }

        return vm;
    }
};


struct SubInstr : public Instruction {

    virtual VM& run(VM & vm) {
        DataItem lhs = vm.pop();
        DataItem rhs = vm.pop();

        if (lhs.type == Double || rhs.type == Double)
        {
            vm.push(SubFunctor<double>()(lhs.cast<double>(), rhs.cast<double>()));
        } else if (lhs.type == Float || rhs.type == Float)
        {
            vm.push(SubFunctor<float>()(lhs.cast<float>(), rhs.cast<float>()));
        } else if (lhs.type == Int || rhs.type == Int)
        {
            vm.push(SubFunctor<int>()(lhs.cast<int>(), rhs.cast<int>()));
        } else if (lhs.type == Char || rhs.type == Char)
        {
            vm.push(SubFunctor<char>()(lhs.cast<char>(), rhs.cast<char>()));
        }

        return vm;
    }
};

struct MulInstr : public Instruction {

    virtual VM& run(VM & vm) {
        DataItem lhs = vm.pop();
        DataItem rhs = vm.pop();

        if (lhs.type == Double || rhs.type == Double)
        {
            vm.push(MulFunctor<double>()(lhs.cast<double>(), rhs.cast<double>()));
        } else if (lhs.type == Float || rhs.type == Float)
        {
            vm.push(MulFunctor<float>()(lhs.cast<float>(), rhs.cast<float>()));
        } else if (lhs.type == Int || rhs.type == Int)
        {
            vm.push(MulFunctor<int>()(lhs.cast<int>(), rhs.cast<int>()));
        } else if (lhs.type == Char || rhs.type == Char)
        {
            vm.push(MulFunctor<char>()(lhs.cast<char>(), rhs.cast<char>()));
        }

        return vm;
    }
};

struct DivInstr : public Instruction {

    virtual VM& run(VM & vm) {
        DataItem lhs = vm.pop();
        DataItem rhs = vm.pop();

        if (lhs.type == Double || rhs.type == Double)
        {
            vm.push(DivFunctor<double>()(lhs.cast<double>(), rhs.cast<double>()));
        } else if (lhs.type == Float || rhs.type == Float)
        {
            vm.push(DivFunctor<float>()(lhs.cast<float>(), rhs.cast<float>()));
        } else if (lhs.type == Int || rhs.type == Int)
        {
            vm.push(DivFunctor<int>()(lhs.cast<int>(), rhs.cast<int>()));
        } else if (lhs.type == Char || rhs.type == Char)
        {
            vm.push(DivFunctor<char>()(lhs.cast<char>(), rhs.cast<char>()));
        }

        return vm;
    }
};
