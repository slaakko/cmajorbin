/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Core/InitSymbolTable.hpp>
#include <Cm.Core/BasicTypeOp.hpp>
#include <Cm.Sym/BasicTypeSymbol.hpp>

namespace Cm { namespace Core {

void MakeBoolOps(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::BasicTypeSymbol* boolType)
{
    symbolTable.AddPredefinedSymbolToGlobalScope(new DefaultCtor(symbolTable.GetTypeRepository(), boolType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new CopyCtor(symbolTable.GetTypeRepository(), boolType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new CopyAssignment(symbolTable.GetTypeRepository(), boolType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new MoveCtor(symbolTable.GetTypeRepository(), boolType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new MoveAssignment(symbolTable.GetTypeRepository(), boolType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpEqual(symbolTable.GetTypeRepository(), boolType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpLess(symbolTable.GetTypeRepository(), boolType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpNot(symbolTable.GetTypeRepository(), boolType));
}

void MakeIntegerOps(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::BasicTypeSymbol* integerType)
{
    symbolTable.AddPredefinedSymbolToGlobalScope(new DefaultCtor(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new CopyCtor(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new CopyAssignment(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new MoveCtor(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new MoveAssignment(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpEqual(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpLess(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpAdd(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpSub(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpMul(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpDiv(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpRem(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpShl(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpShr(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpBitAnd(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpBitOr(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpBitXor(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpUnaryPlus(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpUnaryMinus(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpComplement(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpIncrement(symbolTable.GetTypeRepository(), integerType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpDecrement(symbolTable.GetTypeRepository(), integerType));
}

void MakeFloatingPointOps(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::BasicTypeSymbol* floatingPointType)
{
    symbolTable.AddPredefinedSymbolToGlobalScope(new DefaultCtor(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new CopyCtor(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new CopyAssignment(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new MoveCtor(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new MoveAssignment(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpEqual(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpLess(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpAdd(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpSub(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpMul(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpDiv(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpUnaryPlus(symbolTable.GetTypeRepository(), floatingPointType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpUnaryMinus(symbolTable.GetTypeRepository(), floatingPointType));
}

void MakeCharOps(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::BasicTypeSymbol* charType)
{
    symbolTable.AddPredefinedSymbolToGlobalScope(new DefaultCtor(symbolTable.GetTypeRepository(), charType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new CopyCtor(symbolTable.GetTypeRepository(), charType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new CopyAssignment(symbolTable.GetTypeRepository(), charType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new MoveCtor(symbolTable.GetTypeRepository(), charType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new MoveAssignment(symbolTable.GetTypeRepository(), charType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpEqual(symbolTable.GetTypeRepository(), charType));
    symbolTable.AddPredefinedSymbolToGlobalScope(new OpLess(symbolTable.GetTypeRepository(), charType));
}

void MakeStandardConversions(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::SByteTypeSymbol* sbyteType, Cm::Sym::ByteTypeSymbol* byteType, 
    Cm::Sym::ShortTypeSymbol* shortType, Cm::Sym::UShortTypeSymbol* ushortType, Cm::Sym::IntTypeSymbol* intType, Cm::Sym::UIntTypeSymbol* uintType, 
    Cm::Sym::LongTypeSymbol* longType, Cm::Sym::ULongTypeSymbol* ulongType, Cm::Sym::FloatTypeSymbol* floatType, Cm::Sym::DoubleTypeSymbol* doubleType, 
    Cm::Sym::CharTypeSymbol* charType, Cm::Sym::WCharTypeSymbol* wcharType, Cm::Sym::UCharTypeSymbol* ucharType, Cm::Sym::BoolTypeSymbol* boolType)
{
    std::vector<ConvertingCtor*> conversions;
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, byteType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, ushortType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptosi, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptosi, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), sbyteType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, sbyteType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, ushortType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), byteType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, sbyteType, Cm::Sym::ConversionType::implicit, ConversionInst::sext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, byteType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 2));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, ushortType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptosi, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptosi, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), shortType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, sbyteType, Cm::Sym::ConversionType::explicit_, ConversionInst::sext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, byteType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ushortType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, sbyteType, Cm::Sym::ConversionType::implicit, ConversionInst::sext, Cm::Sym::ConversionRank::promotion, 3));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, byteType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 4));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, shortType, Cm::Sym::ConversionType::implicit, ConversionInst::sext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, ushortType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 2));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptosi, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptosi, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), intType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, sbyteType, Cm::Sym::ConversionType::explicit_, ConversionInst::sext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, byteType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 2));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::sext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, ushortType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), uintType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, sbyteType, Cm::Sym::ConversionType::implicit, ConversionInst::sext, Cm::Sym::ConversionRank::promotion, 5));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, byteType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 6));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, shortType, Cm::Sym::ConversionType::implicit, ConversionInst::sext, Cm::Sym::ConversionRank::promotion, 3));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, ushortType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 4));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, intType, Cm::Sym::ConversionType::implicit, ConversionInst::sext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, uintType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 2));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptosi, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptosi, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), longType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, sbyteType, Cm::Sym::ConversionType::explicit_, ConversionInst::sext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, byteType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 3));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::sext, Cm::Sym::ConversionRank::conversion, 100 ));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, ushortType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 2));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::sext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, uintType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ulongType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, sbyteType, Cm::Sym::ConversionType::implicit, ConversionInst::sitofp, Cm::Sym::ConversionRank::conversion, 5));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, byteType, Cm::Sym::ConversionType::implicit, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 6));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, shortType, Cm::Sym::ConversionType::implicit, ConversionInst::sitofp, Cm::Sym::ConversionRank::conversion, 3));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, ushortType, Cm::Sym::ConversionType::implicit, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 4));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, intType, Cm::Sym::ConversionType::implicit, ConversionInst::sitofp, Cm::Sym::ConversionRank::conversion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, uintType, Cm::Sym::ConversionType::implicit, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 2));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::sitofp, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptrunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), floatType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, sbyteType, Cm::Sym::ConversionType::implicit, ConversionInst::sitofp, Cm::Sym::ConversionRank::conversion, 8));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, byteType, Cm::Sym::ConversionType::implicit, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 9));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, shortType, Cm::Sym::ConversionType::implicit, ConversionInst::sitofp, Cm::Sym::ConversionRank::conversion, 6));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, ushortType, Cm::Sym::ConversionType::implicit, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 7));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, intType, Cm::Sym::ConversionType::implicit, ConversionInst::sitofp, Cm::Sym::ConversionRank::conversion, 4));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, uintType, Cm::Sym::ConversionType::implicit, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 5));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, longType, Cm::Sym::ConversionType::implicit, ConversionInst::sitofp, Cm::Sym::ConversionRank::conversion, 2));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, ulongType, Cm::Sym::ConversionType::implicit, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 3));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, floatType, Cm::Sym::ConversionType::implicit, ConversionInst::fpext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), doubleType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::uitofp, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, sbyteType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, byteType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, ushortType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), charType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, charType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, sbyteType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, byteType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, ushortType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), wcharType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, charType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 2));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, wcharType, Cm::Sym::ConversionType::implicit, ConversionInst::zext, Cm::Sym::ConversionRank::promotion, 1));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, sbyteType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, byteType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, ushortType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::bitcast, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), ucharType, boolType, Cm::Sym::ConversionType::explicit_, ConversionInst::zext, Cm::Sym::ConversionRank::conversion, 100));

    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, sbyteType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, byteType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, shortType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, ushortType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, intType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, uintType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, longType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, ulongType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, floatType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, doubleType, Cm::Sym::ConversionType::explicit_, ConversionInst::fptoui, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, charType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, wcharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));
    conversions.push_back(new ConvertingCtor(symbolTable.GetTypeRepository(), boolType, ucharType, Cm::Sym::ConversionType::explicit_, ConversionInst::trunc, Cm::Sym::ConversionRank::conversion, 100));


    for (ConvertingCtor* conversion : conversions)
    {
        symbolTable.AddPredefinedSymbolToGlobalScope(conversion);
        if (conversion->GetConversionType() == Cm::Sym::ConversionType::implicit)
        {
            symbolTable.GetStandardConversionTable().AddConversion(conversion);
        }
    }
}

