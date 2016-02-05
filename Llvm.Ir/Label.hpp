/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/
 
    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

 ========================================================================*/

#ifndef LLVM_IR_LABEL_INCLUDED
#define LLVM_IR_LABEL_INCLUDED
#include <Ir.Intf/Label.hpp>

namespace Llvm { 

class LabelObject : public Ir::Intf::LabelObject
{
public:
    LabelObject();
    LabelObject(const std::string& label_);
    std::string Target() const override { return GetLabelName() + ":"; }
    void Set(Ir::Intf::LabelObject* from) override;
    void InitTo(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Object* to) override;
    void InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Constant& constant) override;
    void InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Global& global) override;
    void InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::RegVar& regVar) override;
    void InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::StackVar& stackVar) override;
    void InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::MemberVar& memberVar) override;
    void InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::RefVar& refVar) override;
    void InitFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Parameter& parameter) override;
    void AssignTo(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Object* to) override;
    void AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Constant& constant) override;
    void AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Global& global) override;
    void AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::RegVar& regVar) override;
    void AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::StackVar& stackVar) override;
    void AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::MemberVar& memberVar) override;
    void AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::RefVar& refVar) override;
    void AssignFrom(Ir::Intf::Emitter& emitter, Ir::Intf::Type* type, Ir::Intf::Parameter& parameter) override;
};

Ir::Intf::LabelObject* CreateLabel();
Ir::Intf::LabelObject* CreateLabel(const std::string& label);
Ir::Intf::LabelObject* CreateNextLocalLabel();
void ResetLocalLabelCounter();

void SetBreaker(int x);
void PushString(const std::string& s);
void PopString();

class Pusher
{
public:
    Pusher(const std::string& s)
    {
        PushString(s);
    }
    ~Pusher()
    {
        PopString();
    }
};

} // namespace Llvm

#endif // LLVM_IR_LABEL_INCLUDED
