/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_SYM_TYPE_SYMBOL_INCLUDED
#define CM_SYM_TYPE_SYMBOL_INCLUDED
#include <Cm.Sym/ContainerSymbol.hpp>
#include <Cm.Sym/TypeId.hpp>
#include <Ir.Intf/Type.hpp>
#include <Ir.Intf/Object.hpp>

namespace Cm { namespace Sym {

struct DerivationCounts
{
    DerivationCounts() : consts(0), refs(0), rvalueRefs(0), pointers(0) {}
    int consts;
    int refs;
    int rvalueRefs;
    int pointers;
};

bool operator<(const DerivationCounts& left, const DerivationCounts& right);

class TypeSymbol : public ContainerSymbol
{
public:
    TypeSymbol(const Span& span_, const std::string& name_);
    TypeSymbol(const Span& span_, const std::string& name_, const TypeId& id_);
    std::string TypeString() const override { return "type"; };
    bool IsTypeSymbol() const override { return true; }
    void Write(Writer& writer) override;
    void Read(Reader& reader) override;
    const TypeId& Id() const { return id; }
    void SetId(const TypeId& id_) { id = id_;  }
    virtual std::string GetMangleId() const = 0;
    virtual bool IsAbstract() const { return false; }
    virtual bool IsDerivedTypeSymbol() const { return false; }
    virtual TypeSymbol* GetBaseType() const { return const_cast<TypeSymbol*>(this); }
    virtual bool IsSignedType() const { return false; }
    virtual bool IsPointerType() const { return false; }
    virtual bool IsFunctionType() const { return false; }
    virtual int GetPointerCount() const { return 0; }
    virtual bool IsPointerToClassType() const { return false; }
    virtual bool IsPointerToTemplateType() const { return false; }
    virtual bool IsReferenceType() const { return false; }
    virtual bool IsNonConstReferenceType() const { return false; }
    virtual bool IsConstType() const { return false; }
    virtual bool IsRvalueRefType() const { return false; }
    virtual bool IsConstReferenceType() const { return false; }
    virtual bool IsVoidPtrType() const { return false; }
    virtual bool IsNullPtrType() const { return false; }
    virtual bool IsConstPointerType() const { return false; }
    virtual bool IsConstPointerPointerType() const { return false; }
    virtual DerivationCounts GetDerivationCounts() const { return DerivationCounts(); }
    void SetIrType(Ir::Intf::Type* irType_);
    Ir::Intf::Type* GetIrType() const;
    void SetDefaultIrValue(Ir::Intf::Object* defaultIrValue_);
    Ir::Intf::Object* GetDefaultIrValue() const;
private:
    TypeId id;
    std::unique_ptr<Ir::Intf::Type> irType;
    std::unique_ptr<Ir::Intf::Object> defaultIrValue;
};

inline bool TypesEqual(TypeSymbol* left, TypeSymbol* right)
{ 
    return left->Id() == right->Id();
}

} } // namespace Cm::Sym

#endif // CM_SYM_TYPE_SYMBOL_INCLUDED
