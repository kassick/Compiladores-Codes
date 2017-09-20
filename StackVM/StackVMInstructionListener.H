/****************************************************************************
 *        Filename: "StackVM/StackVMLabelListener.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Sep 18 18:08:15 2017"
 *         Updated: "2017-09-20 15:45:36 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "StackVMBaseListener.h"
#include "vm.H"
#include "instructions.H"

namespace StackVM {


struct InstructionListener : public StackVMBaseListener
{
    int cur_line;
    int errors;
    ostream & out;
    VM & vm;

    Instruction::pointer instr;

    int intarg;
    char chararg;
    double doublearg;
    string strarg;
    int resolved_label_arg;

    InstructionListener(VM& vm_, ostream & out_) :
            vm(vm_),
            out(out_),
            cur_line(0),
            errors(0),
            instr(nullptr)
    {}

    virtual
    void enterInstructionLine(StackVMParser::InstructionLineContext * ctx)
            override
    {
    }

    virtual
    void exitInstructionLine(StackVMParser::InstructionLineContext * /*ctx*/)
            override
    {
        if (!instr) {
            vm.ok_to_go = false;
            errors++;
            throw logic_error("Arrived at the end of instruction line without an instruction");
        }

        vm.instructions[cur_line] = instr;
        cur_line++;
        instr.reset();
    }

    virtual void exitMark(StackVMParser::MarkContext * ctx) override
    {
        instr = make_shared<PushStackMarkInstr>();
    }

    virtual void exitMarkArg(StackVMParser::MarkArgContext * /*ctx*/) override {
        instr = make_shared<PushStackMarkInstr>(this->intarg);
    }

    virtual void exitPopMark(StackVMParser::PopMarkContext * /*ctx*/) override {
        instr = make_shared<PopStackMarkInstr>();
    }

    virtual void exitDropMark(StackVMParser::DropMarkContext * /*ctx*/) override {
        instr = make_shared<DropStackMarkInstr>();
    }

    virtual void exitCrunchNoArgs(StackVMParser::CrunchNoArgsContext * /*ctx*/) override {
        instr = make_shared<CrunchInstr>();
    }

    virtual
    void exitCrunchBaseIndirect(StackVMParser::CrunchBaseIndirectContext * ctx)
            override
    {
        int size = this->intarg;

        instr = make_shared<CrunchInstr>(size);
    }

    virtual void exitCrunchDirect(StackVMParser::CrunchDirectContext * ctx)
            override
    {
        int size = stoi(ctx->size->getText());
        int base = stoi(ctx->base->getText());

        instr = make_shared<CrunchInstr>(base, size);
    }

    virtual void exitTrimIndirect(StackVMParser::TrimIndirectContext * /*ctx*/)
            override
    {
        instr = make_shared<TrimInstr>();
    }

    virtual void exitTrimDirect(StackVMParser::TrimDirectContext * ctx)
            override
    {
        int base = this->intarg;

        instr = make_shared<TrimInstr>(base);
    }

    virtual void exitSwapTopWithNext(StackVMParser::SwapTopWithNextContext * /*ctx*/)
            override
    {
        instr = make_shared<SwapInstr>();
    }

    virtual void exitSwapTopWithArg(StackVMParser::SwapTopWithArgContext * ctx)
            override
    {
        int other = this->intarg;

        instr = make_shared<SwapInstr>(other);
    }

    virtual void exitSwap(StackVMParser::SwapContext * ctx)
            override
    {
        int top = stoi(ctx->a->getText());
        int other = stoi(ctx->b->getText());

        instr = make_shared<SwapInstr>(top, other);
    }

    virtual void exitDupTop(StackVMParser::DupTopContext * /*ctx*/)
        override
    {
        instr = make_shared<DupInstr>();
    }

    virtual void exitArrayCreate(StackVMParser::ArrayCreateContext * /*ctx*/)
        override
    {
        instr = make_shared<ArrayCreateInstr>();
    }

    virtual void exitArrayCreateArg(StackVMParser::ArrayCreateArgContext * ctx)
        override
    {
        int len = this->intarg;

        instr = make_shared<ArrayCreateInstr>(len);
    }

    virtual void exitArrayLoad(StackVMParser::ArrayLoadContext * /*ctx*/)
        override
    {
        instr = make_shared<ArrayLoadInstr>();
    }

    virtual void exitArrayLen(StackVMParser::ArrayLenContext * /*ctx*/)
        override
    {
        instr = make_shared<ArrayLenInstr>();
    }

    virtual void exitArrayGet(StackVMParser::ArrayGetContext * /*ctx*/)
        override
    {
        instr = make_shared<ArrayGetInstr>();
    }

    virtual void exitArrayGetArg(StackVMParser::ArrayGetArgContext * ctx)
        override
    {
        int i = this->intarg;

        instr = make_shared<ArrayGetInstr>(i);
    }

    virtual void exitArraySet(StackVMParser::ArraySetContext * /*ctx*/)
        override
    {
        instr = make_shared<ArraySetInstr>();
    }

    virtual void exitArraySetArg(StackVMParser::ArraySetArgContext * ctx)
        override
    {
        int i = this->intarg;

        instr = make_shared<ArraySetInstr>(i);
    }

    virtual void exitPushChar(StackVMParser::PushCharContext * ctx)
        override
    {
        char c = this->chararg;
        instr = make_shared<PushInstr<char>>(c);
    }

    virtual void exitPushInt(StackVMParser::PushIntContext * ctx)
        override
    {
        int i = this->intarg;
        instr = make_shared<PushInstr<int>>(i);
    }

    virtual void exitPushDouble(StackVMParser::PushDoubleContext * ctx)
        override
    {
        double d = this->doublearg;
        instr = make_shared<PushInstr<double>>(d);
    }

    virtual void exitPushString(StackVMParser::PushStringContext * ctx)
        override
    {
        string value = this->strarg;
        instr = make_shared<PushInstr<string>>(value);
    }

    virtual void exitPushLabel(StackVMParser::PushLabelContext * ctx)
        override
    {
        instr = make_shared<PushInstr<int>>(this->resolved_label_arg);
    }

    virtual void exitPushNull(StackVMParser::PushNullContext * /*ctx*/)
        override
    {
        instr = make_shared<PushInstr<Null>>();
    }

    virtual void exitPushStackSize(StackVMParser::PushStackSizeContext * /*ctx*/)
        override
    {
        instr = make_shared<PushInstr<StackSize>>();
    }

    virtual void exitPop(StackVMParser::PopContext * /*ctx*/)
        override
    {
        instr = make_shared<PopNInstr>(1);
    }

    virtual void exitPopNImediate(StackVMParser::PopNImediateContext * /*ctx*/)
            override
    {
        instr = make_shared<PopNInstr>(intarg);
    }

    virtual void exitPopN(StackVMParser::PopNContext * /*ctx*/)
            override
    {
        instr = make_shared<PopNInstr>();
    }

    virtual void exitStoreIndirect(StackVMParser::StoreIndirectContext * /*ctx*/)
        override
    {
        instr = make_shared<StoreInstr>();
    }

    virtual void exitStore(StackVMParser::StoreContext * ctx)
        override
    {
        int target = this->intarg;

        instr = make_shared<StoreInstr>(target);
    }

    virtual void exitLoadIndirect(StackVMParser::LoadIndirectContext * /*ctx*/)
        override
    {
        instr = make_shared<LoadInstr>();
    }

    virtual void exitLoad(StackVMParser::LoadContext * ctx)
            override
    {
        int target = this->intarg;

        instr = make_shared<LoadInstr>(target);
    }

    virtual void exitReadChar(StackVMParser::ReadCharContext * /*ctx*/)
        override
    {
        instr = make_shared<ReadInstr<char>>();
    }

    virtual void exitReadInt(StackVMParser::ReadIntContext * /*ctx*/)
        override
    {
        instr = make_shared<ReadInstr<int>>();
    }

    virtual void exitReadDouble(StackVMParser::ReadDoubleContext * /*ctx*/)
        override
    {
        instr = make_shared<ReadInstr<double>>();
    }

    virtual void exitReadString(StackVMParser::ReadStringContext * /*ctx*/)
        override
    {
        instr = make_shared<ReadInstr<string>>();
    }

    virtual void exitPrint(StackVMParser::PrintContext * /*ctx*/)
        override
    {
        instr = make_shared<PrintInstr>();
    }

    virtual void exitPrintString(StackVMParser::PrintStringContext * /*ctx*/)
        override
    {
        instr = make_shared<PrintStringInstr>();
    }

    virtual void exitAdd(StackVMParser::AddContext * /*ctx*/)
        override
    {
        instr = make_shared<AddInstr>();
    }

    virtual void exitSub(StackVMParser::SubContext * /*ctx*/)
        override
    {
        instr = make_shared<SubInstr>();
    }

    virtual void exitMult(StackVMParser::MultContext * /*ctx*/)
        override
    {
        instr = make_shared<MulInstr>();
    }

    virtual void exitDiv(StackVMParser::DivContext * /*ctx*/)
        override
    {
        instr = make_shared<DivInstr>();
    }

    virtual void exitAnd(StackVMParser::AndContext * /*ctx*/)
        override
    {
        instr = make_shared<AndInstr>();
    }

    virtual void exitOr(StackVMParser::OrContext * /*ctx*/)
        override
    {
        instr = make_shared<OrInstr>();
    }

    virtual void exitNot(StackVMParser::NotContext * /*ctx*/)
        override
    {
        instr = make_shared<NotInstr>();
    }

    virtual void exitNullPredicate(StackVMParser::NullPredicateContext * /*ctx*/)
        override
    {
        instr = make_shared<NullPredInstr>();
    }

    virtual void exitBitwiseAnd(StackVMParser::BitwiseAndContext * /*ctx*/)
        override
    {
        instr = make_shared<BAndInstr>();
    }

    virtual void exitBitwiseOr(StackVMParser::BitwiseOrContext * /*ctx*/)
        override
    {
        instr = make_shared<BOrInstr>();
    }

    virtual void exitBitwiseNot(StackVMParser::BitwiseNotContext * /*ctx*/)
        override
    {
        instr = make_shared<BNotInstr>();
    }

    virtual void exitJump(StackVMParser::JumpContext * /*ctx*/)
        override
    {
        instr = make_shared<JumpInstr>();
    }

    virtual void exitJumpImediate(StackVMParser::JumpImediateContext * ctx)
        override
    {
        instr = make_shared<JumpInstr>(resolved_label_arg);
    }

    virtual void exitBranchZero(StackVMParser::BranchZeroContext * /*ctx*/)
        override
    {
        instr = make_shared<BzInstr>();
    }

    virtual void exitBranchZeroImediate(StackVMParser::BranchZeroImediateContext * /*ctx*/)
        override
    {
        instr = make_shared<BzInstr>(resolved_label_arg);
    }

    virtual void exitBranchNotZero(StackVMParser::BranchNotZeroContext * /*ctx*/)
        override
    {
        instr = make_shared<BnzInstr>();
    }

    virtual void exitBranchNotZeroImediate(StackVMParser::BranchNotZeroImediateContext * /*ctx*/)
        override
    {
        instr = make_shared<BnzInstr>(resolved_label_arg);
    }

    virtual void exitBranchNegative(StackVMParser::BranchNegativeContext * /*ctx*/)
        override
    {
        instr = make_shared<BNegInstr>();
    }

    virtual void exitBranchNegativeImediate(StackVMParser::BranchNegativeImediateContext * /*ctx*/)
        override
    {
        instr = make_shared<BNegInstr>(resolved_label_arg);
    }

    virtual void exitBranchPositive(StackVMParser::BranchPositiveContext * /*ctx*/)
        override
    {
        instr = make_shared<BPosInstr>();
    }

    virtual void exitBranchPositiveImediate(StackVMParser::BranchPositiveImediateContext * /*ctx*/)
        override
    {
        instr = make_shared<BPosInstr>(resolved_label_arg);
    }

    virtual void exitPushPC(StackVMParser::PushPCContext * /*ctx*/)
        override
    {
        instr = make_shared<PushInstr<PC>>();
    }

    virtual void exitExit(StackVMParser::ExitContext * /*ctx*/) override {
        instr = make_shared<ExitInstr>();
    }

    virtual void exitRef_label(StackVMParser::Ref_labelContext * ctx)
            override
    {
        this->resolved_label_arg = 0;

        const auto it = vm.label_map.find(ctx->getText());
        if (it == vm.label_map.cend()) {
            out << "Could not find label ``"
                << ctx->getText()
                << "'' referenced at line "
                << ctx->getStart()->getLine()
                << endl;
            errors++;
        } else {
            this->resolved_label_arg = it->second;
        }
    }

    virtual void exitIntarg(StackVMParser::IntargContext * ctx)
        override
    {
        if (ctx->getText().length() > 2 &&
            ctx->getText()[0] == '0' &&
            ctx->getText()[1] == 'x')
        {
            this->intarg = stoi(ctx->getText() , 0, 16);
            //cout << "found hexa " << ctx->getText() << " now is " << intarg << endl;
        } else {
            this->intarg = stoi(ctx->getText());
            //cout << "found decimal " << ctx->getText() << " now is " << intarg << endl;
        }
    }

    virtual void exitNon_negative_int_arg(StackVMParser::Non_negative_int_argContext * ctx)
            override
    {
        if (this->intarg < 0) {
            out << "expected a non-negative value in line "
                << ctx->start->getLine()
                << endl;

            errors++;

            intarg = 0;
        }
    }

    virtual void exitChararg(StackVMParser::CharargContext * ctx)
        override
    {
        this->chararg = ctx->getText()[1];
        if (chararg == '\\') {
            switch(ctx->getText()[2]) {
                case 'n': chararg = '\n'; return;
                case 'r': chararg = '\r'; return;
                case 't': chararg = '\t'; return;
                case 'b': chararg = '\b'; return;
                default:
                    out << "Unexpected escape: " << ctx->getText()
                        << endl;
                    errors++;
                    chararg = '\n';
            }
        }
    }

    virtual void exitDoublearg(StackVMParser::DoubleargContext * ctx)
        override
    {
        this->doublearg = stod(ctx->getText());
    }

    virtual void exitStrarg(StackVMParser::StrargContext * ctx)
        override
    {
        this->strarg = ctx->getText().substr(1, ctx->getText().length() - 2);
    }

    virtual void exitCastAsChar(StackVMParser::CastAsCharContext * /*ctx*/)
            override {
        instr = make_shared<CastToInstr<char>>();
    }

    virtual void exitCastAsInt(StackVMParser::CastAsIntContext * /*ctx*/) override {
        instr = make_shared<CastToInstr<int>>();
    }

    virtual void exitCastAsDouble(StackVMParser::CastAsDoubleContext * /*ctx*/) override {
        instr = make_shared<CastToInstr<double>>();
    }

    virtual void exitCastAsStr(StackVMParser::CastAsStrContext * /*ctx*/) override {
        instr = make_shared<CastToInstr<string>>();
    }
};

}