void MakeBasicTypes(Cm::Sym::SymbolTable& symbolTable)
{
    std::vector<Cm::Sym::BasicTypeSymbol*> integerTypes;
    Cm::Sym::BoolTypeSymbol* boolTypeSymbol = new Cm::Sym::BoolTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(boolTypeSymbol);
    Cm::Sym::CharTypeSymbol* charTypeSymbol = new Cm::Sym::CharTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(charTypeSymbol);
    Cm::Sym::WCharTypeSymbol* wcharTypeSymbol = new Cm::Sym::WCharTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(wcharTypeSymbol);
    Cm::Sym::UCharTypeSymbol* ucharTypeSymbol = new Cm::Sym::UCharTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(ucharTypeSymbol);
    Cm::Sym::VoidTypeSymbol* voidTypeSymbol = new Cm::Sym::VoidTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(voidTypeSymbol);
    Cm::Sym::SByteTypeSymbol* sbyteTypeSymbol = new Cm::Sym::SByteTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(sbyteTypeSymbol);
    integerTypes.push_back(sbyteTypeSymbol);
    Cm::Sym::ByteTypeSymbol* byteTypeSymbol = new Cm::Sym::ByteTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(byteTypeSymbol);
    integerTypes.push_back(byteTypeSymbol);
    Cm::Sym::ShortTypeSymbol* shortTypeSymbol = new Cm::Sym::ShortTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(shortTypeSymbol);
    integerTypes.push_back(shortTypeSymbol);
    Cm::Sym::UShortTypeSymbol* ushortTypeSymbol = new Cm::Sym::UShortTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(ushortTypeSymbol);
    integerTypes.push_back(ushortTypeSymbol);
    Cm::Sym::IntTypeSymbol* intTypeSymbol = new Cm::Sym::IntTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(intTypeSymbol);
    integerTypes.push_back(intTypeSymbol);
    Cm::Sym::UIntTypeSymbol* uintTypeSymbol = new Cm::Sym::UIntTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(uintTypeSymbol);
    integerTypes.push_back(uintTypeSymbol);
    Cm::Sym::LongTypeSymbol* longTypeSymbol = new Cm::Sym::LongTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(longTypeSymbol);
    integerTypes.push_back(longTypeSymbol);
    Cm::Sym::ULongTypeSymbol* ulongTypeSymbol = new Cm::Sym::ULongTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(ulongTypeSymbol);
    integerTypes.push_back(ulongTypeSymbol);
    Cm::Sym::FloatTypeSymbol* floatTypeSymbol = new Cm::Sym::FloatTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(floatTypeSymbol);
    Cm::Sym::DoubleTypeSymbol* doubleTypeSymbol = new Cm::Sym::DoubleTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(doubleTypeSymbol);
    Cm::Sym::NullPtrTypeSymbol* nullPtrTypeSymbol = new Cm::Sym::NullPtrTypeSymbol();
    symbolTable.AddPredefinedSymbolToGlobalScope(nullPtrTypeSymbol);

    MakeBoolOps(symbolTable, boolTypeSymbol);
    for (Cm::Sym::BasicTypeSymbol* integerType : integerTypes)
    {
        MakeIntegerOps(symbolTable, integerType);
    }
    MakeFloatingPointOps(symbolTable, floatTypeSymbol);
    MakeFloatingPointOps(symbolTable, doubleTypeSymbol);
    MakeCharOps(symbolTable, charTypeSymbol);
    MakeCharOps(symbolTable, wcharTypeSymbol);
    MakeCharOps(symbolTable, ucharTypeSymbol);

    MakeStandardConversions(symbolTable, sbyteTypeSymbol, byteTypeSymbol, shortTypeSymbol, ushortTypeSymbol, intTypeSymbol, uintTypeSymbol, longTypeSymbol, ulongTypeSymbol, 
        floatTypeSymbol, doubleTypeSymbol, charTypeSymbol, wcharTypeSymbol, ucharTypeSymbol, boolTypeSymbol);
}

void InitSymbolTable(Cm::Sym::SymbolTable& symbolTable, GlobalConceptData& globalConceptData)
{
    MakeBasicTypes(symbolTable);
    globalConceptData.AddIntrinsicConceptsToSymbolTable(symbolTable);
}

} } // namespace Cm::Core