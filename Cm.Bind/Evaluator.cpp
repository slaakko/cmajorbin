/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Bind/Evaluator.hpp>
#include <Cm.Bind/Exception.hpp>
#include <Cm.Bind/TypeResolver.hpp>
#include <Cm.Bind/Constant.hpp>
#include <Cm.Bind/Enumeration.hpp>
#include <Cm.Sym/ConstantSymbol.hpp>
#include <Cm.Sym/EnumSymbol.hpp>
#include <Cm.Ast/Literal.hpp>
#include <Cm.Ast/Expression.hpp>
#include <functional>

namespace Cm { namespace Bind {

typedef Cm::Sym::Value* (*BinaryOperatorFun)(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span);
typedef Cm::Sym::Value* (*UnaryOperatorFun)(Cm::Sym::Value* subject, const Span& span);

class EvaluationStack
{
public:
    void Push(Cm::Sym::Value* value);
    Cm::Sym::Value* Pop();
private:
    std::stack<std::unique_ptr<Cm::Sym::Value>> stack;
};

void EvaluationStack::Push(Cm::Sym::Value* value)
{ 
    stack.push(std::unique_ptr<Cm::Sym::Value>(value)); 
}

Cm::Sym::Value* EvaluationStack::Pop() 
{ 
    if (stack.empty()) 
    {
        throw std::runtime_error("evaluation stack is empty");
    }
    std::unique_ptr<Cm::Sym::Value> value = std::move(stack.top()); 
    stack.pop(); 
    return value.release(); 
}

Cm::Sym::Value* NotSupported(Cm::Sym::Value* subject, const Span& span)
{
    throw Exception("operation not supported for type " + ValueTypeStr(subject->GetValueType()), span);
}

Cm::Sym::Value* NotSupported(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    throw Exception("operation not supported for types " + ValueTypeStr(left->GetValueType()) + " and " + ValueTypeStr(right->GetValueType()), span);
}

template<typename ValueT, typename Op>
Cm::Sym::Value* UnaryEvaluate(Cm::Sym::Value* subject, Op op)
{
    ValueT* subject_ = static_cast<ValueT*>(subject);
    return new ValueT(op(subject_->Value()));
}

template<typename ValueT>
Cm::Sym::Value* Not(Cm::Sym::Value* subject, const Span& span)
{
    return UnaryEvaluate<ValueT>(subject, std::logical_not<ValueT::OperandType>());
}

UnaryOperatorFun not_[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, Not<Cm::Sym::BoolValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Cm::Sym::Value* UnaryPlus(Cm::Sym::Value* subject, const Span& span)
{
    return UnaryEvaluate<ValueT>(subject, std::identity<ValueT::OperandType>());
}

UnaryOperatorFun unaryPlus[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    UnaryPlus<Cm::Sym::SByteValue>, UnaryPlus<Cm::Sym::ByteValue>, UnaryPlus<Cm::Sym::ShortValue>, UnaryPlus<Cm::Sym::UShortValue>, UnaryPlus<Cm::Sym::IntValue>, UnaryPlus<Cm::Sym::UIntValue>, 
    UnaryPlus<Cm::Sym::LongValue>, UnaryPlus<Cm::Sym::ULongValue>, UnaryPlus<Cm::Sym::FloatValue>, UnaryPlus<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* Negate(Cm::Sym::Value* subject, const Span& span)
{
    return UnaryEvaluate<ValueT>(subject, std::negate<ValueT::OperandType>());
}

UnaryOperatorFun negate[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    Negate<Cm::Sym::SByteValue>, Negate<Cm::Sym::ByteValue>, Negate<Cm::Sym::ShortValue>, Negate<Cm::Sym::UShortValue>, Negate<Cm::Sym::IntValue>, Negate<Cm::Sym::UIntValue>, 
    Negate<Cm::Sym::LongValue>, Negate<Cm::Sym::ULongValue>, Negate<Cm::Sym::FloatValue>, Negate<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* Complement(Cm::Sym::Value* subject, const Span& span)
{
    return UnaryEvaluate<ValueT>(subject, std::bit_not<ValueT::OperandType>());
}

UnaryOperatorFun complement[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    Complement<Cm::Sym::SByteValue>, Complement<Cm::Sym::ByteValue>, Complement<Cm::Sym::ShortValue>, Complement<Cm::Sym::UShortValue>, Complement<Cm::Sym::IntValue>, Complement<Cm::Sym::UIntValue>, 
    Complement<Cm::Sym::LongValue>, Complement<Cm::Sym::ULongValue>, NotSupported, NotSupported
};

void EvaluateUnaryOp(Cm::Sym::ValueType targetType, EvaluationStack& stack, UnaryOperatorFun* fun, const Span& span)
{
    std::unique_ptr<Cm::Sym::Value> subject(stack.Pop());
    Cm::Sym::ValueType subjectType = subject->GetValueType();
    Cm::Sym::ValueType operationType = subjectType;
    if (targetType > subjectType)
    {
        operationType = targetType;
    }
    std::unique_ptr<Cm::Sym::Value> subject_(subject->As(operationType, false, span));
    UnaryOperatorFun operation = fun[uint8_t(operationType)];
    stack.Push(operation(subject_.get(), span));
}

template <typename ValueT, typename Op>
Cm::Sym::Value* BinaryEvaluate(Cm::Sym::Value* left, Cm::Sym::Value* right, Op op)
{
    ValueT* left_ = static_cast<ValueT*>(left);
    ValueT* right_ = static_cast<ValueT*>(right);
    return new ValueT(op(left_->Value(), right_->Value()));
}

template<typename T>
struct shiftLeftFun : std::binary_function<T, T, T>
{
    T operator()(const T& left, const T& right) const
    {
        return left << right;
    }
};

template<typename ValueT>
Cm::Sym::Value* ShiftLeft(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, shiftLeftFun<ValueT::OperandType>());
}

BinaryOperatorFun shiftLeft[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    ShiftLeft<Cm::Sym::SByteValue>, ShiftLeft<Cm::Sym::ByteValue>, ShiftLeft<Cm::Sym::ShortValue>, ShiftLeft<Cm::Sym::UShortValue>, ShiftLeft<Cm::Sym::IntValue>, ShiftLeft<Cm::Sym::UIntValue>, 
    ShiftLeft<Cm::Sym::LongValue>, ShiftLeft<Cm::Sym::ULongValue>, NotSupported, NotSupported
};

template<typename T>
struct shiftRightFun : std::binary_function<T, T, T>
{
    T operator()(const T& left, const T& right) const
    {
        return left >> right;
    }
};

template<typename ValueT>
Cm::Sym::Value* ShiftRight(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, shiftRightFun<ValueT::OperandType>());
}

BinaryOperatorFun shiftRight[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    ShiftRight<Cm::Sym::SByteValue>, ShiftRight<Cm::Sym::ByteValue>, ShiftRight<Cm::Sym::ShortValue>, ShiftRight<Cm::Sym::UShortValue>, ShiftRight<Cm::Sym::IntValue>, ShiftRight<Cm::Sym::UIntValue>, 
    ShiftRight<Cm::Sym::LongValue>, ShiftRight<Cm::Sym::ULongValue>, NotSupported, NotSupported
};

template<typename ValueT>
Cm::Sym::Value* Add(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::plus<ValueT::OperandType>());
}

BinaryOperatorFun add[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    Add<Cm::Sym::SByteValue>, Add<Cm::Sym::ByteValue>, Add<Cm::Sym::ShortValue>, Add<Cm::Sym::UShortValue>, Add<Cm::Sym::IntValue>, Add<Cm::Sym::UIntValue>, 
    Add<Cm::Sym::LongValue>, Add<Cm::Sym::ULongValue>, Add<Cm::Sym::FloatValue>, Add<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* Sub(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::minus<ValueT::OperandType>());
}

BinaryOperatorFun sub[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    Sub<Cm::Sym::SByteValue>, Sub<Cm::Sym::ByteValue>, Sub<Cm::Sym::ShortValue>, Sub<Cm::Sym::UShortValue>, Sub<Cm::Sym::IntValue>, Sub<Cm::Sym::UIntValue>, 
    Sub<Cm::Sym::LongValue>, Sub<Cm::Sym::ULongValue>, Sub<Cm::Sym::FloatValue>, Sub<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* Mul(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::multiplies<ValueT::OperandType>());
}

BinaryOperatorFun mul[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    Mul<Cm::Sym::SByteValue>, Mul<Cm::Sym::ByteValue>, Mul<Cm::Sym::ShortValue>, Mul<Cm::Sym::UShortValue>, Mul<Cm::Sym::IntValue>, Mul<Cm::Sym::UIntValue>, 
    Mul<Cm::Sym::LongValue>, Mul<Cm::Sym::ULongValue>, Mul<Cm::Sym::FloatValue>, Mul<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* Div(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::divides<ValueT::OperandType>());
}

BinaryOperatorFun div[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    Div<Cm::Sym::SByteValue>, Div<Cm::Sym::ByteValue>, Div<Cm::Sym::ShortValue>, Div<Cm::Sym::UShortValue>, Div<Cm::Sym::IntValue>, Div<Cm::Sym::UIntValue>, 
    Div<Cm::Sym::LongValue>, Div<Cm::Sym::ULongValue>, Div<Cm::Sym::FloatValue>, Div<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* Rem(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::modulus<ValueT::OperandType>());
}

BinaryOperatorFun rem[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    Rem<Cm::Sym::SByteValue>, Rem<Cm::Sym::ByteValue>, Rem<Cm::Sym::ShortValue>, Rem<Cm::Sym::UShortValue>, Rem<Cm::Sym::IntValue>, Rem<Cm::Sym::UIntValue>, 
    Rem<Cm::Sym::LongValue>, Rem<Cm::Sym::ULongValue>, NotSupported, NotSupported
};

template<typename ValueT>
Cm::Sym::Value* BitAnd(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_and<ValueT::OperandType>());
}

BinaryOperatorFun bitAnd[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    BitAnd<Cm::Sym::SByteValue>, BitAnd<Cm::Sym::ByteValue>, BitAnd<Cm::Sym::ShortValue>, BitAnd<Cm::Sym::UShortValue>, BitAnd<Cm::Sym::IntValue>, BitAnd<Cm::Sym::UIntValue>, 
    BitAnd<Cm::Sym::LongValue>, BitAnd<Cm::Sym::ULongValue>, NotSupported, NotSupported
};

template<typename ValueT>
Cm::Sym::Value* BitOr(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_or<ValueT::OperandType>());
}

