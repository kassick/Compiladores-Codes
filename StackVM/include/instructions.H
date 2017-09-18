/****************************************************************************
 *        Filename: "StackVM/include/instructions.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Sep 18 14:14:39 2017"
 *         Updated: "2017-09-18 16:58:03 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "vm.H"

// mark
// Adds a marker that makes the stack begin anew on top of the current stack
struct PushStackMarkInstr: public Instruction {
    virtual VM& run(VM& vm) const override {
        vm.push_mark();
        return vm;
    }
};

// pop_mark
// Pops the mark and gives access to the previous stack
struct PopStackMarkInstr: public Instruction {
    virtual VM& run(VM& vm) const override {
        int mark = vm.pop_mark();
        DataItem itm(mark);
        vm.push(itm);
        return vm;
    }
};

// drop_mark     ==    pop_mark ; pop
// Pops the mark and gives access to the previous stack
struct DropStackMarkInstr: public Instruction {
    virtual VM& run(VM& vm) const override {
        vm.pop_mark();
        return vm;
    }
};

// crunch        : crunch indirect (top = size, top-1 = base)
// crunsh s      : crunch indirect base
// crunch b s    : crunch from base , s items
struct CrunchInstr : public Instruction {
    int base, size;

    CrunchInstr(int base_, int size_) : base(base_), size(size_) {}
    CrunchInstr(int size_) : base(-1), size(size_) {}
    CrunchInstr() : base(-1), size(-1) {}

    virtual VM& run(VM& vm) const override {

        int the_base = base;
        int the_size = size;

        if (the_size < 0) {
            const DataItem size_itm = vm.pop();
            if (size_itm.type != Int)
                throw logic_error("Indirect crunch with something not an integer on base");
            the_size = size_itm.as<int>();
        }

        if (the_base < 0) {
            const DataItem base_itm = vm.pop();
            if (base_itm.type != Int)
                throw logic_error("Indirect crunch with something not an integer on base");
            the_base = base_itm.as<int>();
        }

        vm.crunch(the_base, the_size);
        return vm;
    }
};

// trim       : trims indirect
// trim b     : trims at base b
struct TrimInstr : public Instruction {
    int base;

    TrimInstr(int base_) : base(base_) {}
    TrimInstr() : base(-1) {}

    virtual VM& run(VM& vm) const override {
        int the_base = base;
        if (the_base < 0) {
            const DataItem base_itm = vm.pop();
            if (base_itm.type != Int)
                throw logic_error("Indirect crunch with something not an integer on base");
            the_base = base_itm.as<int>();
        }

        vm.trim(the_base);
        return vm;
    }
};

// swap      : swaps the top two items
// swap i    : swapths the top with the position i
// swap i j  : swaps stack position i with j
struct SwapInstr : public Instruction {
    int i, j;

    // Swap two positions
    SwapInstr(int _i, int _j) : i(_i), j(_j) {}

    // Swap top with other
    SwapInstr(int _j) : SwapInstr(-1, _j) {}

    // Swap top with next
    SwapInstr() : SwapInstr(-1, -1) {}

    virtual VM& run(VM& vm) const override {
        if (i < 0 && j < 0)
            vm.swap();
        else if (i < 0)
            vm.swap(j);
        else
            vm.swap(i, j);

        return vm;
    }
};

// dup          : dups the top of the stack
struct DupInstr : public Instruction {
    virtual VM& run(VM& vm) const override {
        vm.dup();
        return vm;
    }
};

// acreate           : creates an array with size=top , value = the rest
// acreate l         : creates an array of size l
struct ArrayCreateInstr: public Instruction {
    int len;

    ArrayCreateInstr(int len_) : len(len_) {}
    ArrayCreateInstr() : ArrayCreateInstr(-1) {}

    VM& run(VM& vm) const override {
        int rl = len;
        if (rl < 0) {
            DataItem len_itm = vm.pop();

            if (len_itm.type != Int)
                throw logic_error("Did not find array len on top");

            rl = len_itm.as<int>();
        }

        std::vector<DataItem> arr(rl);
        for (int i = 0; i < rl; i++) {
            DataItem itm = vm.pop();
            arr[rl - i - 1] = itm;
        }

        return vm;
    }
};

// aload               : unpacks the array at the top of the stack
struct ArrayLoadInstr: public Instruction {

    VM& run(VM& vm) const override {
        const DataItem arr = vm.pop();
        if (arr.type != Array)
            throw logic_error("Trying to unpack something not an array");

        // this is owned by the arr DataItem
        const vector<DataItem>* arr_data = arr.as<vector<DataItem>*>();

        for(const DataItem& itm : *arr_data)
            vm.push(itm);

        vm.push((int)arr_data->size());

        return vm;
    }
};

// alen             : gets the len of the array at the top of the stack
struct ArrayLenInstr: public Instruction {

    VM& run(VM& vm) const override {

        const DataItem arr = vm.pop();
        if (arr.type != Array)
            throw logic_error("Trying to unpack something not an array");

        // this is owned by the arr DataItem
        const vector<DataItem>* arr_data = arr.as<vector<DataItem>*>();

        vm.push((int)arr_data->size());

        return vm;
    }
};

// aget     : Array get the indirect position
// aget i   : Array get the direct position
struct ArrayGetInstr: public Instruction {
    int pos;

    ArrayGetInstr(int pos_) : pos(pos_) {}
    ArrayGetInstr(): ArrayGetInstr(-1) {}

    VM& run(VM& vm) const override {

        int the_pos = pos;

        if (the_pos < 0) {
            DataItem pos_itm = vm.pop();
            if (pos_itm.type != Int) {
                throw logic_error("Trying to access array with something not an integer");
            }

            the_pos = pos_itm.as<int>();
        }

        const DataItem arr = vm.pop();
        if (arr.type != Array)
            throw logic_error("Trying to unpack something not an array");

        // this is owned by the arr DataItem
        const vector<DataItem>* arr_data = arr.as<vector<DataItem>*>();

        if (the_pos < 0 || the_pos >= arr_data->size())
            throw logic_error("Invalid position for array access");

        DataItem itm = (*arr_data)[the_pos];

        vm.push(itm);

        return vm;
    }
};

// aset      : Indirect array set stack = [val pos arr]
// aset i    : Direct array set stack = [val arr]
struct ArraySetInstr: public Instruction {
    int pos;

    ArraySetInstr(int pos_) : pos(pos_) {}
    ArraySetInstr(): ArraySetInstr(-1) {}

    VM& run(VM& vm) const override {
        // value
        // pos?
        // array

        DataItem value_itm = vm.pop();

        int the_pos = pos;
        if (the_pos < 0) {
            DataItem pos_itm = vm.pop();
            if (pos_itm.type != Int) {
                throw logic_error("Trying to access array with something not an integer");
            }

            the_pos = pos_itm.as<int>();
        }

        DataItem arr = vm.pop();
        if (arr.type != Array)
            throw logic_error("Trying to unpack something not an array");

        // this is owned by the arr DataItem
        vector<DataItem>* arr_data = arr.as<vector<DataItem>*>();

        if (pos < 0 || pos >= arr_data->size())
            throw logic_error("Invalid position for array access");

        (*arr_data)[the_pos] = value_itm;

        vm.push(arr);

        return vm;
    }
};

// push[cifd] literal
// pushi 1
// pushd 2.0
template <typename T>
struct PushInstr : public Instruction {

    DataItem d;

    PushInstr(T val) : d(val) {}

    virtual VM& run(VM& vm) const override {
        vm.push(d);
        return vm;
    }
};

// pushs "strvalue"
template <>
struct PushInstr<string> : public Instruction {

    string val;

    PushInstr(string val_) : val(val_) {}

    virtual VM& run(VM& vm) const override {
        for (const char c : val)
            vm.push(c);

        vm.push((int)val.size());

        return vm;
    }
};

// push null
struct Null {};
template <>
struct PushInstr<Null> : public Instruction {

    virtual VM& run(VM& vm) const override {

        vm.push(DataItem());

        return vm;
    }
};

// push stack_size
struct StackSize {};
template<>
struct PushInstr<StackSize>: public Instruction {
    virtual VM& run(VM& vm) const override {
        DataItem itm(vm.size());
        vm.push(itm);
        return vm;
    }
};

// pop
struct PopInstr : public Instruction {
    virtual VM& run(VM & vm) const override{
        vm.pop();
        return vm;
    }
};

// store          : store indirect
// store 0        : store at position
// Stores the top of the stack on the given fixed position
struct StoreInstr : public Instruction {
    int target;

    StoreInstr(int target_) : target(target_) {}
    StoreInstr() : target(-1) {}

    virtual VM& run(VM& vm) const override {
        // swap top with target

        int the_target = target;
        if (the_target < 0) {
            const DataItem target_itm = vm.pop();
            if (target_itm.type != Int)
                throw logic_error("Indirect store with something not an integer on base");
            the_target = target_itm.as<int>();
        }

        vm.swap(the_target);
        vm.pop();
        return vm;
    }
};

// load 0
// Load (dup) a given position of the stack on the top
struct LoadInstr : public Instruction {
    int target;

    LoadInstr(int target_) : target(target_) {}
    LoadInstr() : target(-1) {}

    virtual VM& run(VM& vm) const override {
        int the_target = target;
        if (the_target < 0) {
            const DataItem target_itm = vm.pop();
            if (target_itm.type != Int)
                throw logic_error("Indirect load with something not an integer on base");
            the_target = target_itm.as<int>();
        }

        vm.dup(the_target);

        return vm;
    }
};

// I/O
// read[cifd]
// reads char, int, float, double and add it to the stack
template <typename T>
struct ReadInstr : public Instruction {
    virtual VM& run(VM & vm) const override {
        T val;
        cin >> val;

        vm.push(val);
    }
};

// reads : reads string and add it (as an unpacked array) to the top of the stack
template<>
struct ReadInstr<string> : public Instruction {
    virtual VM& run(VM & vm) const override {
        string val;

        cin >> val;

        for (const char c: val)
            vm.push(c);

        vm.push((int)val.length());

        return vm;
    }
};

// print  : prints the char, int, float, double on the top of the stack
struct PrintInstr : public Instruction {
    virtual VM& run(VM & vm) const override {
        DataItem val = vm.pop();

        cout << val.value_to_string();

        return vm;
    }
};

// prints  : prints the unpacked string in the stack
struct PrintString : public Instruction {
    virtual VM& run(VM & vm) const override {

        DataItem len = vm.pop();
        if (len.type != Int)
            throw logic_error("Printing something that ain't string on PrintStr");

        int l = len.as<int>();

        if (l < 0) throw logic_error("Invalid string len");

        while (l--)
        {
            DataItem c = vm.pop();
            if (c.type != Char)
                throw logic_error("Printing something that ain't a string");

            cout << c.as<char>();
        }

        return vm;
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

    virtual VM& run(VM & vm) const override {
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

    virtual VM& run(VM & vm) const override {
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

    virtual VM& run(VM & vm) const override {
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

    virtual VM& run(VM & vm) const override {
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


// and   : logical and
struct AndInstr : public Instruction {

    virtual VM& run(VM & vm) const override {
        DataItem lhs = vm.pop();
        DataItem rhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
            throw logic_error("AND requires int types");

        vm.push(lhs.as<int>() && rhs.as<int>());

        return vm;
    }
};

// or  : logical or
struct OrInstr : public Instruction {

    virtual VM& run(VM & vm) const override {
        DataItem lhs = vm.pop();
        DataItem rhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
            throw logic_error("OR requires int types");

        vm.push(lhs.as<int>() || rhs.as<int>());

        return vm;
    }
};

// nullp
struct NullPredInstr : public Instruction {
    virtual VM& run(VM& vm) const override {
        DataItem top = vm.pop();

        vm.push((int)top.isNull());

        return vm;
    }
};

// not  : logical not
struct NotInstr : public Instruction {

    virtual VM& run(VM & vm) const override {
        DataItem lhs = vm.pop();

        if (lhs.type != Int)
            throw logic_error("NOT requires int types");

        vm.push(!lhs.as<int>());

        return vm;
    }
};

// band   : bitwise and
struct BAndInstr : public Instruction {

    virtual VM& run(VM & vm) const override {
        DataItem lhs = vm.pop();
        DataItem rhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
            throw logic_error("AND requires int types");

        vm.push(lhs.as<int>() & rhs.as<int>());

        return vm;
    }
};

// bor  : logical or
struct BOrInstr : public Instruction {

    virtual VM& run(VM & vm) const override {
        DataItem lhs = vm.pop();
        DataItem rhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
            throw logic_error("OR requires int types");

        vm.push(lhs.as<int>() | rhs.as<int>());

        return vm;
    }
};

// bnot  : bitwise not
struct BNotInstr : public Instruction {

    virtual VM& run(VM & vm) const override {
        DataItem lhs = vm.pop();

        if (lhs.type != Int)
            throw logic_error("NOT requires int types");

        vm.push(~ lhs.as<int>());

        return vm;
    }
};

// jump
// jump target
struct JumpInstr : public Instruction {

    int target;

    JumpInstr(int target_) : target(target_) {}
    JumpInstr(): JumpInstr(-1) {}

    virtual VM& run(VM & vm) const override {
        int the_target = target;
        if (the_target < 0) {
            const DataItem target_itm = vm.pop();
            if (target_itm.type != Int)
                throw logic_error("Indirect jump with non-int target");

            the_target = target_itm.as<int>();
        }

        vm.jump(the_target);

        return vm;
    }
};

// bnz
// bnz target   : branch if not zero
struct BnzInstr : public Instruction {

    int target;

    BnzInstr(int target_) : target(target_) {}
    BnzInstr(): BnzInstr(-1) {}

    virtual VM& run(VM & vm) const override {
        int the_target = target;
        if (the_target < 0) {
            const DataItem target_itm = vm.pop();
            if (target_itm.type != Int)
                throw logic_error("Indirect jump with non-int target");

            the_target = target_itm.as<int>();
        }

        const DataItem b = vm.pop();

        if (!b.isZero())
            vm.jump(the_target);

        return vm;
    }
};

// bz
// bz target   : branch if zero
struct BzInstr : public Instruction {

    int target;

    BzInstr(int target_) : target(target_) {}
    BzInstr(): BzInstr(-1) {}

    virtual VM& run(VM & vm) const override {
        int the_target = target;
        if (the_target < 0) {
            const DataItem target_itm = vm.pop();
            if (target_itm.type != Int)
                throw logic_error("Indirect jump with non-int target");

            the_target = target_itm.as<int>();
        }

        const DataItem b = vm.pop();

        if (b.isZero())
            vm.jump(the_target);

        return vm;
    }
};

// bneg
// bneg target   : branch if negative
struct BNegInstr : public Instruction {

    int target;

    BNegInstr(int target_) : target(target_) {}
    BNegInstr(): BNegInstr(-1) {}

    virtual VM& run(VM & vm) const override {
        int the_target = target;
        if (the_target < 0) {
            const DataItem target_itm = vm.pop();
            if (target_itm.type != Int)
                throw logic_error("Indirect jump with non-int target");

            the_target = target_itm.as<int>();
        }

        const DataItem b = vm.pop();

        if (b.isNegative())
            vm.jump(the_target);

        return vm;
    }
};

// bpos
// bpos target   : branch if negative
struct BPosInstr : public Instruction {

    int target;

    BPosInstr(int target_) : target(target_) {}
    BPosInstr(): BPosInstr(-1) {}

    virtual VM& run(VM & vm) const override {
        int the_target = target;
        if (the_target < 0) {
            const DataItem target_itm = vm.pop();
            if (target_itm.type != Int)
                throw logic_error("Indirect jump with non-int target");

            the_target = target_itm.as<int>();
        }

        const DataItem b = vm.pop();

        if (b.isPositive())
            vm.jump(the_target);

        return vm;
    }
};

// push_pc
struct PC {} ;
template <>
struct PushInstr<PC> : public Instruction {

    virtual VM& run(VM & vm) const override {
        vm.push(vm.pc);
        return vm;
    }
};
