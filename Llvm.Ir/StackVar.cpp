/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#include <Llvm.Ir/StackVar.hpp>
#include <Llvm.Ir/Function.hpp>
#include <Llvm.Ir/Constant.hpp>
#include <Llvm.Ir/Global.hpp>
#include <Llvm.Ir/RegVar.hpp>
#include <Llvm.Ir/RefVar.hpp>
#include <Llvm.Ir/MemberVar.hpp>
#include <Llvm.Ir/Instruction.hpp>

namespace Llvm { 

StackVar::StackVar(const std::string& name_, Ir::Intf::Type* pointerType_): Ir::Intf::StackVar("%" + name_, pointerType_)
{
}

void StackVar::InitTo(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Object* to)
{
    to->InitFrom(emitter, type, *this);
}

void StackVar::InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Constant& constant)
{
    emitter.Emit(Store(type, &constant, this));
}

void StackVar::InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Global& global)
{
    if (global.GetType()->IsFunctionPtrPtrType())
    {
        emitter.Emit(Store(type, &global, this));
    }
    else
    {
        Ir::Intf::RegVar* reg = CreateTemporaryRegVar(type);
        emitter.Own(reg);
        emitter.Emit(Load(type, reg, &global));
        emitter.Emit(Store(type, reg, this));
    }
}

void StackVar::InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::RegVar& regVar)
{
    PointerType* ptrType = MakePointerType(type);
    emitter.Own(ptrType);
    if (TypesEqual(regVar.GetType(), ptrType))
    {
        Ir::Intf::RegVar* reg = CreateTemporaryRegVar(type);
        emitter.Own(reg);
        emitter.Emit(Load(ptrType, reg, &regVar));
        emitter.Emit(Store(type, reg, this));
    }
    else
    {
        emitter.Emit(Store(type, &regVar, this));
    }
}

void StackVar::InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::StackVar& stackVar)
{
    PointerType* ptrType = MakePointerType(stackVar.GetType());
    emitter.Own(ptrType);
    if (TypesEqual(GetType(), ptrType))
    {
        emitter.Emit(Store(type, &stackVar, this));
    }
    else
    {
        PointerType* ptrType = MakePointerType(type);
        emitter.Own(ptrType);
        Ir::Intf::RegVar* reg = CreateTemporaryRegVar(type);
        emitter.Own(reg);
        emitter.Emit(Load(ptrType, reg, &stackVar));
        emitter.Emit(Store(type, reg, this));
    }
}

void StackVar::InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::MemberVar& memberVar)
{
    PointerType* ptrType = MakePointerType(type);
    emitter.Own(ptrType);
    Ir::Intf::RegVar* ptr = CreateTemporaryRegVar(ptrType);
    emitter.Own(ptr);
    Ir::Intf::Object* zero = CreateI32Constant(0);
    emitter.Own(zero);
    Ir::Intf::Object* index = CreateI32Constant(memberVar.Index());
    emitter.Own(index);
    emitter.Emit(GetElementPtr(memberVar.Ptr()->GetType(), ptr, memberVar.Ptr(), zero, index));
    Ir::Intf::RegVar* value = CreateTemporaryRegVar(type);
    emitter.Own(value);
    emitter.Emit(Load(ptrType, value, ptr));
    emitter.Emit(Store(type, value, this));
}

void StackVar::InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::RefVar& refVar)
{
    PointerType* ptrType = MakePointerType(type);
    emitter.Own(ptrType);
    Ir::Intf::RegVar* refReg = CreateTemporaryRegVar(ptrType);
    emitter.Own(refReg);
    emitter.Emit(Load(ptrType, refReg, &refVar));
    Ir::Intf::RegVar* reg = CreateTemporaryRegVar(type);
    emitter.Own(reg);
    emitter.Emit(Load(type, reg, refReg));
    emitter.Emit(Store(type, reg, this));
}

void StackVar::InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Parameter& parameter)
{
    emitter.Emit(Store(type, &parameter, this));
}

void StackVar::AssignTo(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Object* to)
{
    to->AssignFrom(emitter, type, *this);
}

void StackVar::AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Constant& constant)
{
    emitter.Emit(Store(type, &constant, this));
}

void StackVar::AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Global& global)
{
    if (global.GetType()->IsFunctionPtrPtrType())
    {
        emitter.Emit(Store(type, &global, this));
    }
    else
    {
        PointerType* ptrType = MakePointerType(type);
        emitter.Own(ptrType);
        Ir::Intf::RegVar* reg = CreateTemporaryRegVar(type);
        emitter.Own(reg);
        emitter.Emit(Load(ptrType, reg, &global));
        emitter.Emit(Store(type, reg, this));
    }
}

void StackVar::AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::RegVar& regVar)
{
    PointerType* ptrType = MakePointerType(type);
    emitter.Own(ptrType);
    if (TypesEqual(regVar.GetType(), ptrType))
    {
        Ir::Intf::RegVar* reg = CreateTemporaryRegVar(type);
        emitter.Own(reg);
        emitter.Emit(Load(ptrType, reg, &regVar));
        emitter.Emit(Store(type, reg, this));
    }
    else
    {
        emitter.Emit(Store(type, &regVar, this));
    }
}

void StackVar::AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::StackVar& stackVar)
{
    PointerType* ptrType = MakePointerType(stackVar.GetType());
    emitter.Own(ptrType);
    if (TypesEqual(GetType(), ptrType))
    {
        emitter.Emit(Store(type, &stackVar, this));
    }
    else
    {
        PointerType* ptrType = MakePointerType(type);
        emitter.Own(ptrType);
        Ir::Intf::RegVar* reg = CreateTemporaryRegVar(type);
        emitter.Own(reg);
        emitter.Emit(Load(ptrType, reg, &stackVar));
        emitter.Emit(Store(type, reg, this));
    }
}

void StackVar::AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::MemberVar& memberVar)
{
    PointerType* ptrType = MakePointerType(type);
    emitter.Own(ptrType);
    Ir::Intf::RegVar* ptr = CreateTemporaryRegVar(ptrType);
    emitter.Own(ptr);
    Ir::Intf::Object* zero = CreateI32Constant(0);
    emitter.Own(zero);
    Ir::Intf::Object* index = CreateI32Constant(memberVar.Index());
    emitter.Own(index);
    emitter.Emit(GetElementPtr(memberVar.Ptr()->GetType(), ptr, memberVar.Ptr(), zero, index));
    Ir::Intf::RegVar* value = CreateTemporaryRegVar(type);
    emitter.Own(value);
    emitter.Emit(Load(type, value, ptr));
    emitter.Emit(Store(type, value, this));
}

void StackVar::AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::RefVar& refVar)
{
    PointerType* ptrType = MakePointerType(type);
    emitter.Own(ptrType);
    Ir::Intf::RegVar* refReg = CreateTemporaryRegVar(ptrType);
    emitter.Own(refReg);
    emitter.Emit(Load(ptrType, refReg, &refVar));
    Ir::Intf::RegVar* reg = CreateTemporaryRegVar(type);
    emitter.Own(reg);
    emitter.Emit(Load(type, reg, refReg));
    emitter.Emit(Store(type, reg, this));
}

void StackVar::AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Parameter& parameter)
{
    emitter.Emit(Store(type, &parameter, this));
}

Ir::Intf::Object* StackVar::CreateAddr(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type)
{
    return this;
}

} // namespace Llvm