BinaryOperatorFun bitOr[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    BitOr<Cm::Sym::SByteValue>, BitOr<Cm::Sym::ByteValue>, BitOr<Cm::Sym::ShortValue>, BitOr<Cm::Sym::UShortValue>, BitOr<Cm::Sym::IntValue>, BitOr<Cm::Sym::UIntValue>, 
    BitOr<Cm::Sym::LongValue>, BitOr<Cm::Sym::ULongValue>, NotSupported, NotSupported
};

template<typename ValueT>
Cm::Sym::Value* BitXor(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_xor<ValueT::OperandType>());
}

BinaryOperatorFun bitXor[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotSupported, NotSupported, 
    BitXor<Cm::Sym::SByteValue>, BitXor<Cm::Sym::ByteValue>, BitXor<Cm::Sym::ShortValue>, BitXor<Cm::Sym::UShortValue>, BitXor<Cm::Sym::IntValue>, BitXor<Cm::Sym::UIntValue>, 
    BitXor<Cm::Sym::LongValue>, BitXor<Cm::Sym::ULongValue>, NotSupported, NotSupported
};

template<typename ValueT>
Cm::Sym::Value* Disjunction(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::logical_or<ValueT::OperandType>());
}

BinaryOperatorFun disjunction[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, Disjunction<Cm::Sym::BoolValue>, 
    NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Cm::Sym::Value* Conjunction(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryEvaluate<ValueT>(left, right, std::logical_and<ValueT::OperandType>());
}

BinaryOperatorFun conjunction[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, Disjunction<Cm::Sym::BoolValue>, 
    NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

void EvaluateBinOp(Cm::Sym::ValueType targetType, EvaluationStack& stack, BinaryOperatorFun* fun, const Span& span)
{
    std::unique_ptr<Cm::Sym::Value> right(stack.Pop());
    std::unique_ptr<Cm::Sym::Value> left(stack.Pop());
    Cm::Sym::ValueType leftType = left->GetValueType();
    Cm::Sym::ValueType rightType = right->GetValueType();
    Cm::Sym::ValueType commonType = GetCommonType(leftType, rightType);
    Cm::Sym::ValueType operationType = commonType;
    if (targetType > commonType)
    {
        operationType = targetType;
    }
    std::unique_ptr<Cm::Sym::Value> left_(left->As(operationType, false, span));
    std::unique_ptr<Cm::Sym::Value> right_(right->As(operationType, false, span));
    BinaryOperatorFun operation = fun[uint8_t(operationType)];
    stack.Push(operation(left_.get(), right_.get(), span));
}

template <typename ValueT, typename Op>
Cm::Sym::Value* BinaryPredEvaluate(Cm::Sym::Value* left, Cm::Sym::Value* right, Op op)
{
    ValueT* left_ = static_cast<ValueT*>(left);
    ValueT* right_ = static_cast<ValueT*>(right);
    return new Cm::Sym::BoolValue(op(left_->Value(), right_->Value()));
}

template<typename ValueT>
Cm::Sym::Value* Equal(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::equal_to<ValueT::OperandType>());
}

BinaryOperatorFun equal[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, Equal<Cm::Sym::BoolValue>, Equal<Cm::Sym::CharValue>, 
    Equal<Cm::Sym::SByteValue>, Equal<Cm::Sym::ByteValue>, Equal<Cm::Sym::ShortValue>, Equal<Cm::Sym::UShortValue>, Equal<Cm::Sym::IntValue>, Equal<Cm::Sym::UIntValue>, 
    Equal<Cm::Sym::LongValue>, Equal<Cm::Sym::ULongValue>, Equal<Cm::Sym::FloatValue>, Equal<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* NotEqual(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::not_equal_to<ValueT::OperandType>());
}

BinaryOperatorFun notEqual[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, NotEqual<Cm::Sym::BoolValue>, NotEqual<Cm::Sym::CharValue>, 
    NotEqual<Cm::Sym::SByteValue>, NotEqual<Cm::Sym::ByteValue>, NotEqual<Cm::Sym::ShortValue>, NotEqual<Cm::Sym::UShortValue>, NotEqual<Cm::Sym::IntValue>, NotEqual<Cm::Sym::UIntValue>, 
    NotEqual<Cm::Sym::LongValue>, NotEqual<Cm::Sym::ULongValue>, NotEqual<Cm::Sym::FloatValue>, NotEqual<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* Less(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::less<ValueT::OperandType>());
}

BinaryOperatorFun less[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, Less<Cm::Sym::BoolValue>, Less<Cm::Sym::CharValue>, 
    Less<Cm::Sym::SByteValue>, Less<Cm::Sym::ByteValue>, Less<Cm::Sym::ShortValue>, Less<Cm::Sym::UShortValue>, Less<Cm::Sym::IntValue>, Less<Cm::Sym::UIntValue>, 
    Less<Cm::Sym::LongValue>, Less<Cm::Sym::ULongValue>, Less<Cm::Sym::FloatValue>, Less<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* Greater(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::greater<ValueT::OperandType>());
}

BinaryOperatorFun greater[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, Greater<Cm::Sym::BoolValue>, Greater<Cm::Sym::CharValue>, 
    Greater<Cm::Sym::SByteValue>, Greater<Cm::Sym::ByteValue>, Greater<Cm::Sym::ShortValue>, Greater<Cm::Sym::UShortValue>, Greater<Cm::Sym::IntValue>, Greater<Cm::Sym::UIntValue>, 
    Greater<Cm::Sym::LongValue>, Greater<Cm::Sym::ULongValue>, Greater<Cm::Sym::FloatValue>, Greater<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* LessOrEqual(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::less_equal<ValueT::OperandType>());
}

BinaryOperatorFun lessOrEqual[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, LessOrEqual<Cm::Sym::BoolValue>, LessOrEqual<Cm::Sym::CharValue>, 
    LessOrEqual<Cm::Sym::SByteValue>, LessOrEqual<Cm::Sym::ByteValue>, LessOrEqual<Cm::Sym::ShortValue>, LessOrEqual<Cm::Sym::UShortValue>, LessOrEqual<Cm::Sym::IntValue>, LessOrEqual<Cm::Sym::UIntValue>, 
    LessOrEqual<Cm::Sym::LongValue>, LessOrEqual<Cm::Sym::ULongValue>, LessOrEqual<Cm::Sym::FloatValue>, LessOrEqual<Cm::Sym::DoubleValue>
};

template<typename ValueT>
Cm::Sym::Value* GreaterOrEqual(Cm::Sym::Value* left, Cm::Sym::Value* right, const Span& span)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::greater_equal<ValueT::OperandType>());
}

