/****************************************************************************
 *        Filename: "StackVM/include/instructions.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Sep 18 14:14:39 2017"
 *         Updated: "2017-10-23 12:07:57 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "vm.H"

#include <cassert>

struct NopInstr : public Instruction {
    virtual VM& run(VM& vm) const override {
        // nop
        return vm;
    }

    virtual string to_string() const override {
        return "nop";
    }
};

// mark
// Adds a marker that makes the stack begin anew on top of the current stack
struct PushStackMarkInstr: public Instruction {
    int keep;

    PushStackMarkInstr() : keep(0) {}
    PushStackMarkInstr(int keep_) : keep(keep_) {}

    virtual VM& run(VM& vm) const override {
        vm.push_mark(vm.size() - keep);
        return vm;
    }

    virtual string to_string() const override {
        return string("mark") + std::to_string(keep);
    }
};

// pop_mark
// Pops the mark and gives access to the previous stack
struct PopStackMarkInstr: public Instruction {
    virtual VM& run(VM& vm) const override {
        int mark = vm.pop_mark();
        DataItem itm(mark);
        vm.push(std::move(itm));
        return vm;
    }

    virtual string to_string() const override {
        return "pop_mark";
    }
};

// drop_mark     ==    pop_mark ; pop
// Pops the mark and gives access to the previous stack
struct DropStackMarkInstr: public Instruction {
    virtual VM& run(VM& vm) const override {
        vm.pop_mark();
        return vm;
    }

    virtual string to_string() const override {
        return "drop_mark";
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

    virtual string to_string() const override {

        return string("crunch ") +
                std::to_string(base) +
                string(" ") +
                std::to_string(size);
    }

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

    virtual string to_string() const override {
        return string("trim ") + std::to_string(base);
    }

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

    virtual string to_string() const override {
        return string("swap ") + std::to_string(i) + string(" ") + std::to_string(j);
    }

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

    virtual string to_string() const override {
        return "dup";
    }
};

// acreate           : creates an array with size=top , value = the rest
// acreate l         : creates an array of size l
struct ArrayCreateInstr: public Instruction {
    int len;

    ArrayCreateInstr(int len_) : len(len_) {}
    ArrayCreateInstr() : ArrayCreateInstr(-1) {}

    virtual string to_string() const override {
        return string("acreate ") +
                std::to_string(len);
    }

    VM& run(VM& vm) const override {
        int rl = len;
        if (rl < 0) {
            DataItem len_itm = vm.pop();

            if (len_itm.type != Int)
                throw logic_error("Did not find array len on top");

            rl = len_itm.as<int>();
        }

        if (rl < 0 || rl > vm.size())
        {
            vm.out << "ERROR: Not enough data to array creation" << endl;
            vm.ok_to_go = false;
            return vm;
        }

        std::vector<DataItem> arr(rl);
        for (int i = 0; i < rl; i++) {
            DataItem itm = vm.pop();
            arr[rl - i - 1] = std::move(itm);
            assert(itm.type == UNINITIALIZED);
        }

        vm.push(arr);

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

    virtual string to_string() const override {
        return "aload";
    }
};

// alen             : gets the len of the array at the top of the stack
struct ArrayLenInstr: public Instruction {

    VM& run(VM& vm) const override {

        const DataItem arr = std::move(vm.pop());
        if (arr.type != Array)
            throw logic_error("Trying to unpack something not an array");

        // this is owned by the arr DataItem
        const vector<DataItem>* arr_data = arr.as<vector<DataItem>*>();

        vm.push((int)arr_data->size());

        return vm;
    }

    virtual string to_string() const override {
        return "alen";
    }
};

// aget     : Array get the indirect position [ arr pos ]
// aget i   : Array get the direct position   [ arr ]
struct ArrayGetInstr: public Instruction {
    int pos;

    ArrayGetInstr(int pos_) : pos(pos_) {}
    ArrayGetInstr(): ArrayGetInstr(-1) {}

    virtual string to_string() const override {
        return string("aget ") + std::to_string(pos);
    }

    VM& run(VM& vm) const override {

        int the_pos = pos;

        if (the_pos < 0) {
            DataItem pos_itm = vm.pop();
            if (pos_itm.type != Int) {
                throw logic_error("Trying to access array with something not an integer");
            }

            the_pos = pos_itm.as<int>();
        }

        const DataItem arr = std::move(vm.pop());
        if (arr.type != Array)
            throw logic_error("Trying to unpack something not an array");

        // this is owned by the arr DataItem
        const vector<DataItem>* arr_data = arr.as<vector<DataItem>*>();

        if (the_pos < 0 || the_pos >= arr_data->size())
            throw logic_error("Invalid position for array access");

        DataItem itm = std::move((*arr_data)[the_pos]);

        vm.push(std::move(itm));

        return vm;
    }
};

// aset      : Indirect array set stack   [arr pos val]
// aset i    : Direct array set stack     [arr val]
struct ArraySetInstr: public Instruction {
    int pos;

    ArraySetInstr(int pos_) : pos(pos_) {}
    ArraySetInstr(): ArraySetInstr(-1) {}

    virtual string to_string() const override {
        return string("aset ") +
                std::to_string(pos);
    }

    VM& run(VM& vm) const override {
        // value
        // pos?
        // array

        DataItem value_itm = std::move(vm.pop());

        int the_pos = pos;
        if (the_pos < 0) {
            DataItem pos_itm = std::move(vm.pop());
            if (pos_itm.type != Int) {
                throw logic_error("Trying to access array with something not an integer");
            }

            the_pos = pos_itm.as<int>();
        }

        DataItem arr = std::move(vm.pop());
        if (arr.type != Array)
            throw logic_error("Trying to unpack something not an array");

        // this is owned by the arr DataItem
        vector<DataItem>* arr_data = arr.as<vector<DataItem>*>();

        if (the_pos < 0 || the_pos >= arr_data->size())
            throw logic_error("Invalid position for array access");

        // cerr << "BEFORE: "
        //      << "arr_data at " << (void*)arr_data
        //      << " swapping " << (*arr_data)[the_pos].to_string()
        //      << " with " << value_itm.to_string()
        //      << endl;
        (*arr_data)[the_pos] = std::move(value_itm);
        assert(value_itm.type == UNINITIALIZED);
        // cerr << "AFTER: "
        //      << "arr_data at " << (void*)arr_data
        //      << " swappped old " << (*arr_data)[the_pos].to_string()
        //      << " with new" << value_itm.to_string()
        //      << endl;

        vm.push(std::move(arr));

        // cerr << "AFTER PUSH: "
        //      << " top of the stack has " << vm.stack[vm.size() - 1].to_string()
        //      << " arr has " << arr.to_string()
        //      << " value_itm has " << value_itm.to_string()
        //      << endl;

        return vm;
    }
};

// push literal
// push 1
// push 2.0
// push 'a'
template <typename T>
struct PushInstr : public Instruction {

    DataItem d;

    PushInstr(T val) : d(val) {}

    virtual string to_string() const override {
        return string("push ") + d.to_string();
    }

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

    virtual string to_string() const override {
        return string("push \"") + str_escape(val) + string("\"");
    }

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

    virtual string to_string() const override {
        return "push null";
    }

    virtual VM& run(VM& vm) const override {

        vm.push(DataItem());

        return vm;
    }
};

// push stack_size
struct StackSize {};
template<>
struct PushInstr<StackSize>: public Instruction {

    virtual string to_string() const override {
        return "push stack_size";
    }

    virtual VM& run(VM& vm) const override {
        DataItem itm(vm.size());
        vm.push(itm);
        return vm;
    }
};

// popn     : pops n items (n in stack)
// popn i   : pops i items
struct PopNInstr : public Instruction {
    int n;

    PopNInstr(int n_) : n(n_) {};
    PopNInstr(): n(-1) {};

    virtual string to_string() const override {
        return string("popn ") + std::to_string(n);
    }

    virtual VM& run(VM & vm) const override{
        int howmany = n;
        if (howmany < 0) {
            DataItem itm = std::move(vm.pop());
            if (itm.type != Int) {
                throw logic_error("Popping N, but in is not an int");
            }

            howmany = itm.as<int>();
        }

        while(howmany--)
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

    virtual string to_string() const override {
        return string("store ") +
                std::to_string(target);
    }

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

    virtual string to_string() const override {
        return string("load ") +
                std::to_string(target);
    }

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
// read[cid]
// reads char, int, double and add it to the stack
template <typename T>
struct ReadInstr : public Instruction {

    virtual string to_string() const override {
        return string( "read_" ) +
                typeid(T).name();
    }

    virtual VM& run(VM & vm) const override {
        T val;

        if (vm.interactive)
            vm.out << "Input (" << typeid(T).name() << ") : ";

        try {
            vm.in >> val;
        } catch (exception & e) {
            vm.out << "Error reading: " << e.what();
        }

        vm.push(val);

        return vm;
    }
};

template <>
struct ReadInstr<int> : public Instruction {

    virtual string to_string() const override {
        return string( "read_i");
    }

    virtual VM& run(VM & vm) const override {
        int val;

        if (vm.interactive)
            vm.out << "Input (int) : ";

        try {
            string tmp;
            vm.in >> tmp;
            val = stoi(tmp);
        } catch (exception & e) {
            vm.out << "Error reading: " << e.what();
        }

        vm.push(val);

        return vm;
    }
};

// reads : reads string and add it (as an unpacked array) to the top of the stack
template<>
struct ReadInstr<string> : public Instruction {

    virtual string to_string() const override {
        return "reads";
    }

    virtual VM& run(VM & vm) const override {
        string val;

        if (vm.interactive)
            vm.out << "Input (string) : ";
        vm.in >> val;

        for (const char c: val)
            vm.push(c);

        vm.push((int)val.length());

        return vm;
    }
};

// print  : prints the char, int, float, double on the top of the stack
struct PrintInstr : public Instruction {

    virtual string to_string() const override {
        return "print";
    }

    virtual VM& run(VM & vm) const override {
        DataItem val = vm.pop();
        stringstream sout;
        sout << val.value_to_string();
        string s = sout.str();

        if (vm.interactive)
            vm.out << "Output : ";
        vm.out << s;
        if (vm.interactive)
            vm.out << endl;

        vm.push((int)s.length());

        return vm;
    }
};

// prints  : prints the unpacked string in the stack
struct PrintStringInstr : public Instruction {

    virtual string to_string() const override {
        return "prints";
    }

    virtual VM& run(VM & vm) const override {

        DataItem len = vm.pop();
        if (len.type != Int)
            throw logic_error("Printing something that ain't string on PrintStr");

        int l = len.as<int>();
        if (l < 0)
            throw logic_error("Invalid string len");

        string s(l, ' '); // whitespace string with the right size
        while (l--)
        {
            DataItem c = vm.pop();
            if (c.type != Char)
                throw logic_error("Printing something that ain't a string");

            s[l] = c.as<char>();
        }

        if (vm.interactive)
            vm.out << "Output : ";

        vm.out << s;

        vm.push((int)s.length());

        if (vm.interactive)
            vm.out << endl;

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

    virtual string to_string() const override {
        return "add";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type == Double || rhs.type == Double)
        {
            vm.push(AddFunctor<double>()(lhs.cast<double>(), rhs.cast<double>()));
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

struct ModInstr : public Instruction {

    virtual string to_string() const override {
        return "mod";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
        {
            vm.out << "Wrong types for mod" << endl;
            vm.ok_to_go = false;
            return vm;
        }

        vm.push(lhs.as<int>() % rhs.as<int>());

        return vm;
    }
};

struct SubInstr : public Instruction {

    virtual string to_string() const override {
        return "sub";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type == Double || rhs.type == Double)
        {
            vm.push(SubFunctor<double>()(lhs.cast<double>(), rhs.cast<double>()));
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

    virtual string to_string() const override {
        return "mul";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type == Double || rhs.type == Double)
        {
            vm.push(MulFunctor<double>()(lhs.cast<double>(), rhs.cast<double>()));
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

    virtual string to_string() const override {
        return "div";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type == Double || rhs.type == Double)
        {
            vm.push(DivFunctor<double>()(lhs.cast<double>(), rhs.cast<double>()));
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


    virtual string to_string() const override {
        return "and";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
            throw logic_error("AND requires int types");

        vm.push(lhs.as<int>() && rhs.as<int>());

        return vm;
    }
};

// or  : logical or
struct OrInstr : public Instruction {

    virtual string to_string() const override {
        return "or";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
            throw logic_error("OR requires int types");

        vm.push(lhs.as<int>() || rhs.as<int>());

        return vm;
    }
};

// nullp
struct NullPredInstr : public Instruction {

    virtual string to_string() const override {
        return "nullp";
    }

    virtual VM& run(VM& vm) const override {
        DataItem top = vm.pop();

        vm.push((int)top.isNull());

        return vm;
    }
};

// not  : logical not
struct NotInstr : public Instruction {

    virtual string to_string() const override {
        return "not";
    }

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

    virtual string to_string() const override {
        return "band";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
            throw logic_error("AND requires int types");

        vm.push(lhs.as<int>() & rhs.as<int>());

        return vm;
    }
};

// bor  : logical or
struct BOrInstr : public Instruction {

    virtual string to_string() const override {
        return "bor";
    }

    virtual VM& run(VM & vm) const override {
        DataItem rhs = vm.pop();
        DataItem lhs = vm.pop();

        if (lhs.type != Int || rhs.type != Int)
            throw logic_error("OR requires int types");

        vm.push(lhs.as<int>() | rhs.as<int>());

        return vm;
    }
};

// bnot  : bitwise not
struct BNotInstr : public Instruction {

    virtual string to_string() const override {
        return "bnot";
    }

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

    virtual string to_string() const override {
        return string("jump ") + std::to_string(target);
    }

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

    virtual string to_string() const override {
        return string("bnz ") + std::to_string(target);
    }

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

    virtual string to_string() const override {
        return string("bz ") + std::to_string(target);
    }

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

    virtual string to_string() const override {
        return string("bneg ") + std::to_string(target);
    }

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

    virtual string to_string() const override {
        return string("bpos ") + std::to_string(target);
    }

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

    virtual string to_string() const override {
        return "push pc";
    }

    virtual VM& run(VM & vm) const override {
        vm.push(vm.pc);
        return vm;
    }
};

struct ExitInstr : public Instruction {

    virtual string to_string() const override {
        return "exit";
    }

    virtual VM& run(VM & vm) const override {
        vm.pc = -1;
        return vm;
    }
};

template <typename T>
struct CastToInstr : public Instruction {

    virtual string to_string() const override {
        return string("cast<") +
                typeid(T).name() +
                string(">");
    }

    virtual VM& run(VM & vm) const override {
        DataItem d = vm.pop();

        T tmp = d.cast<T>();

        vm.push(tmp);

        return vm;
    }
};

template <>
struct CastToInstr<string> : public Instruction {

    virtual string to_string() const override {
        return string("cast<string>");
    }

    virtual VM& run(VM & vm) const override {
        DataItem d = vm.pop();

        string s = d.value_to_string();
        PushInstr<string>(s).run(vm);

        return vm;
    }
};