BinaryOperatorFun greaterOrEqual[uint8_t(Cm::Sym::ValueType::max)] =
{
    NotSupported, GreaterOrEqual<Cm::Sym::BoolValue>, GreaterOrEqual<Cm::Sym::CharValue>, 
    GreaterOrEqual<Cm::Sym::SByteValue>, GreaterOrEqual<Cm::Sym::ByteValue>, GreaterOrEqual<Cm::Sym::ShortValue>, GreaterOrEqual<Cm::Sym::UShortValue>, GreaterOrEqual<Cm::Sym::IntValue>, GreaterOrEqual<Cm::Sym::UIntValue>, 
    GreaterOrEqual<Cm::Sym::LongValue>, GreaterOrEqual<Cm::Sym::ULongValue>, GreaterOrEqual<Cm::Sym::FloatValue>, GreaterOrEqual<Cm::Sym::DoubleValue>
};

class Evaluator : public Cm::Ast::Visitor
{
public:
    Evaluator(Cm::Sym::ValueType targetType_, bool cast, Cm::Sym::SymbolTable& symbolTable_, Cm::Sym::ContainerScope* currentContainerScope_, Cm::Sym::FileScope* fileScope_);
    Cm::Sym::Value* DoEvaluate(Cm::Ast::Node* value);
    void Visit(Cm::Ast::BooleanLiteralNode& booleanLiteralNode) override;
    void Visit(Cm::Ast::SByteLiteralNode& sbyteLiteralNode) override;
    void Visit(Cm::Ast::ByteLiteralNode& byteLiteralNode) override;
    void Visit(Cm::Ast::ShortLiteralNode& shortLiteralNode) override;
    void Visit(Cm::Ast::UShortLiteralNode& ushortLiteralNode) override;
    void Visit(Cm::Ast::IntLiteralNode& intLiteralNode) override;
    void Visit(Cm::Ast::UIntLiteralNode& uintLiteralNode) override;
    void Visit(Cm::Ast::LongLiteralNode& longLiteralNode) override;
    void Visit(Cm::Ast::ULongLiteralNode& ulongLiteralNode) override;
    void Visit(Cm::Ast::FloatLiteralNode& floatLitealNode) override;
    void Visit(Cm::Ast::DoubleLiteralNode& doubleLiteralNode) override;
    void Visit(Cm::Ast::CharLiteralNode& charLiteralNode) override;
    void Visit(Cm::Ast::StringLiteralNode& stringLiteralNode) override;
    void Visit(Cm::Ast::NullLiteralNode& nullLiteralNode) override;

    void BeginVisit(Cm::Ast::EquivalenceNode& equivalenceNode) override;
    void BeginVisit(Cm::Ast::ImplicationNode& implicationNode) override;
    void EndVisit(Cm::Ast::DisjunctionNode& disjunctionNode) override;
    void EndVisit(Cm::Ast::ConjunctionNode& conjunctionNode) override;
    void EndVisit(Cm::Ast::BitOrNode& bitOrNode) override;
    void EndVisit(Cm::Ast::BitXorNode& bitXorNode) override;
    void EndVisit(Cm::Ast::BitAndNode& bitAndNode) override;
    void EndVisit(Cm::Ast::EqualNode& equalNode) override;
    void EndVisit(Cm::Ast::NotEqualNode& notEqualNode) override;
    void EndVisit(Cm::Ast::LessNode& lessNode) override;
    void EndVisit(Cm::Ast::GreaterNode& greaterNode) override;
    void EndVisit(Cm::Ast::LessOrEqualNode& lessOrEqualNode) override;
    void EndVisit(Cm::Ast::GreaterOrEqualNode& greaterOrEqualNode) override;
    void EndVisit(Cm::Ast::ShiftLeftNode& shiftLeftNode) override;
    void EndVisit(Cm::Ast::ShiftRightNode& shiftRightNode) override;
    void EndVisit(Cm::Ast::AddNode& addNode) override;
    void EndVisit(Cm::Ast::SubNode& subNode) override;
    void EndVisit(Cm::Ast::MulNode& mulNode) override;
    void EndVisit(Cm::Ast::DivNode& divNode) override;
    void EndVisit(Cm::Ast::RemNode& remNode) override;
    void BeginVisit(Cm::Ast::PrefixIncNode& prefixIncNode) override;
    void BeginVisit(Cm::Ast::PrefixDecNode& prefixDecNode) override;
    void EndVisit(Cm::Ast::UnaryPlusNode& unaryPlusNode) override;
    void EndVisit(Cm::Ast::UnaryMinusNode& unaryMinusNode) override;
    void EndVisit(Cm::Ast::NotNode& notNode) override;
    void EndVisit(Cm::Ast::ComplementNode& complementNode) override;
    void Visit(Cm::Ast::AddrOfNode& addrOfNode) override;
    void Visit(Cm::Ast::DerefNode& derefNode) override;
    void Visit(Cm::Ast::PostfixIncNode& postfixIncNode) override;
    void Visit(Cm::Ast::PostfixDecNode& postfixDecNode) override;
    void Visit(Cm::Ast::DotNode& dotNode) override;
    void Visit(Cm::Ast::ArrowNode& arrowNode) override;
    void BeginVisit(Cm::Ast::InvokeNode& invokeNode) override;
    void Visit(Cm::Ast::IndexNode& indexNode) override;

    void Visit(Cm::Ast::SizeOfNode& sizeOfNode) override;
    void Visit(Cm::Ast::CastNode& castNode) override;
    void BeginVisit(Cm::Ast::ConstructNode& constructNode) override;
    void BeginVisit(Cm::Ast::NewNode& newNode) override;
    void Visit(Cm::Ast::TemplateIdNode& templateIdNode) override;
    void Visit(Cm::Ast::IdentifierNode& identifierNode) override;
    void Visit(Cm::Ast::ThisNode& thisNode) override;
    void Visit(Cm::Ast::BaseNode& baseNode) override;
    void Visit(Cm::Ast::TypeNameNode& typeNameNode) override;
private:
    Cm::Sym::ValueType targetType;
    bool cast;
    Cm::Sym::SymbolTable& symbolTable;
    Cm::Sym::ContainerScope* currentContainerScope;
    Cm::Sym::FileScope* fileScope;
    EvaluationStack stack;
};

Evaluator::Evaluator(Cm::Sym::ValueType targetType_, bool cast_, Cm::Sym::SymbolTable& symbolTable_, Cm::Sym::ContainerScope* currentContainerScope_, Cm::Sym::FileScope* fileScope_) :
    Visitor(true), targetType(targetType_), cast(cast_), symbolTable(symbolTable_), currentContainerScope(currentContainerScope_), fileScope(fileScope_)
{
}

Cm::Sym::Value* Evaluator::DoEvaluate(Cm::Ast::Node* value)
{
    value->Accept(*this);
    std::unique_ptr<Cm::Sym::Value> result(stack.Pop());
    result.reset(result->As(targetType, cast, value->GetSpan()));
    return result.release();
}

void Evaluator::Visit(Cm::Ast::BooleanLiteralNode& booleanLiteralNode)
{
    stack.Push(new Cm::Sym::BoolValue(booleanLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::SByteLiteralNode& sbyteLiteralNode)
{
    stack.Push(new Cm::Sym::SByteValue(sbyteLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::ByteLiteralNode& byteLiteralNode)
{
    stack.Push(new Cm::Sym::ByteValue(byteLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::ShortLiteralNode& shortLiteralNode)
{
    stack.Push(new Cm::Sym::ShortValue(shortLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::UShortLiteralNode& ushortLiteralNode)
{
    stack.Push(new Cm::Sym::UShortValue(ushortLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::IntLiteralNode& intLiteralNode)
{
    stack.Push(new Cm::Sym::IntValue(intLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::UIntLiteralNode& uintLiteralNode)
{
    stack.Push(new Cm::Sym::UIntValue(uintLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::LongLiteralNode& longLiteralNode)
{
    stack.Push(new Cm::Sym::LongValue(longLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::ULongLiteralNode& ulongLiteralNode)
{
    stack.Push(new Cm::Sym::ULongValue(ulongLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::FloatLiteralNode& floatLiteralNode)
{
    stack.Push(new Cm::Sym::FloatValue(floatLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::DoubleLiteralNode& doubleLiteralNode)
{
    stack.Push(new Cm::Sym::DoubleValue(doubleLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::CharLiteralNode& charLiteralNode)
{
    stack.Push(new Cm::Sym::CharValue(charLiteralNode.Value()));
}

void Evaluator::Visit(Cm::Ast::StringLiteralNode& stringLiteralNode)
{
    throw Exception("cannot evaluate statically", stringLiteralNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::NullLiteralNode& nullLiteralNode)
{
    throw Exception("cannot evaluate statically", nullLiteralNode.GetSpan());
}

void Evaluator::BeginVisit(Cm::Ast::EquivalenceNode& equivalenceNode)
{
    throw Exception("operation not supported", equivalenceNode.GetSpan());
}

void Evaluator::BeginVisit(Cm::Ast::ImplicationNode& implicationNode)
{
    throw Exception("operation not supported", implicationNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::DisjunctionNode& disjunctionNode)
{
    EvaluateBinOp(targetType, stack, disjunction, disjunctionNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::ConjunctionNode& conjunctionNode)
{
    EvaluateBinOp(targetType, stack, conjunction, conjunctionNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::BitOrNode& bitOrNode)
{
    EvaluateBinOp(targetType, stack, bitOr, bitOrNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::BitXorNode& bitXorNode)
{
    EvaluateBinOp(targetType, stack, bitXor, bitXorNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::BitAndNode& bitAndNode)
{
    EvaluateBinOp(targetType, stack, bitAnd, bitAndNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::EqualNode& equalNode)
{
    EvaluateBinOp(targetType, stack, equal, equalNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::NotEqualNode& notEqualNode)
{
    EvaluateBinOp(targetType, stack, notEqual, notEqualNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::LessNode& lessNode)
{
    EvaluateBinOp(targetType, stack, less, lessNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::GreaterNode& greaterNode)
{
    EvaluateBinOp(targetType, stack, greater, greaterNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::LessOrEqualNode& lessOrEqualNode)
{
    EvaluateBinOp(targetType, stack, lessOrEqual, lessOrEqualNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::GreaterOrEqualNode& greaterOrEqualNode)
{
    EvaluateBinOp(targetType, stack, greaterOrEqual, greaterOrEqualNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::ShiftLeftNode& shiftLeftNode)
{
    EvaluateBinOp(targetType, stack, shiftLeft, shiftLeftNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::ShiftRightNode& shiftRightNode)
{
    EvaluateBinOp(targetType, stack, shiftRight, shiftRightNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::AddNode& addNode)
{
    EvaluateBinOp(targetType, stack, add, addNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::SubNode& subNode)
{
    EvaluateBinOp(targetType, stack, sub, subNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::MulNode& mulNode)
{
    EvaluateBinOp(targetType, stack, mul, mulNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::DivNode& divNode)
{
    EvaluateBinOp(targetType, stack, div, divNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::RemNode& remNode)
{
    EvaluateBinOp(targetType, stack, rem, remNode.GetSpan());
}

void Evaluator::BeginVisit(Cm::Ast::PrefixIncNode& prefixIncNode)
{
    throw Exception("cannot evaluate statically", prefixIncNode.GetSpan());
}

void Evaluator::BeginVisit(Cm::Ast::PrefixDecNode& prefixDecNode)
{
    throw Exception("cannot evaluate statically", prefixDecNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::UnaryPlusNode& unaryPlusNode)
{
    EvaluateUnaryOp(targetType, stack, unaryPlus, unaryPlusNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::UnaryMinusNode& unaryMinusNode)
{
    EvaluateUnaryOp(targetType, stack, negate, unaryMinusNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::NotNode& notNode)
{
    EvaluateUnaryOp(targetType, stack, not_, notNode.GetSpan());
}

void Evaluator::EndVisit(Cm::Ast::ComplementNode& complementNode)
{
    EvaluateUnaryOp(targetType, stack, complement, complementNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::AddrOfNode& addrOfNode)
{
    throw Exception("cannot evaluate statically", addrOfNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::DerefNode& derefNode)
{
    throw Exception("cannot evaluate statically", derefNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::PostfixIncNode& postfixIncNode)
{
    throw Exception("cannot evaluate statically", postfixIncNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::PostfixDecNode& postfiDecNode)
{
    throw Exception("cannot evaluate statically", postfiDecNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::DotNode& dotNode)
{
    throw Exception("cannot evaluate statically", dotNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::ArrowNode& arrowNode)
{
    throw Exception("cannot evaluate statically", arrowNode.GetSpan());
}

void Evaluator::BeginVisit(Cm::Ast::InvokeNode& invokeNode)
{
    throw Exception("cannot evaluate statically", invokeNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::IndexNode& indexNode)
{
    throw Exception("cannot evaluate statically", indexNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::SizeOfNode& sizeOfNode)
{
    throw Exception("not implemented yet", sizeOfNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::CastNode& castNode)
{
    Cm::Ast::Node* targetTypeExpr = castNode.TargetTypeExpr();
    std::unique_ptr<Cm::Sym::TypeSymbol> type(ResolveType(symbolTable, currentContainerScope, fileScope, targetTypeExpr));
    Cm::Sym::SymbolType symbolType = type->GetSymbolType();
    Cm::Sym::ValueType valueType = GetValueTypeFor(symbolType);
    stack.Push(Evaluate(valueType, true, castNode.SourceExpr(), symbolTable, currentContainerScope, fileScope));
}

void Evaluator::BeginVisit(Cm::Ast::ConstructNode& constructNode)
{
    throw Exception("cannot evaluate statically", constructNode.GetSpan());
}

void Evaluator::BeginVisit(Cm::Ast::NewNode& newNode)
{
    throw Exception("cannot evaluate statically", newNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::TemplateIdNode& templateIdNode)
{
    throw Exception("cannot evaluate statically", templateIdNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::IdentifierNode& identifierNode)
{
    Cm::Sym::Symbol* symbol = currentContainerScope->Lookup(identifierNode.Str(), Cm::Sym::ScopeLookup::this_and_base_and_parent);
    if (!symbol)
    {
        symbol = fileScope->Lookup(identifierNode.Str());
    }
    if (symbol)
    {
        if (symbol->IsConstantSymbol())
        {
            Cm::Sym::ConstantSymbol* constantSymbol = static_cast<Cm::Sym::ConstantSymbol*>(symbol);
            if (!constantSymbol->GetValue())
            {
                Cm::Ast::Node* node = symbolTable.GetNode(constantSymbol);
                if (node->IsConstantNode())
                {
                    Cm::Ast::ConstantNode* constantNode = static_cast<Cm::Ast::ConstantNode*>(node);
                    BindConstant(symbolTable, currentContainerScope, fileScope, constantNode);
                }
                else
                {
                    throw std::runtime_error("node is not constant node");
                }
            }
            stack.Push(constantSymbol->GetValue()->Clone());
        }
        else if (symbol->IsEnumConstantSymbol())
        {
            Cm::Sym::EnumConstantSymbol* enumConstantSymbol = static_cast<Cm::Sym::EnumConstantSymbol*>(symbol);
            if (!enumConstantSymbol->GetValue())
            {
                Cm::Ast::Node* node = symbolTable.GetNode(enumConstantSymbol);
                if (node->IsEnumConstantNode())
                {
                    Cm::Ast::EnumConstantNode* enumConstantNode = static_cast<Cm::Ast::EnumConstantNode*>(node);
                    Cm::Sym::ContainerScope* enumConstantContainerScope = symbolTable.GetContainerScope(enumConstantNode);
                    BindEnumConstant(symbolTable, enumConstantContainerScope, fileScope, enumConstantNode);
                }
                else
                {
                    throw std::runtime_error("node is not enumeration constant node");
                }
            }
            stack.Push(enumConstantSymbol->GetValue()->Clone());
        }
        else
        {
            throw Exception("cannot evaluate statically", identifierNode.GetSpan());
        }
    }
    else
    {
        throw Exception("symbol '" + identifierNode.Str() + "' not found", identifierNode.GetSpan());
    }
}

void Evaluator::Visit(Cm::Ast::ThisNode& thisNode)
{
    throw Exception("cannot evaluate statically", thisNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::BaseNode& baseNode)
{
    throw Exception("cannot evaluate statically", baseNode.GetSpan());
}

void Evaluator::Visit(Cm::Ast::TypeNameNode& typeNameNode)
{
    throw Exception("cannot evaluate statically", typeNameNode.GetSpan());
}

Cm::Sym::Value* Evaluate(Cm::Sym::ValueType targetType, bool cast, Cm::Ast::Node* value, Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ContainerScope* currentContainerScope, Cm::Sym::FileScope* fileScope)
{
    Evaluator evaluator(targetType, cast, symbolTable, currentContainerScope, fileScope);
    return evaluator.DoEvaluate(value);
}

} } // namespace Cm::Bind